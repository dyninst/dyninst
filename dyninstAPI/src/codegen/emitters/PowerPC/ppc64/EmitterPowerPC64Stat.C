#include "codegen/codegen-power.h"
#include "codegen/emitters/PowerPC/generators.h"
#include "codegen/emitters/PowerPC/ppc64/EmitterPowerPC64Stat.h"
#include "inst-power.h"

namespace Dyninst { namespace DyninstAPI {

  bool EmitterPowerPC64Stat::emitCallInstruction(codeGen &gen, func_instance *callee, bool,
                                                 Address) {
    // if the TOC changes, generate a PIC call
    Address dest = callee->addr();
    if(dest == 0) {
      dest = getInterModuleFuncAddr(callee, gen);
    }

    if(gen.func()->obj() != callee->obj()) {
      return emitPLTCall(callee, gen);
    }
    // For local calls, we should not need to set R2.
    //
    // If the callee has a preamble to set up R2, we skip these two instructions.
    // For PIE code, the preamble uses R12 to set up R2; calling the global entry
    // will require setting R12 to be the global entry of the callee. So, we just
    // skip the preabmle.
    //
    // TODO: if the premable changes, the amount of bytes to skip should change as well.
    //
    if(callee->ifunc()->containsPowerPreamble()) {
      insnCodeGen::generateCall(gen, gen.currAddr(), dest + 8);
    } else {
      // For functions that do not have the R2 preabmle,
      // it means it will not change R2, we just call it.
      insnCodeGen::generateCall(gen, gen.currAddr(), dest);
    }
    return true;
  }

  Register EmitterPowerPC64Stat::emitCallReplacement(opCode, codeGen &, func_instance *) {
    assert(0 && "emitCallReplacement not implemented for binary rewriter");
    return {};
  }

  // TODO 32/64-bit?
  bool EmitterPowerPC64Stat::emitPLTCall(func_instance *callee, codeGen &gen) {
    return emitPLTCommon(callee, true, gen);
  }

  bool EmitterPowerPC64Stat::emitPLTCommon(func_instance *callee, bool call, codeGen &gen) {
    /* This function generates code to call/jump to an external function.
     * The Power ABI v2 specifies that R12 should contain the callee address.
     *
     * The steps include:
     * 1. Save registers
     * 2. Create/Load the PLT entry for the callee
     * 3. Make the call
     * 4. Restore registers
     * 5. Generate a return for jump case
     */

    const unsigned TOCreg = 2;
    const unsigned wordsize = gen.width();
    assert(wordsize == 8);

    Address func_desc = getInterModuleFuncAddr(callee, gen);

    // We need a scratch register and a place to store the LR.
    // Because modification can also call this function, there may not
    // be an instrumentation frame. So, we move down the stack before the
    // call and move up the stack after the call
    ppc::pushStack(gen);

    unsigned r_tmp = 12; // R12 ; We need to put callee address into R12

    // Save R12
    insnCodeGen::generateMemAccess64(gen, STDop, STDxop, r_tmp, REG_SP, 3 * wordsize);

    // Save LR
    insnCodeGen::generateMoveFromLR(gen, r_tmp);
    insnCodeGen::generateMemAccess64(gen, STDop, STDxop, r_tmp, REG_SP, 4 * wordsize);

    // Save R2
    insnCodeGen::generateMemAccess64(gen, STDop, STDxop, TOCreg, REG_SP, 5 * wordsize);

    // r_tmp := func_desc
    // We first load PC into R12
    Address pcVal = emitMovePCToReg(r_tmp, gen);

    Address func_desc_from_cur = func_desc - pcVal;

    // Here we use R2 as a temp register:
    // We load the offset into R2
    insnCodeGen::loadImmIntoReg(gen, TOCreg, func_desc_from_cur);

    // Add the offset to R12, which contains the PC
    insnCodeGen::generateAddReg(gen, CAXop, r_tmp, r_tmp, TOCreg);

    // r_tmp := *(r_tmp)
    insnCodeGen::generateMemAccess64(gen, LDop, LDxop, r_tmp, r_tmp, 0);

    // lr := r_tmp; Loading the call target
    insnCodeGen::generateMoveToLR(gen, r_tmp);

    // blrl
    instruction branch_insn(BRLraw);
    insnCodeGen::generate(gen, branch_insn);

    // Restore LR, R2, and R12
    insnCodeGen::generateMemAccess64(gen, LDop, LDxop, r_tmp, REG_SP, 4 * wordsize);
    insnCodeGen::generateMoveToLR(gen, r_tmp);
    insnCodeGen::generateMemAccess64(gen, LDop, LDxop, r_tmp, REG_SP, 3 * wordsize);
    insnCodeGen::generateMemAccess64(gen, LDop, LDxop, TOCreg, REG_SP, 5 * wordsize);

    // Move back the stack
    ppc::popStack(gen);

    if(!call) {
      // We genearte a return here for jump case,
      // because Dyninst will relocate the original function.
      // We do not want to execute the original code
      // in cases like function replacement and function wrapping.
      instruction ret(BRraw);
      insnCodeGen::generate(gen, ret);
    }

    return true;
  }

  bool EmitterPowerPC64Stat::emitPLTJump(func_instance *callee, codeGen &gen) {
    return emitPLTCommon(callee, false, gen);
  }

  bool EmitterPowerPC64Stat::emitTOCCall(block_instance *block, codeGen &gen) {
    return emitTOCCommon(block, true, gen);
  }

  bool EmitterPowerPC64Stat::emitTOCCommon(block_instance *block, bool call, codeGen &gen) {
    // Right now we can only jump to a block if it's the entry of a function
    // since it needs a relocation entry, which implies symbols, which implies... a
    // function. Theoretically, we could create symbols for this block, if anyone ever
    // cares.
    std::vector<func_instance *> funcs;
    block->getFuncs(std::back_inserter(funcs));
    for(unsigned i = 0; i < funcs.size(); ++i) {
      if(block == funcs[i]->entry()) {
        return emitPLTCommon(funcs[i], call, gen);
      }
    }
    return false;
  }

  bool EmitterPowerPC64Stat::emitTOCJump(block_instance *block, codeGen &gen) {
    return emitTOCCommon(block, false, gen);
  }

}}
