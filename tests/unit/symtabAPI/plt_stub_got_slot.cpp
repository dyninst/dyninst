// Checks plt_stub_got_slot, which finds the GOT slot a PLT stub reads its
// jump target from, on instruction bytes rather than whole binaries.
//
// The bytes and the expected slots come from GNU as and ld, with each case
// linked at the address given here, and objdump agrees with every slot.
#include "symtabAPI/src/plt_stubs.h"

#include "Architecture.h"
#include "dyntypes.h"

#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {

struct Case {
  char const *name;
  Dyninst::Architecture arch;
  Dyninst::Address addr;
  std::vector<unsigned char> code;
  Dyninst::Address slot;
};

}

int main()
{
  using Dyninst::ADDRESS_INVALID;
  using Dyninst::Arch_aarch64;

  // clang-format off
  std::vector<Case> const cases{
    // adrp gives the page of the pc plus its immediate.  In the first three
    // cases, pc + immediate without the rounding to a page gives another slot.
    {"aarch64, stub mid-page, GOT above", Arch_aarch64, 0x2a0,
     {0x10, 0x01, 0x00, 0x90,     // adrp x16, 0x20000
      0x11, 0x06, 0x40, 0xf9},    // ldr  x17, [x16, #8]
     0x20008},
    {"aarch64, stub in a page's last words", Arch_aarch64, 0x400ff8,
     {0x10, 0x01, 0x00, 0x90,     // adrp x16, 0x420000
      0x11, 0x0a, 0x40, 0xf9},    // ldr  x17, [x16, #16]
     0x420010},
    {"aarch64, GOT below the stub", Arch_aarch64, 0x50010,
     {0x10, 0xfe, 0xff, 0x90,     // adrp x16, 0x10000
      0x11, 0xfa, 0x47, 0xf9},    // ldr  x17, [x16, #4080]
     0x10ff0},

    {"aarch64, ldr through another register", Arch_aarch64, 0x2a0000,
     {0x10, 0xec, 0xff, 0x90,     // adrp x16, 0x20000
      0x31, 0x06, 0x40, 0xf9},    // ldr  x17, [x17, #8]
     ADDRESS_INVALID},
    {"aarch64, adrp then add", Arch_aarch64, 0x2b0000,
     {0x90, 0xeb, 0xff, 0x90,     // adrp x16, 0x20000
      0x10, 0x22, 0x00, 0x91},    // add  x16, x16, #0x8
     ADDRESS_INVALID},
    {"aarch64, bti c first", Arch_aarch64, 0x2c0000,
     {0x5f, 0x24, 0x03, 0xd5,     // bti  c
      0x10, 0xeb, 0xff, 0x90},    // adrp x16, 0x20000
     ADDRESS_INVALID},
    {"aarch64, adrp alone", Arch_aarch64, 0x2a0,
     {0x10, 0x01, 0x00, 0x90},    // adrp x16, 0x20000
     ADDRESS_INVALID},

    {"ppc64, not handled", Dyninst::Arch_ppc64, 0x2a0,
     {0x10, 0x01, 0x00, 0x90,
      0x11, 0x06, 0x40, 0xf9},
     ADDRESS_INVALID},
  };
  // clang-format on

  int failures{0};
  for (auto const &c : cases)  {
    Dyninst::Address got{Dyninst::SymtabAPI::plt_stub_got_slot(c.code.data(), c.code.size(),
                                                               c.addr, c.arch)};
    if (got != c.slot)  {
      std::printf("FAIL %s: slot %#lx, expected %#lx\n", c.name, got, c.slot);
      failures++;
    }
  }
  std::printf("%d of %zu cases failed\n", failures, cases.size());
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
