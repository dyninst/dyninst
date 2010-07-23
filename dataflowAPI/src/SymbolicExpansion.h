#if !defined(_SYMBOLIC_EXPANSION_H_)
#define _SYMBOLIC_EXPANSION_H_

class SgAsmInstruction;

namespace Dyninst {
namespace DataflowAPI {

class SymEvalPolicy;

 class SymbolicExpansion {
 public:
  SymbolicExpansion() {};
  ~SymbolicExpansion() {};
  
  static bool expandX86(SgAsmInstruction *rose_insn,
			SymEvalPolicy &policy);
  static bool expandPPC(SgAsmInstruction *rose_insn,
			SymEvalPolicy &policy);
  
};

};
};
#endif
