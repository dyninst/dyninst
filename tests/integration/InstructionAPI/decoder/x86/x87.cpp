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
 *  5.2 X87 FPU Instructions
 *
 *  These tests only cover a sample of the instructions.
 */

namespace di = Dyninst::InstructionAPI;

struct x87_test {
  std::vector<unsigned char> opcode;
  di::register_rw_test regs;
  di::mem_test mem;
};

static std::vector<x87_test> make_tests(Dyninst::Architecture);
static bool run(Dyninst::Architecture, std::vector<x87_test> const &);

int main() {
  bool ok = run(Dyninst::Arch_x86, make_tests(Dyninst::Arch_x86));

  if(!run(Dyninst::Arch_x86_64, make_tests(Dyninst::Arch_x86_64))) {
    ok = false;
  }

  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<x87_test> const &tests) {
  bool failed = false;
  int test_id = 0;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for x87 in " << sarch << " mode\n";
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

std::vector<x87_test> make_tests(Dyninst::Architecture arch) {
  bool const is_64 = (arch == Dyninst::Arch_x86_64);

  auto ebx = is_64 ? Dyninst::x86_64::rbx : Dyninst::x86::ebx;
  auto ebp = is_64 ? Dyninst::x86_64::rbp : Dyninst::x86::ebp;
  auto ss = is_64 ? Dyninst::x86_64::ss : Dyninst::x86::ss;
  auto ds = is_64 ? Dyninst::x86_64::ds : Dyninst::x86::ds;

  auto st0 = is_64 ? Dyninst::x86_64::st0 : Dyninst::x86::st0;
  auto st1 = is_64 ? Dyninst::x86_64::st1 : Dyninst::x86::st1;

  auto eflags = is_64 ? Dyninst::x86_64::flags : Dyninst::x86::flags;
  auto cf = is_64 ? Dyninst::x86_64::cf : Dyninst::x86::cf;
  auto pf = is_64 ? Dyninst::x86_64::pf : Dyninst::x86::pf;
  auto af = is_64 ? Dyninst::x86_64::af : Dyninst::x86::af;
  auto zf = is_64 ? Dyninst::x86_64::zf : Dyninst::x86::zf;
  auto sf = is_64 ? Dyninst::x86_64::sf : Dyninst::x86::sf;
  auto of = is_64 ? Dyninst::x86_64::of : Dyninst::x86::of;

  auto fsw = is_64 ? Dyninst::x86_64::fsw : Dyninst::x86::fsw;
  auto cr0 = is_64 ? Dyninst::x86_64::cr0 : Dyninst::x86::cr0;
  auto cr1 = is_64 ? Dyninst::x86_64::cr1 : Dyninst::x86::cr1;
  auto cr2 = is_64 ? Dyninst::x86_64::cr2 : Dyninst::x86::cr2;
  auto cr3 = is_64 ? Dyninst::x86_64::cr3 : Dyninst::x86::cr3;

  using reg_set = Dyninst::register_set;

  constexpr bool reads_memory = true;
  constexpr bool writes_memory = true;

  // clang-format off
  return {
    { //  fcom qword ptr [ebp]
      {0xdc, 0x55, 0x00},
      di::register_rw_test{
        reg_set({st0, ebp, ss}),
        reg_set({fsw, cr0, cr1, cr2, cr3})
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{ebp},
          reg_set{}
        }
      }
    },
    { //  fcom st1
      {0xd8, 0xd1},
      di::register_rw_test{
        reg_set({st0, st1}),
        reg_set({fsw, cr0, cr1, cr2, cr3})
      },
      di::mem_test{}
    },
    { //  fcomp dword ptr [ebp]
      {0xd8, 0x5d, 0x00},
      di::register_rw_test{
        reg_set({st0, ebp, ss}),
        reg_set({fsw, cr0, cr1, cr2, cr3})
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set{ebp},
          reg_set{}
        }
      }
    },
    { //  fcomp st1
      {0xd8, 0xd9},
      di::register_rw_test{
        reg_set({st0, st1}),
        reg_set({fsw, cr0, cr1, cr2, cr3})
      },
      di::mem_test{}
    },
    { //  fcompp
      {0xde, 0xd9},
      di::register_rw_test{
        reg_set({st0, st1}),
        reg_set({fsw, cr0, cr1, cr2, cr3})
      },
      di::mem_test{}
    },
    { //  fcomi st0, st0
      {0xdb, 0xf0},
      di::register_rw_test{
        reg_set({st0}),
        reg_set({eflags, cf, pf, af, zf, sf, of, fsw, cr0, cr1, cr3})
      },
      di::mem_test{}
    },
    { //  fcomip st0, st1
      {0xdf, 0xf1},
      di::register_rw_test{
        reg_set({st0, st1}),
        reg_set({eflags, cf, pf, af, zf, sf, of, fsw, cr0, cr1, cr3})
      },
      di::mem_test{}
    },
    { //  fucomi st0, st1
      {0xdb, 0xe9},
      di::register_rw_test{
        reg_set({st0, st1}),
        reg_set({eflags, cf, pf, af, zf, sf, of, fsw, cr0, cr1, cr3})
      },
      di::mem_test{}
    },
    { //  fucomip st0, st1
      {0xdf, 0xe9},
      di::register_rw_test{
        reg_set({st0, st1}),
        reg_set({eflags, cf, pf, af, zf, sf, of, fsw, cr0, cr1, cr3})
      },
      di::mem_test{}
    },
    { //  fucom st1 (aka, 'fucom')
      {0xdd, 0xe1},
      di::register_rw_test{
        reg_set({st0, st1}),
        reg_set({fsw, cr0, cr1, cr2, cr3})
      },
      di::mem_test{}
    },
    { //  fucomp
      {0xdd, 0xe8},
      di::register_rw_test{
        reg_set({st0}),
        reg_set({fsw, cr0, cr1, cr2, cr3})
      },
      di::mem_test{}
    },
    { //  fucompp
      {0xdd, 0xe9},
      di::register_rw_test{
        reg_set({st0, st1}),
        reg_set({fsw, cr0, cr1, cr2, cr3})
      },
      di::mem_test{}
    },
    { //  ficom dword ptr [ebx]
      {0xda, 0x13},
      di::register_rw_test{
        reg_set({st0, ds, ebx}),
        reg_set({fsw, cr0, cr1, cr2, cr3})
      },
      di::mem_test{
        reads_memory, !writes_memory,
        di::register_rw_test{
          reg_set({ebx}),
          reg_set{}
        }
      }
    },
    { //  ftst
      {0xd9, 0xe4},
      di::register_rw_test{
        reg_set({st0}),
        reg_set({fsw, cr0, cr1, cr2, cr3})
      },
      di::mem_test{}
    },
    { //  fxam
      {0xd9, 0xe5},
      di::register_rw_test{
        reg_set({st0}),
        reg_set({fsw, cr0, cr1, cr2, cr3, st0})
      },
      di::mem_test{}
    },
  };
  // clang-format on
}
