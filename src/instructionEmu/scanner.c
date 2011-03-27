#include "common/debug.h"

#include "guestManager/blockCache.h"

#include "vm/omap35xx/serial.h"

#include "instructionEmu/decoder.h"
#include "instructionEmu/scanner.h"

#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"


#ifdef DUMP_SCANNER_COUNTER
static u32int scannerReqCounter = 0;
#endif

// http://www.concentric.net/~Ttwang/tech/inthash.htm
// 32bit mix function
static inline u32int getHash(u32int key)
{
  key = ~key + (key << 15); // key = (key << 15) - key - 1;
  key = key ^ (key >> 12);
  key = key + (key << 2);
  key = key ^ (key >> 4);
  key = key * 2057; // key = (key + (key << 3)) + (key << 11);
  key = key ^ (key >> 16);
  return key >> 2;
}

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
#endif

#ifdef CONFIG_DECODER_TABLE_SEARCH
  struct instruction32bit * decodedInstruction = 0;
#else
# ifdef CONFIG_DECODER_AUTO
  //instructionHandler decodedInstruction = 0;
  instructionHandler decodedInstruction = 0;
# else
#  error Decoder must be set!
# endif
#endif
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

#ifdef CONFIG_DECODER_TABLE_SEARCH
  while ((decodedInstruction = decodeInstr(instruction))->replaceCode == 0)
#else
# ifdef CONFIG_DECODER_AUTO
  while ((decodedInstruction = decodeInstr(instruction)) == 0)
# else
#  error Decoder must be set!
# endif
#endif
  {
    currAddress++;
    instruction = *currAddress;
  } // while ends

  if ((instruction & INSTR_SWI) == INSTR_SWI)
  {
    u32int svcCode = (instruction & 0x00FFFFFF);
    if ((svcCode >= 0) && (svcCode <= 0xFF))
    {
      serial_putstring("scanBlock: SWI code = ");
      serial_putint(svcCode);
      serial_newline();
      DIE_NOW(gc, "scanBlock: SVC instruction not placed by hypervisor!");
    }
    else
    {
      // we hit a SWI that we placed ourselves as EOB. retrieve the real EOB...
      u32int cacheIndex = (svcCode >> 8) - 1;
      if (cacheIndex >= BLOCK_CACHE_SIZE)
      {
        DIE_NOW(gc, "scanner: block cache index in SWI out of range.");
      }
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
  }
  else
  {
    // save end of block instruction and handler function pointer close to us...
    gc->endOfBlockInstr = instruction;
#ifdef CONFIG_DECODER_TABLE_SEARCH
    gc->hdlFunct = decodedInstruction->hdlFunct;
#else
# ifdef CONFIG_DECODER_AUTO
    gc->hdlFunct = decodedInstruction;
# else
#  error Decoder must be set!
# endif
#endif
    // replace end of block instruction with hypercall of the appropriate code
    *currAddress = INSTR_SWI | ((bcIndex + 1) << 8);
    // if guest instruction stream is mapped with caching enabled, must maintain
    // i and d cache coherency
    // iCacheFlushByMVA((u32int)currAddress);
  }
  
#ifdef SCANNER_DEBUG
  serial_putstring("scanner: EOB @ ");
  serial_putint((u32int)currAddress);
  serial_putstring(" instr ");
  serial_putint(gc->endOfBlockInstr);
  serial_putstring(" SWIcode ");
  serial_putint((bcIndex + 1) << 8);
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
        DIE_NOW(0, "protectScannedBlock: Basic block crosses section boundary!");
      }
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
          DIE_NOW(0, "Unimplemented.");
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
          DIE_NOW(0, "Unimplemented.");
          break;
        default:
          DIE_NOW(0, "Unrecognized second level entry");
          serial_newline();
          break;
      }
      break;
    }
    case FAULT:
      serial_putstring("Entry for basic block: invalid, 0x");
      serial_putint(*(u32int*)ptEntryAddr);
      serial_newline();
      DIE_NOW(0, "Unimplemented.");
      break;
    case RESERVED:
      DIE_NOW(0, "Entry for basic block: reserved. Error.");
      break;
    default:
      DIE_NOW(0, "Unrecognized second level entry. Error.");
  }
}

#ifdef DUMP_SCANNER_COUNTER
void resetScannerCounter()
{
  scannerReqCounter = 0;
}
#endif
