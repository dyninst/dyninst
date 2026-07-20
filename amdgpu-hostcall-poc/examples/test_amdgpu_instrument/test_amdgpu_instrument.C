/*
 *  AMDGPU hostcall instrumentation TEST HARNESS (DyninstAPI static binary rewrite).
 *
 *  Loads the device hostcall library into a GPU code object and inserts GPU->CPU
 *  hostcalls around/into a target kernel:
 *    - hc_open  at kernel ENTRY (open the trace file)
 *    - hc_close at kernel EXIT  (close it)
 *    - a per-site hc_write_* before every WRITE_STRIDE-th instruction (cap MAX_WRITES)
 *
 *  This binary is a CAPABILITY DEMONSTRATOR: env vars select which inserted hook /
 *  argument to exercise (each corresponds to a feature brought up in the AMDGPU
 *  emitter). Inserted calls DO pass arguments now — constants, computed ASTs,
 *  GPU-value reads, and implicit ABI args are all supported:
 *    (default)       hc_write_id(site)            leaf hook, constant arg
 *    HC_ARG_EXPR     hc_write_id(site*100)        computed arg (generate-then-move)
 *    HC_HWID/HC_EXEC hc_write_id(<gpu value>)     site-read GPU value (HW_ID / EXEC mask)
 *    HC_NONLEAF      hc_write_nl                  non-leaf callee (own stack frame)
 *    HC_BLOCKIDX / HC_BLOCKDIM / HC_THREADIDX     hooks reading implicit ABI args
 *    HC_FTID         hc_write_ftid                hook computing the flattened wave id
 *  Each inserted external call also triggers the caller kernel-descriptor register
 *  bump (emit-amdgpu.C bumpCallerKdForCallee) so the callee's registers fit.
 *
 *  For a minimal, single-purpose usage of the API, see minimal_instrument.C.
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
  BPatch_function* fnOpen    = findFuncByName(appImage, "hc_open");
  BPatch_function* fnWrite   = findFuncByName(appImage, "hc_write");
  BPatch_function* fnWriteId = findFuncByName(appImage, "hc_write_id");
  BPatch_function* fnClose   = findFuncByName(appImage, "hc_close");
  if(!fnOpen || !fnWrite || !fnWriteId || !fnClose) {
    cerr << "Instrumentation library is missing hc_open/hc_write/hc_write_id/hc_close" << endl;
    return EXIT_FAILURE;
  }
  // Non-leaf call-ABI test (HC_NONLEAF): insert hc_write_nl (which calls a
  // non-inlined helper => has its own s[0:3]+s32 buffer frame) at the write
  // points, to exercise dyninst's inserted-call ABI for non-leaf callees.
  BPatch_function* fnWriteNl = getenv("HC_NONLEAF") ? findFuncByName(appImage, "hc_write_nl") : nullptr;
  if(getenv("HC_NONLEAF") && !fnWriteNl) {
    cerr << "HC_NONLEAF set but hc_write_nl not found" << endl;
    return EXIT_FAILURE;
  }
  // Implicit-arg (blockIdx) test (HC_BLOCKIDX): insert hc_write_bid, which reads
  // blockIdx.x (an implicit ABI arg in s12) — exercises Phase 3a implicit-arg
  // forwarding. Run the mutator with DYNINST_IMPLICIT_ARGS and the launcher with
  // multiple workgroups (HOSTCALL_WG) so blockIdx varies.
  BPatch_function* fnWriteBid = getenv("HC_BLOCKIDX") ? findFuncByName(appImage, "hc_write_bid") : nullptr;
  if(getenv("HC_BLOCKIDX") && !fnWriteBid) {
    cerr << "HC_BLOCKIDX set but hc_write_bid not found" << endl;
    return EXIT_FAILURE;
  }
  // Implicit-arg (blockDim/gridDim) test (HC_BLOCKDIM): insert hc_write_bdim, which
  // reads blockDim.x by dereferencing the implicit-args pointer (Phase 3b).
  BPatch_function* fnWriteBdim = getenv("HC_BLOCKDIM") ? findFuncByName(appImage, "hc_write_bdim") : nullptr;
  if(getenv("HC_BLOCKDIM") && !fnWriteBdim) {
    cerr << "HC_BLOCKDIM set but hc_write_bdim not found" << endl;
    return EXIT_FAILURE;
  }
  // Implicit-arg (threadIdx) test (HC_THREADIDX): insert hc_write_tid, which reads
  // threadIdx.x (per-lane packed workitem-id in v31) — exercises Phase 3c.
  BPatch_function* fnWriteTid = getenv("HC_THREADIDX") ? findFuncByName(appImage, "hc_write_tid") : nullptr;
  if(getenv("HC_THREADIDX") && !fnWriteTid) {
    cerr << "HC_THREADIDX set but hc_write_tid not found" << endl;
    return EXIT_FAILURE;
  }
  // Flattened-id test (HC_FTID): insert hc_write_ftid, an arbitrary HIP hook that
  // computes the flattened wave id from blockIdx/blockDim/threadIdx (all forwarded by
  // Phase 3) — the "arbitrary device-code hook using forwarded ABI context" payoff.
  BPatch_function* fnWriteFtid = getenv("HC_FTID") ? findFuncByName(appImage, "hc_write_ftid") : nullptr;
  if(getenv("HC_FTID") && !fnWriteFtid) {
    cerr << "HC_FTID set but hc_write_ftid not found" << endl;
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
        // Pass the site index as a scalar call argument. Default: a plain
        // BPatch_constExpr (the immediate fast path). With HC_ARG_EXPR set, pass a
        // computed BPatch_arithExpr (site*100) instead, to exercise the general
        // generate-then-move argument path (Tier 1): the trampoline evaluates the
        // AST into a scalar register and broadcasts it into the CC arg VGPR.
        BPatch_constExpr siteConst(instrumented);
        BPatch_constExpr hundred(100);
        BPatch_arithExpr siteExpr(BPatch_times, siteConst, hundred);
        // HC_HWID / HC_EXEC: pass a GPU-value snippet (read at the site) as the arg to
        // hc_write_id, so the trace records the HW_ID / EXEC mask — the user-crafted
        // GPU-value AST path (milestone 1).
        BPatch_gpuHwWaveIdExpr hwid;
        BPatch_gpuExecMaskExpr execmask;
        BPatch_snippet* arg =
            getenv("HC_HWID")     ? (BPatch_snippet*)&hwid
          : getenv("HC_EXEC")     ? (BPatch_snippet*)&execmask
          : getenv("HC_ARG_EXPR") ? (BPatch_snippet*)&siteExpr
                                  : (BPatch_snippet*)&siteConst;
        BPatch_Vector<BPatch_snippet*> args;
        args.push_back(arg);
        BPatch_function* wtarget = fnWriteFtid ? fnWriteFtid           // HC_FTID      -> flattened wave id
                                 : fnWriteTid  ? fnWriteTid            // HC_THREADIDX -> threadIdx reader
                                 : fnWriteBdim ? fnWriteBdim           // HC_BLOCKDIM  -> blockDim reader
                                 : fnWriteBid  ? fnWriteBid            // HC_BLOCKIDX  -> blockIdx reader
                                 : fnWriteNl   ? fnWriteNl             // HC_NONLEAF   -> non-leaf callee
                                 : fnWriteId;                          // default      -> leaf id logger
        BPatch_funcCallExpr call(*wtarget, args);
        if(appBin->insertSnippet(call, *pt, BPatch_callBefore, BPatch_lastSnippet)) {
          const char* wname = fnWriteFtid ? "hc_write_ftid" : fnWriteTid ? "hc_write_tid"
                            : fnWriteBdim ? "hc_write_bdim" : fnWriteBid ? "hc_write_bid"
                            : fnWriteNl   ? "hc_write_nl"   : "hc_write_id";
          const char* akind = getenv("HC_HWID") ? "hw_wave_id" : getenv("HC_EXEC") ? "exec_mask"
                            : getenv("HC_ARG_EXPR") ? "site*100" : "site";
          cout << "Inserting " << wname << "(arg=" << akind << ") @ site " << instrumented
               << " (0x" << std::hex << addr << std::dec << ")" << endl;
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
