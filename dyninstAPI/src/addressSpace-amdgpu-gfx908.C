#include "ASTs/codeGenAST.h"
#include "Instruction.h"
#include "addressSpace.h"
#include "emit-amdgpu.h"

#include <vector>

namespace di = Dyninst::InstructionAPI;
namespace da = Dyninst::DyninstAPI;

using codeGenASTPtr = da::codeGenASTPtr;

bool AddressSpace::getDynamicCallSiteArgs(di::Instruction, Address,
                                          std::vector<codeGenASTPtr> &) {
  assert(!"Not implemented for AMDGPU");
  return false;
}

Emitter *AddressSpace::getEmitter() {
  static EmitterAmdgpuGfx908 gfx908Emitter;
  return &gfx908Emitter;
}
