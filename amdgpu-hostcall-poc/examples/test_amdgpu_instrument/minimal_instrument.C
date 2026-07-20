/*
 *  minimal_instrument.C — the canonical, single-purpose AMDGPU hostcall
 *  instrumentation example (DyninstAPI static binary rewrite).
 *
 *  What a user actually writes: open a GPU code object, load a device hostcall
 *  library, pick a hook, and insert a call to it — here recording the active-lane
 *  (EXEC) mask at a few points in a kernel, so a CPU service thread can log which
 *  lanes were active at each site. No env-var gates, no capability matrix; see
 *  test_amdgpu_instrument.C for the full demonstrator.
 *
 *  Usage:  minimal_instrument <in.co> <out.co> [kernel] [hostcall_lib]
 *  Then run <out.co> under a host loader that defines the mailbox and services the
 *  hostcalls (see launcher/hostcall_launcher.cpp).
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
  // incUninstrumentable=true: the hook is a CALL TARGET (a small device function),
  // not something we instrument into, so dyninst's leaf/uninstrumentable filter
  // must not hide it.
  if (!img->findFunction(name, fs, true, false, /*incUninstrumentable=*/true) || fs.empty())
    return nullptr;
  return fs[0];
}

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "usage: " << argv[0] << " <in.co> <out.co> [kernel] [hostcall_lib]\n";
    return EXIT_FAILURE;
  }
  const char *in = argv[1], *out = argv[2];
  const char *kernelName = (argc > 3) ? argv[3] : "_Z9vectoraddPfPKfS1_i";
  const char *lib        = (argc > 4) ? argv[4] : "hostcall_lib.aliased.elf";

  // 1. Open the code object and load the device hostcall library (a dependency of
  //    the output; its hooks become callable from the mutatee).
  BPatch_binaryEdit *bin = bpatch.openBinary(in, /*openDependencies=*/true);
  if (!bin || !bin->loadLibrary(lib)) {
    std::cerr << "failed to open '" << in << "' or load '" << lib << "'\n";
    return EXIT_FAILURE;
  }
  BPatch_image *img = bin->getImage();

  // 2. Resolve the hooks: hc_open/hc_close bracket the trace file; hc_write_id(int)
  //    logs its argument through the hostcall mailbox.
  BPatch_function *hcOpen  = find(img, "hc_open");
  BPatch_function *hcClose = find(img, "hc_close");
  BPatch_function *hcWrite = find(img, "hc_write_id");
  BPatch_function *kernel  = find(img, kernelName);
  if (!hcOpen || !hcClose || !hcWrite || !kernel) {
    std::cerr << "missing hc_open/hc_close/hc_write_id or kernel '" << kernelName << "'\n";
    return EXIT_FAILURE;
  }

  // 3. Bracket the kernel: open the trace file at entry, close it at exit.
  //    (nullary calls — no argument snippet needed.)
  //    Always pass BPatch_lastSnippet as the insertion order — on AMDGPU the
  //    default order produces faulting trampoline codegen.
  BPatch_Vector<BPatch_snippet *> noArgs;
  BPatch_funcCallExpr openCall(*hcOpen, noArgs), closeCall(*hcClose, noArgs);
  if (auto *e = kernel->findPoint(BPatch_entry))
    bin->insertSnippet(openCall, *e, BPatch_callBefore, BPatch_lastSnippet);
  if (auto *x = kernel->findPoint(BPatch_exit))
    bin->insertSnippet(closeCall, *x, BPatch_callBefore, BPatch_lastSnippet);

  // 4. Record the EXEC mask at a few kernel instructions:
  //    hc_write_id( gpu_exec_mask() ). BPatch_gpuExecMaskExpr is a user-crafted
  //    GPU-value argument — read at the site and passed into the hook's parameter.
  //    We space sites out (every 4th instruction) purely to sample a few points;
  //    instrumenting adjacent instructions works too.
  BPatch_flowGraph *cfg = kernel->getCFG();
  std::set<BPatch_basicBlock *> blockSet;
  int done = 0, considered = 0;
  const int kMaxSites = 4, kStride = 4;  /* a few points spread across the kernel */
  if (cfg && cfg->getAllBasicBlocks(blockSet)) {
    std::vector<BPatch_basicBlock *> blocks(blockSet.begin(), blockSet.end());
    std::sort(blocks.begin(), blocks.end(), [](BPatch_basicBlock *a, BPatch_basicBlock *b) {
      return a->getStartAddress() < b->getStartAddress();       // deterministic, address order
    });
    for (BPatch_basicBlock *bb : blocks) {
      std::vector<std::pair<InstructionAPI::Instruction, Address>> insns;
      if (!bb->getInstructions(insns)) continue;
      for (auto &ia : insns) {
        if (done >= kMaxSites) break;
        if (considered++ % kStride != 0) continue;              // every 4th instruction
        BPatch_point *pt = kernel->findPoint(ia.second);
        if (!pt) continue;
        BPatch_gpuExecMaskExpr execMask;                        // <-- the GPU value to record
        BPatch_Vector<BPatch_snippet *> args{&execMask};
        BPatch_funcCallExpr writeCall(*hcWrite, args);
        if (bin->insertSnippet(writeCall, *pt, BPatch_callBefore, BPatch_lastSnippet)) {
          std::cout << "recording EXEC @ 0x" << std::hex << ia.second << std::dec << "\n";
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
  std::cout << "wrote " << out << " (" << done << " EXEC-record sites)\n";
  return EXIT_SUCCESS;
}
