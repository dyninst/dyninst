/*
 * Exception-handling integration test driver.
 *
 * Opens the input binary as a rewriter, relocates every one of its functions
 * (no instrumentation needed -- relocation alone moves the code out of .text
 * into the instrumentation section, which is what forces Dyninst to synthesize
 * the .eh_frame / .gcc_except_table for the relocated code), and writes the
 * result. The companion mutatees throw and catch C++ exceptions; if the
 * synthesized unwind info is wrong the rewritten mutatee terminates instead of
 * catching, so the "did it still catch?" check in CMake is the actual test.
 *
 * Mirrors the classic testsuite `test_reloc` mutator, minus the harness.
 */
#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_image.h"
#include "BPatch_function.h"
#include "BPatch_object.h"
#include "BPatch_module.h"

#include <cstdio>
#include <string>
#include <vector>

static BPatch bpatch;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::fprintf(stderr, "usage: %s <input-binary> <output-binary>\n", argv[0]);
    return 2;
  }
  const char* in = argv[1];
  const char* out = argv[2];

  BPatch_binaryEdit* be = bpatch.openBinary(in, /*openDependencies=*/false);
  if (!be) {
    std::fprintf(stderr, "eh_rewrite_driver: openBinary(%s) failed\n", in);
    return 2;
  }

  BPatch_image* image = be->getImage();
  std::vector<BPatch_function*>* funcs = image->getProcedures();
  if (!funcs || funcs->empty()) {
    std::fprintf(stderr, "eh_rewrite_driver: no procedures in %s\n", in);
    return 2;
  }

  // Relocate everything at once (per-function relocation of thousands of
  // functions individually would be pathologically slow).
  be->beginInsertionSet();
  for (BPatch_function* f : *funcs) {
    if (!f) continue;
    // Never relocate the runtime library itself.
    BPatch_module* m = f->getModule();
    if (m && m->getObject() &&
        m->getObject()->name().find("libdyninstAPI_RT") != std::string::npos)
      continue;
    f->relocateFunction();
  }
  be->finalizeInsertionSet(/*atomic=*/false, /*modified=*/nullptr);

  if (!be->writeFile(out)) {
    std::fprintf(stderr, "eh_rewrite_driver: writeFile(%s) failed\n", out);
    return 2;
  }
  return 0;
}
