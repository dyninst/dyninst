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
 *  Chapter 4: RV64I Base Integer Instruction Set, Version 2.1
 */

namespace di = Dyninst::InstructionAPI;

struct rv64m_tests {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<rv64m_tests> make_tests64();
static bool run(Dyninst::Architecture, std::vector<rv64m_tests> const &);

int main() {
  bool ok = run(Dyninst::Arch_riscv64, make_tests64());
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<rv64m_tests> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'rv64m' in " << sarch << " mode\n";
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

std::vector<rv64m_tests> make_tests64() {
  auto zero = Dyninst::riscv64::zero;

  auto t0 = Dyninst::riscv64::t0;
  auto t1 = Dyninst::riscv64::t1;
  auto t2 = Dyninst::riscv64::t2;

  auto s1 = Dyninst::riscv64::s1;

  auto a0 = Dyninst::riscv64::a0;
  auto a1 = Dyninst::riscv64::a1;
  auto a2 = Dyninst::riscv64::a2;
  auto a3 = Dyninst::riscv64::a3;
  auto a4 = Dyninst::riscv64::a4;
  auto a5 = Dyninst::riscv64::a5;
  auto a6 = Dyninst::riscv64::a6;

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

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // mul t0, t1, t2
      {0xb3,0x02,0x73,0x02},
      di::register_rw_test{ reg_set{t1, t2}, reg_set{t0} },
      di::mem_test{}
    },
    { // mulh t3, t4, t5
      {0x33,0x9e,0xee,0x03},
      di::register_rw_test{ reg_set{t4, t5}, reg_set{t3} },
      di::mem_test{}
    },
    { // mulhsu s1, s2, a0
      {0xb3,0x24,0xa9,0x02},
      di::register_rw_test{ reg_set{s2, a0}, reg_set{s1} },
      di::mem_test{}
    },
    { // mulhu a1, a2, a3
      {0xb3,0x35,0xd6,0x02},
      di::register_rw_test{ reg_set{a2, a3}, reg_set{a1} },
      di::mem_test{}
    },
    { // div s3, s4, s5
      {0xb3,0x49,0x5a,0x03},
      di::register_rw_test{ reg_set{s4, s5}, reg_set{s3} },
      di::mem_test{}
    },
    { // divu s6, s7, s8
      {0x33,0xdb,0x8b,0x03},
      di::register_rw_test{ reg_set{s7, s8}, reg_set{s6} },
      di::mem_test{}
    },
    { // rem s9, s10, s11
      {0xb3,0x6c,0xbd,0x03},
      di::register_rw_test{ reg_set{s10, s11}, reg_set{s9} },
      di::mem_test{}
    },
    { // remu a4, a5, a6
      {0x33,0xf7,0x07,0x03},
      di::register_rw_test{ reg_set{a5, a6}, reg_set{a4} },
      di::mem_test{}
    },
  };
}
