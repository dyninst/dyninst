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
 *  aarch64 SIMD decoding tests
 *
 *  These verify the register read/write sets produced by the decoder
 *  for Advanced SIMD (NEON) instructions.
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
  constexpr auto num_tests = 131;

  // clang-format off
  std::array<uint8_t, 4*num_tests> buffer = {{
    0x4e, 0x30, 0x38, 0x20,     // saddlv  h0, q1.8b
    0x0e, 0x30, 0xa9, 0x0f,     // smaxv   b15, d8
    0x6e, 0x70, 0x38, 0x42,     // uaddlv  s2, q2
    0x2e, 0x31, 0xa8, 0x05,     // uminv   b5, d0
    0x6e, 0x30, 0xc8, 0x25,     // fmaxnmv s5, q1
    0x0e, 0x11, 0x04, 0x41,     // dup     d1, q2
    0x4e, 0x02, 0x0c, 0xd8,     // dup     q20, w5
    0x0e, 0x18, 0x0c, 0x40,     // dup     d0, x2
    0x4e, 0x08, 0x1c, 0x04,     // ins     q4, x0
    0x4e, 0x01, 0x1f, 0xff,     // ins     q31, wzr
    0x0e, 0x05, 0x2c, 0xa1,     // smov    w1, d5
    0x4e, 0x10, 0x3c, 0x42,     // umov    x2, q2
    0x6e, 0x01, 0x04, 0x00,     // ins     q0, d0
    0x2e, 0x02, 0x28, 0x20,     // ext     d0, d1, d2, #5
    0x6e, 0x02, 0x7a, 0x08,     // ext     q8, q16, q2, #15
    0x0f, 0x00, 0xe5, 0x00,     // movi    d0, #8
    0x4f, 0x07, 0xa7, 0xe1,     // movi    q1, ff lsl #8
    0x0f, 0x00, 0x74, 0x05,     // orr     d5, #0 lsl #24
    0x0f, 0x00, 0xb4, 0x05,     // orr     d5, #0 lsl #8
    0x4f, 0x07, 0xf7, 0xe8,     // fmov    q8, ff
    0x6f, 0x00, 0xc5, 0x80,     // mvni    q0, #12 lsl #8
    0x6f, 0x00, 0x97, 0x08,     // bic     d8, #24 lsl #0
    0x2f, 0x07, 0xe7, 0xe2,     // movi    d2, (all ones)
    0x0e, 0x00, 0x1a, 0x08,     // uzp1    d8, d16, d0
    0x4e, 0x04, 0x28, 0x62,     // trn1    q2, q3, q4
    0x0e, 0x08, 0x79, 0x08,     // zip2    d8, d8, d8
    0x5e, 0x11, 0x04, 0x25,     // dup     b5, q1
    0x5e, 0x08, 0x05, 0x08,     // dup     d8, d8
    0x5e, 0xf1, 0xb8, 0xa2,     // addp    d2, q5
    0x7e, 0x30, 0xc8, 0x81,     // fmaxnmp s1, d4
    0x7e, 0xf0, 0xc9, 0xef,     // fminnmp d15, q15
    0x5f, 0x78, 0x04, 0x82,     // sshr    d2, d4, #8
    0x5f, 0x40, 0x14, 0x20,     // ssra    d0, d1, #64
    0x5f, 0x41, 0x57, 0xff,     // shl     d31, d31, #1
    0x5f, 0x0e, 0x76, 0x08,     // sqshl   b8, b16, 6
    0x5f, 0x10, 0x9d, 0x02,     // sqrshrn h2, h8, #16
    0x5f, 0x20, 0xfc, 0x00,     // fcvtzs  s0, s0, #32
    0x7f, 0x48, 0x55, 0x04,     // sli     d4, d8, #8
    0x7f, 0x09, 0x94, 0x42,     // uqshrn  b2, h2, #7
    0x5e, 0x60, 0x91, 0x08,     // sqdmlal s8, h8, h0
    0x5e, 0xa1, 0xb0, 0x82,     // sqdmlsl d2, s4, s1
    0x5e, 0x7f, 0xd0, 0x00,     // sqdmull s0, h0, h31
    0x5e, 0xe0, 0x34, 0x22,     // cmgt    d2, d1, d0
    0x5e, 0x64, 0x4c, 0x40,     // sqshl   h0, h2, h4
    0x5e, 0xa0, 0xb4, 0x3f,     // sqdmulh s31, s1, s0
    0x5e, 0x68, 0xfd, 0x08,     // frecps  d8, d8, d8
    0x7e, 0xe2, 0x57, 0xe4,     // urshl   d4, d31, d2
    0x7e, 0xb0, 0xe5, 0x02,     // fcmgt   s2, s8, s16
    0x5e, 0x20, 0x39, 0x02,     // suqadd  b2, b8
    0x5e, 0x61, 0x48, 0x04,     // sqxtn   h4, s0
    0x5e, 0x61, 0xb8, 0x3f,     // fcvtms  d31, d1
    0x5e, 0xa1, 0xd8, 0xa9,     // frecpe  s9, s5
    0x7e, 0x21, 0x28, 0x48,     // sqxtun  b8, h2
    0x7e, 0x61, 0x68, 0x4f,     // fcvtxn  s15, d2
    0x7e, 0xa1, 0xdb, 0xff,     // frsqrte s31, s31
    0x5f, 0x49, 0x38, 0xa2,     // sqdmlal s2, h5, q9
    0x5f, 0x9f, 0x73, 0xe0,     // sqdmlsl d0, s31, d31
    0x5f, 0x4f, 0xd0, 0x88,     // sqrdmulh h8, h4, d15
    0x5f, 0x94, 0x18, 0x49,     // fmla    s9, s2, q20
    0x5f, 0xd1, 0x50, 0xa8,     // fmls    d8, d5, d17
    0x4e, 0x09, 0x01, 0x04,     // tbl     q4, q8, q9
    0x0e, 0x03, 0x30, 0x20,     // tbx     d0, d1, d2, d3
    0x4e, 0x1f, 0x43, 0x00,     // tbl     q0, q24, q25, q26, q31
    0x0e, 0x15, 0x70, 0x14,     // tbx     d20, d0, d1, d2, d3, d21
    0x4e, 0x23, 0x00, 0x41,     // saddl   q1, hq2, hq3
    0x0e, 0x25, 0x20, 0x9f,     // ssubl   q31, d4, d5
    0x4e, 0x30, 0x41, 0x02,     // addhn   hq2, q8, q16
    0x0e, 0x68, 0x70, 0x45,     // sabdl   q5, d2, d8
    0x4e, 0xa2, 0xb0, 0x20,     // sqdmlsl q0, hq1, hq2
    0x2e, 0x3f, 0x61, 0xe2,     // rsubhn  d2, q15, q31
    0x4e, 0x63, 0x60, 0x48,     // subhn   hq8, q2,q3
    0x0e, 0x25, 0x0c, 0x62,     // sqadd   d2, d3, d5
    0x4e, 0x3f, 0x34, 0x20,     // cmgt    q0, q1, q31
    0x0e, 0x2a, 0x7d, 0x28,     // saba    d8, d9, d10
    0x6e, 0x66, 0x1c, 0xa4,     // bsl     q4, q5, q6
    0x0e, 0x20, 0x59, 0x28,     // cnt     d8, d9
    0x4e, 0x21, 0x2b, 0xe0,     // xtn     q0, q31
    0x4e, 0x61, 0xc8, 0x62,     // fcvtas  q2, q3
    0x2e, 0xa1, 0xf9, 0xff,     // fsqrt   d31, d15
    0x0f, 0x40, 0x20, 0xa2,     // smlal   q2, d5, d0
    0x4f, 0x85, 0x6b, 0xe1,     // smlsl   q1, hq31, q5
    0x4f, 0x9f, 0x80, 0x49,     // mul     q9, q2, d31
    0x0f, 0x45, 0xb8, 0x88,     // sqdmull q8, d4, q5
    0x0f, 0x83, 0xd0, 0x41,     // sqrdmulh d1. d2, d3
    0x4f, 0x8b, 0x59, 0x45,     // fmls    q5, q10, q11
    0x6f, 0x9f, 0x21, 0x04,     // umlal   q4, hq8, d31
    0x0f, 0x88, 0x19, 0x84,     // fmla    d4, d12, q8
    0x4c, 0x00, 0x71, 0x04,     // st1     q4, [x8]
    0x0c, 0x00, 0x83, 0xe9,     // st2     d9, d10, [sp]
    0x4c, 0x00, 0xa3, 0xc2,     // st1     q2, q3, [x30]
    0x4c, 0x40, 0x63, 0xfe,     // ld1     q30, q31, q0, [sp]
    0x0c, 0x40, 0x20, 0x01,     // ld1     d1, d2, d3, d4, [x0]
    0x4c, 0x40, 0x40, 0x25,     // ld3     q5, q6, q7, [x1]
    0x0c, 0x94, 0x00, 0x40,     // st4     d0, d1, d2, d3, [x2], x20
    0x4c, 0x85, 0x23, 0xfe,     // st1     q30, q31, q0, q1, [sp], x5
    0x4c, 0x87, 0x61, 0x04,     // st1     q4, q5, q6, [x8], x7
    0x0c, 0xde, 0xa3, 0xff,     // ld1     d31, d0, [sp], x30
    0x4c, 0xc0, 0x71, 0x45,     // ld1     q5, [x10], x0
    0x0c, 0x9f, 0x43, 0xe5,     // st3     d5, d6, d7, [sp], #24
    0x4c, 0x9f, 0x20, 0x5f,     // st1     q31, q0, q1, q2, [x2], #48
    0x4c, 0xdf, 0x63, 0xe1,     // ld1     q1, q2, q3, [sp], #48
    0x0c, 0xdf, 0xa3, 0xdf,     // ld1     d31, d0, [x30], #16
    0x4c, 0x9f, 0x70, 0x04,     // st1     q4, [x0], #16
    0x4d, 0x00, 0x00, 0x45,     // st1     q5, [x2]
    0x0d, 0x20, 0x43, 0xe4,     // st2     d4, d5, [sp]
    0x4d, 0x00, 0xa1, 0x1f,     // st3     q31, q0, q1, [x8]
    0x0d, 0x20, 0xa7, 0xe0,     // st4     d0, d1, d2, d3, [sp]
    0x4d, 0x20, 0x43, 0xd5,     // st2     q21, q22, [x30]
    0x0d, 0x20, 0xa4, 0x1e,     // st4     d30, d31, d0, d1, [x0]
    0x4d, 0x40, 0x00, 0x45,     // ld1     q5, [x2]
    0x0d, 0x60, 0x43, 0xe4,     // ld2     d4, d5, [sp]
    0x4d, 0x40, 0xa1, 0x1f,     // ld3     q31, q0, q1, [x8]
    0x0d, 0x60, 0xa7, 0xe0,     // ld4     d0, d1, d2, d3, [sp]
    0x0d, 0x40, 0x80, 0x09,     // ld1     d9, [x0]
    0x4d, 0x40, 0xa4, 0x21,     // ld3     q1, q2, q3, [x1]
    0x4d, 0x88, 0x00, 0x45,     // st1     q5, [x2], x8
    0x0d, 0xbf, 0x43, 0xe4,     // st2     d4, d5, [sp], #4
    0x4d, 0x9e, 0xa1, 0x1f,     // st3     q31, q0, q1, [x8], x30
    0x0d, 0xbf, 0xa7, 0xe0,     // st4     d0, d1, d2, d3, [sp], #32
    0x4d, 0xdf, 0x00, 0x45,     // ld1     q5,[x2], #1
    0x0d, 0xe0, 0x43, 0xe4,     // ld2     d4, d5, [sp], x0
    0x4d, 0xdf, 0xa1, 0x1f,     // ld3     q31, q0, q1, [x8], #12
    0x0d, 0xe9, 0xa7, 0xe0,     // ld4     d0, d1, d2, d3, [sp], x9
    0x0f, 0x08, 0x04, 0x82,     // sshr    d2, d4, #8
    0x4f, 0x40, 0x14, 0x20,     // ssra    q0, q1, #64
    0x0f, 0x09, 0x57, 0xff,     // shl     d31, d31, #1
    0x4f, 0x30, 0x76, 0x08,     // sqshl   q8, q16, #16
    0x0f, 0x10, 0x9d, 0x02,     // sqrshrn d2, d8, #16
    0x4f, 0x20, 0xfc, 0x00,     // fcvtzs  q0, q0, #32
    0x2f, 0x28, 0x55, 0x04,     // sli     d4, d8, #8
    0x6f, 0x09, 0x94, 0xa2      // uqshrn  q2, q6, #7
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

  namespace R = Dyninst::aarch64;
  using reg_set = Dyninst::register_set;

  std::vector<reg_set> expectedRead, expectedWritten;

  // SADDLV H0, Q1.8B
  expectedRead.push_back(reg_set{R::q1});
  expectedWritten.push_back(reg_set{R::h0});

  // SMAXV B15, D8
  expectedRead.push_back(reg_set{R::d8});
  expectedWritten.push_back(reg_set{R::b15});

  // UADDLV S2, Q2
  expectedRead.push_back(reg_set{R::q2});
  expectedWritten.push_back(reg_set{R::s2});

  // UMINV B5, D0
  expectedRead.push_back(reg_set{R::d0});
  expectedWritten.push_back(reg_set{R::b5});

  // FMAXNMV S5, Q1
  expectedRead.push_back(reg_set{R::q1});
  expectedWritten.push_back(reg_set{R::s5});

  // DUP D1, Q2
  expectedRead.push_back(reg_set{R::q2});
  expectedWritten.push_back(reg_set{R::d1});

  // DUP Q20, W5
  expectedRead.push_back(reg_set{R::w6});
  expectedWritten.push_back(reg_set{R::q24});

  // DUP D0, X2
  expectedRead.push_back(reg_set{R::x2});
  expectedWritten.push_back(reg_set{R::d0});

  // INS Q4, X0
  expectedRead.push_back(reg_set{R::x0});
  expectedWritten.push_back(reg_set{R::q4});

  // INS Q31, WZR
  expectedRead.push_back(reg_set{R::wzr});
  expectedWritten.push_back(reg_set{R::q31});

  // SMOV W1, D5
  expectedRead.push_back(reg_set{R::d5});
  expectedWritten.push_back(reg_set{R::w1});

  // UMOV X2, Q2
  expectedRead.push_back(reg_set{R::q2});
  expectedWritten.push_back(reg_set{R::x2});

  // INS Q0, D0
  expectedRead.push_back(reg_set{R::d0});
  expectedWritten.push_back(reg_set{R::q0});

  // EXT D0, D1, D2, #5
  expectedRead.push_back(reg_set{R::d1, R::d2});
  expectedWritten.push_back(reg_set{R::d0});

  // EXT Q8, Q16, Q2, #15
  expectedRead.push_back(reg_set{R::q16, R::q2});
  expectedWritten.push_back(reg_set{R::q8});

  // MOVI D0, #8
  expectedRead.push_back(reg_set{});
  expectedWritten.push_back(reg_set{R::d0});

  // MOVI Q1, FF LSL #8
  expectedRead.push_back(reg_set{});
  expectedWritten.push_back(reg_set{R::q1});

  // ORR D5, #0 LSL #24
  expectedRead.push_back(reg_set{R::d5});
  expectedWritten.push_back(reg_set{R::d5});

  // ORR D5, #0 LSL #8
  expectedRead.push_back(reg_set{R::d5});
  expectedWritten.push_back(reg_set{R::d5});

  // FMOV Q8, FF
  expectedRead.push_back(reg_set{});
  expectedWritten.push_back(reg_set{R::q8});

  // MVNI Q0, #12 LSL #8
  expectedRead.push_back(reg_set{});
  expectedWritten.push_back(reg_set{R::q0});

  // BIC D8, #24 LSL #0
  expectedRead.push_back(reg_set{R::q8});
  expectedWritten.push_back(reg_set{R::q8});

  // MOVI D2, (all ones)
  expectedRead.push_back(reg_set{});
  expectedWritten.push_back(reg_set{R::d2});

  // UZP1 D8, D16, D0
  expectedRead.push_back(reg_set{R::d16, R::d0});
  expectedWritten.push_back(reg_set{R::d8});

  // TRN1 Q2, Q3, Q4
  expectedRead.push_back(reg_set{R::q3, R::q4});
  expectedWritten.push_back(reg_set{R::q2});

  // ZIP2 D8, D8, D8
  expectedRead.push_back(reg_set{R::d8});
  expectedWritten.push_back(reg_set{R::d8});

  // DUP B5, Q1
  expectedRead.push_back(reg_set{R::q1});
  expectedWritten.push_back(reg_set{R::b5});

  // DUP D8, D8
  expectedRead.push_back(reg_set{R::d8});
  expectedWritten.push_back(reg_set{R::d8});

  // ADDP D2, Q5
  expectedRead.push_back(reg_set{R::q5});
  expectedWritten.push_back(reg_set{R::d2});

  // FMAXNMP S1, D4
  expectedRead.push_back(reg_set{R::d4});
  expectedWritten.push_back(reg_set{R::s1});

  // FMINNMP D15, Q15
  expectedRead.push_back(reg_set{R::q15});
  expectedWritten.push_back(reg_set{R::d15});

  // SSHR D2, D4, #8
  expectedRead.push_back(reg_set{R::d4});
  expectedWritten.push_back(reg_set{R::d2});

  // SSRA D0, D1, #64
  expectedRead.push_back(reg_set{R::d1});
  expectedWritten.push_back(reg_set{R::d0});

  // SHL D31, D31, #1
  expectedRead.push_back(reg_set{R::d31});
  expectedWritten.push_back(reg_set{R::d31});

  // SQSHL B8, B16, 6
  expectedRead.push_back(reg_set{R::b16});
  expectedWritten.push_back(reg_set{R::b8});

  // SQRSHRN H2, H8, #16
  expectedRead.push_back(reg_set{R::s8});
  expectedWritten.push_back(reg_set{R::h2});

  // FCVTZS S0, S0, #32
  expectedRead.push_back(reg_set{R::s0});
  expectedWritten.push_back(reg_set{R::s0});

  // SLI D4, D8, #8
  expectedRead.push_back(reg_set{R::d8});
  expectedWritten.push_back(reg_set{R::d4});

  // UQSHRN B2, H2, #7
  expectedRead.push_back(reg_set{R::h2});
  expectedWritten.push_back(reg_set{R::b2});

  // SQDMLAL S8, H8, H0
  expectedRead.push_back(reg_set{R::h8, R::h0});
  expectedWritten.push_back(reg_set{R::s8});

  // SQDMLSL D2, S4, S1
  expectedRead.push_back(reg_set{R::s4, R::s1});
  expectedWritten.push_back(reg_set{R::d2});

  // SQDMULL S0, H0, H31
  expectedRead.push_back(reg_set{R::h0, R::h31});
  expectedWritten.push_back(reg_set{R::s0});

  // CMGT D2, D1, D0
  expectedRead.push_back(reg_set{R::d1, R::d0});
  expectedWritten.push_back(reg_set{R::d2});

  // SQSHL H0, H2, H4
  expectedRead.push_back(reg_set{R::h2, R::h4});
  expectedWritten.push_back(reg_set{R::h0});

  // SQDMULH S31, S1, S0
  expectedRead.push_back(reg_set{R::s1, R::s0});
  expectedWritten.push_back(reg_set{R::s31});

  // FRECPS D8, D8, D8
  expectedRead.push_back(reg_set{R::d8});
  expectedWritten.push_back(reg_set{R::d8});

  // URSHL D4, D31, D2
  expectedRead.push_back(reg_set{R::d31, R::d2});
  expectedWritten.push_back(reg_set{R::d4});

  // FCMGT S2, S8, S16
  expectedRead.push_back(reg_set{R::s8, R::s16});
  expectedWritten.push_back(reg_set{R::s2});

  // SUQADD B2, B8
  expectedRead.push_back(reg_set{R::b8});
  expectedWritten.push_back(reg_set{R::b2});

  // SQXTN H4, S0
  expectedRead.push_back(reg_set{R::s0});
  expectedWritten.push_back(reg_set{R::h4});

  // FCVTMS D31, D1
  expectedRead.push_back(reg_set{R::d1});
  expectedWritten.push_back(reg_set{R::d31});

  // FRECPE S9, S5
  expectedRead.push_back(reg_set{R::s5});
  expectedWritten.push_back(reg_set{R::s9});

  // SQXTUN B8, H2
  expectedRead.push_back(reg_set{R::h2});
  expectedWritten.push_back(reg_set{R::b8});

  // FCVTXN S15, D2
  expectedRead.push_back(reg_set{R::d2});
  expectedWritten.push_back(reg_set{R::s15});

  // FRSQRTE S31, S31
  expectedRead.push_back(reg_set{R::s31});
  expectedWritten.push_back(reg_set{R::s31});

  // SQDMLAL S2, H5, Q9
  expectedRead.push_back(reg_set{R::s2, R::h5, R::q9});
  expectedWritten.push_back(reg_set{R::s2});

  // SQDMLSL D0, S31, D31
  expectedRead.push_back(reg_set{R::d0, R::d31, R::s31});
  expectedWritten.push_back(reg_set{R::d0});

  // SQRDMULH H8, H4, D15
  expectedRead.push_back(reg_set{R::h4, R::d15});
  expectedWritten.push_back(reg_set{R::h8});

  // FMLA S9, S2, Q20
  expectedRead.push_back(reg_set{R::s2, R::s9, R::q20});
  expectedWritten.push_back(reg_set{R::s9});

  // FMLS D8, D5, D17
  expectedRead.push_back(reg_set{R::d5, R::d8, R::d17});
  expectedWritten.push_back(reg_set{R::d8});

  // TBL Q4, Q8, Q9
  expectedRead.push_back(reg_set{R::q8, R::q9});
  expectedWritten.push_back(reg_set{R::q4});

  // TBX D0, D1, D2, D3
  expectedRead.push_back(reg_set{R::d1, R::d2, R::d3});
  expectedWritten.push_back(reg_set{R::d0});

  // TBL Q0, Q24, Q25, Q26, Q31
  expectedRead.push_back(reg_set{R::q24, R::q25, R::q26, R::q31});
  expectedWritten.push_back(reg_set{R::q0});

  // TBX D20, D0, D1, D2, D3, D21
  expectedRead.push_back(reg_set{R::d0, R::d1, R::d2, R::d3, R::d21});
  expectedWritten.push_back(reg_set{R::d20});

  // SADDL Q1, HQ2, HQ3
  expectedRead.push_back(reg_set{R::hq2, R::hq3});
  expectedWritten.push_back(reg_set{R::q1});

  // SSUBL Q31, D4, D5
  expectedRead.push_back(reg_set{R::d4, R::d5});
  expectedWritten.push_back(reg_set{R::q31});

  // ADDHN HQ2, Q8, Q16
  expectedRead.push_back(reg_set{R::q8, R::q16});
  expectedWritten.push_back(reg_set{R::hq2});

  // SABDL Q5, D2, D8
  expectedRead.push_back(reg_set{R::d2, R::d8});
  expectedWritten.push_back(reg_set{R::q5});

  // SQDMLSL Q0, HQ1, HQ2
  expectedRead.push_back(reg_set{R::hq1, R::hq2});
  expectedWritten.push_back(reg_set{R::q0});

  // RSUBHN D2, Q15, Q31
  expectedRead.push_back(reg_set{R::q15, R::q31});
  expectedWritten.push_back(reg_set{R::d2});

  // SUBHN HQ8, Q2,Q3
  expectedRead.push_back(reg_set{R::q2, R::q3});
  expectedWritten.push_back(reg_set{R::hq8});

  // SQADD D2, D3, D5
  expectedRead.push_back(reg_set{R::d3, R::d5});
  expectedWritten.push_back(reg_set{R::d2});

  // CMGT Q0, Q1, Q31
  expectedRead.push_back(reg_set{R::q1, R::q31});
  expectedWritten.push_back(reg_set{R::q0});

  // SABA D8, D9, D10
  expectedRead.push_back(reg_set{R::d9, R::d10});
  expectedWritten.push_back(reg_set{R::d8});

  // BSL Q4, Q5, Q6
  expectedRead.push_back(reg_set{R::q5, R::q6});
  expectedWritten.push_back(reg_set{R::q4});

  // CNT D8, D9
  expectedRead.push_back(reg_set{R::d9});
  expectedWritten.push_back(reg_set{R::d8});

  // XTN Q0, Q31
  expectedRead.push_back(reg_set{R::q31});
  expectedWritten.push_back(reg_set{R::q0});

  // FCVTAS Q2, Q3
  expectedRead.push_back(reg_set{R::q3});
  expectedWritten.push_back(reg_set{R::q2});

  // FSQRT D31, D15
  expectedRead.push_back(reg_set{R::d15});
  expectedWritten.push_back(reg_set{R::d31});

  // SMLAL Q2, D5, D0
  expectedRead.push_back(reg_set{R::d5, R::d0, R::q2});
  expectedWritten.push_back(reg_set{R::q2});

  // SMLSL Q1, HQ31, Q5
  expectedRead.push_back(reg_set{R::q1, R::hq31, R::q5});
  expectedWritten.push_back(reg_set{R::q1});

  // MUL Q9, Q2, D31
  expectedRead.push_back(reg_set{R::q2, R::d31});
  expectedWritten.push_back(reg_set{R::q9});

  // SQDMULL Q8, D4, Q5
  expectedRead.push_back(reg_set{R::d4, R::q5});
  expectedWritten.push_back(reg_set{R::q8});

  // SQRDMULH D1. D2, D3
  expectedRead.push_back(reg_set{R::d2, R::d3});
  expectedWritten.push_back(reg_set{R::d1});

  // FMLS Q5, Q10, Q11
  expectedRead.push_back(reg_set{R::q5, R::q10, R::q11});
  expectedWritten.push_back(reg_set{R::q5});

  // UMLAL Q4, HQ8, D31
  expectedRead.push_back(reg_set{R::q4, R::hq8, R::d31});
  expectedWritten.push_back(reg_set{R::q4});

  // FMLA D4, D12, Q8
  expectedRead.push_back(reg_set{R::d4, R::d12, R::q8});
  expectedWritten.push_back(reg_set{R::d4});

  // ST1 Q4, [X8]
  expectedRead.push_back(reg_set{R::q4, R::x8});
  expectedWritten.push_back(reg_set{});

  // ST2 D9, D10, [SP]
  expectedRead.push_back(reg_set{R::d9, R::d10, R::sp});
  expectedWritten.push_back(reg_set{});

  // ST1 Q2, Q3, [X30]
  expectedRead.push_back(reg_set{R::q2, R::q3, R::x30});
  expectedWritten.push_back(reg_set{});

  // LD1 Q30, Q31, Q0, [SP]
  expectedRead.push_back(reg_set{R::sp});
  expectedWritten.push_back(reg_set{R::q0, R::q30, R::q31});

  // LD1 D1, D2, D3, D4, [X0]
  expectedRead.push_back(reg_set{R::x0});
  expectedWritten.push_back(reg_set{R::d1, R::d2, R::d3, R::d4});

  // LD3 Q5, Q6, Q7, [X1]
  expectedRead.push_back(reg_set{R::x1});
  expectedWritten.push_back(reg_set{R::q5, R::q6, R::q7});

  // ST4 D0, D1, D2, D3, [X2], X20
  expectedRead.push_back(reg_set{R::x2, R::x20, R::d0, R::d1, R::d2, R::d3});
  expectedWritten.push_back(reg_set{R::x2});

  // ST1 Q30, Q31, Q0, Q1, [SP], X5
  expectedRead.push_back(reg_set{R::q30, R::q31, R::q0, R::q1, R::sp, R::x5});
  expectedWritten.push_back(reg_set{R::sp});

  // ST1 Q4, Q5, Q6, [X8], X7
  expectedRead.push_back(reg_set{R::q4, R::q5, R::q6, R::x7, R::x8});
  expectedWritten.push_back(reg_set{R::x8});

  // LD1 D31, D0, [SP], X30
  expectedRead.push_back(reg_set{R::sp, R::x30});
  expectedWritten.push_back(reg_set{R::d0, R::d31, R::sp});

  // LD1 Q5, [X10], X0
  expectedRead.push_back(reg_set{R::x10, R::x0});
  expectedWritten.push_back(reg_set{R::q5, R::x10});

  // ST3 D5, D6, D7, [SP], #24
  expectedRead.push_back(reg_set{R::d5, R::d6, R::d7, R::sp});
  expectedWritten.push_back(reg_set{R::sp});

  // ST1 Q31, Q0, Q1, Q2, [X2], #48
  expectedRead.push_back(reg_set{R::q0, R::q1, R::q2, R::q31, R::x2});
  expectedWritten.push_back(reg_set{R::x2});

  // LD1 Q1, Q2, Q3, [SP], #48
  expectedRead.push_back(reg_set{R::sp});
  expectedWritten.push_back(reg_set{R::q1, R::q2, R::q3, R::sp});

  // LD1 D31, D0, [X30], #16
  expectedRead.push_back(reg_set{R::x30});
  expectedWritten.push_back(reg_set{R::d0, R::d31, R::x30});

  // ST1 Q4, [X0], #16
  expectedRead.push_back(reg_set{R::q4, R::x0});
  expectedWritten.push_back(reg_set{R::x0});

  // ST1 Q5, [X2]
  expectedRead.push_back(reg_set{R::q5, R::x2});
  expectedWritten.push_back(reg_set{});

  // ST2 D4, D5, [SP]
  expectedRead.push_back(reg_set{R::d4, R::d5, R::sp});
  expectedWritten.push_back(reg_set{});

  // ST3 Q31, Q0, Q1, [X8]
  expectedRead.push_back(reg_set{R::q0, R::q1, R::q31, R::x8});
  expectedWritten.push_back(reg_set{});

  // ST4 D0, D1, D2, D3, [SP]
  expectedRead.push_back(reg_set{R::d0, R::d1, R::d2, R::d3, R::sp});
  expectedWritten.push_back(reg_set{});

  // ST2 Q21, Q22, [X30]
  expectedRead.push_back(reg_set{R::q21, R::q22, R::x30});
  expectedWritten.push_back(reg_set{});

  // ST4 D30, D31, D0, D1, [X0]
  expectedRead.push_back(reg_set{R::d0, R::d1, R::d30, R::d31, R::x0});
  expectedWritten.push_back(reg_set{});

  // LD1 Q5, [X2]
  expectedRead.push_back(reg_set{R::x2});
  expectedWritten.push_back(reg_set{R::q5});

  // LD2 D4, D5, [SP]
  expectedRead.push_back(reg_set{R::sp});
  expectedWritten.push_back(reg_set{R::d4, R::d5});

  // LD3 Q31, Q0, Q1, [X8]
  expectedRead.push_back(reg_set{R::x8});
  expectedWritten.push_back(reg_set{R::q0, R::q1, R::q31});

  // LD4 D0, D1, D2, D3, [SP]
  expectedRead.push_back(reg_set{R::sp});
  expectedWritten.push_back(reg_set{R::d0, R::d1, R::d2, R::d3});

  // LD1 D9, [X0]
  expectedRead.push_back(reg_set{R::x0});
  expectedWritten.push_back(reg_set{R::d9});

  // LD3 Q1, Q2, Q3, [X1]
  expectedRead.push_back(reg_set{R::x1});
  expectedWritten.push_back(reg_set{R::q1, R::q2, R::q3});

  // ST1 Q5, [X2], X8
  expectedRead.push_back(reg_set{R::q5, R::x2, R::x8});
  expectedWritten.push_back(reg_set{R::x2});

  // ST2 D4, D5, [SP], #4
  expectedRead.push_back(reg_set{R::d4, R::d5, R::sp});
  expectedWritten.push_back(reg_set{R::sp});

  // ST3 Q31, Q0, Q1, [X8], X30
  expectedRead.push_back(reg_set{R::q0, R::q1, R::q31, R::x8, R::x30});
  expectedWritten.push_back(reg_set{R::x8});

  // ST4 D0, D1, D2, D3, [SP], #32
  expectedRead.push_back(reg_set{R::d0, R::d1, R::d2, R::d3, R::sp});
  expectedWritten.push_back(reg_set{R::sp});

  // LD1 Q5,[X2], #1
  expectedRead.push_back(reg_set{R::x2});
  expectedWritten.push_back(reg_set{R::q5, R::x2});

  // LD2 D4, D5, [SP], X0
  expectedRead.push_back(reg_set{R::sp, R::x0});
  expectedWritten.push_back(reg_set{R::sp, R::d4, R::d5});

  // LD3 Q31, Q0, Q1, [X8], #12
  expectedRead.push_back(reg_set{R::x8});
  expectedWritten.push_back(reg_set{R::q0, R::q1, R::q31, R::x8});

  // LD4 D0, D1, D2, D3, [SP], X9
  expectedRead.push_back(reg_set{R::sp, R::x9});
  expectedWritten.push_back(reg_set{R::d0, R::d1, R::d2, R::d3, R::sp});

  // SSHR D2, D4, #8
  expectedRead.push_back(reg_set{R::d4});
  expectedWritten.push_back(reg_set{R::d2});

  // SSRA Q0, Q1, #64
  expectedRead.push_back(reg_set{R::q1});
  expectedWritten.push_back(reg_set{R::q0});

  // SHL D31, D31, #1
  expectedRead.push_back(reg_set{R::d31});
  expectedWritten.push_back(reg_set{R::d31});

  // SQSHL Q8, Q16, #16
  expectedRead.push_back(reg_set{R::q16});
  expectedWritten.push_back(reg_set{R::q8});

  // SQRSHRN D2, D8, #16
  expectedRead.push_back(reg_set{R::d8});
  expectedWritten.push_back(reg_set{R::d2});

  // FCVTZS Q0, Q0, #32
  expectedRead.push_back(reg_set{R::q0});
  expectedWritten.push_back(reg_set{R::q0});

  // SLI D4, D8, #8
  expectedRead.push_back(reg_set{R::d8});
  expectedWritten.push_back(reg_set{R::d4});

  // UQSHRN Q2, Q6, #7
  expectedRead.push_back(reg_set{R::q5});
  expectedWritten.push_back(reg_set{R::q2});

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
