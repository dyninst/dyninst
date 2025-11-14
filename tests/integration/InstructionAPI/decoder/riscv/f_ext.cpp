#include "Architecture.h"
#include "InstructionDecoder.h"
#include "memory_tests.h"
#include "register_tests.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/riscv64_regs.h"

#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  The RISC-V Instruction Set Manual Volume I: Unprivileged Architecture
 *  Chapter 20: "F" Extension for Single-Precision Floating-Point, Version 2.2
 */

namespace di = Dyninst::InstructionAPI;

struct rv64f_tests {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<rv64f_tests> make_tests64();
static bool run(Dyninst::Architecture, std::vector<rv64f_tests> const &);

int main() {
  bool ok = run(Dyninst::Arch_riscv64, make_tests64());
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<rv64f_tests> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'rv64f' in " << sarch << " mode\n";
  for (auto const &t : tests) {
    test_id++;
    di::InstructionDecoder d(t.opcode.data(), t.opcode.size(), arch);
    auto insn = d.decode();
    if (!insn.isValid()) {
      std::cerr << "Failed to decode " << sarch << " test " << test_id << '\n';
      failed = true;
      continue;
    }

    std::clog << "Verifying '" << insn.format() << "'\n";

    if (!di::verify(insn, t.regs)) {
      failed = true;
    }
    if (!di::verify(insn, t.mem)) {
      failed = true;
    }
  }
  return !failed;
}

std::vector<rv64f_tests> make_tests64() {
  auto s0 = Dyninst::riscv64::s0;
  auto s1 = Dyninst::riscv64::s1;
  auto a0 = Dyninst::riscv64::a0;
  auto a1 = Dyninst::riscv64::a1;
  auto a2 = Dyninst::riscv64::a2;
  auto a3 = Dyninst::riscv64::a3;
  auto a4 = Dyninst::riscv64::a4;
  auto a5 = Dyninst::riscv64::a5;
  auto a6 = Dyninst::riscv64::a6;
  auto a7 = Dyninst::riscv64::a7;

  auto f0 = Dyninst::riscv64::f0;
  auto f1 = Dyninst::riscv64::f1;
  auto f2 = Dyninst::riscv64::f2;
  auto f3 = Dyninst::riscv64::f3;
  auto f4 = Dyninst::riscv64::f4;
  auto f5 = Dyninst::riscv64::f5;
  auto f6 = Dyninst::riscv64::f6;
  auto f7 = Dyninst::riscv64::f7;
  auto f8 = Dyninst::riscv64::f8;
  auto f9 = Dyninst::riscv64::f9;
  auto f10 = Dyninst::riscv64::f10;
  auto f11 = Dyninst::riscv64::f11;
  auto f12 = Dyninst::riscv64::f12;
  auto f13 = Dyninst::riscv64::f13;
  auto f14 = Dyninst::riscv64::f14;
  auto f15 = Dyninst::riscv64::f15;
  auto f16 = Dyninst::riscv64::f16;
  auto f17 = Dyninst::riscv64::f17;
  auto f18 = Dyninst::riscv64::f18;
  auto f19 = Dyninst::riscv64::f19;
  auto f20 = Dyninst::riscv64::f20;
  auto f21 = Dyninst::riscv64::f21;
  auto f22 = Dyninst::riscv64::f22;
  auto f28 = Dyninst::riscv64::f28;
  auto f29 = Dyninst::riscv64::f29;
  auto f30 = Dyninst::riscv64::f30;
  auto f31 = Dyninst::riscv64::f31;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    // FP Loads / Stores
    { // flw ft0, 0(s1)
      {0x07,0xa0,0x04,0x00},
      di::register_rw_test{ reg_set{s1}, reg_set{f0} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{s1}, reg_set{} } }
    },
    { // fsw ft1, 4(s0)
      {0x27,0x22,0x14,0x00},
      di::register_rw_test{ reg_set{f1, s0}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{s0} } }
    },

    // FP Move to/from Integer Registers
    { // fmv.x.w a0, f2
      {0x53,0x05,0x01,0xe0},
      di::register_rw_test{ reg_set{f2}, reg_set{a0} },
      di::mem_test{}
    },
    { // fmv.w.x f3, a1
      {0xd3,0x81,0x05,0xf0},
      di::register_rw_test{ reg_set{a1}, reg_set{f3} },
      di::mem_test{}
    },

    // FP Arithmetic
    { // fadd.s f4, f5, f6
      {0x53,0xf2,0x62,0x00},
      di::register_rw_test{ reg_set{f5, f6}, reg_set{f4} },
      di::mem_test{}
    },
    { // fsub.s f7, f8, f9
      {0xd3,0x73,0x94,0x08},
      di::register_rw_test{ reg_set{f8, f9}, reg_set{f7} },
      di::mem_test{}
    },
    { // fmul.s f28, f18, f19
      {0x53,0x7e,0x39,0x11},
      di::register_rw_test{ reg_set{f18, f19}, reg_set{f28} },
      di::mem_test{}
    },
    { // fdiv.s f29, f20, f21
      {0xd3,0x7e,0x5a,0x19},
      di::register_rw_test{ reg_set{f20, f21}, reg_set{f29} },
      di::mem_test{}
    },
    { // fsqrt.s f30, f22
      {0x53,0x7f,0x0b,0x58},
      di::register_rw_test{ reg_set{f22}, reg_set{f30} },
      di::mem_test{}
    },

    // FP Sign Manipulation
    { // fsgnj.s f31, f0, f1
      {0xd3,0x0f,0x10,0x20},
      di::register_rw_test{ reg_set{f0, f1}, reg_set{f31} },
      di::mem_test{}
    },
    { // fsgnjn.s f10, f11, f12
      {0x53,0x95,0xc5,0x20},
      di::register_rw_test{ reg_set{f11, f12}, reg_set{f10} },
      di::mem_test{}
    },
    { // fsgnjx.s f13, f14, f15
      {0xd3,0x26,0xf7,0x20},
      di::register_rw_test{ reg_set{f14, f15}, reg_set{f13} },
      di::mem_test{}
    },

    // FP Comparisons
    { // feq.s a2, f16, f17
      {0x53,0x26,0x18,0xa1},
      di::register_rw_test{ reg_set{f16, f17}, reg_set{a2} },
      di::mem_test{}
    },
    { // flt.s a3, f0, f1
      {0xd3,0x16,0x10,0xa0},
      di::register_rw_test{ reg_set{f0, f1}, reg_set{a3} },
      di::mem_test{}
    },
    { // fle.s a4, f2, f3
      {0x53,0x07,0x31,0xa0},
      di::register_rw_test{ reg_set{f2, f3}, reg_set{a4} },
      di::mem_test{}
    },

    // FP Conversions
    { // fcvt.s.w f0, a5
      {0x53,0xf0,0x07,0xd0},
      di::register_rw_test{ reg_set{a5}, reg_set{f0} },
      di::mem_test{}
    },
    { // fcvt.s.wu f1, a6
      {0xd3,0x70,0x18,0xd0},
      di::register_rw_test{ reg_set{a6}, reg_set{f1} },
      di::mem_test{}
    },
    { // fcvt.w.s a7, f4
      {0xd3,0x78,0x02,0xc0},
      di::register_rw_test{ reg_set{f4}, reg_set{a7} },
      di::mem_test{}
    },
    { // fcvt.wu.s s0, f5
      {0x53,0xf4,0x12,0xc0},
      di::register_rw_test{ reg_set{f5}, reg_set{s0} },
      di::mem_test{}
    },

    // FP Classify
    { // fclass.s a0, f6
      {0x53,0x15,0x03,0xe0},
      di::register_rw_test{ reg_set{f6}, reg_set{a0} },
      di::mem_test{}
    },
  };
}
