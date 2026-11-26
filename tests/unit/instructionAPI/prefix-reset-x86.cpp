#include "InstructionDecoder.h"
#include "Register.h"
#include "registers/x86_64_regs.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>

/*
 * One InstructionDecoder decodes a sequence of instructions. The REP/REPNZ and
 * segment-override prefixes of one instruction must not show up on the next one:
 * a 'REP add' also reads rcx and df (and zf for REPNZ), and a stale fs override
 * makes it read fs.
 *
 * The REX.W byte of 'add %rdx,%rax' is itself a prefix and so clears a stale
 * segment override, but not a stale REP; 'add %edx,%eax' has no prefix at all.
 */

namespace di = Dyninst::InstructionAPI;

namespace {

  // An add after each prefixed instruction
  constexpr std::array<unsigned char, 41> buffer = {{
    0xf3, 0x0f, 0xbc, 0xc2,                                // tzcnt %edx,%eax         (F3)
    0x48, 0x01, 0xd0,                                      // add %rdx,%rax
    0xf3, 0xa4,                                            // rep movsb               (F3)
    0x48, 0x01, 0xd0,                                      // add %rdx,%rax
    0x64, 0x48, 0x8b, 0x04, 0x25, 0x28, 0x00, 0x00, 0x00,  // mov %fs:0x28,%rax       (64)
    0x48, 0x01, 0xd0,                                      // add %rdx,%rax
    0x64, 0x48, 0x8b, 0x04, 0x25, 0x28, 0x00, 0x00, 0x00,  // mov %fs:0x28,%rax       (64)
    0x01, 0xd0,                                            // add %edx,%eax           (no prefix)
    0xf2, 0xae,                                            // repnz scasb             (F2)
    0x48, 0x01, 0xd0,                                      // add %rdx,%rax
    0x90
  }};

  struct pair_case {
    char const* after;
    std::set<Dyninst::MachRegister> expected;
  };

  bool check_add(di::Instruction const& insn, pair_case const& c) {
    std::set<di::RegisterAST::Ptr> read;
    insn.getReadSet(read);
    std::set<Dyninst::MachRegister> regs;
    for(auto const& r : read) {
      regs.insert(r->getID());
    }
    if(regs != c.expected) {
      std::cerr << "'" << insn.format() << "' after " << c.after << " reads";
      for(auto const& r : regs) {
        std::cerr << " " << r.name();
      }
      std::cerr << ", expected";
      for(auto const& r : c.expected) {
        std::cerr << " " << r.name();
      }
      std::cerr << "\n";
      return false;
    }
    return true;
  }

}

int main() {
  std::set<Dyninst::MachRegister> const rax_rdx{Dyninst::x86_64::rax, Dyninst::x86_64::rdx};
  std::set<Dyninst::MachRegister> const eax_edx{Dyninst::x86_64::eax, Dyninst::x86_64::edx};
  std::array<pair_case, 5> const cases = {{
    {"tzcnt", rax_rdx},
    {"rep movsb", rax_rdx},
    {"mov %fs:0x28", rax_rdx},
    {"mov %fs:0x28", eax_edx},
    {"repnz scasb", rax_rdx},
  }};

  di::InstructionDecoder dec(buffer.data(), buffer.size(), Dyninst::Arch_x86_64);
  bool ok = true;
  for(auto const& c : cases) {
    auto prefixed = dec.decode();
    auto add = dec.decode();
    if(!prefixed.isValid() || !add.isValid()) {
      std::cerr << "decode failed after " << c.after << "\n";
      return EXIT_FAILURE;
    }
    if(!check_add(add, c)) {
      ok = false;
    }
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
