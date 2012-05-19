#ifndef __MEMORY_MANAGER__SHADOW_MAP_H__
#define __MEMORY_MANAGER__SHADOW_MAP_H__

#include "common/types.h"

#include "memoryManager/pageTable.h"


bool shadowMap(u32int virtAddr);

void shadowMapSection(sectionEntry* guest, sectionEntry* shadow, u32int virtual);
void shadowUnmapSection(simpleEntry* shadow, sectionEntry* guest, u32int virtual);

void shadowMapPageTable(pageTableEntry* guest, pageTableEntry* shadow);
void shadowUnmapPageTable(pageTableEntry* shadow, pageTableEntry* guest, u32int virtual);

void shadowMapSmallPage(smallPageEntry* guest, smallPageEntry* shadow, u32int dom);
void shadowUnmapSmallPage(smallPageEntry* shadow, smallPageEntry* guest, u32int virtual);

u32int mapAccessPermissionBits(u32int guestAP, u32int domain);
void mapAPBitsSection(sectionEntry* guest, simpleEntry* shadow, u32int virtual);
void mapAPBitsPageTable(pageTableEntry* guest, pageTableEntry* shadow);
void mapAPBitsSmallPage(u32int dom, smallPageEntry* guest, smallPageEntry* shadow);

u32int mapExecuteNeverBit(u32int guestDomain, u32int xn);

u8int mapGuestDomain(u8int guestDomain);

#endif /* __MEMORY_MANAGER__SHADOW_MAP_H__ */
