#ifndef __INSTRUCTION_EMU__INTERPRETER__INTERPRETER_H__
#define __INSTRUCTION_EMU__INTERPRETER__INTERPRETER_H__

#include "instructionEmu/interpreter/common.h"

#include "instructionEmu/interpreter/arm/branchInstructions.h"
#include "instructionEmu/interpreter/arm/coprocInstructions.h"
#include "instructionEmu/interpreter/arm/dataProcessInstructions.h"
#include "instructionEmu/interpreter/arm/loadInstructions.h"
#include "instructionEmu/interpreter/arm/miscInstructions.h"
#include "instructionEmu/interpreter/arm/miscMediaInstructions.h"
#include "instructionEmu/interpreter/arm/multiplyInstructions.h"
#include "instructionEmu/interpreter/arm/parallelAddSubInstructions.h"
#include "instructionEmu/interpreter/arm/saturatingAddSubInstructions.h"
#include "instructionEmu/interpreter/arm/storeInstructions.h"
#include "instructionEmu/interpreter/arm/syncInstructions.h"


#ifdef CONFIG_BLOCK_COPY

#include "instructionEmu/interpreter/arm/branchPCInstructions.h"
#include "instructionEmu/interpreter/arm/coprocPCInstructions.h"
#include "instructionEmu/interpreter/arm/dataProcessPCInstructions.h"
#include "instructionEmu/interpreter/arm/loadPCInstructions.h"
#include "instructionEmu/interpreter/arm/miscPCInstructions.h"
#include "instructionEmu/interpreter/arm/miscMediaPCInstructions.h"
#include "instructionEmu/interpreter/arm/multiplyPCInstructions.h"
#include "instructionEmu/interpreter/arm/parallelAddSubPCInstructions.h"
#include "instructionEmu/interpreter/arm/saturatingAddSubPCInstructions.h"
#include "instructionEmu/interpreter/arm/storePCInstructions.h"
#include "instructionEmu/interpreter/arm/syncPCInstructions.h"

#endif /* CONFIG_BLOCK_COPY */


#ifdef CONFIG_THUMB2

#include "instructionEmu/interpreter/t16/branchInstructions.h"
#include "instructionEmu/interpreter/t16/dataProcessInstructions.h"
#include "instructionEmu/interpreter/t16/loadInstructions.h"
#include "instructionEmu/interpreter/t16/miscInstructions.h"
#include "instructionEmu/interpreter/t16/storeInstructions.h"

#include "instructionEmu/interpreter/t32/branchInstructions.h"
#include "instructionEmu/interpreter/t32/dataProcessInstructions.h"
#include "instructionEmu/interpreter/t32/loadInstructions.h"
#include "instructionEmu/interpreter/t32/miscInstructions.h"
#include "instructionEmu/interpreter/t32/multiplyInstructions.h"
#include "instructionEmu/interpreter/t32/storeInstructions.h"

#endif /* CONFIG_THUMB2 */

#endif /* __INSTRUCTION_EMU__INTERPRETER__INTERPRETER_H__ */
