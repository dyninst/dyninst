#include "Architecture.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "register_tests.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/ppc32_regs.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  PowerPC instruction decoding tests
 *
 *  These verify the register read/write sets produced by the decoder for a
 *  selection of fixed-point, floating-point, load/store, and branch
 *  instructions.
 *
 *  NOTE: The decoder currently produces registers from the ppc32 namespace
 *  even when decoding in 64-bit mode, so these tests decode with
 *  Dyninst::Arch_ppc32 and expect ppc32 registers.
 */

namespace di = Dyninst::InstructionAPI;

struct decode_test {
  uint32_t opcode;
  di::register_rw_test regs;
};

static std::vector<decode_test> make_tests();

int main() {
  bool failed = false;
  int test_id = 0;
  const auto arch = Dyninst::Arch_ppc32;

  for(auto const &t : make_tests()) {
    test_id++;
    di::InstructionDecoder d(&t.opcode, sizeof(t.opcode), arch);
    auto insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode test " << test_id << '\n';
      failed = true;
      continue;
    }

    std::clog << "Verifying '" << insn.format() << "'\n";

    if(!di::verify(insn, t.regs)) {
      failed = true;
    }

    std::clog << '\n';
  }

  { // lhzux r5, r7, r9 must read memory
    const uint32_t lhzux = 0x7ca74a6e;
    di::InstructionDecoder d(&lhzux, sizeof(lhzux), arch);
    auto insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode memory-read test\n";
      failed = true;
    } else if(!insn.readsMemory()) {
      std::cerr << "'" << insn.format() << "' did not read memory\n";
      failed = true;
    }
  }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

std::vector<decode_test> make_tests() {
  auto r0 = Dyninst::ppc32::r0;
  auto r1 = Dyninst::ppc32::r1;
  auto r2 = Dyninst::ppc32::r2;
  auto r5 = Dyninst::ppc32::r5;
  auto r7 = Dyninst::ppc32::r7;
  auto r8 = Dyninst::ppc32::r8;
  auto r9 = Dyninst::ppc32::r9;
  auto cr0 = Dyninst::ppc32::cr0;
  auto cr2 = Dyninst::ppc32::cr2;
  auto cr4 = Dyninst::ppc32::cr4;
  auto cr6 = Dyninst::ppc32::cr6;
  auto cr7 = Dyninst::ppc32::cr7;
  auto xer = Dyninst::ppc32::xer;
  auto fpr0 = Dyninst::ppc32::fpr0;
  auto fpr1 = Dyninst::ppc32::fpr1;
  auto fpr2 = Dyninst::ppc32::fpr2;
  auto fsr0 = Dyninst::ppc32::fsr0;
  auto fsr1 = Dyninst::ppc32::fsr1;
  auto fsr2 = Dyninst::ppc32::fsr2;
  auto fpscw = Dyninst::ppc32::fpscw;
  auto fpscw0 = Dyninst::ppc32::fpscw0;
  auto fpscw2 = Dyninst::ppc32::fpscw2;
  auto fpscw4 = Dyninst::ppc32::fpscw4;
  auto fpscw6 = Dyninst::ppc32::fpscw6;
  auto fpscw7 = Dyninst::ppc32::fpscw7;
  auto pc = Dyninst::ppc32::pc;
  auto ctr = Dyninst::ppc32::ctr;
  auto lr = Dyninst::ppc32::lr;

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // add. r9, r0, r8
      0x7d204215,
      di::register_rw_test{ reg_set{r0, r8}, reg_set{r9, cr0} }
    },
    { // add r9, r0, r8
      0x7d204214,
      di::register_rw_test{ reg_set{r0, r8}, reg_set{r9} }
    },
    { // addo r9, r0, r8
      0x7d204614,
      di::register_rw_test{ reg_set{r0, r8}, reg_set{r9, xer} }
    },
    { // fadd fpr0, fpr1, fpr2
      0xfc01102a,
      di::register_rw_test{ reg_set{fpr1, fpr2}, reg_set{fpr0} }
    },
    { // fadd. fpr0, fpr1, fpr2
      0xfc01102b,
      di::register_rw_test{ reg_set{fpr1, fpr2}, reg_set{fpr0, fpscw} }
    },
    { // addi r1, 0, 1
      0x38200001,
      di::register_rw_test{ reg_set{}, reg_set{r1} }
    },
    { // addi r1, r1, 1
      0x38210001,
      di::register_rw_test{ reg_set{r1}, reg_set{r1} }
    },
    { // fcmpu fpscw7, fpr0, fpr1
      0xff800800,
      di::register_rw_test{ reg_set{fpr0, fpr1}, reg_set{fpscw7} }
    },
    { // fcmpu cr7, r0, r1
      0x7f800800,
      di::register_rw_test{ reg_set{r0, r1}, reg_set{cr7} }
    },
    { // mtcrf cr0, cr2, cr4, cr6, r0
      0x7c0aa120,
      di::register_rw_test{ reg_set{r0}, reg_set{cr0, cr2, cr4, cr6} }
    },
    { // mtfsf fpscw0, fpscw2, fpscw4, fpscw6, fpr0
      0xfd54058e,
      di::register_rw_test{ reg_set{fpr0}, reg_set{fpscw0, fpscw2, fpscw4, fpscw6} }
    },
    { // lwz r0, 0(r1)
      0x80010000,
      di::register_rw_test{ reg_set{r1}, reg_set{r0} }
    },
    { // lwzu r0, 0(r1)
      0x84010000,
      di::register_rw_test{ reg_set{r1}, reg_set{r0, r1} }
    },
    { // lwzx r0, r2(r1)
      0x7c01102e,
      di::register_rw_test{ reg_set{r1, r2}, reg_set{r0} }
    },
    { // lwzux r0, r2(r1)
      0x7c01106e,
      di::register_rw_test{ reg_set{r1, r2}, reg_set{r0, r1} }
    },
    { // rlimi r0, r1
      0x7801440c,
      di::register_rw_test{ reg_set{r0}, reg_set{r1} }
    },
    { // fpmul fpr0, fpr1
      0x00010090,
      di::register_rw_test{ reg_set{fpr1, fpr2, fsr1, fsr2}, reg_set{fpr0, fsr0} }
    },
    { // fxmul fpr0, fpr1, or qvfxmadds
      0x00010092,
      di::register_rw_test{ reg_set{fpr1, fpr2, fsr1, fsr2}, reg_set{fpr0, fsr0} }
    },
    { // fxcpmul fpr0, fpr1
      0x00010094,
      di::register_rw_test{ reg_set{fpr1, fpr2, fsr2}, reg_set{fpr0, fsr0} }
    },
    { // fxcsmul fpr0, fpr1, or qvfxxnpmadds
      0x00010096,
      di::register_rw_test{ reg_set{fpr2, fsr1, fsr2}, reg_set{fpr0, fsr0} }
    },
    { // bdnzl cr0, +0x100
      0x40010101,
      di::register_rw_test{ reg_set{pc, cr0, ctr}, reg_set{pc, ctr, lr} }
    },
    { // bdnz cr0, +0x100
      0x40010100,
      di::register_rw_test{ reg_set{pc, cr0, ctr}, reg_set{pc, ctr} }
    },
    { // lhzux r5, r7, r9
      0x7ca74a6e,
      di::register_rw_test{ reg_set{r7, r9}, reg_set{r5, r7} }
    },
    { // bclr (with BH=0, return from subroutine)
      0x4e800020,
      di::register_rw_test{ reg_set{ctr, lr, cr0}, reg_set{pc, ctr} }
    },
    { // bclrl
      0x4e800021,
      di::register_rw_test{ reg_set{ctr, lr, cr0}, reg_set{ctr, lr, pc} }
    },
    { // bctr (LK=0)
      0x4e800420,
      di::register_rw_test{ reg_set{ctr, cr0}, reg_set{pc} }
    },
    { // bctrl (LK=1)
      0x4e800421,
      di::register_rw_test{ reg_set{lr, ctr, cr0}, reg_set{lr, pc} }
    },
  };
  // clang-format on
}
