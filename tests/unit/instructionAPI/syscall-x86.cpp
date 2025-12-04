#include "InstructionDecoder.h"
#include "instructionAPI/h/syscalls.h"

#include <iostream>
#include <array>

namespace di = Dyninst::InstructionAPI;

static bool run_32();
static bool run_64();

int main() {
  bool ok = run_32();

  if(!run_64()) {
    ok = false;
  }

  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}


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
static bool run(di::InstructionDecoder dec, std::array<bool, N> const& answers) {
  bool failed = false;
  for(auto i=0; i < N; i++) {
    auto insn = dec.decode();
    if(!insn.isValid()) {
      std::cerr << "Decode failed for test " << (i+1) << "\n";
      failed = true;
      continue;
    }
    auto const expected = answers[i];
    auto const actual = di::isSystemCall(insn);
    if(actual != expected) {
      std::cerr << "Test (" << (i+1) << "), '" << insn.format() << "' failed. Expected '"
                << std::boolalpha << expected << "', got '" << actual << "'\n";
      failed = true;
    }
  }
  return !failed;
}

bool run_32() {
  auto constexpr arch = Dyninst::Arch_x86;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'syscall-x86' in " << sarch << " mode\n";

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

  di::InstructionDecoder decoder(buffer.data(), buffer.size(), arch);
  bool failed = !run<num_tests>(decoder, answers);

  // `into` is only valid in 32-bit mode
  constexpr auto num_32bit_tests = 1;
  std::array<const unsigned char, num_32bit_tests> buffer32 = {{ 0xce }};
  std::array<bool, num_32bit_tests> answers32 = {{ false }};
  di::InstructionDecoder decoder32(buffer32.data(), buffer32.size(), arch);

  if(!run<num_32bit_tests>(decoder32, answers32)) {
    failed = true;
  }

  return !failed;
}

bool run_64()  {
  auto constexpr arch = Dyninst::Arch_x86_64;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'syscall-x86' in " << sarch << " mode\n";

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

  di::InstructionDecoder decoder(buffer.data(), buffer.size(), arch);
  return run<num_tests>(decoder, answers);
}
