#ifndef __MEMORY_MANAGER__PAGE_TABLE_H__
#define __MEMORY_MANAGER__PAGE_TABLE_H__

#include "common/types.h"

#include "drivers/beagle/memoryMap.h"


#define PT1_ENTRIES        4096
#define PT2_ENTRIES         256
#define PT_ENTRY_SIZE        4
#define PT1_SIZE (PT1_ENTRIES * PT_ENTRY_SIZE)
#define PT2_SIZE (PT2_ENTRIES * PT_ENTRY_SIZE)

/* Access Control */
#define HYPERVISOR_ACCESS_DOMAIN  15
#define HYPERVISOR_ACCESS_BITS    PRIV_RW_USR_NO //R/W priviledged, No Access USR (ARM ARM B3-28)
#define GUEST_ACCESS_DOMAIN       1
#define GUEST_ACCESS_BITS         PRIV_RW_USR_RW //Priv R/W, USR R/W

#define PT1_ALIGN_MASK     0xFFFF8000
#define PT1_ALIGN_BITS     15
#define PT2_ALIGN_MASK     0xFFFFFC00
#define PT2_ALIGN_BITS     10
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
  u32int type:2; //1-0
  u32int:3; // 4-2 Ignored
  u32int domain:4; // 8-5 domain
  u32int imp:1; // 9 imp use to indicate memory protection is active
  u32int:8; // 17-10 Ignored
  u32int superSection:1; //18
  u32int:12; //31-19 Ignored
};
typedef struct simpleDescriptor simpleEntry;

struct pageTableDescriptor
{
  u16int type:2; //1-0
  u16int sbz2:1; //2 b bit
  u16int ns:1; //3
  u16int sbz:1;  //4 xn (execute never)
  u16int domain:4; //8-5
  u16int imp:1; //9 implementation defined
  u32int addr:22; //31-10
};
typedef struct pageTableDescriptor pageTableEntry;

struct sectionDescriptor
{
  u16int type:2; //1-0
  u16int b:1; //2
  u16int c:1; //3
  u16int xn:1; //4
  u16int domain:4; //8-5
  u16int imp:1; //9 implementation defined
  u16int ap10:2; //11-10 bits 1:0 of the access permission field
  u16int tex:3; //14-12
  u16int ap2:1; //15 - bit 2 of the access permission field
  u16int s:1; //16
  u16int nG:1; //17
  u16int superSection:1; //18 - set to 0 for a section, 1 for a supersection
  u16int ns:1; //19
  u16int addr:12; //31-20
};
typedef struct sectionDescriptor sectionEntry;

struct largePageDescriptor
{
  u16int type:2; //1-1 0b01 for large pT
  u16int b:1; //2
  u16int c:1; //3
  u16int ap10:2; //5-4 AP[1:0]
  u16int sbz:3; //8-6
  u16int ap2:1; //9 AP[2]
  u16int s:1; //10
  u16int nG:1; //11
  u16int tex:3; //14-12 tex[2:0]
  u16int xn:1; //15
  u16int addr:16; //31-16
};
typedef struct largePageDescriptor largePageEntry;

struct smallPageDescriptor
{
  u16int xn:1; //0
  u16int type:1; //1 set to 1 for small pT
  u16int b:1; //2
  u16int c:1; //3
  u16int ap10:2; //5-4 AP[1:0]
  u16int tex:3; //8-6 tex[2:0]
  u16int ap2:1; //9 AP[2]
  u16int s:1; //10
  u16int nG:1; //11
  u32int addr:20; //31-12
};
typedef struct smallPageDescriptor smallPageEntry;


struct PageTableMetaData
{
  pageTableEntry* firstLevelEntry;
  u32int virtAddr;
  u32int physAddr;
  u32int mappedMegabyte;
  bool host;
  struct PageTableMetaData* nextEntry;
};
typedef struct PageTableMetaData ptInfo;


/*************** rewritten functions *********************/
u32int* newLevelOnePageTable(void);
u32int* newLevelTwoPageTable(void);
void deleteLevelTwoPageTable(pageTableEntry* pageTable);

void mapHypervisorMemory(simpleEntry* ptd);
void mapSection(simpleEntry* pageTable, u32int virtAddr, u32int physical,
                u8int domain, u8int accessBits, bool c, bool b, u8int tex);
void mapSmallPage(simpleEntry* pageTable, u32int virtAddr, u32int physical,
                u8int domain, u8int accessBits, u8int c, u8int b, u8int tex, u8int xn);

void addSectionEntry(sectionEntry* sectionEntryPtr, u32int physAddr,
        u8int domain, u8int accessBits, bool cacheable, bool bufferable, u8int tex);
void addSmallPageEntry(smallPageEntry* smallPageEntryPtr, u32int physical,
        u8int accessBits, u8int cacheable, u8int bufferable, u8int tex, u8int xn);
void addPageTableEntry(pageTableEntry* pageTableEntryPtr, u32int physical, u8int domain);

u32int getPhysicalAddress(simpleEntry* pageTable, u32int virtAddr);
simpleEntry* getEntryFirst(simpleEntry* pageTable, u32int virtAddr);
simpleEntry* getEntrySecond(pageTableEntry* firstLevelEntry, u32int virtAddr);

void splitSectionToSmallPages(simpleEntry* pageTable, u32int virtAddr);

bool isAddrInPageTable(simpleEntry* pageTablePhys, u32int physAddr);

void pageTableEdit(u32int address, u32int newVal);

void addPageTableInfo(pageTableEntry* entry, u32int virtual, u32int physical, u32int mapped, bool host);
ptInfo* getPageTableInfo(pageTableEntry* firstLevelEntry);
void removePageTableInfo(pageTableEntry* firstLevelEntry, bool host);
void dumpPageTableInfo(void);
void invalidatePageTableInfo(void);

void editAttributesSection(sectionEntry* oldSection, sectionEntry* newSection, simpleEntry* shadow, u32int virtual);
void editAttributesPageTable(pageTableEntry* oldTable, pageTableEntry* newTable, pageTableEntry* shadowTable, u32int virtual);
void editAttributesSmallPage(smallPageEntry* oldPage, smallPageEntry* newPage, smallPageEntry* shadowPage, u32int virtual);

#endif
