#include "common/types.h"
#include "common/debug.h"
#include "common/alignFunctions.h"
#include "common/memFunctions.h"

#include "io/block.h"
#include "io/mmc.h"

#include "io/fs/fat.h"

//HACK implement a better fs system, this is clearly not going to work
//for anything more sophisticated than a single fs

fatfs mainFilesystem;
partitionTable primaryPartitionTable;

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
  //assume partitions have already been read
  fs->part = &primaryPartitionTable.partitions[partNum-1];
  if (!(fs->part->type == 0x0B || fs->part->type == 0x0C))
  {
    printf("Partition type code is not FAT32 (0x0B or 0x0C)\n");
    return -1;
  }
  fs->blockDevice = dev;

  //load first sector
  char sector[512];
  int blksRead = fatBlockRead(fs, 0, 1, sector);
  if (blksRead != 1)
  {
    printf("Could not read FAT volume ID\n");
    return -1;
  }

  //initial checks for fs validity
  if (sector[510] != 0x55 || sector[511] != 0xAA)
  {
    printf("Invalid FAT volume ID signature, must be 0xAA");
    return -1;
  }
  
  fs->bytesPerSector = uaLoadHWord(sector+0x0B);
  fs->sectorsPerCluster = sector[0x0D];
  fs->numReservedSectors = uaLoadHWord(sector+0x0E);
  fs->numFats = sector[0x10];
  fs->sectorsPerFat = uaLoadWord(sector+0x24); //don't really need ua_load here
  fs->rootDirFirstCluster = uaLoadWord(sector+0x2C);
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

/* Write a directory entry to a dentry-index position */
void writeFatDirEntry(fatfs *fs, dentry *dirEntry, u32int position)
{
  u32int offset = position * 32;
  char * buf = (char*)mallocBytes(fs->sectorsPerCluster * fs->bytesPerSector);
  if (buf == 0)
  {
    DIE_NOW(0, "writeFatDirEntry: failed to allocate read buffer.\n");
  }
  else
  {
    memset((void*)buf, 0x0, fs->sectorsPerCluster * fs->bytesPerSector);
  }

  fatBlockRead(fs, CLUSTER_REL_LBA(fs, dirEntry->parentCluster),
               fs->sectorsPerCluster, buf);

  int i;
  for (i = 0; i < 11; i++)
  {
    buf[offset+i] = dirEntry->filename[i];
  }
  buf[offset + 0xB] = dirEntry->attrib;

  //assume that the caller has sorted out the cluster hi/lows
  *((u16int*)(buf+offset+0x14)) = dirEntry->firstClusterHigh;
  *((u16int*)(buf+offset+0x1A)) = dirEntry->firstClusterLow;
  *((u32int*)(buf+offset+0x1C)) = dirEntry->fileSize;
  fatBlockWrite(fs, CLUSTER_REL_LBA(fs, dirEntry->parentCluster),
                fs->sectorsPerCluster, buf);
}


/* prints file structure tree */
void tree(fatfs *fs, u32int currentCluster, u32int level)
{
  int i = 0;
  char * buf = (char*)mallocBytes(fs->sectorsPerCluster * fs->bytesPerSector);
  if (buf == 0)
  {
    DIE_NOW(0, "farRootLfs: failed to allocate read buffer.\n");
  }
  else
  {
    memset((void*)buf, 0x0, fs->sectorsPerCluster * fs->bytesPerSector);
  }

  do
  {
    fatBlockRead(fs, CLUSTER_REL_LBA(fs, currentCluster), fs->sectorsPerCluster, buf);
    dentry dirEntry;
    // each sector can hold 16 dentry records
    for (i = 0; i < (16 * fs->sectorsPerCluster); i++)
    {
      loadFatDirEntry(buf + 32*i, &dirEntry);

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


/* Convert a null-termed normal filename string into upper case */
void nameToUpper(char *s)
{
  while (*s)
  {
    if (*s >= 'a' && *s <= 'z')
      *s -= 32;
    s++;
  }
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
  printf("fatLoadClusFatSector: clus %x\n", clus);
  u32int sector = clus >> 7;
  printf("fatLoadClusFatSector: sector %x\n", sector);
  u32int offset = (clus & 0x7f) * 4;
  printf("fatLoadClusFatSector: offset %x\n", offset);
  fatBlockRead(fs, fs->fatBegin + sector, 1, buf);
  return offset;
}

/* Gets the cluster number which is pointed to by clus */
u32int fatGetNextClus(fatfs *fs, u32int clus)
{
  char * buf = (char*)mallocBytes(fs->bytesPerSector);
  if (buf == 0)
  {
    DIE_NOW(0, "fatGetNextClus: failed to allocate read buffer.\n");
  }
  else
  {
    memset((void*)buf, 0x0, fs->bytesPerSector);
  }
  printf("fatGetNextClus: clus %x\n", clus);
  u32int offset = fatLoadClusFatSector(fs, clus, buf);
  printf("fatGetNextClus: offs %x\n", offset);
  u32int next = *(u32int*)(buf+offset);
  printf("fatGetNextClus: next %x\n", next);
#ifdef FAT_DEBUG
  printf("fatGetNextClus: current cluster %08x, next cluster = %08x\n", clus, next);
#endif
  return next;
}

/* Set the value of the FAT entry to val */
void fatSetClusterValue(fatfs *fs, u32int clus, u32int val)
{
  char buf[512];
  u32int offset = fatLoadClusFatSector(fs, clus, buf);
#ifdef FAT_DEBUG
  printf("fatSetClusterValue: Writing %x to cluster: %08x\n", val, clus);
#endif
  *(u32int*)(buf+offset) = val;

  //write back change
  fatBlockWrite(fs, fs->fatBegin + clus / 128, 1, buf);
}


dentry getPathDirEntry(fatfs *fs, char *fname, int createNew)
{
  //root dir searching only, assume its a single file we're searching for
  nameToUpper(fname);
  dentry dirEntry = {{0}}; //stop gcc complaining about uninitialized...
  dirEntry.free = 0;

  int i;
  char * buf = (char*)mallocBytes(fs->sectorsPerCluster * fs->bytesPerSector);
  if (buf == 0)
  {
    DIE_NOW(0, "getPathDirEntry: failed to allocate read buffer.\n");
  }
  else
  {
    memset((void*)buf, 0x0, fs->sectorsPerCluster * fs->bytesPerSector);
  }

  u32int currentCluster =  fs->rootDirFirstCluster;

  do
  {
    fatBlockRead(fs, CLUSTER_REL_LBA(fs, currentCluster), fs->sectorsPerCluster, buf);
    for (i = 0; i < 16 * fs->sectorsPerCluster; i++)
    {
      loadFatDirEntry(buf + 32*i, &dirEntry);
      dirEntry.position = i;
      if (!(dirEntry.filename[0]))
      {
        // file wasn't found, return an a free directory entry. no need to create
        // by convention, treat empty dentries as files
        dirEntry.isDirectory = FALSE;
        dirEntry.parentCluster = currentCluster;
        dirEntry.free = 1;
        dirEntry.valid = TRUE;
        return dirEntry;
      }

      if (dirEntry.attrib & FAT_LF_MASK)
      {
        continue; //lfname, we support only 8.3 atm
      }

      if (filenameMatch(fname, dirEntry.filename))
      {
        dirEntry.parentCluster = currentCluster;
        if (dirEntry.attrib & FAT_DE_DIR_MASK)
        {
#ifdef FAT_DEBUG
          printf("found directory! first cluster: %08x\n", dirEntry.firstCluster);
#endif
          //object is a directory
          dirEntry.isDirectory = TRUE;
          dirEntry.free = FALSE;
          dirEntry.valid = TRUE;
          return dirEntry;
        }

        // found a file
#ifdef FAT_DEBUG
        printf("found file! first cluster: %08x\n", dirEntry.firstCluster);
#endif
        dirEntry.isDirectory = FALSE;
        dirEntry.free = FALSE;
        dirEntry.valid = TRUE;
        return dirEntry;
      }
    }
    currentCluster = fatGetNextClus(fs, currentCluster);
  }
  while (!FAT_EOC_MARKER(currentCluster));

  // if we reach end of directory, the file does not exist
  if (createNew)
  {
    //current cluster is full, find a free cluster and extend the current directory
    u32int freeClus = fatGetFreeClus(fs);
    if (!freeClus)
    {
      DIE_NOW(0, "FAT32: Could not allocate free cluster to extend directory\n");
      dirEntry.valid = FALSE;
      return dirEntry;
    }
    // set current directory cluster to point to newly found free cluster
    fatSetClusterValue(fs, currentCluster, freeClus);
    dirEntry.parentCluster = freeClus;
    dirEntry.free = TRUE;
    dirEntry.valid = TRUE;
    dirEntry.position = 0;
    // now write the end of directory entry in the new cluster, 
    // this will be the 2nd dentry record in the new cluster

    dentry eod;
    eod.parentCluster = freeClus;
    eod.filename[0] = 0;
    eod.attrib = 0;
    eod.firstClusterHigh = 0;
    eod.firstClusterLow = 0;
    eod.fileSize = 0;
    writeFatDirEntry(fs, &eod, 1);
    return dirEntry;
  }
  else
  {
    dirEntry.valid = FALSE;
    return dirEntry;
  }
}

/* Scan the FAT for the first free cluster, returns the cluster number */
u32int fatGetFreeClus(fatfs *fs)
{
  char buf[512];
  u32int clusNum = fs->rootDirFirstCluster;
  u32int sectorOffset = clusNum / 128; //128 records in a sector for fat32
  u32int val;

  int index;
  for (index = 0; index < fs->sectorsPerFat; index++)
  {
    fatBlockRead(fs, fs->fatBegin + sectorOffset, 1, buf);
    int i;
    for (i = 0; i < 128; i++)
    {
      // clusNum indexes into the FAT, but we offset it here to index into the buffer
      val = *(u32int*)(buf + (clusNum - 128 * sectorOffset)*4);
      if (val == 0x0)
      {
        //free cluster
        return clusNum;
      }
      clusNum++;
      sectorOffset = clusNum / 128;
    }
  }

  //no free clusters
  printf("No free clusters found!\n");
  //0 is an invalid cluster number, use as error code. caller must check for this
  return 0;
}

/* Writes n bytes from src to fname. fname is truncated to 8 characters */
int fatWriteFile(fatfs *fs, char *fname, void *src, u32int lenToWrite)
{
  char * buf = (char*)mallocBytes(fs->sectorsPerCluster * fs->bytesPerSector);
  if (buf == 0)
  {
    DIE_NOW(0, "getPathDirEntry: failed to allocate read buffer.\n");
  }
  else
  {
    memset((void*)buf, 0x0, fs->sectorsPerCluster * fs->bytesPerSector);
  }

  int i;
  dentry dirEntry = getPathDirEntry(fs, fname, TRUE);

  nameToUpper(fname);

  // we expect a free dentry
  if (!dirEntry.free)
  {
    printf("ERROR: dentry not free!\n");
    return 0;
  }

  if (dirEntry.isDirectory)
  {
    printf("Directory exists with same name\n");
    return 0;
  }

  //write a really simple filename, ignore extensions
  for (i=0; i < 8; i++)
  {
    if (fname[i] == '\0')
    {
      dirEntry.filename[i] = ' ';
    }
    else
    {
      dirEntry.filename[i] = fname[i];
    }
  }
  for (i = 8; i < 11; i++)
  {
    dirEntry.filename[i] = ' ';
  }
  dirEntry.attrib = 0;
  dirEntry.fileSize = lenToWrite;

  if (lenToWrite == 0)
  {
    //create empty file
    dirEntry.firstClusterHigh = 0;
    dirEntry.firstClusterLow = 0;
    dirEntry.firstCluster = 0;
    writeFatDirEntry(fs, &dirEntry, dirEntry.position);
    // written an empty file record
    return 0;
  }

  //find a free cluster to start writing this file
  u32int clusterNr = fatGetFreeClus(fs);
  if (!clusterNr)
  {
    printf("fatWriteFile: No free clusters\n");
    return 0;
  }

  dirEntry.firstClusterHigh = clusterNr >> 16;
  dirEntry.firstClusterLow = clusterNr;
  
  writeFatDirEntry(fs, &dirEntry, dirEntry.position);

  u32int count = 0;
  u32int clusterSize = fs->bytesPerSector * fs->sectorsPerCluster;
  do
  {
    if (count + clusterSize > lenToWrite)
    {
      //current cluster is final and will only be partially used
      for (i = 0; count < lenToWrite; i++, count++)
      {
        buf[i] = ((char *)src)[count];
      }

      fatBlockWrite(fs, CLUSTER_REL_LBA(fs, clusterNr), fs->sectorsPerCluster, buf);
      //write the end of chain marker
      fatSetClusterValue(fs, clusterNr, FAT_EOC_VAL);
      return count;
    }
    else
    {
      for (i=0; i < clusterSize; i++, count++)
      {
        buf[i] = ((char *)src)[count];
      }

      fatBlockWrite(fs, CLUSTER_REL_LBA(fs, clusterNr), fs->sectorsPerCluster, buf);
      u32int nextClusterNr = fatGetFreeClus(fs);
      
      if (count < lenToWrite)
      {
        fatSetClusterValue(fs, clusterNr, nextClusterNr);
        clusterNr = nextClusterNr;
      }
      else
      {
        //exactly finish on cluster boundary
        return count;
      }
    }
  }
  while (count < lenToWrite);
  return count;
}

/* Read the data in a file in the root directory. */
int fatReadFile(fatfs *fs, char *fname, void *out, u32int maxlen)
{
  char * buf = (char*)mallocBytes(fs->sectorsPerCluster * fs->bytesPerSector);
  if (buf == 0)
  {
    DIE_NOW(0, "fatReadFile: failed to allocate read buffer.\n");
  }
  else
  {
    memset((void*)buf, 0x0, fs->sectorsPerCluster * fs->bytesPerSector);
  }

  int i;

  dentry dirEntry = getPathDirEntry(fs, fname, FALSE);
  if (dirEntry.free)
  {
    printf("fatReadFile: File '%s' does not exist!\n", fname);
    return 0;
  }

  u32int currentCluster = dirEntry.firstCluster;
  u32int currentLength = 0;

  do
  {
    fatBlockRead(fs, CLUSTER_REL_LBA(fs, currentCluster), fs->sectorsPerCluster, buf);

    //for now ignore maxlen
    if ((dirEntry.fileSize - currentLength) < 
        (fs->sectorsPerCluster * fs->bytesPerSector))
    {
      u32int left = dirEntry.fileSize - currentLength;
      // remaining data to read is found in this cluster
      for (i = 0; i < left; i++)
      {
        if (currentLength < maxlen)
        {
          *((char*)out) = buf[i];
          out = ((char*)out)+1;
          currentLength++;
        }
        else
        {
          break;
        }
      } // for
    } // if 
    else
    {
      for (i = 0; i < (fs->sectorsPerCluster * fs->bytesPerSector); i++)
      {
        if (currentLength < maxlen)
        {
          *((char*)out) = buf[i];
          out = ((char*)out)+1;
          currentLength++;
        }
      }
    }
    currentCluster = fatGetNextClus(fs, currentCluster);
  }
  while (!FAT_EOC_MARKER(currentCluster));

  return currentLength;
}


/* delete a file in the root directory. */
int fatDeleteFile(fatfs *fs, char *fname)
{
#ifdef FAT_DEBUG
  printf("fatDeleteFile: %s\n", fname);
#endif

  /* what is involved in deleting a file in the root folder?
     we need to get the root directory file listing cluster
     find the directory entry of the file in question
     make the first byte of the entry 0xE5 (a.k.a. unused)
     now file doesnt exist.
     also need to free up all (if any) of the clusters occupied by the file
     look at the file's directory entry to get it's first cluster number
     load the FAT, find the entry for that cluster, and mark it as free
     follow the file chain in FAT marking all following (if any) clusters as free */

  char * buf = (char*)mallocBytes(fs->sectorsPerCluster * fs->bytesPerSector);
  if (buf == 0)
  {
    DIE_NOW(0, "fatDeleteFile: failed to allocate read buffer.\n");
  }
  else
  {
    memset((void*)buf, 0x0, fs->sectorsPerCluster * fs->bytesPerSector);
  }

  // root dir searching only, assume its a single file we're searching for
  nameToUpper(fname);
  dentry dirEntry = {{0}}; //stop gcc complaining about uninitialized...
  dirEntry.free = 0;
  int i = 0;
  bool found = FALSE;

  u32int currentCluster =  fs->rootDirFirstCluster;

  do
  {
    fatBlockRead(fs, CLUSTER_REL_LBA(fs, currentCluster), fs->sectorsPerCluster, buf);
    for (i = 0; i < 16 * fs->sectorsPerCluster; i++)
    {
      loadFatDirEntry(buf + FAT32_DIR_ENTRY_LENGTH * i, &dirEntry);
      if (dirEntry.filename[0] == FAT_DE_EOD)
      {
        // end of directory! file wasn't found, return an a free directory entry.
        printf("fatDeleteFile: EOD foudn! '%s' not in root directory\n", fname);
        return -1;
      }

      if (dirEntry.attrib & FAT_LF_MASK)
      {
        continue; //lfname, we support only 8.3 atm
      }

      if (filenameMatch(fname, dirEntry.filename))
      {
        printf("fatDeleteFile: found filename match.\n");
        if (dirEntry.attrib & FAT_DE_DIR_MASK)
        {
          // filename match is a folder.
          // TODO: implement delete folder 
          printf("fatDeleteFile: file '%s' is a folder, unimplemented\n", fname);
          return -1;
        }

        // found a file
#ifdef FAT_DEBUG
        printf("fatDeleteFile: found file! first cluster: %08x\n",
               dirEntry.firstCluster);
#endif
        dirEntry.parentCluster = currentCluster;
        dirEntry.isDirectory = FALSE;
        dirEntry.free = FALSE;
        dirEntry.valid = TRUE;
        dirEntry.position = i;
        found = TRUE;
        break;
      } // filename match
    } // for loop
    currentCluster = fatGetNextClus(fs, currentCluster);
  }
  while (!FAT_EOC_MARKER(currentCluster) && !found);

  if (!found)
  {
    printf("fatDeleteFile: file '%s' not found in root directory\n", fname);
    return -1;
  }

  // file was found, information now in direntry;
  // buf holds the block with the file entry
  // dirEntry.position now holds the position of directory entry in block
  // currentCluster holds the cluster number to write back to.
  printf("fatDeleteFile: file %s found in cluster %x\n", fname, dirEntry.firstCluster);
  printf("fatDeleteFile: position in cluster %x\n", dirEntry.position);
  
  // make entry 'unused'
  buf[dirEntry.position * FAT32_DIR_ENTRY_LENGTH] = FAT_DE_UNUSED;
  
  for (i = 0; i < FAT32_DIR_ENTRY_LENGTH; i++)
  {
    printf("%02x ", buf[dirEntry.position * FAT32_DIR_ENTRY_LENGTH + i]);
  }
  printf("\n");

  // write buf back to position
  fatBlockWrite(fs, CLUSTER_REL_LBA(fs, dirEntry.parentCluster),
                fs->sectorsPerCluster, buf);

  u32int workingCluster = dirEntry.firstCluster;
  u32int fatSect = fs->sectorsPerFat+1;
  u32int offset = 0;
  do
  {
    // get FAT
    if (fatSect != (workingCluster >> 7))
    {
      fatSect = workingCluster >> 7;
      fatBlockRead(fs, fs->fatBegin + fatSect, 1, buf);
    }
    offset = workingCluster & 0x7F;

    u32int sectorNr = (fatSect << 7) + offset;
    u32int* entry = (u32int*)(&buf[offset*4]);

    printf("fatDeleteFile: offs @ FAT %x for sector %08x: %08x\n", offset, sectorNr, *entry);

    workingCluster = *entry;

    *entry = 0x0;

    // update FAT table block
    fatBlockWrite(fs, fs->fatBegin + fatSect, 1, buf);
  }
  while (!FAT_EOC_MARKER(workingCluster));

  return 0;
} // end fuction
