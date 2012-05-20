#include "common/bit.h"
#include "common/debug.h"
#include "common/stddef.h"
#include "common/string.h"

#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"


typedef struct superSectionDescriptor
{
  unsigned type : 2;
  unsigned bufferable : 1;
  unsigned cacheable : 1;
  unsigned executeNever : 1;
  unsigned physicalAddress_39_36 : 4;
  unsigned : 1;
  unsigned accessPermissions_1_0 : 2;
  unsigned regionAttributes : 3;
  unsigned accessPermissions_2 : 1;
  unsigned shareable : 1;
  unsigned nonGlobal : 1;
  unsigned superSection : 1;
  unsigned nonSecure : 1;
  unsigned physicalAddress_35_32 : 4;
  unsigned physicalAddress_31_24 : 8;
} superSectionEntry;

typedef union
{
  simpleEntry simple;
  sectionEntry section;
  superSectionEntry superSection;
  pageTableEntry pageTable;
  u32int value;
} l1Descriptor;

typedef union
{
  smallPageEntry smallPage;
  largePageEntry largePage;
  u32int value;
} l2Descriptor;

struct region
{
  u32int physicalStartAddress;
  u32int physicalEndAddress;
  u32int virtualStartAddress;
  u32int virtualEndAddress;
  AccessType accessPermissions;
  u8int domain;
  u8int regionAttributes;
  bool bufferable;
  bool cacheable;
  bool executeNever;
  bool nonGlobal;
  bool nonSecure;
  bool shareable;
  bool containsSuperSections;
  bool containsSections;
  bool containsLargePages;
  bool containsSmallPages;
};

struct state
{
  struct region currentRegion;
  l1Descriptor *l1Table;
  l1Descriptor *currentL1Entry;
  s8int l1SuperSectionEntries;
  l2Descriptor *l2Table;
  l2Descriptor *currentL2Entry;
  s8int l2LargePageEntries;
  u32int currentVirtualAddress;
};


static void dumpL1Table(l1Descriptor *l1Table);
static void dumpL2Table(struct state *s);
static void endRegion(struct state *s);
static void extendRegion(struct state *s, u32int physicalStartAddress, u32int size,
                         AccessType accessPermissions, u8int domain, u8int regionAttributes,
                         bool bufferable, bool cacheable, bool executeNever, bool nonGlobal,
                         bool nonSecure, bool shareable);


void dumpTranslationTable(simpleEntry *table)
{
  dumpL1Table((l1Descriptor *)table);
}

static void dumpL1Table(l1Descriptor *l1Table)
{
  printf("Dumping page table @ %p" EOL, l1Table);
  /*
   * Make sure we got an aligned and valid pointer...
   */
  if (l1Table == NULL)
  {
    return;
  }
  if (!isAlignedToMask(l1Table, PT1_ALIGN_MASK))
  {
    printf("Error: L1 page table base pointer unaligned; cannot dump!" EOL);
    return;
  }
  struct state s =
    {
      .l1Table = l1Table,
      .currentL1Entry = l1Table,
      .l1SuperSectionEntries = 0,
      .l2Table = NULL,
      .currentL2Entry = NULL
    };
  memset(&s.currentRegion, 0, sizeof(struct region));
  while (s.currentL1Entry < (s.l1Table + PT1_ENTRIES))
  {
    if (isAlignedToMask(s.currentL1Entry, SMALL_PAGE_MASK) && isMmuEnabled())
    {
      /*
       * An L1 page table is 16 kB, aligned on a 16 kB boundary, so it could be mapped in 4 small
       * pages. This entry may be on a new page; make sure this page is mapped and accessible!
       */
      PhysicalAddressRegister par = mmuTryTranslate((u32int)s.currentL1Entry,
                                                    TTP_PRIVILEGED_READ);
      if (par.fault.fault)
      {
        endRegion(&s);
        printf("Error: L1 descriptors @ %p--%#.8x not mapped" EOL, s.currentL1Entry,
               ((u32int)s.currentL1Entry) + SMALL_PAGE_SIZE);
        s.currentVirtualAddress += SMALL_PAGE_SIZE / PT_ENTRY_SIZE * SECTION_SIZE;
        s.currentL1Entry += SMALL_PAGE_SIZE / PT_ENTRY_SIZE;
        continue;
      }
    }

    bool isRepeated = FALSE;
    if (s.l1SuperSectionEntries > 0)
    {
      --s.l1SuperSectionEntries;
      l1Descriptor *base = (l1Descriptor *)((u32int)s.currentL1Entry & ~(16 * 4 - 1));
      if (s.currentL1Entry->simple.type == SECTION && s.currentL1Entry->section.superSection && base->value == s.currentL1Entry->value)
      {
        isRepeated = TRUE;
      }
      else
      {
        endRegion(&s);
        printf("Error: descriptor at @ %p not repeated from base descriptor of supersection @ %p"
               EOL, s.currentL1Entry, base);
      }
    }

    if (!isRepeated)
    {
      s.l1SuperSectionEntries = 0;
      if (s.currentL1Entry->simple.type == FAULT)
      {
        endRegion(&s);
      }
      else if (s.currentL1Entry->simple.type == PAGE_TABLE)
      {
        s.l2Table = s.currentL2Entry = (l2Descriptor *)(s.currentL1Entry->pageTable.addr << 10);
        dumpL2Table(&s);
        ++s.currentL1Entry;
        continue;
      }
      else if (s.currentL1Entry->simple.type == SECTION)
      {
        if (s.currentL1Entry->section.superSection == 0)
        {
          /*
           * Section
           */
          extendRegion(&s, s.currentL1Entry->section.addr << 20, SECTION_SIZE,
                       ((s.currentL1Entry->section.ap2 << 2) | (s.currentL1Entry->section.ap10)),
                       s.currentL1Entry->section.domain, s.currentL1Entry->section.tex,
                       s.currentL1Entry->section.b, s.currentL1Entry->section.c,
                       s.currentL1Entry->section.xn, s.currentL1Entry->section.nG,
                       s.currentL1Entry->section.ns, s.currentL1Entry->section.s);
        }
        else
        {
          /*
           * Supersection
           */
          l1Descriptor *base = (l1Descriptor *)((u32int)s.currentL1Entry & ~(16 * 4 - 1));
          if (s.currentL1Entry != base)
          {
            endRegion(&s);
            printf("Error: first supersection descriptor unaligned @ %p" EOL, s.currentL1Entry);
          }
          if (s.currentL1Entry->superSection.physicalAddress_39_36 != 0
              || s.currentL1Entry->superSection.physicalAddress_35_32 != 0)
          {
            endRegion(&s);
            printf("Error: found supersection descriptor with extended physical address @ %p" EOL,
                   s.currentL1Entry);
          }
          else
          {
            /*
             * Supersection always has domain = 0.
             */
            extendRegion(&s, s.currentL1Entry->superSection.physicalAddress_31_24 << 24,
                         SUPER_SECTION_SIZE, ((s.currentL1Entry->superSection.accessPermissions_2 << 2)
                                              | (s.currentL1Entry->superSection.accessPermissions_1_0)),
                         0, s.currentL1Entry->superSection.regionAttributes,
                         s.currentL1Entry->superSection.bufferable,
                         s.currentL1Entry->superSection.cacheable,
                         s.currentL1Entry->superSection.executeNever,
                         s.currentL1Entry->superSection.nonGlobal,
                         s.currentL1Entry->superSection.nonSecure,
                         s.currentL1Entry->superSection.shareable);
            if (s.currentL1Entry == base)
            {
              s.l1SuperSectionEntries = 15;
            }
            else
            {
              endRegion(&s);
            }
          }
        }
      }
      else
      {
        endRegion(&s);
        printf("Error: invalid L1 descriptor @ %p: %#.8x" EOL, s.currentL1Entry, s.currentL1Entry->value);
      }
    }

    ++s.currentL1Entry;
    s.currentVirtualAddress += SECTION_SIZE;
  }
  endRegion(&s);
}

static void dumpL2Table(struct state *s)
{
  /*
   * L2 table size always fits in ONE small page.
   */
  if (isMmuEnabled())
  {
    PhysicalAddressRegister par = mmuTryTranslate((u32int)s->l2Table, TTP_PRIVILEGED_READ);
    if (par.fault.fault)
    {
      endRegion(s);
      printf("Error: L2 table @ %p--%#.8x not mapped" EOL, s->l2Table, ((u32int)s->l2Table) + PT2_SIZE);
      s->currentVirtualAddress += SECTION_SIZE;
      return;
    }
  }
  /*
   * Now we should be able to read it. Dump...
   */
  while (s->currentL2Entry < (s->l2Table + PT2_ENTRIES))
  {
    bool isRepeated = FALSE;
    if (s->l2LargePageEntries > 0)
    {
      s->l2LargePageEntries--;
      l2Descriptor *base = (l2Descriptor *)((u32int)s->currentL2Entry & ~(16 * 4 - 1));
      if (s->currentL2Entry->largePage.type == LARGE_PAGE
          && base->value == s->currentL2Entry->value)
      {
        isRepeated = TRUE;
      }
      else
      {
        endRegion(s);
        printf("Error: descriptor at @ %p not repeated from base descriptor of large page @ %p"
               EOL, s->currentL2Entry, base);
      }
    }

    if (!isRepeated)
    {
      s->l2LargePageEntries = 0;
      if (s->currentL2Entry->largePage.type == FAULT)
      {
        endRegion(s);
      }
      else if ((s->currentL2Entry->largePage.type & SMALL_PAGE))
      {
        extendRegion(s, s->currentL2Entry->smallPage.addr << 12, SMALL_PAGE_SIZE,
                     ((s->currentL2Entry->smallPage.ap2 << 2) | s->currentL2Entry->smallPage.ap10),
                     s->currentL1Entry->pageTable.domain, s->currentL2Entry->smallPage.tex,
                     s->currentL2Entry->smallPage.b, s->currentL2Entry->smallPage.c,
                     s->currentL2Entry->smallPage.xn, s->currentL2Entry->smallPage.nG,
                     s->currentL1Entry->pageTable.ns, s->currentL2Entry->smallPage.s);
      }
      else if (s->currentL2Entry->largePage.type == LARGE_PAGE)
      {
        l2Descriptor *base = (l2Descriptor *)((u32int)s->currentL2Entry & ~(16 * 4 - 1));
        if (s->currentL2Entry != base)
        {
          endRegion(s);
          printf("Error: first large page descriptor unaligned @ %p" EOL, s->currentL2Entry);
        }
        /*
         * Supersection always has domain = 0.
         */
        extendRegion(s, s->currentL2Entry->largePage.addr << 16, LARGE_PAGE_SIZE,
                     ((s->currentL2Entry->largePage.ap2  << 2) | (s->currentL2Entry->largePage.ap10)),
                     s->currentL1Entry->pageTable.domain, s->currentL2Entry->largePage.tex,
                     s->currentL2Entry->largePage.b, s->currentL2Entry->largePage.c,
                     s->currentL2Entry->largePage.xn, s->currentL2Entry->largePage.nG,
                     s->currentL1Entry->pageTable.ns, s->currentL2Entry->largePage.s);
        if (s->currentL2Entry == base)
        {
          s->l2LargePageEntries = 15;
        }
        else
        {
          endRegion(s);
        }
      }
      else
      {
        endRegion(s);
        printf("Error: invalid L2 descriptor @ %p: %#.8x" EOL, s->currentL2Entry, s->currentL2Entry->value);
      }
    }

    s->currentL2Entry++;
    s->currentVirtualAddress += SMALL_PAGE_SIZE;
  }
}

static void endRegion(struct state *s)
{
  /*
   * Do not print blank entries.
   */
  if (s->currentRegion.physicalEndAddress - s->currentRegion.physicalStartAddress == 0)
  {
    return;
  }
  const char *accessPermissionsString;
  switch (s->currentRegion.accessPermissions)
  {
    case PRIV_NO_USR_NO:
    {
      accessPermissionsString = "----";
      break;
    }
    case PRIV_RW_USR_NO:
    {
      accessPermissionsString = "rw--";
      break;
    }
    case PRIV_RW_USR_RO:
    {
      accessPermissionsString = "rwr-";
      break;
    }
    case PRIV_RW_USR_RW:
    {
      accessPermissionsString = "rwrw";
      break;
    }
    case PRIV_RO_USR_NO:
    {
      accessPermissionsString = "r---";
      break;
    }
    case PRIV_RO_USR_RO:
    {
      accessPermissionsString = "r-r-";
      break;
    }
    default:
    {
      accessPermissionsString = "????";
      break;
    }
  }
  printf("%.8x-%.8x -> %.8x-%.8x D%x %s%s %s%s%s%s%s%s%s%s %s%s%s%s" EOL,
      s->currentRegion.physicalStartAddress, s->currentRegion.physicalEndAddress,
      s->currentRegion.virtualStartAddress, s->currentRegion.virtualEndAddress,
      s->currentRegion.domain,
      accessPermissionsString, (s->currentRegion.executeNever ? "-" : "x"),
      (s->currentRegion.regionAttributes & 0b100 ? "1" : "0"),
      (s->currentRegion.regionAttributes & 0b010 ? "1" : "0"),
      (s->currentRegion.regionAttributes & 0b001 ? "1" : "0"),
      (s->currentRegion.cacheable ? "C" : "-"), (s->currentRegion.bufferable ? "B" : "-"),
      (s->currentRegion.shareable ? "S" : "-"), (s->currentRegion.nonGlobal ? "-" : "G"),
      (s->currentRegion.nonSecure ? "-" : "Z"),
      (s->currentRegion.containsSuperSections ? "!" : "-"),
      (s->currentRegion.containsSections ? "$" : "-"),
      (s->currentRegion.containsLargePages ? "P" : "-"),
      (s->currentRegion.containsSmallPages ? "p" : "-"));
  /*
   * Blank for next round...
   */
  memset(&s->currentRegion, 0, sizeof(struct region));
}

static void extendRegion(struct state *s, u32int physicalStartAddress, u32int size,
                         AccessType accessPermissions, u8int domain, u8int regionAttributes,
                         bool bufferable, bool cacheable, bool executeNever, bool nonGlobal,
                         bool nonSecure, bool shareable)
{
  if (physicalStartAddress != s->currentRegion.physicalEndAddress
      || s->currentVirtualAddress != s->currentRegion.virtualEndAddress
      || accessPermissions != s->currentRegion.accessPermissions
      || domain != s->currentRegion.domain || regionAttributes != s->currentRegion.regionAttributes
      || bufferable != s->currentRegion.bufferable || cacheable != s->currentRegion.cacheable
      || executeNever != s->currentRegion.executeNever || nonGlobal != s->currentRegion.nonGlobal
      || nonSecure != s->currentRegion.nonSecure || shareable != s->currentRegion.shareable)
  {
    /*
     * Have to start a new region!
     */
    endRegion(s);
    s->currentRegion = (struct region)
      {
        .physicalStartAddress = physicalStartAddress,
        .physicalEndAddress = physicalStartAddress + size,
        .virtualStartAddress = s->currentVirtualAddress,
        .virtualEndAddress = s->currentVirtualAddress + size,
        .accessPermissions = accessPermissions,
        .domain = domain,
        .regionAttributes = regionAttributes,
        .bufferable = bufferable,
        .cacheable = cacheable,
        .executeNever = executeNever,
        .nonGlobal = nonGlobal,
        .nonSecure = nonSecure,
        .shareable = shareable
      };
  }
  else
  {
    /*
     * Linear extension, same attributes.
     */
    s->currentRegion.physicalEndAddress += size;
    s->currentRegion.virtualEndAddress += size;
  }
  /*
   * Update contains* fields
   */
  switch (size)
  {
    case SUPER_SECTION_SIZE:
    {
      s->currentRegion.containsSuperSections = TRUE;
      break;
    }
    case SECTION_SIZE:
    {
      s->currentRegion.containsSections = TRUE;
      break;
    }
    case LARGE_PAGE_SIZE:
    {
      s->currentRegion.containsLargePages = TRUE;
      break;
    }
    case SMALL_PAGE_SIZE:
    {
      s->currentRegion.containsSmallPages = TRUE;
      break;
    }
  }
}



#if 0

  /*
   * We're debugging PT problems here, so be very careful.
   */
  u32int blockStart = (u32int)l1Table;
  u32int blockEnd;
  u32int pageTableEnd = (u32int)l1Table + PT1_SIZE;
  u32int virtualAddress = 0;
  while (blockStart < pageTableEnd)
  {
    blockEnd = blockStart + SMALL_PAGE_SIZE;
    /*
     * If the MMU is enabled, make sure this block of the PT is mapped.
     */
    if (isMmuEnabled())
    {
      PhysicalAddressRegister par = mmuTryTranslate(blockStart, TTP_PRIVILEGED_READ);
      if (par.fault.fault)
      {
        printf("Error: L1 descriptors @ %#.8x--%#.8x not mapped" EOL, blockStart,
               blockStart + SMALL_PAGE_SIZE);
        virtualAddress += SMALL_PAGE_SIZE / PT_ENTRY_SIZE * SECTION_SIZE;
        continue;
      }
    }
    /*
     * Dump the L1 descriptors in the current block.
     */
    for (; blockStart < blockEnd; blockStart += PT_ENTRY_SIZE, virtualAddress += SECTION_SIZE)
    {
      simpleEntry *entry = (simpleEntry *)blockStart;
      switch (entry->type)
      {
        case FAULT:
        {
          break;
        }
        case PAGE_TABLE:
        {
          pageTableEntry *pageTable = (pageTableEntry *)entry;
          largePageEntry *l2Table = (largePageEntry *)(pageTable->addr << 10);
          printf(" PT ------------------ ------------------ @ %p" EOL, l2Table);
          dumpL2PageTable(l2Table, virtualAddress);
          break;
        }
        case SECTION:
        {
          sectionEntry *section = (sectionEntry *)entry;
          u32int physicalAddress = section->addr << 20;
          if (section->superSection)
          {
            printf(" SS %.8x--%.8x %.8x--%.8x" EOL, physicalAddress,
                   physicalAddress + (SECTION_SIZE << 4), virtualAddress,
                   virtualAddress + (SECTION_SIZE << 4));
          }
          else
          {
            printf(" S  %.8x--%.8x %.8x--%.8x" EOL, physicalAddress,
                   physicalAddress + SECTION_SIZE, virtualAddress, virtualAddress + SECTION_SIZE);
          }
          break;
        }
        default:
        {
          printf(" ?? ------------------ ------------------ %.8x" EOL, *(u32int *)entry);
          break;
        }
      } // switch entry type
    } // for each entry in block
  } // while blocks in table
}

void dumpL2PageTable(largePageEntry *l2Table, u32int virtualBaseAddress)
{
  /*
   * Make sure we got an aligned and valid pointer...
   */
  if (l2Table == NULL)
  {
    return;
  }
  if (!isAlignedToMask(l2Table, PT2_ALIGN_MASK))
  {
    printf("Error: L2 page table base pointer unaligned; cannot dump!");
    return;
  }
  /*
   * L2 table size always fits in ONE small page.
   */
  if (isMmuEnabled())
  {
    PhysicalAddressRegister par = mmuTryTranslate((u32int)l2Table, TTP_PRIVILEGED_READ);
    if (par.fault.fault)
    {
      printf("Error: L2 table @ %p--%#.8x not mapped" EOL, l2Table, ((u32int)l2Table) + SMALL_PAGE_SIZE);
      return;
    }
  }
  /*
   * We should be able to read this table now; dump it.
   */
  u32int virtualAddress = virtualBaseAddress;
  for (largePageEntry *entry = l2Table; entry < l2Table + PT2_ENTRIES; entry++,
       virtualAddress += SMALL_PAGE_SIZE)
  {
    switch (entry->type)
    {
      case FAULT:
      {
        break;
      }
      case LARGE_PAGE:
      {
        u32int physicalAddress = entry->addr << 16;
        printf(" LP %.8x--%.8x %.8x--%.8x" EOL, physicalAddress, physicalAddress + LARGE_PAGE_SIZE,
               virtualAddress, virtualAddress + LARGE_PAGE_SIZE);
        break;
      }
      case SMALL_PAGE:
      case SMALL_PAGE_3:
      {
        smallPageEntry *smallPage = (smallPageEntry *)entry;
        u32int physicalAddress = smallPage->addr << 12;
        printf(" SP %.8x--%.8x %.8x--%.8x" EOL, physicalAddress, physicalAddress + SMALL_PAGE_SIZE,
               virtualAddress, virtualAddress + SMALL_PAGE_SIZE);
        break;
      }
      default:
      {
        printf(" ?? ------------------ ------------------ %.8x" EOL, *(u32int *)entry);
        break;
      }
    } // switch entry type
  } // for each entry
}
#endif
