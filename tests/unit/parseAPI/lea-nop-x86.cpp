#include "InstructionDecoder.h"
#include "parseAPI/src/IA_x86.h"

#include <iostream>
#include <array>

int main() {
  constexpr auto num_bytes = 147UL;
  constexpr auto num_instructions = 27;

  // clang-format off
  std::array<const unsigned char, num_bytes> buffer = {{
    0x48, 0x8d, 0x00,                                 // (0) lea rax, [rax]
    0x48, 0x8d, 0x04, 0x00,                           // (1) lea rax, [rax + rax*1 + 0 ]
    0x48, 0x8d, 0x44, 0x00, 0x01,                     // (2) lea rax, [rax + rax*1 + 1 ]
    0x48, 0x8d, 0x00,                                 // (3) lea rax, [rax + 0]
    0x48, 0x8d, 0x40, 0x01,                           // (4) lea rax, [rax + 1]
    0x48, 0x8d, 0x40, 0xff,                           // (5) lea rax, [rax - 1]
    0x48, 0x8d, 0x40, 0x01,                           // (6) lea rax, [1 + rax]
    0x48, 0x8d, 0x04, 0x05, 0x00, 0x00, 0x00, 0x00,   // (7) lea rax, [rax*1]
    0x48, 0x8d, 0x04, 0x05, 0x00, 0x00, 0x00, 0x00,   // (8) lea rax, [1*rax]
    0x48, 0x8d, 0x04, 0x05, 0x00, 0x00, 0x00, 0x00,   // (9) lea rax, [rax*1 + 0]
    0x48, 0x8d, 0x04, 0x05, 0x01, 0x00, 0x00, 0x00,   // (10) lea rax, [rax*1 + 1]
    0x48, 0x8d, 0x03,                                 // (11) lea rax, [rbx]
    0x48, 0x8d, 0x04, 0x1b,                           // (12) lea rax, [rbx + rbx*1 + 0 ]
    0x48, 0x8d, 0x44, 0x1b, 0x01,                     // (13) lea rax, [rbx + rbx*1 + 1 ]
    0x48, 0x8d, 0x03,                                 // (14) lea rax, [rbx + 0]
    0x48, 0x8d, 0x43, 0x01,                           // (15) lea rax, [rbx + 1]
    0x48, 0x8d, 0x43, 0xff,                           // (16) lea rax, [rbx - 1]
    0x48, 0x8d, 0x43, 0x01,                           // (17) lea rax, [1 + rbx]
    0x48, 0x8d, 0x04, 0x1d, 0x00, 0x00, 0x00, 0x00,   // (18) lea rax, [rbx*1]
    0x48, 0x8d, 0x04, 0x1d, 0x00, 0x00, 0x00, 0x00,   // (19) lea rax, [1*rbx]
    0x48, 0x8d, 0x04, 0x1d, 0x00, 0x00, 0x00, 0x00,   // (20) lea rax, [rbx*1 + 0]
    0x48, 0x8d, 0x04, 0x1d, 0x01, 0x00, 0x00, 0x00,   // (21) lea rax, [rbx*1 + 1]
    0x48, 0x8d, 0x04, 0x25, 0x56, 0x00, 0x00, 0x00,   // (22) lea rax, [ 12*7 + 2]  (lea rax,ds:0x56)
    0x8d, 0xb4, 0x26, 0x00, 0x00, 0x00, 0x00,         // (23) lea esi,[rsi+riz*1+0x0] (also, capstone:'lea esi, [rsi + riz]', xed:'lea esi, [rsi]', bddisasm:'lea esi, [rsi+0x0]')
    0x48, 0x8d, 0xb4, 0x26, 0x00, 0x00, 0x00, 0x00,   // (24) lea rsi,[rsi+riz*1+0x0] (also, capstone:'lea rsi, [rsi + riz]', xed:'lea rsi, [rsi]', bddisasm:'lea rsi, [rsi+0x0]')
    0x8d, 0x00,                                       // (25) lea eax, [rax]
    0x67, 0x48, 0x8d, 0x00                            // (26) lea rax, [eax]
  }};

  std::array<bool, num_instructions> expected = {{
    true,   // (0)
    false,  // (1)
    false,  // (2)
    true,   // (3)
    false,  // (4)
    false,  // (5)
    false,  // (6)
    true,   // (7)
    true,   // (8)
    true,   // (9)
    false,  // (10)
    false,  // (11)
    false,  // (12)
    false,  // (13)
    false,  // (14)
    false,  // (15)
    false,  // (16)
    false,  // (17)
    false,  // (18)
    false,  // (19)
    false,  // (20)
    false,  // (21)
    false,  // (22)
    false,  // (23)
    true,   // (24)
    false,  // (25)
    false,  // (26)
  }};
  // clang-format on

  namespace di = Dyninst::InstructionAPI;

  di::InstructionDecoder decoder(buffer.data(), buffer.size(),
                                 Dyninst::Architecture::Arch_x86_64);

  auto adapter = Dyninst::InsnAdapter::IA_x86(decoder, 0x0, nullptr, nullptr, nullptr, nullptr);

  int test_num=0;

  while(!adapter.isInvalidInsn()) {
    if(expected[test_num] != adapter.isNop()) {
      std::cout << "Test " << (test_num+1) << " failed\n";
      return -1;
    }
    adapter.advance();
    test_num++;
  }
}
