#if !defined(BPMAA_H)
#define BPMAA_H

#include "Visitor.h"
#include "Instruction.h"
#include "common/h/Types.h"

class BPatch_memoryAccess;

class BPatch_memoryAccessAdapter : public Dyninst::InstructionAPI::Visitor
{
 public:
  BPatch_memoryAccessAdapter() {
  }
  
  virtual ~BPatch_memoryAccessAdapter() {
  }
  
  BPatch_memoryAccess* convert(Dyninst::InstructionAPI::Instruction::Ptr insn,
			       Address current, bool is64);
  virtual void visit(Dyninst::InstructionAPI::BinaryFunction* b);
  virtual void visit(Dyninst::InstructionAPI::Dereference* d);
  virtual void visit(Dyninst::InstructionAPI::RegisterAST* r);
  virtual void visit(Dyninst::InstructionAPI::Immediate* i);
  

};

#endif // !defined(BPMAA_H)
