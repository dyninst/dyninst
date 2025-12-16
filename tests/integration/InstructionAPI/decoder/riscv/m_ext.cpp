#include "Architecture.h"
#include "InstructionDecoder.h"
#include "memory_tests.h"
#include "register_tests.h"
#include "opcode_tests.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/riscv64_regs.h"

#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  The RISC-V Instruction Set Manual Volume I: Unprivileged Architecture
 *  Chapter 12: M Extension for Integer Multiplication and Division, Version 2.0
 */

namespace di = Dyninst::InstructionAPI;

struct m_ext_tests {
  std::vector<unsigned char> rawBytes;
  di::opcode_test opcodes;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<m_ext_tests> make_tests64();
static bool run(Dyninst::Architecture, std::vector<m_ext_tests> const &);

int main() {
  bool ok = run(Dyninst::Arch_riscv64, make_tests64());
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<m_ext_tests> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'm_ext' in " << sarch << " mode\n";
  for (auto const &t : tests) {
    test_id++;
    di::InstructionDecoder d(t.rawBytes.data(), t.rawBytes.size(), arch);
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
    if (!di::verify(insn, t.opcodes)) {
      failed = true;
    }
    if (!di::verify(insn, t.mem)) {
      failed = true;
    }
  }
  return !failed;
}

std::vector<m_ext_tests> make_tests64() {
  auto zero = Dyninst::riscv64::zero;
  auto ra = Dyninst::riscv64::ra;
  auto sp = Dyninst::riscv64::sp;
  auto gp = Dyninst::riscv64::gp;
  auto tp = Dyninst::riscv64::tp;
  auto t0 = Dyninst::riscv64::t0;
  auto t1 = Dyninst::riscv64::t1;
  auto t2 = Dyninst::riscv64::t2;
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
  auto s2 = Dyninst::riscv64::s2;
  auto s3 = Dyninst::riscv64::s3;
  auto s4 = Dyninst::riscv64::s4;
  auto s5 = Dyninst::riscv64::s5;
  auto s6 = Dyninst::riscv64::s6;
  auto s7 = Dyninst::riscv64::s7;
  auto s8 = Dyninst::riscv64::s8;
  auto s9 = Dyninst::riscv64::s9;
  auto s10 = Dyninst::riscv64::s10;
  auto s11 = Dyninst::riscv64::s11;
  auto t3 = Dyninst::riscv64::t3;
  auto t4 = Dyninst::riscv64::t4;
  auto t5 = Dyninst::riscv64::t5;
  auto t6 = Dyninst::riscv64::t6;

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // mul t0, tp, s7
      {0xb3,0x02,0x72,0x03},
      di::opcode_test{riscv64_op_mul, "mul"},
      di::register_rw_test{ reg_set{tp, s7}, reg_set{t0} },
      di::mem_test{}
    },
    { // mul sp, zero, ra
      {0x33,0x01,0x10,0x02},
      di::opcode_test{riscv64_op_mul, "mul"},
      di::register_rw_test{ reg_set{zero, ra}, reg_set{sp} },
      di::mem_test{}
    },
    { // mulh t3, t4, a2
      {0x33,0x9e,0xce,0x02},
      di::opcode_test{riscv64_op_mulh, "mulh"},
      di::register_rw_test{ reg_set{t4, a2}, reg_set{t3} },
      di::mem_test{}
    },
    { // mulh gp, t6, s11
      {0xb3,0x91,0xbf,0x03},
      di::opcode_test{riscv64_op_mulh, "mulh"},
      di::register_rw_test{ reg_set{t6, s11}, reg_set{gp} },
      di::mem_test{}
    },
    { // mulhsu s1, s2, a0
      {0xb3,0x24,0xa9,0x02},
      di::opcode_test{riscv64_op_mulhsu, "mulhsu"},
      di::register_rw_test{ reg_set{s2, a0}, reg_set{s1} },
      di::mem_test{}
    },
    { // mulhu s0, a2, s9
      {0x33,0x34,0x96,0x03},
      di::opcode_test{riscv64_op_mulhu, "mulhu"},
      di::register_rw_test{ reg_set{a2, s9}, reg_set{s0} },
      di::mem_test{}
    },
    { // div s3, s4, s5
      {0xb3,0x49,0x5a,0x03},
      di::opcode_test{riscv64_op_div, "div"},
      di::register_rw_test{ reg_set{s4, s5}, reg_set{s3} },
      di::mem_test{}
    },
    { // divu s6, t2, s8
      {0x33,0xdb,0x83,0x03},
      di::opcode_test{riscv64_op_divu, "divu"},
      di::register_rw_test{ reg_set{t2, s8}, reg_set{s6} },
      di::mem_test{}
    },
    { // rem t1, s10, zero
      {0x33,0x63,0x0d,0x02},
      di::opcode_test{riscv64_op_rem, "rem"},
      di::register_rw_test{ reg_set{s10, zero}, reg_set{t1} },
      di::mem_test{}
    },
    { // remu t1, a4, s3
      {0x33,0x73,0x37,0x03},
      di::opcode_test{riscv64_op_remu, "remu"},
      di::register_rw_test{ reg_set{a4, s3}, reg_set{t1} },
      di::mem_test{}
    },
    { // mulw a6, t5, a1
      {0x3b,0x08,0xbf,0x02},
      di::opcode_test{riscv64_op_mulw, "mulw"},
      di::register_rw_test{ reg_set{t5, a1}, reg_set{a6} },
      di::mem_test{}
    },
    { // divw s3, s4, s5
      {0xbb,0x49,0x5a,0x03},
      di::opcode_test{riscv64_op_divw, "divw"},
      di::register_rw_test{ reg_set{s4, s5}, reg_set{s3} },
      di::mem_test{}
    },
    { // remw a5, a3, a7
      {0xbb,0xe7,0x16,0x03},
      di::opcode_test{riscv64_op_remw, "remw"},
      di::register_rw_test{ reg_set{a3, a7}, reg_set{a5} },
      di::mem_test{}
    },
  };
}
