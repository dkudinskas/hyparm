#ifndef __FAT_H__
#define __FAT_H__

#include "io/partitions.h"
#include "io/block.h"

#include "common/types.h"


// uncomment me to enable debug: #define FAT_DEBUG

#define CLUSTER_REL_LBA(fs, x) (fs->clusterBegin + (x - 2) * fs->sectorsPerCluster)
#define FAT_EOC_MARKER(x) ((x & 0x0FFFFFFF) >= 0x0FFFFFF8 && (x & 0x0FFFFFFF) <= 0x0FFFFFFF)

#define FAT32_DIR_ENTRY_LENGTH  32  // well, obviously

#define FAT_DE_UNUSED   0xE5
#define FAT_DE_DIR_MASK 0x10
#define FAT_DE_EOD      0x0
#define FAT_LF_MASK     0xF
#define FAT_EOC_VAL     0xFFFFFFFF

/* Representation of a "mounted" fat filesystem. */
typedef struct FAT
{
  /* Some of these are just checked once, can remove later*/
  u16int bytesPerSector;      // always 512
  u8int sectorsPerCluster; 
  u16int numReservedSectors;  // usually 0x20
  u8int numFats;              // always 2
  u32int sectorsPerFat;
  u32int rootDirFirstCluster; // usually 0x2
  bool mounted;
  blockDevice *blockDevice;
  struct Partition *part;     // partition in primary table where this fs is located
  u32int fatBegin;            // partition relative fat location
  u32int clusterBegin;        // partition relative cluster begin location
} fatfs;


typedef struct DirectoryEntry
{
  char filename[11];
  char attrib;
  u16int firstClusterHigh;
  u16int firstClusterLow;
  u32int firstCluster;
  u32int fileSize;
  u32int parentCluster;
  bool free;
  int isDirectory;
  bool valid;
  u32int position; // position in parent cluster - used during write
} dentry;


typedef struct FatFileHandle
{
  dentry *dirEntry;
  u32int lastClusterNr;
  u32int bytesInLastCluster;
  u8int *lastCluster;
} file;


int fatMount(fatfs *fs, blockDevice *dev, int part_num);

void tree(fatfs *fs, u32int currentCluster, u32int level);

void loadFatDirEntry(char *record, dentry *d);

void setFatDirEntry(fatfs *fs, dentry *d, u32int position);

/*** operations on files ***/
int fread(fatfs *fs, file *handle, void *out, u32int maxlen);
int fwrite(fatfs *fs, file *handle, void *src, u32int length);
int fdelete(fatfs *fs, file *handle);
void fclose(fatfs *fs, file *handle);
file* fopen(fatfs *fs, const char *fname);
file* fnew(fatfs *fs, const char *fname);

/*** READ / WRITE BLOCKS ***/
u32int fatBlockRead(fatfs *fs, u32int start, u64int blkCount, void *dst);
u32int fatBlockWrite(fatfs *fs, u32int start, u64int blkCount, const void *src);

u32int fatLoadClusFatSector(fatfs *fs, u32int clus, char *buf);

void fatSetClusterValue(fatfs *fs, u32int clus, u32int val);

u32int fatGetFreeClus(fatfs *fs);

u32int fatGetNextClus(fatfs *fs, u32int clus);

bool filenameMatch(const char *user, const char *fatname);

dentry *getPathDirEntry(fatfs *fs, const char *fname, int createNew);

#endif
