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
 *  Chapter 20: "F" Extension for Single-Precision Floating-Point, Version 2.2
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
    { // flw ft0, 0(s1)
      {0x07,0xa0,0x04,0x00},
      di::opcode_test{riscv64_op_flw, "flw", "flw ft0, 0x0(s1)"},
      di::register_rw_test{ reg_set{s1}, reg_set{f0} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{s1}, reg_set{} } }
    },
    { // flw fs6, 0(sp)
      {0x07,0x2b,0x01,0x00},
      di::opcode_test{riscv64_op_flw, "flw", "flw fs6, 0x0(sp)"},
      di::register_rw_test{ reg_set{sp}, reg_set{f22} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{sp}, reg_set{} } }
    },
    { // flw ft2, -27(s3)
      {0x07,0xa1,0x59,0xfe},
      di::opcode_test{riscv64_op_flw, "flw", "flw ft2, 0xffffffe5(s3)"},
      di::register_rw_test{ reg_set{s3}, reg_set{f2} },
      di::mem_test{ reads_memory, !writes_memory, di::register_rw_test{ reg_set{s3}, reg_set{} } }
    },
    { // fsw ft1, 4(s0)
      {0x27,0x22,0x14,0x00},
      di::opcode_test{riscv64_op_fsw, "fsw", "fsw ft1, 0x4(s0)"},
      di::register_rw_test{ reg_set{f1, s0}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{s0} } }
    },
    { // fsw fs9, 4(t3)
      {0x27,0x22,0x9e,0x01},
      di::opcode_test{riscv64_op_fsw, "fsw", "fsw fs9, 0x4(t3)"},
      di::register_rw_test{ reg_set{f25, t3}, reg_set{} },
      di::mem_test{ !reads_memory, writes_memory, di::register_rw_test{ reg_set{}, reg_set{t3} } }
    },

    // FP Move to/from Integer Registers
    { // fmv.x.w a0, ft2
      {0x53,0x05,0x01,0xe0},
      di::opcode_test{riscv64_op_fmv_x_w, "fmv.x.w", "fmv.x.w a0, ft2"},
      di::register_rw_test{ reg_set{f2}, reg_set{a0} },
      di::mem_test{}
    },
    { // fmv.x.w s10, fs11
      {0x53,0x8d,0x0d,0xe0},
      di::opcode_test{riscv64_op_fmv_x_w, "fmv.x.w", "fmv.x.w s10, fs11"},
      di::register_rw_test{ reg_set{f27}, reg_set{s10} },
      di::mem_test{}
    },
    { // fmv.x.w s9, ft5
      {0xd3,0x8c,0x02,0xe0},
      di::opcode_test{riscv64_op_fmv_x_w, "fmv.x.w", "fmv.x.w s9, ft5"},
      di::register_rw_test{ reg_set{f5}, reg_set{s9} },
      di::mem_test{}
    },
    { // fmv.w.x ft3, a1
      {0xd3,0x81,0x05,0xf0},
      di::opcode_test{riscv64_op_fmv_w_x, "fmv.w.x", "fmv.w.x ft3, a1"},
      di::register_rw_test{ reg_set{a1}, reg_set{f3} },
      di::mem_test{}
    },
    { // fmv.w.x fs10, tp
      {0x53,0x0d,0x02,0xf0},
      di::opcode_test{riscv64_op_fmv_w_x, "fmv.w.x", "fmv.w.x fs10, tp"},
      di::register_rw_test{ reg_set{tp}, reg_set{f26} },
      di::mem_test{}
    },
    { // fmv.w.x fs6, s11
      {0x53,0x8b,0x0d,0xf0},
      di::opcode_test{riscv64_op_fmv_w_x, "fmv.w.x", "fmv.w.x fs6, s11"},
      di::register_rw_test{ reg_set{s11}, reg_set{f22} },
      di::mem_test{}
    },

    // FP Arithmetic
    { // fadd.s ft4, ft5, ft6
      {0x53,0xf2,0x62,0x00},
      di::opcode_test{riscv64_op_fadd_s, "fadd.s", "fadd.s ft4, ft5, ft6"},
      di::register_rw_test{ reg_set{f5, f6}, reg_set{f4} },
      di::mem_test{}
    },
    { // fsub.s ft7, fs0, fs1
      {0xd3,0x73,0x94,0x08},
      di::opcode_test{riscv64_op_fsub_s, "fsub.s", "fsub.s ft7, fs0, fs1"},
      di::register_rw_test{ reg_set{f8, f9}, reg_set{f7} },
      di::mem_test{}
    },
    { // fmul.s ft8, fs2, fs3
      {0x53,0x7e,0x39,0x11},
      di::opcode_test{riscv64_op_fmul_s, "fmul.s", "fmul.s ft8, fs2, fs3"},
      di::register_rw_test{ reg_set{f18, f19}, reg_set{f28} },
      di::mem_test{}
    },
    { // fdiv.s ft9, fs4, fs5
      {0xd3,0x7e,0x5a,0x19},
      di::opcode_test{riscv64_op_fdiv_s, "fdiv.s", "fdiv.s ft9, fs4, fs5"},
      di::register_rw_test{ reg_set{f20, f21}, reg_set{f29} },
      di::mem_test{}
    },
    { // fsqrt.s ft10, fs6
      {0x53,0x7f,0x0b,0x58},
      di::opcode_test{riscv64_op_fsqrt_s, "fsqrt.s", "fsqrt.s ft10, fs6"},
      di::register_rw_test{ reg_set{f22}, reg_set{f30} },
      di::mem_test{}
    },

    // FP Sign Manipulation
    { // fsgnj.s ft11, ft0, ft1
      {0xd3,0x0f,0x10,0x20},
      di::opcode_test{riscv64_op_fsgnj_s, "fsgnj.s", "fsgnj.s ft11, ft0, ft1"},
      di::register_rw_test{ reg_set{f0, f1}, reg_set{f31} },
      di::mem_test{}
    },
    { // fsgnjn.s fa0, fa1, fa2
      {0x53,0x95,0xc5,0x20},
      di::opcode_test{riscv64_op_fsgnjn_s, "fsgnjn.s", "fsgnjn.s fa0, fa1, fa2"},
      di::register_rw_test{ reg_set{f11, f12}, reg_set{f10} },
      di::mem_test{}
    },
    { // fsgnjx.s fa3, fa4, fa5
      {0xd3,0x26,0xf7,0x20},
      di::opcode_test{riscv64_op_fsgnjx_s, "fsgnjx.s", "fsgnjx.s fa3, fa4, fa5"},
      di::register_rw_test{ reg_set{f14, f15}, reg_set{f13} },
      di::mem_test{}
    },

    // FP Comparisons
    { // feq.s a2, fa6, fa7
      {0x53,0x26,0x18,0xa1},
      di::opcode_test{riscv64_op_feq_s, "feq.s", "feq.s a2, fa6, fa7"},
      di::register_rw_test{ reg_set{f16, f17}, reg_set{a2} },
      di::mem_test{}
    },
    { // feq.s t6, fa6, ft7
      {0xd3,0x2f,0x78,0xa0},
      di::opcode_test{riscv64_op_feq_s, "feq.s", "feq.s t6, fa6, ft7"},
      di::register_rw_test{ reg_set{f16, f7}, reg_set{t6} },
      di::mem_test{}
    },
    { // feq.s s4, fs7, fs8
      {0x53,0xaa,0x8b,0xa1},
      di::opcode_test{riscv64_op_feq_s, "feq.s", "feq.s s4, fs7, fs8"},
      di::register_rw_test{ reg_set{f23, f24}, reg_set{s4} },
      di::mem_test{}
    },
    { // flt.s a3, ft0, fs5
      {0xd3,0x16,0x50,0xa1},
      di::opcode_test{riscv64_op_flt_s, "flt.s", "flt.s a3, ft0, fs5"},
      di::register_rw_test{ reg_set{f0, f21}, reg_set{a3} },
      di::mem_test{}
    },
    { // flt.s s8, fa3, ft0
      {0x53,0x9c,0x06,0xa0},
      di::opcode_test{riscv64_op_flt_s, "flt.s", "flt.s s8, fa3, ft0"},
      di::register_rw_test{ reg_set{f13, f0}, reg_set{s8} },
      di::mem_test{}
    },
    { // flt.s t5, ft9, ft5
      {0x53,0x9f,0x5e,0xa0},
      di::opcode_test{riscv64_op_flt_s, "flt.s", "flt.s t5, ft9, ft5"},
      di::register_rw_test{ reg_set{f29, f5}, reg_set{t5} },
      di::mem_test{}
    },
    { // fle.s a4, ft2, ft3
      {0x53,0x07,0x31,0xa0},
      di::opcode_test{riscv64_op_fle_s, "fle.s", "fle.s a4, ft2, ft3"},
      di::register_rw_test{ reg_set{f2, f3}, reg_set{a4} },
      di::mem_test{}
    },
    { // fle.s ra, fs11, fa3
      {0xd3,0x80,0xdd,0xa0},
      di::opcode_test{riscv64_op_fle_s, "fle.s", "fle.s ra, fs11, fa3"},
      di::register_rw_test{ reg_set{f27, f13}, reg_set{ra} },
      di::mem_test{}
    },
    { // fle.s zero, ft10, ft2
      {0x53,0x00,0x2f,0xa0},
      di::opcode_test{riscv64_op_fle_s, "fle.s", "fle.s zero, ft10, ft2"},
      di::register_rw_test{ reg_set{f30, f2}, reg_set{zero} },
      di::mem_test{}
    },

    // FP Conversions
    { // fcvt.s.w ft0, a5
      {0x53,0xf0,0x07,0xd0},
      di::opcode_test{riscv64_op_fcvt_s_w, "fcvt.s.w", "fcvt.s.w ft0, a5"},
      di::register_rw_test{ reg_set{a5}, reg_set{f0} },
      di::mem_test{}
    },
    { // fcvt.s.w fs1, t0
      {0xd3,0xf4,0x02,0xd0},
      di::opcode_test{riscv64_op_fcvt_s_w, "fcvt.s.w", "fcvt.s.w fs1, t0"},
      di::register_rw_test{ reg_set{t0}, reg_set{f9} },
      di::mem_test{}
    },
    { // fcvt.s.w ft11, s1
      {0xd3,0xff,0x04,0xd0},
      di::opcode_test{riscv64_op_fcvt_s_w, "fcvt.s.w", "fcvt.s.w ft11, s1"},
      di::register_rw_test{ reg_set{s1}, reg_set{f31} },
      di::mem_test{}
    },
    { // fcvt.s.wu ft1, a6
      {0xd3,0x70,0x18,0xd0},
      di::opcode_test{riscv64_op_fcvt_s_wu, "fcvt.s.wu", "fcvt.s.wu ft1, a6"},
      di::register_rw_test{ reg_set{a6}, reg_set{f1} },
      di::mem_test{}
    },
    { // fcvt.s.wu ft6, s2
      {0x53,0x73,0x19,0xd0},
      di::opcode_test{riscv64_op_fcvt_s_wu, "fcvt.s.wu", "fcvt.s.wu ft6, s2"},
      di::register_rw_test{ reg_set{s2}, reg_set{f6} },
      di::mem_test{}
    },
    { // fcvt.s.wu ft8, s5
      {0x53,0xfe,0x1a,0xd0},
      di::opcode_test{riscv64_op_fcvt_s_wu, "fcvt.s.wu", "fcvt.s.wu ft8, s5"},
      di::register_rw_test{ reg_set{s5}, reg_set{f28} },
      di::mem_test{}
    },
    { // fcvt.w.s a7, ft4
      {0xd3,0x78,0x02,0xc0},
      di::opcode_test{riscv64_op_fcvt_w_s, "fcvt.w.s", "fcvt.w.s a7, ft4"},
      di::register_rw_test{ reg_set{f4}, reg_set{a7} },
      di::mem_test{}
    },
    { // fcvt.w.s t4, fa5
      {0xd3,0xfe,0x07,0xc0},
      di::opcode_test{riscv64_op_fcvt_w_s, "fcvt.w.s", "fcvt.w.s t4, fa5"},
      di::register_rw_test{ reg_set{f15}, reg_set{t4} },
      di::mem_test{}
    },
    { // fcvt.w.s t2, fs2
      {0xd3,0x73,0x09,0xc0},
      di::opcode_test{riscv64_op_fcvt_w_s, "fcvt.w.s", "fcvt.w.s t2, fs2"},
      di::register_rw_test{ reg_set{f18}, reg_set{t2} },
      di::mem_test{}
    },
    { // fcvt.wu.s s7, ft5
      {0xd3,0xfb,0x12,0xc0},
      di::opcode_test{riscv64_op_fcvt_wu_s, "fcvt.wu.s", "fcvt.wu.s s7, ft5"},
      di::register_rw_test{ reg_set{f5}, reg_set{s7} },
      di::mem_test{}
    },
    { // fcvt.wu.s s10, fa1
      {0x53,0xfd,0x15,0xc0},
      di::opcode_test{riscv64_op_fcvt_wu_s, "fcvt.wu.s", "fcvt.wu.s s10, fa1"},
      di::register_rw_test{ reg_set{f11}, reg_set{s10} },
      di::mem_test{}
    },
    { // fcvt.wu.s t1, fs7
      {0x53,0xf3,0x1b,0xc0},
      di::opcode_test{riscv64_op_fcvt_wu_s, "fcvt.wu.s", "fcvt.wu.s t1, fs7"},
      di::register_rw_test{ reg_set{f23}, reg_set{t1} },
      di::mem_test{}
    },

    // FP Classify
    { // fclass.s a0, ft6
      {0x53,0x15,0x03,0xe0},
      di::opcode_test{riscv64_op_fclass_s, "fclass.s", "fclass.s a0, ft6"},
      di::register_rw_test{ reg_set{f6}, reg_set{a0} },
      di::mem_test{}
    },
    { // fclass.s gp, fa4
      {0xd3,0x11,0x07,0xe0},
      di::opcode_test{riscv64_op_fclass_s, "fclass.s", "fclass.s gp, fa4"},
      di::register_rw_test{ reg_set{f14}, reg_set{gp} },
      di::mem_test{}
    },
    { // fclass.s s6, ft7
      {0x53,0x9b,0x03,0xe0},
      di::opcode_test{riscv64_op_fclass_s, "fclass.s", "fclass.s s6, ft7"},
      di::register_rw_test{ reg_set{f7}, reg_set{s6} },
      di::mem_test{}
    },
  };
}
