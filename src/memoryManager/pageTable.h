#ifndef __MEMORY_MANAGER__PAGE_TABLE_H__
#define __MEMORY_MANAGER__PAGE_TABLE_H__

#include "common/compiler.h"
#include "common/types.h"

#include "drivers/beagle/memoryMap.h"

#include "guestManager/types.h"


#define PT1_ENTRIES        4096
#define PT2_ENTRIES         256
#define PT_ENTRY_SIZE        4
#define PT1_SIZE (PT1_ENTRIES * PT_ENTRY_SIZE)
#define PT2_SIZE (PT2_ENTRIES * PT_ENTRY_SIZE)

/* Access Control */
#define HYPERVISOR_ACCESS_DOMAIN  15
#define HYPERVISOR_ACCESS_BITS    PRIV_RW_USR_NO //priv R/W , USR no access
#define GUEST_ACCESS_DOMAIN       1
#define GUEST_ACCESS_BITS         PRIV_RW_USR_RW //Priv R/W, USR R/W

#define PT1_ALIGN_MASK         0xFFFFC000
#define PT1_ALIGN_BITS         14
#define PT2_ALIGN_MASK         0xFFFFFC00
#define PT2_ALIGN_BITS         10
#define SUPER_SECTION_SIZE     0x01000000
#define SUPER_SECTION_MASK     0xFF000000
#define SECTION_SIZE           0x00100000
#define SECTION_MASK           0xFFF00000
#define LARGE_PAGE_SIZE        0x00010000
#define LARGE_PAGE_MASK        0xFFFF0000
#define SMALL_PAGE_SIZE        0x00001000
#define SMALL_PAGE_MASK        0xFFFFF000


enum memAccessCtrl
{
  PRIV_NO_USR_NO = 0b000,   //priv no access, usr no access
  PRIV_RW_USR_NO = 0b001,   //priv read/write, usr no access
  PRIV_RW_USR_RO = 0b010,   //priv read/write, usr read only
  PRIV_RW_USR_RW = 0b011,   //priv read/write, usr read/write
  AP_RESERVED = 0b100,      // is ignored
  PRIV_RO_USR_NO= 0b101,    //priv read only, usr no access
  DEPRECATED=0b110,         //priv read only, usr read only
  PRIV_RO_USR_RO= 0b111,    //priv read only, usr read only
};
typedef enum memAccessCtrl AccessType;

enum PageTableEntryType
{
  FAULT = 0,
  PAGE_TABLE = 1,
  SECTION = 2,
  RESERVED = 3,
  LARGE_PAGE = 1,
  SMALL_PAGE = 2,
  SMALL_PAGE_3 = 3 //Two types of small page
};
typedef enum PageTableEntryType pageType;


/***************************************************************
 ******************** Page Table entry formats *****************
 ***************************************************************/
struct simpleDescriptor
{
  unsigned type:2; //1-0
  unsigned:3; // 4-2 Ignored
  unsigned domain:4; // 8-5 domain
  unsigned imp:1; // 9 imp use to indicate memory protection is active
  unsigned:8; // 17-10 Ignored
  unsigned superSection:1; //18
  unsigned:13; //31-19 Ignored
};
typedef struct simpleDescriptor simpleEntry;

struct pageTableDescriptor
{
  unsigned type:2; //1-0
  unsigned sbz2:1; //2 b bit
  unsigned ns:1; //3
  unsigned sbz:1;  //4 xn (execute never)
  unsigned domain:4; //8-5
  unsigned imp:1; //9 implementation defined
  unsigned addr:22; //31-10
};
typedef struct pageTableDescriptor pageTableEntry;

struct sectionDescriptor
{
  unsigned type:2; //1-0
  unsigned b:1; //2
  unsigned c:1; //3
  unsigned xn:1; //4
  unsigned domain:4; //8-5
  unsigned imp:1; //9 implementation defined
  unsigned ap10:2; //11-10 bits 1:0 of the access permission field
  unsigned tex:3; //14-12
  unsigned ap2:1; //15 - bit 2 of the access permission field
  unsigned s:1; //16
  unsigned nG:1; //17
  unsigned superSection:1; //18 - set to 0 for a section, 1 for a supersection
  unsigned ns:1; //19
  unsigned addr:12; //31-20
};
typedef struct sectionDescriptor sectionEntry;

struct largePageDescriptor
{
  unsigned type:2; //1-1 0b01 for large pT
  unsigned b:1; //2
  unsigned c:1; //3
  unsigned ap10:2; //5-4 AP[1:0]
  unsigned sbz:3; //8-6
  unsigned ap2:1; //9 AP[2]
  unsigned s:1; //10
  unsigned nG:1; //11
  unsigned tex:3; //14-12 tex[2:0]
  unsigned xn:1; //15
  unsigned addr:16; //31-16
};
typedef struct largePageDescriptor largePageEntry;

struct smallPageDescriptor
{
  unsigned xn:1; //0
  unsigned type:1; //1 set to 1 for small pT
  unsigned b:1; //2
  unsigned c:1; //3
  unsigned ap10:2; //5-4 AP[1:0]
  unsigned tex:3; //8-6 tex[2:0]
  unsigned ap2:1; //9 AP[2]
  unsigned s:1; //10
  unsigned nG:1; //11
  unsigned addr:20; //31-12
};
typedef struct smallPageDescriptor smallPageEntry;


void dumpTranslationTable(simpleEntry *table) __cold__;


/*************** rewritten functions *********************/
simpleEntry *newLevelOnePageTable(void);
u32int* newLevelTwoPageTable(void);
void deleteLevelTwoPageTable(GCONTXT *context, pageTableEntry* pageTable);

void mapHypervisorMemory(simpleEntry* ptd);

u32int mapRegion(simpleEntry *pageTable, u32int virtualStartAddress, u32int physicalStartAddress,
                 u32int physicalEndAddress, u8int domain, u8int accessBits, bool cacheable,
                 bool bufferable, u8int regionAttributes, bool executeNever);

void mapSection(simpleEntry* pageTable, u32int virtAddr, u32int physical,
                u8int domain, u8int accessBits, bool c, bool b, u8int tex, bool executeNever);
void mapSmallPage(simpleEntry* pageTable, u32int virtAddr, u32int physical,
                u8int domain, u8int accessBits, u8int c, u8int b, u8int tex, u8int xn);
void mapLargePage(simpleEntry *pageTable, u32int virtAddr, u32int physical, u8int domain,
                  u8int accessBits, bool c, bool b, u8int tex, bool executeNever);
void addSectionEntry(sectionEntry *sectionEntryPtr, u32int physAddr, u8int domain,
                     u8int accessBits, bool cacheable, bool bufferable, u8int tex, bool executeNever);
void addSmallPageEntry(smallPageEntry* smallPageEntryPtr, u32int physical,
        u8int accessBits, u8int cacheable, u8int bufferable, u8int tex, u8int xn);
void addLargePageEntry(largePageEntry *entry, u32int physical, u8int accessBits, bool cacheable,
                       bool bufferable, u8int tex, bool executeNever);
void addPageTableEntry(pageTableEntry* pageTableEntryPtr, u32int physical, u8int domain);

u32int getPhysicalAddress(GCONTXT *context, simpleEntry* pageTable, u32int virtAddr);
simpleEntry* getEntryFirst(simpleEntry* pageTable, u32int virtAddr);
simpleEntry* getEntrySecond(GCONTXT *context, pageTableEntry* firstLevelEntry, u32int virtAddr);
simpleEntry* getEntry(simpleEntry* pageTable, u32int virtAddr);

void splitSectionToSmallPages(simpleEntry* pageTable, u32int virtAddr);

bool isAddrInPageTable(GCONTXT *context, simpleEntry* pageTablePhys, u32int physAddr);

void pageTableEdit(GCONTXT *context, u32int address, u32int newVal);

void editAttributesSection(GCONTXT *context, sectionEntry* oldSection, sectionEntry* newSection, simpleEntry* shadow, u32int virtual);
void editAttributesPageTable(pageTableEntry* oldTable, pageTableEntry* newTable, pageTableEntry* shadowTable, u32int virtual);
void editAttributesSmallPage(smallPageEntry* oldPage, smallPageEntry* newPage, smallPageEntry* shadowPage, u32int virtual);

#endif
