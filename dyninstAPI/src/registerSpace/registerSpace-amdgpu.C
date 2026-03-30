#include "arch-amdgpu.h"
#include "registerSpace.h"
#include "debug.h"

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

Dyninst::Register registerSpace::allocateGprBlock(Dyninst::RegKind regKind, uint32_t numRegs,
                                                  uint32_t alignment) {
  uint32_t minGprId = 0;
  uint32_t maxGprId = 0;

  switch(regKind) {
    case RegKind::SCALAR:
      minGprId = NS_amdgpu::MIN_SGPR_ID;
      maxGprId = NS_amdgpu::MAX_SGPR_ID;
      break;
    case RegKind::VECTOR:
      minGprId = NS_amdgpu::MIN_VGPR_ID;
      maxGprId = NS_amdgpu::MAX_VGPR_ID;
      break;
    default:
      regalloc_printf("regKind can't be allocated\n");
      assert(0);
      return Null_Register;
  }

  assert(minGprId % alignment == 0);

  for (uint32_t id = minGprId; id + numRegs - 1 <= maxGprId; id += alignment) {
    // Check whether the single individual consecutive registers can be allocated
    bool canAllocateBlock = true;
    for (uint32_t currentId = id; currentId < id + numRegs; ++currentId) {
      Dyninst::Register singleReg(OperandRegId(currentId), regKind, BlockSize(1));
      canAllocateBlock &= canAllocate(singleReg);
    }

    if (canAllocateBlock) {
      // Allocate those individual registers
      for (uint32_t currentId = id; currentId < id + numRegs; ++currentId) {
        Dyninst::Register reg(OperandRegId(currentId), regKind, BlockSize(1));
        auto *regSlot = this->registers_[reg];
        regSlot->markUsed(true);
        regSlot->refCount = 1;

        const char *regIdPrefix = regKind == RegKind::SCALAR ? "s" : "v";
        regalloc_printf("Allocated register %s%u\n", regIdPrefix, currentId);
      }
      return Register(OperandRegId(id), regKind, BlockSize(numRegs));
    }
  }
  return Null_Register;
}


void registerSpace::freeGprBlock(Dyninst::Register regBlock) {
  bool scalarOrVector = regBlock.getKind() == Dyninst::RegKind::SCALAR || regBlock.getKind() == Dyninst::RegKind::VECTOR;
  assert(scalarOrVector && "regBlock must be a scalar or vector block");

  std::vector<Dyninst::Register> individualRegs = regBlock.getIndividualRegisters();
  for (auto reg : individualRegs) {
    freeRegister(reg);
  }
}
