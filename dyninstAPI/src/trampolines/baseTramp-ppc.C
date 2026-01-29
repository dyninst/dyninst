#include "baseTramp-ppc.h"
#include "inst-power.h"
#include "debug.h"
#include "codegen.h"
#include "BPatch.h"

bool baseTramp_ppc::generateSaves(codeGen &gen, registerSpace *) {
  regalloc_printf("========== baseTramp::generateSaves\n");
  unsigned int width = gen.width();

  int gpr_off, fpr_off;
  gpr_off = TRAMP_GPR_OFFSET(width);
  fpr_off = TRAMP_FPR_OFFSET(width);

  // Make a stack frame.
  pushStack(gen);

  // Save GPRs
  saveGPRegisters(gen, gen.rs(), gpr_off);

  if (BPatch::bpatch->isSaveFPROn() || // Save FPRs
      BPatch::bpatch->isForceSaveFPROn())
    saveFPRegisters(gen, gen.rs(), fpr_off);

  // Save LR
  saveLR(gen, REG_SCRATCH /* register to use */,
         TRAMP_SPR_OFFSET(width) + STK_LR);

  saveSPRegisters(gen, gen.rs(), TRAMP_SPR_OFFSET(width),
                  true); // FIXME get liveness fixed
  return true;
}

bool baseTramp_ppc::generateRestores(codeGen &gen, registerSpace *) {
  unsigned int width = gen.width();

  regalloc_printf("========== baseTramp::generateRestores\n");

  int gpr_off, fpr_off;
  gpr_off = TRAMP_GPR_OFFSET(width);
  fpr_off = TRAMP_FPR_OFFSET(width);

  // Restore possible SPR saves
  restoreSPRegisters(gen, gen.rs(), TRAMP_SPR_OFFSET(width), false);

  // LR
  restoreLR(gen, REG_SCRATCH, TRAMP_SPR_OFFSET(width) + STK_LR);

  // FPRs
  if (BPatch::bpatch->isSaveFPROn() || BPatch::bpatch->isForceSaveFPROn())
    restoreFPRegisters(gen, gen.rs(), fpr_off);

  // GPRs
  restoreGPRegisters(gen, gen.rs(), gpr_off);

  popStack(gen);

  return true;
}
