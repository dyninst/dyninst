#include "baseTramp-aarch64.h"
#include "BPatch.h"
#include "codegen-aarch64.h"
#include "codegen.h"
#include "debug.h"
#include "emit-aarch64.h"
#include "inst-aarch64.h"
#include "registerSpace.h"

bool baseTramp_aarch64::generateSaves(codeGen &gen, registerSpace *) {
  regalloc_printf("========== baseTramp::generateSaves\n");

  // Make a stack frame.
  pushStack(gen);

  EmitterAARCH64SaveRegs saveRegs;
  unsigned int width = gen.width();

  saveRegs.saveGPRegisters(gen, gen.rs(), TRAMP_GPR_OFFSET(width));
  // After saving GPR, we move SP to FP to create the instrumentation frame.
  // Note that Dyninst instrumentation frame has a different structure
  // compared to stack frame created by the compiler.
  //
  // Dyninst instrumentation frame makes sure that FP and SP are the same.
  // So, during stack walk, the FP retrived from the previous frame is
  // the SP of the current instrumentation frame.
  //
  // Note: If the implementation of the instrumentation frame layout
  // needs to be changed, DyninstDynamicStepperImpl::getCallerFrameArch
  // in stackwalk/src/aarch64-swk.C also likely needs to be changed accordingly
  insnCodeGen::generateMoveSP(gen, REG_SP, REG_FP, true);
  gen.markRegDefined(REG_FP);

  bool saveFPRs =
      BPatch::bpatch->isForceSaveFPROn() ||
      (BPatch::bpatch->isSaveFPROn() && gen.rs()->anyLiveFPRsAtEntry() && this->saveFPRs());

  if(saveFPRs) {
    saveRegs.saveFPRegisters(gen, gen.rs(), TRAMP_FPR_OFFSET(width));
  }
  this->savedFPRs = saveFPRs;

  saveRegs.saveSPRegisters(gen, gen.rs(), TRAMP_SPR_OFFSET(width), false);
  // gen.rs()->debugPrint();

  return true;
}

bool baseTramp_aarch64::generateRestores(codeGen &gen, registerSpace *) {
  EmitterAARCH64RestoreRegs restoreRegs;
  unsigned int width = gen.width();

  restoreRegs.restoreSPRegisters(gen, gen.rs(), TRAMP_SPR_OFFSET(width), false);

  if(this->savedFPRs) {
    restoreRegs.restoreFPRegisters(gen, gen.rs(), TRAMP_FPR_OFFSET(width));
  }

  restoreRegs.restoreGPRegisters(gen, gen.rs(), TRAMP_GPR_OFFSET(width));

  // Tear down the stack frame.
  popStack(gen);

  return true;
}
