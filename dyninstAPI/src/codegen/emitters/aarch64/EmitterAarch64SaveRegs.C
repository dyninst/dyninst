#include "codegen/codegen-aarch64.h"
#include "EmitterAarch64SaveRegs.h"
#include "inst-aarch64.h"
#include "registerSpace/RegisterConversion.h"

#include <map>

namespace Dyninst { namespace DyninstAPI {

  void EmitterAarch64SaveRegs::saveSPR(codeGen &gen, Register scratchReg, int sprnum,
                                       int stkOffset) {
    assert(scratchReg != Null_Register);

    // TODO move map to common location
    std::map<int, int> sysRegCodeMap = {{SPR_NZCV, 0x5A10}, {SPR_FPCR, 0x5A20}, {SPR_FPSR, 0x5A21}};
    if(!sysRegCodeMap.count(sprnum)) {
      assert(!"Invalid/unknown system register passed to saveSPR()!");
    }

    instruction insn;
    insn.clear();

    // Set opcode for MRS instruction
    INSN_SET(insn, 20, 31, MRSOp);
    // Set destination register
    INSN_SET(insn, 0, 4, scratchReg & 0x1F);
    // Set bits representing source system register
    INSN_SET(insn, 5, 19, sysRegCodeMap[sprnum]);
    insnCodeGen::generate(gen, insn);

    insnCodeGen::generateMemAccess(gen, insnCodeGen::Store, scratchReg, REG_SP, stkOffset, 4,
                                   insnCodeGen::Pre);
  }

  void EmitterAarch64SaveRegs::saveFPRegister(codeGen &gen, Register reg, int save_off) {
    // Always performing save of the full FP register
    insnCodeGen::generateMemAccessFP(gen, insnCodeGen::Store, reg, REG_SP, save_off, 0, true);
  }

  unsigned EmitterAarch64SaveRegs::saveGPRegisters(codeGen &gen, registerSpace *theRegSpace,
                                                   int offset, int numReqGPRs) {
    int ret = 0;
    if(numReqGPRs == -1) {
      numReqGPRs = theRegSpace->numGPRs();
    }

    for(int idx = 0; idx < numReqGPRs; idx++) {
      registerSlot *reg = theRegSpace->GPRs()[idx];
      // We always save FP and LR for stack walking out of instrumentation
      if(reg->liveState == registerSlot::live || reg->number == REG_FP || reg->number == REG_LR) {
        int offset_from_sp = offset + (reg->encoding() * gen.width());
        insnCodeGen::saveRegister(gen, reg->number, offset_from_sp);
        theRegSpace->markSavedRegister(reg->number, offset_from_sp);
        ret++;
      }
    }

    return ret;
  }

  unsigned EmitterAarch64SaveRegs::saveFPRegisters(codeGen &gen, registerSpace *theRegSpace,
                                                   int offset) {
    unsigned ret = 0;

    for(int idx = 0; idx < theRegSpace->numFPRs(); idx++) {
      registerSlot *reg = theRegSpace->FPRs()[idx];

      // if(reg->liveState == registerSlot::live) {
      int offset_from_sp = offset + (reg->encoding() * FPRSIZE_64);
      saveFPRegister(gen, reg->number, offset_from_sp);
      // reg->liveState = registerSlot::spilled;
      theRegSpace->markSavedRegister(reg->number, offset_from_sp);
      ret++;
      //}
    }

    return ret;
  }

  unsigned EmitterAarch64SaveRegs::saveSPRegisters(codeGen &gen, registerSpace *theRegSpace,
                                                   int offset, bool force_save) {
    int ret = 0;

    std::vector<registerSlot *> spRegs;
    std::map<registerSlot *, int> regMap;

    registerSlot *regNzcv = (*theRegSpace)[registerSpace::nzcv];
    assert(regNzcv);
    regMap[regNzcv] = SPR_NZCV;
    if(force_save || regNzcv->liveState == registerSlot::live) {
      spRegs.push_back(regNzcv);
    }

    registerSlot *regFpcr = (*theRegSpace)[registerSpace::fpcr];
    assert(regFpcr);
    regMap[regFpcr] = SPR_FPCR;
    if(force_save || regFpcr->liveState == registerSlot::live) {
      spRegs.push_back(regFpcr);
    }

    registerSlot *regFpsr = (*theRegSpace)[registerSpace::fpsr];
    assert(regFpsr);
    regMap[regFpsr] = SPR_FPSR;
    if(force_save || regFpsr->liveState == registerSlot::live) {
      spRegs.push_back(regFpsr);
    }

    for(std::vector<registerSlot *>::iterator itr = spRegs.begin(); itr != spRegs.end(); itr++) {
      registerSlot *cur = *itr;
      saveSPR(gen, theRegSpace->getScratchRegister(gen), regMap[cur], -4 * GPRSIZE_32);
      theRegSpace->markSavedRegister(cur->number, offset);

      offset += 4 * GPRSIZE_32;
      ret++;
    }

    return ret;
  }

}}
