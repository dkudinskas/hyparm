#include "io/partitions.h"
#include "common/debug.h"
#include "common/alignFunctions.h"

/* Read the partition table from a block device */
int partTableRead(blockDevice *dev, partitionTable *table)
{
  // read mbr, check signature validity
  char mbr[512];
  if (dev->blockRead(dev->devID, 0, 1, mbr) != 1)
  {
    printf("partTableRead: Could not read the MBR block\n");
    return -1;
  }

  if (mbr[510] != 0x55 || mbr[511] != 0xAA)
  {
    printf("partTableRead: invalid mbr signature\n");
    return -1;
  }

  // read partitions
  fillPartitionEntries(mbr + PART_TBL_OFFSET, table->partitions);
  fillPartitionEntries(mbr + PART_TBL_OFFSET + PART_ENTRY_SIZE, table->partitions+1);
  fillPartitionEntries(mbr + PART_TBL_OFFSET + 2*PART_ENTRY_SIZE, table->partitions+2);
  fillPartitionEntries(mbr + PART_TBL_OFFSET + 3*PART_ENTRY_SIZE, table->partitions+3);

  return 0;
}

void fillPartitionEntries(char *record, struct Partition *part)
{
  part->status = record[0];
  part->chsBegin[0] = record[1];
  part->chsBegin[1] = record[2];
  part->chsBegin[2] = record[3];
  part->type = record[4];
  part->chsEnd[0] = record[5];
  part->chsEnd[1] = record[6];
  part->chsEnd[2] = record[7];
  part->lbaBegin = uaLoadWord(record+8);
  part->numberOfSectors = uaLoadWord(record+12);
#ifdef PART_DEBUG
  printf("fillPartitionEntries: part->status %02x\n", part->status); 
  printf("fillPartitionEntries: part->chsStart[0] %02x\n", part->chsBegin[0]); 
  printf("fillPartitionEntries: part->chsStart[1] %02x\n", part->chsBegin[1]); 
  printf("fillPartitionEntries: part->chsStart[2] %02x\n", part->chsBegin[2]); 
  printf("fillPartitionEntries: part->type %02x\n", part->type); 
  printf("fillPartitionEntries: part->chsEnd[0] %02x\n", part->chsEnd[0]); 
  printf("fillPartitionEntries: part->chsEnd[1] %02x\n", part->chsEnd[1]); 
  printf("fillPartitionEntries: part->chsEnd[2] %02x\n", part->chsEnd[2]); 
  printf("fillPartitionEntries: part->lbaBegin %08x\n", part->lbaBegin); 
  printf("fillPartitionEntries: part->numberOfSectors %08x\n", part->numberOfSectors); 
#endif
}

