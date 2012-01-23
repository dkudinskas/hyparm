
/*
void dumpMallocs()
{
  u32int i = 0;
  memchunkListElem * listPtr = chunkListRoot;
  printf("Dumping malloc internal structures:" EOL);
  printf("***********************************" EOL);
  for (i = 0; i < nrOfChunksAllocd; i++)
  {
    printf("Chunk %x: prev = %p; next = %p" EOL, i, listPtr->prevChunk, listPtr->nextChunk);
    printf("Start address: %#.8x; size %#x" EOL, listPtr->chunk.startAddress, listPtr->chunk.size);
    printf("-----------------------------------" EOL);
    listPtr = listPtr->nextChunk;
  }
}*/


