#include "Architecture.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "register_tests.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/x86_regs.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  x87 floating-point comparison decoding tests
 *
 *  These verify the register read/write sets produced by the decoder
 *  for the fcom/fcomp/fcompp, fcomi/fcomip, and fucom/fucomp/fucompp/
 *  fucomi/fucomip families.
 */

namespace di = Dyninst::InstructionAPI;

int main() {
  constexpr auto num_tests = 24;

  // clang-format off
  constexpr std::array<const uint8_t, 52> buffer = {{
      // ordered comparisons
      0xd8, 0x55, 0x00,   // fcom dword ptr [ebp]
      0xdc, 0x55, 0x00,   // fcom qword ptr [ebp]
      0xd8, 0xd0,         // fcom st0
      0xd8, 0xd1,         // fcom st1
      0xd8, 0xd2,         // fcom st2
      0xd8, 0x5d, 0x00,   // fcomp dword ptr [ebp]
      0xdc, 0x5d, 0x00,   // fcomp qword ptr [ebp]
      0xd8, 0xd8,         // fcomp st0
      0xd8, 0xd9,         // fcomp st1
      0xd8, 0xda,         // fcomp st2
      0xde, 0xd9,         // fcompp

      // compare and set eflags
      0xdb, 0xf0,         // fcomi st0, st0
      0xdf, 0xf0,         // fcomip st0, st0
      0xdf, 0xf1,         // fcomip st0, st1
      0xdb, 0xe8,         // fucomi st0, st0
      0xdb, 0xe9,         // fucomi st0, st1
      0xdf, 0xe8,         // fucomip st0, st0
      0xdf, 0xe9,         // fucomip st0, st1

      // unordered comparisons
      0xdd, 0xe0,     // fucom st0
      0xdd, 0xe1,     // fucom st1 (aka, 'fucom')
      0xdd, 0xe2,     // fucom st2
      0xdd, 0xe8,     // fucomp
      0xdd, 0xe9,     // fucompp
      0xdd, 0xea,     // fucomp st2
  }};
  // clang-format on

  std::vector<di::Instruction> decodedInsns;
  decodedInsns.reserve(num_tests);

  di::InstructionDecoder d(buffer.data(), buffer.size(), Dyninst::Arch_x86);
  for(int idx = 0; idx < num_tests; idx++) {
    di::Instruction insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode x86 test " << (idx + 1) << '\n';
      return EXIT_FAILURE;
    }
    decodedInsns.push_back(insn);
  }

  auto st0 = Dyninst::x86::st0;
  auto st1 = Dyninst::x86::st1;
  auto st2 = Dyninst::x86::st2;
  auto ebp = Dyninst::x86::ebp;

  using reg_set = Dyninst::register_set;

  std::vector<reg_set> expectedRead, expectedWritten;

  // fcom dword ptr [ebp]
  expectedRead.push_back(reg_set{st0, ebp});
  expectedWritten.push_back(reg_set{st0});

  // fcom qword ptr [ebp]
  expectedRead.push_back(reg_set{st0, ebp});
  expectedWritten.push_back(reg_set{st0});

  // fcom st0
  expectedRead.push_back(reg_set{st0});
  expectedWritten.push_back(reg_set{st0});

  // fcom st1
  expectedRead.push_back(reg_set{st0, st1});
  expectedWritten.push_back(reg_set{st0});

  // fcom st2
  expectedRead.push_back(reg_set{st0, st2});
  expectedWritten.push_back(reg_set{st0});

  // fcomp dword ptr [ebp]
  expectedRead.push_back(reg_set{st0, ebp});
  expectedWritten.push_back(reg_set{st0});

  // fcomp qword ptr [ebp]
  expectedRead.push_back(reg_set{st0, ebp});
  expectedWritten.push_back(reg_set{st0});

  // fcomp st0
  expectedRead.push_back(reg_set{st0});
  expectedWritten.push_back(reg_set{st0});

  // fcomp st1
  expectedRead.push_back(reg_set{st0, st1});
  expectedWritten.push_back(reg_set{st0});

  // fcomp st2
  expectedRead.push_back(reg_set{st0, st2});
  expectedWritten.push_back(reg_set{st0});

  // fcompp
  expectedRead.push_back(reg_set{st0, st1});
  expectedWritten.push_back(reg_set{st0});

  // fcomi st0, st0
  expectedRead.push_back(reg_set{st0});
  expectedWritten.push_back(reg_set{st0});

  // fcomip st0, st0
  expectedRead.push_back(reg_set{st0});
  expectedWritten.push_back(reg_set{st0});

  // fcomip st0, st1
  expectedRead.push_back(reg_set{st0, st1});
  expectedWritten.push_back(reg_set{st0});

  // fucomi st0, st0
  expectedRead.push_back(reg_set{st0});
  expectedWritten.push_back(reg_set{st0});

  // fucomi st0, st1
  expectedRead.push_back(reg_set{st0, st1});
  expectedWritten.push_back(reg_set{st0});

  // fucomip st0, st0
  expectedRead.push_back(reg_set{st0});
  expectedWritten.push_back(reg_set{st0});

  // fucomip st0, st1
  expectedRead.push_back(reg_set{st0, st1});
  expectedWritten.push_back(reg_set{st0});

  // fucom st0
  expectedRead.push_back(reg_set{st0});
  expectedWritten.push_back(reg_set{});

  // fucom st1 (aka, 'fucom')
  expectedRead.push_back(reg_set{st0, st1});
  expectedWritten.push_back(reg_set{});

  // fucom st2
  expectedRead.push_back(reg_set{st0, st2});
  expectedWritten.push_back(reg_set{});

  // fucomp
  expectedRead.push_back(reg_set{st0});
  expectedWritten.push_back(reg_set{st0});

  // fucompp
  expectedRead.push_back(reg_set{st0, st1});
  expectedWritten.push_back(reg_set{st0});

  // fucomp st(3)
  expectedRead.push_back(reg_set{st0, st2});
  expectedWritten.push_back(reg_set{st0});

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
