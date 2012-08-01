#ifndef __GUEST_MANAGER__TRANSLATION_STORE_H__
#define __GUEST_MANAGER__TRANSLATION_STORE_H__

#include "guestManager/basicBlockStore.h"

#include "common/stddef.h"
#include "common/types.h"


typedef struct TranslationStore
{
  u32int* codeStore;
  u32int* codeStoreFreePtr;
  BasicBlock* basicBlockStore;
  u32int spillLocation;
} TranslationStore;


void initialiseTranslationStore(TranslationStore* ts);

void instructionToCodeStore(TranslationStore* ts, u32int instruction);

void clearTranslationsSmallPage(TranslationStore* ts, u32int addressStart, u32int addressEnd);
void clearTranslationsByAddress(TranslationStore* ts, u32int address);
void clearTranslationsByAddressRange(TranslationStore* ts, u32int addressStart, u32int addressEnd);


#endif
