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
 *  Chapter 21: "D" Extension for Double-Precision Floating-Point, Version 2.2
 */

namespace di = Dyninst::InstructionAPI;

struct rv64f_tests {
  std::vector<unsigned char> rawBytes;
  di::opcode_test opcodes;
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

std::vector<rv64f_tests> make_tests64() {
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

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    // FP Loads / Stores
    { // fld f0, 0(zero)
      {0x07,0x30,0x00,0x00},
      di::opcode_test{riscv64_op_fld, "fld"},
      di::register_rw_test{ reg_set{zero}, reg_set{f0} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{zero}, reg_set{} } }
    },
    { // fld f23, 0(ra)
      {0x87,0xbb,0x00,0x00},
      di::opcode_test{riscv64_op_fld, "fld"},
      di::register_rw_test{ reg_set{ra}, reg_set{f23} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{ra}, reg_set{} } }
    },
    { // fsd f1, 8(s2)
      {0x27,0x34,0x19,0x00},
      di::opcode_test{riscv64_op_fsd, "fsd"},
      di::register_rw_test{ reg_set{f1, s2}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{s2} } }
    },
    { // fsd f24, -14(t0)
      {0x27,0xb9,0x82,0xff},
      di::opcode_test{riscv64_op_fsd, "fsd"},
      di::register_rw_test{ reg_set{f24, t0}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{t0} } }
    },

    // FP Move to/from Integer Registers
    { // fmv.x.d a0, f2
      {0x53,0x05,0x01,0xe2},
      di::opcode_test{riscv64_op_fmv_x_d, "fmv.x.d"},
      di::register_rw_test{ reg_set{f2}, reg_set{a0} },
      di::mem_test{}
    },
    { // fmv.x.d sp, f28
      {0x53,0x01,0x0e,0xe2},
      di::opcode_test{riscv64_op_fmv_x_d, "fmv.x.d"},
      di::register_rw_test{ reg_set{f28}, reg_set{sp} },
      di::mem_test{}
    },
    { // fmv.d.x f3, a1
      {0xd3,0x81,0x05,0xf2},
      di::opcode_test{riscv64_op_fmv_d_x, "fmv.d.x"},
      di::register_rw_test{ reg_set{a1}, reg_set{f3} },
      di::mem_test{}
    },
    { // fmv.d.x f25, s11
      {0xd3,0x8c,0x0d,0xf2},
      di::opcode_test{riscv64_op_fmv_d_x, "fmv.d.x"},
      di::register_rw_test{ reg_set{s11}, reg_set{f25} },
      di::mem_test{}
    },

    // FP Arithmetic
    { // fadd.d f4, f5, f6
      {0x53,0xf2,0x62,0x02},
      di::opcode_test{riscv64_op_fadd_d, "fadd.d"},
      di::register_rw_test{ reg_set{f5, f6}, reg_set{f4} },
      di::mem_test{}
    },
    { // fsub.d f7, f8, f9
      {0xd3,0x73,0x94,0x0a},
      di::opcode_test{riscv64_op_fsub_d, "fsub.d"},
      di::register_rw_test{ reg_set{f8, f9}, reg_set{f7} },
      di::mem_test{}
    },
    { // fmul.d f8, f18, f19
      {0x53,0x74,0x39,0x13},
      di::opcode_test{riscv64_op_fmul_d, "fmul.d"},
      di::register_rw_test{ reg_set{f18, f19}, reg_set{f8} },
      di::mem_test{}
    },
    { // fdiv.d f9, f20, f21
      {0xd3,0x74,0x5a,0x1b},
      di::opcode_test{riscv64_op_fdiv_d, "fdiv.d"},
      di::register_rw_test{ reg_set{f20, f21}, reg_set{f9} },
      di::mem_test{}
    },
    { // fsqrt.d f10, f22
      {0x53,0x75,0x0b,0x5a},
      di::opcode_test{riscv64_op_fsqrt_d, "fsqrt.d"},
      di::register_rw_test{ reg_set{f22}, reg_set{f10} },
      di::mem_test{}
    },

    // FP Sign Manipulation
    { // fsgnj.d f11, f0, f1
      {0xd3,0x05,0x10,0x22},
      di::opcode_test{riscv64_op_fsgnj_d, "fsgnj.d"},
      di::register_rw_test{ reg_set{f0, f1}, reg_set{f11} },
      di::mem_test{}
    },
    { // fsgnjn.d f10, f11, f12
      {0x53,0x95,0xc5,0x22},
      di::opcode_test{riscv64_op_fsgnjn_d, "fsgnjn.d"},
      di::register_rw_test{ reg_set{f11, f12}, reg_set{f10} },
      di::mem_test{}
    },
    { // fsgnjx.d f13, f14, f15
      {0xd3,0x26,0xf7,0x22},
      di::opcode_test{riscv64_op_fsgnjx_d, "fsgnjx.d"},
      di::register_rw_test{ reg_set{f14, f15}, reg_set{f13} },
      di::mem_test{}
    },

    // FP Comparisons
    { // feq.d a2, f16, f17
      {0x53,0x26,0x18,0xa3},
      di::opcode_test{riscv64_op_feq_d, "feq.d"},
      di::register_rw_test{ reg_set{f16, f17}, reg_set{a2} },
      di::mem_test{}
    },
    { // feq.d s7, f31, f29
      {0xd3,0xab,0xdf,0xa3},
      di::opcode_test{riscv64_op_feq_d, "feq.d"},
      di::register_rw_test{ reg_set{f31, f29}, reg_set{s7} },
      di::mem_test{}
    },
    { // flt.d a3, f0, f1
      {0xd3,0x16,0x10,0xa2},
      di::opcode_test{riscv64_op_flt_d, "flt.d"},
      di::register_rw_test{ reg_set{f0, f1}, reg_set{a3} },
      di::mem_test{}
    },
    { // flt.d s9, f4, f26
      {0xd3,0x1c,0xa2,0xa3},
      di::opcode_test{riscv64_op_flt_d, "flt.d"},
      di::register_rw_test{ reg_set{f4, f26}, reg_set{s9} },
      di::mem_test{}
    },
    { // fle.d a4, f2, f3
      {0x53,0x07,0x31,0xa2},
      di::opcode_test{riscv64_op_fle_d, "fle.d"},
      di::register_rw_test{ reg_set{f2, f3}, reg_set{a4} },
      di::mem_test{}
    },
    { // fle.d gp, f27, f16
      {0xd3,0x81,0x0d,0xa3},
      di::opcode_test{riscv64_op_fle_d, "fle.d"},
      di::register_rw_test{ reg_set{f27, f16}, reg_set{gp} },
      di::mem_test{}
    },

    // FP Conversions (Word / Long, Signed / Unsigned)
    { // fcvt.d.w f0, a5
      {0x53,0x80,0x07,0xd2},
      di::opcode_test{riscv64_op_fcvt_d_w, "fcvt.d.w"},
      di::register_rw_test{ reg_set{a5}, reg_set{f0} },
      di::mem_test{}
    },
    { // fcvt.d.w f30, s6
      {0x53,0x0f,0x0b,0xd2},
      di::opcode_test{riscv64_op_fcvt_d_w, "fcvt.d.w"},
      di::register_rw_test{ reg_set{s6}, reg_set{f30} },
      di::mem_test{}
    },
    { // fcvt.d.wu f1, a6
      {0xd3,0x00,0x18,0xd2},
      di::opcode_test{riscv64_op_fcvt_d_wu, "fcvt.d.wu"},
      di::register_rw_test{ reg_set{a6}, reg_set{f1} },
      di::mem_test{}
    },
    { // fcvt.d.wu f7, tp
      {0xd3,0x03,0x12,0xd2},
      di::opcode_test{riscv64_op_fcvt_d_wu, "fcvt.d.wu"},
      di::register_rw_test{ reg_set{tp}, reg_set{f7} },
      di::mem_test{}
    },
    { // fcvt.w.d a7, f4
      {0xd3,0x78,0x02,0xc2},
      di::opcode_test{riscv64_op_fcvt_w_d, "fcvt.w.d"},
      di::register_rw_test{ reg_set{f4}, reg_set{a7} },
      di::mem_test{}
    },
    { // fcvt.w.d t4, f20
      {0xd3,0x7e,0x0a,0xc2},
      di::opcode_test{riscv64_op_fcvt_w_d, "fcvt.w.d"},
      di::register_rw_test{ reg_set{f20}, reg_set{t4} },
      di::mem_test{}
    },
    { // fcvt.wu.d s0, f5
      {0x53,0xf4,0x12,0xc2},
      di::opcode_test{riscv64_op_fcvt_wu_d, "fcvt.wu.d"},
      di::register_rw_test{ reg_set{f5}, reg_set{s0} },
      di::mem_test{}
    },
    { // fcvt.wu.d t6, f1
      {0xd3,0xff,0x10,0xc2},
      di::opcode_test{riscv64_op_fcvt_wu_d, "fcvt.w.d"},
      di::register_rw_test{ reg_set{f1}, reg_set{t6} },
      di::mem_test{}
    },
    { // fcvt.d.l f6, s1
      {0x53,0xf3,0x24,0xd2},
      di::opcode_test{riscv64_op_fcvt_d_l, "fcvt.d.l"},
      di::register_rw_test{ reg_set{s1}, reg_set{f6} },
      di::mem_test{}
    },
    { // fcvt.d.l f29, s8
      {0xd3,0x7e,0x2c,0xd2},
      di::opcode_test{riscv64_op_fcvt_d_l, "fcvt.d.l"},
      di::register_rw_test{ reg_set{s8}, reg_set{f29} },
      di::mem_test{}
    },
    { // fcvt.d.lu f7, t1
      {0xd3,0x73,0x33,0xd2},
      di::opcode_test{riscv64_op_fcvt_d_lu, "fcvt.d.lu"},
      di::register_rw_test{ reg_set{t1}, reg_set{f7} },
      di::mem_test{}
    },
    { // fcvt.d.lu f13, s10
      {0xd3,0x76,0x3d,0xd2},
      di::opcode_test{riscv64_op_fcvt_d_lu, "fcvt.d.lu"},
      di::register_rw_test{ reg_set{s10}, reg_set{f13} },
      di::mem_test{}
    },
    { // fcvt.l.d t5, f3
      {0x53,0xff,0x21,0xc2},
      di::opcode_test{riscv64_op_fcvt_l_d, "fcvt.l.d"},
      di::register_rw_test{ reg_set{f3}, reg_set{t5} },
      di::mem_test{}
    },
    { // fcvt.l.d s3, f8
      {0xd3,0x79,0x24,0xc2},
      di::opcode_test{riscv64_op_fcvt_l_d, "fcvt.l.d"},
      di::register_rw_test{ reg_set{f8}, reg_set{s3} },
      di::mem_test{}
    },
    { // fcvt.lu.d s4, f9
      {0x53,0xfa,0x34,0xc2},
      di::opcode_test{riscv64_op_fcvt_lu_d, "fcvt.lu.d"},
      di::register_rw_test{ reg_set{f9}, reg_set{s4} },
      di::mem_test{}
    },
    { // fcvt.lu.d t3, f21
      {0x53,0xfe,0x3a,0xc2},
      di::opcode_test{riscv64_op_fcvt_lu_d, "fcvt.lu.d"},
      di::register_rw_test{ reg_set{f21}, reg_set{t3} },
      di::mem_test{}
    },

    // Cross-Precision Conversion
    { // fcvt.s.d f10, f11
      {0x53,0xf5,0x15,0x40},
      di::opcode_test{riscv64_op_fcvt_s_d, "fcvt.s.d"},
      di::register_rw_test{ reg_set{f11}, reg_set{f10} },
      di::mem_test{}
    },
    { // fcvt.d.s f0, f1
      {0x53,0x80,0x00,0x42},
      di::opcode_test{riscv64_op_fcvt_d_s, "fcvt.d.s"},
      di::register_rw_test{ reg_set{f1}, reg_set{f0} },
      di::mem_test{}
    },

    // FP Classify
    { // fclass.d t2, f2
      {0xd3,0x13,0x01,0xe2},
      di::opcode_test{riscv64_op_fclass_d, "fclass.d"},
      di::register_rw_test{ reg_set{f2}, reg_set{t2} },
      di::mem_test{}
    },
    { // fclass.d s5, f31
      {0xd3,0x9a,0x0f,0xe2},
      di::opcode_test{riscv64_op_fclass_d, "fclass.d"},
      di::register_rw_test{ reg_set{f31}, reg_set{s5} },
      di::mem_test{}
    },
  };
}
