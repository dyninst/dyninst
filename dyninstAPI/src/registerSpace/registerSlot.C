#include "registerSlot.h"
#include "registerSpace.h"
#include "dyn_register.h"


void registerSlot::cleanSlot() {
  // number does not change
  refCount = 0;
  // liveState = live;
  // liveState does not change
  keptValue = false;
  beenUsed = false;
  // initialState doesn't change
  // offLimits doesn't change
  spilledState = unspilled;
  saveOffset = 0;
  // type doesn't change
}

unsigned registerSlot::encoding() const {
  // Should write this for all platforms when the encoding is done.
#if defined(DYNINST_CODEGEN_ARCH_POWER)
  switch (type) {
  case GPR:
    return registerSpace::GPR(number);
    break;
  case FPR:
    return registerSpace::FPR(number);
    break;
  case SPR:
    return registerSpace::SPR(number);
    break;
  default:
    assert(0);
    return Dyninst::Null_Register;
    break;
  }
#elif defined(DYNINST_CODEGEN_ARCH_I386) || defined(DYNINST_CODEGEN_ARCH_X86_64)
  // Should do a mapping here from entire register space to "expected"
  // encodings.
  return number;
#elif defined(DYNINST_CODEGEN_ARCH_AARCH64)
  switch (type) {
  case GPR:
    return registerSpace::GPR(number);
    break;
  case FPR:
    return registerSpace::FPR(number);
    break;
  default:
    assert(0);
    return Dyninst::Null_Register;
    break;
  }
#elif defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
  switch (type) {
  case SGPR:
    return registerSpace::SGPR(number);
    break;
  case VGPR:
    return registerSpace::VGPR(number);
    break;
    /*      case AGPR:
                return registerSpace::AGPR(number);
                break;*/
  default:
    assert(0);
    return Dyninst::Null_Register;
    break;
  }
#else
  assert(0);
  return 0;
#endif
}
