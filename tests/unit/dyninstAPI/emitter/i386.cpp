#include "Architecture.h"
#include "common/src/arch-regs-x86.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/emit-x86.h"
#include "registerSpace/registerSpace.h"
#include "emitter_test.h"

int main() {
  using Dyninst::verify_emitter;
  using Dyninst::emitter_buffer_t;

  constexpr auto arch = Dyninst::Arch_x86;
  auto const size = Dyninst::getArchAddressWidth(arch);
  auto *rs = registerSpace::getRegisterSpace(size);

  // Unlike the 64-bit emitter, the 32-bit one operates on *virtual* registers
  // that the register space maps onto the eight real registers on demand. A
  // register space is normally specialized from an instrumentation point's
  // liveness before use; there is no public entry point for a bare 32-bit
  // space, so mark the virtual registers dead (i.e., allocatable) directly.
  for(auto *slot : rs->GPRs()) {
    slot->liveState = registerSlot::dead;
  }

  BinaryEdit bin_edit{};

  // See x86_64.cpp: 128 bytes is plenty for any single operation.
  constexpr auto buffer_size = 128;

  codeGen gen(buffer_size);
  gen.setAddrSpace(&bin_edit);
  gen.setRegisterSpace(rs);

  // With no binary loaded the address space hands out the host's emitter, so
  // construct the 32-bit one directly.
  Dyninst::DyninstAPI::EmitterIA32Stat emitter{};

  bool failed = false;

  // Each case allocates its operands and pins them to a real register the way
  // a preceding snippet operation would have left them, then releases them
  // since verify_emitter does not reset the real-register map.

  // Signed divide with the dividend already in eax: cdq sign-extends eax into
  // edx:eax, and eax must not be spilled between loading the dividend and idiv.
  {
    auto const dividend = rs->allocateRegister(gen);
    auto const divisor = rs->allocateRegister(gen);
    auto const quotient = rs->allocateRegister(gen);
    rs->noteVirtualInReal(dividend, RealRegister(REGNUM_EAX));
    rs->noteVirtualInReal(divisor, RealRegister(REGNUM_EBX));
    emitter.emitDiv(quotient, dividend, divisor, gen, true);
    rs->freeRegister(divisor);
    rs->freeRegister(quotient);
    failed |= !verify_emitter(gen, emitter_buffer_t<3>{{
      0x99,       // cdq
      0xf7, 0xfb  // idiv ebx
    }});
  }

  // Unsigned divide: the dividend is 0:eax, so edx is zeroed rather than
  // sign-extended (cdq would make a top-bit-set dividend overflow the quotient).
  {
    auto const dividend = rs->allocateRegister(gen);
    auto const divisor = rs->allocateRegister(gen);
    auto const quotient = rs->allocateRegister(gen);
    rs->noteVirtualInReal(dividend, RealRegister(REGNUM_EAX));
    rs->noteVirtualInReal(divisor, RealRegister(REGNUM_EBX));
    emitter.emitDiv(quotient, dividend, divisor, gen, false);
    rs->freeRegister(divisor);
    rs->freeRegister(quotient);
    failed |= !verify_emitter(gen, emitter_buffer_t<7>{{
      0xba, 0x0, 0x0, 0x0, 0x0,  // mov edx, 0
      0xf7, 0xf3                 // div ebx
    }});
  }

  // Signed divide with the dividend in ecx: it is moved into eax first
  {
    auto const dividend = rs->allocateRegister(gen);
    auto const divisor = rs->allocateRegister(gen);
    auto const quotient = rs->allocateRegister(gen);
    rs->noteVirtualInReal(dividend, RealRegister(REGNUM_ECX));
    rs->noteVirtualInReal(divisor, RealRegister(REGNUM_EBX));
    emitter.emitDiv(quotient, dividend, divisor, gen, true);
    rs->freeRegister(divisor);
    rs->freeRegister(quotient);
    failed |= !verify_emitter(gen, emitter_buffer_t<5>{{
      0x8b, 0xc1,  // mov eax, ecx
      0x99,        // cdq
      0xf7, 0xfb   // idiv ebx
    }});
  }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
