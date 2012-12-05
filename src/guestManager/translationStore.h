#ifndef __GUEST_MANAGER__TRANSLATION_STORE_H__
#define __GUEST_MANAGER__TRANSLATION_STORE_H__

#include "guestManager/basicBlockStore.h"

#include "common/stddef.h"
#include "common/types.h"


#define INSTR_SWI            0xEF000000U
#define INSTR_SWI_THUMB      0x0000DF00U
#define INSTR_NOP_THUMB      0x0000BF00U
#define INSTR_SWI_THUMB_MASK 0x0000FF00U
#define INSTR_SWI_THUMB_MIX  ((INSTR_SWI_THUMB << 16) | INSTR_NOP_THUMB)



typedef struct TranslationStore
{
  u32int* codeStore;
  u32int* codeStoreFreePtr;
  BasicBlock* basicBlockStore;
  u32int spillLocation;
  bool write;
} TranslationStore;


void initialiseTranslationStore(TranslationStore* ts);

void instructionToCodeStore(TranslationStore* ts, u32int instruction);

void clearTranslationsAll(TranslationStore* ts);
void clearTranslationsByAddress(TranslationStore* ts, u32int address);
void clearTranslationsByAddressRange(TranslationStore* ts, u32int addressStart, u32int addressEnd);


#endif
