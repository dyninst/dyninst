#include "Architecture.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "register_tests.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/aarch64_regs.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  aarch64 load/store decoding tests
 *
 *  These verify the register read/write sets produced by the decoder
 *  for load and store instructions (plus a few data-processing
 *  instructions at the end of the table).
 */

namespace di = Dyninst::InstructionAPI;

namespace {

// The instruction bytes below are written in big-endian order for
// readability; the decoder expects little-endian words.
void reverseBuffer(unsigned char *buffer, int bufferSize) {
  int elementCount = bufferSize / 4;

  for(int loop_index = 0; loop_index < elementCount; loop_index++) {
    std::swap(buffer[0], buffer[3]);
    std::swap(buffer[1], buffer[2]);
    buffer += 4;
  }
}

} // namespace

int main() {
  constexpr auto num_tests = 136;

  // clang-format off
  std::array<unsigned char, 4*num_tests> buffer = {{
    //literal
    0x58, 0x00, 0x00, 0x21,         // ldr x1,  #4
    0x58, 0x00, 0x08, 0x01,         // ldr x1,  #256
    0x18, 0x00, 0x00, 0x21,         // ldr w1,  #4
    0x18, 0x00, 0x08, 0x01,         // ldr w1,  #1048576

    //post-inc
    0xf8, 0x40, 0x14, 0x41,         // ldr,     x1, [x2], #1
    0xf8, 0x4f, 0xf4, 0x41,         // ldr,     x1, [x2], #255
    0x38, 0x40, 0x14, 0x41,         // ldrb,    w1, [x2], #1
    0x38, 0xc0, 0x14, 0x41,         // ldrsb,   w1, [x2], #1
    0x78, 0x40, 0x14, 0x41,         // ldrh,    w1, [x2], #1
    0x78, 0xc0, 0x14, 0x41,         // ldrsh,    x1, [x2], #1

    //imm
    0xf9, 0x40, 0x04, 0x41,         // ldr,     x1, [x2, #8]
    0xf9, 0x40, 0x80, 0x41,         // ldr,     x1, [x2, #256]
    0x39, 0x40, 0x10, 0x41,         // ldrb,    w1, [x2, #4]
    0x39, 0xc0, 0x10, 0x41,         // ldrsb,   w1, [x2, #4]
    0x79, 0x40, 0x08, 0x41,         // ldrh,    w1, [x2, #4]
    0x79, 0xc0, 0x08, 0x41,         // ldrsh,   w1, [x2, #4]
    0xb9, 0x80, 0x04, 0x41,         // ldrsw,   x1, [x2, #4]

    //register off not ext
    0xf8, 0x63, 0x68, 0x41,         // ldr,     x1, [x2, x3]
    0x38, 0x63, 0x68, 0x41,         // ldrb,    w1, [x2, x3]
    0x38, 0xe3, 0x68, 0x41,         // ldrsb,   w1, [x2, x3]
    0x78, 0x63, 0x68, 0x41,         // ldrh,    w1, [x2, x3]
    0x78, 0xe3, 0x68, 0x41,         // ldrsh,   w1, [x2, x3]
    0xb8, 0xa3, 0x68, 0x41,         // ldrsw,    x1, [x2, x3]

    //pre-inc
    0xf8, 0x40, 0x1c, 0x41,         // ldr,     x1, [x2, #1]!
    0xf8, 0x4f, 0xfc, 0x41,         // ldr,     x1, [x2, #255]!
    0x38, 0x40, 0x1c, 0x41,         // ldrb,    w1, [x2, #1]!
    0x38, 0xc0, 0x1c, 0x41,         // ldrsb,   w1, [x2, #1]!
    0x78, 0x40, 0x1c, 0x41,         // ldrh,    w1, [x2, #1]!
    0x78, 0xc0, 0x1c, 0x41,         // ldrsh,   w1, [x2, #1]!
    0xb8, 0x80, 0x1c, 0x41,         // ldrsw,   x1, [x2, #1]!

    //exclusive
    0xc8, 0xdf, 0xfc, 0x41,          //ldar    x1, [x2]
    0x88, 0xdf, 0xfc, 0x41,          //ldar    w1, [x2]
    0x08, 0x5f, 0x7c, 0x41,          //ldxrb   w1, [x2]
    0x08, 0x5f, 0xfc, 0x41,          //ldaxrb  w1, [x2]

    0x08, 0xdf, 0xfc, 0x41,          //ldarb   w1, [x2]
    0x48, 0x5f, 0x7c, 0x41,          //ldxrh   w1, [x2]
    0x48, 0x5f, 0xfc, 0x41,          //ldaxrh  w1, [x2]
    0x48, 0xdf, 0xfc, 0x41,          //ldarh   w1, [x2]

    0xc8, 0x5f, 0x7c, 0x41,          //ldxr    x1, [x2]
    0xc8, 0x5f, 0xfc, 0x41,          //ldaxr   x1, [x2]
    0xc8, 0xdf, 0xfc, 0x41,          //ldar    x1, [x2]
    0x88, 0x5f, 0x7c, 0x41,          //ldxr    w1, [x2]

    0x88, 0x5f, 0xfc, 0x41,          //ldaxr   w1, [x2]
    0x88, 0xdf, 0xfc, 0x41,          //ldar    w1, [x2]
    0xc8, 0x7f, 0x0c, 0x41,          //ldxp    x1, x3, [x2]
    0xc8, 0x7f, 0x8c, 0x41,          //ldaxp   x1, x3, [x2]

    0x88, 0x7f, 0x0c, 0x41,          //ldxp    w1, w3, [x2]
    0x88, 0x7f, 0x8c, 0x41,          //ldaxp   w1, w3, [x2]

    //pair
    0xa8,   0x40,   0x88,   0x61,        //ldnp    x1, x2, [x3,#8]
    0xa9,   0x40,   0x88,   0x61,        //ldp     x1, x2, [x3,#8]
    0xa9,   0xc0,   0x88,   0x61,        //ldp     x1, x2, [x3,#8]!
    0xa8,   0xc0,   0x88,   0x61,        //ldp     x1, x2, [x3],#8
    0xa8,   0xc0,   0x88,   0x61,        //ldp     x1, x2, [x3],#8

    //unsacled
    0x38,   0x40,   0x10,   0x61,        //ldurb   w1, [x3,#1]
    0x38,   0x80,   0x10,   0x61,        //ldursb  x1, [x3,#1]
    0xf8,   0x40,   0x10,   0x61,        //ldur     x1, [x3,#1]
    0x78,   0x40,   0x10,   0x61,        //ldurh    w1, [x3,#1]
    0x78,   0x80,   0x10,   0x61,        //ldursh   x1, [x3,#1]
    0xb8,   0x80,   0x10,   0x61,        //ldursw   x1, [x3,#1]

    //unprevlidged
    0x38,   0x40,   0x18,   0x61,        //ldtrb   w1, [x3,#1]
    0x38,   0x80,   0x18,   0x61,        //ldtrsb  x1, [x3,#1]
    0xf8,   0x40,   0x18,   0x61,        //ldtr    x1, [x3,#1]
    0x78,   0x40,   0x18,   0x61,        //ldtrh   w1, [x3,#1]
    0x78,   0x80,   0x18,   0x61,        //ldtrsh  x1, [x3,#1]
    0xb8,   0x80,   0x18,   0x61,        //ldtrsw  x1, [x3,#1]

    //----store----
    0xf8,   0x00,   0x14,   0x41,        //str     x1, [x2],#1
    0xf8,   0x0f,   0xf4,   0x41,        //str     x1, [x2],#255
    0x38,   0x00,   0x14,   0x41,        //strb    w1, [x2],#1
    0x78,   0x00,   0x14,   0x41,        //strh    w1, [x2],#1

    0xf9,   0x00,   0x04,   0x41,        //str     x1, [x2,#8]
    0xf9,   0x00,   0x80,   0x41,        //str     x1, [x2,#256]
    0x39,   0x00,   0x10,   0x41,        //strb    w1, [x2,#4]
    0x79,   0x00,   0x08,   0x41,        //strh    w1, [x2,#4]

    0xf8,   0x23,   0x68,   0x41,        //str     x1, [x2,x3]
    0xf8,   0x23,   0x68,   0x41,        //str     x1, [x2,x3]
    0x38,   0x23,   0x68,   0x41,        //strb    w1, [x2,x3]
    0x78,   0x23,   0x68,   0x41,        //strh    w1, [x2,x3]

    0xf8,   0x00,   0x1c,   0x41,        //str     x1, [x2,#1]!
    0xf8,   0x0f,   0xfc,   0x41,        //str     x1, [x2,#255]!
    0x38,   0x00,   0x1c,   0x41,        //strb    w1, [x2,#1]!
    0x78,   0x00,   0x1c,   0x41,        //strh    w1, [x2,#1]!

    0x08,   0x00,   0x7c,   0x41,        //stxrb   w0, w1, [x2]
    0x48,   0x00,   0x7c,   0x41,        //stxrh   w0, w1, [x2]
    0x88,   0x00,   0x7c,   0x41,        //stxr    w0, w1, [x2]
    0x88,   0x20,   0x0c,   0x41,        //stxp    w0, w1, w3, [x2]

    0xa8,   0x00,   0x88,   0x61,        //stnp    x1, x2, [x3,#8]
    0xa9,   0x00,   0x88,   0x61,        //stp     x1, x2, [x3,#8]
    0xa9,   0x80,   0x88,   0x61,        //stp     x1, x2, [x3,#8]!
    0xa8,   0x80,   0x88,   0x61,        //stp     x1, x2, [x3],#8
    0xa8,   0x80,   0x88,   0x61,        //stp     x1, x2, [x3],#8

    0x38,   0x00,   0x10,   0x61,        //sturb   w1, [x3,#1]
    0xf8,   0x00,   0x10,   0x61,        //str     x1, [x3,#1]
    0x78,   0x00,   0x10,   0x61,        //strh    w1, [x3,#1]

    0xf8,   0x00,   0x18,   0x61,        //sttr    x1, [x3,#1]
    0x38,   0x00,   0x18,   0x61,        //sttrb   w1, [x3,#1]
    0x78,   0x00,   0x18,   0x61,        //sttrh   w1, [x3,#1]

    0x08,   0x9f,   0xfc,   0x61,        //stlrb   w1, [x3]
    0xc8,   0x9f,   0xfc,   0x61,        //stlr    x1, [x3]
    0x48,   0x9f,   0xfc,   0x61,        //stlrh   w1, [x3]
    0xc8,   0x20,   0x88,   0x61,        //stlxp   w0, x1, x2, [x3]
    0x08,   0x00,   0xfc,   0x61,        //stlxrb  w0, w1, [x3]
    0xc8,   0x00,   0xfc,   0x61,        //stlxr   w0, x1, [x3]
    0x48,   0x00,   0xfc,   0x61,        //stlxrh  w0, w1, [x3]

    0xf8,   0x63,   0x68,   0x41,        //ldr     x1, [x2,x3]
    0xf8,   0x63,   0x78,   0x41,        //ldr     x1, [x2,x3,lsl #3]
    0xb8,   0x63,   0x78,   0x41,        //ldr     w1, [x2,x3,lsl #2]
    0xf8,   0x63,   0x48,   0x41,        //ldr     x1, [x2,w3,uxtw]

    0xf8,   0x63,   0xe8,   0x41,        //ldr     x1, [x2,x3,sxtx]
    0x9a,   0x82,   0x04,   0x20,        //csinc   x0, x1, x2, eq
    0xda,   0x82,   0x00,   0x20,        //csinv   x0, x1, x2, eq
    0xda,   0x82,   0x04,   0x20,        //csneg   x0, x1, x2, eq

    0xd3,   0x7f,   0x00,   0x20,        //ubfiz   x0, x1, #1, #1
    0x53,   0x00,   0x7c,   0x20,        //lsr     w0, w1, #0
    0x92,   0x9f,   0xff,   0xe0,        //mov     x0, #0xffffffffffff0000         // #-65536
    0x92,   0xff,   0xff,   0xe0,        //mov     x0, #0xffffffffffff             // #281474976710655

    0x12,   0xbf,   0xff,   0xe0,        //movn    w0, #0xffff, lsl #16
    0xd2,   0x9f,   0xff,   0xe0,        //mov     x0, #0xffff                     // #65535
    0xd2,   0xff,   0xff,   0xe0,        //mov     x0, #0xffff000000000000         // #-281474976710656
    0x52,   0xbf,   0xff,   0xe0,        //mov     w0, #0xffff0000                 // #-65536

    0x18,   0x7f,   0xff,   0xbe,        //ldr     w30, 500630 0xffff4
    0x58,   0xf3,   0xcb,   0x1e,        //ldr     x30, 3e7fa0  -0x184d0
    0xb8,   0x4f,   0xf4,   0x0f,        //ldr     w15, [x0],#255
    0xf8,   0x51,   0x07,   0xcf,        //ldr     x15, [x30],#-240

    0xb9,   0x7f,   0xff,   0xfd,        //ldr     w29, [sp,#16380]
    0xf9,   0x7f,   0xff,   0xfd,        //ldr     x29, [sp,#32760]
    0xf8,   0x7f,   0x68,   0x41,        //ldr     x1, [x2,x31]
    0xf8,   0x4f,   0xff,   0xe1,        //ldr     x1, [sp,#255]!

    //prfm
    0xd8,   0x00,   0x00,   0x3f,	//prfm	   1F, [pc + 4]
    0xd8,   0xff,   0xff,   0xe0,	//prfm	   0, [pc + fffffffffffffffc]
    0xd8,   0x0f,   0xd8,   0x08,	//prfm	   8, [pc + 1fb00]
    0xf9,   0x80,   0x04,   0x3d,	//prfm	   1d, [x1 + 8]
    0xf9,   0xbf,   0xff,   0xdf,	//prfm	   1f, [x30 + 7ff8]
    0xf8,   0xa5,   0x68,   0x44,	//prfm	   4, [x2 + x5 << 0]
    0xf8,   0xa1,   0x5b,   0xc5,	//prfm	   5, [x30 + w1 << 3]
    0xf8,   0xa5,   0xe9,   0x06,	//prfm	   6, [x8 + x5 << 0]

    0xd5,   0x03,   0x20,   0x1f,        //nop
  }};
  // clang-format on

  reverseBuffer(buffer.data(), buffer.size());
  di::InstructionDecoder d(buffer.data(), buffer.size(), Dyninst::Arch_aarch64);

  std::vector<di::Instruction> decodedInsns;
  decodedInsns.reserve(num_tests);
  for(int idx = 0; idx < num_tests; idx++) {
    di::Instruction insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode test " << (idx + 1) << '\n';
      return EXIT_FAILURE;
    }
    decodedInsns.push_back(insn);
  }

  auto x0 = Dyninst::aarch64::x0;
  auto x1 = Dyninst::aarch64::x1;
  auto x2 = Dyninst::aarch64::x2;
  auto x3 = Dyninst::aarch64::x3;
  auto x5 = Dyninst::aarch64::x5;
  auto x8 = Dyninst::aarch64::x8;
  auto x15 = Dyninst::aarch64::x15;
  auto x29 = Dyninst::aarch64::x29;
  auto x30 = Dyninst::aarch64::x30;

  auto w0 = Dyninst::aarch64::w0;
  auto w1 = Dyninst::aarch64::w1;
  auto w3 = Dyninst::aarch64::w3;
  auto w15 = Dyninst::aarch64::w15;
  auto w29 = Dyninst::aarch64::w29;
  auto w30 = Dyninst::aarch64::w30;

  auto xzr = Dyninst::aarch64::xzr;
  auto sp = Dyninst::aarch64::sp;
  auto pc = Dyninst::aarch64::pc;
  auto nzcv = Dyninst::aarch64::nzcv;

  using reg_set = Dyninst::register_set;

  std::vector<reg_set> expectedRead, expectedWritten;

  // ldr x1, #4
  expectedRead.push_back(reg_set{pc});
  expectedWritten.push_back(reg_set{x1});

  // ldr x1, #256
  expectedRead.push_back(reg_set{pc});
  expectedWritten.push_back(reg_set{x1});

  // ldr w1, #4
  expectedRead.push_back(reg_set{pc});
  expectedWritten.push_back(reg_set{w1});

  // ldr w1, #1048576
  expectedRead.push_back(reg_set{pc});
  expectedWritten.push_back(reg_set{w1});

  // ldr, x1, [x2], #1
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{x1});

  // ldr, x1, [x2], #255
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{x1});

  // ldrb, w1, [x2], #1
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldrsb, w1, [x2], #1
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldrh, w1, [x2], #1
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldrsh, x1, [x2], #1
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldr, x1, [x2, #8]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{x1});

  // ldr, x1, [x2, #256]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{x1});

  // ldrb, w1, [x2, #4]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldrsb, w1, [x2, #4]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldrh, w1, [x2, #4]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldrsh, w1, [x2, #4]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldrsw, x1, [x2, #4]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{x1});

  // ldr, x1, [x2, x3]
  expectedRead.push_back(reg_set{x2, x3});
  expectedWritten.push_back(reg_set{x1});

  // ldrb, w1, [x2, x3]
  expectedRead.push_back(reg_set{x2, x3});
  expectedWritten.push_back(reg_set{w1});

  // ldrsb, w1, [x2, x3]
  expectedRead.push_back(reg_set{x2, x3});
  expectedWritten.push_back(reg_set{w1});

  // ldrh, w1, [x2, x3]
  expectedRead.push_back(reg_set{x2, x3});
  expectedWritten.push_back(reg_set{w1});

  // ldrsh, w1, [x2, x3]
  expectedRead.push_back(reg_set{x2, x3});
  expectedWritten.push_back(reg_set{w1});

  // ldrsw, x1, [x2, x3]
  expectedRead.push_back(reg_set{x2, x3});
  expectedWritten.push_back(reg_set{x1});

  // ldr, x1, [x2, #1]!
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{x1});

  // ldr, x1, [x2, #255]!
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{x1});

  // ldrb, w1, [x2, #1]!
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldrsb, w1, [x2, #1]!
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldrh, w1, [x2, #1]!
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldrsh, w1, [x2, #1]!
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldrsw, x1, [x2, #1]!
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{x1});

  // ldar x1, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{x1});

  // ldar w1, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldxrb w1, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldaxrb w1, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldarb w1, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldxrh w1, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldaxrh w1, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldarh w1, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldxr x1, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{x1});

  // ldaxr x1, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{x1});

  // ldar x1, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{x1});

  // ldxr w1, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldaxr w1, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldar w1, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1});

  // ldxp x1, x3, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{x1, x3});

  // ldaxp x1, x3, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{x1, x3});

  // ldxp w1, w3, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1, w3});

  // ldaxp w1, w3, [x2]
  expectedRead.push_back(reg_set{x2});
  expectedWritten.push_back(reg_set{w1, w3});

  // ldnp x1, x2, [x3,#8]
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{x1, x2});

  // ldp x1, x2, [x3,#8]
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{x1, x2});

  // ldp x1, x2, [x3,#8]!
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{x1, x2});

  // ldp x1, x2, [x3],#8
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{x1, x2});

  // ldp x1, x2, [x3],#8
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{x1, x2});

  // ldurb w1, [x3,#1]
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{w1});

  // ldursb x1, [x3,#1]
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{x1});

  // ldur x1, [x3,#1]
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{x1});

  // ldurh w1, [x3,#1]
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{w1});

  // ldursh x1, [x3,#1]
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{x1});

  // ldursw x1, [x3,#1]
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{x1});

  // ldtrb w1, [x3,#1]
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{w1});

  // ldtrsb x1, [x3,#1]
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{x1});

  // ldtr x1, [x3,#1]
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{x1});

  // ldtrh w1, [x3,#1]
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{w1});

  // ldtrsh x1, [x3,#1]
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{x1});

  // ldtrsw x1, [x3,#1]
  expectedRead.push_back(reg_set{x3});
  expectedWritten.push_back(reg_set{x1});

  // str x1, [x2],#1
  expectedRead.push_back(reg_set{x2, x1});
  expectedWritten.push_back(reg_set{});

  // str x1, [x2],#255
  expectedRead.push_back(reg_set{x2, x1});
  expectedWritten.push_back(reg_set{});

  // strb w1, [x2],#1
  expectedRead.push_back(reg_set{x2, w1});
  expectedWritten.push_back(reg_set{});

  // strh w1, [x2],#1
  expectedRead.push_back(reg_set{x2, w1});
  expectedWritten.push_back(reg_set{});

  // str x1, [x2,#8]
  expectedRead.push_back(reg_set{x2, x1});
  expectedWritten.push_back(reg_set{});

  // str x1, [x2,#256]
  expectedRead.push_back(reg_set{x2, x1});
  expectedWritten.push_back(reg_set{});

  // strb w1, [x2,#4]
  expectedRead.push_back(reg_set{x2, w1});
  expectedWritten.push_back(reg_set{});

  // strh w1, [x2,#4]
  expectedRead.push_back(reg_set{x2, w1});
  expectedWritten.push_back(reg_set{});

  // str x1, [x2,x3]
  expectedRead.push_back(reg_set{x2, x1, x3});
  expectedWritten.push_back(reg_set{});

  // str x1, [x2,x3]
  expectedRead.push_back(reg_set{x2, x1, x3});
  expectedWritten.push_back(reg_set{});

  // strb w1, [x2,x3]
  expectedRead.push_back(reg_set{x2, w1, x3});
  expectedWritten.push_back(reg_set{});

  // strh w1, [x2,x3]
  expectedRead.push_back(reg_set{x2, w1, x3});
  expectedWritten.push_back(reg_set{});

  // str x1, [x2,#1]!
  expectedRead.push_back(reg_set{x2, x1});
  expectedWritten.push_back(reg_set{});

  // str x1, [x2,#255]!
  expectedRead.push_back(reg_set{x2, x1});
  expectedWritten.push_back(reg_set{});

  // strb w1, [x2,#1]!
  expectedRead.push_back(reg_set{x2, w1});
  expectedWritten.push_back(reg_set{});

  // strh w1, [x2,#1]!
  expectedRead.push_back(reg_set{x2, w1});
  expectedWritten.push_back(reg_set{});

  // stxrb w0, w1, [x2]
  expectedRead.push_back(reg_set{x2, w1});
  expectedWritten.push_back(reg_set{w0});

  // stxrh w0, w1, [x2]
  expectedRead.push_back(reg_set{x2, w1});
  expectedWritten.push_back(reg_set{w0});

  // stxr w0, w1, [x2]
  expectedRead.push_back(reg_set{x2, w1});
  expectedWritten.push_back(reg_set{w0});

  // stxp w0, w1, w3, [x2]
  expectedRead.push_back(reg_set{x2, w1, w3});
  expectedWritten.push_back(reg_set{w0});

  // stnp x1, x2, [x3,#8]
  expectedRead.push_back(reg_set{x2, x1, x3});
  expectedWritten.push_back(reg_set{});

  // stp x1, x2, [x3,#8]
  expectedRead.push_back(reg_set{x2, x1, x3});
  expectedWritten.push_back(reg_set{});

  // stp x1, x2, [x3,#8]!
  expectedRead.push_back(reg_set{x2, x1, x3});
  expectedWritten.push_back(reg_set{});

  // stp x1, x2, [x3],#8
  expectedRead.push_back(reg_set{x2, x1, x3});
  expectedWritten.push_back(reg_set{});

  // stp x1, x2, [x3],#8
  expectedRead.push_back(reg_set{x2, x1, x3});
  expectedWritten.push_back(reg_set{});

  // sturb w1, [x3,#1]
  expectedRead.push_back(reg_set{x3, w1});
  expectedWritten.push_back(reg_set{});

  // str x1, [x3,#1]
  expectedRead.push_back(reg_set{x3, x1});
  expectedWritten.push_back(reg_set{});

  // strh w1, [x3,#1]
  expectedRead.push_back(reg_set{x3, w1});
  expectedWritten.push_back(reg_set{});

  // sttr x1, [x3,#1]
  expectedRead.push_back(reg_set{x3, x1});
  expectedWritten.push_back(reg_set{});

  // sttrb w1, [x3,#1]
  expectedRead.push_back(reg_set{x3, w1});
  expectedWritten.push_back(reg_set{});

  // sttrh w1, [x3,#1]
  expectedRead.push_back(reg_set{x3, w1});
  expectedWritten.push_back(reg_set{});

  // stlrb w1, [x3]
  expectedRead.push_back(reg_set{x3, w1});
  expectedWritten.push_back(reg_set{});

  // stlr x1, [x3]
  expectedRead.push_back(reg_set{x3, x1});
  expectedWritten.push_back(reg_set{});

  // stlrh w1, [x3]
  expectedRead.push_back(reg_set{x3, w1});
  expectedWritten.push_back(reg_set{});

  // stlxp w0, x1, x2, [x3]
  expectedRead.push_back(reg_set{x3, x1, x2});
  expectedWritten.push_back(reg_set{w0});

  // stlxrb w0, w1, [x3]
  expectedRead.push_back(reg_set{w1, x3});
  expectedWritten.push_back(reg_set{w0});

  // stlxr w0, x1, [x3]
  expectedRead.push_back(reg_set{x1, x3});
  expectedWritten.push_back(reg_set{w0});

  // stlxrh w0, w1, [x3]
  expectedRead.push_back(reg_set{w1, x3});
  expectedWritten.push_back(reg_set{w0});

  // ldr x1, [x2,x3]
  expectedRead.push_back(reg_set{x3, x2});
  expectedWritten.push_back(reg_set{x1});

  // ldr x1, [x2,x3,lsl #3]
  expectedRead.push_back(reg_set{x3, x2});
  expectedWritten.push_back(reg_set{x1});

  // ldr w1, [x2,x3,lsl #2]
  expectedRead.push_back(reg_set{x3, x2});
  expectedWritten.push_back(reg_set{w1});

  // ldr x1, [x2,w3,uxtw]
  expectedRead.push_back(reg_set{w3, x2});
  expectedWritten.push_back(reg_set{x1});

  // ldr x1, [x2,x3,sxtx]
  expectedRead.push_back(reg_set{x3, x2});
  expectedWritten.push_back(reg_set{x1});

  // csinc x0, x1, x2, eq
  expectedRead.push_back(reg_set{x1, x2, nzcv});
  expectedWritten.push_back(reg_set{x0});

  // csinv x0, x1, x2, eq
  expectedRead.push_back(reg_set{x1, x2, nzcv});
  expectedWritten.push_back(reg_set{x0});

  // csneg x0, x1, x2, eq
  expectedRead.push_back(reg_set{x1, x2, nzcv});
  expectedWritten.push_back(reg_set{x0});

  // ubfiz x0, x1, #1, #1
  expectedRead.push_back(reg_set{x1});
  expectedWritten.push_back(reg_set{x0});

  // lsr w0, w1, #0
  expectedRead.push_back(reg_set{w1});
  expectedWritten.push_back(reg_set{w0});

  // mov x0, #0xffffffffffff0000
  expectedRead.push_back(reg_set{});
  expectedWritten.push_back(reg_set{x0});

  // mov x0, #0xffffffffffff
  expectedRead.push_back(reg_set{});
  expectedWritten.push_back(reg_set{x0});

  // movn w0, #0xffff, lsl #16
  expectedRead.push_back(reg_set{});
  expectedWritten.push_back(reg_set{w0});

  // mov x0, #0xffff
  expectedRead.push_back(reg_set{});
  expectedWritten.push_back(reg_set{x0});

  // mov x0, #0xffff000000000000
  expectedRead.push_back(reg_set{});
  expectedWritten.push_back(reg_set{x0});

  // mov w0, #0xffff0000
  expectedRead.push_back(reg_set{});
  expectedWritten.push_back(reg_set{w0});

  // ldr w30, 500630 0xffff4
  expectedRead.push_back(reg_set{pc});
  expectedWritten.push_back(reg_set{w30});

  // ldr x30, 3e7fa0 -0x184d0
  expectedRead.push_back(reg_set{pc});
  expectedWritten.push_back(reg_set{x30});

  // ldr w15, [x0],#255
  expectedRead.push_back(reg_set{x0});
  expectedWritten.push_back(reg_set{w15});

  // ldr x15, [x30],#-240
  expectedRead.push_back(reg_set{x30});
  expectedWritten.push_back(reg_set{x15});

  // ldr w29, [sp,#16380]
  expectedRead.push_back(reg_set{sp});
  expectedWritten.push_back(reg_set{w29});

  // ldr x29, [sp,#32760]
  expectedRead.push_back(reg_set{sp});
  expectedWritten.push_back(reg_set{x29});

  // ldr x1, [x2,x31]
  expectedRead.push_back(reg_set{x2, xzr});
  expectedWritten.push_back(reg_set{x1});

  // ldr x1, [sp,#255]!
  expectedRead.push_back(reg_set{sp});
  expectedWritten.push_back(reg_set{x1});

  // prfm 1F, [pc + 4]
  expectedRead.push_back(reg_set{pc});
  expectedWritten.push_back(reg_set{});

  // prfm 0, [pc + fffffffffffffffc]
  expectedRead.push_back(reg_set{pc});
  expectedWritten.push_back(reg_set{});

  // prfm 8, [pc + 1fb00]
  expectedRead.push_back(reg_set{pc});
  expectedWritten.push_back(reg_set{});

  // prfm 1d, [x1 + 8]
  expectedRead.push_back(reg_set{x1});
  expectedWritten.push_back(reg_set{});

  // prfm 1f, [x30 + 7ff8]
  expectedRead.push_back(reg_set{x30});
  expectedWritten.push_back(reg_set{});

  // prfm 4, [x2 + x5 << 0]
  expectedRead.push_back(reg_set{x2, x5});
  expectedWritten.push_back(reg_set{});

  // prfm 5, [x30 + w1 << 3]
  expectedRead.push_back(reg_set{x30, w1});
  expectedWritten.push_back(reg_set{});

  // prfm 6, [x8 + x5 << 0]
  expectedRead.push_back(reg_set{x8, x5});
  expectedWritten.push_back(reg_set{});

  // nop
  expectedRead.push_back(reg_set{});
  expectedWritten.push_back(reg_set{});

  if(expectedRead.size() != expectedWritten.size()) {
    std::cerr << "FATAL: expectedRead.size() != expectedWritten.size()\n";
    return EXIT_FAILURE;
  }

  if(expectedRead.size() != decodedInsns.size()) {
    std::cerr << "FATAL: expectedRead.size() != decodedInsns.size()\n";
    return EXIT_FAILURE;
  }

  bool failed = false;
  for(size_t i = 0; i < decodedInsns.size(); i++) {
    auto const &insn = decodedInsns[i];

    std::clog << "Verifying '" << insn.format() << "'\n";

    if(!di::verify(insn, di::register_rw_test{expectedRead[i], expectedWritten[i]})) {
      failed = true;
    }
  }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
