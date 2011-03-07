#!/bin/bash
#Blijkbaar werkt \( \)  niet bij insert command
#sed '
#/^u32int \([a-z]*\)Instruction(GCONTXT \* context)/ i\
#u32int \1PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)\
#{\
#  dumpGuestContext(context);\
#  DIE_NOW(0, "\1 PCFunct unfinished\\n");\
#  return 0;\
#}
#' </home/peter/Thesis_workingdir/hypervisor/hyparm/src/instructionEmu/miscInstructions_orig.c >/home/peter/Thesis_workingdir/hypervisor/hyparm/src/instructionEmu/miscInstructions.c
#%s/^u32int [a-z]*Instruction(GCONTXT \* context)/a/
pathInstructionEmu=/home/peter/Thesis_workingdir/hypervisor/hyparm/src/instructionEmu/
#files without extensions so that it is easy to add .c or _orig.c
filename[1]=miscInstructions
filename[2]=coprocInstructions
filename[3]=dataMoveInstr
filename[4]=dataProcessInstr

for (( i=1; i<=4; i++ ))
do
  bronbestand="$pathInstructionEmu${filename[i]}_orig.c"
  tempbestand="$pathInstructionEmu${filename[i]}temp.c"
  doelbestand="$pathInstructionEmu${filename[i]}.c"
  #echo "$bronbestand"
  #echo "$doelbestand"
  #Er is geen €-teken noch $-teken in het bestand met functies
  #For every function we should also match the { at the next line -> sed scans only 1 line at a time
  #just delete all the { corresponding with a function (easy since they are first char of line
  #And insert it wherever you want
  sed 's/^{//' <$bronbestand | sed 's/^u32int \([a-zA-Z0-9]*\)Instruction(GCONTXT \* context)/€$€u32int \1PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr)€{€  dumpGuestContext(context);€  DIE_NOW(0, "\1 PCFunct unfinished\\n");€  return 0;€}€$€&€{€  DIE_NOW(0, "\1Instruction is executed but not yet checked for blockCopyCompatibility");/g' |tr -s '€' '\012' |tr -d '$' >$tempbestand
  #For the files which have functions different from handle functions a { will be short
  #-> add it using the manually added tags
  sed 's_//blockCopyHelp_{_' <$tempbestand >$doelbestand
  rm $tempbestand
done
#AND THE HEADER FILES:
filename[1]=miscInstructions
filename[2]=coprocInstructions
filename[3]=dataMoveInstr
filename[4]=dataProcessInstr

for (( i=1; i<=4; i++ ))
do
  bronbestand="$pathInstructionEmu${filename[i]}_orig.h"
  doelbestand="$pathInstructionEmu${filename[i]}.h"
  #echo "$bronbestand"
  #echo "$doelbestand"
  #Er is geen €-teken noch $-teken in het bestand met functies
  sed 's/^u32int \([a-zA-Z0-9]*\)Instruction(GCONTXT \* context)/€$€u32int \1PCInstruction(GCONTXT * context, u32int *  instructionAddr, u32int * currBlockCopyCacheAddr);€&/g' <$bronbestand |tr -s '€' '\012' |tr -d '$' >$doelbestand
done

