#include "memory_tests.h"
#include "opcode_tests.h"
#include "Architecture.h"
#include "InstructionDecoder.h"

#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  Intel 64 and IA-32 Architectures Software Developer's Manual (SDM)
 *  June 2025
 *  Prefetch instructions (PREFETCHh, PREFETCHW, PREFETCHWT1, and the
 *  3DNow! PREFETCH).
 *
 *  A prefetch is a hint: it moves data toward the cache but performs no
 *  architectural read or write of memory and cannot fault. Every prefetch
 *  must therefore be categorized as c_PrefetchInsn and must report neither
 *  a memory read nor a memory write.
 *
 *  Regression coverage for the categorization of prefetches that Capstone
 *  does not assign to any instruction group (e.g. prefetchw, prefetchwt1).
 */

namespace di = Dyninst::InstructionAPI;

struct prefetch_test {
  std::vector<unsigned char> bytes;
  di::opcode_test opcode;
};

static bool run(Dyninst::Architecture, std::vector<prefetch_test> const &);

static std::vector<prefetch_test> make_tests(Dyninst::Architecture arch) {
  // ModRM byte 0x00 selects [eax]/[rax] as the address.
  auto const addr = (arch == Dyninst::Arch_x86_64) ? " (%rax)" : " (%eax)";
  return {
    {{0x0f, 0x0d, 0x00}, di::opcode_test(e_prefetch,    "prefetch" + std::string{addr})},
    {{0x0f, 0x18, 0x00}, di::opcode_test(e_prefetchnta, "prefetchnta" + std::string{addr})},
    {{0x0f, 0x18, 0x08}, di::opcode_test(e_prefetcht0,  "prefetcht0" + std::string{addr})},
    {{0x0f, 0x18, 0x10}, di::opcode_test(e_prefetcht1,  "prefetcht1" + std::string{addr})},
    {{0x0f, 0x18, 0x18}, di::opcode_test(e_prefetcht2,  "prefetcht2" + std::string{addr})},
    {{0x0f, 0x0d, 0x08}, di::opcode_test(e_prefetchw,   "prefetchw" + std::string{addr})},
    {{0x0f, 0x0d, 0x10}, di::opcode_test(e_prefetchwt1, "prefetchwt1" + std::string{addr})},
  };
}

int main() {
  bool ok = run(Dyninst::Arch_x86, make_tests(Dyninst::Arch_x86));
  if(!run(Dyninst::Arch_x86_64, make_tests(Dyninst::Arch_x86_64))) {
    ok = false;
  }
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool run(Dyninst::Architecture arch, std::vector<prefetch_test> const &tests) {
  bool failed = false;
  auto sarch = Dyninst::getArchitectureName(arch);
  std::clog << "Running tests for 'prefetch' in " << sarch << " mode\n";
  for(auto const &t : tests) {
    di::InstructionDecoder d(t.bytes.data(), t.bytes.size(), arch);
    auto insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode " << sarch << " prefetch test\n";
      failed = true;
      continue;
    }

    std::clog << "Verifying '" << insn.format() << "'\n";

    if(!di::verify(insn, t.opcode)) {
      failed = true;
    }

    if(!insn.isPrefetch()) {
      std::cerr << "'" << insn.format() << "' is not categorized as a prefetch\n";
      failed = true;
    }

    // A prefetch is only a hint; it accesses no architectural memory.
    if(insn.readsMemory()) {
      std::cerr << "'" << insn.format() << "' should not read memory\n";
      failed = true;
    }
    if(insn.writesMemory()) {
      std::cerr << "'" << insn.format() << "' should not write memory\n";
      failed = true;
    }
  }
  return !failed;
}
