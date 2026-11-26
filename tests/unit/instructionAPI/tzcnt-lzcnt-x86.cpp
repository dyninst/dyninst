#include "InstructionDecoder.h"
#include "Register.h"
#include "registers/x86_64_regs.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <set>
#include <vector>

/*
 * F3 0F BC is tzcnt and F3 0F BD is lzcnt: the F3 selects the instruction and is not a
 * REP prefix. Without it they are bsf and bsr. Each instruction reads only its source.
 */

namespace di = Dyninst::InstructionAPI;

namespace {

  struct test_case {
    char const* name;
    std::vector<unsigned char> bytes;
    entryID id;
    std::set<Dyninst::MachRegister> reads;
  };

}

int main() {
  namespace x = Dyninst::x86_64;
  std::array<test_case, 8> const cases = {{
    {"bsf %edx,%eax", {0x0f, 0xbc, 0xc2}, e_bsf, {x::edx}},
    {"tzcnt %edx,%eax", {0xf3, 0x0f, 0xbc, 0xc2}, e_tzcnt, {x::edx}},
    {"tzcnt %rdx,%rax", {0xf3, 0x48, 0x0f, 0xbc, 0xc2}, e_tzcnt, {x::rdx}},
    {"tzcnt %dx,%ax", {0x66, 0xf3, 0x0f, 0xbc, 0xc2}, e_tzcnt, {x::dx}},
    {"tzcnt (%rdi),%eax", {0xf3, 0x0f, 0xbc, 0x07}, e_tzcnt, {x::rdi}},
    {"bsr %edx,%eax", {0x0f, 0xbd, 0xc2}, e_bsr, {x::edx}},
    {"lzcnt %edx,%eax", {0xf3, 0x0f, 0xbd, 0xc2}, e_lzcnt, {x::edx}},
    {"lzcnt %rdx,%rax", {0xf3, 0x48, 0x0f, 0xbd, 0xc2}, e_lzcnt, {x::rdx}},
  }};

  bool ok = true;
  for(auto const& c : cases) {
    di::InstructionDecoder dec(c.bytes.data(), c.bytes.size(), Dyninst::Arch_x86_64);
    auto const insn = dec.decode();
    if(!insn.isValid() || insn.size() != c.bytes.size()) {
      std::cerr << c.name << ": decode failed\n";
      ok = false;
      continue;
    }
    std::set<di::RegisterAST::Ptr> read;
    insn.getReadSet(read);
    std::set<Dyninst::MachRegister> regs;
    for(auto const& r : read) {
      regs.insert(r->getID());
    }
    if(insn.getOperation().getID() != c.id || regs != c.reads) {
      std::cerr << c.name << ": decoded as '" << insn.format() << "' reading";
      for(auto const& r : regs) {
        std::cerr << " " << r.name();
      }
      std::cerr << "\n";
      ok = false;
    }
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
