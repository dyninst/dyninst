#include "CFG.h"
#include "CodeObject.h"
#include "Location.h"
#include "bitArray.h"
#include "liveness.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"

#include <cstdio>
#include <cstdlib>
#include <map>

/*
 * Liveness of M0 and SCC around real writer/reader pairs in the gfx908
 * IndirectControlFlowAnalysis test binary:
 *
 *   0x1c70  s_and_b32 m0, s14, 0xff    writes M0
 *   0x1c7c  global_store_dwordx2 ...
 *   0x1c84  s_sendmsg MSG_INTERRUPT    reads M0
 *   0x1cbc  s_cmp_eq_u32 s10, 0        writes SCC
 *   0x1cc0  s_cbranch_scc1 ...         reads SCC
 *   0x1cc4  s_sleep 1
 */

namespace {
  struct Expectation {
    Dyninst::Address addr;
    Dyninst::MachRegister reg;
    bool live;  // before the instruction at addr
  };

  Expectation const expected[] = {
    {0x1c70, Dyninst::amdgpu_gfx908::m0, false},
    {0x1c7c, Dyninst::amdgpu_gfx908::m0, true},
    {0x1c84, Dyninst::amdgpu_gfx908::m0, true},
    {0x1cbc, Dyninst::amdgpu_gfx908::src_scc, false},
    {0x1cc0, Dyninst::amdgpu_gfx908::src_scc, true},
    {0x1cc4, Dyninst::amdgpu_gfx908::src_scc, false},
  };
}

int main(int argc, char **argv) {
  if(argc != 2) {
    std::fprintf(stderr, "Usage: %s <gfx908 IndirectControlFlowAnalysis binary>\n", argv[0]);
    return EXIT_FAILURE;
  }

  auto *sts = new Dyninst::ParseAPI::SymtabCodeSource(argv[1]);
  auto *co = new Dyninst::ParseAPI::CodeObject(sts);

  // The analyzer for the binary's own architecture: the ABI of getABI(int)
  // depends on the codegen architecture of the build.
  LivenessAnalyzer la(sts->getArch(), 8);

  std::map<Dyninst::Address, bool> checked;
  bool ok = true;
  for(auto *f : co->funcs()) {
    for(auto *b : f->blocks()) {
      Dyninst::ParseAPI::Block::Insns insns;
      b->getInsns(insns);
      for(auto const &e : expected) {
        auto i = insns.find(e.addr);
        if(i == insns.end() || checked[e.addr]) {
          continue;
        }
        int const index = la.getIndex(e.reg);
        bitArray live;
        Dyninst::ParseAPI::Location loc(f, Dyninst::ParseAPI::InsnLoc(b, i->first, i->second));
        bool const queried = la.query(loc, LivenessAnalyzer::Before, live);
        if(index < 0 || !queried || live[index] != e.live) {
          std::fprintf(stderr, "FAILED: 0x%lx %s: expected %s, got %s\n", static_cast<unsigned long>(e.addr),
                              e.reg.name().c_str(), e.live ? "live" : "dead",
                              index < 0 ? "untracked" : (!queried ? "no result" : (live[index] ? "live" : "dead")));
          ok = false;
        }
      }
      for(auto const &e : expected) {
        if(insns.count(e.addr)) {
          checked[e.addr] = true;
        }
      }
    }
  }

  for(auto const &e : expected) {
    if(!checked[e.addr]) {
      std::fprintf(stderr, "FAILED: no instruction at 0x%lx\n", static_cast<unsigned long>(e.addr));
      ok = false;
    }
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
