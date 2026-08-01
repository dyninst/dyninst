/*
 * Exception-handling integration test driver (dynamic instrumentation).
 *
 * processCreate()s the mutatee, then instruments the entry of every basic block
 * of every function in the mutatee's own image with a counter increment. That
 * forces Dyninst to relocate those functions in the live process (the same
 * thing the original issue reproducer does), which is what triggers .eh_frame /
 * .gcc_except_table synthesis for the relocated code. The mutatee then throws
 * and catches; if the synthesized unwind info is missing or wrong, the mutatee
 * terminates instead of catching.
 *
 * Companion of eh_rewrite_driver: same mutatees, but the dynamic-instrumentation
 * path (BPatch_process) instead of the static rewriter (BPatch_binaryEdit). Pass
 * iff the mutatee exits normally with code 0 (it prints CAUGHT); a non-zero exit
 * or a termination by signal (std::terminate -> abort) is a failure.
 */
#include "BPatch.h"
#include "BPatch_process.h"
#include "BPatch_image.h"
#include "BPatch_function.h"
#include "BPatch_flowGraph.h"
#include "BPatch_basicBlock.h"
#include "BPatch_point.h"
#include "BPatch_snippet.h"
#include "BPatch_module.h"
#include "BPatch_object.h"

#include <cstdio>
#include <set>
#include <string>
#include <vector>

static BPatch bpatch;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::fprintf(stderr, "usage: %s <mutatee>\n", argv[0]);
    return 2;
  }
  const char* path = argv[1];
  const char* args[] = { path, nullptr };

  BPatch_process* proc = bpatch.processCreate(path, args);
  if (!proc) {
    std::fprintf(stderr, "eh_dynamic_driver: processCreate(%s) failed\n", path);
    return 2;
  }

  BPatch_image* image = proc->getImage();
  std::vector<BPatch_function*>* funcs = image->getProcedures();
  if (!funcs || funcs->empty()) {
    std::fprintf(stderr, "eh_dynamic_driver: no procedures in %s\n", path);
    return 2;
  }

  // A scratch counter; storing to it is just a pretext to force relocation.
  BPatch_variableExpr* counter = proc->malloc(sizeof(long));
  if (!counter) {
    std::fprintf(stderr, "eh_dynamic_driver: malloc failed\n");
    return 2;
  }

  // Instrument the entry of every basic block of every function in the
  // mutatee's own image (skip shared libraries / the runtime), forcing those
  // functions to be relocated in the live process.
  proc->beginInsertionSet();
  unsigned instrumented = 0;
  for (BPatch_function* f : *funcs) {
    if (!f) continue;
    BPatch_module* m = f->getModule();
    if (!m || m->isSharedLib()) continue;   // mutatee code only
    BPatch_flowGraph* cfg = f->getCFG();
    if (!cfg) continue;
    std::set<BPatch_basicBlock*> blocks;
    cfg->getAllBasicBlocks(blocks);
    for (BPatch_basicBlock* bb : blocks) {
      BPatch_point* pt = bb->findEntryPoint();
      if (!pt) continue;
      BPatch_arithExpr bump(BPatch_assign, *counter,
                            BPatch_arithExpr(BPatch_plus, *counter, BPatch_constExpr(1)));
      if (proc->insertSnippet(bump, *pt, BPatch_callBefore)) instrumented++;
    }
  }
  proc->finalizeInsertionSet(false, nullptr);
  std::fprintf(stderr, "eh_dynamic_driver: instrumented %u basic-block entries\n", instrumented);

  // Run to completion.
  proc->continueExecution();
  while (!proc->isTerminated()) {
    bpatch.waitForStatusChange();
    if (proc->isStopped()) proc->continueExecution();
  }

  if (proc->terminationStatus() == ExitedViaSignal) {
    std::fprintf(stderr, "MUTATEE_SIGNALED=%d (exception not caught -> terminate)\n",
                 proc->getExitSignal());
    return 1;
  }
  int code = proc->getExitCode();
  std::fprintf(stderr, "MUTATEE_EXIT=%d\n", code);
  return code;  // 0 iff the mutatee caught and returned cleanly
}
