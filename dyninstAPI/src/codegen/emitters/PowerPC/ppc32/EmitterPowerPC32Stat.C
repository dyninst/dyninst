#include "arch-power.h"
#include "codegen/codegen-power.h"
#include "codegen/emitters/PowerPC/generators.h"
#include "codegen/emitters/PowerPC/ppc32/EmitterPowerPC32Stat.h"
#include "debug.h"
#include "inst-power.h"
#include "RegisterConversion.h"
#include "registerSpace/registerSpace.h"

namespace Dyninst { namespace DyninstAPI {

  // It seems like we should be able to do a better job...
  bool EmitterPowerPC32Stat::emitCallInstruction(codeGen &gen, func_instance *callee,
                                                 bool /* setTOC */, Address) {
    // 32 - No TOC
    // if inter module, gen PIC code
    if(gen.func()->obj() != callee->obj()) {
      return emitPLTCall(callee, gen);
    }
    insnCodeGen::generateCall(gen, gen.currAddr(), callee->addr());
    return true;
  }

  Register EmitterPowerPC32Stat::emitCallReplacement(opCode, codeGen &, func_instance *) {
    assert(0 && "emitCallReplacement not implemented for binary rewriter");
    return {};
  }

  // TODO 32/64-bit?
  bool EmitterPowerPC32Stat::emitPLTCall(func_instance *callee, codeGen &gen) {
    return emitPLTCommon(callee, true, gen);
  }

  bool EmitterPowerPC32Stat::emitPLTCommon(func_instance *callee, bool call, codeGen &gen) {
    Dyninst::Register scratchReg = gen.rs()->getScratchRegister(gen);
    if(scratchReg == Null_Register) {
      return false;
    }

    Dyninst::Register scratchLR = Null_Register;
    std::vector<Dyninst::Register> excluded;
    excluded.push_back(scratchReg);
    scratchLR = gen.rs()->getScratchRegister(gen, excluded);
    if(scratchLR == Null_Register) {
      if(scratchReg == registerSpace::r0) {
        return false;
      }
      // We can use r0 for this, since it's volatile.
      scratchLR = registerSpace::r0;
    }

    if(!call) {
      // Save the LR in scratchLR
      insnCodeGen::generateMoveFromLR(gen, scratchLR);
    }

    // Generate the PLT call

    Address dest = getInterModuleFuncAddr(callee, gen);
    Address pcVal = emitMovePCToReg(scratchReg, gen);

    if(!call) {
      insnCodeGen::generateMoveToLR(gen, scratchLR);
    }

    // We can now use scratchLR

    Address varOffset = dest - pcVal;
    emitLoadRelative(scratchLR, varOffset, scratchReg, gen.width(), gen);

    insnCodeGen::generateMoveToCR(gen, scratchLR);

    if(!call) {
      instruction br(BCTRraw);
      insnCodeGen::generate(gen, br);
    } else {
      instruction brl(BCTRLraw);
      insnCodeGen::generate(gen, brl);
    }

    return true;
  }

  bool EmitterPowerPC32Stat::emitPLTJump(func_instance *callee, codeGen &gen) {
    return emitPLTCommon(callee, false, gen);
  }

  bool EmitterPowerPC32Stat::emitTOCCall(block_instance *block, codeGen &gen) {
    return emitTOCCommon(block, true, gen);
  }

  bool EmitterPowerPC32Stat::emitTOCCommon(block_instance *block, bool call, codeGen &gen) {
    Dyninst::Register scratchReg = gen.rs()->getScratchRegister(gen);
    if(scratchReg == Null_Register) {
      return false;
    }

    Dyninst::Register scratchLR = Null_Register;
    std::vector<Dyninst::Register> excluded;
    excluded.push_back(scratchReg);
    scratchLR = gen.rs()->getScratchRegister(gen, excluded);
    if(scratchLR == Null_Register) {
      if(scratchReg == registerSpace::r0) {
        return false;
      }
      // We can use r0 for this, since it's volatile.
      scratchReg = registerSpace::r0;
    }

    if(!call) {
      // Save the LR in scratchLR
      insnCodeGen::generateMoveFromLR(gen, scratchLR);
    }

    // Generate the PLT call

    Address dest = block->llb()->start();
    Address pcVal = emitMovePCToReg(scratchReg, gen);

    if(!call) {
      insnCodeGen::generateMoveToLR(gen, scratchLR);
    }

    // We can now use scratchLR

    Address varOffset = dest - pcVal;
    emitLoadRelative(scratchLR, varOffset, scratchReg, gen.width(), gen);

    insnCodeGen::generateMoveToCR(gen, scratchLR);

    if(!call) {
      instruction br(BCTRraw);
      insnCodeGen::generate(gen, br);
    } else {
      instruction brl(BCTRLraw);
      insnCodeGen::generate(gen, brl);
    }

    return true;
  }

  bool EmitterPowerPC32Stat::emitTOCJump(block_instance *block, codeGen &gen) {
    return emitTOCCommon(block, false, gen);
  }

}}
