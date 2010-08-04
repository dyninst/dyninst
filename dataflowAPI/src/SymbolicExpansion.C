
#include "SymbolicExpansion.h"
#include "SymEvalPolicy.h"

#include "../rose/SgAsmInstruction.h"
#include "../rose/SgAsmPowerpcInstruction.h"
#include "../rose/SgAsmx86Instruction.h"

#include "../rose/x86InstructionSemantics.h"
#include "../rose/powerpcInstructionSemantics.h"


using namespace Dyninst;
using namespace DataflowAPI;

bool SymbolicExpansion::expandX86(SgAsmInstruction *rose_insn,
				  SymEvalPolicy &policy) {
  SgAsmx86Instruction *insn = static_cast<SgAsmx86Instruction *>(rose_insn);
  
  X86InstructionSemantics<SymEvalPolicy, Handle> t(policy);
  t.processInstruction(insn);
  return true;
}

bool SymbolicExpansion::expandPPC(SgAsmInstruction *rose_insn,
				  SymEvalPolicy &policy) {
  SgAsmPowerpcInstruction *insn = static_cast<SgAsmPowerpcInstruction *>(rose_insn);
  
  PowerpcInstructionSemantics<SymEvalPolicy, Handle> t(policy);
  t.processInstruction(insn);
  return true;
}

