/*
 *  user_probe_instrument.C — instrument a mutatee to call a USER-WRITTEN probe.
 *
 *  This is the "CPU-like" Dyninst-on-AMDGPU workflow. The user writes an ordinary
 *  device function (user_probe, in user_lib/user_probe.cpp) that calls into the
 *  hostcall runtime we provide (hc_write_id); the two are compiled together into
 *  one code object (user_lib/combined.aliased.elf). This mutator simply inserts a
 *  nullary call to `user_probe` at a few points in the target kernel — it does not
 *  know or care what the probe does. Compare minimal_instrument.C, which inserts a
 *  runtime wrapper (hc_write_id) directly with a tool-crafted argument; here the
 *  analysis lives in the user's own device code.
 *
 *  Usage:  user_probe_instrument <in.co> <out.co> [kernel] [combined_lib]
 *  Then run <out.co> under the hostcall launcher (which defines the mailbox and
 *  services the hostcalls). user_probe records the active-lane count per site.
 */
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_basicBlock.h"
#include "BPatch_flowGraph.h"
#include "BPatch_function.h"
#include "BPatch_point.h"
#include "BPatch_snippet.h"
#include "Instruction.h"

using namespace Dyninst;

BPatch bpatch;   // every mutator needs exactly one BPatch instance

static BPatch_function *find(BPatch_image *img, const char *name) {
  BPatch_Vector<BPatch_function *> fs;
  // incUninstrumentable=true: these are CALL TARGETS (small device functions),
  // so dyninst's leaf/uninstrumentable filter must not hide them.
  if (!img->findFunction(name, fs, true, false, /*incUninstrumentable=*/true) || fs.empty())
    return nullptr;
  return fs[0];
}

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "usage: " << argv[0] << " <in.co> <out.co> [kernel] [combined_lib]\n";
    return EXIT_FAILURE;
  }
  const char *in = argv[1], *out = argv[2];
  const char *kernelName = (argc > 3) ? argv[3] : "_Z9vectoraddPfPKfS1_i";
  const char *lib        = (argc > 4) ? argv[4] : "user_lib/combined.aliased.elf";

  // 1. Open the mutatee and load the combined (user probe + runtime) code object.
  BPatch_binaryEdit *bin = bpatch.openBinary(in, /*openDependencies=*/true);
  if (!bin || !bin->loadLibrary(lib)) {
    std::cerr << "failed to open '" << in << "' or load '" << lib << "'\n";
    return EXIT_FAILURE;
  }
  BPatch_image *img = bin->getImage();

  // 2. Resolve the runtime bracketers (hc_open/hc_close) and the USER probe. The
  //    tool treats user_probe as an opaque nullary hook.
  BPatch_function *hcOpen    = find(img, "hc_open");
  BPatch_function *hcClose   = find(img, "hc_close");
  BPatch_function *userProbe = find(img, "user_probe");
  BPatch_function *kernel    = find(img, kernelName);
  if (!hcOpen || !hcClose || !userProbe || !kernel) {
    std::cerr << "missing hc_open/hc_close/user_probe or kernel '" << kernelName << "'\n";
    return EXIT_FAILURE;
  }

  // 3. Bracket the kernel: open the trace file at entry, close it at exit.
  //    Always pass BPatch_lastSnippet — on AMDGPU the default order produces
  //    faulting trampoline codegen.
  BPatch_Vector<BPatch_snippet *> noArgs;
  BPatch_funcCallExpr openCall(*hcOpen, noArgs), closeCall(*hcClose, noArgs);
  if (auto *e = kernel->findPoint(BPatch_entry))
    bin->insertSnippet(openCall, *e, BPatch_callBefore, BPatch_lastSnippet);
  if (auto *x = kernel->findPoint(BPatch_exit))
    bin->insertSnippet(closeCall, *x, BPatch_callBefore, BPatch_lastSnippet);

  // 4. Insert a nullary call to the user's probe at a few kernel instructions.
  BPatch_flowGraph *cfg = kernel->getCFG();
  std::set<BPatch_basicBlock *> blockSet;
  int done = 0, considered = 0;
  const int kMaxSites = 8, kStride = 4;
  if (cfg && cfg->getAllBasicBlocks(blockSet)) {
    std::vector<BPatch_basicBlock *> blocks(blockSet.begin(), blockSet.end());
    std::sort(blocks.begin(), blocks.end(), [](BPatch_basicBlock *a, BPatch_basicBlock *b) {
      return a->getStartAddress() < b->getStartAddress();
    });
    for (BPatch_basicBlock *bb : blocks) {
      std::vector<std::pair<InstructionAPI::Instruction, Address>> insns;
      if (!bb->getInstructions(insns)) continue;
      for (auto &ia : insns) {
        if (done >= kMaxSites) break;
        if (considered++ % kStride != 0) continue;
        BPatch_point *pt = kernel->findPoint(ia.second);
        if (!pt) continue;
        BPatch_funcCallExpr probeCall(*userProbe, noArgs);   // nullary call to USER code
        if (bin->insertSnippet(probeCall, *pt, BPatch_callBefore, BPatch_lastSnippet)) {
          std::cout << "calling user_probe @ 0x" << std::hex << ia.second << std::dec << "\n";
          ++done;
        }
      }
    }
  }

  // 5. Write the instrumented code object.
  if (!bin->writeFile(out)) {
    std::cerr << "failed to write '" << out << "'\n";
    return EXIT_FAILURE;
  }
  std::cout << "wrote " << out << " (" << done << " user_probe call sites)\n";
  return EXIT_SUCCESS;
}
