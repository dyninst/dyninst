#include "arch-amdgpu.h"
#include "registerSpace/registerSpace.h"
#include "registerSpace/RegisterConversion.h"
#include "debug.h"
#include "dyn_register.h"
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
    // s[100:101] alias vcc (see MAX_ALLOCATABLE_SGPR_ID), so they are never handed out.
    registers.push_back(new registerSlot(id, name,
                                         /*offLimits =*/id > NS_amdgpu::MAX_ALLOCATABLE_SGPR_ID,
                                         registerSlot::liveAlways, registerSlot::SGPR));
  }

  // VGPRs
  for (unsigned id = 0; id <= 255; id++) {
    char name[32];
    sprintf(name, "v%u", id);
    registers.push_back(new registerSlot(NS_amdgpu::VGPR_OPERAND_BASE + id, name,
                                         /*offLimits =*/false, registerSlot::liveAlways,
                                         registerSlot::VGPR));
  }

  // SPRs: reserved special registers. Their codegen Register id (the ISA operand code) is derived from
  // the MachRegister via convertRegID, so the operand codes live in one place (the generated
  // specialRegs[] table) rather than being hand-listed here.
  // clang-format off
    registers.push_back(new registerSlot(convertRegID(amdgpu_gfx908::flat_scratch_lo), "flat_scratch_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(convertRegID(amdgpu_gfx908::flat_scratch_hi), "flat_scratch_hi", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(convertRegID(amdgpu_gfx908::xnack_mask_lo), "xnack_mask_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(convertRegID(amdgpu_gfx908::xnack_mask_hi), "xnack_mask_hi", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(convertRegID(amdgpu_gfx908::vcc_lo), "vcc_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(convertRegID(amdgpu_gfx908::vcc_hi), "vcc_hi", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(convertRegID(amdgpu_gfx908::exec_lo), "exec_lo", true, registerSlot::liveAlways, registerSlot::SPR));
    registers.push_back(new registerSlot(convertRegID(amdgpu_gfx908::exec_hi), "exec_hi", true, registerSlot::liveAlways, registerSlot::SPR));
  // clang-format on

  registerSpace::createRegisterSpace64(registers);
  done = true;
}

void registerSpace::initialize64() { assert(!"No 64-bit registers for AMDGPU"); }

void registerSpace::initialize() { initialize32(); }
