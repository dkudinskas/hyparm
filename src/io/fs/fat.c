#include "common/types.h"
#include "common/debug.h"
#include "common/alignFunctions.h"
#include "common/memFunctions.h"

#include "io/block.h"
#include "io/mmc.h"

#include "io/fs/fat.h"

//HACK implement a better fs system, this is clearly not going to work
//for anything more sophisticated than a single fs
char * buffer;

u32int nextFreeIndex;
extern fatfs mainFilesystem;
extern partitionTable primaryPartitionTable;

/* Function to abstract away partition lba offsets.
   The fs object must have it's partition and block devices associated. */
u32int fatBlockRead(fatfs *fs, u32int start, u64int blkCount, void *dst)
{
  DEBUG(FS_FAT, "fatBlockRead: relative start = %#x, blkCount = %#Lx" EOL, start, blkCount);
  return fs->blockDevice->blockRead(fs->blockDevice->devID,
                                   (start + fs->part->lbaBegin), blkCount, dst);
}

u32int fatBlockWrite(fatfs *fs, u32int start, u64int blkCount, const void *src)
{
  DEBUG(FS_FAT, "fatBlockWrite: relative start = %#.8x" EOL, start);
  return fs->blockDevice->blockWrite(fs->blockDevice->devID,
                                     start + fs->part->lbaBegin, blkCount, src);
}

/* Mount a fat filesystem located on the partNum-th partition on block device dev. */
int fatMount(fatfs *fs, blockDevice *dev, int partNum)
{
  DEBUG(FS_FAT, "fatMount: mount partition %#x" EOL, partNum);
  buffer = (char*)mallocBytes(0x1000);
  if (buffer == 0)
  {
    DIE_NOW(0, "fatMount: failed to allocate block buffer." EOL);
  }
  else
  {
    memset((void*)buffer, 0x0, 0x1000);
  }

  //assume partitions have already been read
  fs->part = &primaryPartitionTable.partitions[partNum-1];
  if (!(fs->part->type == 0x0B || fs->part->type == 0x0C))
  {
    printf("Partition type code is not FAT32 (0x0B or 0x0C)" EOL);
    return -1;
  }
  fs->blockDevice = dev;

  //load first sector
  int blksRead = fatBlockRead(fs, 0, 1, buffer);
  if (blksRead != 1)
  {
    printf("Could not read FAT volume ID" EOL);
    return -1;
  }

  //initial checks for fs validity
  if (buffer[510] != 0x55 || buffer[511] != 0xAA)
  {
    printf("Invalid FAT volume ID signature, must be 0xAA");
    return -1;
  }

  fs->bytesPerSector = uaLoadHWord(buffer+0x0B);
  fs->sectorsPerCluster = buffer[0x0D];
  fs->numReservedSectors = uaLoadHWord(buffer+0x0E);
  fs->numFats = buffer[0x10];
  fs->sectorsPerFat = uaLoadWord(buffer+0x24); //don't really need ua_load here
  fs->rootDirFirstCluster = uaLoadWord(buffer+0x2C);

  DEBUG(FS_FAT, "fatMount: bytesPerSector %#x, sectorsPerCluster %#x, numReservedSectors %#x" EOL,
        fs->bytesPerSector, fs->sectorsPerCluster, fs->numReservedSectors);
  DEBUG(FS_FAT, "fatMount: numFats %#x, sectorsPerFat %#x, rootDirFirstCluster %#x" EOL,
        fs->numFats, fs->sectorsPerFat, fs->rootDirFirstCluster);

  if (fs->bytesPerSector != 512)
  {
    printf("FAT32 bytes per sector not 512: %#x" EOL, fs->bytesPerSector);
    return -1;
  }
  if (fs->numFats != 2)
  {
    printf("FAT32 number of FATs must be 2: %#x" EOL, fs->numFats);
    return -1;
  }

  fs->fatBegin = fs->numReservedSectors;
  fs->clusterBegin = fs->numReservedSectors + fs->numFats * fs->sectorsPerFat;

  DEBUG(FS_FAT, "fatMount: fatBegin %#x" EOL, fs->fatBegin);
  DEBUG(FS_FAT, "fatMount: clusterBegin %#x" EOL, fs->clusterBegin);

  nextFreeIndex = 0;
  fs->mounted = TRUE;
  return 0;
}


/* Populate a directory entry from a byte record */
void loadFatDirEntry(char *record, dentry *d)
{
  int i;
  for (i = 0; i < 11; i++)
  {
    d->filename[i] = record[i];
  }
  d->attrib = record[0xB];
  d->firstClusterHigh = *((u16int*)(record+0x14));
  d->firstClusterLow = *((u16int*)(record+0x1A));

  d->firstCluster = (*(record+0x1A)) | (*(record+0x1B)) << 8 |
                     (*(record+0x14)) << 16 | (*(record+0x15)) << 24;
  //should always be word aligned, right?
  d->fileSize = *((u32int*)(record+0x1C));
}


/* prints file structure tree */
void tree(fatfs *fs, u32int currentCluster, u32int level)
{
  int i = 0;

  do
  {
    fatBlockRead(fs, CLUSTER_REL_LBA(fs, currentCluster), fs->sectorsPerCluster, buffer);
    dentry dirEntry;
    // each sector can hold 16 dentry records
    for (i = 0; i < (16 * fs->sectorsPerCluster); i++)
    {
      loadFatDirEntry(buffer + 32*i, &dirEntry);

      if (!(dirEntry.filename[0]))
      {
        return;
      }

      if (dirEntry.filename[0] == FAT_DE_UNUSED)
      {
        continue;
      }

      //check for long filenames, ignore for now
      if (dirEntry.attrib & FAT_LF_MASK)
      {
        continue;
      }

      u32int lvlIndex = 0;
      for (lvlIndex = 0; lvlIndex < level; lvlIndex++)
      {
        printf("|   ");
      }
      printf("|-- ");

      //anything now is directory data, check attrib for file/dir
      if (dirEntry.attrib & FAT_DE_DIR_MASK)
      {
        // print folder name
        int nameIndex;
        for (nameIndex = 0; nameIndex < 11; nameIndex++)
        {
          //spaces not allowed
          if (dirEntry.filename[nameIndex] == 0x20)
          {
            break;
          }
          printf("%c", dirEntry.filename[nameIndex]);
        }
        printf("" EOL);

        char this[11] = ".\0";
        char prev[11] = "..\0";
        bool dontCall = filenameMatch(this, dirEntry.filename) ||
                        filenameMatch(prev, dirEntry.filename);
        if (!dontCall)
        {
          tree(fs, dirEntry.firstCluster, level+1);
        }
      }
      else
      {
        // print filename
        int index;

        for (index = 0; index < 8; index++)
        {
          // spaces not allowed
          if (dirEntry.filename[index] == 0x20)
          {
            break;
          }
          printf("%c", dirEntry.filename[index]);
        }

        if (dirEntry.filename[8] != 0x20)
        {
          printf(".");
          for (index = 8; index < 11; index++)
          {
            //spaces not allowed
            if (dirEntry.filename[index] == 0x20)
            {
              break;
            }
            printf("%c", dirEntry.filename[index]);
          }
        }

        printf("" EOL);
      }
    }
    //we've still not reached EOD, repeat
    currentCluster = fatGetNextClus(fs, currentCluster);
  }
  while (!FAT_EOC_MARKER(currentCluster));
}


/* Sees if the user filename matches the format in the FAT */
bool filenameMatch(const char *user, const char *fatname)
{
  int i;
  for (i = 0; i < 11; i++)
  {
    if (fatname[i] == 0x20 && user[i] == '\0')
    {
      return (i > 0) ? TRUE : FALSE;
    }

    if (fatname[i] != user[i])
    {
      return FALSE;
    }
  }
  //matched all 11 characters so far
  return TRUE;
}

/* Read the FAT sector which contains the dentry for the given cluster.
   Returns the pointer offset into the buffer of the cluster's fat entry */
u32int fatLoadClusFatSector(fatfs *fs, u32int clus, char *buf)
{
  fatBlockRead(fs, fs->fatBegin + (clus >> 7), 1, buf);
  return (clus & 0x7f) * 4;
}

/* Gets the cluster number which is pointed to by clus */
u32int fatGetNextClus(fatfs *fs, u32int clus)
{
  u32int offset = fatLoadClusFatSector(fs, clus, buffer);
  u32int next = *(u32int*)(buffer+offset);
  DEBUG(FS_FAT, "fatGetNextClus: current cluster %#.8x, next cluster = %#.8x" EOL, clus, next);
  return next;
}

/* Set the value of the FAT entry to val */
void fatSetClusterValue(fatfs *fs, u32int clus, u32int val)
{
  u32int offset = (clus & 0x7f) * 4;
  fatBlockRead(fs, fs->fatBegin + (clus >> 7), 1, buffer);
  DEBUG(FS_FAT, "fatSetClusterValue: Writing %#x to cluster: %#.8x" EOL, val, clus);
  *(u32int*)(buffer+offset) = val;

  //write back change
  fatBlockWrite(fs, fs->fatBegin + (clus >> 7), 1, buffer);
}


dentry *getPathDirEntry(fatfs *fs, const char *fname, int createNew)
{
  DEBUG(FS_FAT, "getPathDirEntry: %s" EOL, fname);
  //root dir searching only, assume its a single file we're searching for
  dentry *dirEntry = (dentry*)mallocBytes(sizeof(dentry));
  if (dirEntry == 0)
  {
    DIE_NOW(0, "getPathDirEntry: failed to allocate dir entry struct." EOL);
  }
  else
  {
    memset((void*)dirEntry, 0x0, sizeof(dentry));
  }

  dirEntry->free = FALSE;

  int i;
  u32int currentCluster =  fs->rootDirFirstCluster;

  do
  {
    fatBlockRead(fs, CLUSTER_REL_LBA(fs, currentCluster), fs->sectorsPerCluster, buffer);
    for (i = 0; i < 16 * fs->sectorsPerCluster; i++)
    {
      loadFatDirEntry(buffer + 32*i, dirEntry);
      dirEntry->position = i;
      if (!(dirEntry->filename[0]))
      {
        // file wasn't found, return an a free directory entry. no need to create
        // by convention, treat empty dentries as files
        dirEntry->isDirectory = FALSE;
        dirEntry->parentCluster = currentCluster;
        dirEntry->free = TRUE;
        dirEntry->valid = TRUE;
        return dirEntry;
      }

      if (dirEntry->attrib & FAT_LF_MASK)
      {
        continue; //lfname, we support only 8.3 atm
      }

      if (filenameMatch(fname, dirEntry->filename))
      {
        dirEntry->parentCluster = currentCluster;
        if (dirEntry->attrib & FAT_DE_DIR_MASK)
        {
          DEBUG(FS_FAT, "getPathDirEntry: found directory! first cluster: %#.8x" EOL,
                 dirEntry->firstCluster);
          //object is a directory
          dirEntry->isDirectory = TRUE;
          dirEntry->free = FALSE;
          dirEntry->valid = TRUE;
          return dirEntry;
        }

        // found a file
        DEBUG(FS_FAT, "getPathDirEntry: found file! first cluster: %#.8x" EOL,
                dirEntry->firstCluster);
        dirEntry->isDirectory = FALSE;
        dirEntry->free = FALSE;
        dirEntry->valid = TRUE;
        return dirEntry;
      }
    }
    currentCluster = fatGetNextClus(fs, currentCluster);
  }
  while (!FAT_EOC_MARKER(currentCluster));

  dirEntry->valid = FALSE;
  return dirEntry;
}


/* Write a directory entry to a dentry-index position */
void setFatDirEntry(fatfs *fs, dentry *dirEntry, u32int position)
{
  u32int offset = position * 32;

  fatBlockRead(fs, CLUSTER_REL_LBA(fs, dirEntry->parentCluster),
               fs->sectorsPerCluster, buffer);
  int i;
  for (i = 0; i < 11; i++)
  {
    buffer[offset+i] = dirEntry->filename[i];
  }
  buffer[offset + 0xB] = dirEntry->attrib;

  //assume that the caller has sorted out the cluster hi/lows
  *((u16int*)(buffer+offset+0x14)) = dirEntry->firstClusterHigh;
  *((u16int*)(buffer+offset+0x1A)) = dirEntry->firstClusterLow;
  *((u32int*)(buffer+offset+0x1C)) = dirEntry->fileSize;
  fatBlockWrite(fs, CLUSTER_REL_LBA(fs, dirEntry->parentCluster),
                fs->sectorsPerCluster, buffer);
}


/* Scan the FAT for the first free cluster, returns the cluster number */
u32int fatGetFreeClus(fatfs *fs)
{
  u32int val = 0;

  // we make a crude assumption that no sectors that we looked at previously
  // suddenly became free! so start at FAT index that we finished last time
  for (; nextFreeIndex < fs->sectorsPerFat; nextFreeIndex++)
  {
    fatBlockRead(fs, fs->fatBegin + nextFreeIndex, 1, buffer);
    int i = 0;
    for (i = 0; i < 128; i++)
    {
      val = *(u32int*)(buffer + i*4);
      if (val == 0x0)
      {
        //free cluster!
        return nextFreeIndex * 128 + i;
      }
    } // loop through all entries in current sector of FAT
  } // loop through all sectors of FAT

  //no free clusters
  printf("No free clusters found!" EOL);
  //0 is an invalid cluster number, use as error code. caller must check for this
  return 0;
}


/* Writes n bytes from src to file */
int fwrite(fatfs *fs, file *handle, void *src, u32int length)
{
  DEBUG(FS_FAT, "fwrite: file '%s' data length %#x" EOL, handle->dirEntry->filename, length);

  // we expect the file to exist already
  if (!handle->dirEntry->valid)
  {
    printf("fwrite: file '%s' dir entry is invalid!" EOL, handle->dirEntry->filename);
    return 0;
  }

  // we expect a file to be used, not free
  if (handle->dirEntry->free)
  {
    printf("fwrite: file entry marked free" EOL);
    return 0;
  }

  // there should be data to write!
  if (length == 0)
  {
    printf("fwrite: length to write - zero." EOL);
    return 0;
  }

  // and file handle should point to a file not a directory
  if (handle->dirEntry->isDirectory)
  {
    printf("fwrite: %s is a directory, not a file" EOL, handle->dirEntry->filename);
    return 0;
  }

  u8int *outputData = (u8int *)src;

  // 1. get last cluster number and how much it's filled in
  u32int currentCluster = handle->lastClusterNr;

  u32int nextByteIndex = handle->bytesInLastCluster;
  u32int byteNumber = 0;
  u32int bytesLeftInCluster = (fs->sectorsPerCluster * fs->bytesPerSector) -
                              handle->bytesInLastCluster;
  while (byteNumber < length)
  {
    // last cluster is buffered in file handle
    buffer = (char *)handle->lastCluster;

    // loop through output data, writing it byte-by-byte into buffer
    while (byteNumber < length && bytesLeftInCluster != 0)
    {
      buffer[nextByteIndex] = outputData[byteNumber];
      bytesLeftInCluster--;
      byteNumber++;
      nextByteIndex++;
      handle->dirEntry->fileSize++;
      handle->bytesInLastCluster++;
    }

    if (bytesLeftInCluster == 0)
    {
      // we filled the cluster with data. write cluster back to device
      fatBlockWrite(fs, CLUSTER_REL_LBA(fs, currentCluster), fs->sectorsPerCluster, buffer);

      // adjust file size in directory entry in the FAT directory listing
      setFatDirEntry(fs, handle->dirEntry, handle->dirEntry->position);

      // if there are more bytes to write:
      // 1. find new empty cluster
      u32int allocatedCluster = fatGetFreeClus(fs);
      if (allocatedCluster == 0)
      {
        DIE_NOW(0, "fwrite: cannot find a new free cluster. out of space??");
      }
      // 2. set FAT entry for current cluster to point to allocatedCluster
      fatSetClusterValue(fs, currentCluster, allocatedCluster);
      currentCluster = allocatedCluster;
      // 3. set allocated cluster entry in FAT as end of file
      fatSetClusterValue(fs, currentCluster, FAT_EOC_VAL);
      // 4. reset counters, no need to zero the write buffer
      bytesLeftInCluster = fs->sectorsPerCluster * fs->bytesPerSector;
      nextByteIndex = 0;
      // 5. update file handle
      handle->lastClusterNr = currentCluster;
      handle->bytesInLastCluster = 0;
    } // if no more free bytes left in cluster
  } // while more bytes left to write

  return length;
}


/* Read the data in a file in the root directory. */
int fread(fatfs *fs, file *handle, void *out, u32int maxlen)
{
  DEBUG(FS_FAT, "fread: file '%s' max data length %#x" EOL, handle->dirEntry->filename, maxlen);

  u32int i;

  u32int currentCluster = handle->dirEntry->firstCluster;
  u32int currentLength = 0;

  do
  {
    fatBlockRead(fs, CLUSTER_REL_LBA(fs, currentCluster),
                 fs->sectorsPerCluster, buffer);

    // for now ignore maxlen
    if ((handle->dirEntry->fileSize - currentLength) <
        (fs->sectorsPerCluster * fs->bytesPerSector))
    {
      u32int left = handle->dirEntry->fileSize - currentLength;
      // remaining data to read is found in this cluster
      for (i = 0; i < left; i++)
      {
        if (currentLength < maxlen)
        {
          *((char*)out) = buffer[i];
          out = ((char*)out)+1;
          currentLength++;
        }
        else
        {
          return currentLength;
        }
      } // for
    } // if
    else
    {
      for (i = 0; i < (fs->sectorsPerCluster * fs->bytesPerSector); i++)
      {
        if (currentLength < maxlen)
        {
          *((char*)out) = buffer[i];
          out = ((char*)out)+1;
          currentLength++;
        }
        else
        {
          return currentLength;
        }
      }
    }

    currentCluster = fatGetNextClus(fs, currentCluster);
  }
  while (!FAT_EOC_MARKER(currentCluster));

  return currentLength;
}


/* delete a file in the root directory. */
int fdelete(fatfs *fs, file *handle)
{
  DEBUG(FS_FAT, "fdelete: %s" EOL, handle->dirEntry->filename);

  // file was found, information now in direntry;
  // currentCluster holds the cluster number to write back to.
  fatBlockRead(fs, CLUSTER_REL_LBA(fs, handle->dirEntry->parentCluster),
               fs->sectorsPerCluster, buffer);

  // mark entry 'unused'
  buffer[handle->dirEntry->position * FAT32_DIR_ENTRY_LENGTH] = FAT_DE_UNUSED;

  // write buf back to position
  fatBlockWrite(fs, CLUSTER_REL_LBA(fs, handle->dirEntry->parentCluster),
                fs->sectorsPerCluster, buffer);

  // now free up the used cluster chain
  u32int workingCluster = handle->dirEntry->firstCluster;
  u32int fatSect = fs->sectorsPerFat+1;
  u32int offset = 0;
  do
  {
    // get FAT
    if (fatSect != (workingCluster >> 7))
    {
      fatSect = workingCluster >> 7;
      fatBlockRead(fs, fs->fatBegin + fatSect, 1, buffer);
    }
    offset = workingCluster & 0x7F;

    u32int* entry = (u32int*)(&buffer[offset*4]);
    workingCluster = *entry;
    *entry = 0x0;

    // update FAT table block
    fatBlockWrite(fs, fs->fatBegin + fatSect, 1, buffer);
  }
  while (!FAT_EOC_MARKER(workingCluster));

  handle->dirEntry->valid = FALSE;
  return 0;
} // end fuction

/* file close function: empties write buffer */
void fclose(fatfs *fs, file *handle)
{
  DEBUG(FS_FAT, "fclose: file '%s'" EOL, handle->dirEntry->filename);

  if (!fs->mounted)
  {
    printf("fclose: file system not mounted" EOL);
    return;
  }

  if (handle->bytesInLastCluster != 0)
  {
    // there is data in last cluster!
    fatBlockWrite(fs, CLUSTER_REL_LBA(fs, handle->lastClusterNr), fs->sectorsPerCluster, buffer);

    // adjust file size in directory entry in the FAT directory listing
    setFatDirEntry(fs, handle->dirEntry, handle->dirEntry->position);

    // update file handle
    handle->lastClusterNr = 0;
    handle->bytesInLastCluster = 0;

    /*
     * FIXME: Once we have free for malloc, we should free this memory, not just invalidate pointer
     */
    handle->dirEntry = 0;
  }
}


file* fopen(fatfs *fs, const char *fname)
{
  DEBUG(FS_FAT, "fopen: file '%s'" EOL, fname);

  if (!fs->mounted)
  {
    return 0;
  }

  // allocate space for file handle
  file *f = (file *)mallocBytes(sizeof(file));
  if (f == 0)
  {
    DIE_NOW(0, "fopen: failed to allocate file struct." EOL);
  }
  memset(f, 0, sizeof(file));

  f->dirEntry = getPathDirEntry(fs, fname, FALSE);

  if (!f->dirEntry->valid)
  {
    printf("fopen: dirEntry invalid for file %s ." EOL, fname);
    return 0;
  }

  // file is free. Make it not free.
  if (f->dirEntry->free)
  {
    printf("fopen: file %s not found. Create new." EOL, fname);
    return fnew(fs, fname);
  }

  // everything seems fine, allocate buffer space
  f->lastCluster = (u8int *)mallocBytes(fs->sectorsPerCluster * fs->bytesPerSector);
  if (f->lastCluster == 0)
  {
    DIE_NOW(0, "fopen: failed to allocate buffer for last cluster");
  }
  memset(f->lastCluster, 0, fs->sectorsPerCluster * fs->bytesPerSector);

  // follow FAT chain to last cluster
  u32int currentCluster = f->dirEntry->firstCluster;
  u32int tempCluster = 0;
  u32int fatSect = fs->sectorsPerFat+1;
  u32int offset = 0;
  u32int fileSizeLeft = f->dirEntry->fileSize;
  do
  {
    // get FAT
    if (fatSect != (currentCluster >> 7))
    {
      fatSect = currentCluster >> 7;
      fatBlockRead(fs, fs->fatBegin + fatSect, 1, buffer);
    }

    offset = currentCluster & 0x7F;
    tempCluster = currentCluster;
    currentCluster = *(u32int*)(&buffer[offset*4]);
    if (!FAT_EOC_MARKER(currentCluster))
    {
      fileSizeLeft = fileSizeLeft - fs->sectorsPerCluster * fs->bytesPerSector;
    }
  }
  while (!FAT_EOC_MARKER(currentCluster));

  DEBUG(FS_FAT, "fopen: f->dirEntry.position = %#x" EOL, f->dirEntry->position);
  DEBUG(FS_FAT, "fopen: f->dirEntry.parentCluster %#x" EOL, f->dirEntry->parentCluster);
  DEBUG(FS_FAT, "fopen: f->dirEntry.firstCluster %#x" EOL, f->dirEntry->firstCluster);
  DEBUG(FS_FAT, "fopen: f->dirEntry.valid %#x" EOL, f->dirEntry->valid);
  DEBUG(FS_FAT, "fopen: f->dirEntry.free %#x" EOL, f->dirEntry->free);
  DEBUG(FS_FAT, "fopen: f->dirEntry.filename %s" EOL, f->dirEntry->filename);

  f->lastClusterNr = tempCluster;
  f->bytesInLastCluster = fileSizeLeft;

  DEBUG(FS_FAT, "fopen: f->lastClusterNr %#x" EOL, f->lastClusterNr);
  DEBUG(FS_FAT, "fopen: f->bytesInLastCluster %#x" EOL, f->bytesInLastCluster);

  // now we have the last cluster number, and how much data is in it
  // we need just to read this bit of data into the buffer
  fatBlockRead(fs, CLUSTER_REL_LBA(fs, f->lastClusterNr), fs->sectorsPerCluster, f->lastCluster);

  return f;
}

file *fnew(fatfs *fs, const char *fname)
{
  DEBUG(FS_FAT, "fnew: file '%s'" EOL, fname);

  if (!fs->mounted)
  {
    return 0;
  }

  file *f = (file *)mallocBytes(sizeof(file));
  if (f == 0)
  {
    DIE_NOW(0, "fnew: failed to allocate file struct." EOL);
  }
  else
  {
    memset(f, 0, sizeof(file));
  }

  dentry *dirEntry = getPathDirEntry(fs, fname, FALSE);

  // we expect a free dentry
  if (!dirEntry->free)
  {
    printf("fnew: error, dentry not free!" EOL);
    return 0;
  }

  // we expect a unique filename -- no directories with the same name
  if (dirEntry->isDirectory)
  {
    printf("fnew: Directory exists with same name" EOL);
    return 0;
  }

  // write a really simple filename, ignore extensions
  int i;
  for (i=0; i < 8; i++)
  {
    if (fname[i] == '\0')
    {
      dirEntry->filename[i] = ' ';
    }
    else
    {
      dirEntry->filename[i] = fname[i];
    }
  }
  for (i = 8; i < 11; i++)
  {
    dirEntry->filename[i] = ' ';
  }

  // create empty file
  dirEntry->firstClusterHigh = 0;
  dirEntry->firstClusterLow = 0;
  dirEntry->firstCluster = 0;
  dirEntry->fileSize = 0;
  dirEntry->attrib = 0;

  // find a free cluster to start writing this file
  u32int clusterNr = fatGetFreeClus(fs);
  if (!clusterNr)
  {
    printf("fnew: No free clusters" EOL);
    return 0;
  }

  // immediatelly mark cluster as occupied, using end of chain marker
  fatSetClusterValue(fs, clusterNr, FAT_EOC_VAL);

  dirEntry->firstClusterHigh = clusterNr >> 16;
  dirEntry->firstClusterLow = clusterNr;
  dirEntry->valid = TRUE;
  dirEntry->free = FALSE;

  setFatDirEntry(fs, dirEntry, dirEntry->position);

  f->dirEntry = dirEntry;
  f->lastClusterNr = clusterNr;
  f->bytesInLastCluster = 0;

  DEBUG(FS_FAT, "fnew: f->dirEntry->position = %#x" EOL, f->dirEntry->position);
  DEBUG(FS_FAT, "fnew: f->dirEntry->parentCluster %#x" EOL, f->dirEntry->parentCluster);
  DEBUG(FS_FAT, "fnew: f->dirEntry->firstCluster %#x" EOL, f->dirEntry->firstCluster);
  DEBUG(FS_FAT, "fnew: f->lastClusterNr = %#x" EOL, f->lastClusterNr);
  DEBUG(FS_FAT, "fnew: f->bytesInLastCluster %#x" EOL, f->bytesInLastCluster);

  // everything seems fine, allocate buffer space
  f->lastCluster = (u8int *)mallocBytes(fs->sectorsPerCluster * fs->bytesPerSector);
  if (f->lastCluster == 0)
  {
    DIE_NOW(0, "fnew: failed to allocate buffer for last cluster");
  }
  memset(f->lastCluster, 0, fs->sectorsPerCluster * fs->bytesPerSector);

  return f;
}
