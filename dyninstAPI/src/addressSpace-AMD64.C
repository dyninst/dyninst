#include "addressSpace.h"
#include "emit-x86.h"

namespace da = Dyninst::DyninstAPI;

Emitter *AddressSpace::getEmitter() {
  static EmitterAMD64Dyn emitter64Dyn;
  static EmitterAMD64Stat emitter64Stat;

  if (proc()) {
    return &emitter64Dyn;
  } else {
    assert(edit());
    return &emitter64Stat;
  }
}
