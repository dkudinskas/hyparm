#include "common/debug.h"

#include "guestManager/blockCache.h"

#include "instructionEmu/decoder.h"
#include "instructionEmu/scanner.h"

#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"


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
    printf("scanner: Block @ %08x hash value %x cache index %x HIT\n", 
           blkStartAddr, hashVal, bcIndex);
#endif
    return;
  }

#ifdef SCANNER_DEBUG
    printf("scanner: Block @ %08x hash value %x cache index %x MISS\n", 
           blkStartAddr, hashVal, bcIndex);
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
      printf("scanBlock: SWI code = %x\n", svcCode);
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
      printf("scanner: EOB instruction is SWI @ %08x code %x\n", (u32int)currAddress, cacheIndex);
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
  printf("scanner: EOB @ %08x insr %08x SVC code %x hdlrFuncPtr %x\n",
        currAddress, gc->endOfBlockInstr, ((bcIndex + 1) << 8), (u32int)gc->hdlFunct);
#endif

  // add the block we just scanned to block cache
  addToBlockCache(blkStartAddr, gc->endOfBlockInstr, (u32int)currAddress, 
                  bcIndex, (u32int)gc->hdlFunct, gc->blockCache);
  /* To ensure that subsequent fetches from eobAddress get a hypercall
   * rather than the old cached copy... 
   * 1. clean data cache entry by address
   * DCCMVAU, Clean data cache line by MVA to PoU: c7, 0, c11, 1 
   * 2. invalidate instruction cache entry by address.
   * ICIMVAU, Invalidate instruction caches by MVA to PoU: c7, 0, c5, 1
   */
  asm("mcr p15, 0, %0, c7, c11, 1"
  :
  :"r"(currAddress)
  :"memory"
  );
  asm("mcr p15, 0, %0, c7, c5, 1"
  :
  :"r"(currAddress)
  :"memory"
  );
  
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
        u32int mbStart = startAddress & 0xFFF00000;
        u32int mbEnd   = endAddress & 0xFFF00000;
        if (mbStart != (mbEnd - 0x00100000))
        {
          printf("startAddress %08x, endAddress %08x\n", startAddress, endAddress); 
          DIE_NOW(0, "protectScannedBlock: Basic block crosses non-sequential section boundary!");
        }
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
          printf("Page size: 64KB (large), %08x\n", ptEntryLvl2);
          DIE_NOW(0, "Unimplemented.");
          break;
        case SMALL_PAGE:
          if ((ptEntryLvl2 & 0x30) != 0x20)
          {
            addProtection(startAddress, endAddress, 0, PRIV_RW_USR_RO);
          }
          break;
        case FAULT:
          printf("Page invalid, %08x\n", ptEntryLvl2);
          DIE_NOW(0, "Unimplemented.");
          break;
        default:
          DIE_NOW(0, "Unrecognized second level entry");
          break;
      }
      break;
    }
    case FAULT:
      printf("Entry for basic block: invalid, %08x\n", ptEntryAddr);
      DIE_NOW(0, "Unimplemented.");
      break;
    case RESERVED:
      DIE_NOW(0, "Entry for basic block: reserved. Error.");
      break;
    default:
      DIE_NOW(0, "Unrecognized second level entry. Error.");
  }
}
