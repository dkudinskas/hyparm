#include "blockCache.h"
#include "memFunctions.h"
#include "serial.h"
#include "guestContext.h"

extern GCONTXT * getGuestContext(void); //from main.c

#define BLOCK_COPY_CACHE_DEBUG 1

#ifdef DUMP_COLLISION_COUNTER
static u32int collisionCounter = 0;
#endif

#define NUMBER_OF_BITMAPS       16
#define MEMORY_PER_BITMAP       0x10000000
#define MEMORY_PER_BITMAP_BIT  (MEMORY_PER_BITMAP / 32) // should be 8 megabytes

static u32int execBitMap[NUMBER_OF_BITMAPS];

void initialiseBlockCache(BCENTRY * bcache)
{
  int i = 0;
#ifdef DUMP_COLLISION_COUNTER
  collisionCounter = 0;
#endif
#ifdef BLOCK_CACHE_DEBUG
  serial_putstring("Initialising basic block cache @ address ");
  serial_putint((u32int)bcache);
  serial_newline();
#endif

  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    bcache[i].startAddress = 0;
    bcache[i].endAddress = 0;
    bcache[i].hyperedInstruction = 0;
    bcache[i].valid = FALSE;
    bcache[i].blockCopyCacheSize = 0;
    bcache[i].blockCopyCacheAddress = 0;
    bcache[i].hdlFunct = 0;
  }
  
  for (i = 0; i < NUMBER_OF_BITMAPS; i++)
  {
    execBitMap[i] = 0;
  }
}

bool checkBlockCache(u32int blkStartAddr, u32int bcIndex, BCENTRY * bcAddr)
{
#ifdef BLOCK_CACHE_DEBUG
  serial_putstring("blockCache: checkBlockCache index ");
  serial_putint(bcIndex);
  serial_newline();
#endif

  if (bcAddr[bcIndex].valid == FALSE)
  {
    // cache entry invalid
    return FALSE;
  }
  else
  {
    // cache entry valid, but is this a collision? ->Hashes are used so it must be checked that the startAddress is indeed equal
    if (bcAddr[bcIndex].startAddress != blkStartAddr)
    {
      return FALSE;
    }
    else
    {
      return TRUE;
    }
  }
}

void addToBlockCache(u32int blkStartAddr, u32int blkEndAddr,
                     u32int index, u32int blockCopyCacheSize, u32int blockCopyCacheAddress,u32int hypInstruction,u32int hdlFunct,BCENTRY * bcAddr)
{

#ifdef BLOCK_CACHE_DEBUG
  serial_putstring("blockCache: ADD[");
  serial_putint(index);
  serial_putstring("] start@");
  serial_putint(blkStartAddr);
  serial_putstring(" end@");
  serial_putint(blkEndAddr);
  serial_putstring(" hdlPtr ");
  serial_putint(hdlFunct);
  serial_putstring(" eobInstr ");
  serial_putint(hypInstruction);
  serial_putstring(" blockCopyCacheSize ")
  serial_putint(blockCopyCacheSize);
  serial_putstring(" blockCopyCache@");
  serial_putint(blockCopyCacheAddress);
  serial_newline();
#endif
  if (bcAddr[index].valid == TRUE)
  {

    // somebody has been sleeping in our cache location!
    resolveCacheConflict(index, bcAddr);
    // now that we resolved the conflict, we arrive at situation where bcAddr[index].valid==false
  }
  //store the new entry data...
  if(blkStartAddr & 0b1)//Last bit of blkStartAddr is used to indicate that a reserved word is present in blockCopyCache (see scanner.c)
  {
    bcAddr[index].reservedWord = 1;//Set reservedWord to true
    bcAddr[index].startAddress = blkStartAddr & 0xFFFFFFFE;//Set last bit back to zero!!
  }
  else
  {
    bcAddr[index].startAddress = blkStartAddr;
  }
  bcAddr[index].endAddress = blkEndAddr;
  bcAddr[index].hyperedInstruction = hypInstruction;
  bcAddr[index].hdlFunct = hdlFunct;
  bcAddr[index].valid = TRUE;
  bcAddr[index].blockCopyCacheSize = blockCopyCacheSize;
  bcAddr[index].blockCopyCacheAddress = blockCopyCacheAddress;

  
  // set bitmap entry to executed
  setExecBitMap(blkEndAddr);
}

BCENTRY * getBlockCacheEntry(u32int index, BCENTRY * bcAddr)
{
#ifdef BLOCK_CACHE_DEBUG
  serial_putstring("blockCache: getBlockCacheEntry index ");
  serial_putint(index);
  serial_newline();
#endif
  return &bcAddr[index];
}


/* input: any address, might be start, end of block or somewhere in the middle... */
/* output: first cache entry index for the block where this address falls into */
/* output: if no such block, return -1 (0xFFFFFFFF) */
u32int findEntryForAddress(BCENTRY * bcAddr, u32int addr)
{
  int i = 0;
  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    if (bcAddr[i].valid == TRUE)
    {
      // entry valid
      if ( (bcAddr[i].startAddress <= addr) && (bcAddr[i].endAddress >= addr) )
      {
        // addr falls in-between start-end inclusive. found a matching entry.
#ifdef BLOCK_CACHE_DEBUG
        serial_putstring("blockCache: found entry for address.");
        serial_newline();
        serial_putstring("blockCache: block start ");
        serial_putint(bcAddr[i].startAddress);
        serial_putstring(" block end ");
        serial_putint(bcAddr[i].endAddress);
        serial_putstring(" index ");
        serial_putint(i);
        serial_newline();
#endif
        return i;
      }
    }
  }
  return -1;
}

/* remove a specific cache entry */
void removeCacheEntry(BCENTRY * bcAddr, u32int cacheIndex)
{
  //The copied code has to be cleaned up
#ifdef BLOCK_COPY_CACHE_DEBUG
  serial_putstring("removeCache entered.  Address of cache(logbook) entry:");
  serial_putint((u32int) (bcAddr+cacheIndex));
  serial_putstring(" gets cleaned");
  serial_newline();
#endif
  u32int blockCopyCacheAddressOfEntry = bcAddr[cacheIndex].blockCopyCacheAddress;
  u32int blockCopyCacheSizeOfEntry = bcAddr[cacheIndex].blockCopyCacheSize;
#ifdef BLOCK_COPY_CACHE_DEBUG
  serial_putstring("removeBlockCopyCache entry: startAddress:");
  serial_putint((u32int)blockCopyCacheAddressOfEntry);
  serial_putstring(" size:");
  serial_putint(blockCopyCacheSizeOfEntry);
  serial_newline();
#endif
  removeBlockCopyCacheEntry(blockCopyCacheAddressOfEntry,blockCopyCacheSizeOfEntry);

  bcAddr[cacheIndex].valid = FALSE;
  bcAddr[cacheIndex].startAddress = 0;
  bcAddr[cacheIndex].endAddress = 0;
  bcAddr[cacheIndex].hdlFunct = 0;
  bcAddr[cacheIndex].blockCopyCacheSize = 0;
  bcAddr[cacheIndex].blockCopyCacheAddress = 0;
  bcAddr[cacheIndex].hyperedInstruction = 0;

  return;
}

void resolveCacheConflict(u32int index, BCENTRY * bcAddr)
{
  /*
    Replacement policy: SIMPLE REPLACE
    Collision: new block is trying to replace old block in cache
    Steps to Carry out:
    1. Remove entry in blockCache (The copied instructions)
    2. Remove log book(the original blockCache) entry
    Since a different startAddress means a different block of Code.
    There will be exactly 1 block in the block cache corresponding with this block
   */

#ifdef BLOCK_CACHE_DEBUG
  serial_putstring("blockCache: COLLISION!!!");
  serial_newline();
#endif
#ifdef DUMP_COLLISION_COUNTER
  collisionCounter++;
  if ((collisionCounter % 10) == 9)
  {
    serial_putint_nozeros(collisionCounter);
    serial_putstring(" ");
    if ((collisionCounter % 200) == 199)
    {
      serial_newline();
    }
  }
#endif
  removeBlockCopyCacheEntry(bcAddr[index].blockCopyCacheAddress,bcAddr[index].blockCopyCacheSize);
}
void removeBlockCopyCacheEntry(u32int blockCopyCacheAddress,u32int blockCopyCacheSize){
  //First we must check if we have to remove a coninuous block are that it is split up (if it was to close to the end
  GCONTXT * context = getGuestContext();
  u32int endOfBlock = (blockCopyCacheAddress+(blockCopyCacheSize<<2));//End of the block that has to be removed
  u32int lastUsableBlockCopyCacheAddress = context->blockCopyCacheEnd-4; //see comment next rule
  //Warning last adress of blockCopyCache is a backpointer and might not be erased therefore
  if( endOfBlock > lastUsableBlockCopyCacheAddress)
  {
    u32int difference = endOfBlock-lastUsableBlockCopyCacheAddress;
    memset((u32int *)blockCopyCacheAddress,0,(blockCopyCacheSize<<2)-(difference));
    memset((u32int *)context->blockCopyCache,0,difference); //The rest of the block is at the start of BlockCopyCache remove it there
  }
  else //safe to remove all at once
  {
    memset((u32int *)blockCopyCacheAddress,0,blockCopyCacheSize<<2);//blockCopyCacheSize is number of u32int entries
  }

}


void explodeCache(BCENTRY * bcache)
{
  serial_putstring("========EXPLODE!!!=========");
  serial_newline();

  int i = 0;

  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    if (bcache[i].valid == TRUE)
    {
      removeCacheEntry(bcache, i);
    }
  }
  for (i = 0; i < NUMBER_OF_BITMAPS; i++)
  {
    execBitMap[i] = 0;
  }
  //Now it is also possible to set last used line of blockCopyCache back to the word just before the blockCopyCache (this way the blockCopyCache
  //will start again from the beginning -> Not absolutely necessary however.
}

// finds block cache entries that include a given address, clears them
void validateCachePreChange(BCENTRY * bcache, u32int address)
{
  if (isBitmapSetForAddress(address))
  {
    u32int cacheIndex = 0;
    while( (cacheIndex = findEntryForAddress(bcache, address)) != -1 )
    {
      removeCacheEntry(bcache, cacheIndex);
    }
  }
}

// finds and clears block cache entries within the given address range
void validateCacheMultiPreChange(BCENTRY * bcache, u32int startAddress, u32int endAddress)
{
#ifdef BLOCK_CACHE_DEBUG
  serial_putstring("blockCache: validateCacheMultiPreChange start ");
  serial_putint(startAddress);
  serial_putstring(" end ");
  serial_putint(endAddress);
  serial_newline();
#endif
  u32int i;
  for (i = 0; i < BLOCK_CACHE_SIZE; i++)
  {
    if (bcache[i].valid == TRUE)
    {
      //We only care if the end address of the block falls inside the address validation range
      if ( (bcache[i].endAddress >= startAddress) && (bcache[i].endAddress <= endAddress) )
      {
        // cached entry falls within given address range
        removeCacheEntry(bcache, i);
      }
    }
  }
}


void dumpBlockCacheEntry(u32int index, BCENTRY * bcache)
{
  serial_putstring("BlockCache: entry #0x ");
  serial_putint_nozeros(index);
  serial_putstring(":");
  serial_newline();

  serial_putstring("BlockCache: startAddress = ");
  serial_putint(bcache[index].startAddress);
  serial_putstring(" endAddress = ");
  serial_putint(bcache[index].endAddress);
  serial_newline();

  serial_putstring("BlockCache: valid = ");
  serial_putint_nozeros(bcache[index].valid);
  serial_putstring("BlockCopyCacheAddress = ");
  serial_putint(bcache[index].blockCopyCacheAddress);
  serial_putstring("BlockCopyCache size = ");
  serial_putint(bcache[index].blockCopyCacheSize);
  serial_putstring(" EOBinstr = ");
  serial_putint(bcache[index].hyperedInstruction);
  serial_putstring(" hdlFunct = ");
  serial_putint(bcache[index].hdlFunct);
  serial_newline();
}

void setExecBitMap(u32int addr)
{
  u32int index = addr / MEMORY_PER_BITMAP; 
  u32int bitNumber = (addr & 0x0FFFFFFF) / MEMORY_PER_BITMAP_BIT;

  execBitMap[index] = execBitMap[index] | (1 << bitNumber);
}

void clearExecBitMap(u32int addr)
{
  u32int index = addr / MEMORY_PER_BITMAP; 
  u32int bitNumber = (addr & 0x0FFFFFFF) / MEMORY_PER_BITMAP_BIT;

  execBitMap[index] = execBitMap[index] & ~(1 << bitNumber);
}

bool isBitmapSetForAddress(u32int addr)
{
  u32int index = addr / MEMORY_PER_BITMAP; 
  u32int bitNumber = (addr & 0x0FFFFFFF) / MEMORY_PER_BITMAP_BIT;
  u32int bitResult = execBitMap[index] & (1 << bitNumber); 
  return (bitResult != 0);
}
/* This function will check if the content at Addr == 0x0 if not make it free -> the address that must be used is returned*/
u32int * checkAndClearBlockCopyCacheAddress(u32int *Addr,BCENTRY *bcStartAddr,u32int* blockCopyCache,u32int* blockCopyCacheEnd){
  if(Addr >= blockCopyCacheEnd){//Last address of BlockCopyCacheAddr
    //Continue at beginning of blockCopyCacheAddress
    Addr=blockCopyCache;
#ifdef BLOCK_COPY_CACHE_DEBUG
  serial_putstring("blockCopyCache: End exceeded continue at start=0x");
  serial_putint((u32int)Addr);
  serial_newline();
#endif
  }
  if(*Addr == 0x0){
    //Do nothing -> address is usable
    return Addr;
  }else{
#ifdef BLOCK_COPY_CACHE_DEBUG
  serial_putstring("blockCopyCache: The contents of 0x");
  serial_putint((u32int)Addr);
  serial_putstring(" differs from 0x0 -> clean ");
  serial_newline();
#endif
    //The address will be the first occurrence of a non 0x0 entry => the contents will be a backpointer to a log entry
    BCENTRY * bcEntry = (BCENTRY*)(*Addr);
    u32int index = bcEntry-bcStartAddr;
#ifdef BLOCK_COPY_CACHE_DEBUG
  serial_putstring("block with blockcache id=");
  serial_putint(index);
  serial_putstring(" gets cleaned");
  serial_newline();
#endif
    //Remove cacheEntry In both blockCache & blockCopyCache
    removeCacheEntry(bcStartAddr, index);
  }
  return Addr;//Return Address that should be used to place next instruction
}

u32int* updateCurrBlockCopyCacheAddr(u32int* oldAddr, u32int nrOfAddedInstr,u32int* blockCopyCacheEnd){
  oldAddr=oldAddr+nrOfAddedInstr;
  if(oldAddr >= blockCopyCacheEnd ){//oldAddr=currBlockCopyCacheAddress blockCopyCacheAddresses will be used in a  cyclic manner
                                    //-> if end of blockCopyCache is passed blockCopyCacheCurrAddress must be updated
    oldAddr=oldAddr - (BLOCK_COPY_CACHE_SIZE-1);
  }
  return oldAddr;
}

