#include "addressSpace.h"
#include "codegen/emitters/x86/IA32/EmitterIA32Dyn.h"
#include "codegen/emitters/x86/IA32/EmitterIA32Stat.h"
#include "emit-x86.h"

namespace da = Dyninst::DyninstAPI;

Emitter *AddressSpace::getEmitter() {
  static EmitterAMD64Dyn emitter64Dyn;
  static EmitterAMD64Stat emitter64Stat;

  if(getAddressWidth() == 8) {
    if (proc()) {
      return &emitter64Dyn;
    } else {
      assert(edit());
      return &emitter64Stat;
    }
  }

  static da::EmitterIA32Dyn emitter32Dyn;
  static da::EmitterIA32Stat emitter32Stat;

  if (proc()) {
    return &emitter32Dyn;
  } else {
    assert(edit());
    return &emitter32Stat;
  }
}
