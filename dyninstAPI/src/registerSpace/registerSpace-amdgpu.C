#include "arch-amdgpu.h"
#include "registerSpace.h"

#include <vector>

void registerSpace::initialize32() {
  using namespace NS_amdgpu;
  static bool done = false;
  if (done)
    return;

  std::vector<registerSlot *> registers;

  // TODO: This initialization doesn't consider all registers. Right now only
  // the registers we typically use for instrumentation are considered. This
  // might need work later on.

  // SGPRs
  for (unsigned idx = sgpr0; idx <= sgpr101; idx++) {
    char name[32];
    sprintf(name, "sgpr%u", idx - sgpr0);
    registers.push_back(new registerSlot(
        idx, name,
        /*offLimits =*/false, registerSlot::liveAlways, registerSlot::SGPR));
  }

  // VGPRs
  for (unsigned idx = v0; idx <= v255; idx++) {
    char name[32];
    sprintf(name, "vgpr%u", idx - v0);
    registers.push_back(new registerSlot(
        idx, name,
        /*offLimits =*/false, registerSlot::liveAlways, registerSlot::VGPR));
  }

  // clang-format off
    // SPRs
    registers.push_back(new registerSlot(flat_scratch_lo, "flat_scratch_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(flat_scratch_lo, "flat_scratch_hi", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(xnack_mask_lo, "xnack_mask_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(xnack_mask_lo, "xnack_mask_hi", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(vcc_lo, "vcc_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(vcc_lo, "vcc_hi", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(exec_lo, "exec_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(exec_lo, "exec_hi", true, registerSlot::liveAlways, registerSlot::SPR));
  // clang-format on

  registerSpace::createRegisterSpace64(registers);
  done = true;
}

void registerSpace::initialize64() {
  assert(!"No 64-bit registers for AMDGPU");
}

void registerSpace::initialize() { initialize32(); }
