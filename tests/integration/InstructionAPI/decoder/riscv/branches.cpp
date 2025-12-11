#include "Architecture.h"
#include "InstructionDecoder.h"
#include "cft_tests.h"
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
 *  This test covers jump and branch instructions
 */

namespace di = Dyninst::InstructionAPI;

struct branch_test {
  std::vector<unsigned char> rawBytes;
  di::opcode_test opcodes;
  di::register_rw_test regs;
  di::mem_test mem;
  di::cft_test cft;
};

static std::vector<branch_test> make_tests64();
static bool run(Dyninst::Architecture, std::vector<branch_test> const &);

int main() {
  bool ok = run(Dyninst::Arch_riscv64, make_tests64());
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<branch_test> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'branches' in " << sarch << " mode\n";
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
    if (!di::verify(insn, t.cft)) {
      failed = true;
    }
  }
  return !failed;
}

std::vector<branch_test> make_tests64() {
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

  using di::has_cft;
  using di::is_branch;
  using di::is_call;
  using di::is_conditional;
  using di::is_fallthrough;
  using di::is_indirect;
  using di::is_return;

  // clang-format off
  return {
    // Return instructions
    { // ret (jalr x0, ra, 0)
      {0x67,0x80,0x00,0x00},
      di::opcode_test{riscv64_op_jalr, "jalr"},
      di::register_rw_test{ reg_set{ra}, reg_set{zero, pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, !is_conditional, is_indirect, !is_fallthrough, !is_branch, is_return}
      }
    },
    { // ret (c.jr ra)
      {0x82,0x80},
      di::opcode_test{riscv64_op_jalr, riscv64_op_c_jr, "jalr", "c.jr"},
      di::register_rw_test{ reg_set{ra}, reg_set{zero, pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, !is_conditional, is_indirect, !is_fallthrough, !is_branch, is_return}
      }
    },
    // Branch instructions
    { // jal zero, 16
      {0x6f,0x00,0x00,0x01},
      di::opcode_test{riscv64_op_jal, "jal"},
      di::register_rw_test{ reg_set{pc}, reg_set{zero, pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, !is_conditional, !is_indirect, !is_fallthrough, is_branch, !is_return}
      }
    },
    { // jal ra, 32
      {0xef,0x00,0x00,0x02},
      di::opcode_test{riscv64_op_jal, "jal"},
      di::register_rw_test{ reg_set{pc}, reg_set{ra, pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {is_call, !is_conditional, !is_indirect, !is_fallthrough, !is_branch, !is_return}
      }
    },
    { // jal t0, -16
      {0xef,0xf2,0x1f,0xff},
      di::opcode_test{riscv64_op_jal, "jal"},
      di::register_rw_test{ reg_set{pc}, reg_set{t0, pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {is_call, !is_conditional, !is_indirect, !is_fallthrough, !is_branch, !is_return}
      }
    },
    { // jal s0, -32
      {0x6f,0xf4,0x1f,0xfe},
      di::opcode_test{riscv64_op_jal, "jal"},
      di::register_rw_test{ reg_set{pc}, reg_set{s0, pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {is_call, !is_conditional, !is_indirect, !is_fallthrough, !is_branch, !is_return}
      }
    },
    { // jalr zero, s11, 64
      {0x67,0x80,0x0d,0x04},
      di::opcode_test{riscv64_op_jalr, "jalr"},
      di::register_rw_test{ reg_set{s11}, reg_set{zero, pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, !is_conditional, is_indirect, !is_fallthrough, is_branch, !is_return}
      }
    },
    { // jalr ra, s10, -64
      {0xe7,0x00,0x0d,0xfc},
      di::opcode_test{riscv64_op_jalr, "jalr"},
      di::register_rw_test{ reg_set{s10}, reg_set{ra, pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {is_call, !is_conditional, is_indirect, !is_fallthrough, !is_branch, !is_return}
      }
    },
    { // jalr s2, s3, 1023
      {0x67,0x89,0xf9,0x3f},
      di::opcode_test{riscv64_op_jalr, "jalr"},
      di::register_rw_test{ reg_set{s3}, reg_set{s2, pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {is_call, !is_conditional, is_indirect, !is_fallthrough, !is_branch, !is_return}
      }
    },
    { // jalr sp, gp, 0
      {0x67,0x81,0xf1,0x3f},
      di::opcode_test{riscv64_op_jalr, "jalr"},
      di::register_rw_test{ reg_set{gp}, reg_set{sp, pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {is_call, !is_conditional, is_indirect, !is_fallthrough, !is_branch, !is_return}
      }
    },
    { // c.j 2
      {0x09,0xa0},
      di::opcode_test{riscv64_op_jal, riscv64_op_c_j, "jal", "c.j"},
      di::register_rw_test{ reg_set{pc}, reg_set{zero, pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, !is_conditional, !is_indirect, !is_fallthrough, is_branch, !is_return}
      }
    },
    { // c.jr t1
      {0x02,0x83},
      di::opcode_test{riscv64_op_jalr, riscv64_op_c_jr, "jalr", "c.jr"},
      di::register_rw_test{ reg_set{t1}, reg_set{zero, pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, !is_conditional, is_indirect, !is_fallthrough, is_branch, !is_return}
      }
    },
    { // c.jr tp
      {0x02,0x82},
      di::opcode_test{riscv64_op_jalr, riscv64_op_c_jr, "jalr", "c.jr"},
      di::register_rw_test{ reg_set{tp}, reg_set{zero, pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, !is_conditional, is_indirect, !is_fallthrough, is_branch, !is_return}
      }
    },
    { // c.jalr ra
      {0x82,0x90},
      di::opcode_test{riscv64_op_jalr, riscv64_op_c_jalr, "jalr", "c.jalr"},
      di::register_rw_test{ reg_set{ra}, reg_set{ra, pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {is_call, !is_conditional, is_indirect, !is_fallthrough, !is_branch, !is_return}
      }
    },
    { // c.jalr s1
      {0x82,0x94},
      di::opcode_test{riscv64_op_jalr, riscv64_op_c_jalr, "jalr", "c.jalr"},
      di::register_rw_test{ reg_set{s1}, reg_set{ra, pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {is_call, !is_conditional, is_indirect, !is_fallthrough, !is_branch, !is_return}
      }
    },
    { // beq t2, a2, 2
      {0x63,0x81,0xc3,0x00},
      di::opcode_test{riscv64_op_beq, "beq"},
      di::register_rw_test{ reg_set{t2, a2, pc}, reg_set{pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // beq t5, t6, 88
      {0x63,0x0c,0xff,0x05},
      di::opcode_test{riscv64_op_beq, "beq"},
      di::register_rw_test{ reg_set{t5, t6, pc}, reg_set{pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // beq s7, s7, -10
      {0xe3,0x8b,0x7b,0xff},
      di::opcode_test{riscv64_op_beq, "beq"},
      di::register_rw_test{ reg_set{s7, pc}, reg_set{pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // beq t4, t4, 0
      {0x63,0x80,0xde,0x01},
      di::opcode_test{riscv64_op_beq, "beq"},
      di::register_rw_test{ reg_set{t4, pc}, reg_set{pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // beq zero, zero, 0
      {0x63,0x00,0x00,0x00},
      di::opcode_test{riscv64_op_beq, "beq"},
      di::register_rw_test{ reg_set{zero, pc}, reg_set{pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, !is_conditional, !is_indirect, !is_fallthrough, is_branch, !is_return}
      }
    },
    { // beq zero, zero, 1024
      {0x63,0x00,0x00,0x40},
      di::opcode_test{riscv64_op_beq, "beq"},
      di::register_rw_test{ reg_set{zero, pc}, reg_set{pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, !is_conditional, !is_indirect, !is_fallthrough, is_branch, !is_return}
      }
    },
    { // bne a3, a4, -2
      {0xe3,0x9f,0xe6,0xfe},
      di::opcode_test{riscv64_op_bne, "bne"},
      di::register_rw_test{ reg_set{a3, a4, pc}, reg_set{pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // bne t3, t3, 100
      {0x63,0x12,0xce,0x07},
      di::opcode_test{riscv64_op_bne, "bne"},
      di::register_rw_test{ reg_set{t3, pc}, reg_set{pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // blt a5, a6, 12
      {0x63,0xc6,0x07,0x01},
      di::opcode_test{riscv64_op_blt, "blt"},
      di::register_rw_test{ reg_set{a5, a6, pc}, reg_set{pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // bge s4, s5, 24
      {0x63,0x5c,0x5a,0x01},
      di::opcode_test{riscv64_op_bge, "bge"},
      di::register_rw_test{ reg_set{s4, s5, pc}, reg_set{pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // bltu s6, a7, -24
      {0xe3,0x64,0x1b,0xff},
      di::opcode_test{riscv64_op_bltu, "bltu"},
      di::register_rw_test{ reg_set{s6, a7, pc}, reg_set{pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // bgeu s8, s9, 72
      {0x63,0x74,0x9c,0x05},
      di::opcode_test{riscv64_op_bgeu, "bgeu"},
      di::register_rw_test{ reg_set{s8, s9, pc}, reg_set{pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // c.beqz a0, 6
      {0x19,0xc1},
      di::opcode_test{riscv64_op_beq, riscv64_op_c_beqz, "beq", "c.beqz"},
      di::register_rw_test{ reg_set{a0, zero, pc}, reg_set{pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // c.bnez a1, 44
      {0x95,0xe5},
      di::opcode_test{riscv64_op_bne, riscv64_op_c_bnez, "bne", "c.bnez"},
      di::register_rw_test{ reg_set{a1, zero, pc}, reg_set{pc} },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
  };
}
