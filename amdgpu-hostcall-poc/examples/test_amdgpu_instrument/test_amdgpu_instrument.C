/*
 *  AMDGPU hostcall instrumentation demo (DyninstAPI, static binary rewrite).
 *
 *  Loads the device hostcall library (hostcall_lib.aliased.elf) into a GPU
 *  code object and inserts GPU->CPU hostcalls:
 *
 *    - hc_open   at the ENTRY of a target kernel      (open the trace file)
 *    - hc_close  at the EXIT  of that target kernel   (close the trace file)
 *    - hc_write  at the ENTRY of a random subset of   (log "function entered")
 *                the mutatee's other functions
 *
 *  The inserted calls take NO arguments: dyninst's AMDGPU emitCall cannot pass
 *  arguments yet, so the hostcall library exposes nullary wrappers (hc_open /
 *  hc_write / hc_close) that marshal fixed values through the shared mailbox.
 *  Each inserted external call also triggers the kernel-descriptor VGPR bump
 *  (emit-amdgpu.C bumpCallerKdForCallee) so the callee's registers fit.
 */

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include "BPatch.h"
#include "BPatch_basicBlock.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_flowGraph.h"
#include "BPatch_function.h"
#include "BPatch_module.h"
#include "BPatch_object.h"
#include "BPatch_point.h"
#include "Instruction.h"

#include <set>
#include <utility>

using namespace Dyninst;

static const char* USAGE =
    " <in_binary> <out_binary> [target_kernel] [inst_library]\n"
    "    target_kernel : mangled name of the kernel to open/close around\n"
    "                    (default: _Z9vectoraddPfPKfS1_i)\n"
    "    inst_library  : device hostcall code object to load\n"
    "                    (default: hostcall_lib.aliased.elf)\n";

// Deterministically pick "some" of the kernel's instructions to precede with an
// hc_write: every WRITE_STRIDE-th instruction, capped at MAX_WRITES so a heavy
// hostcall isn't injected everywhere. Deterministic => reproducible rewrites.
static const int WRITE_STRIDE = 4;
static const int MAX_WRITES = 8;

/* Every Dyninst mutator needs exactly one BPatch instance. */
BPatch bpatch;

BPatch_function* findFuncByName(BPatch_image* image, const char* name) {
  BPatch_Vector<BPatch_function*> funcs;
  // incUninstrumentable=true: the hostcall wrappers are CALL TARGETS, not
  // functions we instrument into, so dyninst's "uninstrumentable" filter
  // (which excludes tiny/leaf device functions) must not hide them.
  if(!image->findFunction(name, funcs, /*showError=*/true, /*regex=*/false,
                          /*incUninstrumentable=*/true) ||
     funcs.empty() || funcs[0] == NULL) {
    cerr << "  ! could not find function '" << name << "'" << endl;
    return NULL;
  }
  return funcs[0];
}

// Insert a no-argument call to `target` at the given location/timing of `fn`.
bool insertCall(BPatch_binaryEdit* appBin, BPatch_function* fn,
                BPatch_procedureLocation loc, BPatch_callWhen when,
                BPatch_function* target, const char* what) {
  vector<BPatch_point*>* points = fn->findPoint(loc);
  if(points == NULL || points->empty()) {
    cerr << "  ! no " << what << " points found" << endl;
    return false;
  }
  BPatch_Vector<BPatch_snippet*> noArgs;               // AMDGPU: nullary calls only
  BPatch_funcCallExpr call(*target, noArgs);
  BPatchSnippetHandle* handle = appBin->insertSnippet(call, *points, when, BPatch_lastSnippet);
  if(!handle) {
    cerr << "  ! failed to insert " << what << " call" << endl;
    return false;
  }
  return true;
}

int main(int argc, char* argv[]) {
  if(argc < 3) {
    cerr << "Usage: " << argv[0] << USAGE;
    return EXIT_FAILURE;
  }
  const char* inBinary  = argv[1];
  const char* outBinary = argv[2];
  const string targetKernel = (argc > 3) ? argv[3] : "_Z9vectoraddPfPKfS1_i";
  const char* instLibrary   = (argc > 4) ? argv[4] : "hostcall_lib.aliased.elf";

  /* Open the target code object for static rewriting. */
  BPatch_binaryEdit* appBin = bpatch.openBinary(inBinary, /*openDependencies=*/true);
  if(appBin == NULL) {
    cerr << "Failed to open binary '" << inBinary << "'" << endl;
    return EXIT_FAILURE;
  }

  /* Load the device hostcall library and make it a dependency of the output. */
  if(!appBin->loadLibrary(instLibrary)) {
    cerr << "Failed to load instrumentation library '" << instLibrary << "'" << endl;
    return EXIT_FAILURE;
  }

  BPatch_image* appImage = appBin->getImage();

  /* Resolve the three nullary hostcall wrappers in the loaded library. */
  BPatch_function* fnOpen  = findFuncByName(appImage, "hc_open");
  BPatch_function* fnWrite = findFuncByName(appImage, "hc_write");
  BPatch_function* fnClose = findFuncByName(appImage, "hc_close");
  if(!fnOpen || !fnWrite || !fnClose) {
    cerr << "Instrumentation library is missing hc_open/hc_write/hc_close" << endl;
    return EXIT_FAILURE;
  }

  /* ---- open at start / close at end of the target kernel ---- */
  BPatch_function* kernel = findFuncByName(appImage, targetKernel.c_str());
  if(!kernel) {
    cerr << "Target kernel '" << targetKernel << "' not found" << endl;
    return EXIT_FAILURE;
  }
  BPatch_module* mutateeMod = kernel->getModule();
  {
    char modName[1024] = {0};
    mutateeMod->getFullName(modName, sizeof modName);
    cout << "Target kernel '" << targetKernel << "' in module " << modName << endl;
  }

  cout << "Inserting hc_open  at ENTRY of " << targetKernel << endl;
  insertCall(appBin, kernel, BPatch_entry, BPatch_callBefore, fnOpen, "entry");
  cout << "Inserting hc_close at EXIT  of " << targetKernel << endl;
  insertCall(appBin, kernel, BPatch_exit, BPatch_callBefore, fnClose, "exit");

  /* ---- hc_write before every WRITE_STRIDE-th INSTRUCTION of the kernel ---- */
  BPatch_flowGraph* cfg = kernel->getCFG();
  std::set<BPatch_basicBlock*> blocks;
  int considered = 0, instrumented = 0;
  if(cfg && cfg->getAllBasicBlocks(blocks)) {
    // Deterministic order: walk blocks by start address, instructions in order.
    vector<BPatch_basicBlock*> ordered(blocks.begin(), blocks.end());
    std::sort(ordered.begin(), ordered.end(),
              [](BPatch_basicBlock* a, BPatch_basicBlock* b) {
                return a->getStartAddress() < b->getStartAddress();
              });
    for(BPatch_basicBlock* bb : ordered) {
      vector<std::pair<InstructionAPI::Instruction, Address>> insns;
      if(!bb->getInstructions(insns))
        continue;
      for(auto& ia : insns) {
        if(instrumented >= MAX_WRITES)
          break;
        bool pick = (considered % WRITE_STRIDE == 0);   // deterministic selection
        considered++;
        if(!pick)
          continue;
        Address addr = ia.second;
        BPatch_point* pt = kernel->findPoint(addr);
        if(!pt)
          continue;
        BPatch_Vector<BPatch_snippet*> noArgs;
        BPatch_funcCallExpr call(*fnWrite, noArgs);
        if(appBin->insertSnippet(call, *pt, BPatch_callBefore, BPatch_lastSnippet)) {
          cout << "Inserting hc_write before instruction @ 0x" << std::hex << addr
               << std::dec << endl;
          instrumented++;
        }
      }
    }
  }
  cout << "hc_write inserted before " << instrumented << " of " << considered
       << " candidate instructions" << endl;

  /* Write the rewritten code object. */
  if(!appBin->writeFile(outBinary)) {
    cerr << "Failed to write output binary '" << outBinary << "'" << endl;
    return EXIT_FAILURE;
  }
  cout << "Wrote instrumented binary: " << outBinary << endl;
  return EXIT_SUCCESS;
}
