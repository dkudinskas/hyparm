#ifndef __GUEST_MANAGER__BASIC_BLOCK_STORE_H__
#define __GUEST_MANAGER__BASIC_BLOCK_STORE_H__

#include "common/stddef.h"
#include "common/types.h"

#include "instructionEmu/decoder.h"

#include "guestManager/types.h"


#define BASIC_BLOCK_STORE_SIZE      0x10000


struct TranslationStore;

enum basicBlockEntryType
{
  BB_TYPE_INVALID = 0,
  BB_TYPE_ARM,
  BB_TYPE_THUMB,
  GB_TYPE_ARM,
  GB_TYPE_THHUMB
};
typedef enum basicBlockEntryType basicBlockType;

struct BasicBlockEntry
{
  basicBlockType type;
  u32int* guestStart;
  u32int* guestEnd;
  u32int* codeStoreStart;
  u32int codeStoreSize;
  InstructionHandler handler;
  u32int versionNumber;
  u8int hotness;
  bool oneHypercall;
};
typedef struct BasicBlockEntry BasicBlock;


u32int getBasicBlockStoreIndex(u32int startAddress);
BasicBlock* getBasicBlockStoreEntry(struct TranslationStore* ts, u32int index);

void addInstructionToBlock(struct TranslationStore* ts, BasicBlock* basicBlock, u32int instruction);

void invalidateBlock(BasicBlock* block);

void setExecBitmap(GCONTXT* context, u32int start, u32int end);
bool isExecBitSet(GCONTXT* context, u32int addr);

void dumpBlockStoreStats(GCONTXT* context);

#endif
