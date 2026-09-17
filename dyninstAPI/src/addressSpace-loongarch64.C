#include "ASTs/codeGenAST.h"
#include "Instruction.h"
#include "Register.h"
#include "registerSpace/RegisterConversion.h"
#include "addressSpace.h"
#include "registerSpace/registerSpace.h"

#include <vector>

namespace di = Dyninst::InstructionAPI;
namespace da = Dyninst::DyninstAPI;

using codeGenASTPtr = da::codeGenASTPtr;

bool AddressSpace::getDynamicCallSiteArgs(di::Instruction, Address,
                                          std::vector<codeGenASTPtr> &) {
  return false;
}

Emitter *AddressSpace::getEmitter() {
  assert(0 && "getEmitter not implemented for loongarch64");
  return nullptr;
}
