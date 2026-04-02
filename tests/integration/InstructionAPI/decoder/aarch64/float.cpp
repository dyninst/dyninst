#include "Architecture.h"
#include "cft_tests.h"
#include "InstructionDecoder.h"
#include "memory_tests.h"
#include "opcode_tests.h"
#include "Register.h"
#include "register_tests.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/aarch64_regs.h"
#include "entryIDs.h"

#include <cstdlib>
#include <iostream>
#include <vector>

namespace di = Dyninst::InstructionAPI;

struct math_test {
  std::vector<unsigned char> bytes;
  di::opcode_test opcode;
  di::register_rw_test regs;
};

static std::vector<math_test> make_tests();

int main() {
  bool failed = false;
  int test_id = 0;
  const auto arch = Dyninst::Arch_aarch64;
  for(auto const &t : make_tests()) {
    test_id++;
    di::InstructionDecoder d(t.bytes.data(), t.bytes.size(), arch);
    auto insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode test " << test_id << '\n';
      failed = true;
      continue;
    }

    std::clog << "Verifying '" << insn.format() << "'\n";

    if(!di::verify(insn, t.opcode)) {
      failed = true;
    }
    if(!di::verify(insn, t.regs)) {
      failed = true;
    }

    std::clog << "\n";
  }
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

std::vector<math_test> make_tests() {
  auto nzcv = Dyninst::aarch64::nzcv;

  auto d0 = Dyninst::aarch64::d0;
  auto d1 = Dyninst::aarch64::d1;
  auto d2 = Dyninst::aarch64::d2;
  auto d4 = Dyninst::aarch64::d4;
  auto d5 = Dyninst::aarch64::d5;
  auto d8 = Dyninst::aarch64::d8;
  auto d9 = Dyninst::aarch64::d9;
  auto d10 = Dyninst::aarch64::d10;
  auto d11 = Dyninst::aarch64::d11;
  auto d15 = Dyninst::aarch64::d15;
  auto d16 = Dyninst::aarch64::d16;
  auto d29 = Dyninst::aarch64::d29;
  auto d30 = Dyninst::aarch64::d30;
  auto d31 = Dyninst::aarch64::d31;

  auto h5 = Dyninst::aarch64::h5;
  auto h8 = Dyninst::aarch64::h8;
  auto h31 = Dyninst::aarch64::h31;

  auto s0 = Dyninst::aarch64::s0;
  auto s1 = Dyninst::aarch64::s1;
  auto s2 = Dyninst::aarch64::s2;
  auto s3 = Dyninst::aarch64::s3;
  auto s4 = Dyninst::aarch64::s4;
  auto s5 = Dyninst::aarch64::s5;
  auto s8 = Dyninst::aarch64::s8;
  auto s10 = Dyninst::aarch64::s10;
  auto s11 = Dyninst::aarch64::s11;
  auto s13 = Dyninst::aarch64::s13;
  auto s16 = Dyninst::aarch64::s16;
  auto s20 = Dyninst::aarch64::s20;
  auto s31 = Dyninst::aarch64::s31;

  auto w2 = Dyninst::aarch64::w2;
  auto w8 = Dyninst::aarch64::w8;
  auto w9 = Dyninst::aarch64::w9;
  auto w10 = Dyninst::aarch64::w10;
  auto w11 = Dyninst::aarch64::w11;
  auto w30 = Dyninst::aarch64::w30;

  auto x0 = Dyninst::aarch64::x0;
  auto x1 = Dyninst::aarch64::x1;
  auto x30 = Dyninst::aarch64::x30;

  using reg_set = Dyninst::register_set;

  return {
    { // fabs d31, d31
      {0xff, 0xc3, 0x60, 0x1e},
      di::opcode_test(aarch64_op_fabs_float, "fabs d31, d31"),
      di::register_rw_test {
        reg_set{d31},
        reg_set{d31},
      },
    },
    { // fccmp d1, d2, #5, eq
      {0x25, 0x04, 0x62, 0x1e},
      di::opcode_test(aarch64_op_fccmp_float, "fccmp d1, d2, #5, eq"),
      di::register_rw_test {
        reg_set{d2, d1, nzcv},
        reg_set{nzcv},
      },
    },
    { // fccmp s20, s31, #8, ge
      {0x88, 0xa6, 0x3f, 0x1e},
      di::opcode_test(aarch64_op_fccmp_float, "fccmp s20, s31, #8, ge"),
      di::register_rw_test {
        reg_set{s31, s20, nzcv},
        reg_set{nzcv},
      },
    },
    { // fccmpe d10, d11, #9, pl
      {0x59, 0x55, 0x6b, 0x1e},
      di::opcode_test(aarch64_op_fccmpe_float, "fccmpe d10, d11, #9, pl"),
      di::register_rw_test {
        reg_set{d11, d10, nzcv},
        reg_set{nzcv},
      },
    },
    { // fcmp s8, #0.0
      {0x08, 0x21, 0x30, 0x1e},
      di::opcode_test(aarch64_op_fcmp_float, "fcmp s8, #0.00000000"),
      di::register_rw_test {
        reg_set{s8},
        reg_set{nzcv},
      },
    },
    { // fcmp d30, d31
      {0xc0, 0x23, 0x7f, 0x1e},
      di::opcode_test(aarch64_op_fcmp_float, "fcmp d30, d31"),
      di::register_rw_test {
        reg_set{d31, d30},
        reg_set{nzcv},
      },
    },
    { // fcmp s0, s31
      {0x00, 0x20, 0x3f, 0x1e},
      di::opcode_test(aarch64_op_fcmp_float, "fcmp s0, s31"),
      di::register_rw_test {
        reg_set{s31, s0},
        reg_set{nzcv},
      },
    },
    { // fcsel s1, s2, s3, mi
      {0x41, 0x4c, 0x23, 0x1e},
      di::opcode_test(aarch64_op_fcsel_float, "fcsel s1, s2, s3, mi"),
      di::register_rw_test {
        reg_set{s3, s2, nzcv},
        reg_set{s1},
      },
    },
    { // fcvt d0, h31
      {0xe0, 0xc3, 0xe2, 0x1e},
      di::opcode_test(aarch64_op_fcvt_float, "fcvt d0, h31"),
      di::register_rw_test {
        reg_set{h31},
        reg_set{d0},
      },
    },
    { // fcvt d2, s0
      {0x02, 0xc0, 0x22, 0x1e},
      di::opcode_test(aarch64_op_fcvt_float, "fcvt d2, s0"),
      di::register_rw_test {
        reg_set{s0},
        reg_set{d2},
      },
    },
    { // fcvt h31, d31
      {0xff, 0xc3, 0x63, 0x1e},
      di::opcode_test(aarch64_op_fcvt_float, "fcvt h31, d31"),
      di::register_rw_test {
        reg_set{d31},
        reg_set{h31},
      },
    },
    { // fcvt h8, s16
      {0x08, 0xc2, 0x23, 0x1e},
      di::opcode_test(aarch64_op_fcvt_float, "fcvt h8, s16"),
      di::register_rw_test {
        reg_set{s16},
        reg_set{h8},
      },
    },
    { // fcvt s1, d1
      {0x21, 0x40, 0x62, 0x1e},
      di::opcode_test(aarch64_op_fcvt_float, "fcvt s1, d1"),
      di::register_rw_test {
        reg_set{d1},
        reg_set{s1},
      },
    },
    { // fcvt s4, h5
      {0xa4, 0x40, 0xe2, 0x1e},
      di::opcode_test(aarch64_op_fcvt_float, "fcvt s4, h5"),
      di::register_rw_test {
        reg_set{h5},
        reg_set{s4},
      },
    },
    { // fcvtns w8, s5
      {0xa8, 0x00, 0x20, 0x1e},
      di::opcode_test(aarch64_op_fcvtns_float, "fcvtns w8, s5"),
      di::register_rw_test {
        reg_set{s5},
        reg_set{w8},
      },
    },
    { // fcvtzs w9, d9, #4
      {0x29, 0xf1, 0x58, 0x1e},
      di::opcode_test(aarch64_op_fcvtzs_float_fix, "fcvtzs w9, d9, #4"),
      di::register_rw_test {
        reg_set{d9},
        reg_set{w9},
      },
    },
    { // fcvtzs x30, d31, #1
      {0xfe, 0xff, 0x58, 0x9e},
      di::opcode_test(aarch64_op_fcvtzs_float_fix, "fcvtzs x30, d31, #1"),
      di::register_rw_test {
        reg_set{d31},
        reg_set{x30},
      },
    },
    { // fcvtzu w11, s0, #0xf
      {0x0b, 0xc4, 0x19, 0x1e},
      di::opcode_test(aarch64_op_fcvtzu_float_fix, "fcvtzu w11, s0, #0"), // WRONG: should be 'fcvtzu w11, s0, #0xf'
      di::register_rw_test {
        reg_set{s0},
        reg_set{w11},
      },
    },
    { // fcvtzu x1, s10, #8
      {0x41, 0xe1, 0x19, 0x9e},
      di::opcode_test(aarch64_op_fcvtzu_float_fix, "fcvtzu x1, s10, #8"),
      di::register_rw_test {
        reg_set{s10},
        reg_set{x1},
      },
    },
    { // fdiv d5, d10, d15
      {0x45, 0x19, 0x6f, 0x1e},
      di::opcode_test(aarch64_op_fdiv_float, "fdiv d5, d10, d15"),
      di::register_rw_test {
        reg_set{d15, d10},
        reg_set{d5},
      },
    },
    { // fmadd s0, s1, s2, s3
      {0x20, 0x0c, 0x02, 0x1f},
      di::opcode_test(aarch64_op_fmadd_float, "fmadd s0, s1, s2, s3"),
      di::register_rw_test {
        reg_set{s2, s3, s1},
        reg_set{s0},
      },
    },
    { // fmax s8, s16, s0
      {0x08, 0x4a, 0x20, 0x1e},
      di::opcode_test(aarch64_op_fmax_float, "fmax s8, s16, s0"),
      di::register_rw_test {
        reg_set{s0, s16},
        reg_set{s8},
      },
    },
    { // fmov d31, #31.00
      {0x1f, 0xf0, 0x67, 0x1e},
      di::opcode_test(aarch64_op_fmov_float_imm, "fmov d31, #31.00000000"),
      di::register_rw_test {
        reg_set{},
        reg_set{d31},
      },
    },
    { // fmov s0, #-3.00
      {0x00, 0x10, 0x31, 0x1e},
      di::opcode_test(aarch64_op_fmov_float_imm, "fmov s0, #-3.00000000"),
      di::register_rw_test {
        reg_set{},
        reg_set{s0},
      },
    },
    { // fmov s1, w30
      {0xc1, 0x03, 0x27, 0x1e},
      di::opcode_test(aarch64_op_fmov_float_gen, "fmov s1, w30"),
      di::register_rw_test {
        reg_set{w30},
        reg_set{s1},
      },
    },
    { // fmov s5, s10
      {0x45, 0x41, 0x20, 0x1e},
      di::opcode_test(aarch64_op_fmov_float, "fmov s5, s10"),
      di::register_rw_test {
        reg_set{s10},
        reg_set{s5},
      },
    },
    { // fmsub d2, d4, d8, d16
      {0x82, 0xc0, 0x48, 0x1f},
      di::opcode_test(aarch64_op_fmsub_float, "fmsub d2, d4, d8, d16"),
      di::register_rw_test {
        reg_set{d8, d16, d4},
        reg_set{d2},
      },
    },
    { // fmul s0, s1, s2
      {0x20, 0x08, 0x22, 0x1e},
      di::opcode_test(aarch64_op_fmul_float, "fmul s0, s1, s2"),
      di::register_rw_test {
        reg_set{s2, s1},
        reg_set{s0},
      },
    },
    { // fminnm s1, s1, s1
      {0x21, 0x78, 0x21, 0x1e},
      di::opcode_test(aarch64_op_fminnm_float, "fminnm s1, s1, s1"),
      di::register_rw_test {
        reg_set{s1, s1},
        reg_set{s1},
      },
    },
    { // fnmadd s10, s11, s11, s13
      {0x6a, 0x35, 0x2b, 0x1f},
      di::opcode_test(aarch64_op_fnmadd_float, "fnmadd s10, s11, s11, s13"),
      di::register_rw_test {
        reg_set{s11, s13, s11},
        reg_set{s10},
      },
    },
    { // fnmsub d8, d4, d2, d1
      {0x88, 0x84, 0x62, 0x1f},
      di::opcode_test(aarch64_op_fnmsub_float, "fnmsub d8, d4, d2, d1"),
      di::register_rw_test {
        reg_set{d2, d1, d4},
        reg_set{d8},
      },
    },
    { // frintp d0, d2
      {0x40, 0xc0, 0x64, 0x1e},
      di::opcode_test(aarch64_op_frintp_float, "frintp d0, d2"),
      di::register_rw_test {
        reg_set{d2},
        reg_set{d0},
      },
    },
    { // fsub d29, d30, d31
      {0xdd, 0x3b, 0x7f, 0x1e},
      di::opcode_test(aarch64_op_fsub_float, "fsub d29, d30, d31"),
      di::register_rw_test {
        reg_set{d31, d30},
        reg_set{d29},
      },
    },
    { // scvtf s0, w30, #27
      {0xc0, 0x97, 0x02, 0x1e},
      di::opcode_test(aarch64_op_scvtf_float_fix, "scvtf s0, w30, #1"), // WRONG: should be 'scvtf s0, w30, #27'
      di::register_rw_test {
        reg_set{w30},
        reg_set{s0},
      },
    },
    { // scvtf s5, w10, #0x10
      {0x45, 0xc1, 0x02, 0x1e},
      di::opcode_test(aarch64_op_scvtf_float_fix, "scvtf s5, w10, #10"),
      di::register_rw_test {
        reg_set{w10},
        reg_set{s5},
      },
    },
    { // ucvtf d1, x0, #55
      {0x01, 0x24, 0x43, 0x9e},
      di::opcode_test(aarch64_op_ucvtf_float_fix, "ucvtf d1, x0, #37"), // WRONG: should be 'ucvtf d1, x0, #55'
      di::register_rw_test {
        reg_set{x0},
        reg_set{d1},
      },
    },
    { // ucvtf d8, w2, #31
      {0x48, 0x84, 0x43, 0x1e},
      di::opcode_test(aarch64_op_ucvtf_float_fix, "ucvtf d8, w2, #1"), // WRONG: should be 'ucvtf d8, w2, #31'
      di::register_rw_test {
        reg_set{w2},
        reg_set{d8},
      },
    },
  };
}
