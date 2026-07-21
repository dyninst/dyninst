/*
 *  preload_perwave_instrument.C — per-wave multi-file instrumentation for the
 *  LD_PRELOAD path. Inserts NULLARY pwg_open @entry and pwg_flush @exit. Each wave
 *  opens its own "wave_<wid>.txt" (via the gpu_fopen hostcall) and writes a line to
 *  it, indexing a GLOBAL per-wave buffer (g_pw_base) the host defines — no kernarg
 *  growth, unlike the dyninst per-wave variable (which the HIP runtime can't fill).
 *
 *  Usage: preload_perwave_instrument <in.co> <out.co> [kernel] [combined_lib]
 */
#include <cstdlib>
#include <iostream>
#include <vector>

#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"
#include "BPatch_point.h"
#include "BPatch_snippet.h"

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

  BPatch_function *pwgOpen  = find(img, "pwg_open");
  BPatch_function *pwgFlush = find(img, "pwg_flush");
  BPatch_function *kernel   = find(img, kernelName);
  if (!pwgOpen || !pwgFlush || !kernel) {
    std::cerr << "missing pwg_open/pwg_flush or kernel '" << kernelName << "'\n";
    return EXIT_FAILURE;
  }

  BPatch_Vector<BPatch_snippet *> noArgs;
  BPatch_funcCallExpr openCall(*pwgOpen, noArgs), flushCall(*pwgFlush, noArgs);
  if (auto *e = kernel->findPoint(BPatch_entry))
    bin->insertSnippet(openCall, *e, BPatch_callBefore, BPatch_lastSnippet);
  if (auto *x = kernel->findPoint(BPatch_exit))
    bin->insertSnippet(flushCall, *x, BPatch_callBefore, BPatch_lastSnippet);

  if (!bin->writeFile(out)) {
    std::cerr << "failed to write '" << out << "'\n";
    return EXIT_FAILURE;
  }
  std::cout << "wrote " << out << " (pwg_open @entry, pwg_flush @exit)\n";
  return EXIT_SUCCESS;
}
