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
  auto wsp = Dyninst::aarch64::wsp;
  auto sp = Dyninst::aarch64::sp;

  auto w0 = Dyninst::aarch64::w0;
  auto w1 = Dyninst::aarch64::w1;
  auto w2 = Dyninst::aarch64::w2;
  auto w3 = Dyninst::aarch64::w3;
  auto w4 = Dyninst::aarch64::w4;
  auto w5 = Dyninst::aarch64::w5;
  auto w8 = Dyninst::aarch64::w8;
  auto w10 = Dyninst::aarch64::w10;
  auto w11 = Dyninst::aarch64::w11;
  auto w12 = Dyninst::aarch64::w12;
  auto w14 = Dyninst::aarch64::w14;
  auto w16 = Dyninst::aarch64::w16;
  auto w24 = Dyninst::aarch64::w24;
  auto w29 = Dyninst::aarch64::w29;
  auto w30 = Dyninst::aarch64::w30;

  auto x0 = Dyninst::aarch64::x0;
  auto x1 = Dyninst::aarch64::x1;
  auto x5 = Dyninst::aarch64::x5;
  auto x7 = Dyninst::aarch64::x7;
  auto x9 = Dyninst::aarch64::x9;
  auto x10 = Dyninst::aarch64::x10;
  auto x11 = Dyninst::aarch64::x11;
  auto x12 = Dyninst::aarch64::x12;
  auto x20 = Dyninst::aarch64::x20;
  auto x21 = Dyninst::aarch64::x21;
  auto x22 = Dyninst::aarch64::x22;
  auto x25 = Dyninst::aarch64::x25;
  auto x30 = Dyninst::aarch64::x30;

  using reg_set = Dyninst::register_set;

  return {
    { // and w1, w2, w3
      {0x41, 0x00, 0x03, 0x0a},
      di::opcode_test(aarch64_op_and_log_shift, "and w1, w2, w3, lsl #0"),
      di::register_rw_test {
        reg_set{w3, w2},
        reg_set{w1},
      },
    },
    { // and wsp, w24, #63
      {0x1f, 0x1f, 0x2a, 0x12},
      di::opcode_test(aarch64_op_and_log_imm, "and wsp, w24, #3"), // WRONG: should be 'and wsp, w24, #0x3fc00000'
      di::register_rw_test {
        reg_set{w24},
        reg_set{wsp},
      },
    },
    { // ands w5, w10, #9
      {0x45, 0x25, 0x00, 0x72},
      di::opcode_test(aarch64_op_ands_log_imm, "ands w5, w10, #3"), // WRONG: should be 'ands w5, w10, #0x3ff'
      di::register_rw_test {
        reg_set{w10},
        reg_set{w5, nzcv},
      },
    },
    { // asrv x7, x9, x11
      {0x27, 0x29, 0xcb, 0x9a},
      di::opcode_test(aarch64_op_asr_asrv, "asr x7, x9, x11"),  // WRONG: should be 'asrv x7, x9, x11'
      di::register_rw_test {
        reg_set{x11, x9},
        reg_set{x7},
      },
    },
    { // bfi x0, x30, #1, #1
      {0xc0, 0x03, 0x7f, 0xb3},
      di::opcode_test(aarch64_op_bfi_bfm, "bfi x0, x30, #1, #1"),
      di::register_rw_test {
        reg_set{x30},
        reg_set{x0},
      },
    },
    { // bfxil  x0, lr, #0, #2
      {0xc0, 0x07, 0x40, 0xb3},
      di::opcode_test(aarch64_op_bfxil_bfm, "bfi"), // WRONG: should be 'bfxil x0, x30, #0, #2'
      di::register_rw_test {
        reg_set{x30},
        reg_set{x0},
      },
    },
    { // bic x0, x5, x10, lsl #5
      {0xa0, 0x14, 0x2a, 0x8a},
      di::opcode_test(aarch64_op_bic_log_shift, "bic x0, x5, x10, lsl #5"),
      di::register_rw_test {
        reg_set{x10, x5},
        reg_set{x0},
      },
    },
    { // bics x1, x1, x1, ror #8
      {0x21, 0x20, 0xe1, 0xea},
      di::opcode_test(aarch64_op_bics, "bics x1, x1, x1 ROR #8"),  // WRONG: should be 'bics x1, x1, x1, ror #8'
      di::register_rw_test {
        reg_set{x1, x1},
        reg_set{x1, nzcv},
      },
    },
    { // cls x11, x12
      {0x8b, 0x15, 0xc0, 0xda},
      di::opcode_test(aarch64_op_cls_int, "cls x11, x12"),
      di::register_rw_test {
        reg_set{x12},
        reg_set{x11},
      },
    },
    { // clz w30, w29
      {0xbe, 0x13, 0xc0, 0x5a},
      di::opcode_test(aarch64_op_clz_int, "clz w30, w29"),
      di::register_rw_test {
        reg_set{w29},
        reg_set{w30},
      },
    },
    { // eor x20, x21, x22, asr #2
      {0xb4, 0x0a, 0x96, 0xca},
      di::opcode_test(aarch64_op_eor_log_shift, "eor x20, x21, x22 ASR #2"),  // WRONG: should be 'eor x20, x21, x22, asr #2'
      di::register_rw_test {
        reg_set{x22, x21},
        reg_set{x20},
      },
    },
    { // eor x20, x25, #0x1ffffffffffe
      {0x34, 0xaf, 0x7f, 0xd2},
      di::opcode_test(aarch64_op_eor_log_imm, "eor x20, x25, #1"),  // WRONG: should be 'eor x20, x25, #0x1ffffffffffe'
      di::register_rw_test {
        reg_set{x25},
        reg_set{x20},
      },
    },
    { // lsl w5, w8, w11
      {0x05, 0x21, 0xcb, 0x1a},
      di::opcode_test(aarch64_op_lsl_lslv, "lsl w5, w8, w11"),
      di::register_rw_test {
        reg_set{w11, w8},
        reg_set{w5},
      },
    },
    { // orn w0, w0, w2, lsr #10
      {0x00, 0x28, 0x62, 0x2a},
      di::opcode_test(aarch64_op_orn_log_shift, "orn w0, w0, w2 LSR #0"), // WRONG: should be 'orn w0, w0, w2, lsr #10'
      di::register_rw_test {
        reg_set{w2, w0},
        reg_set{w0},
      },
    },
    { // orr sp, lr, #0x100000001
      {0xdf, 0x03, 0x00, 0xb2},
      di::opcode_test(aarch64_op_orr_log_imm, "orr sp, x30, #100000001"),
      di::register_rw_test {
        reg_set{x30},
        reg_set{sp},
      },
    },
    { // rbit w1, w2
      {0x41, 0x00, 0xc0, 0x5a},
      di::opcode_test(aarch64_op_rbit_int, "rbit w1, w2"),
      di::register_rw_test {
        reg_set{w2},
        reg_set{w1},
      },
    },
    { // ror w16, w0, w4
      {0x10, 0x2c, 0xc4, 0x1a},
      di::opcode_test(aarch64_op_ror_rorv, "ror w16, w0, w4"),
      di::register_rw_test {
        reg_set{w4, w0},
        reg_set{w16},
      },
    },
    { // sbfiz w12, w14, #7, #11
      {0xcc, 0x29, 0x19, 0x13},
      di::opcode_test(aarch64_op_sbfiz_sbfm, "sbfiz w12, w14, #7, #0"),  // WRONG: should be 'sbfiz w12, w14, #7, #11'
      di::register_rw_test {
        reg_set{w14},
        reg_set{w12},
      },
    },
    { // ubfx x20, x0, #1, #8
      {0x14, 0x20, 0x41, 0xd3},
      di::opcode_test(aarch64_op_ubfx_ubfm, "lsl"),  // WRONG: should be 'ubfx x20, x0, #1, #8'
      di::register_rw_test {
        reg_set{x0},
        reg_set{x20},
      },
    },

  };
}
