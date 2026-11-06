#include "arch-amdgpu.h"
#include "registerSpace/registerSpace.h"
#include "registerSpace/RegisterConversion.h"
#include "debug.h"
#include "dyn_register.h"
#include "amdgpu-reg-adapter.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"
#include "registers/AMDGPU/amdgpu_gfx90a_regs.h"
#include "registers/AMDGPU/amdgpu_gfx940_regs.h"
#include "registers/AMDGPU/amdgpu_gfx950_regs.h"
#include <vector>

using Register = Dyninst::Register;
using OperandRegId = Dyninst::OperandRegId;
using BlockSize = Dyninst::BlockSize;
using namespace Dyninst::amdgpu_adapter;

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
registerSpace *registerSpace::buildAmdgpuRegisterSpace(Dyninst::Architecture arch) {
  std::vector<registerSlot *> registers;

  // TODO: This initialization doesn't consider all registers. Right now only the registers we typically
  // use for instrumentation are considered. This might need work later on.

  // SGPRs. GPR id ranges are gfx9-common (NS_amdgpu), so the SGPR slots are the same for every gen.
  for (unsigned id = NS_amdgpu::MIN_SGPR_ID; id <= NS_amdgpu::MAX_SGPR_ID; id++) {
    char name[32];
    sprintf(name, "s%u", id);
    Register sgpr = Register::makeScalarRegister(OperandRegId(id), BlockSize(1));
    registers.push_back(new registerSlot(sgpr, name,
                                         /*offLimits =*/false, registerSlot::liveAlways,
                                         registerSlot::SGPR));
  }

  // VGPRs. Also gfx9-common.
  for (unsigned id = 0; id <= 255; id++) {
    char name[32];
    sprintf(name, "v%u", id);
    Register vgpr = Register::makeVectorRegister(OperandRegId(id), BlockSize(1));
    registers.push_back(new registerSlot(vgpr, name,
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

Dyninst::Register registerSpace::allocateGprBlock(Dyninst::RegKind regKind, uint32_t numRegs,
                                                  Dyninst::Alignment alignment) {
  uint32_t minGprId = 0;
  uint32_t maxGprId = 0;

  switch(regKind) {
    case Dyninst::RegKind::SCALAR:
      minGprId = NS_amdgpu::MIN_SGPR_ID;
      maxGprId = NS_amdgpu::MAX_ALLOCATABLE_SGPR_ID;
      break;
    case Dyninst::RegKind::VECTOR:
      minGprId = NS_amdgpu::MIN_VGPR_ID;
      maxGprId = NS_amdgpu::MAX_VGPR_ID;
      break;
    default:
      regalloc_printf("regKind can't be allocated\n");
      assert(0);
      return Dyninst::Null_Register;
  }

  uint32_t alignmentValue{alignment.getValue()};
  assert(minGprId % alignmentValue == 0);

  // Check whether the single individual consecutive registers can be allocated
  auto nextId{minGprId};  // next register to test
  auto baseId{nextId};    // alignment aligned base id of register block

  while (nextId + numRegs - 1 <= maxGprId) {
    Dyninst::Register singleReg(OperandRegId(nextId), regKind, BlockSize(1));
    if (canAllocate(singleReg)) {
      if (nextId - baseId + 1 == numRegs) {
        break;            // found enough registers: [baseId, nextId]
      }
      ++nextId;
    } else {              // failed, begin at next alignment
      baseId = ((nextId + alignmentValue) / alignmentValue) * alignmentValue;
      nextId = baseId;
    }
  }

  if (nextId - baseId + 1 != numRegs) {
    return Dyninst::Null_Register; // not enough registers, fail
  }

  for (auto allocId = baseId; allocId <= nextId; ++allocId) {
    Dyninst::Register reg(OperandRegId(allocId), regKind, BlockSize(1));
    auto *regSlot = this->registers_[reg];
    regSlot->markUsed(true);
    regSlot->refCount = 1;

    const char *regIdPrefix = regKind == Dyninst::RegKind::SCALAR ? "s" : "v";
    regalloc_printf("Allocated register %s%u\n", regIdPrefix, baseId);
  }

  return Register(OperandRegId(baseId), regKind, BlockSize(numRegs));
}

void registerSpace::freeGprBlock(Dyninst::Register regBlock) {
  // Blocks returned by allocateGprBlock are SGPR or VGPR by construction.
  for (auto reg : regBlock.getIndividualRegisters()) {
    freeRegister(reg);
  }
}
