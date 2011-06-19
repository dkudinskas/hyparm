#ifndef __PARTITIONS_H__
#define __PARTITIONS_H__

#include "common/types.h"
#include "io/block.h"

// uncomment me to enable partition debugging: #define PART_DEBUG

#define PART_TBL_OFFSET 0x1BE
#define PART_ENTRY_SIZE 16

struct Partition
{
  u8int status;
  u8int chsBegin[3];      // cylinder-head-sector where partition starts; ignore this, use lba
  u8int type;             // partition type (FAT, NTFS, EXT2, ..)
  u8int chsEnd[3];        // cylinder-head-sector where partition ends; ignore
  u32int lbaBegin;        // logical block address where paritition starts;
  u32int numberOfSectors; // number of sectors in partition;
};

struct PartitionTable
{
  struct Partition partitions[4];
};

typedef struct PartitionTable partitionTable;

int partTableRead(blockDevice *dev, partitionTable *tbl);

void fillPartitionEntries(char *record, struct Partition *part); 

#endif
