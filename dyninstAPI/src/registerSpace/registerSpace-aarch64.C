#include "registerSpace.h"
#include "arch-aarch64.h"

#include <cstdio>
#include <vector>

void registerSpace::initialize32() {
  assert(!"No 32-bit implementation for the ARM architecture!");
}

void registerSpace::initialize64() {
  static bool done = false;
  if (done)
    return;

  std::vector<registerSlot *> registers;

  // GPRs
  for (unsigned idx = r0; idx <= r28; idx++) {
    char name[32];
    sprintf(name, "r%u", idx - r0);
    registers.push_back(new registerSlot(
        idx, name, false, registerSlot::liveAlways, registerSlot::GPR));
  }
  // Mark r29 (frame pointer) and r30 (link register) as off-limits
  registers.push_back(new registerSlot(
      r29, "r29", true, registerSlot::liveAlways, registerSlot::GPR));
  registers.push_back(new registerSlot(
      r30, "r30", true, registerSlot::liveAlways, registerSlot::GPR));

  // SPRs
  registers.push_back(new registerSlot(lr, "lr", true, registerSlot::liveAlways,
                                       registerSlot::SPR));
  registers.push_back(new registerSlot(sp, "sp", true, registerSlot::liveAlways,
                                       registerSlot::SPR));
  registers.push_back(new registerSlot(
      nzcv, "nzcv", true, registerSlot::liveAlways, registerSlot::SPR));
  registers.push_back(new registerSlot(
      fpcr, "fpcr", true, registerSlot::liveAlways, registerSlot::SPR));
  registers.push_back(new registerSlot(
      fpsr, "fpsr", true, registerSlot::liveAlways, registerSlot::SPR));

  // FPRs
  for (unsigned idx = fpr0; idx <= fpr31; idx++) {
    char name[32];
    sprintf(name, "fpr%u", idx - fpr0);
    // TODO mov SP to FP
    registers.push_back(new registerSlot(
        idx, name, false, registerSlot::liveAlways, registerSlot::FPR));
  }

  registerSpace::createRegisterSpace64(registers);
  done = true;
}

void registerSpace::initialize() { initialize64(); }
