#include "codegen/codegen-aarch64.h"
#include "EmitterAarch64RestoreRegs.h"
#include "inst-aarch64.h"
#include "registerSpace/RegisterConversion.h"

#include <map>

namespace Dyninst { namespace DyninstAPI {

  void EmitterAarch64RestoreRegs::restoreFPRegister(codeGen &gen, Register reg, int save_off) {
    insnCodeGen::generateMemAccessFP(gen, insnCodeGen::Load, reg, REG_SP, save_off, 0, true);
  }

  unsigned EmitterAarch64RestoreRegs::restoreFPRegisters(codeGen &gen, registerSpace *theRegSpace,
                                                         int offset) {
    unsigned ret = 0;

    for(int idx = theRegSpace->numFPRs() - 1; idx >= 0; idx--) {
      registerSlot *reg = theRegSpace->FPRs()[idx];

      // if(reg->liveState == registerSlot::spilled) {
      int offset_from_sp = offset + (reg->encoding() * FPRSIZE_64);
      restoreFPRegister(gen, reg->number, offset_from_sp);
      ret++;
      //}
    }

    return ret;
  }

  unsigned EmitterAarch64RestoreRegs::restoreGPRegisters(codeGen &gen, registerSpace *theRegSpace,
                                                         int offset) {
    unsigned ret = 0;

    for(int idx = theRegSpace->numGPRs() - 1; idx >= 0; idx--) {
      registerSlot *reg = theRegSpace->GPRs()[idx];

      if(reg->liveState == registerSlot::spilled) {
        // #sasha this should be GPRSIZE_64 and not gen.width
        int offset_from_sp = offset + (reg->encoding() * gen.width());
        insnCodeGen::restoreRegister(gen, reg->number, offset_from_sp);
        ret++;
      }
    }

    return ret;
  }

  void EmitterAarch64RestoreRegs::restoreSPR(codeGen &gen, Register scratchReg, int sprnum,
                                             int stkOffset) {
    insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, scratchReg, REG_SP, stkOffset, 4);

    // TODO move map to common location
    std::map<int, int> sysRegCodeMap = {{SPR_NZCV, 0x5A10}, {SPR_FPCR, 0x5A20}, {SPR_FPSR, 0x5A21}};
    if(!sysRegCodeMap.count(sprnum)) {
      assert(!"Invalid/unknown system register passed to restoreSPR()!");
    }

    instruction insn;
    insn.clear();

    // Set opcode for MSR (register) instruction
    INSN_SET(insn, 20, 31, MSROp);
    // Set source register
    INSN_SET(insn, 0, 4, scratchReg & 0x1F);
    // Set bits representing destination system register
    INSN_SET(insn, 5, 19, sysRegCodeMap[sprnum]);
    insnCodeGen::generate(gen, insn);
  }

  unsigned EmitterAarch64RestoreRegs::restoreSPRegisters(codeGen &gen, registerSpace *theRegSpace,
                                                         int, int force_save) {
    int ret = 0;

    std::vector<registerSlot *> spRegs;
    std::map<registerSlot *, int> regMap;

    registerSlot *regFpsr = (*theRegSpace)[registerSpace::fpsr];
    assert(regFpsr);
    regMap[regFpsr] = SPR_FPSR;
    if(force_save || regFpsr->liveState == registerSlot::spilled) {
      spRegs.push_back(regFpsr);
    }

    registerSlot *regFpcr = (*theRegSpace)[registerSpace::fpcr];
    assert(regFpcr);
    regMap[regFpcr] = SPR_FPCR;
    if(force_save || regFpcr->liveState == registerSlot::spilled) {
      spRegs.push_back(regFpcr);
    }

    registerSlot *regNzcv = (*theRegSpace)[registerSpace::nzcv];
    assert(regNzcv);
    regMap[regNzcv] = SPR_NZCV;
    if(force_save || regNzcv->liveState == registerSlot::spilled) {
      spRegs.push_back(regNzcv);
    }

    for(std::vector<registerSlot *>::iterator itr = spRegs.begin(); itr != spRegs.end(); itr++) {
      registerSlot *cur = *itr;
      restoreSPR(gen, theRegSpace->getScratchRegister(gen), regMap[cur], 4 * GPRSIZE_32);
      ret++;
    }

    return ret;
  }

}}
