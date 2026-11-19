#include "CFG.h"
#include "CodeObject.h"
#include "CodeSource.h"
#include "InstructionCategories.h"
#include "Location.h"
#include "bitArray.h"
#include "liveness.h"
#include "registers/x86_64_regs.h"

#include <cstdio>
#include <cstdlib>

/*
 * A branch into a function entry is a tail call, so the callee's argument registers are
 * live at the branch. tailcall_fixture.S loads an argument into %rdi before branching into
 * callee with a jmp, a jne whose fall-through returns, and a jump table with one slot at
 * callee. Instrumentation before such a branch must not see %rdi as dead and clobber it.
 */

namespace pa = Dyninst::ParseAPI;
namespace di = Dyninst::InstructionAPI;

namespace {
  struct Expectation {
    char const *func;
    di::InsnCategory category;  // the last instruction of this kind in func
    bool live;                  // %rdi before the instruction
  };

  Expectation const expected[] = {
    {"jmp_tail", di::c_BranchInsn, true},
    {"jne_tail", di::c_BranchInsn, true},
    {"table_tail", di::c_BranchInsn, true},
    // The fall-through returns; %rdi is not a return register.
    {"jne_tail", di::c_ReturnInsn, false},
  };

  struct Site {
    pa::Block *block{nullptr};
    Dyninst::Address addr{0};
    di::Instruction insn{};
  };

  // The last instruction of the given category in f. For table_tail, the indirect jump
  // comes after the bounds check.
  Site find(pa::Function *f, di::InsnCategory cat) {
    Site site;
    for(auto *b : f->blocks()) {
      pa::Block::Insns insns;
      b->getInsns(insns);
      for(auto const &i : insns) {
        if(i.second.getCategory() == cat && (!site.block || i.first > site.addr)) {
          site.block = b;
          site.addr = i.first;
          site.insn = i.second;
        }
      }
    }
    return site;
  }
}

int main(int argc, char **argv) {
  if(argc != 2) {
    std::fprintf(stderr, "Usage: %s <tailcall fixture>\n", argv[0]);
    return EXIT_FAILURE;
  }

  auto *sts = new pa::SymtabCodeSource(argv[1]);
  auto *co = new pa::CodeObject(sts);
  co->parse();

  LivenessAnalyzer la(8);
  int const rdi = la.getIndex(Dyninst::x86_64::rdi);
  if(rdi < 0) {
    std::fprintf(stderr, "FAILED: %%rdi is not tracked\n");
    return EXIT_FAILURE;
  }

  bool ok = true;
  for(auto const &e : expected) {
    pa::Function *f = nullptr;
    for(auto *g : co->funcs()) {
      if(g->name() == e.func) {
        f = g;
      }
    }
    char const *what = e.category == di::c_ReturnInsn ? "ret" : "branch";
    Site site;
    if(f) {
      site = find(f, e.category);
    }
    if(!site.block) {
      std::fprintf(stderr, "FAILED: %s: no %s\n", e.func, what);
      ok = false;
      continue;
    }
    pa::Location loc(f, pa::InsnLoc(site.block, site.addr, site.insn));
    bitArray live;
    if(!la.query(loc, LivenessAnalyzer::Before, live)) {
      std::fprintf(stderr, "FAILED: %s %s: no liveness result\n", e.func, what);
      ok = false;
      continue;
    }
    if(live[rdi] != e.live) {
      std::fprintf(stderr, "FAILED: %s %s at 0x%lx: %%rdi expected %s, got %s\n", e.func, what,
                   static_cast<unsigned long>(site.addr), e.live ? "live" : "dead",
                   live[rdi] ? "live" : "dead");
      ok = false;
    }
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
