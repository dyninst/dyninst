#include "ASTs/codeGenAST.h"
#include "IAPI_to_AST.h"
#include "Instruction.h"
#include "Register.h"
#include "addressSpace.h"
#include "debug.h"
#include "registerSpace/registerSpace.h"

#include <vector>

namespace di = Dyninst::InstructionAPI;
namespace da = Dyninst::DyninstAPI;

using codeGenASTPtr = da::codeGenASTPtr;
using operandAST = da::operandAST;

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
