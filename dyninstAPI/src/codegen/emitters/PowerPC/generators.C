#include "arch-power.h"
#include "codegen/codegen-power.h"
#include "codegen/codegen.h"
#include "codegen/emitters/PowerPC/generators.h"
#include "debug.h"
#include "dyn_register.h"
#include "emitter.h"
#include "inst-power.h"
#include "opcode.h"
#include "RegisterConversion.h"

namespace {
  /* Pseudoregisters definitions */
  constexpr int POWER_XER2531 = 9999;

  void restoreGPRtoGPR(codeGen &gen, Dyninst::Register reg, Dyninst::Register dest);

  void restoreXERtoGPR(codeGen &gen, Dyninst::Register dest);

  void moveGPR2531toGPR(codeGen &gen, Dyninst::Register reg, Dyninst::Register dest);
}

namespace Dyninst { namespace DyninstAPI { namespace ppc {

  // VG(11/16/01): Emit code to add the original value of a register to
  // another. The original value may need to be restored from stack...
  // VG(03/15/02): Made functionality more obvious by adding the above functions
  void emitAddOriginal(Dyninst::Register src, Dyninst::Register acc, codeGen &gen) {
    bool nr = ppc::needsRestore(src);
    Dyninst::Register temp;

    if(nr) {
      // this needs gen because it uses emitV...
      temp = gen.rs()->allocateRegister(gen);

      // Emit code to restore the original ra register value in temp.
      // The offset compensates for the gap 0, 3, 4, ...
      // This writes at insn, and updates insn and base.

      if(src == POWER_XER2531) { // hack for XER_25:31
        restoreXERtoGPR(gen, temp);
        moveGPR2531toGPR(gen, temp, temp);
      } else {
        restoreGPRtoGPR(gen, src, temp);
      }
    } else {
      temp = src;
    }

    // add temp to dest;
    // writes at gen+base and updates base, we must update insn...
    gen.emitter()->emitV(plusOp, temp, acc, acc, gen, 0);

    if(nr) {
      gen.rs()->freeRegister(temp);
    }
  }

  // VG(11/16/01): Say if we have to restore a register to get its original value
  bool needsRestore(Dyninst::Register x) {
    return ((x <= 12) && !(x == 2)) || (x == POWER_XER2531);
  }

  void saveRegisterAtOffset(codeGen &gen, Dyninst::Register reg, int save_off) {
    if(gen.width() == 4) {
      insnCodeGen::generateImm(gen, STop, reg, REG_SP, save_off);
    } else /* gen.width() == 8 */ {
      insnCodeGen::generateMemAccess64(gen, STDop, STDxop, reg, REG_SP, save_off);
    }
  }

  void restoreRegister(codeGen &gen, Dyninst::Register reg, int save_off) {
    ppc::restoreRegister(gen, reg, reg, save_off);
  }

  // Dest != reg : optimizate away a load/move pair
  void restoreRegister(codeGen &gen, Dyninst::Register source, Dyninst::Register dest,
                       int saved_off) {
    return ppc::restoreRegisterAtOffset(gen, dest, saved_off + (source * gen.width()));
  }

  void restoreRegisterAtOffset(codeGen &gen, Dyninst::Register dest, int saved_off) {
    if(gen.width() == 4) {
      insnCodeGen::generateImm(gen, Lop, dest, REG_SP, saved_off);
    } else /* gen.width() == 8 */ {
      insnCodeGen::generateMemAccess64(gen, LDop, LDxop, dest, REG_SP, saved_off);
    }
  }

  void saveRegister(codeGen &gen, Dyninst::Register reg, int save_off) {
    ppc::saveRegister(gen, reg, reg, save_off);
  }

  // Dest != reg : optimizate away a load/move pair
  void saveRegister(codeGen &gen, Dyninst::Register source, Dyninst::Register dest, int save_off) {
    ppc::saveRegisterAtOffset(gen, source, save_off + (dest * gen.width()));
  }

  ppc::parsed_regs calcUsedRegs(parse_func *func) {
    ppc::parsed_regs usedRegisters;
    using namespace Dyninst::InstructionAPI;
    std::set<RegisterAST::Ptr> writtenRegs;

    auto bl = func->blocks();
    auto curBlock = bl.begin();
    for(; curBlock != bl.end(); ++curBlock) {
      InstructionDecoder d(func->getPtrToInstruction((*curBlock)->start()), (*curBlock)->size(),
                           func->isrc()->getArch());
      Instruction i;
      unsigned size = 0;
      while(size < (*curBlock)->size()) {
        i = d.decode();
        size += i.size();
        i.getWriteSet(writtenRegs);
      }
    }

    for(auto const &reg : writtenRegs) {
      MachRegister r = reg->getID();
      auto regID = convertRegID(r);
      if(regID == registerSpace::ignored) {
        logLine("parse_func::calcUsedRegs: unknown written register\n");
        continue;
      }
      auto const category = r.regClass();

      // ppc{32,64}::{G,F}PR can be the same value, so avoid a -Wlogical-op
      // warning
      auto const is_gpr32 = (category == ppc32::GPR);
      auto const is_gpr64 = (category == ppc64::GPR);
      auto const is_gpr = (is_gpr32 || is_gpr64);

      auto const is_fpr32 = (category == ppc32::FPR);
      auto const is_fpr64 = (category == ppc64::FPR);
      auto const is_fpr = (is_fpr32 || is_fpr64);

      if(is_gpr) {
        usedRegisters.gprs.insert(regID);
      } else if(is_fpr) {
        usedRegisters.fprs.insert(regID);
      }
    }
    return usedRegisters;
  }

}}}

// clang-format off
namespace {

  // VG(03/15/02): Restore mutatee value of GPR reg to dest GPR
  void restoreGPRtoGPR(codeGen &gen, Dyninst::Register reg, Dyninst::Register dest) {
    int frame_size, gpr_size, gpr_off;
    if(gen.width() == 4) {
      frame_size = TRAMP_FRAME_SIZE_32;
      gpr_size = GPRSIZE_32;
      gpr_off = TRAMP_GPR_OFFSET_32;
    } else /* gen.width() == 8 */ {
      frame_size = TRAMP_FRAME_SIZE_64;
      gpr_size = GPRSIZE_64;
      gpr_off = TRAMP_GPR_OFFSET_64;
    }

    // SP is in a different place, but we don't need to
    // restore it, just subtract the stack frame size
    if(reg == 1) {
      insnCodeGen::generateImm(gen, CALop, dest, REG_SP, frame_size);
    }

    else if((reg == 0) || ((reg >= 3) && (reg <= 12))) {
      insnCodeGen::generateMemAccess64(gen, LDop, LDxop, dest, REG_SP, gpr_off + reg * gpr_size);
    }
    // Past code restoring 32bit's of register instead of entire register
    //        insnCodeGen::generateImm(gen, Lop, dest, REG_SP,
    //                                 gpr_off + reg*gpr_size);
    else {
      bperr("GPR %u should not be restored...", reg.getId());
      assert(0);
    }
  }

  // VG(03/15/02): Restore mutatee value of XER to dest GPR
  void restoreXERtoGPR(codeGen &gen, Dyninst::Register dest) {
    if(gen.width() == 4) {
      insnCodeGen::generateImm(gen, Lop, dest, REG_SP, TRAMP_SPR_OFFSET(4) + STK_XER_32);
    } else /* gen.width() == 8 */ {
      insnCodeGen::generateMemAccess64(gen, LDop, LDxop, dest, REG_SP,
                                       TRAMP_SPR_OFFSET(8) + STK_XER_64);
    }
  }

  // VG(03/15/02): Move bits 25:31 of GPR reg to GPR dest
  void moveGPR2531toGPR(codeGen &gen, Dyninst::Register reg, Dyninst::Register dest) {
    // keep only bits 32+25:32+31; extrdi dest, reg, 7 (n bits), 32+25 (start at b)
    // which is actually: rldicl dest, reg, 32+25+7 (b+n), 64-7 (64-n)
    // which is the same as: clrldi dest,reg,57 because 32+25+7 = 64
    using namespace NS_power;

    instruction rld;
    rld.clear();
    MDFORM_OP_SET(rld, RLDop);
    MDFORM_RS_SET(rld, reg);
    MDFORM_RA_SET(rld, dest);
    MDFORM_SH_SET(rld, 0); //(32+25+7) % 32;
    MDFORM_MB_SET(rld, (64 - 7) % 32);
    MDFORM_MB2_SET(rld, (64 - 7) / 32);
    MDFORM_XO_SET(rld, ICLxop);
    MDFORM_SH2_SET(rld, 0); //(32+25+7) / 32;
    MDFORM_RC_SET(rld, 0);
    insnCodeGen::generate(gen, rld);
  }

}

// clang-format on
