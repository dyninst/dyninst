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
 *  Chapter 4: RV64I Base Integer Instruction Set, Version 2.1
 */

namespace di = Dyninst::InstructionAPI;

struct rv64i_tests {
  std::vector<unsigned char> rawBytes;
  di::opcode_test opcodes;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<rv64i_tests> make_tests64();
static bool run(Dyninst::Architecture, std::vector<rv64i_tests> const &);

int main() {
  bool ok = run(Dyninst::Arch_riscv64, make_tests64());
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<rv64i_tests> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'rv64i' in " << sarch << " mode\n";
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

std::vector<rv64i_tests> make_tests64() {
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
  auto pc = Dyninst::riscv64::pc;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    // --- U-type ---
    { // lui gp,0x12345
      {0xb7,0x51,0x34,0x12},
      di::opcode_test{riscv64_op_lui, "lui"},
      di::register_rw_test{ reg_set{}, reg_set{gp} },
      di::mem_test{}
    },
    { // auipc tp,0x12345
      {0x17,0x52,0x34,0x12},
      di::opcode_test{riscv64_op_auipc, "auipc"},
      di::register_rw_test{ reg_set{pc}, reg_set{tp} },
      di::mem_test{}
    },

    // --- J-type ---
    { // jal s0, 24
      {0x6f,0x04,0x80,0x01},
      di::opcode_test{riscv64_op_jal, "jal"},
      di::register_rw_test{ reg_set{pc}, reg_set{s0, pc} },
      di::mem_test{}
    },

    // --- B-type branches ---
    { // beq t1, zero, 12
      {0x63,0x06,0x03,0x00},
      di::opcode_test{riscv64_op_beq, "beq"},
      di::register_rw_test{ reg_set{t1, zero, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // bne s1, s2, 12
      {0x63,0x96,0x24,0x01},
      di::opcode_test{riscv64_op_bne, "bne"},
      di::register_rw_test{ reg_set{s1, s2, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // blt a0, a1, 8
      {0x63,0x44,0xb5,0x00},
      di::opcode_test{riscv64_op_blt, "blt"},
      di::register_rw_test{ reg_set{a0, a1, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // bge a2, a3, -66
      {0xe3,0x5f,0xd6,0xfa},
      di::opcode_test{riscv64_op_bge, "bge"},
      di::register_rw_test{ reg_set{a2, a3, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // bltu t3, t4, -1024
      {0xe3,0x60,0xde,0xc1},
      di::opcode_test{riscv64_op_bltu, "bltu"},
      di::register_rw_test{ reg_set{t3, t4, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // bgeu t5, t6, 0
      {0x63,0x70,0xff,0x01},
      di::opcode_test{riscv64_op_bgeu, "bgeu"},
      di::register_rw_test{ reg_set{t5, t6, pc}, reg_set{pc} },
      di::mem_test{}
    },

    // --- Jalr (I-type) ---
    { // jalr s11, t6, 0x10
      {0xe7,0x8d,0x0f,0x01},
      di::opcode_test{riscv64_op_jalr, "jalr"},
      di::register_rw_test{ reg_set{t6}, reg_set{s11, pc} },
      di::mem_test{}
    },
    // --- Loads (I-type) ---
    { // lb s1, 0(sp)
      {0x83,0x04,0x01,0x00},
      di::opcode_test{riscv64_op_lb, "lb"},
      di::register_rw_test{ reg_set{sp}, reg_set{s1} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{sp}, reg_set{} } }
    },
    { // lh s2, 0(sp)
      {0x03,0x19,0x01,0x00},
      di::opcode_test{riscv64_op_lh, "lh"},
      di::register_rw_test{ reg_set{sp}, reg_set{s2} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{sp}, reg_set{} } }
    },
    { // lw a0, 4(s1)
      {0x03,0xa5,0x44,0x00},
      di::opcode_test{riscv64_op_lw, "lw"},
      di::register_rw_test{ reg_set{s1}, reg_set{a0} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{s1}, reg_set{} } }
    },
    { // lbu a1, 8(s1)
      {0x83,0x45,0x89,0x00},
      di::opcode_test{riscv64_op_lbu, "lbu"},
      di::register_rw_test{ reg_set{s2}, reg_set{a1} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{s2}, reg_set{} } }
    },
    { // lhu a2, 12(s3)
      {0x03,0xd6,0xc9,0x00},
      di::opcode_test{riscv64_op_lhu, "lhu"},
      di::register_rw_test{ reg_set{s3}, reg_set{a2} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{s3}, reg_set{} } }
    },
    { // lwu a3, 16(s4)
      {0x83,0x66,0x0a,0x01},
      di::opcode_test{riscv64_op_lwu, "lwu"},
      di::register_rw_test{ reg_set{s4}, reg_set{a3} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{s4}, reg_set{} } }
    },
    { // ld s3, 0(s2)
      {0x83,0x39,0x09,0x00},
      di::opcode_test{riscv64_op_ld, "ld"},
      di::register_rw_test{ reg_set{s2}, reg_set{s3} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{s2}, reg_set{} } }
    },

    // --- Stores (S-type) ---
    { // sb a4, 0(s2)
      {0x23,0x00,0xe9,0x00},
      di::opcode_test{riscv64_op_sb, "sb"},
      di::register_rw_test{ reg_set{a4, s2}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{s2} } }
    },
    { // sh a5, 2(s3)
      {0x23,0x91,0xf9,0x00},
      di::opcode_test{riscv64_op_sh, "sh"},
      di::register_rw_test{ reg_set{a5, s3}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{s3} } }
    },
    { // sw s4, 4(t5)
      {0x23,0x22,0x4f,0x01},
      di::opcode_test{riscv64_op_sw, "sw"},
      di::register_rw_test{ reg_set{s4, t5}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{t5} } }
    },
    { // sd s5, 8(t6)
      {0x23,0xb4,0x5f,0x01},
      di::opcode_test{riscv64_op_sd, "sd"},
      di::register_rw_test{ reg_set{s5, t6}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{t6} } }
    },

    // --- Immediate arithmetic/logical (I-type) ---
    { // addi t0, t1, 5
      {0x93,0x02,0x53,0x00},
      di::opcode_test{riscv64_op_addi, "addi"},
      di::register_rw_test{ reg_set{t1}, reg_set{t0} },
      di::mem_test{}
    },
    { // slti t2, t3, -1
      {0x93,0x23,0xfe,0xff},
      di::opcode_test{riscv64_op_slti, "slti"},
      di::register_rw_test{ reg_set{t3}, reg_set{t2} },
      di::mem_test{}
    },
    { // sltiu t4, t5, 10
      {0x93,0x3e,0xaf,0x00},
      di::opcode_test{riscv64_op_sltiu, "sltiu"},
      di::register_rw_test{ reg_set{t5}, reg_set{t4} },
      di::mem_test{}
    },
    { // xori t6, s6, 0xff
      {0x93,0x4f,0xfb,0x0f},
      di::opcode_test{riscv64_op_xori, "xori"},
      di::register_rw_test{ reg_set{s6}, reg_set{t6} },
      di::mem_test{}
    },
    { // ori s7, s0, 0x1
      {0x93,0x6b,0x14,0x00},
      di::opcode_test{riscv64_op_ori, "ori"},
      di::register_rw_test{ reg_set{s0}, reg_set{s7} },
      di::mem_test{}
    },
    { // andi s8, s1, 0xff
      {0x13,0xfc,0xf4,0x0f},
      di::opcode_test{riscv64_op_andi, "andi"},
      di::register_rw_test{ reg_set{s1}, reg_set{s8} },
      di::mem_test{}
    },

    // --- Shift-immediate (I-type with shamt) ---
    { // slli a4, a5, 4
      {0x13,0x97,0x47,0x00},
      di::opcode_test{riscv64_op_slli, "slli"},
      di::register_rw_test{ reg_set{a5}, reg_set{a4} },
      di::mem_test{}
    },
    { // srli a6, a7, 8
      {0x13,0xd8,0x88,0x00},
      di::opcode_test{riscv64_op_srli, "srli"},
      di::register_rw_test{ reg_set{a7}, reg_set{a6} },
      di::mem_test{}
    },
    { // srai s9, s10, 16
      {0x93,0x5c,0x0d,0x41},
      di::opcode_test{riscv64_op_srai, "srai"},
      di::register_rw_test{ reg_set{s10}, reg_set{s9} },
      di::mem_test{}
    },

    // --- R-type ---
    { // add s10, s11, a0
      {0x33,0x8d,0xad,0x00},
      di::opcode_test{riscv64_op_add, "add"},
      di::register_rw_test{ reg_set{s11, a0}, reg_set{s10} },
      di::mem_test{}
    },
    { // sub a1, a2, a3
      {0xb3,0x05,0xd6,0x40},
      di::opcode_test{riscv64_op_sub, "sub"},
      di::register_rw_test{ reg_set{a2, a3}, reg_set{a1} },
      di::mem_test{}
    },
    { // sll a4, a5, a6
      {0x33,0x97,0x07,0x01},
      di::opcode_test{riscv64_op_sll, "sll"},
      di::register_rw_test{ reg_set{a5, a6}, reg_set{a4} },
      di::mem_test{}
    },
    { // slt a7, t0, t1
      {0xb3,0xa8,0x62,0x00},
      di::opcode_test{riscv64_op_slt, "slt"},
      di::register_rw_test{ reg_set{t0, t1}, reg_set{a7} },
      di::mem_test{}
    },
    { // sltu t2, t3, t4
      {0xb3,0x33,0xde,0x01},
      di::opcode_test{riscv64_op_sltu, "sltu"},
      di::register_rw_test{ reg_set{t3, t4}, reg_set{t2} },
      di::mem_test{}
    },
    { // xor t5, t6, s0
      {0x33,0xcf,0x8f,0x00},
      di::opcode_test{riscv64_op_xor, "xor"},
      di::register_rw_test{ reg_set{t6, s0}, reg_set{t5} },
      di::mem_test{}
    },
    { // srl s1, s2, s3
      {0xb3,0x54,0x39,0x01},
      di::opcode_test{riscv64_op_srl, "srl"},
      di::register_rw_test{ reg_set{s2, s3}, reg_set{s1} },
      di::mem_test{}
    },
    { // sra s4, s5, s6
      {0x33,0xda,0x6a,0x41},
      di::opcode_test{riscv64_op_sra, "sra"},
      di::register_rw_test{ reg_set{s5, s6}, reg_set{s4} },
      di::mem_test{}
    },
    { // or s7, s8, s9
      {0xb3,0x6b,0x9c,0x01},
      di::opcode_test{riscv64_op_or, "or"},
      di::register_rw_test{ reg_set{s8, s9}, reg_set{s7} },
      di::mem_test{}
    },
    { // and s10, s11, ra
      {0x33,0xfd,0x1d,0x00},
      di::opcode_test{riscv64_op_and, "and"},
      di::register_rw_test{ reg_set{s11, ra}, reg_set{s10} },
      di::mem_test{}
    },
    // --- Fence / memory ordering ---
    { // fence
      {0x0f,0x00,0xf0,0x0f},
      di::opcode_test{riscv64_op_fence, "fence"},
      di::register_rw_test{ reg_set{}, reg_set{} },
      di::mem_test{}
    },
    { // fence.i
      {0x0f,0x10,0x00,0x00},
      di::opcode_test{riscv64_op_fence_i, "fence.i"},
      di::register_rw_test{ reg_set{}, reg_set{} },
      di::mem_test{}
    },
    // --- Environment Calls and Breakpoints ---
    { // ecall
      {0x73,0x00,0x00,0x00},
      di::opcode_test{riscv64_op_ecall, "ecall"},
      di::register_rw_test{ reg_set{}, reg_set{} },
      di::mem_test{}
    },
    { // ebreak
      {0x73,0x00,0x10,0x00},
      di::opcode_test{riscv64_op_ebreak, "ebreak"},
      di::register_rw_test{ reg_set{}, reg_set{} },
      di::mem_test{}
    },
  };
}
