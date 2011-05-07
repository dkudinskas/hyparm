#include "common/debug.h"

#include "guestManager/blockCache.h"
#include "guestManager/guestExceptions.h"

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
  instructionHandler decodedInstruction = 0;
# else
#  error Decoder must be set!
# endif
#endif
  u32int * currAddress = (u32int*)blkStartAddr;
  // Thumb pointer
  u16int * currhwAddress = (u16int*)blkStartAddr;
  u16int * currtmpAddress = 0;
  u32int instruction = 0;
  u16int halfinstruction = 0;
  u32int hashVal = getHash(blkStartAddr);
  u32int bcIndex = (hashVal & (BLOCK_CACHE_SIZE-1)); // 0x1FF mask for 512 entry cache
  
  bool inBlockCache = checkBlockCache(blkStartAddr, bcIndex, gc->blockCache);
  if (inBlockCache)
  {
	
	BCENTRY * bcEntry = getBlockCacheEntry(bcIndex, gc->blockCache);
    gc->hdlFunct = (u32int (*)(GCONTXT * context))bcEntry->hdlFunct;
    gc->endOfBlockInstr = bcEntry->hyperedInstruction;
	gc->endOfBlockHalfInstr = bcEntry->halfhyperedInstruction;

//#ifdef SCANNER_DEBUG
	if(gc->CPSR & T_BIT)
	{
	    printf("scanner: Block @ %08x hash value %x cache index %x HIT\n", 
    	       blkStartAddr, hashVal, bcIndex);
	 	printf("OriginFull %08x Half %08x\n", gc->endOfBlockInstr, gc->endOfBlockHalfInstr);
	}
//#endif
    return;
  }

#ifdef SCANNER_DEBUG
    printf("scanner: Block @ %08x hash value %x cache index %x MISS\n", 
           blkStartAddr, hashVal, bcIndex);
#endif
		
  // Adjust values for Thumb-2
  if(gc->CPSR & T_BIT)
  {
  	
	// point to halfward rather than a whole word
	halfinstruction = *currhwAddress;
	//backup pointer
	currtmpAddress = currhwAddress;
#ifdef SCANNER_DEBUG
	printf("Thumb 16-bit Instruction: %08x\n",halfinstruction);
#endif
	switch(halfinstruction & THUMB32)
	{
		// Is this halfword a thumb32 encoding?
		case THUMB32_1:
		case THUMB32_2:
		case THUMB32_3:
		{
			// If this is a thumb32 encoding, then move one instruction ahead and concatenate both
			currhwAddress++;
			instruction = halfinstruction<<16|*currhwAddress;
			break;
		}
		default:
		{
			// Maybe this instruction is the second word of a 32-bit Thumb instruction. Check!
			currhwAddress--; // move one instruction before
			switch(*currhwAddress & THUMB32)
			{
				case THUMB32_1:
				case THUMB32_2:
				case THUMB32_3:
				{
					// the previous instruction was a Thumb-32 first halfword. Concatenate them
					halfinstruction = *currhwAddress; // <- this holds the first word of the Thumb-32 instruction
					// move back to the original instruction
					currhwAddress++;
					instruction = halfinstruction<<16|*currhwAddress;
				}
				default:
				{
					// OK the previous halfword wasn't a Thumb-32 encoding. Move again to our instruction
					currhwAddress++;
					instruction = halfinstruction;
				}
			break;
		}
	}
//#ifdef SCANNER_DEBUG
	printf("Thumb: %08x@%08x\n",instruction,(u32int)currhwAddress);
//#endif
   }
  }
   else
   {
   	//grab the ARM instruction
	instruction = *currAddress;
   }
#ifdef CONFIG_DECODER_TABLE_SEARCH
  while ((decodedInstruction = decodeInstr(instruction,currhwAddress))->replaceCode == 0)
#else
# ifdef CONFIG_DECODER_AUTO
  while ((decodedInstruction = decodeInstr(instruction)) == 0)
# else
#  error Decoder must be set!
# endif
#endif
	{	
		// Thumb-2 is moving by 2 bytes at a time
		if(gc->CPSR & T_BIT)
		// We start by scanning two byte at a time.
		{
			currhwAddress++;
			halfinstruction = *currhwAddress;
			// backup pointer
			currtmpAddress = currhwAddress;
			// check for Thumb-32 bit encoding
			switch(halfinstruction & THUMB32) 
			{
				case THUMB32_1:
				case THUMB32_2:
				case THUMB32_3:
				{
					// fetch the remaining halfword
					currhwAddress++;
					instruction = halfinstruction<<16|*currhwAddress;
					break;
				}
				// if the halfword is a 16bit instruction
				default:
				{
					// maybe this is the second half word of a thumb-2 instruction?. Check the previous one
					currhwAddress--;
					switch(*currhwAddress & THUMB32)
					{
						case THUMB32_1:
						case THUMB32_2:
						case THUMB32_3:
						{	// if we are here, it means the the current instruction is the second halfword
							// of a thumb 32-bit instr
							halfinstruction = *currhwAddress; // fetch the first half word of Thumb-32
							currhwAddress++; // move forward, fetch the next one, and concatenate them
							instruction = halfinstruction<<16|*currhwAddress;
							break;
						}
						default:
						{
							// if we are here it means that the previous instruction was a 16bit standalone instruction
							// and so is this one
							currhwAddress++;
							//keep only the last 16 bits
							instruction = halfinstruction;
							break;
						}
					}
				}
			}
		}
		// ARM Mode
		else
		{
			currAddress++;
			instruction = *currAddress;
		}
	} // while ends
  	//ARM Mode
	if(!(gc->CPSR & T_BIT))
  	{
	  if ((instruction & INSTR_SWI) == INSTR_SWI)
  	  {
      	u32int svcCode = (instruction & 0x00FFFFFF);
	  	if(!((svcCode >= 0) && (svcCode <= 0xFF)))
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
		/* If the instruction is not a SWI placed by the hypervisor OR 
		 * it is a non-SWI instruction, then proceed as normal
		 */
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
    	// Thumb compatibility
		gc->endOfBlockHalfInstr = 0;
		// replace end of block instruction with hypercall of the appropriate code
	    *currAddress = INSTR_SWI | ((bcIndex + 1) << 8);
    	// if guest instruction stream is mapped with caching enabled, must maintain
	    // i and d cache coherency
    	// iCacheFlushByMVA((u32int)currAddress);
  	  }
	}
	
	//------------------------------ THUMB ---------------------------------//
	else
	{
		if ( 
				(	
					(instruction & INSTR_SWI_THUMB_MIX) == INSTR_SWI_THUMB_MIX)
				|| 
				( 
					((instruction & 0xFFFFDFFF) >= 0xDF00)
					&& 
					((instruction & 0xFFFFDFFF) <= 0xDFFF)
				)
			) // FIX ME -> This doesn't look right
 		{
			u32int svcCode = (instruction & 0x000000FF); // NOP|SVC -> Keep the last 8 bits
	  		if(svcCode > 0)
	  		{
				// we hit a SWI that we placed ourselves as EOB. retrieve the real EOB...
    		  	u32int cacheIndex = svcCode - 1;
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
				gc->endOfBlockHalfInstr = bcEntry->halfhyperedInstruction;
				gc->hdlFunct = (u32int (*)(GCONTXT * context))bcEntry->hdlFunct;
	    	 }
		}	
		/* If the instruction is not a SWI placed by the hypervisor OR 
		 * it is a non-SWI instruction, then proceed as normal
		 */
	  	else
	  	{	
    	
		/* Replace policy:
		 * CurrAddress can point to any of these
		 * 1) 32-bit word where lowest halfword is a single Thumb 16-bit instruction
		 * 2) 32-bit word where lowest halfword is the second halfword of a Thumb 32-bit instruction
		 * . In this case, the high halfword instruction is located in CurrAddres-2bytes
		 * 3) 32-bit word where lowest halfword is the high halfword of a Thumb-32 instruction. In
		 * this case, the remaining halfword is located in CurrAddress+2 bytes
		 * To identify what kind of instruction this is, each 16bit portion has to be checked for 
		 * Thumb-32 compatible encoding.
		 */
		currhwAddress = currtmpAddress; // restore starting pointer and do what we did before
		halfinstruction = * currhwAddress;
		switch(halfinstruction & THUMB32)
		{
			case THUMB32_1:
			case THUMB32_2:
			case THUMB32_3:
			{
				// This looks like the first halfword of a thumb 32-bit instruction but
				// it may as well be a lower halfword. So check the previous word as well
				currhwAddress--;
				switch(*currhwAddress & THUMB32)
				{
					case THUMB32_1:
					case THUMB32_2:
					case THUMB32_3:
					{
						//so the previous halfword matches the Thumb-2 encoding so it
						//*SHOULD* be the first halfword of a thumb2 32-bit instruction					
						halfinstruction = *currhwAddress;
						currhwAddress++;
						instruction = halfinstruction<<16|*currhwAddress;
						// Mark this entry as Thumb 32
						gc->endOfBlockHalfInstr = THUMB32_LOW;
						gc->endOfBlockInstr = instruction;
						
						// replace what needs to be replaced
						currhwAddress--; // First Thumb word gets a NOT
						*currhwAddress = INSTR_NOP_THUMB;
						currhwAddress++; // Second Thumb word is replaced by SWI
						*currhwAddress = INSTR_SWI_THUMB|((bcIndex+1) & 0xFF);
						break;
					}
					default:
					{
						// so the previous instruction is not a Thumb-32. We need to fetch the next instruction is concatenate with the thumb32 instruction
						currhwAddress ++; // this points to the first instruction of thumb
						halfinstruction = *currhwAddress;
						currhwAddress ++; // this points to the next one
						instruction = halfinstruction<<16|*currhwAddress;
						gc->endOfBlockHalfInstr = THUMB32_HIGH;
						gc->endOfBlockInstr = instruction;
						currhwAddress --; // this points to the first instruction of thumb
						*currhwAddress = INSTR_NOP_THUMB;
						currhwAddress ++;
						*currhwAddress = INSTR_SWI_THUMB|((bcIndex+1) & 0xFF);
					}
				}
				break;
			}
			default:
			{
				//This seems to be a single 16-bit instruction or a low Thumb 32bit instruction.
				//check the previous halfword to see if it is the high halfword instruction there
				currhwAddress--;
				switch((*currhwAddress) & THUMB32)
				{
					case THUMB32_1:
					case THUMB32_2:
					case THUMB32_3:
					{
						halfinstruction = *currhwAddress; // Fetch the first thumb-32 instruction
						currhwAddress++;//fetch the next one and construct the instruction
						instruction = halfinstruction<<16|*currhwAddress;
						gc->endOfBlockHalfInstr = THUMB32_HIGH; // Mark it as Thumb-32
						gc->endOfBlockInstr = instruction;
						
						// replace what needs to be replaced
						currhwAddress--;
						*currhwAddress = INSTR_NOP_THUMB;
						currhwAddress++; // go 2 bytes ahead
						*currhwAddress = INSTR_SWI_THUMB | (( bcIndex +1 ) & 0xFF); // keep only the lowest 8 bits
						break;
					}
					default:
					{
						//The previous instruction was a standalone 16-bit instruction, so is the current one
						currhwAddress++;
						instruction = *currhwAddress;
						gc->endOfBlockInstr = instruction;
						gc->endOfBlockHalfInstr = THUMB16;
						*currhwAddress=INSTR_SWI_THUMB | ((bcIndex + 1) & 0xFF);
						break;
					}
				}
				break;
			}
		}
		printf("Thumb svc on %08x\n",(u32int)currhwAddress);
#ifdef CONFIG_DECODER_TABLE_SEARCH
	    gc->hdlFunct = decodedInstruction->hdlFunct;
#else
# ifdef CONFIG_DECODER_AUTO
    	gc->hdlFunct = decodedInstruction;
# else
#  error Decoder must be set!
# endif
#endif
  	  }
	}
#ifdef SCANNER_DEBUG
  printf("scanner: EOB @ %08x insr %08x SVC code %x hdlrFuncPtr %x\n",
        currAddress, gc->endOfBlockInstr, ((bcIndex + 1) << 8), (u32int)gc->hdlFunct);
#endif

  /* add the block we just scanned to block cache
   * Ehm... Do not do that for guest SVC code. It messes up everything so 
   * skipt it until I figure out what it going on
   */

  //currAddress has to point to be in sync with currhwAddress
  if(gc->CPSR & T_BIT)
  {
  	currAddress=(u32int*)(u32int)(currhwAddress);
  }
  addToBlockCache(blkStartAddr, gc->endOfBlockInstr, gc->endOfBlockHalfInstr, (u32int)currAddress, 
                  bcIndex, (u32int)gc->hdlFunct, gc->blockCache);
  /* To ensure that subsequent fetches from eobAddress get a hypercall
   * rather than the old cached copy... 
   * 1. clean data cache entry by address
   * DCCMVAU, Clean data cache line by MVA to PoU: c7, 0, c11, 1 
   * 2. invalidate instruction cache entry by address.
   * ICIMVAU, Invalidate instruction caches by MVA to PoU: c7, 0, c5, 1
   */
 	
	if(gc->CPSR & T_BIT)
	{
		if(gc->endOfBlockHalfInstr == THUMB16)
		{
			asm("mcr p15, 0, %0, c7, c11, 1"
			:
			:"r"(currhwAddress)
			:"memory"
			);
			asm("mcr p15, 0, %0, c7, c5, 1"
			:
			:"r"(currhwAddress)
			:"memory"
			);
		}
		else
		{
			//currhwAddress points to the second halfword
			asm("mcr p15, 0, %0, c7, c11, 1"
			:
			:"r"(currhwAddress)
			:"memory"
			);
			asm("mcr p15, 0, %0, c7, c5, 1"
			:
			:"r"(currhwAddress)
			:"memory"
			);
			currhwAddress--;
			asm("mcr p15, 0, %0, c7, c11, 1"
			:
			:"r"(currhwAddress)
			:"memory"
			);
			asm("mcr p15, 0, %0, c7, c5, 1"
			:
			:"r"(currhwAddress)
			:"memory"
			);
		}
	}
    else
	{
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
	}
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
