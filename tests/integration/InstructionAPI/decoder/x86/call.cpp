#include "Architecture.h"
#include "cft_tests.h"
#include "InstructionDecoder.h"
#include "memory_tests.h"
#include "Register.h"
#include "register_tests.h"
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
 *  5.1.7 Control Transfer Instructions
 *
 *  These tests only cover function call/return instructions.
 */

namespace di = Dyninst::InstructionAPI;

struct call_test {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
  di::mem_test mem;
  di::cft_test cft;
};

constexpr bool reads_memory = true;
constexpr bool writes_memory = true;

constexpr bool hasCFT = true;
constexpr bool isCall = true;
constexpr bool isConditional = true;
constexpr bool isIndirect = true;
constexpr bool isFallthrough = true;
constexpr bool isBranch = true;
constexpr bool isReturn = true;

static std::vector<call_test> make_tests(Dyninst::Architecture arch);
static std::vector<call_test> make_tests32();
static std::vector<call_test> make_tests64();
static bool run(Dyninst::Architecture, std::vector<call_test> const &);

int main() {
  bool ok = run(Dyninst::Arch_x86, make_tests(Dyninst::Arch_x86));

  if(!run(Dyninst::Arch_x86_64, make_tests(Dyninst::Arch_x86_64))) {
    ok = false;
  }
  if(!run(Dyninst::Arch_x86, make_tests32())) {
    ok = false;
  }
  if(!run(Dyninst::Arch_x86_64, make_tests64())) {
    ok = false;
  }
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<call_test> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'call' in " << sarch << " mode\n";
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
    if(!di::verify(insn, t.mem)) {
      failed = true;
    }
    if(!di::verify(insn, t.cft)) {
      failed = true;
    }

    std::clog << "\n";
  }
  return !failed;
}

std::vector<call_test> make_tests(Dyninst::Architecture arch) {
  const auto is_64 = (arch == Dyninst::Arch_x86_64);

  auto eax = is_64 ? Dyninst::x86_64::rax : Dyninst::x86::eax;
  auto ecx = is_64 ? Dyninst::x86_64::rcx : Dyninst::x86::ecx;

  auto sp = Dyninst::MachRegister::getStackPointer(arch);
  auto ip = Dyninst::MachRegister::getPC(arch);

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // call [8*EAX + ECX + 0xDEADBEEF]
      {0xff, 0x94, 0xc1, 0xef, 0xbe, 0xad, 0xde},
      di::register_rw_test{
        reg_set{eax, ecx, ip, sp},
        reg_set{sp, ip}
      },
      di::mem_test{
        reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{eax, ecx},
          reg_set{sp}
        }
      },
      di::cft_test{
        hasCFT,
        {isCall, !isConditional, isIndirect, !isFallthrough, !isBranch, !isReturn}
      }
    },
    { // call 0x12345678
      {0xe8, 0x73, 0x56, 0x34, 0x12, },
      di::register_rw_test{
        reg_set{ip, sp},
        reg_set{sp, ip}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{sp}
        }
      },
      di::cft_test{
        hasCFT,
        {isCall, !isConditional, !isIndirect, !isFallthrough, !isBranch, !isReturn}
      }
    },
    { // call eax
      {0xff, 0xd0, },
      di::register_rw_test{
        reg_set{ip, sp, eax},
        reg_set{sp, ip}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{sp}
        }
      },
      di::cft_test{
        hasCFT,
        {isCall, !isConditional, isIndirect, !isFallthrough, !isBranch, !isReturn}
      }
    },
  };
  // clang-format on
}

std::vector<call_test> make_tests32() {
  auto sp = Dyninst::MachRegister::getStackPointer(Dyninst::Arch_x86);
  auto ip = Dyninst::MachRegister::getPC(Dyninst::Arch_x86);

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // call far 0x9abc:0x12345678
      {0x9a, 0x78, 0x56, 0x34, 0x12, 0xbc, 0x9a, },
      di::register_rw_test{
        reg_set{sp, ip},
        reg_set{sp, ip}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{sp}
        }
      },
      di::cft_test{
        hasCFT,
        {isCall, !isConditional, !isIndirect, !isFallthrough, !isBranch, !isReturn}
      }
    },
  };
  // clang-format on
}

std::vector<call_test> make_tests64() {
  auto sp = Dyninst::MachRegister::getStackPointer(Dyninst::Arch_x86_64);
  auto ip = Dyninst::MachRegister::getPC(Dyninst::Arch_x86_64);

  auto rax = Dyninst::x86_64::rax;
  auto rcx = Dyninst::x86_64::rcx;

  using reg_set = Dyninst::register_set;

  // clang-format off
  return {
    { // call qword ptr [rcx+rax*8-0x12345678]
      {0xff, 0x94, 0xc1, 0x78, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{sp, ip, rcx, rax},
        reg_set{sp, ip}
      },
      di::mem_test{
        reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{rcx, rax},
          reg_set{sp}
        }
      },
      di::cft_test{
        hasCFT,
        {isCall, !isConditional, isIndirect, !isFallthrough, !isBranch, !isReturn}
      }
    },
  };
  // clang-format on
}
