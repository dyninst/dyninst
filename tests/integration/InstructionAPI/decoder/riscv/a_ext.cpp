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
 *  Chapter 13: "A" Extension for Atomic Instructions, Version 2.1
 */

namespace di = Dyninst::InstructionAPI;

struct rv64a_tests {
  std::vector<unsigned char> rawBytes;
  di::opcode_test opcodes;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<rv64a_tests> make_tests64();
static bool run(Dyninst::Architecture, std::vector<rv64a_tests> const &);

int main() {
  bool ok = run(Dyninst::Arch_riscv64, make_tests64());
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<rv64a_tests> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'rv64a' in " << sarch << " mode\n";
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

std::vector<rv64a_tests> make_tests64() {
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

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    // --- Load-Reserved / Store-Conditional ---
    { // lr.w t0, (s1)
      {0xaf,0xa2,0x04,0x10},
      di::opcode_test{riscv64_op_lr_w, "lr.w"},
      di::register_rw_test{ reg_set{s1}, reg_set{t0} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{s1}, reg_set{} } }
    },
    { // sc.w t1, t2, (s3)
      {0x2f,0xa3,0x79,0x18},
      di::opcode_test{riscv64_op_sc_w, "sc.w"},
      di::register_rw_test{ reg_set{t2, s3}, reg_set{t1} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{s3} } }
    },
    { // lr.d zero, (t3)
      {0x2f,0x30,0x0e,0x10},
      di::opcode_test{riscv64_op_lr_d, "lr.d"},
      di::register_rw_test{ reg_set{t3}, reg_set{zero} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{t3}, reg_set{} } }
    },
    { // sc.d t6, s0, (s2)
      {0xaf,0x3f,0x89,0x18},
      di::opcode_test{riscv64_op_sc_d, "sc.d"},
      di::register_rw_test{ reg_set{s0, s2}, reg_set{t6} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{s2} } }
    },

    // --- AMOs (32-bit) ---
    { // amoadd.w s4, s5, (s6)
      {0x2f,0x2a,0x5b,0x01},
      di::opcode_test{riscv64_op_amoadd_w, "amoadd.w"},
      di::register_rw_test{ reg_set{s5, s6}, reg_set{s4} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{s6}, reg_set{s6} } }
    },
    { // amoswap.w s7, s8, (s9)
      {0xaf,0xab,0x8c,0x09},
      di::opcode_test{riscv64_op_amoswap_w, "amoswap.w"},
      di::register_rw_test{ reg_set{s8, s9}, reg_set{s7} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{s9}, reg_set{s9} } }
    },
    { // amoxor.w t3, t4, (t5)
      {0x2f,0x2e,0xdf,0x21},
      di::opcode_test{riscv64_op_amoxor_w, "amoxor.w"},
      di::register_rw_test{ reg_set{t4, t5}, reg_set{t3} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{t5}, reg_set{t5} } }
    },
    { // amoand.w a0, a1, (a2)
      {0x2f,0x25,0xb6,0x60},
      di::opcode_test{riscv64_op_amoand_w, "amoand.w"},
      di::register_rw_test{ reg_set{a1, a2}, reg_set{a0} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{a2}, reg_set{a2} } }
    },
    { // amoor.w a3, a4, (a5)
      {0xaf,0xa6,0xe7,0x40},
      di::opcode_test{riscv64_op_amoor_w, "amoor.w"},
      di::register_rw_test{ reg_set{a4, a5}, reg_set{a3} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{a5}, reg_set{a5} } }
    },
    { // amomin.w a6, a7, (s10)
      {0x2f,0x28,0x1d,0x81},
      di::opcode_test{riscv64_op_amomin_w, "amomin.w"},
      di::register_rw_test{ reg_set{a7, s10}, reg_set{a6} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{s10}, reg_set{s10} } }
    },
    { // amomax.w s11, ra, (sp)
      {0xaf,0x2d,0x11,0xa0},
      di::opcode_test{riscv64_op_amomax_w, "amomax.w"},
      di::register_rw_test{ reg_set{ra, sp}, reg_set{s11} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{sp}, reg_set{sp} } }
    },
    { // amominu.w gp, tp, (t0)
      {0xaf,0xa1,0x42,0xc0},
      di::opcode_test{riscv64_op_amominu_w, "amominu.w"},
      di::register_rw_test{ reg_set{tp, t0}, reg_set{gp} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{t0}, reg_set{t0} } }
    },
    { // amomaxu.w t1, t2, (t3)
      {0x2f,0x23,0x7e,0xe0},
      di::opcode_test{riscv64_op_amomaxu_w, "amomaxu.w"},
      di::register_rw_test{ reg_set{t2, t3}, reg_set{t1} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{t3}, reg_set{t3} } }
    },

    // --- AMOs (64-bit) ---
    { // amoadd.d s4, s5, (s6)
      {0x2f,0x3a,0x5b,0x01},
      di::opcode_test{riscv64_op_amoadd_d, "amoadd.d"},
      di::register_rw_test{ reg_set{s5, s6}, reg_set{s4} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{s6}, reg_set{s6} } }
    },
    { // amoswap.d s7, s8, (s9)
      {0xaf,0xbb,0x8c,0x09},
      di::opcode_test{riscv64_op_amoswap_d, "amoswap.d"},
      di::register_rw_test{ reg_set{s8, s9}, reg_set{s7} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{s9}, reg_set{s9} } }
    },
    { // amoxor.d t3, t4, (t5)
      {0x2f,0x3e,0xdf,0x21},
      di::opcode_test{riscv64_op_amoxor_d, "amoxor.d"},
      di::register_rw_test{ reg_set{t4, t5}, reg_set{t3} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{t5}, reg_set{t5} } }
    },
    { // amoand.d a0, a1, (a2)
      {0x2f,0x35,0xb6,0x60},
      di::opcode_test{riscv64_op_amoand_d, "amoand.d"},
      di::register_rw_test{ reg_set{a1, a2}, reg_set{a0} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{a2}, reg_set{a2} } }
    },
    { // amoor.d a3, a4, (a5)
      {0xaf,0xb6,0xe7,0x40},
      di::opcode_test{riscv64_op_amoor_d, "amoor.d"},
      di::register_rw_test{ reg_set{a4, a5}, reg_set{a3} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{a5}, reg_set{a5} } }
    },
    { // amomin.d a6, a7, (s10)
      {0x2f,0x38,0x1d,0x81},
      di::opcode_test{riscv64_op_amomin_d, "amomin.d"},
      di::register_rw_test{ reg_set{a7, s10}, reg_set{a6} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{s10}, reg_set{s10} } }
    },
    { // amomax.d s11, ra, (sp)
      {0xaf,0x3d,0x11,0xa0},
      di::opcode_test{riscv64_op_amomax_d, "amomax.d"},
      di::register_rw_test{ reg_set{ra, sp}, reg_set{s11} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{sp}, reg_set{sp} } }
    },
    { // amominu.d gp, tp, (t0)
      {0xaf,0xb1,0x42,0xc0},
      di::opcode_test{riscv64_op_amominu_d, "amominu.d"},
      di::register_rw_test{ reg_set{tp, t0}, reg_set{gp} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{t0}, reg_set{t0} } }
    },
    { // amomaxu.d t1, t2, (t3)
      {0x2f,0x33,0x7e,0xe0},
      di::opcode_test{riscv64_op_amomaxu_d, "amomaxu.d"},
      di::register_rw_test{ reg_set{t2, t3}, reg_set{t1} },
      di::mem_test{ reads_memory, writes_memory, di::register_rw_test{ reg_set{t3}, reg_set{t3} } }
    },
  };
}
