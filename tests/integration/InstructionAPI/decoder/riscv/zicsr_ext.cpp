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

struct rv64zicsr_tests {
  std::vector<unsigned char> rawBytes;
  di::opcode_test opcodes;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<rv64zicsr_tests> make_tests64();
static bool run(Dyninst::Architecture, std::vector<rv64zicsr_tests> const &);

int main() {
  bool ok = run(Dyninst::Arch_riscv64, make_tests64());
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<rv64zicsr_tests> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'rv64zicsr' in " << sarch << " mode\n";
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

std::vector<rv64zicsr_tests> make_tests64() {
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
    // --- CSRs ---
    { // csrrw ra
      {0x73,0x90,0x00,0x30},
      di::opcode_test{riscv64_op_csrrw, "csrrw", "csrrw ra"},
      di::register_rw_test{ reg_set{ra}, reg_set{} },
      di::mem_test{}
    },
    { // csrrw sp, gp
      {0x73,0x91,0x11,0x30},
      di::opcode_test{riscv64_op_csrrw, "csrrw", "csrrw sp, gp"},
      di::register_rw_test{ reg_set{gp}, reg_set{sp} },
      di::mem_test{}
    },
    { // csrrw tp, t0
      {0x73,0x92,0x42,0x30},
      di::opcode_test{riscv64_op_csrrw, "csrrw", "csrrw tp, t0"},
      di::register_rw_test{ reg_set{t0}, reg_set{tp} },
      di::mem_test{}
    },
    { // csrrs t1, t2
      {0x73,0xa3,0x53,0x30},
      di::opcode_test{riscv64_op_csrrs, "csrrs", "csrrs t1, t2"},
      di::register_rw_test{ reg_set{t2}, reg_set{t1} },
      di::mem_test{}
    },
    { // csrrs s0, s1
      {0x73,0xa4,0x04,0x32},
      di::opcode_test{riscv64_op_csrrs, "csrrs", "csrrs s0, s1"},
      di::register_rw_test{ reg_set{s1}, reg_set{s0} },
      di::mem_test{}
    },
    { // csrrs a0, a1
      {0x73,0xa5,0x35,0x32},
      di::opcode_test{riscv64_op_csrrs, "csrrs", "csrrs a0, a1"},
      di::register_rw_test{ reg_set{a1}, reg_set{a0} },
      di::mem_test{}
    },
    { // csrrc a2, a3
      {0x73,0xb6,0x36,0xb0},
      di::opcode_test{riscv64_op_csrrc, "csrrc", "csrrc a2, a3"},
      di::register_rw_test{ reg_set{a3}, reg_set{a2} },
      di::mem_test{}
    },
    { // csrrc a4, a5
      {0x73,0xb7,0x07,0xb0},
      di::opcode_test{riscv64_op_csrrc, "csrrc", "csrrc a4, a5"},
      di::register_rw_test{ reg_set{a5}, reg_set{a4} },
      di::mem_test{}
    },
    { // csrrc a6, a7
      {0x73,0xb8,0x28,0xb0},
      di::opcode_test{riscv64_op_csrrc, "csrrc", "csrrc a6, a7"},
      di::register_rw_test{ reg_set{a7}, reg_set{a6} },
      di::mem_test{}
    },
    { // csrrc s2, s3
      {0x73,0xb9,0x19,0xf1},
      di::opcode_test{riscv64_op_csrrc, "csrrc", "csrrc s2, s3"},
      di::register_rw_test{ reg_set{s3}, reg_set{s2} },
      di::mem_test{}
    },
    { // csrrwi s4, 1
      {0x73,0xda,0x20,0xf1},
      di::opcode_test{riscv64_op_csrrwi, "csrrwi", "csrrwi s4, 0x1"},
      di::register_rw_test{ reg_set{}, reg_set{s4} },
      di::mem_test{}
    },
    { // csrrwi s5, 2
      {0xf3,0x5a,0x31,0xf1},
      di::opcode_test{riscv64_op_csrrwi, "csrrwi", "csrrwi s5, 0x2"},
      di::register_rw_test{ reg_set{}, reg_set{s5} },
      di::mem_test{}
    },
    { // csrrwi s6, 3
      {0x73,0xdb,0x41,0xf1},
      di::opcode_test{riscv64_op_csrrwi, "csrrwi", "csrrwi s6, 0x3"},
      di::register_rw_test{ reg_set{}, reg_set{s6} },
      di::mem_test{}
    },
    { // csrrwi s7, 4
      {0xf3,0x5b,0x52,0xf1},
      di::opcode_test{riscv64_op_csrrwi, "csrrwi", "csrrwi s7, 0x4"},
      di::register_rw_test{ reg_set{}, reg_set{s7} },
      di::mem_test{}
    },
    { // csrrsi s8, 8
      {0x73,0x6c,0x04,0x34},
      di::opcode_test{riscv64_op_csrrsi, "csrrsi", "csrrsi s8, 0x8"},
      di::register_rw_test{ reg_set{}, reg_set{s8} },
      di::mem_test{}
    },
    { // csrrsi s9, 12
      {0xf3,0x6c,0x16,0x34},
      di::opcode_test{riscv64_op_csrrsi, "csrrsi", "csrrsi s9, 0xc"},
      di::register_rw_test{ reg_set{}, reg_set{s9} },
      di::mem_test{}
    },
    { // csrrsi s10, 16
      {0x73,0x6d,0x28,0x34},
      di::opcode_test{riscv64_op_csrrsi, "csrrsi", "csrrsi s10, 0x10"},
      di::register_rw_test{ reg_set{}, reg_set{s10} },
      di::mem_test{}
    },
    { // csrrsi s11, 19
      {0xf3,0xed,0x39,0x34},
      di::opcode_test{riscv64_op_csrrsi, "csrrsi", "csrrsi s11, 0x13"},
      di::register_rw_test{ reg_set{}, reg_set{s11} },
      di::mem_test{}
    },
    { // csrrci t3, 20
      {0x73,0x7e,0x4a,0x34},
      di::opcode_test{riscv64_op_csrrci, "csrrci", "csrrci t3, 0x14"},
      di::register_rw_test{ reg_set{}, reg_set{t3} },
      di::mem_test{}
    },
    { // csrrci t4, 23
      {0xf3,0xfe,0x0b,0x7a},
      di::opcode_test{riscv64_op_csrrci, "csrrci", "csrrci t4, 0x17"},
      di::register_rw_test{ reg_set{}, reg_set{t4} },
      di::mem_test{}
    },
    { // csrrci t5, 24
      {0x73,0x7f,0x1c,0x7a},
      di::opcode_test{riscv64_op_csrrci, "csrrci", "csrrci t5, 0x18"},
      di::register_rw_test{ reg_set{}, reg_set{t5} },
      di::mem_test{}
    },
    { // csrrci t6, 31
      {0xf3,0xff,0x2f,0x7a},
      di::opcode_test{riscv64_op_csrrci, "csrrci", "csrrci t6, 0x1f"},
      di::register_rw_test{ reg_set{}, reg_set{t6} },
      di::mem_test{}
    },
  };
}
