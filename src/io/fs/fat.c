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

extern fatfs mainFilesystem;
extern partitionTable primaryPartitionTable;

/* Function to abstract away partition lba offsets.
   The fs object must have it's partition and block devices associated. */
u32int fatBlockRead(fatfs *fs, u32int start, u64int blkCount, void *dst)
{
#ifdef FAT_DEBUG
  printf("fatBlockRead: relative start = %x, blkCount = %x\n", start, blkCount);
#endif
  return fs->blockDevice->blockRead(fs->blockDevice->devID,
                                   (start + fs->part->lbaBegin), blkCount, dst);
}

u32int fatBlockWrite(fatfs *fs, u32int start, u64int blkCount, const void *src)
{
#ifdef FAT_DEBUG
  printf("fatBlockWrite: relative start = %08x\n", start);
#endif
  return fs->blockDevice->blockWrite(fs->blockDevice->devID,
                                     start + fs->part->lbaBegin, blkCount, src);
}

/* Mount a fat filesystem located on the partNum-th partition on block device dev. */
int fatMount(fatfs *fs, blockDevice *dev, int partNum)
{
#ifdef FAT_DEBUG
  printf("fatMount: mount partition %x\n", partNum);
#endif
  buffer = (char*)mallocBytes(0x1000);
  if (buffer == 0)
  {
    DIE_NOW(0, "fatMount: failed to allocate block buffer.\n");
  }
  else
  {
    memset((void*)buffer, 0x0, fs->sectorsPerCluster * fs->bytesPerSector);
  }

  //assume partitions have already been read
  fs->part = &primaryPartitionTable.partitions[partNum-1];
  if (!(fs->part->type == 0x0B || fs->part->type == 0x0C))
  {
    printf("Partition type code is not FAT32 (0x0B or 0x0C)\n");
    return -1;
  }
  fs->blockDevice = dev;

  //load first sector
  int blksRead = fatBlockRead(fs, 0, 1, buffer);
  if (blksRead != 1)
  {
    printf("Could not read FAT volume ID\n");
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
#ifdef FAT_DEBUG
  printf("fatMount: bytesPerSector %x, sectorsPerCluster %x, numReservedSectors %x\n",
        fs->bytesPerSector, fs->sectorsPerCluster, fs->numReservedSectors);
  printf("fatMount: numFats %x, sectorsPerFat %x, rootDirFirstCluster %x\n",
        fs->numFats, fs->sectorsPerFat, fs->rootDirFirstCluster);
#endif
    
  if (fs->bytesPerSector != 512)
  {
    printf("FAT32 bytes per sector not 512: %x\n", fs->bytesPerSector);
    return -1;
  }
  if (fs->numFats != 2)
  {
    printf("FAT32 number of FATs must be 2: %x\n", fs->numFats);
    return -1;
  }

  fs->fatBegin = fs->numReservedSectors;
  fs->clusterBegin = fs->numReservedSectors + fs->numFats * fs->sectorsPerFat;
#ifdef FAT_DEBUG
  printf("fatMount: fatBegin %x\n", fs->fatBegin);
  printf("fatMount: clusterBegin %x\n", fs->clusterBegin);
#endif
  fs->mounted = 1;
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
        printf("\n");

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

        printf("\n");
      }
    }
    //we've still not reached EOD, repeat
    currentCluster = fatGetNextClus(fs, currentCluster);
  }
  while (!FAT_EOC_MARKER(currentCluster));
}


/* Sees if the user filename matches the format in the FAT */
bool filenameMatch(char *user, char *fatname)
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
#ifdef FAT_DEBUG
  printf("fatGetNextClus: current cluster %08x, next cluster = %08x\n", clus, next);
#endif
  return next;
}

/* Set the value of the FAT entry to val */
void fatSetClusterValue(fatfs *fs, u32int clus, u32int val)
{
  u32int offset = (clus & 0x7f) * 4;
  fatBlockRead(fs, fs->fatBegin + (clus >> 7), 1, buffer);
#ifdef FAT_DEBUG
  printf("fatSetClusterValue: Writing %x to cluster: %08x\n", val, clus);
#endif
  *(u32int*)(buffer+offset) = val;

  //write back change
  fatBlockWrite(fs, fs->fatBegin + (clus >> 7), 1, buffer);
}


dentry *getPathDirEntry(fatfs *fs, char *fname, int createNew)
{
#ifdef FAT_DEBUG
  printf("getPathDirEntry: %s\n", fname);
#endif
  //root dir searching only, assume its a single file we're searching for
  dentry * dirEntry = (dentry*)mallocBytes(sizeof(dentry));
  if (dirEntry == 0)
  {
    DIE_NOW(0, "getPathDirEntry: failed to allocate dir entry struct.\n");
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
#ifdef FAT_DEBUG
          printf("getPathDirEntry: found directory! first cluster: %08x\n", 
                 dirEntry->firstCluster);
#endif
          //object is a directory
          dirEntry->isDirectory = TRUE;
          dirEntry->free = FALSE;
          dirEntry->valid = TRUE;
          return dirEntry;
        }

        // found a file
#ifdef FAT_DEBUG
        printf("getPathDirEntry: found file! first cluster: %08x\n", 
                dirEntry->firstCluster);
#endif
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

  int index = 0;
  for (index = 0; index < fs->sectorsPerFat; index++)
  {
    fatBlockRead(fs, fs->fatBegin + index, 1, buffer);
    int i = 0;
    for (i = 0; i < 128; i++)
    {
      val = *(u32int*)(buffer + i*4);
      if (val == 0x0)
      {
        //free cluster!
        return index * 128 + i;
      }
    } // loop through all entries in current sector of FAT
  } // loop through all sectors of FAT

  //no free clusters
  printf("No free clusters found!\n");
  //0 is an invalid cluster number, use as error code. caller must check for this
  return 0;
}


/* Writes n bytes from src to file */
int fwrite(fatfs *fs, file *handle, void *src, u32int length)
{
#ifdef FAT_DEBUG
  printf("fwrite: file '%s' data length %x\n", handle->dirEntry->filename, length);
#endif

  // we expect a free dentry
  if (handle->dirEntry->free)
  {
    printf("fwrite: file entry marked free.\n");
    return 0;
  }

  if (length == 0)
  {
    printf("fwrite: length to write - zero.\n");
    return 0;
  }

  u8int* outputData = (u8int*)src; 

  // we expect the file to exist already
  if (!handle->dirEntry->valid)
  {
    printf("fwrite: file '%s' dir entry is invalid!\n", handle->dirEntry->filename);
    return 0;
  }

  if (handle->dirEntry->isDirectory)
  {
    printf("fwrite: %s is a directory, not a file\n", handle->dirEntry->filename);
    return 0;
  }

  // 1. get last cluster number and how much it's filled in
  u32int currentCluster = handle->lastCluster;

  u32int nextByteIndex = handle->bytesInLastCluster;
  u32int byteNumber = 0;
  u32int bytesLeftInCluster = (fs->sectorsPerCluster * fs->bytesPerSector) -
                              handle->bytesInLastCluster;
  while (byteNumber < length)
  {
    // get current cluster into buffer
    fatBlockRead(fs, CLUSTER_REL_LBA(fs, currentCluster),
                 fs->sectorsPerCluster, buffer);
    
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

    fatBlockWrite(fs, CLUSTER_REL_LBA(fs, currentCluster),
                  fs->sectorsPerCluster, buffer);
    if (bytesLeftInCluster == 0)
    {
      // we filled the cluster with data. write cluster back to device
      if (byteNumber < length)
      {
        // if there are more bytes to write:
        // 1. find new empty cluster
        u32int allocatedCluster = fatGetFreeClus(fs);
        // 2. set FAT entry for current cluster to point to allocatedCluster
        fatSetClusterValue(fs, currentCluster, allocatedCluster);
        currentCluster = allocatedCluster;
        // 3. set allocated cluster entry in FAT as end of file
        fatSetClusterValue(fs, currentCluster, FAT_EOC_VAL);
        // 4. reset counters
        bytesLeftInCluster = fs->sectorsPerCluster * fs->bytesPerSector;
        nextByteIndex = 0;
        // 5. update file handle
        handle->lastCluster = currentCluster;
        handle->bytesInLastCluster = 0;
      }
    } // if no more free bytes left in cluster
  } // while more bytes left to write
  
  // adjust file size in directory entry in the FAT directory listing
  setFatDirEntry(fs, handle->dirEntry, handle->dirEntry->position);
  return length;
}


/* Read the data in a file in the root directory. */
int fread(fatfs *fs, file *handle, void *out, u32int maxlen)
{
#ifdef FAT_DEBUG
  printf("fread: file '%s' max data length %x\n", handle->dirEntry->filename, maxlen);
#endif

  int i;

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
#ifdef FAT_DEBUG
  printf("fdelete: %s\n", handle->dirEntry->filename);
#endif


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


file* fopen(fatfs* fs, char* fname)
{
#ifdef FAT_DEBUG
  printf("fopen: file '%s'\n", fname);
#endif

  file* f = (file*)mallocBytes(sizeof(file));
  if (f == 0)
  {
    DIE_NOW(0, "fopen: failed to allocate file struct.\n");
  }
  else
  {
    memset((void*)f, 0x0, sizeof(file));
  }

  f->dirEntry = getPathDirEntry(fs, fname, FALSE);
  
  if (!f->dirEntry->valid)
  {
    printf("fopen: dirEntry invalid for file %s .\n", fname);
    return 0;
  }

  if (f->dirEntry->free)
  {
    printf("fopen: file %s not found. Create new.\n", fname);
    return fnew(fs, fname);
  }

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

#ifdef FAT_DEBUG
  printf("fopen: f->dirEntry.position = %x\n", f->dirEntry->position);
  printf("fopen: f->dirEntry.parentCluster %x\n", f->dirEntry->parentCluster);  
  printf("fopen: f->dirEntry.firstCluster %x\n", f->dirEntry->firstCluster);
  printf("fopen: f->dirEntry.valid %x\n", f->dirEntry->valid);
  printf("fopen: f->dirEntry.free %x\n", f->dirEntry->free);
  printf("fopen: f->dirEntry.filename %s\n", f->dirEntry->filename);
#endif

  f->lastCluster = tempCluster;
  f->bytesInLastCluster = fileSizeLeft;

#ifdef FAT_DEBUG
  printf("fopen: f->lastCluster %x\n", f->lastCluster);
  printf("fopen: f->bytesInLastCluster %x\n", f->bytesInLastCluster);
#endif

  return f;
}

file* fnew(fatfs *fs, char *fname)
{
#ifdef FAT_DEBUG
  printf("fnew: file '%s'\n", fname);
#endif

  file* f = (file*)mallocBytes(sizeof(file));
  if (f == 0)
  {
    DIE_NOW(0, "fnew: failed to allocate file struct.\n");
  }
  else
  {
    memset((void*)f, 0x0, sizeof(file));
  }

  dentry *dirEntry = getPathDirEntry(fs, fname, FALSE);

  // we expect a free dentry
  if (!dirEntry->free)
  {
    printf("fnew: error, dentry not free!\n");
    return 0;
  }

  if (dirEntry->isDirectory)
  {
    printf("fnew: Directory exists with same name\n");
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
    printf("fnew: No free clusters\n");
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
  f->lastCluster = clusterNr;
  f->bytesInLastCluster = 0;

#ifdef FAT_DEBUG
  printf("fnew: f->dirEntry->position = %x\n", f->dirEntry->position);
  printf("fnew: f->dirEntry->parentCluster %x\n", f->dirEntry->parentCluster);  
  printf("fnew: f->dirEntry->firstCluster %x\n", f->dirEntry->firstCluster);
  printf("fnew: f->lastCluster = %x\n", f->lastCluster);
  printf("fnew: f->bytesInLastCluster %x\n", f->bytesInLastCluster);  
#endif

  return f;
}
