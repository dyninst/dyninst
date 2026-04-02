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
  auto sp = Dyninst::aarch64::sp;
  auto wsp = Dyninst::aarch64::wsp;
  auto w0 = Dyninst::aarch64::w0;
  auto w1 = Dyninst::aarch64::w1;
  auto w2 = Dyninst::aarch64::w2;
  auto w3 = Dyninst::aarch64::w3;
  auto w4 = Dyninst::aarch64::w4;
  auto w5 = Dyninst::aarch64::w5;
  auto w8 = Dyninst::aarch64::w8;
  auto w10 = Dyninst::aarch64::w10;
  auto w12 = Dyninst::aarch64::w12;
  auto w22 = Dyninst::aarch64::w22;
  auto w25 = Dyninst::aarch64::w25;
  auto x0 = Dyninst::aarch64::x0;
  auto x1 = Dyninst::aarch64::x1;
  auto x2 = Dyninst::aarch64::x2;
  auto x4 = Dyninst::aarch64::x4;
  auto x5 = Dyninst::aarch64::x5;
  auto x6 = Dyninst::aarch64::x6;
  auto x7 = Dyninst::aarch64::x7;
  auto x8 = Dyninst::aarch64::x8;
  auto x9 = Dyninst::aarch64::x9;
  auto x10 = Dyninst::aarch64::x10;
  auto x11 = Dyninst::aarch64::x11;
  auto x15 = Dyninst::aarch64::x15;
  auto x16 = Dyninst::aarch64::x16;
  auto x20 = Dyninst::aarch64::x20;
  auto x25 = Dyninst::aarch64::x25;
  auto x30 = Dyninst::aarch64::x30;

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // adc w5, w22, w25
      {0xc5, 0x02, 0x19, 0x1a},
      di::opcode_test(aarch64_op_adc, "adc w5, w22, w25"),
      di::register_rw_test {
        reg_set{w25, w22},
        reg_set{w5},
      },
    },
    { // add w0, w5, w8, lsl #5
      {0xa0, 0x14, 0x08, 0x0b},
      di::opcode_test(aarch64_op_add_addsub_shift, "add w0, w5, w8, lsl #5"),
      di::register_rw_test {
        reg_set{w8, w5},
        reg_set{w0},
      },
    },
    { // add w0, wsp, #11
      {0xe0, 0x2f, 0x00, 0x11},
      di::opcode_test(aarch64_op_add_addsub_imm, "add w0, wsp, #0, lsl #0"),  // WRONG: should be 'add  w0, wsp, #11'
      di::register_rw_test {
        reg_set{wsp},
        reg_set{w0},
      },
    },
    { // add w1, w10, w12
      {0x41, 0x01, 0x0c, 0x0b},
      di::opcode_test(aarch64_op_add_addsub_shift, "add w1, w10, w12, lsl #0"),
      di::register_rw_test {
        reg_set{w12, w10},
        reg_set{w1},
      },
    },
    { // add  w5, w10, w15, uxtx #2
      {0x45, 0x69, 0x2f, 0x0b},
      di::opcode_test(aarch64_op_add_addsub_ext, "add w5, w10, x15, lsl #2"), // WRONG: should be 'add  w5, w10, w15, uxtx #2'
      di::register_rw_test {
        reg_set{x15, w10},
        reg_set{w5},
      },
    },
    { // add x4, x7, x9, lsr #10
      {0xe4, 0x28, 0x49, 0x8b},
      di::opcode_test(aarch64_op_add_addsub_shift, "add x4, x7, x9 LSR #0"),  // WRONG: should be 'add x4, x7, x9, lsr #0'
      di::register_rw_test {
        reg_set{x9, x7},
        reg_set{x4},
      },
    },
    { // adds w0, w5, w8, lsl #5
      {0xa0, 0x14, 0x08, 0x2b},
      di::opcode_test(aarch64_op_adds_addsub_shift, "adds w0, w5, w8, lsl #5"),
      di::register_rw_test {
        reg_set{w8, w5},
        reg_set{w0, nzcv},
      },
    },
    { // adds w5, w10, #0, lsl #12
      {0x45, 0x01, 0x40, 0x31},
      di::opcode_test(aarch64_op_adds_addsub_imm, "adds w5, w10, #0, lsl #0"), // WRONG: should be 'adds w5, w10, #0, lsl #12'
      di::register_rw_test {
        reg_set{w10},
        reg_set{w5, nzcv},
      },
    },
    { // madd w1, w3, w2, w0
      {0x61, 0x00, 0x02, 0x1b},
      di::opcode_test(aarch64_op_madd, "madd w1, w3, w2, w0"),
      di::register_rw_test {
        reg_set{w2, w0, w3},
        reg_set{w1},
      },
    },
    { // msub x4, x8, x16, x30
      {0x04, 0xf9, 0x10, 0x9b},
      di::opcode_test(aarch64_op_msub, "msub x4, x8, x16, x30"),
      di::register_rw_test {
        reg_set{x16, x30, x8},
        reg_set{x4},
      },
    },
    { // sbc x0, x1, x2
      {0x20, 0x00, 0x02, 0xda},
      di::opcode_test(aarch64_op_sbc, "sbc x0, x1, x2"),
      di::register_rw_test {
        reg_set{x2, x1},
        reg_set{x0},
      },
    },
    { // sdiv x15, x20, x25
      {0x8f, 0x0e, 0xd9, 0x9a},
      di::opcode_test(aarch64_op_sdiv, "sdiv x15, x20, x25"),
      di::register_rw_test {
        reg_set{x25, x20},
        reg_set{x15},
      },
    },
    { // smsubl x0, w0, w1, x1
      {0x00, 0x84, 0x21, 0x9b},
      di::opcode_test(aarch64_op_smsubl, "smsubl x0, w0, w1, x1"),
      di::register_rw_test {
        reg_set{x1, w1, w0},
        reg_set{x0},
      },
    },
    { // sub sp, x10, #12
      {0x5f, 0x31, 0x40, 0xd1},
      di::opcode_test(aarch64_op_sub_addsub_imm, "sub sp, x10, #0, lsl #0"), // WRONG: should be 'sub sp, x10, #12, lsl #12'
      di::register_rw_test {
        reg_set{x10},
        reg_set{sp},
      },
    },
    { // sub w0, w2, w4
      {0x40, 0x00, 0x04, 0x4b},
      di::opcode_test(aarch64_op_sub_addsub_shift, "sub w0, w2, w4, lsl #0"),
      di::register_rw_test {
        reg_set{w4, w2},
        reg_set{w0},
      },
    },
    { // sub x0, x1, x1, sxtb #0
      {0x20, 0x80, 0x21, 0xcb},
      di::opcode_test(aarch64_op_sub_addsub_ext, "sub x0, x1, w1, lsl #0"), // WRONG: should be 'sub x0, x1, x1, sxtb #0'
      di::register_rw_test {
        reg_set{w1, x1},
        reg_set{x0},
      },
    },
    { // sub x6, x8, x11, asr #7
      {0x06, 0x1d, 0x8b, 0xcb},
      di::opcode_test(aarch64_op_sub_addsub_shift, "sub x6, x8, x11 ASR #7"), // WRONG: should be 'sub x6, x8, x11, asr #7"'
      di::register_rw_test {
        reg_set{x11, x8},
        reg_set{x6},
      },
    },
    { // subs x2, x2, x2, sxtw #4
      {0x42, 0x70, 0x22, 0xeb},
      di::opcode_test(aarch64_op_subs_addsub_ext, "subs x2, x2, x2, lsl #4"), // WRONG: should be 'subs x2, x2, x2, sxtw #4"'
      di::register_rw_test {
        reg_set{x2, x2},
        reg_set{x2, nzcv},
      },
    },
    { // udiv w0, w2, w4
      {0x40, 0x08, 0xc4, 0x1a},
      di::opcode_test(aarch64_op_udiv, "udiv w0, w2, w4"),
      di::register_rw_test {
        reg_set{w4, w2},
        reg_set{w0},
      },
    },
    { // umulh x5, x5, x10
      {0xa5, 0x28, 0xca, 0x9b},
      di::opcode_test(aarch64_op_umulh, "umulh x5, x5, x10"),
      di::register_rw_test {
        reg_set{x10, x5},
        reg_set{x5},
      },
    },
  };
  // clang-format on
}
