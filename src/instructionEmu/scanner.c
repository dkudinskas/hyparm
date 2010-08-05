#include "scanner.h"
#include "blockCache.h"
#include "common.h"
#include "mmu.h"
#include "pageTable.h"

#ifdef DUMP_SCANNER_COUNTER
bool dumpTrace = FALSE;
static u32int scannerReqCounter = 0;
#endif

void scanBlock(GCONTXT * gc, u32int blkStartAddr)
{
#ifdef DUMP_SCANNER_COUNTER
  scannerReqCounter++;
  if ((scannerReqCounter % 4000) == 3999)
  {
    serial_putint_nozeros(scannerReqCounter);
    serial_putstring(" ");
    if ((scannerReqCounter % 80000) == 79999)
    {
      serial_newline();
    }
  }
  dumpTrace = (scannerReqCounter >= 0xda23bf);
#endif
  struct instruction32bit * decodedInstruction = 0;
  u32int * currAddress = (u32int*)blkStartAddr;
  u32int instruction = *currAddress;

  u32int hashVal = getHash(blkStartAddr);
  u32int bcIndex = (hashVal & (BLOCK_CACHE_SIZE-1)); // 0x1FF mask for 512 entry cache
  
  bool inBlockCache = checkBlockCache(blkStartAddr, bcIndex, gc->blockCache);

  if (inBlockCache)
  {
    BCENTRY * bcEntry = getBlockCacheEntry(bcIndex, gc->blockCache);
    gc->hdlFunct = (u32int (*)(GCONTXT * context))bcEntry->hdlFunct;
    gc->endOfBlockInstr = bcEntry->hyperedInstruction;
#ifdef SCANNER_DEBUG
    serial_putstring("scanner: Block @ ");
    serial_putint(blkStartAddr);
    serial_putstring(" hash value ");
    serial_putint(hashVal);
    serial_putstring(" cache index ");
    serial_putint(bcIndex);
    serial_putstring(" HIT");
    serial_newline();
#endif
    return;
  }

#ifdef SCANNER_DEBUG
  serial_putstring("scanner: Block @ ");
  serial_putint(blkStartAddr);
  serial_putstring(" hash value ");
  serial_putint(hashVal);
  serial_putstring(" cache index ");
  serial_putint(bcIndex);
  serial_putstring(" MISS!!!");
  serial_newline();
#endif


  while ((decodedInstruction = decodeInstr(instruction))->replaceCode == 0)
  {
    currAddress++;
    instruction = *currAddress;
  } // while ends

  if ((instruction & INSTR_SWI) == INSTR_SWI)
  {
    // we hit a SWI that we placed ourselves as EOB. retrieve the real EOB...
    u32int cacheIndex = instruction & 0x00FFFFFF;
#ifdef SCANNER_DEBUG
    serial_putstring("scanner: EOB instruction is SWI @ ");
    serial_putint((u32int)currAddress);
    serial_putstring(" code ");
    serial_putint(cacheIndex);
    serial_newline();
#endif
    BCENTRY * bcEntry = getBlockCacheEntry(cacheIndex, gc->blockCache);

    // retrieve end of block instruction and handler function pointer
    gc->endOfBlockInstr = bcEntry->hyperedInstruction;
    gc->hdlFunct = (u32int (*)(GCONTXT * context))bcEntry->hdlFunct;
  }
  else
  {
    // save end of block instruction and handler function pointer close to us...
    gc->endOfBlockInstr = instruction;
    gc->hdlFunct = decodedInstruction->hdlFunct;
  }

  // replace end of block instruction with hypercall of the appropriate code
  *currAddress = (INSTR_SWI | bcIndex);
  
#ifdef SCANNER_DEBUG
  serial_putstring("scanner: EOB @ ");
  serial_putint((u32int)currAddress);
  serial_putstring(" instr ");
  serial_putint(gc->endOfBlockInstr);
  serial_putstring(" SWIcode ");
  serial_putint(bcIndex);
  serial_putstring(" hdlrFuncPtr ");
  serial_putint((u32int)gc->hdlFunct);
  serial_newline();
#endif

  // add the block we just scanned to block cache
  addToBlockCache(blkStartAddr, gc->endOfBlockInstr, (u32int)currAddress, 
                  bcIndex, (u32int)gc->hdlFunct, gc->blockCache);


  protectScannedBlock(blkStartAddr, (u32int)currAddress);
  // and we're done.
}

void protectScannedBlock(u32int startAddress, u32int endAddress)
{
  // 1. get page table entry for this address
  descriptor* ptBase = mmuGetPt0();
  descriptor* ptEntryAddr = get1stLevelPtDescriptorAddr(ptBase, startAddress);

  switch(ptEntryAddr->type)
  {
    case SECTION:
    {
      if ((startAddress & 0xFFF00000) != (endAddress & 0xFFF00000))
      {
        serial_ERROR("protectScannedBlock: Basic block crosses section boundary!");
      }
      splitSectionToSmallPages(ptBase, startAddress);
      addProtection(startAddress, endAddress, 0, PRIV_RW_USR_RO);
      break;
    }
    case PAGE_TABLE:
    {
      u32int ptEntryLvl2 = *(u32int*)(get2ndLevelPtDescriptor((pageTableDescriptor*)ptEntryAddr, endAddress));
      switch(ptEntryLvl2 & 0x3)
      {
        case LARGE_PAGE:
          serial_putstring("Page size: 64KB (large), 0x");
          serial_putint(ptEntryLvl2);
          serial_newline();
          serial_ERROR("Unimplemented.");
          break;
        case SMALL_PAGE:
          if ((ptEntryLvl2 & 0x30) != 0x20)
          {
            addProtection(startAddress, endAddress, 0, PRIV_RW_USR_RO);
          }
          break;
        case FAULT:
          serial_putstring("Page invalid, 0x");
          serial_putint(ptEntryLvl2);
          serial_newline();
          serial_ERROR("Unimplemented.");
          break;
        default:
          serial_ERROR("Unrecognized second level entry");
          serial_newline();
          break;
      }
      break;
    }
    case FAULT:
      serial_putstring("Entry for basic block: invalid, 0x");
      serial_putint(*(u32int*)ptEntryAddr);
      serial_newline();
      serial_ERROR("Unimplemented.");
      break;
    case RESERVED:
      serial_ERROR("Entry for basic block: reserved. Error.");
      break;
    default:
      serial_ERROR("Unrecognized second level entry. Error.");
  }
}

#ifdef DUMP_SCANNER_COUNTER
void resetScannerCounter()
{
  scannerReqCounter = 0;
}
#endif
