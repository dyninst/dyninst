/*
 *  per_wave_instrument.C — pass a PER-WAVE buffer slice to an inserted probe.
 *
 *  Demonstrates the first-class per-wave variable: the tool allocates a
 *  BPatch_perWaveVar (per-wave-distinct storage), and passes THIS wave's slice as a
 *  pointer argument to the user probe (pw_probe, in user_lib/user_probe.cpp). A
 *  compile-time global would be shared by every wavefront; a per-wave variable gives
 *  each wave its own slice, so a probe can keep per-wave state.
 *
 *  Usage:  per_wave_instrument <in.co> <out.co> [kernel] [combined_lib]
 *  Then run <out.co> under the hostcall launcher, which backs the per-wave buffer and
 *  dumps it after the kernel. [M1a: the probe writes a marker at slice[0].]
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

BPatch bpatch;

static BPatch_function *find(BPatch_image *img, const char *name) {
  BPatch_Vector<BPatch_function *> fs;
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

  BPatch_binaryEdit *bin = bpatch.openBinary(in, /*openDependencies=*/true);
  if (!bin || !bin->loadLibrary(lib)) {
    std::cerr << "failed to open '" << in << "' or load '" << lib << "'\n";
    return EXIT_FAILURE;
  }
  BPatch_image *img = bin->getImage();

  BPatch_function *pwOpen  = find(img, "pw_open");
  BPatch_function *pwProbe = find(img, "pw_probe");
  BPatch_function *pwFlush = find(img, "pw_flush");
  BPatch_function *kernel  = find(img, kernelName);
  if (!pwOpen || !pwProbe || !pwFlush || !kernel) {
    std::cerr << "missing pw_open/pw_probe/pw_flush or kernel '" << kernelName << "'\n";
    return EXIT_FAILURE;
  }

  // Allocate a per-wave variable (256 bytes/wave). Each wave opens its OWN file at
  // entry (handle stashed in the slice), accumulates during the kernel, and flushes
  // one line to that file at exit. All three inserted calls get this wave's slice.
  BPatch_perWaveVar pw(/*bytesPerWave=*/256);

  // Entry: open this wave's file.
  if (auto *e = kernel->findPoint(BPatch_entry)) {
    BPatch_snippet opSlice = pw.address();
    BPatch_Vector<BPatch_snippet *> opArgs{ &opSlice };
    bin->insertSnippet(BPatch_funcCallExpr(*pwOpen, opArgs), *e, BPatch_callBefore, BPatch_lastSnippet);
  }
  // Exit: flush this wave's accumulated stats to its file.
  if (auto *x = kernel->findPoint(BPatch_exit)) {
    BPatch_snippet flSlice = pw.address();
    BPatch_Vector<BPatch_snippet *> flArgs{ &flSlice };
    bin->insertSnippet(BPatch_funcCallExpr(*pwFlush, flArgs), *x, BPatch_callBefore, BPatch_lastSnippet);
  }

  BPatch_flowGraph *cfg = kernel->getCFG();
  std::set<BPatch_basicBlock *> blockSet;
  int done = 0, considered = 0;
  const int kMaxSites = 4, kStride = 4;
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
        BPatch_snippet slice = pw.address();               // this wave's slice base (void*)
        BPatch_Vector<BPatch_snippet *> args{ &slice };
        if (bin->insertSnippet(BPatch_funcCallExpr(*pwProbe, args),
                               *pt, BPatch_callBefore, BPatch_lastSnippet)) {
          std::cout << "pw_probe(perWaveSlice) @ 0x" << std::hex << ia.second << std::dec << "\n";
          ++done;
        }
      }
    }
  }

  if (!bin->writeFile(out)) {
    std::cerr << "failed to write '" << out << "'\n";
    return EXIT_FAILURE;
  }
  std::cout << "wrote " << out << " (" << done << " pw_probe call sites)\n";
  return EXIT_SUCCESS;
}
