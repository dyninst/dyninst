#include "ASTs/codeGenAST.h"
#include "Instruction.h"
#include "Register.h"
#include "registerSpace/RegisterConversion.h"
#include "addressSpace.h"
#include "codegen/emitters/aarch64/EmitterAarch64Dyn.h"
#include "codegen/emitters/aarch64/EmitterAarch64Stat.h"
#include "registerSpace/registerSpace.h"

#include <vector>

namespace di = Dyninst::InstructionAPI;
namespace da = Dyninst::DyninstAPI;

using codeGenASTPtr = da::codeGenASTPtr;
using operandAST = da::operandAST;

// First AST node: target of the call
// Second AST node: source of the call
// This can handle indirect control transfers as well
bool AddressSpace::getDynamicCallSiteArgs(di::Instruction i, Address addr,
                                          std::vector<codeGenASTPtr> &args) {

  auto cft = i.getControlFlowTarget();
  auto target_reg = boost::dynamic_pointer_cast<di::RegisterAST>(cft);
  if (!target_reg)
    return false;
  auto branch_target = convertRegID(target_reg);

  if (branch_target == registerSpace::ignored)
    return false;

  // jumping to Xn (BLR Xn)
  args.push_back(operandAST::origRegister((void *)(long)branch_target));
  args.push_back(operandAST::Constant((void *)addr));

  return true;
}

Emitter *AddressSpace::getEmitter() {
  static da::EmitterAarch64Stat emitter64Stat;
  static da::EmitterAarch64Dyn emitter64Dyn;

  if (proc())
    return &emitter64Dyn;

  return &emitter64Stat;
}
