#include "memory_tests.h"
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
 *  5.1.1 Data Transfer Instructions
 *
 *  These were imported from the 'test_instruction_read_write' test
 *  in the test suite. It's unclear why these specific instructions
 *  were originally chosen.
 */

namespace di = Dyninst::InstructionAPI;

struct data_transfer_test {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
  di::mem_test mem;
  bool is_conditional;
};

namespace {
  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;
  constexpr bool is_conditional = true;
}

static std::vector<data_transfer_test> make_tests(Dyninst::Architecture arch);
static std::vector<data_transfer_test> make_tests64();
static bool run(Dyninst::Architecture, std::vector<data_transfer_test> const &);

int main() {
  bool ok = run(Dyninst::Arch_x86, make_tests(Dyninst::Arch_x86));

  if(!run(Dyninst::Arch_x86_64, make_tests(Dyninst::Arch_x86_64))) {
    ok = false;
  }
  if(!run(Dyninst::Arch_x86_64, make_tests64())) {
    ok = false;
  }

  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<data_transfer_test> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'data transfer' in " << sarch << " mode\n";
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
    if(insn.isConditional() != t.is_conditional) {
      std::cerr << "isConditional: expected '" << std::boolalpha << t.is_conditional << "', got '"
                << insn.isConditional() << "'\n";
      failed = true;
    }
  }
  return !failed;
}

std::vector<data_transfer_test> make_tests(Dyninst::Architecture arch) {
  const auto is_64 = (arch == Dyninst::Arch_x86_64);

  auto flags = is_64 ? Dyninst::x86_64::flags : Dyninst::x86::flags;
  auto af = is_64 ? Dyninst::x86_64::af : Dyninst::x86::af;
  auto cf = is_64 ? Dyninst::x86_64::cf : Dyninst::x86::cf;
  auto of = is_64 ? Dyninst::x86_64::of : Dyninst::x86::of;
  auto pf = is_64 ? Dyninst::x86_64::pf : Dyninst::x86::pf;
  auto sf = is_64 ? Dyninst::x86_64::sf : Dyninst::x86::sf;
  auto zf = is_64 ? Dyninst::x86_64::zf : Dyninst::x86::zf;

  auto ax16 = is_64 ? Dyninst::x86_64::ax : Dyninst::x86::ax;
  auto ax32 = is_64 ? Dyninst::x86_64::eax : Dyninst::x86::eax;
  auto ebx = is_64 ? Dyninst::x86_64::rbx : Dyninst::x86::ebx;
  auto ebp = is_64 ? Dyninst::x86_64::rbp : Dyninst::x86::ebp;
  auto si32 = is_64 ? Dyninst::x86_64::esi : Dyninst::x86::esi;
  auto bp32 = is_64 ? Dyninst::x86_64::ebp : Dyninst::x86::ebp;
  auto dl = is_64 ? Dyninst::x86_64::dl : Dyninst::x86::dl;
  auto gs = is_64 ? Dyninst::x86_64::gs : Dyninst::x86::gs;

  // clang-format off
  return {
    { // cmpxchg dword ptr [ebp + 0x12345678], esi
      {0x0f, 0xb1, 0xb5, 0x78, 0x56, 0x34, 0x12},
      di::register_rw_test {
        reg_set{ax32, ebp, si32},
        reg_set{ax32, flags, af, cf, of, pf, sf, zf}
      },
      di::mem_test{
        reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{ebp},
          reg_set{ebp}
        }
      },
      !is_conditional
    },
    { // cmova rax, rbp
      {0x0f, 0x47, 0xc5},
      di::register_rw_test {
        reg_set{bp32, flags, cf, zf},
        reg_set{ax32}
      },
      di::mem_test{},
      is_conditional
    },
    { // mov dword ptr [ebp - 4], 1
      {0xc7, 0x45, 0xfc, 0x01, 0x00, 0x00, 0x00},
      di::register_rw_test {
        reg_set{ebp},
        reg_set{}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{ebp}
        }
      },
      !is_conditional
    },
    { // mov byte ptr [ebp - 0x34], dl
      {0x88, 0x55, 0xcc},
      di::register_rw_test {
        reg_set{ebp, dl},
        reg_set{}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{ebp}
        }
      },
      !is_conditional
    },
    { // mov ax, gs
      {0x66, 0x8c, 0xe8},
      di::register_rw_test {
        reg_set{gs},
        reg_set{ax16}
      },
      di::mem_test{},
      !is_conditional
    },
    { // mov eax, ebp
      {0x89, 0xe8},
      di::register_rw_test {
        reg_set{bp32},
        reg_set{ax32}
      },
      di::mem_test{},
      !is_conditional
    },
    { // lea eax, [ebx - 0xe8]
      {0x8d, 0x83, 0x18, 0xff, 0xff, 0xff},
      di::register_rw_test {
        reg_set{ebx},
        reg_set{ax32}
      },
      di::mem_test{},
      !is_conditional
    },
  };
  // clang-format on
}

std::vector<data_transfer_test> make_tests64() {
  auto rbp = Dyninst::x86_64::rbp;
  auto r8d = Dyninst::x86_64::r8d;
  auto rax = Dyninst::x86_64::rax;

  auto rflags = Dyninst::x86_64::flags;
  auto af = Dyninst::x86_64::af;
  auto cf = Dyninst::x86_64::cf;
  auto of = Dyninst::x86_64::of;
  auto pf = Dyninst::x86_64::pf;
  auto sf = Dyninst::x86_64::sf;
  auto zf = Dyninst::x86_64::zf;

  // clang-format off
  return {
    { // mov dword ptr [rbp - 0x3c], r8d
      {0x44, 0x89, 0x45, 0xc4},
      di::register_rw_test {
        reg_set{rbp, r8d},
        reg_set{}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{rbp}
        }
      },
      !is_conditional
    },
    { // REX.W cmova rax, rbp
      {0x48, 0x0f, 0x47, 0xc5},
      di::register_rw_test {
        reg_set{rbp, rflags, cf, zf},
        reg_set{rax}
      },
      di::mem_test{},
      is_conditional
    },
    { // REX.W cmpxchg dword ptr [rbp + 0x12345678], rax
      {0x48, 0x0f, 0xb1, 0x85, 0x78, 0x56, 0x34, 0x12},
      di::register_rw_test {
        reg_set{rax, rbp},
        reg_set{rax, rflags, af, cf, of, pf, sf, zf}
      },
      di::mem_test{
        reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{rbp},
          reg_set{rbp}
        }
      },
      !is_conditional
    },
  };
  // clang-format on
}
