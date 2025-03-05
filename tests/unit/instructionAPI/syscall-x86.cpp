#include "InstructionDecoder.h"
#include "instructionAPI/h/syscalls.h"

#include <iostream>
#include <array>

namespace di = Dyninst::InstructionAPI;

constexpr auto num_tests = 14;
constexpr auto num_bytes = 62UL;
std::array<const unsigned char, num_bytes> buffer = {{
  0xe8, 0x10, 0x00, 0x00, 0x00,                     // (1)  call 0x10 + rip
  0x64, 0xff, 0x14, 0x25, 0x10, 0x00, 0x00, 0x00,   // (2)  call  [q|d]word ptr fs:[0x10]
  0x64, 0xff, 0x15, 0x10, 0x00, 0x00, 0x00,         // (3)  32-bit 'call dword ptr fs:0x10', 64-bit 'call qword ptr fs:[rip + 0x10]'
  0xff, 0x14, 0x25, 0x10, 0x00, 0x00, 0x00,         // (4)  call  [q|d]word ptr [0x10]
  0xff, 0x15, 0x10, 0x00, 0x00, 0x00,               // (5)  32-bit 'call dword ptr [0x10]', 64-bit 'call qword ptr [rip + 0x10]'
  0x65, 0xff, 0x14, 0x25, 0x10, 0x00, 0x00, 0x00,   // (6)  call  [q|d]word ptr gs:[0x10]
  0x65, 0xff, 0x15, 0x10, 0x00, 0x00, 0x00,         // (7)  32-bit 'call dword ptr gs:[0x10]', 64-bit 'call qword ptr gs:[rip + 0x10]'
  0x65, 0xff, 0x50, 0x10,                           // (8)  call gs:[rax + 0x10]
  0xcd, 0x80,                                       // (9)  int 0x80
  0xcc,                                             // (10) int3
  0xf1,                                             // (11) int1
  0x0f, 0x05,                                       // (12) syscall
  0x0f, 0x34,                                       // (13) sysenter
  0xcd, 0x0a                                        // (14) int 0x0a
}};

template <int N>
bool run(di::InstructionDecoder dec, std::array<bool, N> const& answers) {
  for(auto i=0; i < N; i++) {
    auto insn = dec.decode();
    if(!insn.isValid()) {
      std::cerr << "Decode failed for test " << (i+1) << "\n";
      return false;
    }
    if(answers[i] != di::isSystemCall(insn)) {
      std::cerr << "Test " << (i+1) << " failed\n";
      return false;
    }
  }
  return true;
}

bool run_32() {
  {
    std::array<bool, num_tests> answers = {{
      false,    // (1)
      false,    // (2)
      false,    // (3)
      false,    // (4)
      false,    // (5)
      true,     // (6)
      true,     // (7)
      false,    // (8)
      true,     // (9)
      false,    // (10)
      false,    // (11)
      true,     // (12)
      false,    // (13)
      true,     // (14)
    }};

    di::InstructionDecoder decoder(buffer.data(), buffer.size(), Dyninst::Arch_x86);
    if(!run<num_tests>(decoder, answers)) {
      return false;
    }
  }

  {
    // `into` is only valid in 32-bit mode
    constexpr auto num_32bit_tests = 1;
    std::array<const unsigned char, num_32bit_tests> buffer32 = {{ 0xce }};
    std::array<bool, num_32bit_tests> answers32 = {{ false }};
    di::InstructionDecoder decoder32(buffer32.data(), buffer32.size(), Dyninst::Arch_x86);

    if(!run<num_32bit_tests>(decoder32, answers32)) {
      return false;
    }
  }
  return true;
}

bool run_64()  {
  std::array<bool, num_tests> answers = {{
    false,    // (1)
    false,    // (2)
    false,    // (3)
    false,    // (4)
    false,    // (5)
    false,    // (6)
    false,    // (7)
    false,    // (8)
    true,     // (9)
    false,    // (10)
    false,    // (11)
    true,     // (12)
    false,    // (13)
    true,     // (14)
  }};

  di::InstructionDecoder decoder(buffer.data(), buffer.size(), Dyninst::Arch_x86_64);
  if(!run<num_tests>(decoder, answers)) {
    return false;
  }

  return true;
}

void usage(char const* prgname) {
  std::cerr << "Usage: " << prgname << " [32|64]\n";
}

int main(int argc, char **argv) {
  // Convention for CTest
  constexpr int PASS =  0;
  constexpr int FAIL =  1;

  if(argc != 2) {
    usage(argv[0]);
    return FAIL;
  }

  std::string type{argv[1]};

  if(type == "32") {
    std::cout << "Running in 32-bit mode\n";
    return run_32() ? PASS : FAIL;
  }
  if(type == "64") {
    std::cout << "Running in 64-bit mode\n";
    return run_64() ? PASS : FAIL;
  }

  usage(argv[0]);
  return FAIL;
}
