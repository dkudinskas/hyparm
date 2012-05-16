#ifndef __INSTRUCTION_EMU__TRANSLATION_INFO_H__
#define __INSTRUCTION_EMU__TRANSLATION_INFO_H__

#include "guestManager/translationCache.h"


typedef struct armTranslationInfo
{
  MetaCacheEntry metaEntry;
  u16int pcRemapBitmapShift;
  u32int *code;
} ARMTranslationInfo;

#endif /* __INSTRUCTION_EMU__TRANSLATION_INFO_H__ */
