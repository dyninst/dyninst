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

/*
 * Saving and restoring registers
 * We create a new stack frame in the base tramp and save registers
 * above it. Currently, the plan is this:
 *          < 220 bytes as per system spec      > + 4 for 64-bit alignment
 *          < 14 GPR slots @ 4 bytes each       >
 *          < 14 FPR slots @ 8 bytes each       >
 *          < 6 SPR slots @ 4 bytes each        >
 *          < 1 FP SPR slot @ 8 bytes           >
 *          < Space to save live regs at func call >
 *          < Func call overflow area, 32 bytes >
 *          < Linkage area, 24 bytes            >
 *
 * Of course, change all the 4's to 8's for 64-bit mode.
 */

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

  /*
   * Save a special purpose register onto the stack
   *
   *  NOTE:
   *    The bit layout of the mfspr instruction is as follows:
   *
   *      opcode:6 ; RT: 5 ; SPR: 10 ; const 339:10 ; Rc: 1
   *
   *  However, the two 5-bit halves of the SPR field are reversed so just using the xfx
   *  form will not work.
   */
  void saveSPR(codeGen &gen, Dyninst::Register scratchReg, int sprnum, int stkOffset) {
    instruction insn;

    // mfspr:  mflr scratchReg
    insn.clear();
    XFORM_OP_SET(insn, EXTop);
    XFORM_RT_SET(insn, scratchReg);
    XFORM_RA_SET(insn, sprnum & 0x1f);
    XFORM_RB_SET(insn, (sprnum >> 5) & 0x1f);
    XFORM_XO_SET(insn, MFSPRxop);
    insnCodeGen::generate(gen, insn);

    if(gen.width() == 4) {
      insnCodeGen::generateImm(gen, STop, scratchReg, REG_SP, stkOffset);
    } else /* gen.width() == 8 */ {
      insnCodeGen::generateMemAccess64(gen, STDop, STDxop, scratchReg, REG_SP, stkOffset);
    }
  }

  void popStack(codeGen &gen) {
    if(gen.width() == 4) {
      insnCodeGen::generateImm(gen, CALop, REG_SP, REG_SP, TRAMP_FRAME_SIZE_32);
    } else /* gen.width() == 8 */ {
      insnCodeGen::generateImm(gen, CALop, REG_SP, REG_SP, TRAMP_FRAME_SIZE_64);
    }
  }

  void pushStack(codeGen &gen) {
    if(gen.width() == 4) {
      insnCodeGen::generateImm(gen, STUop, REG_SP, REG_SP, -TRAMP_FRAME_SIZE_32);
    } else /* gen.width() == 8 */ {
      insnCodeGen::generateMemAccess64(gen, STDop, STDUxop, REG_SP, REG_SP, -TRAMP_FRAME_SIZE_64);
    }
  }

  ////////////////////////////////////////////////////////////////////
  // Generates instructions to restore a special purpose register from
  // the stack.
  //   Returns the number of bytes needed to store the generated
  //     instructions.
  //   The instruction storage pointer is advanced the number of
  //     instructions generated.
  //
  void restoreSPR(codeGen &gen,                 // Instruction storage pointer
                  Dyninst::Register scratchReg, // Scratch register
                  int sprnum,                   // SPR number
                  int stkOffset)                // Offset from stack pointer
  {
    if(gen.width() == 4) {
      insnCodeGen::generateImm(gen, Lop, scratchReg, REG_SP, stkOffset);
    } else /* gen.width() == 8 */ {
      insnCodeGen::generateMemAccess64(gen, LDop, LDxop, scratchReg, REG_SP, stkOffset);
    }

    instruction insn;
    insn.clear();

    // mtspr:  mtlr scratchReg
    XFORM_OP_SET(insn, EXTop);
    XFORM_RT_SET(insn, scratchReg);
    XFORM_RA_SET(insn, sprnum & 0x1f);
    XFORM_RB_SET(insn, (sprnum >> 5) & 0x1f);
    XFORM_XO_SET(insn, MTSPRxop);
    insnCodeGen::generate(gen, insn);
  }

  ////////////////////////////////////////////////////////////////////
  // Generates instructions to save link register onto stack.
  //   Returns the number of bytes needed to store the generated
  //     instructions.
  //   The instruction storage pointer is advanced the number of
  //     instructions generated.
  void saveLR(codeGen &gen,                 // Instruction storage pointer
              Dyninst::Register scratchReg, // Scratch register
              int stkOffset)                // Offset from stack pointer
  {
    saveSPR(gen, scratchReg, SPR_LR, stkOffset);
    gen.rs()->markSavedRegister(registerSpace::lr, stkOffset);
  }

  ////////////////////////////////////////////////////////////////////
  // Generates instructions to restore link register from stack.
  //   Returns the number of bytes needed to store the generated
  //    instructions.
  //  The instruction storage pointer is advanced the number of
  //    instructions generated.
  //
  void restoreLR(codeGen &gen,                 // Instruction storage pointer
                 Dyninst::Register scratchReg, // Scratch register
                 int stkOffset)                // Offset from stack pointer
  {
    restoreSPR(gen, scratchReg, SPR_LR, stkOffset);
  }

  /////////////////////////////////////////////////////////////////////////
  // Generates instructions to save the condition codes register onto stack.
  //   Returns the number of bytes needed to store the generated
  //     instructions.
  //   The instruction storage pointer is advanced the number of
  //     instructions generated.
  //
  void saveCR(codeGen &gen,                 // Instruction storage pointer
              Dyninst::Register scratchReg, // Scratch register
              int stkOffset)                // Offset from stack pointer
  {
    instruction insn;

    // mfcr:  mflr scratchReg
    insn.clear();
    XFXFORM_OP_SET(insn, EXTop);
    XFXFORM_RT_SET(insn, scratchReg);
    XFXFORM_XO_SET(insn, MFCRxop);
    insnCodeGen::generate(gen, insn);

    if(gen.width() == 4) {
      insnCodeGen::generateImm(gen, STop, scratchReg, REG_SP, stkOffset);
    } else /* gen.width() == 8 */ {
      insnCodeGen::generateMemAccess64(gen, STDop, STDxop, scratchReg, REG_SP, stkOffset);
    }
  }

  ///////////////////////////////////////////////////////////////////////////
  // Generates instructions to restore the condition codes register from stack.
  //   Returns the number of bytes needed to store the generated
  //     instructions.
  //   The instruction storage pointer is advanced the number of
  //     instructions generated.
  //
  void restoreCR(codeGen &gen,                 // Instruction storage pointer
                 Dyninst::Register scratchReg, // Scratch register
                 int stkOffset)                // Offset from stack pointer
  {
    instruction insn;

    if(gen.width() == 4) {
      insnCodeGen::generateImm(gen, Lop, scratchReg, REG_SP, stkOffset);
    } else /* gen.width() == 8 */ {
      insnCodeGen::generateMemAccess64(gen, LDop, LDxop, scratchReg, REG_SP, stkOffset);
    }

    // mtcrf:  scratchReg
    insn.clear();
    XFXFORM_OP_SET(insn, EXTop);
    XFXFORM_RT_SET(insn, scratchReg);
    XFXFORM_SPR_SET(insn, 0xff << 1);
    XFXFORM_XO_SET(insn, MTCRFxop);
    insnCodeGen::generate(gen, insn);
  }

  /////////////////////////////////////////////////////////////////////////
  // Generates instructions to save the floating point status and control
  // register on the stack.
  //   Returns the number of bytes needed to store the generated
  //     instructions.
  //   The instruction storage pointer is advanced the number of
  //     instructions generated.
  //
  void saveFPSCR(codeGen &gen,                 // Instruction storage pointer
                 Dyninst::Register scratchReg, // Scratch fp register
                 int stkOffset)                // Offset from stack pointer
  {
    instruction mffs;

    // mffs scratchReg
    mffs.clear();
    XFORM_OP_SET(mffs, X_FP_EXTENDEDop);
    XFORM_RT_SET(mffs, scratchReg);
    XFORM_XO_SET(mffs, MFFSxop);
    insnCodeGen::generate(gen, mffs);

    // st:     st scratchReg, stkOffset(r1)
    insnCodeGen::generateImm(gen, STFDop, scratchReg, REG_SP, stkOffset);
  }

  ///////////////////////////////////////////////////////////////////////////
  // Generates instructions to restore the floating point status and control
  // register from the stack.
  //   Returns the number of bytes needed to store the generated
  //     instructions.
  //   The instruction storage pointer is advanced the number of
  //     instructions generated.
  //
  void restoreFPSCR(codeGen &gen,                 // Instruction storage pointer
                    Dyninst::Register scratchReg, // Scratch fp register
                    int stkOffset)                // Offset from stack pointer
  {
    insnCodeGen::generateImm(gen, LFDop, scratchReg, REG_SP, stkOffset);

    instruction mtfsf;

    // mtfsf:  scratchReg
    mtfsf.clear();
    XFLFORM_OP_SET(mtfsf, X_FP_EXTENDEDop);
    XFLFORM_FLM_SET(mtfsf, 0xff);
    XFLFORM_FRB_SET(mtfsf, scratchReg);
    XFLFORM_XO_SET(mtfsf, MTFSFxop);
    insnCodeGen::generate(gen, mtfsf);
  }

  //////////////////////////////////////////////////////////////////////////
  // Writes out a `br' instruction
  //
  void resetBR(AddressSpace *p, // Process to write instruction into
               Address loc)     // Address in process to write into
  {
    instruction i = BRraw;
    if(!p->writeDataSpace((void *)loc, instruction::size(), i.ptr())) {
      fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
    }
  }

  void saveFPRegister(codeGen &gen, Dyninst::Register reg, int save_off) {
    assert("WE SHOULD NOT BE HERE" == 0);

    insnCodeGen::generateImm(gen, STFDop, reg, REG_SP, save_off + reg * FPRSIZE);
  }

  void restoreFPRegister(codeGen &gen, Dyninst::Register source, Dyninst::Register dest,
                         int save_off) {
    assert("WE SHOULD NOT BE HERE" == 0);

    insnCodeGen::generateImm(gen, LFDop, dest, REG_SP, save_off + source * FPRSIZE);
  }

  void restoreFPRegister(codeGen &gen, Dyninst::Register reg, int save_off) {
    restoreFPRegister(gen, reg, reg, save_off);
  }

  /*
   * Save necessary registers on the stack
   * insn, base: for code generation. Offset: regs saved at offset + reg
   * Returns: number of registers saved.
   * Side effects: instruction pointer and base param are shifted to
   *   next free slot.
   */
  unsigned saveGPRegisters(codeGen &gen, registerSpace *theRegSpace, int save_off, int numReqGPRs) {
    int numRegs = 0;
    if(numReqGPRs == -1) {
      numReqGPRs = theRegSpace->numGPRs();
    }
    for(int i = 0; i < theRegSpace->numGPRs(); i++) {
      registerSlot *reg = theRegSpace->GPRs()[i];
      if(reg->liveState == registerSlot::live) {
        saveRegister(gen, reg->encoding(), save_off);
        // saveRegister implicitly adds in (reg * word size)
        // Do that by hand here.

        int actual_save_off = save_off;

        actual_save_off += (reg->encoding() * gen.width());

        gen.rs()->markSavedRegister(reg->number, actual_save_off);
        numRegs++;
        if(numRegs == numReqGPRs) {
          break;
        }
      }
    }
    return numRegs;
  }

  /*
   * Restore necessary registers from the stack
   * insn, base: for code generation. Offset: regs restored from offset + reg
   * Returns: number of registers restored.
   * Side effects: instruction pointer and base param are shifted to
   *   next free slot.
   */

  unsigned restoreGPRegisters(codeGen &gen, registerSpace *theRegSpace, int save_off) {
    unsigned numRegs = 0;
    for(int i = 0; i < theRegSpace->numGPRs(); i++) {
      registerSlot *reg = theRegSpace->GPRs()[i];
      if(reg->liveState == registerSlot::spilled) {
        restoreRegister(gen, reg->encoding(), save_off);
        numRegs++;
      }
    }

    return numRegs;
  }

  /*
   * Save FPR registers on the stack. (0-13)
   * insn, base: for code generation. Offset: regs saved at offset + reg
   * Returns: number of regs saved.
   */

  unsigned saveFPRegisters(codeGen &gen, registerSpace *, int save_off) {
    insnCodeGen::saveVectors(gen, save_off);
    return 32;
  }

  /*
   * Restore FPR registers from the stack. (0-13)
   * insn, base: for code generation. Offset: regs restored from offset + reg
   * Returns: number of regs restored.
   */

  unsigned restoreFPRegisters(codeGen &gen, registerSpace *, int save_off) {

    insnCodeGen::restoreVectors(gen, save_off);
    return 32;
  }

  /*
   * Save the special purpose registers (for Dyninst conservative tramp)
   * CTR, CR, XER, SPR0, FPSCR
   */
  unsigned saveSPRegisters(codeGen &gen, registerSpace *, int save_off, int force_save) {
    unsigned num_saved = 0;
    int cr_off, ctr_off, xer_off, fpscr_off;

    if(gen.width() == 4) {
      cr_off = STK_CR_32;
      ctr_off = STK_CTR_32;
      xer_off = STK_XER_32;
      fpscr_off = STK_FP_CR_32;
    } else /* gen.width() == 8 */ {
      cr_off = STK_CR_64;
      ctr_off = STK_CTR_64;
      xer_off = STK_XER_64;
      fpscr_off = STK_FP_CR_64;
    }

    registerSlot *regCR = (*(gen.rs()))[registerSpace::cr];
    assert(regCR != NULL);
    if(force_save || regCR->liveState == registerSlot::live) {
      saveCR(gen, 10, save_off + cr_off);
      num_saved++;
      gen.rs()->markSavedRegister(registerSpace::cr, save_off + cr_off);
    }
    registerSlot *regCTR = (*(gen.rs()))[registerSpace::ctr];
    assert(regCTR != NULL);
    if(force_save || regCTR->liveState == registerSlot::live) {
      saveSPR(gen, 10, SPR_CTR, save_off + ctr_off);
      num_saved++;
      gen.rs()->markSavedRegister(registerSpace::ctr, save_off + ctr_off);
    }

    registerSlot *regXER = (*(gen.rs()))[registerSpace::xer];
    assert(regXER != NULL);
    if(force_save || regXER->liveState == registerSlot::live) {
      saveSPR(gen, 10, SPR_XER, save_off + xer_off);
      num_saved++;
      gen.rs()->markSavedRegister(registerSpace::xer, save_off + xer_off);
    }

    saveFPSCR(gen, 10, save_off + fpscr_off);
    num_saved++;

    return num_saved;
  }

  /*
   * Restore the special purpose registers (for Dyninst conservative tramp)
   * CTR, CR, XER, SPR0, FPSCR
   */
  unsigned restoreSPRegisters(codeGen &gen, registerSpace *, int save_off, int force_save) {
    int cr_off, ctr_off, xer_off, fpscr_off;
    unsigned num_restored = 0;

    if(gen.width() == 4) {
      cr_off = STK_CR_32;
      ctr_off = STK_CTR_32;
      xer_off = STK_XER_32;
      fpscr_off = STK_FP_CR_32;
    } else /* gen.width() == 8 */ {
      cr_off = STK_CR_64;
      ctr_off = STK_CTR_64;
      xer_off = STK_XER_64;
      fpscr_off = STK_FP_CR_64;
    }

    restoreFPSCR(gen, 10, save_off + fpscr_off);
    num_restored++;

    registerSlot *regXER = (*(gen.rs()))[registerSpace::xer];
    assert(regXER != NULL);
    if(force_save || regXER->liveState == registerSlot::spilled) {
      restoreSPR(gen, 10, SPR_XER, save_off + xer_off);
      num_restored++;
    }
    registerSlot *regCTR = (*(gen.rs()))[registerSpace::ctr];
    assert(regCTR != NULL);
    if(force_save || regCTR->liveState == registerSlot::spilled) {
      restoreSPR(gen, 10, SPR_CTR, save_off + ctr_off);
      num_restored++;
    }
    registerSlot *regCR = (*(gen.rs()))[registerSpace::cr];
    assert(regCR != NULL);
    if(force_save || regCR->liveState == registerSlot::spilled) {
      restoreCR(gen, 10, save_off + cr_off);
      num_restored++;
    }

    return num_restored;
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
