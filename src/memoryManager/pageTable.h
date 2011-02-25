#ifndef __PAGE_TABLE_H__
#define __PAGE_TABLE_H__

#include "types.h"

#define PAGE_TABLE_ENTRIES 4096
#define SECOND_LEVEL_PAGE_TABLE_ENTRIES 256
#define PAGE_TABLE_ENTRY_WIDTH 4
#define PAGE_TABLE_SIZE (PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRY_WIDTH)
#define SECOND_LEVEL_PAGE_TABLE_SIZE (SECOND_LEVEL_PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRY_WIDTH)

/* Access Control */
#define HYPERVISOR_ACCESS_DOMAIN  15
#define HYPERVISOR_ACCESS_BITS    PRIV_RW_USR_NO //R/W priviledged, No Access USR (ARM ARM B3-28)
#define GUEST_ACCESS_DOMAIN       1
#define GUEST_ACCESS_BITS         PRIV_RW_USR_RW //Priv R/W, USR R/W

//All values in bytes
#define CHUNKS_FOR_PAGE_TABLE ((PAGE_TABLE_ENTRIES * PAGE_TABLE_ENTRY_WIDTH)/FRAME_TABLE_CHUNK_SIZE)
#define SECTION_SIZE    0x100000
#define LARGE_PAGE_SIZE 0x10000
#define SMALL_PAGE_SIZE 0x1000


#define PAGE_64KB   1
#define PAGE_4KB    2
#define PAGE_1KB    3

struct pTSimpleDescriptor;
typedef struct pTSimpleDescriptor descriptor;

struct pTDescriptor;
typedef struct pTDescriptor pageTableDescriptor;

struct pTSectionDescriptor;
typedef struct pTSectionDescriptor sectionDescriptor;
/*
struct pTSuperSectionDescriptor;
typedef struct pTSuperSectionDescriptor superDescriptor;
*/
struct pTLargeDescriptor;
typedef struct pTLargeDescriptor largeDescriptor;

struct pTSmallDescriptor;
typedef struct pTSmallDescriptor smallDescriptor;

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
typedef enum memAccessCtrl ACCESS_TYPE;

descriptor* createNewPageTable(void);
descriptor* createHypervisorPageTable(void);
descriptor* createGuestOSPageTable(void);
void copyPageTable(descriptor* guest, descriptor* shadow);

ACCESS_TYPE setAccessBits(descriptor* ptd, u32int virtual, ACCESS_TYPE newAccessBits);


void pageTableEdit(u32int address, u32int newVal);

descriptor* getPageTableEntry(descriptor* ptd, u32int address);

u32int getPageEndAddr(descriptor* ptd, u32int address);
u32int getPhysicalAddress(descriptor* ptd, u32int virtualAddress);

bool isAddrInGuestPT(u32int vaddr);

void dumpPageTable(descriptor* ptd);

u32int findVAforPA(u32int physAddr);
u32int findGuestVAforPA(u32int physAddr);


enum enum_pageType
{
  FAULT = 0,
  PAGE_TABLE = 1,
  SECTION = 2,
  RESERVED = 3,
  LARGE_PAGE = 1,
  SMALL_PAGE = 2,
  SMALL_PAGE_3 = 3//Two types of small page, easier to use if(! FAULT || LARGE_PAGE)?
};
typedef enum enum_pageType pageType;

u32int addSectionPtEntry(descriptor* ptd, u32int virtual, u32int physical, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex);
u32int addSmallPtEntry(descriptor* ptd, u32int virtual, u32int physical, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex);
u32int addLargePtEntry(descriptor* ptd, u32int virtual, u32int physical, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex);
descriptor* get1stLevelPtDescriptorAddr(descriptor* ptd, u32int virtual);
descriptor* get2ndLevelPtDescriptor(pageTableDescriptor* ptd1st, u32int virtual);

descriptor* createNew1stLevelPageTable(u8int domain);
void mapHypervisorMemory(descriptor* ptd);
void largeMapMemory(descriptor* ptd, u32int startAddr, u32int endAddr, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex);
void sectionMapMemory(descriptor* ptd, u32int startAddr, u32int endAddr, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex);
void smallMapMemory(descriptor* ptd, u32int startAddr, u32int endAddr, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex);

void addNewPageTableDescriptor(descriptor* ptd1st, u32int physical, u8int domain);
void addNewSectionDescriptor(sectionDescriptor* sd, u32int physical, u8int domain, u8int accessBits, u8int c, u8int b, u8int tex);
void addNewSmallDescriptor(smallDescriptor* sd, u32int physical, u8int accessBits, u8int c, u8int b, u8int tex);
void addNewLargeDescriptor(largeDescriptor* sd, u32int physical, u8int accessBits, u8int c, u8int b, u8int tex);


void replicateLargeEntry(largeDescriptor* ld);
void copySectionEntry(sectionDescriptor* guest, sectionDescriptor* shadow);
void copySuperSectionEntry(sectionDescriptor* guest, sectionDescriptor* shadow);
void copyPageTableEntry(pageTableDescriptor* guest, pageTableDescriptor* shadow);
void copyLargeEntry(largeDescriptor* guest, largeDescriptor* shadow);
void copySmallEntry(smallDescriptor* guest, smallDescriptor* shadow);

void removePageTableEntry(pageTableDescriptor* shadow);
void removeSectionEntry(sectionDescriptor* shadow);
void removeLargePageEntry(largeDescriptor* shadow);
void removeSmallPageEntry(smallDescriptor* shadow);
void removePT2Metadata(void);

void invalidateSPT1(descriptor* spt);

void splitSectionToSmallPages(descriptor* ptd, u32int vAddr);

u8int mapGuestDomain(u8int guestDomain);
u32int mapAccessPermissionBits(u32int guestAP, u32int domain);
void mapAPBitsSection(u32int vAddr, sectionDescriptor* guestNewSD, descriptor* shadowSD);

void dumpPageTable(descriptor* ptd);
void dumpSection(sectionDescriptor* sd);
void dumpSuperSection(void* sd);
void dump2ndLevel(pageTableDescriptor* ptd, u32int virtual);
void dumpVirtAddr(u32int i);
void dump2ndVirtAddr(u32int virtual, u32int i, u32int pageSize);
void dumpLargePage(largeDescriptor* ld);
void dumpSmallPage(smallDescriptor* sd);


/* 1st level page table descriptor formats */
/*
4KB or 64KB page have a second level table
|31   10| 9 |8    5| 4 |3 | 2 |1  0|
| PTBA  |IMP|DOMAIN|SBZ|NS|SBZ|TYPE|

TYPE is 00 for invalid, 01 for pagetable descriptor, 10 for sections, 11 for reserved

*/

/* 1st Level Descriptors */
// Simple pt descriptor, we only care about the bottom two "type bits"
// and if a section (type=0b01) bit 18
struct pTSimpleDescriptor
{
  u32int type:2; //1-0
  u32int:7; // 8-2 Ignored
  u32int imp:1; // 9 imp use to indicate memory protection is active
  u32int:8; // 17-10 Ignored
  u32int sectiontype:1; //18
  u32int:12; //31-19 Ignored
};

//large/small page descriptor type 0b01
struct pTDescriptor
{
  u16int type:2; //1-0
  u16int sbz2:1; //2 b bit
  u16int ns:1; //3
  u16int sbz:1;  //4 xn (execute never)
  u16int domain:4; //8-5
  u16int imp:1; //9 implementation defined
  u32int addr:22; //31-10
};

//Section type 0b10
struct pTSectionDescriptor
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
  u16int sectionType:1; //18 - set to 0 for a section, 1 for a supersection
  u16int ns:1; //19
  u16int addr:12; //31-20
};

/*
*Wont pack to a 32bit word!
//SuperSection type 0b10
struct pTSuperSectionDescriptor
{
  u16int type:2; //1-0
  u16int b:1; //2
  u16int c:1; //3
  u16int xn:1; //4
  u16int extendedBaseAddr2:4; //8-5 PA[39:36] of the extended base addr
  u16int imp:1; //9 implementation defined
  u16int ap10:2; //11-10 bits 1:0 of the access permission field
  u16int tex:3; //14-12
  u16int ap2:1; //15 bit 2 of the access permission field
  u16int s:1; //16
  u16int nG:1; //17
  u16int sectionType:1; //18 set to 1 for a supersection
  u16int ns:1;//19
  u16int extendedBaseAddr:4; //23-20 PA[35:32]
  u16int supersectionBaseAddr:12; //31-24 PA[31:24]
};
*/

/* End 1st Level Descriptors */

/* 2nd Level Descriptors */

struct pTLargeDescriptor
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

struct pTSmallDescriptor
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
/* End 2nd Level Descriptors */

struct pageTableMetaDataElement
{
  u32int valid;
  u32int pAddr;
  u32int vAddr;
};

typedef struct pageTableMetaDataElement ptMetaData;

#endif