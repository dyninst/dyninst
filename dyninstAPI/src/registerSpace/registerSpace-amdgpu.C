#include "arch-amdgpu.h"
#include "registerSpace/registerSpace.h"
#include "registerSpace/RegisterConversion.h"
#include "debug.h"
#include "dyn_register.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"
#include "registers/AMDGPU/amdgpu_gfx90a_regs.h"
#include "registers/AMDGPU/amdgpu_gfx940_regs.h"
#include "registers/AMDGPU/amdgpu_gfx950_regs.h"
#include <vector>

using Register = Dyninst::Register;

// SPRs: reserved special registers. Their codegen Register id (the ISA operand code) is derived from
// the per-gen MachRegister via convertRegID (which reads the register's own arch), so a gfx940
// MachRegister yields gfx940 operand codes. The operand codes live in the generated specialRegs[]
// table rather than being hand-listed here; this macro only names the slots once per generation.
#define DYNINST_AMDGPU_PUSH_SPRS(NS)                                                                      \
  do {                                                                                                    \
    registers.push_back(new registerSlot(convertRegID(NS::flat_scratch_lo), "flat_scratch_lo", true, registerSlot::liveAlways, registerSlot::SPR)); \
    registers.push_back(new registerSlot(convertRegID(NS::flat_scratch_hi), "flat_scratch_hi", true, registerSlot::liveAlways, registerSlot::SPR)); \
    registers.push_back(new registerSlot(convertRegID(NS::xnack_mask_lo), "xnack_mask_lo", true, registerSlot::liveAlways, registerSlot::SPR)); \
    registers.push_back(new registerSlot(convertRegID(NS::xnack_mask_hi), "xnack_mask_hi", true, registerSlot::liveAlways, registerSlot::SPR)); \
    registers.push_back(new registerSlot(convertRegID(NS::vcc_lo), "vcc_lo", true, registerSlot::liveAlways, registerSlot::SPR)); \
    registers.push_back(new registerSlot(convertRegID(NS::vcc_hi), "vcc_hi", true, registerSlot::liveAlways, registerSlot::SPR)); \
    registers.push_back(new registerSlot(convertRegID(NS::exec_lo), "exec_lo", true, registerSlot::liveAlways, registerSlot::SPR)); \
    registers.push_back(new registerSlot(convertRegID(NS::exec_hi), "exec_hi", true, registerSlot::liveAlways, registerSlot::SPR)); \
  } while (0)

// Build the register space for one AMDGPU generation. GPR id ranges are
// gfx9-common (NS_amdgpu) so the SGPR/VGPR slots are identical for every gen;
// only the SPRs are picked per-gen. The caller (getRegisterSpaceForArch) owns
// caching, so this just builds and returns a fresh space.
//
// A codegen Register is the ISA operand code: SGPR n is n, VGPR n is VGPR_OPERAND_BASE + n,
// and a special register is convertRegID(MachRegister). s[100:101] alias vcc
// (see MAX_ALLOCATABLE_SGPR_ID) and are marked off limits so they are never handed out.
registerSpace *registerSpace::buildAmdgpuRegisterSpace(Dyninst::Architecture arch) {
  std::vector<registerSlot *> registers;

  // TODO: This initialization doesn't consider all registers. Right now only the registers we typically
  // use for instrumentation are considered. This might need work later on.

  // SGPRs. GPR id ranges are gfx9-common (NS_amdgpu), so the SGPR slots are the same for every gen.
  for (unsigned id = NS_amdgpu::MIN_SGPR_ID; id <= NS_amdgpu::MAX_SGPR_ID; id++) {
    char name[32];
    sprintf(name, "s%u", id);
    registers.push_back(new registerSlot(id, name,
                                         /*offLimits =*/id > NS_amdgpu::MAX_ALLOCATABLE_SGPR_ID,
                                         registerSlot::liveAlways, registerSlot::SGPR));
  }

  // VGPRs. Also gfx9-common.
  for (unsigned id = 0; id <= 255; id++) {
    char name[32];
    sprintf(name, "v%u", id);
    registers.push_back(new registerSlot(NS_amdgpu::VGPR_OPERAND_BASE + id, name,
                                         /*offLimits =*/false, registerSlot::liveAlways,
                                         registerSlot::VGPR));
  }

  // SPRs: pick the generation's MachRegister namespace so the operand codes match the target gen.
  // clang-format off
  switch (arch) {
    case Dyninst::Arch_amdgpu_gfx908: DYNINST_AMDGPU_PUSH_SPRS(Dyninst::amdgpu_gfx908); break;
    case Dyninst::Arch_amdgpu_gfx90a: DYNINST_AMDGPU_PUSH_SPRS(Dyninst::amdgpu_gfx90a); break;
    case Dyninst::Arch_amdgpu_gfx940: DYNINST_AMDGPU_PUSH_SPRS(Dyninst::amdgpu_gfx940); break;
    case Dyninst::Arch_amdgpu_gfx950: DYNINST_AMDGPU_PUSH_SPRS(Dyninst::amdgpu_gfx950); break;
    default: assert(!"registerSpace::buildAmdgpuRegisterSpace: unsupported AMDGPU architecture");
  }
  // clang-format on

  registerSpace *rs = new registerSpace();
  rs->addr_width = 8;
  createRegSpaceInt(registers, rs);
  return rs;
}

#undef DYNINST_AMDGPU_PUSH_SPRS

// The shared contract keeps a no-arg initialize32(); on AMDGPU the per-gen spaces
// are built lazily through getRegisterSpaceForArch, so this just materializes the
// baseline (gfx908) generation.
void registerSpace::initialize32() { getRegisterSpaceForArch(Dyninst::Arch_amdgpu_gfx908); }

void registerSpace::initialize64() { assert(!"No 64-bit registers for AMDGPU"); }

void registerSpace::initialize() { initialize32(); }
