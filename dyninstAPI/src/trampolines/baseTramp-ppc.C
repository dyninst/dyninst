#include "baseTramp-ppc.h"
#include "inst-power.h"
#include "debug.h"
#include "codegen/codegen.h"
#include "BPatch.h"
#include "codegen/emitters/PowerPC/generators.h"

namespace ppc = Dyninst::DyninstAPI::ppc;

bool baseTramp_ppc::generateSaves(codeGen &gen, registerSpace *) {
  regalloc_printf("========== baseTramp::generateSaves\n");
  unsigned int width = gen.width();

  int gpr_off, fpr_off;
  gpr_off = TRAMP_GPR_OFFSET(width);
  fpr_off = TRAMP_FPR_OFFSET(width);

  // Make a stack frame.
  ppc::pushStack(gen);

  // Save GPRs
  ppc::saveGPRegisters(gen, gen.rs(), gpr_off);

  if (BPatch::bpatch->isSaveFPROn() || // Save FPRs
      BPatch::bpatch->isForceSaveFPROn())
    ppc::saveFPRegisters(gen, gen.rs(), fpr_off);

  // Save LR
  ppc::saveLR(gen, REG_SCRATCH /* register to use */,
         TRAMP_SPR_OFFSET(width) + STK_LR);

  ppc::saveSPRegisters(gen, gen.rs(), TRAMP_SPR_OFFSET(width),
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
  ppc::restoreSPRegisters(gen, gen.rs(), TRAMP_SPR_OFFSET(width), false);

  // LR
  ppc::restoreLR(gen, REG_SCRATCH, TRAMP_SPR_OFFSET(width) + STK_LR);

  // FPRs
  if (BPatch::bpatch->isSaveFPROn() || BPatch::bpatch->isForceSaveFPROn())
    ppc::restoreFPRegisters(gen, gen.rs(), fpr_off);

  // GPRs
  ppc::restoreGPRegisters(gen, gen.rs(), gpr_off);

  Dyninst::DyninstAPI::ppc::popStack(gen);

  return true;
}
