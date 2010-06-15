#if !defined(_SYMBOLIC_EXPANSION_H_)
#define _SYMBOLIC_EXPANSION_H_

class SageInstruction_t;

namespace Dyninst {
namespace SymbolicEvaluation {

class SymEvalPolicy;

class SymbolicExpansion {
 public:
  SymbolicExpansion() {};
  ~SymbolicExpansion() {};

  static virtual bool expand(SageInstruction_t *rose_insn,
			     SymEvalPolicy &policy) { return false; }
};

class SymbolicExpansionX86 : public SymbolicExpansion {
 public:
  SymbolicExpansionX86() {};
  ~SymbolicExpansionX86() {};
  
  static virtual bool expand(SageInstruction_t *rose_insn,
			     SymEvalPolicy &policy);
};

class SymbolicExpansionPPC : public SymbolicExpansion {
 public:
  SymbolicExpansionPPC() {};
  ~SymbolicExpansionPPC() {};
  
  static virtual bool expand(SageInstruction_t *rose_insn,
			     SymEvalPolicy &policy);
};

};
};
#endif
