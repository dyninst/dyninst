#include "InstructionDecoder.h"
#include "Register.h"
#include "entryIDs.h"
#include "registers/x86_64_regs.h"

#include <array>
#include <cstdlib>
#include <iostream>
#include <set>
#include <vector>

/*
 * A REP/REPNZ prefix repeats only the string instructions (ins, outs, movs, lods,
 * stos, cmps, scas), which then read and write rcx and read df; repe/repne cmps and
 * scas also read zf. On any other instruction F3/F2 is ignored (rep ret) or is the
 * BND prefix (bnd jmp), and the instruction must not read or write rcx, df or zf
 * because of it.
 */

namespace di = Dyninst::InstructionAPI;

namespace {

  struct test_case {
    char const* name;
    std::vector<unsigned char> bytes;
    prefixEntryID prefix;
    bool rcx_df; // reads rcx and df, writes rcx
    bool zf;     // reads zf
  };

  std::set<Dyninst::MachRegister> ids(std::set<di::RegisterAST::Ptr> const& regs) {
    std::set<Dyninst::MachRegister> out;
    for(auto const& r : regs) {
      out.insert(r->getID());
    }
    return out;
  }

}

int main() {
  namespace x = Dyninst::x86_64;
  std::array<test_case, 11> const cases = {{
    {"rep movsb", {0xf3, 0xa4}, prefix_rep, true, false},
    {"rep stos %rax", {0xf3, 0x48, 0xab}, prefix_rep, true, false},
    {"repz cmpsb", {0xf3, 0xa6}, prefix_rep, true, true},
    {"repz scasb", {0xf3, 0xae}, prefix_rep, true, true},
    {"repnz scasb", {0xf2, 0xae}, prefix_repnz, true, true},
    {"repnz cmpsb", {0xf2, 0xa6}, prefix_repnz, true, true},
    {"repz ret", {0xf3, 0xc3}, prefix_none, false, false},
    {"bnd ret", {0xf2, 0xc3}, prefix_none, false, false},
    {"bnd jmp *%rbx", {0xf2, 0xff, 0xe3}, prefix_none, false, false},
    {"repz add %rdx,%rax", {0xf3, 0x48, 0x01, 0xd0}, prefix_none, false, false},
    {"movsd %xmm1,%xmm0", {0xf2, 0x0f, 0x10, 0xc1}, prefix_none, false, false},
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
    std::set<di::RegisterAST::Ptr> r, w;
    insn.getReadSet(r);
    insn.getWriteSet(w);
    auto const reads = ids(r);
    auto const writes = ids(w);
    bool const rcx_df = reads.count(x::rcx) && reads.count(x::df) && writes.count(x::rcx);
    bool const any_rcx_df = reads.count(x::rcx) || reads.count(x::df) || writes.count(x::rcx);
    bool const zf = reads.count(x::zf) != 0;
    bool const good = insn.getOperation().getPrefixID() == c.prefix &&
                      (c.rcx_df ? rcx_df : !any_rcx_df) && zf == c.zf;
    if(!good) {
      std::cerr << c.name << ": '" << insn.format() << "' reads";
      for(auto const& reg : reads) {
        std::cerr << " " << reg.name();
      }
      std::cerr << ", writes";
      for(auto const& reg : writes) {
        std::cerr << " " << reg.name();
      }
      std::cerr << "\n";
      ok = false;
    }
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
