#ifndef __MEMORY_MANAGER__SHADOW_MAP_H__
#define __MEMORY_MANAGER__SHADOW_MAP_H__

#include "common/types.h"

#include "memoryManager/pageTable.h"


bool shadowMap(GCONTXT *context, u32int virtAddr);

void shadowMapSection(GCONTXT *context, sectionEntry* guest, sectionEntry* shadow, u32int virtual);
void shadowUnmapSection(GCONTXT *context, simpleEntry* shadow, sectionEntry* guest, u32int virtual);

void shadowMapPageTable(GCONTXT *context, pageTableEntry* guest, pageTableEntry* shadow);
void shadowUnmapPageTable(GCONTXT *context, pageTableEntry* shadow, pageTableEntry* guest, u32int virtual);

void shadowMapSmallPage(GCONTXT *context, smallPageEntry* guest, smallPageEntry* shadow, u32int dom);
void shadowUnmapSmallPage(GCONTXT *context, smallPageEntry* shadow, smallPageEntry* guest, u32int virtual);

u32int mapAccessPermissionBits(GCONTXT *context, u32int guestAP, u32int domain);
void mapAPBitsSection(GCONTXT *context, sectionEntry* guest, simpleEntry* shadow, u32int virtual);
void mapAPBitsPageTable(GCONTXT *context, pageTableEntry* guest, pageTableEntry* shadow);
void mapAPBitsSmallPage(GCONTXT *context, u32int dom, smallPageEntry* guest, smallPageEntry* shadow);

#endif /* __MEMORY_MANAGER__SHADOW_MAP_H__ */
