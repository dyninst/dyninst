#include "register_tests.h"
#include "Architecture.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"

#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  Intel 64 and IA-32 Architectures Software Developer’s Manual (SDM)
 *  June 2025
 *  5.1.2 Binary Arithmetic Instructions
 *
 *  These were imported from the 'test_instruction_read_write' test
 *  in the test suite. It's unclear why these specific instructions
 *  were originally chosen.
 */

namespace di = Dyninst::InstructionAPI;

struct arith_test {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
};

static std::vector<arith_test> make_tests(Dyninst::Architecture arch);
static bool run(Dyninst::Architecture, std::vector<arith_test> const &);

int main() {
  bool ok = run(Dyninst::Arch_x86, make_tests(Dyninst::Arch_x86));

  if(!run(Dyninst::Arch_x86_64, make_tests(Dyninst::Arch_x86_64))) {
    ok = false;
  }

  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<arith_test> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'sysctl' in " << sarch << " mode\n";
  for(auto const &t : tests) {
    test_id++;
    di::InstructionDecoder d(t.opcode.data(), t.opcode.size(), arch);
    auto insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode " << sarch << " test " << test_id << '\n';
      failed = true;
      continue;
    }

    std::clog << "Verifying '" << insn.format() << "'\n";

    if(!di::verify(insn, t.regs)) {
      failed = true;
    }
  }
  return !failed;
}

std::vector<arith_test> make_tests(Dyninst::Architecture arch) {
  const auto is_64 = (arch == Dyninst::Arch_x86_64);

  auto al = is_64 ? Dyninst::x86_64::al : Dyninst::x86::al;

  auto flags = is_64 ? Dyninst::x86_64::flags : Dyninst::x86::flags;
  auto af = is_64 ? Dyninst::x86_64::af : Dyninst::x86::af;
  auto cf = is_64 ? Dyninst::x86_64::cf : Dyninst::x86::cf;
  auto of = is_64 ? Dyninst::x86_64::of : Dyninst::x86::of;
  auto pf = is_64 ? Dyninst::x86_64::pf : Dyninst::x86::pf;
  auto sf = is_64 ? Dyninst::x86_64::sf : Dyninst::x86::sf;
  auto zf = is_64 ? Dyninst::x86_64::zf : Dyninst::x86::zf;

  auto ax32 = is_64 ? Dyninst::x86_64::eax : Dyninst::x86::eax;

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // add eax, 0xdeadbeef
      {0x05, 0xef, 0xbe, 0xad, 0xde},
      di::register_rw_test {
        reg_set{ax32},
        reg_set{ax32, af, zf, of, pf, sf, cf, flags}
      },
    },
    { // clc
      {0xf8},
      di::register_rw_test {
        reg_set{},
        reg_set{cf, flags}
      },
    },
    { // add al, 0x30
      {0x04, 0x30},
      di::register_rw_test {
        reg_set{al},
        reg_set{al, zf, cf, sf, of, pf, af, flags}
      },
    },
  };
  // clang-format on
}
