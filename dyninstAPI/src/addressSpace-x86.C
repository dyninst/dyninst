#include "ASTs/codeGenAST.h"
#include "Expression.h"
#include "Instruction.h"
#include "Register.h"
#include "registerSpace/RegisterConversion.h"
#include "Visitor.h"
#include "addressSpace.h"
#include "debug.h"
#include "registerSpace/registerSpace.h"

#include <deque>
#include <vector>

namespace di = Dyninst::InstructionAPI;
namespace da = Dyninst::DyninstAPI;

using codeGenASTPtr = da::codeGenASTPtr;
using operandAST = da::operandAST;
using operatorAST = da::operatorAST;

class ASTFactory : public di::Visitor {
public:
  void visit(di::BinaryFunction *b) override;
  void visit(di::Dereference *d) override;
  void visit(di::Immediate *i) override;
  void visit(di::RegisterAST *r) override;
  void visit(di::MultiRegisterAST *) override {}

  std::deque<codeGenASTPtr> m_stack;
};

// First AST node: target of the call
// Second AST node: source of the call
// This can handle indirect control transfers as well
bool AddressSpace::getDynamicCallSiteArgs(InstructionAPI::Instruction insn, Address addr,
                                          std::vector<codeGenASTPtr> &args) {
  using namespace Dyninst::InstructionAPI;
  Expression::Ptr cft = insn.getControlFlowTarget();
  ASTFactory f;
  cft->apply(&f);
  assert(f.m_stack.size() == 1);
  args.push_back(f.m_stack[0]);
  args.push_back(operandAST::Constant((void *)addr));
  inst_printf("%s[%d]:  Inserting dynamic call site instrumentation for %s\n", FILE__,
              __LINE__, cft->format(insn.getArch()).c_str());
  return true;
}

void ASTFactory::visit(di::BinaryFunction *b) {
  codeGenASTPtr rhs = m_stack.back();
  m_stack.pop_back();
  codeGenASTPtr lhs = m_stack.back();
  m_stack.pop_back();

  if (b->isAdd()) {
    m_stack.push_back(operatorAST::plus(lhs, rhs));
  } else if (b->isMultiply()) {
    m_stack.push_back(operatorAST::times(lhs, rhs));
  } else {
    assert(0);
  }
}

void ASTFactory::visit(di::Dereference *) {
  codeGenASTPtr effaddr = m_stack.back();
  m_stack.pop_back();
  m_stack.push_back(operandAST::DataIndir(effaddr));
}

void ASTFactory::visit(di::Immediate *i) {
  m_stack.push_back(operandAST::Constant((void *)(i->eval().convert<long>())));
}

void ASTFactory::visit(di::RegisterAST *r) {
  m_stack.push_back(operandAST::origRegister((void *)(intptr_t)(convertRegID(r))));
}
