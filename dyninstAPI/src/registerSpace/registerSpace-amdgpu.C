#include "arch-amdgpu.h"
#include "registerSpace/registerSpace.h"
#include "registerSpace/RegisterConversion.h"
#include "debug.h"
#include "dyn_register.h"
#include "amdgpu-reg-adapter.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"
#include <vector>

using Register = Dyninst::Register;
using OperandRegId = Dyninst::OperandRegId;
using BlockSize = Dyninst::BlockSize;
using namespace Dyninst::amdgpu_adapter;

namespace {
// Build a gfx908 MachRegister from a codegen Register (number from the id, category from vector-ness, width
// from the dword count), naming only amdgpu_gfx908 -- this is the gfx908 register space. No macro, no arch
// switch. Under runtime codegen-arch dispatch this names the instrumented object's gen.
inline Dyninst::MachRegister mrOf(Dyninst::Register r) {
  const int32_t num = static_cast<int32_t>(r.getId() & 0xffu);
  const int32_t category = r.isVector() ? Dyninst::amdgpu_gfx908::VGPR : Dyninst::amdgpu_gfx908::SGPR;
  int32_t sizeBits = Dyninst::amdgpu_gfx908::BITS_32;
  switch (r.getCount()) {
  case 2: sizeBits = Dyninst::amdgpu_gfx908::BITS_64; break;
  case 4: sizeBits = Dyninst::amdgpu_gfx908::BITS_128; break;
  case 8: sizeBits = Dyninst::amdgpu_gfx908::BITS_256; break;
  case 16: sizeBits = Dyninst::amdgpu_gfx908::BITS_512; break;
  default: break; // 1 (and unexpected) -> BITS_32
  }
  return Dyninst::MachRegister(static_cast<int32_t>(Dyninst::Arch_amdgpu_gfx908) | category | sizeBits | num);
}
} // namespace

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
  Dyninst::MachRegister mr = mrOf(regBlock);
  bool scalarOrVector = amdgpuIsSgpr(mr) || amdgpuIsVgpr(mr);
  assert(scalarOrVector && "regBlock must be a scalar or vector block");

  for (auto reg : regBlock.getIndividualRegisters()) {
    freeRegister(reg);
  }
}
