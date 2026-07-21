/*
 *  multikernel_instrument.C — instrument SEVERAL kernels in one code object. For each
 *  kernel argv[4+i] it brackets the kernel with hc_open @entry (dedup'd to one shared
 *  trace file by the host) and hc_close @exit, and logs a distinct per-kernel id via
 *  hc_write_id(i) @entry — so the trace shows which kernels ran. Verifies the preload
 *  services ALL instrumented kernels dispatched by an app.
 *
 *  Usage: multikernel_instrument <in.co> <out.co> <hostcall_lib> <kernel> [kernel...]
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
  if (argc < 5) {
    std::cerr << "usage: " << argv[0] << " <in.co> <out.co> <hostcall_lib> <kernel> [kernel...]\n";
    return EXIT_FAILURE;
  }
  const char *in = argv[1], *out = argv[2], *lib = argv[3];

  BPatch_binaryEdit *bin = bpatch.openBinary(in, /*openDependencies=*/true);
  if (!bin || !bin->loadLibrary(lib)) { std::cerr << "open/load failed\n"; return EXIT_FAILURE; }
  BPatch_image *img = bin->getImage();

  BPatch_function *hcOpen  = find(img, "hc_open");
  BPatch_function *hcWrite = find(img, "hc_write_id");
  BPatch_function *hcClose = find(img, "hc_close");
  if (!hcOpen || !hcWrite || !hcClose) { std::cerr << "missing hc_* hooks\n"; return EXIT_FAILURE; }

  const int baseId = getenv("DYNINST_ID_BASE") ? atoi(getenv("DYNINST_ID_BASE")) : 0;
  for (int k = 4; k < argc; k++) {
    BPatch_function *kernel = find(img, argv[k]);
    if (!kernel) { std::cerr << "missing kernel '" << argv[k] << "'\n"; return EXIT_FAILURE; }
    int id = baseId + (k - 4);
    BPatch_Vector<BPatch_snippet *> noArgs;
    BPatch_constExpr idExpr(id);
    BPatch_Vector<BPatch_snippet *> idArg{&idExpr};
    if (auto *e = kernel->findPoint(BPatch_entry)) {
      bin->insertSnippet(BPatch_funcCallExpr(*hcOpen, noArgs), *e, BPatch_callBefore, BPatch_lastSnippet);
      bin->insertSnippet(BPatch_funcCallExpr(*hcWrite, idArg), *e, BPatch_callBefore, BPatch_lastSnippet);
    }
    if (auto *x = kernel->findPoint(BPatch_exit))
      bin->insertSnippet(BPatch_funcCallExpr(*hcClose, noArgs), *x, BPatch_callBefore, BPatch_lastSnippet);
    std::cout << "instrumented kernel[" << id << "] " << argv[k]
              << " (hc_open + hc_write_id(" << id << ") @entry, hc_close @exit)\n";
  }

  if (!bin->writeFile(out)) { std::cerr << "write failed\n"; return EXIT_FAILURE; }
  std::cout << "wrote " << out << "\n";
  return EXIT_SUCCESS;
}
