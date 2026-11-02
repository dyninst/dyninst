#include "ASTs/codeGenAST.h"
#include "Architecture.h"
#include "Instruction.h"
#include "addressSpace.h"
#include "codegen/emitters/amdgpu/gfx908/emit-amdgpu-gfx908.h"
#include "codegen/emitters/amdgpu/gfx90a/emit-amdgpu-gfx90a.h"
#include "codegen/emitters/amdgpu/gfx940/emit-amdgpu-gfx940.h"

#include <vector>

namespace di = Dyninst::InstructionAPI;
namespace da = Dyninst::DyninstAPI;

using codeGenASTPtr = da::codeGenASTPtr;

bool AddressSpace::getDynamicCallSiteArgs(di::Instruction, Address,
                                          std::vector<codeGenASTPtr> &) {
  assert(!"Not implemented for AMDGPU");
  return false;
}

// Runtime emitter selection: pick the concrete per-gen emitter by the instrumented object's architecture.
// Each gen's emitter is compiled in; this is the ONE place the gen is chosen (everything downstream is
// virtual dispatch on EmitterAmdgpu). Adding a gen = one case + compiling its emitter.
Emitter *AddressSpace::getEmitter() {
  static EmitterAmdgpuGfx908 gfx908Emitter;
  static EmitterAmdgpuGfx90a gfx90aEmitter;
  static EmitterAmdgpuGfx940 gfx940Emitter;
  assert(Dyninst::isArchitectureAMDGPU(getArch()) && "getEmitter: non-AMDGPU address space");
  switch (getArch()) {
  case Dyninst::Arch_amdgpu_gfx908: return &gfx908Emitter;
  case Dyninst::Arch_amdgpu_gfx90a: return &gfx90aEmitter;
  case Dyninst::Arch_amdgpu_gfx940: return &gfx940Emitter;
  default:
    assert(!"getEmitter: no emitter built for this AMDGPU generation (gfx908/gfx90a/gfx940)");
    return nullptr;
  }
}
