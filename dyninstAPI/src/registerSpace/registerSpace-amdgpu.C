#include "arch-amdgpu.h"
#include "registerSpace.h"

#include <vector>

void registerSpace::initialize32() {
  static bool done = false;
  if (done)
    return;

  std::vector<registerSlot *> registers;

  // TODO: This initialization doesn't consider all registers. Right now only the registers we typically
  // use for instrumentation are considered. This might need work later on.

  // SGPRs
  for (unsigned id = NS_amdgpu::MIN_SGPR_ID; id <= NS_amdgpu::MAX_SGPR_ID; id++) {
    char name[32];
    sprintf(name, "s%u", id);
    Register sgpr = Register::makeScalarRegister(OperandRegId(id), BlockSize(1));
    registers.push_back(new registerSlot(sgpr, name,
                                         /*offLimits =*/false, registerSlot::liveAlways,
                                         registerSlot::SGPR));
  }

  // VGPRs
  for (unsigned id = 0; id <= 255; id++) {
    char name[32];
    sprintf(name, "v%u", id);
    Register vgpr = Register::makeVectorRegister(OperandRegId(id), BlockSize(1));
    registers.push_back(new registerSlot(vgpr, name,
                                         /*offLimits =*/false, registerSlot::liveAlways,
                                         registerSlot::VGPR));
  }

  // clang-format off
    // SPRs
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::flat_scratch_lo, "flat_scratch_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::flat_scratch_hi, "flat_scratch_hi", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::xnack_mask_lo, "xnack_mask_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::xnack_mask_hi, "xnack_mask_hi", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::vcc_lo, "vcc_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::vcc_hi, "vcc_hi", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::exec_lo, "exec_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(NS_amdgpu::RegisterConstants::exec_hi, "exec_hi", true, registerSlot::liveAlways, registerSlot::SPR));
  // clang-format on

  registerSpace::createRegisterSpace64(registers);
  done = true;
}

void registerSpace::initialize64() { assert(!"No 64-bit registers for AMDGPU"); }

void registerSpace::initialize() { initialize32(); }
