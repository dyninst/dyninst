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
 *  These tests only cover system-level control instructions.
 */

namespace di = Dyninst::InstructionAPI;

struct sysctl_test {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
  di::mem_test mem;
  bool is_software_interrupt;
  bool is_system_call;
};

static std::vector<sysctl_test> make_tests(Dyninst::Architecture arch);
std::vector<sysctl_test> make_tests32();
std::vector<sysctl_test> make_tests64();
static bool run(Dyninst::Architecture, std::vector<sysctl_test> const &);

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

bool run(Dyninst::Architecture arch, std::vector<sysctl_test> const &tests) {
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
    if(!di::verify(insn, t.mem)) {
      failed = true;
    }
    //    if(insn.isInterrupt() != t.is_software_interrupt) {
    //      std::cerr << std::boolalpha << "Expected isInterrupt = " << t.is_software_interrupt << ", got '"
    //                << insn.isInterrupt() << "'\n";
    //      failed = true;
    //    }
    //    bool const is_software_exception = (t.is_software_interrupt && !t.is_system_call);
    //    if(insn.isSoftwareException() != is_software_exception) {
    //      std::cerr << std::boolalpha << "Expected isSoftwareException = '" << is_software_exception << "', got '"
    //                << insn.isSoftwareException() << "'\n";
    //      failed = true;
    //    }
  }
  return !failed;
}

std::vector<sysctl_test> make_tests(Dyninst::Architecture arch) {
  const auto is_64 = (arch == Dyninst::Arch_x86_64);

  auto flags = is_64 ? Dyninst::x86_64::flags : Dyninst::x86::flags;
  auto flagc = is_64 ? Dyninst::x86_64::flagc : Dyninst::x86::flagc;
  auto flagd = is_64 ? Dyninst::x86_64::flagd : Dyninst::x86::flagd;
  auto ac = is_64 ? Dyninst::x86_64::ac : Dyninst::x86::ac;
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
  auto vif = is_64 ? Dyninst::x86_64::vif : Dyninst::x86::vif;
  auto vm = is_64 ? Dyninst::x86_64::vm : Dyninst::x86::vm;
  auto zf = is_64 ? Dyninst::x86_64::zf : Dyninst::x86::zf;

  auto sp = Dyninst::MachRegister::getStackPointer(arch);
  auto ip = Dyninst::MachRegister::getPC(arch);

  auto ss = is_64 ? Dyninst::x86_64::ss : Dyninst::x86::ss;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  constexpr bool is_software_interrupt = true;
  constexpr bool is_system_call = true;

  // clang-format off
  return {
    { // int3
      {0xcc},
      di::register_rw_test{
        reg_set{flags, sp, flagc, flagd, vm},
        reg_set{flags, ip, sp, tf, if_, nt, rf, vm, ac, vif}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{sp}
        }
      },
      is_software_interrupt, !is_system_call
    },
    { // int 80
      {0xcd, 0x80},
      di::register_rw_test{
        reg_set{flags, sp, flagc, flagd, vm},
        reg_set{flags, ip, sp, tf, if_, nt, rf, vm, ac, vif}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{sp}
        }
      },
      is_software_interrupt, is_system_call
    },
    { // int1
      {0xf1},
      di::register_rw_test{
        reg_set{sp},
        reg_set{ip, sp}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{sp}
        }
      },
      is_software_interrupt, !is_system_call
    },
    { // iret
      {0xcf},
      di::register_rw_test {
        reg_set{flags, sp, ss},
        reg_set{flags, ip, sp, af, cf, df, if_, nt, of, pf, rf, sf, tf, zf, }
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{sp, ss},
          reg_set{}
        }
      },
      is_software_interrupt, !is_system_call
    },
    { // hlt
      {0xf4},
      di::register_rw_test{},
      di::mem_test{},
      is_software_interrupt, !is_system_call
    },
    { // ud2
      {0x0f, 0xff},
      di::register_rw_test{},
      di::mem_test{},
      is_software_interrupt, !is_system_call
    },
  };
  // clang-format on
}

std::vector<sysctl_test> make_tests64() {
  auto sp = Dyninst::MachRegister::getStackPointer(Dyninst::Arch_x86_64);
  auto ip = Dyninst::MachRegister::getPC(Dyninst::Arch_x86_64);

  auto ss = Dyninst::x86_64::ss;

  auto rcx = Dyninst::x86_64::rcx;
  auto r11 = Dyninst::x86_64::r11;

  auto flags = Dyninst::x86_64::flags;
  auto af = Dyninst::x86_64::af;
  auto cf = Dyninst::x86_64::cf;
  auto df = Dyninst::x86_64::df;
  auto if_ = Dyninst::x86_64::if_;
  auto nt = Dyninst::x86_64::nt_;
  auto of = Dyninst::x86_64::of;
  auto pf = Dyninst::x86_64::pf;
  auto rf = Dyninst::x86_64::rf;
  auto sf = Dyninst::x86_64::sf;
  auto tf = Dyninst::x86_64::tf;
  auto zf = Dyninst::x86_64::zf;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  constexpr bool is_software_interrupt = true;
  constexpr bool is_system_call = true;

  // clang-format off
  return {
    { // iretq
      {0x40, 0xcf}, // REX.W
      di::register_rw_test {
        reg_set{flags, sp, ss},
        reg_set{flags, ip, sp, af, cf, df, if_, nt, of, pf, rf, sf, tf, zf }
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{sp, ss},
          reg_set{}
        }
      },
      is_software_interrupt, !is_system_call
    },
    { // iret
      {0xcf},
      di::register_rw_test {
        reg_set{flags, sp, ss},
        reg_set{flags, ip, sp, af, cf, df, if_, nt, of, pf, rf, sf, tf, zf }
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{sp, ss},
          reg_set{}
        }
      },
      is_software_interrupt, !is_system_call
    },
    { // syscall
      {0x0f, 0x05},
      di::register_rw_test {
        reg_set{},
        reg_set{rcx, ip, r11, flags, ip, af, cf, df, if_, nt, of, pf, rf, sf, tf, zf, }
      },
      di::mem_test{
        !reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{}
        }
      },
      is_software_interrupt, is_system_call
    },
    { // sysret
      {0x0f, 0x07},
      di::register_rw_test {
        reg_set{rcx, r11 },
        reg_set{ip, flags, ip, af, cf, df, if_, nt, of, pf, rf, sf, tf, zf, }
      },
      di::mem_test{
        !reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{}
        }
      },
      is_software_interrupt, is_system_call
    },
  };
  // clang-format on
}

std::vector<sysctl_test> make_tests32() {
  auto sp = Dyninst::MachRegister::getStackPointer(Dyninst::Arch_x86);
  auto ip = Dyninst::MachRegister::getPC(Dyninst::Arch_x86);

  auto flags = Dyninst::x86::flags;
  auto ac = Dyninst::x86::ac;
  auto flagc = Dyninst::x86::flagc;
  auto flagd = Dyninst::x86::flagd;
  auto if_ = Dyninst::x86::if_;
  auto nt = Dyninst::x86::nt_;
  auto of = Dyninst::x86::of;
  auto rf = Dyninst::x86::rf;
  auto tf = Dyninst::x86::tf;
  auto vm = Dyninst::x86::vm;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  constexpr bool is_software_interrupt = true;
  constexpr bool is_system_call = true;

  // clang-format off
  return {
    { // into
      {0xce},
      di::register_rw_test {
        reg_set{flags, sp, flagc, flagd, of, vm},
        reg_set{flags, ip, sp, tf, if_, nt, rf, vm, ac}
      },
      di::mem_test{
        !reads_memory, writes_memory,
        di::register_rw_test{
          reg_set{},
          reg_set{sp}
        }
      },
      is_software_interrupt, !is_system_call
    },
  };
  // clang-format on
}
