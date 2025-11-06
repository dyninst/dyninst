#include "Architecture.h"
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
 *  5.1.1 Data Transfer Instructions
 *
 *  These tests only cover stack-manipulation instructions.
 */

namespace di = Dyninst::InstructionAPI;

struct call_test {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<call_test> make_tests(Dyninst::Architecture arch);
static std::vector<call_test> make_tests32();
static bool run(Dyninst::Architecture, std::vector<call_test> const &);

int main() {
  bool ok = run(Dyninst::Arch_x86, make_tests(Dyninst::Arch_x86));

  if(!run(Dyninst::Arch_x86_64, make_tests(Dyninst::Arch_x86_64))) {
    ok = false;
  }

  if(!run(Dyninst::Arch_x86, make_tests32())) {
    ok = false;
  }

  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<call_test> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'stack' in " << sarch << " mode\n";
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
  }
  return !failed;
}

std::vector<call_test> make_tests(Dyninst::Architecture arch) {
  const auto is_64 = (arch == Dyninst::Arch_x86_64);

  auto eax = is_64 ? Dyninst::x86_64::rax : Dyninst::x86::eax;

  auto flags = is_64 ? Dyninst::x86_64::flags : Dyninst::x86::flags;
  auto af = is_64 ? Dyninst::x86_64::af : Dyninst::x86::af;
  auto cf = is_64 ? Dyninst::x86_64::cf : Dyninst::x86::cf;
  auto df = is_64 ? Dyninst::x86_64::df : Dyninst::x86::df;
  auto if_ = is_64 ? Dyninst::x86_64::if_ : Dyninst::x86::if_;
  auto nt = is_64 ? Dyninst::x86_64::nt_ : Dyninst::x86::nt_;
  auto of = is_64 ? Dyninst::x86_64::of : Dyninst::x86::of;
  auto pf = is_64 ? Dyninst::x86_64::pf : Dyninst::x86::pf;
  auto rf = is_64 ? Dyninst::x86_64::rf : Dyninst::x86::rf;
  auto sf = is_64 ? Dyninst::x86_64::sf : Dyninst::x86::sf;
  auto tf = is_64 ? Dyninst::x86_64::tf : Dyninst::x86::tf;
  auto zf = is_64 ? Dyninst::x86_64::zf : Dyninst::x86::zf;

  auto sp = Dyninst::MachRegister::getStackPointer(arch);

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    { // push eax
      {0x50},
      di::register_rw_test{
        reg_set {sp, eax},
        reg_set {sp}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{sp}
        }
      }
    },
    { // pop eax
      {0x58},
      di::register_rw_test{
        reg_set {sp},
        reg_set {sp, eax}
      },
      di::mem_test {
        reads_memory, !writes_memory,
        di::register_rw_test {
          reg_set{sp},
          reg_set{}
        }
      }
    },
    { // pushf
      {0x9c},
      di::register_rw_test{
        reg_set {sp, flags},
        reg_set {sp}
      },
      di::mem_test {
        !reads_memory, writes_memory,
        di::register_rw_test {
          reg_set{},
          reg_set{sp}
        }
      }
    },
    { // popf
      {0x9d},
      di::register_rw_test{
        reg_set {sp},
        reg_set {sp, flags, af, cf, df, if_, nt, of, pf, rf, sf, tf, zf}
      },
      di::mem_test {
        reads_memory, !writes_memory,
        di::register_rw_test {
          reg_set{sp},
          reg_set{}
        }
      }
    },
  };
  // clang-format on
}

std::vector<call_test> make_tests32() {
  auto eax = Dyninst::x86::eax;
  auto ecx = Dyninst::x86::ecx;
  auto edx = Dyninst::x86::edx;
  auto ebx = Dyninst::x86::ebx;
  auto esp = Dyninst::x86::esp;
  auto ebp = Dyninst::x86::ebp;
  auto esi = Dyninst::x86::esi;
  auto edi = Dyninst::x86::edi;

  auto sp = Dyninst::MachRegister::getStackPointer(Dyninst::Arch_x86);

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    { // pusha
      {0x60},
      di::register_rw_test{
        reg_set {sp, eax, eax, ecx, edx, ebx, esp, ebp, esi, edi},
        reg_set {sp}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{sp}
        }
      }
    },
    { // popa
      {0x61},
      di::register_rw_test{
        reg_set {sp},
        reg_set {sp, eax, eax, ecx, edx, ebx, esp, ebp, esi, edi}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{sp},
          reg_set{}
        }
      }
    },
  };
}
