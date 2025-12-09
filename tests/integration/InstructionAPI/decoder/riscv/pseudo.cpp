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
 *  RISC-V ASM Manual
 *  A listing of standard RISC-V pseudoinstructions
 *  https://github.com/riscv-non-isa/riscv-asm-manual/blob/main/src/asm-manual.adoc#a-listing-of-standard-risc-v-pseudoinstructions
 */

namespace di = Dyninst::InstructionAPI;

struct pseudo_tests {
  std::vector<unsigned char> rawBytes;
  di::opcode_test opcodes;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<pseudo_tests> make_tests64();
static bool run(Dyninst::Architecture, std::vector<pseudo_tests> const &);

int main() {
  bool ok = run(Dyninst::Arch_riscv64, make_tests64());
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<pseudo_tests> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'pseudo' in " << sarch << " mode\n";
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

std::vector<pseudo_tests> make_tests64() {
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
  auto f23 = Dyninst::riscv64::f23;
  auto f24 = Dyninst::riscv64::f24;
  auto f25 = Dyninst::riscv64::f25;
  auto f26 = Dyninst::riscv64::f26;
  auto f27 = Dyninst::riscv64::f27;
  auto f28 = Dyninst::riscv64::f28;
  auto f29 = Dyninst::riscv64::f29;
  auto f30 = Dyninst::riscv64::f30;
  auto f31 = Dyninst::riscv64::f31;

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // nop -> addi zero, zero, 0
      {0x13,0x00,0x00,0x00},
      di::opcode_test{riscv64_op_addi, "addi"},
      di::register_rw_test{ reg_set{zero}, reg_set{zero} },
      di::mem_test{}
    },
    { // mv s7, t6 -> addi s7, t6, 0
      {0x93,0x8b,0x0f,0x00},
      di::opcode_test{riscv64_op_addi, "addi"},
      di::register_rw_test{ reg_set{t6}, reg_set{s7} },
      di::mem_test{}
    },
    { // not t0, t1 -> xori t0, t1, -1
      {0x93,0x42,0xf3,0xff},
      di::opcode_test{riscv64_op_xori, "xori"},
      di::register_rw_test{ reg_set{t1}, reg_set{t0} },
      di::mem_test{}
    },
    { // neg s8, s1 -> sub s8, zero, s1
      {0x33,0x0c,0x90,0x40},
      di::opcode_test{riscv64_op_sub, "sub"},
      di::register_rw_test{ reg_set{s1, zero}, reg_set{s8} },
      di::mem_test{}
    },
    { // neg a6, s11 -> sub a6, zero, s11
      {0x33,0x08,0xb0,0x41},
      di::opcode_test{riscv64_op_sub, "sub"},
      di::register_rw_test{ reg_set{s11, zero}, reg_set{a6} },
      di::mem_test{}
    },
    { // negw s0, s1 -> subw s0, zero, s1
      {0x3b,0x04,0x90,0x40},
      di::opcode_test{riscv64_op_subw, "subw"},
      di::register_rw_test{ reg_set{s1, zero}, reg_set{s0} },
      di::mem_test{}
    },
    { // sext.w s2, s3 -> addiw s2, s3, 0
      {0x1b,0x89,0x09,0x00},
      di::opcode_test{riscv64_op_addiw, "addiw"},
      di::register_rw_test{ reg_set{s3}, reg_set{s2} },
      di::mem_test{}
    },
    { // zext.b s4, s5 -> andi s4, s5, 255
      {0x13,0xfa,0xfa,0x0f},
      di::opcode_test{riscv64_op_andi, "andi"},
      di::register_rw_test{ reg_set{s5}, reg_set{s4} },
      di::mem_test{}
    },
    { // seqz a0, tp -> sltiu a0, tp, 1
      {0x13,0x35,0x12,0x00},
      di::opcode_test{riscv64_op_sltiu, "sltiu"},
      di::register_rw_test{ reg_set{tp}, reg_set{a0} },
      di::mem_test{}
    },
    { // snez t0, t1 -> sltu t0, zero, t1
      {0xb3,0x32,0x60,0x00},
      di::opcode_test{riscv64_op_sltu, "sltu"},
      di::register_rw_test{ reg_set{zero, t1}, reg_set{t0} },
      di::mem_test{}
    },
    { // sltz a2, a3 -> slt a2, a3, zero
      {0x33,0xa6,0x06,0x00},
      di::opcode_test{riscv64_op_slt, "slt"},
      di::register_rw_test{ reg_set{zero, a3}, reg_set{a2} },
      di::mem_test{}
    },
    { // sgtz sp, s1 -> slt sp, zero, s1
      {0x33,0x21,0x90,0x00},
      di::opcode_test{riscv64_op_slt, "slt"},
      di::register_rw_test{ reg_set{zero, s1}, reg_set{sp} },
      di::mem_test{}
    },
    { // fmv.s f4, f5 -> fsgnj.s f4, f5, f5
      {0x53,0x82,0x52,0x20},
      di::opcode_test{riscv64_op_fsgnj_s, "fsgnj.s"},
      di::register_rw_test{ reg_set{f5}, reg_set{f4} },
      di::mem_test{}
    },
    { // fabs.s f6, f1 -> fsgnjx.s f6, f1, f1
      {0x53,0xa3,0x10,0x20},
      di::opcode_test{riscv64_op_fsgnjx_s, "fsgnjx.s"},
      di::register_rw_test{ reg_set{f1}, reg_set{f6} },
      di::mem_test{}
    },
    { // fneg.s f22, f11 -> fsgnjn.s f22, f11, f11
      {0x53,0x9b,0xb5,0x20},
      di::opcode_test{riscv64_op_fsgnjn_s, "fsgnjn.s"},
      di::register_rw_test{ reg_set{f11}, reg_set{f22} },
      di::mem_test{}
    },
    { // fgt.s a7, f8, f7 -> flt.s a7, f7, f8
      {0xd3,0x98,0x83,0xa0},
      di::opcode_test{riscv64_op_flt_s, "flt.s"},
      di::register_rw_test{ reg_set{f7, f8}, reg_set{a7} },
      di::mem_test{}
    },
    { // fgt.s t1, f19, f3 -> flt.s t1, f3, f19
      {0x53,0x93,0x31,0xa1},
      di::opcode_test{riscv64_op_flt_s, "flt.s"},
      di::register_rw_test{ reg_set{f3, f19}, reg_set{t1} },
      di::mem_test{}
    },
    { // fgt.s s9, f27, f13 -> flt.s s9, f13, f27
      {0xd3,0x9c,0xb6,0xa1},
      di::opcode_test{riscv64_op_flt_s, "flt.s"},
      di::register_rw_test{ reg_set{f13, f27}, reg_set{s9} },
      di::mem_test{}
    },
    { // fge.s a1, f23, f18 -> fle.s a1, f18, f23
      {0xd3,0x05,0x79,0xa1},
      di::opcode_test{riscv64_op_fle_s, "fle.s"},
      di::register_rw_test{ reg_set{f23, f18}, reg_set{a1} },
      di::mem_test{}
    },
    { // fge.s s6, f20, f21 -> fle.s a7, f21, f20
      {0x53,0x8b,0x4a,0xa1},
      di::opcode_test{riscv64_op_fle_s, "fle.s"},
      di::register_rw_test{ reg_set{f20, f21}, reg_set{s6} },
      di::mem_test{}
    },
    { // fge.s a4, f25, f30 -> fle.s a4, f25, f30
      {0x53,0x07,0x9f,0xa1},
      di::opcode_test{riscv64_op_fle_s, "fle.s"},
      di::register_rw_test{ reg_set{f25, f30}, reg_set{a4} },
      di::mem_test{}
    },
    { // fgt.d a0, f16, f28 -> flt.d a0, f28, f16
      {0x53,0x15,0x0e,0xa3},
      di::opcode_test{riscv64_op_flt_d, "flt.d"},
      di::register_rw_test{ reg_set{f16, f28}, reg_set{a0} },
      di::mem_test{}
    },
    { // fgt.d s3, f29, f24 -> flt.d s3, f24, f29
      {0xd3,0x19,0xdc,0xa3},
      di::opcode_test{riscv64_op_flt_d, "flt.d"},
      di::register_rw_test{ reg_set{f24, f29}, reg_set{s3} },
      di::mem_test{}
    },
    { // fge.d a1, f9, f26 -> fle.d a1, f26, f9
      {0xd3,0x05,0x9d,0xa2},
      di::opcode_test{riscv64_op_fle_d, "fle.d"},
      di::register_rw_test{ reg_set{f26, f9}, reg_set{a1} },
      di::mem_test{}
    },
    { // fge.d gp, f31, f17 -> fle.d gp, f17, f31
      {0xd3,0x81,0xf8,0xa3},
      di::opcode_test{riscv64_op_fle_d, "fle.d"},
      di::register_rw_test{ reg_set{f17, f31}, reg_set{gp} },
      di::mem_test{}
    },
    { // fmv.d f10, f0 -> fsgnj.d f10, f0, f0
      {0x53,0x05,0x00,0x22},
      di::opcode_test{riscv64_op_fsgnj_d, "sgnj.d"},
      di::register_rw_test{ reg_set{f0}, reg_set{f10} },
      di::mem_test{}
    },
    { // fabs.d f12, f2 -> fsgnjx.d f12, f2, f2
      {0x53,0x26,0x21,0x22},
      di::opcode_test{riscv64_op_fsgnjx_d, "fsgnjx.d"},
      di::register_rw_test{ reg_set{f2}, reg_set{f12} },
      di::mem_test{}
    },
    { // fneg.d f14, f15 -> fsgnjn.d f14, f15, f15
      {0x53,0x97,0xf7,0x22},
      di::opcode_test{riscv64_op_fsgnjn_d, "fsgnjn.d"},
      di::register_rw_test{ reg_set{f15}, reg_set{f14} },
      di::mem_test{}
    },
    { // beqz t0, 4 -> beq t0, zero, 4
      {0x63,0x82,0x02,0x00},
      di::opcode_test{riscv64_op_beq, "beq"},
      di::register_rw_test{ reg_set{t0, zero, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // bnez t1, -4 -> bne t1, zero, -4
      {0xe3,0x1e,0x03,0xfe},
      di::opcode_test{riscv64_op_bne, "bne"},
      di::register_rw_test{ reg_set{t1, zero, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // blez s0, 8 -> bge zero, s0, 8
      {0x63,0x54,0x80,0x00},
      di::opcode_test{riscv64_op_bge, "bge"},
      di::register_rw_test{ reg_set{zero, s0, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // bgez s1, -8 -> bge s1, zero, -8
      {0xe3,0xdc,0x04,0xfe},
      di::opcode_test{riscv64_op_bge, "bge"},
      di::register_rw_test{ reg_set{s1, zero, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // bltz a0, 16 -> blt a0, zero, 16
      {0x63,0x48,0x05,0x00},
      di::opcode_test{riscv64_op_blt, "blt"},
      di::register_rw_test{ reg_set{a0, zero, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // bgtz a1, -16 -> blt zero, a1, -16
      {0xe3,0x48,0xb0,0xfe},
      di::opcode_test{riscv64_op_blt, "blt"},
      di::register_rw_test{ reg_set{zero, a1, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // bgt a2, a3, 32 -> blt a3, a2, 32
      {0x63,0xc0,0xc6,0x02},
      di::opcode_test{riscv64_op_blt, "blt"},
      di::register_rw_test{ reg_set{a2, a3, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // ble a4, a5, -32 -> bge a5, a4, -32
      {0xe3,0xd0,0xe7,0xfe},
      di::opcode_test{riscv64_op_bge, "bge"},
      di::register_rw_test{ reg_set{a4, a5, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // bgtu t2, t3, 64 -> bltu t3, t2, 64
      {0x63,0x60,0x7e,0x04},
      di::opcode_test{riscv64_op_bltu, "bltu"},
      di::register_rw_test{ reg_set{t2, t3, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // bleu t4, t5, -64 -> bgeu t5, t4, -64
      {0xe3,0x70,0xdf,0xfd},
      di::opcode_test{riscv64_op_bgeu, "bgeu"},
      di::register_rw_test{ reg_set{t4, t5, pc}, reg_set{pc} },
      di::mem_test{}
    },
    { // j 50 -> jal x0, 50
      {0x6f,0x00,0x20,0x03},
      di::opcode_test{riscv64_op_jal, "jal"},
      di::register_rw_test{ reg_set{pc}, reg_set{zero, pc} },
      di::mem_test{}
    },
    { // jal -50 -> jal ra, -50
      {0xef,0xf0,0xff,0xfc},
      di::opcode_test{riscv64_op_jal, "jal"},
      di::register_rw_test{ reg_set{pc}, reg_set{ra, pc} },
      di::mem_test{}
    },
    { // jr s9 -> jalr x0, s9, 0
      {0x67,0x80,0x0c,0x00},
      di::opcode_test{riscv64_op_jalr, "jalr"},
      di::register_rw_test{ reg_set{s9}, reg_set{pc, zero} },
      di::mem_test{}
    },
    { // jalr s10 -> jalr ra, s10, 0
      {0xe7,0x00,0x0d,0x00},
      di::opcode_test{riscv64_op_jalr, "jalr"},
      di::register_rw_test{ reg_set{s10}, reg_set{ra, pc} },
      di::mem_test{}
    },
    { // ret -> jalr x0, ra, 0
      {0x67,0x80,0x00,0x00},
      di::opcode_test{riscv64_op_jalr, "jalr"},
      di::register_rw_test{ reg_set{ra}, reg_set{zero, pc} },
      di::mem_test{},
    },
  };
}
