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
 *  Chapter 27: "C" Extension for Compressed Instructions, Version 2.0
 */

namespace di = Dyninst::InstructionAPI;

struct c_ext_tests {
  std::vector<unsigned char> rawBytes;
  di::opcode_test opcodes;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<c_ext_tests> make_tests64();
static bool run(Dyninst::Architecture, std::vector<c_ext_tests> const &);

int main() {
  bool ok = run(Dyninst::Arch_riscv64, make_tests64());
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<c_ext_tests> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'c_ext' in " << sarch << " mode\n";
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

std::vector<c_ext_tests> make_tests64() {
  // General purpose registers
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

  // Floating point registers
  // Only f8 ~ f15 is used in compressed instructions
  auto f8 = Dyninst::riscv64::f8;
  auto f9 = Dyninst::riscv64::f9;
  auto f10 = Dyninst::riscv64::f10;
  auto f11 = Dyninst::riscv64::f11;
  auto f12 = Dyninst::riscv64::f12;
  auto f13 = Dyninst::riscv64::f13;
  auto f14 = Dyninst::riscv64::f14;
  auto f15 = Dyninst::riscv64::f15;

  auto pc = Dyninst::riscv64::pc;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    { // c.addi tp, 8
      {0x21,0x02},
      di::opcode_test{riscv64_op_addi, riscv64_op_c_addi, "addi", "c.addi"},
      di::register_rw_test{ reg_set{tp}, reg_set{tp} },
      di::mem_test{}
    },
    { // c.addi s4, -27
      {0x15,0x1a},
      di::opcode_test{riscv64_op_addi, riscv64_op_c_addi, "addi", "c.addi"},
      di::register_rw_test{ reg_set{s4}, reg_set{s4} },
      di::mem_test{}
    },
    { // c.nop
      {0x01,0x00},
      di::opcode_test{riscv64_op_addi, riscv64_op_c_nop, "addi", "c.nop"},
      di::register_rw_test{ reg_set{zero}, reg_set{zero} },
      di::mem_test{}
    },
    { // c.li t0, 5
      {0x95,0x42},
      di::opcode_test{riscv64_op_addi, riscv64_op_c_li, "addi", "c.li"},
      di::register_rw_test{ reg_set{zero}, reg_set{t0} },
      di::mem_test{}
    },
    { // c.li s9, 26
      {0xe9,0x4c},
      di::opcode_test{riscv64_op_addi, riscv64_op_c_li, "addi", "c.li"},
      di::register_rw_test{ reg_set{zero}, reg_set{s9} },
      di::mem_test{}
    },
    { // c.lui s6, 22
      {0x59,0x6b},
      di::opcode_test{riscv64_op_lui, riscv64_op_c_lui, "lui", "c.lui"},
      di::register_rw_test{ reg_set{}, reg_set{s6} },
      di::mem_test{}
    },
    { // c.lui s11, 30
      {0xf9,0x6d},
      di::opcode_test{riscv64_op_lui, riscv64_op_c_lui, "lui", "c.lui"},
      di::register_rw_test{ reg_set{}, reg_set{s11} },
      di::mem_test{}
    },
    { // c.slli t1, 3
      {0x0e,0x03},
      di::opcode_test{riscv64_op_slli, riscv64_op_c_slli, "slli", "c.slli"},
      di::register_rw_test{ reg_set{t1}, reg_set{t1} },
      di::mem_test{}
    },
    { // c.slli t4, 19
      {0xce,0x0e},
      di::opcode_test{riscv64_op_slli, riscv64_op_c_slli, "slli", "c.slli"},
      di::register_rw_test{ reg_set{t4}, reg_set{t4} },
      di::mem_test{}
    },
    { // c.add s7, s8
      {0xe2,0x9b},
      di::opcode_test{riscv64_op_add, riscv64_op_c_add, "add", "c.add"},
      di::register_rw_test{ reg_set{s7, s8}, reg_set{s7} },
      di::mem_test{}
    },
    { // c.add s5, t5
      {0xfa,0x9a},
      di::opcode_test{riscv64_op_add, riscv64_op_c_add, "add", "c.add"},
      di::register_rw_test{ reg_set{s5, t5}, reg_set{s5} },
      di::mem_test{}
    },
    { // c.sub a1, a4
      {0x99,0x8d},
      di::opcode_test{riscv64_op_sub, riscv64_op_c_sub, "sub", "c.sub"},
      di::register_rw_test{ reg_set{a1, a4}, reg_set{a1} },
      di::mem_test{}
    },
    { // c.xor a3, s1
      {0xa5,0x8e},
      di::opcode_test{riscv64_op_xor, riscv64_op_c_xor, "xor", "c.xor"},
      di::register_rw_test{ reg_set{a3, s1}, reg_set{a3} },
      di::mem_test{}
    },
    { // c.xor s0, a3
      {0x35,0x8c},
      di::opcode_test{riscv64_op_xor, riscv64_op_c_xor, "xor", "c.xor"},
      di::register_rw_test{ reg_set{s0, a3}, reg_set{s0} },
      di::mem_test{}
    },
    { // c.or s0, a5
      {0x5d,0x8c},
      di::opcode_test{riscv64_op_or, riscv64_op_c_or, "or", "c.or"},
      di::register_rw_test{ reg_set{s0, a5}, reg_set{s0} },
      di::mem_test{}
    },
    { // c.and a1, a0
      {0xe9,0x8d},
      di::opcode_test{riscv64_op_and, riscv64_op_c_and, "and", "c.and"},
      di::register_rw_test{ reg_set{a1, a0}, reg_set{a1} },
      di::mem_test{}
    },
    { // c.mv s2, s10
      {0x6a,0x89},
      di::opcode_test{riscv64_op_add, riscv64_op_c_mv, "add", "c.mv"},
      di::register_rw_test{ reg_set{s10, zero}, reg_set{s2} },
      di::mem_test{}
    },
    { // c.mv s3, t5
      {0xfa,0x89},
      di::opcode_test{riscv64_op_add, riscv64_op_c_mv, "add", "c.mv"},
      di::register_rw_test{ reg_set{t5, zero}, reg_set{s3} },
      di::mem_test{}
    },
    { // c.mv t2, t3
      {0xf2,0x83},
      di::opcode_test{riscv64_op_add, riscv64_op_c_mv, "add", "c.mv"},
      di::register_rw_test{ reg_set{t3, zero}, reg_set{t2} },
      di::mem_test{}
    },
    { // c.beqz s0, 16
      {0x01,0xc8},
      di::opcode_test{riscv64_op_beq, riscv64_op_c_beqz, "beq", "c.beqz"},
      di::register_rw_test{ reg_set{s0, zero, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // c.bnez s1, -8
      {0xe5,0xfc},
      di::opcode_test{riscv64_op_bne, riscv64_op_c_bnez, "bne", "c.bnez"},
      di::register_rw_test{ reg_set{s1, zero, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // c.lwsp gp, 8(sp)
      {0xa2,0x41},
      di::opcode_test{riscv64_op_lw, riscv64_op_c_lwsp, "lw", "c.lwsp"},
      di::register_rw_test{ reg_set{sp}, reg_set{gp} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{sp}, reg_set{} } }
    },
    { // c.swsp a7, 12(sp)
      {0x46,0xc6},
      di::opcode_test{riscv64_op_sw, riscv64_op_c_swsp, "sw", "c.swsp"},
      di::register_rw_test{ reg_set{a7, sp}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{sp} } }
    },
    { // c.lw a2, 4(s1)
      {0xd0,0x40},
      di::opcode_test{riscv64_op_lw, riscv64_op_c_lw, "lw", "c.lw"},
      di::register_rw_test{ reg_set{s1}, reg_set{a2} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{s1}, reg_set{} } }
    },
    { // c.sw a3, 8(a0)
      {0x14,0xc5},
      di::opcode_test{riscv64_op_sw, riscv64_op_c_sw, "sw", "c.sw"},
      di::register_rw_test{ reg_set{a3, a0}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{a0} } }
    },
    { // c.j 256
      {0x01,0xa2},
      di::opcode_test{riscv64_op_jal, riscv64_op_c_j, "jal", "c.j"},
      di::register_rw_test{ reg_set{pc}, reg_set{pc, zero} },
      di::mem_test{}
    },
    { // c.jr ra
      {0x82,0x80},
      di::opcode_test{riscv64_op_jalr, riscv64_op_c_jr, "jalr", "c.jr"},
      di::register_rw_test{ reg_set{ra}, reg_set{zero, pc} },
      di::mem_test{},
    },
    { // c.jr a6
      {0x02,0x88},
      di::opcode_test{riscv64_op_jalr, riscv64_op_c_jr, "jalr", "c.jr"},
      di::register_rw_test{ reg_set{a6}, reg_set{zero, pc} },
      di::mem_test{},
    },
    { // c.jalr a0
      {0x02,0x95},
      di::opcode_test{riscv64_op_jalr, riscv64_op_c_jalr, "jalr", "c.jalr"},
      di::register_rw_test{ reg_set{a0}, reg_set{ra, pc} },
      di::mem_test{},
    },
    { // c.jalr t6
      {0x82,0x9f},
      di::opcode_test{riscv64_op_jalr, riscv64_op_c_jalr, "jalr", "c.jalr"},
      di::register_rw_test{ reg_set{t6}, reg_set{ra, pc} },
      di::mem_test{},
    },
    { // c.ebreak
      {0x02,0x90},
      di::opcode_test{riscv64_op_ebreak, riscv64_op_c_ebreak, "ebreak", "c.ebreak"},
      di::register_rw_test{ reg_set{}, reg_set{} },
      di::mem_test{}
    },
    { // c.addi4spn a3, sp, 32
      {0x14,0x10},
      di::opcode_test{riscv64_op_addi, riscv64_op_c_addi4spn, "addi", "c.addi4spn"},
      di::register_rw_test{ reg_set{sp}, reg_set{a3} },
      di::mem_test{}
    },
    { // c.addi16sp sp, 16
      {0x41,0x61},
      di::opcode_test{riscv64_op_addi, riscv64_op_c_addi16sp, "addi", "c.addi16sp"},
      di::register_rw_test{ reg_set{sp}, reg_set{sp} },
      di::mem_test{}
    },
    { // c.addi16sp sp, 64
      {0x21,0x61},
      di::opcode_test{riscv64_op_addi, riscv64_op_c_addi16sp, "addi", "c.addi16sp"},
      di::register_rw_test{ reg_set{sp}, reg_set{sp} },
      di::mem_test{}
    },
    { // c.srli a0, 2
      {0x09,0x81},
      di::opcode_test{riscv64_op_srli, riscv64_op_c_srli, "srli", "c.srli"},
      di::register_rw_test{ reg_set{a0}, reg_set{a0} },
      di::mem_test{}
    },
    { // c.srai a1, 12
      {0xb1,0x85},
      di::opcode_test{riscv64_op_srai, riscv64_op_c_srai, "srai", "c.srai"},
      di::register_rw_test{ reg_set{a1}, reg_set{a1} },
      di::mem_test{}
    },
    { // c.andi a3, 15
      {0xbd,0x8a},
      di::opcode_test{riscv64_op_andi, riscv64_op_c_andi, "andi", "c.andi"},
      di::register_rw_test{ reg_set{a3}, reg_set{a3} },
      di::mem_test{}
    },
    { // c.slli a2, 40
      {0x22,0x16},
      di::opcode_test{riscv64_op_slli, riscv64_op_c_slli, "slli", "c.slli"},
      di::register_rw_test{ reg_set{a2}, reg_set{a2} },
      di::mem_test{}
    },
    // The following are RV64C only instructions
    { // c.addiw ra, 0xc
      {0xb1,0x20},
      di::opcode_test{riscv64_op_addiw, riscv64_op_c_addiw, "addiw", "c.addiw"},
      di::register_rw_test{ reg_set{ra}, reg_set{ra} },
      di::mem_test{}
    },
    { // c.addw a3, s1
      {0xa5,0x9e},
      di::opcode_test{riscv64_op_addw, riscv64_op_c_addw, "addw", "c.addw"},
      di::register_rw_test{ reg_set{a3, s1}, reg_set{a3} },
      di::mem_test{}
    },
    { // c.subw a4, a2
      {0x11,0x9f},
      di::opcode_test{riscv64_op_subw, riscv64_op_c_subw, "subw", "c.subw"},
      di::register_rw_test{ reg_set{a4, a2}, reg_set{a4} },
      di::mem_test{}
    },
    { // c.ldsp a0, 24(sp)
      {0x62,0x65},
      di::opcode_test{riscv64_op_ld, riscv64_op_c_ldsp, "ld", "c.ldsp"},
      di::register_rw_test{ reg_set{sp}, reg_set{a0} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{sp}, reg_set{} } }
    },
    { // c.sdsp a6, 12(sp)
      {0x42,0xec},
      di::opcode_test{riscv64_op_sd, riscv64_op_c_sdsp, "sd", "c.sdsp"},
      di::register_rw_test{ reg_set{a6, sp}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{sp} } }
    },
    { // c.fldsp f8, 16(sp)
      {0x42,0x24},
      di::opcode_test{riscv64_op_fld, riscv64_op_c_fldsp, "fld", "c.fldsp"},
      di::register_rw_test{ reg_set{sp}, reg_set{f8} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{sp}, reg_set{} } }
    },
    { // c.fldsp f9, 304(sp)
      {0xd2,0x34},
      di::opcode_test{riscv64_op_fld, riscv64_op_c_fldsp, "fld", "c.fldsp"},
      di::register_rw_test{ reg_set{sp}, reg_set{f9} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{sp}, reg_set{} } }
    },
    { // c.fsdsp f10, 192(sp)
      {0xaa,0xa1},
      di::opcode_test{riscv64_op_fsd, riscv64_op_c_fsdsp, "fsd", "c.fsdsp"},
      di::register_rw_test{ reg_set{f10, sp}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{sp} } }
    },
    { // c.fsdsp f11, 88(sp)
      {0xae,0xac},
      di::opcode_test{riscv64_op_fsd, riscv64_op_c_fsdsp, "fsd", "c.fsdsp"},
      di::register_rw_test{ reg_set{f11, sp}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{sp} } }
    },
    { // c.ld a4, 0(a2)
      {0x18,0x62},
      di::opcode_test{riscv64_op_ld, riscv64_op_c_ld, "ld", "c.ld"},
      di::register_rw_test{ reg_set{a2}, reg_set{a4} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{a2}, reg_set{} } }
    },
    { // c.sd a5, 8(a4)
      {0x1c,0xe7},
      di::opcode_test{riscv64_op_sd, riscv64_op_c_sd, "sd", "c.sd"},
      di::register_rw_test{ reg_set{a5, a4}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{a4} } }
    },
    { // c.fld f12, 56(a1)
      {0x90,0x3d},
      di::opcode_test{riscv64_op_fld, riscv64_op_c_fld, "fld", "c.fld"},
      di::register_rw_test{ reg_set{a1}, reg_set{f12} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{a1}, reg_set{} } }
    },
    { // c.fld f13, 184(a3)
      {0xd4,0x3e},
      di::opcode_test{riscv64_op_fld, riscv64_op_c_fld, "fld", "c.fld"},
      di::register_rw_test{ reg_set{a3}, reg_set{f13} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{a3}, reg_set{} } }
    },
    { // c.fsd f14, 200(s1)
      {0xf8,0xa4},
      di::opcode_test{riscv64_op_fsd, riscv64_op_c_fsd, "fsd", "c.fsd"},
      di::register_rw_test{ reg_set{s1, f14}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{s1} } }
    },
    { // c.fsd f15, 56(a4)
      {0x1c,0xbf},
      di::opcode_test{riscv64_op_fsd, riscv64_op_c_fsd, "fsd", "c.fsd"},
      di::register_rw_test{ reg_set{a4, f15}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{a4} } }
    },
  };
}
