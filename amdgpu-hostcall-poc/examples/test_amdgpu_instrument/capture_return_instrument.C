/*
 *  capture_return_instrument.C — per-wave variable that HOLDS a call's return value.
 *
 *  Demonstrates the composable model: a per-wave variable is assigned from an inserted
 *  call's ABI return value, then that held value is passed as an argument to a later
 *  inserted call. No hand-written wrapper stores the handle — the call SITE captures it.
 *
 *    hv = pw_openfile();        // insert at entry; capture the returned handle into hv
 *    pw_writeln( hv.value() );  // insert at sites; pass the held handle as the arg
 *
 *  pw_openfile()/pw_writeln() just follow the ABI return/arg contract (see user_lib).
 *
 *  Usage:  capture_return_instrument <in.co> <out.co> [kernel] [combined_lib]
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

  BPatch_function *pwOpen  = find(img, "pw_openfile");
  BPatch_function *pwWrite = find(img, "pw_writeln");
  BPatch_function *kernel  = find(img, kernelName);
  if (!pwOpen || !pwWrite || !kernel) {
    std::cerr << "missing pw_openfile/pw_writeln or kernel '" << kernelName << "'\n";
    return EXIT_FAILURE;
  }

  // A per-wave variable holding a 64-bit file handle.
  BPatch_perWaveVar hv(/*bytesPerWave=*/8);

  // Entry:  hv = pw_openfile()  -- capture the fopen HANDLE (the call's ABI return
  // value) into hv. No hand-written store: the call SITE captures it.
  if (auto *e = kernel->findPoint(BPatch_entry)) {
    BPatch_snippet cap = hv.captureReturn();            // marker: store the return into hv
    BPatch_Vector<BPatch_snippet *> args{ &cap };
    bin->insertSnippet(BPatch_funcCallExpr(*pwOpen, args), *e, BPatch_callBefore, BPatch_lastSnippet);
  }

  // Sites:  pw_writeln(hv.value())  -- pass the CAPTURED handle as fwrite's first
  // parameter (the original goal: per-wave var holds fopen's return, fed to fwrite).
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
        BPatch_snippet h = hv.value();                  // the captured file handle
        BPatch_Vector<BPatch_snippet *> args{ &h };
        if (bin->insertSnippet(BPatch_funcCallExpr(*pwWrite, args), *pt, BPatch_callBefore, BPatch_lastSnippet)) {
          std::cout << "pw_writeln(hv.value()) @ 0x" << std::hex << ia.second << std::dec << "\n";
          ++done;
        }
      }
    }
  }

  if (!bin->writeFile(out)) {
    std::cerr << "failed to write '" << out << "'\n";
    return EXIT_FAILURE;
  }
  std::cout << "wrote " << out << " (hv=pw_openfile() @entry, " << done << " pw_writeln(hv.value()) sites)\n";
  return EXIT_SUCCESS;
}
