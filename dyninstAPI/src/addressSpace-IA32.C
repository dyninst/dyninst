#include "addressSpace.h"
#include "codegen/emitters/x86/IA32/EmitterIA32Dyn.h"
#include "codegen/emitters/x86/IA32/EmitterIA32Stat.h"

namespace da = Dyninst::DyninstAPI;

Emitter *AddressSpace::getEmitter() {
  static da::EmitterIA32Dyn emitter32Dyn;
  static da::EmitterIA32Stat emitter32Stat;

  if (proc()) {
    return &emitter32Dyn;
  } else {
    assert(edit());
    return &emitter32Stat;
  }
}
