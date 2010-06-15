
#include "SymbolicExpansion.h"

using namespace Dyninst;
using namespace SymbolicEvaluation;

bool SymbolicExpansionX86::expand(SageInstruction_t *rose_insn,
				  SymEvalPolicy &policy) {
  SgAsmx86Instruction *insn = static_cast<SgAsmx86Instruction *>(rose_insn);
  
  X86InstructionSemantics<SymEval
