#include "common/debug.h"

#include "guestManager/blockCache.h"

#include "vm/omap35xx/serial.h"

#include "instructionEmu/decoder.h"
#include "instructionEmu/scanner.h"

#include "memoryManager/mmu.h"
#include "memoryManager/pageTable.h"


//Uncomment to enable debugging: #define PC_DEBUG


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
#ifdef SCANNER_COUNTER
  scannerReqCounter++;

  if(scannerReqCounter == 0x1ad7d5)
            {
              //DIE_NOW(0,"Time is up!");
              //asm volatile("BKPT #0");
            }

#endif
#ifdef DUMP_SCANNER_COUNTER
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
# ifdef CONFIG_BLOCK_COPY
  struct instruction32bit * decodedInstruction = 0;
# endif
# ifdef CONFIG_DECODER_AUTO
  //instructionHandler decodedInstruction = 0;
  instructionHandler decodedInstruction = 0;
# else
#  error Decoder must be set!
# endif
#endif
#ifdef CONFIG_BLOCK_COPY
  u32int * blockCopyCacheCurrAddress = ((u32int* )(gc->blockCopyCacheLastUsedLine))+1;
  u32int * blockCopyCacheStartAddress = ((u32int* )(gc->blockCopyCacheLastUsedLine))+1;
  bool reservedWord = 0;//See struct blockCacheEntry in blockCache.h for explanation
  u32int blockCopyCacheSize=0;
#endif
  u32int * currAddress = (u32int*)blkStartAddr;
  u32int instruction = *currAddress;
  u32int hashVal = getHash(blkStartAddr);
  u32int bcIndex = (hashVal & (BLOCK_CACHE_SIZE-1)); // 0x1FF mask for 512 entry cache
  bool inBlockCache = checkBlockCache(blkStartAddr, bcIndex, gc->blockCache);
  if (inBlockCache)
  {
    //Check the logbook
    BCENTRY * bcEntry = getBlockCacheEntry(bcIndex, gc->blockCache);
    gc->hdlFunct = (u32int (*)(GCONTXT * context))bcEntry->hdlFunct;
    gc->endOfBlockInstr = bcEntry->hyperedInstruction;
#ifdef SCANNER_DEBUG
    serial_putstring("scanner: Block ");
    serial_putint(scannerReqCounter);
    serial_putstring(" @ ");
    serial_putint(blkStartAddr);
    serial_putstring(" hash value ");
    serial_putint(hashVal);
    serial_putstring(" cache index ");
    serial_putint(bcIndex);
    serial_putstring(" HIT");
    serial_newline();
#endif
#ifdef CONFIG_BLOCK_COPY
    u32int * addressInBlockCopyCache = 0;
    // The programcounter of the code that is executing should be set to the code in the blockCache
    if(bcEntry->reservedWord == 0)
    {
      addressInBlockCopyCache= ((u32int*)(bcEntry->blockCopyCacheAddress)) + 1;// First word is a backpointer
    }
    else
    {
      addressInBlockCopyCache= ((u32int*)(bcEntry->blockCopyCacheAddress)) + 2;//First word is a backpointer & 2nd word is reservedWord
    }

    if((u32int)addressInBlockCopyCache >= gc->blockCopyCacheEnd ){
      /* blockCopyCacheAddresses will be used in a  cyclic manner
      -> if end of blockCopyCache is passed blockCopyCacheCurrAddress must be updated */
      addressInBlockCopyCache=addressInBlockCopyCache - (BLOCK_COPY_CACHE_SIZE-1);
    }

    gc->R15 = (u32int)addressInBlockCopyCache;
    //But also the PC of the last instruction of the block should be set
    gc->PCOfLastInstruction = (u32int)bcEntry->endAddress;
#endif
    return;
  }

#ifdef SCANNER_DEBUG
  serial_putstring("scanner: Block ");
  serial_putint(scannerReqCounter);
  serial_putstring(" @ ");
  serial_putint(blkStartAddr);
  serial_putstring(" hash value ");
  serial_putint(hashVal);
  serial_putstring(" cache index ");
  serial_putint(bcIndex);
  serial_putstring(" MISS!!!");
  serial_newline();
#endif

#ifdef CONFIG_BLOCK_COPY
  /* Check if there is room on blockCopyCacheCurrAddres and if not make it */
  blockCopyCacheCurrAddress=checkAndClearBlockCopyCacheAddress(blockCopyCacheCurrAddress,gc->blockCache,(u32int*)gc->blockCopyCache,(u32int*)gc->blockCopyCacheEnd);
  /* Install Backpointer in BlockCache: */
  *(blockCopyCacheCurrAddress++)=(u32int)(gc->blockCache + bcIndex);//pointer arithmetic gc->blcokCache+bcIndex  and save pointer as u32int
  /* After the Backpointer the instructions will be installed.  Here the guestprocess should continue it's execution. */
  gc->R15=(u32int)checkAndClearBlockCopyCacheAddress(blockCopyCacheCurrAddress,gc->blockCache,(u32int*)gc->blockCopyCache,(u32int*)gc->blockCopyCacheEnd);

# ifdef SCANNER_DEBUG_BLOCKCOPY
  serial_putstring("Backpointer installed at: ");
  serial_putint((u32int)(blockCopyCacheCurrAddress-1));
  serial_putstring("Contents= ");
  serial_putint(*(blockCopyCacheCurrAddress-1));
  serial_newline();
# endif

  while (1)//Just keep on scanning untill function scanBlock returns.
  {
    //binary & checks types -> do a cast of function pointer to u32int
    if((decodedInstruction = decodeInstr(instruction))->replaceCode == 1)
    {  
		//Critical instruction
      /*----------------Install HdlFunct----------------*/
      /*Non of the source registers is the ProgramCounter -> Just End Of Block
       *Finish block by installing SVC
       *Save end of block instruction and handler function pointer close to us... */
      gc->endOfBlockInstr = instruction;
      gc->hdlFunct = decodedInstruction->hdlFunct;
      gc->PCOfLastInstruction = (u32int)currAddress;
      /* replace end of block instruction with hypercall of the appropriate code
       *Check if there is room on blockCopyCacheCurrAddress and if not make it */
      blockCopyCacheCurrAddress = checkAndClearBlockCopyCacheAddress(blockCopyCacheCurrAddress,gc->blockCache,(u32int*)gc->blockCopyCache,(u32int*)gc->blockCopyCacheEnd);
      *(blockCopyCacheCurrAddress++)= INSTR_SWI | ((bcIndex+1)<<8);

# ifdef SCANNER_DEBUG2
      serial_putstring("scanner: EOB @ ");
      serial_putint((u32int)currAddress);
      serial_putstring(" instr ");
      serial_putint(gc->endOfBlockInstr);
      serial_putstring(" SWIcode ");
      serial_putint(bcIndex);
      serial_putstring(" hdlrFuncPtr ");
      serial_putint((u32int)gc->hdlFunct);
      serial_newline();
# endif

      /*
       * We have to determine the size of the BlockCopyCache & If necessary patch the code
       * Patching of code is necessary when block is split up and a reserved word is used!
       */
      if(blockCopyCacheCurrAddress<blockCopyCacheStartAddress)
      {

        if(reservedWord==1)
        {
# ifdef SCANNER_DEBUG2
          serial_putstring("Reserved WORD");
# endif
          u32int* blockCopyLast = checkAndMergeBlock((u32int*)gc->blockCopyCache,blockCopyCacheCurrAddress, gc->blockCache,blockCopyCacheStartAddress, (u32int*)gc->blockCopyCacheEnd);

          /*
           * Make sure that block is safed correctly in blockCopyCache
           */
          //UPDATE blockCopyCacheStartAddress
          if(blockCopyLast!=blockCopyCacheCurrAddress)
          {
            blockCopyCacheCurrAddress=blockCopyLast;
            blockCopyCacheStartAddress=(u32int*)gc->blockCopyCache;
            //Set blockCopyCacheSize
            blockCopyCacheSize=blockCopyLast-blockCopyCacheStartAddress;
            /* Indicate that a free word is available at start of blockCopyCache */
            blockCopyCacheStartAddress=(u32int*) ( ((u32int)blockCopyCacheStartAddress) |0b1);
            /* Block is merged and moved to start of blockCopyCache.  We have to execute instructions from there!
             * But first word is backpointer and 2nd word is reserved word*/
            gc->R15=gc->blockCopyCache+8;
# ifdef SCANNER_DEBUG2
            serial_putstring("Block Merged.  New size = ");
            serial_putint(blockCopyCacheSize);
# endif
          }
          else
          {
            /* No patching needs to be done just set blockCopyCacheSize */
            blockCopyCacheSize=gc->blockCopyCacheEnd - ( ((u32int)blockCopyCacheStartAddress) & 0xFFFFFFFE );
            blockCopyCacheSize+=(u32int)blockCopyCacheCurrAddress - gc->blockCopyCache;
            blockCopyCacheSize=blockCopyCacheSize>>2;//we have casted pointers to u32int thus divide by 4 to get size in words
          }
        }//end of reserved word case
        else
        {
            blockCopyCacheSize=gc->blockCopyCacheEnd - ( ((u32int)blockCopyCacheStartAddress) & 0xFFFFFFFE );
            blockCopyCacheSize+=(u32int)blockCopyCacheCurrAddress - gc->blockCopyCache;
            blockCopyCacheSize=blockCopyCacheSize>>2;//we have casted pointers to u32int thus divide by 4 to get size in words
# ifdef SCANNER_DEBUG_BLOCKCOPY
            serial_putstring("Block exceeding end: blockCopyCacheSize=");
            serial_putint(blockCopyCacheSize);
# endif
        }
      }
      else
      {
        blockCopyCacheSize=blockCopyCacheCurrAddress- ( (u32int*)(((u32int)blockCopyCacheStartAddress) & 0xFFFFFFFE) );
      }
      /* This check is probably not necessary in production code but this can catch errors early on */
      if(blockCopyCacheSize > 0xFFFF)
      {
        serial_putstring("blockCache: ADD[");
        serial_putint(bcIndex);
        serial_putstring("] start@");
        serial_putint(blkStartAddr);
        serial_putstring(" end@");
        serial_putint((u32int)currAddress);
        serial_putstring(" hdlPtr ");
        serial_putint((u32int)gc->hdlFunct);
        serial_putstring(" eobInstr ");
        serial_putint(gc->endOfBlockInstr);
        serial_putstring(" blockCopyCacheSize ");
        serial_putint(blockCopyCacheSize);
        serial_putstring(" blockCopyCache@");
        serial_putint(((u32int)blockCopyCacheStartAddress));
        serial_newline();
        serial_putstring("Block nr:");
        serial_putint(scannerReqCounter);
        asm volatile("BKPT #0");
      }
      /* add the block we just scanned to block cache */
      addToBlockCache(blkStartAddr, (u32int)currAddress, bcIndex, blockCopyCacheSize,
                                        (u32int)blockCopyCacheStartAddress,gc->endOfBlockInstr,(u32int)gc->hdlFunct,gc->blockCache);

      blockCopyCacheStartAddress=(u32int*)( ((u32int)blockCopyCacheStartAddress) & 0xFFFFFFFE );

      protectScannedBlock(blkStartAddr, (u32int)currAddress);

      //update blockCopyCacheLastUsedLine (blockCopyCacheLastUsedLine is u32int -> add nrOfInstructions*4
      /* It doesn't matter if it points to the word before the blockCopyCache. (Guestcontext was even initialized this way) */
      gc->blockCopyCacheLastUsedLine=(u32int)(blockCopyCacheCurrAddress-1);




# ifdef SCANNER_DEBUG_BLOCKCOPY
      serial_putstring("Block added with size of ");
      serial_putint(((u32int)blockCopyCacheCurrAddress-(u32int)blockCopyCacheStartAddress));
      serial_putstring(" words.");
      serial_newline();
# endif
        /*----------------END Install HdlFunct----------------*/
        return;
    }else
    {
	  //Non critical instruction
      if(allSrcRegNonPC(instruction))
      { 
        //Non of the source registers is the ProgramCounter -> Safe instruction
        //Check if there is room on blockCopyCacheCurrAddress and if not make it
        blockCopyCacheCurrAddress=checkAndClearBlockCopyCacheAddress(blockCopyCacheCurrAddress,gc->blockCache,(u32int*)gc->blockCopyCache,(u32int*)gc->blockCopyCacheEnd);
        //copy instruction to Block Copy Cache
        *(blockCopyCacheCurrAddress++)=instruction;//Copy instruction and update pointer
      }
      else
      {  
        /* One of the source registers of the instruction is the ProgramCounter */
        /* Perform PCFunct-> necessary information = currAddress, */
        /* ----------------Execute PCFunct---------------- */
        gc->endOfBlockInstr = instruction;  /* Not really the endOfBlockInstr but we can use it */

        blockCopyCacheCurrAddress= decodedInstruction->PCFunct(gc,currAddress,blockCopyCacheCurrAddress,blockCopyCacheStartAddress);
        if(((u32int)blockCopyCacheCurrAddress & 0b1) == 0b1)
        {
          /* Last bit of returnAddress is used to indicate that a reserved word is necessary
           * -> we can assume 2 byte alignment (even in worst case scenario (thumb))*/
          if(reservedWord==1)
          {
            /* Place has already been made -> just restore blockCopyCacheCurrAddress */
            blockCopyCacheCurrAddress=(u32int*)((u32int)blockCopyCacheCurrAddress & 0xFFFFFFFE);/*Set last bit back to zero*/
          }
          else
          {
            /* Entry in blockCopyCache will have to look like:
             * |-------------------|
             * |  backpointer      |  = indicated by blockCopyCacheStartAddress
             * |  emptyWord        |  = resevedWord for storing backup registers
             * |      ...          |  = Here starts the translated block
             *
             * blockCopyCacheStartAddress**/
            u32int emptyWordPointer;
            u32int destEmptyWord = (u32int)(blockCopyCacheStartAddress + 1);
            u32int tempWordPointer;
            blockCopyCacheCurrAddress=(u32int*)((u32int)blockCopyCacheCurrAddress & 0xFFFFFFFE);/*Set last bit back to zero*/
            /* destEmptyWord can be set incorrectly if blockCopyCacheStartAddress is blockCopyCacheEnd -4
             * Because then destEmptyWord will be blockCopyCacheEnd but blockCopyCacheEnd contains a static branch
             * to start of blockCopyCache*/
            if(destEmptyWord == gc->blockCopyCacheEnd)
            {
              destEmptyWord = gc->blockCopyCache; //Reserved word will be at start of blockCopyCache
            }

            /*Set place for the reserved word correct now it is right before the instructions for the last instruction*/
            /*set pointer to the empty word.*/
            emptyWordPointer=(u32int)(blockCopyCacheCurrAddress-6);
            if(emptyWordPointer < gc->blockCopyCache)
            {
              /* If we are before the start of the Block Copy Cache than we need to go to the corresponding place near the end*/
              u32int diff = gc->blockCopyCache - emptyWordPointer;
              emptyWordPointer = gc->blockCopyCacheEnd-diff;
            }
            /* emptyWordPointer now points to the empty word*/

# ifdef SCANNER_DEBUG_BLOCKCOPY
            serial_putstring("Processing block :");
            serial_putint(scannerReqCounter);
            serial_newline();
            serial_putstring("emptyWordPointer :");
            serial_putint((u32int)emptyWordPointer);
            serial_newline();
            serial_putstring("destEmptyWord :");
            serial_putint((u32int)destEmptyWord);
            serial_newline();
# endif

            while(emptyWordPointer!=destEmptyWord)
            {

              /* As long as the empty word isn't at its place keep on moving instructions */
              tempWordPointer = emptyWordPointer - 4; /* previous word */
              if(tempWordPointer == (gc->blockCopyCache - 4))
              {
                /* Be careful when exceeding start of blockCopyCache */
                tempWordPointer = gc->blockCopyCacheEnd - 4;
              }
              *((u32int*)emptyWordPointer) = *((u32int*)tempWordPointer);
              emptyWordPointer=tempWordPointer;
            }
            *( (u32int*)emptyWordPointer)=0x0;/*Clear it so it cannot be a cause for confusion while debugging*/

            reservedWord=1;/*From now on there is a reserved word to save backups*/
            /* Indicate that a free word is available at start of blockCopyCache */
            blockCopyCacheStartAddress=(u32int*) ( ((u32int)blockCopyCacheStartAddress) |0b1);
          }

        }
        /*----------------END Execute PCFunct----------------*/
      }
    }
    // Instruction handled -> go to next instruction
    currAddress++;
    instruction = *currAddress;
  } // decoding while ends
}//end of scanBlock
# else  //not CONFIG_BLOCK_COPY
# ifdef CONFIG_DECODER_TABLE_SEARCH
  while ((decodedInstruction = decodeInstr(instruction))->replaceCode == 0)
# else
#  ifdef CONFIG_DECODER_AUTO
  while ((decodedInstruction = decodeInstr(instruction)) == 0)
#  else
#   error Decoder must be set!
#  endif
# endif
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
# ifdef SCANNER_DEBUG2
      serial_putstring("scanner: EOB instruction is SWI @ ");
      serial_putint((u32int)currAddress);
      serial_putstring(" code ");
      serial_putint(cacheIndex);
      serial_newline();
# endif
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
# ifdef CONFIG_DECODER_TABLE_SEARCH
    gc->hdlFunct = decodedInstruction->hdlFunct;
# else
#  ifdef CONFIG_DECODER_AUTO
    gc->hdlFunct = decodedInstruction;
#  else
#   error Decoder must be set!
#  endif
# endif
    // replace end of block instruction with hypercall of the appropriate code
    *currAddress = INSTR_SWI | ((bcIndex + 1) << 8);
    // if guest instruction stream is mapped with caching enabled, must maintain
    // i and d cache coherency
    // iCacheFlushByMVA((u32int)currAddress);
  }
  
# ifdef SCANNER_DEBUG2
  serial_putstring("scanner: EOB @ ");
  serial_putint((u32int)currAddress);
  serial_putstring(" instr ");
  serial_putint(gc->endOfBlockInstr);
  serial_putstring(" SWIcode ");
  serial_putint((bcIndex + 1) << 8);
  serial_putstring(" hdlrFuncPtr ");
  serial_putint((u32int)gc->hdlFunct);
  serial_newline();
# endif

  // add the block we just scanned to block cache
  addToBlockCache(blkStartAddr, gc->endOfBlockInstr, (u32int)currAddress, 
                  bcIndex, (u32int)gc->hdlFunct, gc->blockCache);

  protectScannedBlock(blkStartAddr, (u32int)currAddress);
  // and we're done.
}
#endif



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

#ifdef CONFIG_BLOCK_COPY
/* allSrcRegNonPC will return true if all source registers of an instruction are zero  */
u32int allSrcRegNonPC(u32int instruction)
{
  //Source registers correspond with the bits [0..3],[8..11] or [16..19]
  if((instruction & 0xF0000)==0xF0000 || (instruction & 0xF00)==0xF00 || (instruction & 0xF)==0xF )
  {
# ifdef PC_DEBUG
    serial_putstring("Instruction 0x");
    serial_putint(instruction);
    serial_putstring(" has a PC register");
    serial_newline();
# endif
    return 0;//false
  }
  else
  {
# ifdef PC_DEBUG
    serial_putstring("Instruction 0x");
    serial_putint(instruction);
    serial_putstring(" doesn't have a PC register");
    serial_newline();
# endif
    return 1;//true
  }
}
#endif // CONFIG_BLOCK_COPY

