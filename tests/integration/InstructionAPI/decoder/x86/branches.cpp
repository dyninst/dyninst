#include "cft_tests.h"
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
 *  5.1.7 Control Transfer Instructions
 *
 *  These tests only cover branching and looping instructions.
 */

namespace di = Dyninst::InstructionAPI;

struct branch_test {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
  di::mem_test mem;
  di::cft_test cft;
};

static std::vector<branch_test> make_tests(Dyninst::Architecture arch);
static std::vector<branch_test> make_tests32();
static std::vector<branch_test> make_tests64();
static bool run(Dyninst::Architecture, std::vector<branch_test> const &);

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

bool run(Dyninst::Architecture arch, std::vector<branch_test> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'branches' in " << sarch << " mode\n";
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
  }
  return !failed;
}

std::vector<branch_test> make_tests(Dyninst::Architecture arch) {
  const auto is_64 = (arch == Dyninst::Arch_x86_64);

  auto flags = is_64 ? Dyninst::x86_64::flags : Dyninst::x86::flags;
  auto cf = is_64 ? Dyninst::x86_64::cf : Dyninst::x86::cf;
  auto of = is_64 ? Dyninst::x86_64::of : Dyninst::x86::of;
  auto pf = is_64 ? Dyninst::x86_64::pf : Dyninst::x86::pf;
  auto sf = is_64 ? Dyninst::x86_64::sf : Dyninst::x86::sf;
  auto zf = is_64 ? Dyninst::x86_64::zf : Dyninst::x86::zf;

  auto ecx = is_64 ? Dyninst::x86_64::rcx : Dyninst::x86::ecx;
  auto eax = is_64 ? Dyninst::x86_64::rax : Dyninst::x86::eax;

  auto ip = Dyninst::MachRegister::getPC(arch);

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  using reg_set = Dyninst::register_set;

  using di::has_cft;
  using di::is_branch;
  using di::is_call;
  using di::is_conditional;
  using di::is_fallthrough;
  using di::is_indirect;
  using di::is_return;

  // clang-format off
  return {
    { // jmp 0x12
      {0xeb, 0x10},
      di::register_rw_test{
        reg_set{ip},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, !is_conditional, !is_indirect, !is_fallthrough, is_branch, !is_return}
      }
    },
    { // jmp 0x12345678
      {0xe9, 0x78, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{ip},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, !is_conditional, !is_indirect, !is_fallthrough, is_branch, !is_return}
      }
    },
    { // jmp dword ptr [eax+0x12345678]
      {0xff, 0xa0, 0x78, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{eax},
        reg_set{ip}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{eax},
          reg_set{}
        }
      },
      di::cft_test{
        has_cft,
        {!is_call, !is_conditional, is_indirect, !is_fallthrough, is_branch, !is_return}
      }
    },
    { // jmp far [eax + 0x1234]
      {0xff, 0xa8, 0x78, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{eax},
        reg_set{ip}
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{eax},
          reg_set{}
        }
      },
      di::cft_test{
        has_cft,
        {!is_call, !is_conditional, is_indirect, !is_fallthrough, is_branch, !is_return}
      }
    },
    { // loop 0x12
      {0xe2, 0x10},
      di::register_rw_test{
        reg_set{ip, ecx},
        reg_set{ip, ecx}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // loope 0x12
      {0xe1, 0x10},
      di::register_rw_test{
        reg_set{ip, ecx, flags, zf},
        reg_set{ip, ecx}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // loopne 0x12
      {0xe0, 0x10},
      di::register_rw_test{
        reg_set{ip, ecx, flags, zf},
        reg_set{ip, ecx}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jo 0x7f
      {0x70, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, of},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jno 0x7f
      {0x71, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, of},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jb 0x7f   (aka jc/jnae)
      {0x72, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, cf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jae 0x7f   (aka jnb/jnc)
      {0x73, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, cf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // je 0x7f   (aka jz)
      {0x74, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, zf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jne 0x7f   (aka jnz)
      {0x75, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, zf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jbe 0x7f   (aka jna)
      {0x76, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, cf, zf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // ja 0x7f   (aka jnbe)
      {0x77, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, cf, zf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // js 0x7f
      {0x78, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, sf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jns 0x7f
      {0x79, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, sf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jp 0x7f   (aka jpe)
      {0x7a, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, pf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jnp 0x7f   (aka jpo)
      {0x7b, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, pf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jl 0x7f   (aka jnge)
      {0x7c, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, of, sf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jge 0x7f   (aka jnl)
      {0x7d, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, of, sf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jle 0x7f   (aka jng)
      {0x7e, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, of, sf, zf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jg 0x7f   (aka jnle)
      {0x7f, 0x7d},
      di::register_rw_test{
        reg_set{flags, ip, of, sf, zf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jrcxz 0x7f   (aka jcxz/jecxz)
      {0xe3, 0x7d},
      di::register_rw_test{
        reg_set{ip, ecx, },
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jo 0x12345678
      {0x0f, 0x80, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, of},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jno 0x12345678
      {0x0f, 0x81, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, of},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jb 0x12345678   (aka jc/jnae)
      {0x0f, 0x82, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, cf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jae 0x12345678   (aka jnb/jnc)
      {0x0f, 0x83, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, cf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // je 0x12345678   (aka jz)
      {0x0f, 0x84, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, zf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jne 0x12345678   (aka jnz)
      {0x0f, 0x85, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, zf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jbe 0x12345678   (aka jna)
      {0x0f, 0x86, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, cf, zf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // ja 0x12345678   (aka jnbe)
      {0x0f, 0x87, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, cf, zf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // js 0x12345678
      {0x0f, 0x88, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, sf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jns 0x12345678
      {0x0f, 0x89, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, sf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jp 0x12345678   (aka jpe)
      {0x0f, 0x8A, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, pf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jnp 0x12345678   (aka jpo)
      {0x0f, 0x8B, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, pf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jl 0x12345678   (aka jnge)
      {0x0f, 0x8C, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, of, sf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jge 0x12345678   (aka jnl)
      {0x0f, 0x8D, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, of, sf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jle 0x12345678   (aka jng)
      {0x0f, 0x8E, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, of, sf, zf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    },
    { // jg 0x12345678   (aka jnle)
      {0x0f, 0x8F, 0x72, 0x56, 0x34, 0x12},
      di::register_rw_test{
        reg_set{flags, ip, of, sf, zf},
        reg_set{ip}
      },
      di::mem_test{},
      di::cft_test{
        has_cft,
        {!is_call, is_conditional, !is_indirect, is_fallthrough, is_branch, !is_return}
      }
    }
  };
  // clang-format on
}

std::vector<branch_test> make_tests32() {

  auto ip = Dyninst::MachRegister::getPC(Dyninst::Arch_x86);
  auto eax = Dyninst::x86::eax;

  using reg_set = Dyninst::register_set;

  using di::has_cft;
  using di::is_branch;
  using di::is_call;
  using di::is_conditional;
  using di::is_fallthrough;
  using di::is_indirect;
  using di::is_return;

  return {
      {// jmp 0x12345678
       {0xe9, 0x73, 0x56, 0x34, 0x12},
       di::register_rw_test{reg_set{ip}, reg_set{ip}},
       di::mem_test{},
       di::cft_test{has_cft, {!is_call, !is_conditional, !is_indirect, !is_fallthrough, is_branch, !is_return}}},
      {// jmp eax
       {0xff, 0xe0},
       di::register_rw_test{reg_set{eax}, reg_set{ip}},
       di::mem_test{},
       di::cft_test{has_cft, {!is_call, !is_conditional, is_indirect, !is_fallthrough, is_branch, !is_return}}},
      {// ljmp 0x5678:0x12345678
       {0xea, 0x78, 0x56, 0x34, 0x12, 0x78, 0x56},
       di::register_rw_test{reg_set{}, reg_set{ip}},
       di::mem_test{},
       di::cft_test{has_cft, {!is_call, !is_conditional, !is_indirect, !is_fallthrough, is_branch, !is_return}}},
  };
}

std::vector<branch_test> make_tests64() {

  auto ip = Dyninst::MachRegister::getPC(Dyninst::Arch_x86_64);
  auto rax = Dyninst::x86_64::rax;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  using di::has_cft;
  using di::is_branch;
  using di::is_call;
  using di::is_conditional;
  using di::is_fallthrough;
  using di::is_indirect;
  using di::is_return;

  return {
      {// REX.W jmp far [rax + 0x1234]
       {0x48, 0xff, 0xa8, 0x78, 0x56, 0x34, 0x12},
       di::register_rw_test{reg_set{rax}, reg_set{ip}},
       di::mem_test{reads_memory, !writes_memory, di::register_rw_test{reg_set{rax}, reg_set{}}},
       di::cft_test{has_cft, {!is_call, !is_conditional, is_indirect, !is_fallthrough, is_branch, !is_return}}},
  };
}
