#include "arch-aarch64.h"
#include "ASTs/codeGenAST.h"
#include "binaryEdit.h"
#include "codegen-aarch64.h"
#include "codegen/emitters/aarch64/EmitterAarch64.h"
#include "codegen/RegControl.h"
#include "debug.h"
#include "emit-aarch64.h"
#include "inst-aarch64.h"
#include "RegisterConversion.h"
#include "registerSpace/registerSpace.h"

using codeGenASTPtr = Dyninst::DyninstAPI::codeGenASTPtr;

// VG(03/15/02): Restore mutatee value of GPR reg to dest GPR
static inline void restoreGPRtoGPR(codeGen &gen, Register reg, Register dest) {
  int frame_size, gpr_size, gpr_off;

  frame_size = TRAMP_FRAME_SIZE_64;
  gpr_size = GPRSIZE_64;
  gpr_off = TRAMP_GPR_OFFSET_64;

  // Stack Point Register
  if(reg == 31) {
    insnCodeGen::generateAddSubImmediate(gen, insnCodeGen::Add, 0, frame_size, REG_SP, dest, true);
  } else {
    insnCodeGen::restoreRegister(gen, dest, gpr_off + reg * gpr_size);
  }

  return;
}

struct parsed_regs {
  std::set<Dyninst::Register> gprs, fprs;
};

/* This does a linear scan to find out which registers are used in the function,
   it then stores these registers so the scan only needs to be done once.
   It returns true or false based on whether the function is a leaf function,
   since if it is not the function could call out to another function that
   clobbers more registers so more analysis would be needed */
static parsed_regs calcUsedRegs(parse_func *func) {

  parsed_regs usedRegisters{};

  using namespace Dyninst::InstructionAPI;
  std::set<RegisterAST::Ptr> writtenRegs;

  auto bl = func->blocks();
  auto curBlock = bl.begin();
  for(; curBlock != bl.end(); ++curBlock) {
    InstructionDecoder d(func->getPtrToInstruction((*curBlock)->start()), (*curBlock)->size(),
                         func->isrc()->getArch());
    Instruction i;
    i = d.decode();
    while(i.isValid()) {
      i.getWriteSet(writtenRegs);
      i = d.decode();
    }
  }

  for(auto const &reg : writtenRegs) {
    MachRegister r = reg->getID();
    auto regID = convertRegID(r.getBaseRegister());
    if(regID == registerSpace::ignored) {
      logLine("parse_func::calcUsedRegs: unknown written register\n");
      continue;
    }
    if(r.isGeneralPurpose()) {
      usedRegisters.gprs.insert(regID);
    } else if(r.isFloatingPoint()) {
      usedRegisters.fprs.insert(regID);
    }
  }
  return usedRegisters;
}

namespace Dyninst { namespace DyninstAPI {

    bool EmitterAarch64::clobberAllFuncCall(registerSpace *rs, func_instance *callee) {
      if(!callee) {
        return true;
      }

      if(clobbered_functions.contains(callee)) {
        return true;
      }
      clobbered_functions.insert(callee);

      if(callee->ifunc()->isLeafFunc()) {
        auto const &regs = calcUsedRegs(callee->ifunc());

        for(auto const &r : regs.gprs) {
          rs->GPRs()[r]->beenUsed = true;
        }
        for(auto const &r : regs.fprs) {
          rs->FPRs()[registerSpace::FPR(r)]->beenUsed = true;
        }
      } else {
        for(int idx = 0; idx < rs->numGPRs(); idx++) {
          rs->GPRs()[idx]->beenUsed = true;
        }
        for(int idx = 0; idx < rs->numFPRs(); idx++) {
          rs->FPRs()[idx]->beenUsed = true;
        }
      }
      return false;
    }

    codeBufIndex_t EmitterAarch64::emitA(opCode op, Register src1, long dest, codeGen &gen,
                                         Dyninst::DyninstAPI::RegControl rc) {
      codeBufIndex_t retval = 0;

      switch(op) {
        case ifOp: {
          // if src1 == 0 jump to dest
          // src1 is a temporary
          // dest is a target address
          retval = gen.codeEmitter()->emitIf(src1, dest, rc, gen);
          break;
        }
        case branchOp: {
          insnCodeGen::generateBranch(gen, dest);
          retval = gen.getIndex();
          break;
        }
        default:
          assert(0); // op not implemented or not expected for this emit!
      }

      return retval;
    }

    // Yuhan(02/04/19): Load in destination the effective address given
    // by the address descriptor. Used for memory access stuff.
    void EmitterAarch64::emitAddrSpecLoad(const BPatch_addrSpec_NP *as, Register dest,
                                          int stackShift, codeGen &gen) {

      // Haven't implemented non-zero shifts yet
      assert(stackShift == 0);
      long int imm = as->getImm();
      int ra = as->getReg(0);
      int rb = as->getReg(1);
      int sc = as->getScale();
      gen.markRegDefined(dest);
      if(ra > -1) {
        if(ra == 32) {
          // Special case where the actual address is store in imm.
          // Need to change this for rewriting PIE or shared libraries
          insnCodeGen::loadImmIntoReg(gen, dest, static_cast<Address>(imm));
          return;
        } else {
          restoreGPRtoGPR(gen, ra, dest);
        }
      } else {
        insnCodeGen::loadImmIntoReg(gen, dest, static_cast<Address>(0));
      }
      if(rb > -1) {
        std::vector<Register> exclude;
        exclude.push_back(dest);
        Register scratch = gen.rs()->getScratchRegister(gen, exclude);
        assert(scratch != Null_Register && "cannot get a scratch register");
        gen.markRegDefined(scratch);
        restoreGPRtoGPR(gen, rb, scratch);
        // call adds, save 2^scale * rb to dest
        insnCodeGen::generateAddSubShifted(gen, insnCodeGen::Add, 0, sc, scratch, dest, dest, true);
      }

      // emit code to load the immediate (constant offset) into dest; this
      // writes at gen+base and updates base, we must update insn..
      if(imm) {
        insnCodeGen::generateAddSubImmediate(gen, insnCodeGen::Add, 0, imm, dest, dest, true);
      }
    }

    void EmitterAarch64::emitAddSignedImm(Address, int, codeGen &) {
      assert(0);
    }

    bool EmitterAarch64::emitAdjustStackPointer(int, codeGen &) {
      assert(0);
      return false;
    }

    void EmitterAarch64::emitASload(int, int, int, long, Register, int, codeGen &) {
      assert(0);
    }

    bool EmitterAarch64::emitBTRestores(baseTramp *, codeGen &) {
      assert(0);
      return false;
    }

    bool EmitterAarch64::emitBTSaves(baseTramp *, codeGen &) {
      assert(0);
      return false;
    }

    // There are four "axes" going on here:
    // 32 bit vs 64 bit
    // Instrumentation vs function call replacement
    // Static vs. dynamic
    Register EmitterAarch64::emitCall(opCode op, codeGen &gen,
                                      const std::vector<codeGenASTPtr> &operands,
                                      func_instance *callee) {
      // #sasha This function implementation is experimental.

      if(op != callOp) {
        cerr << "ERROR: emitCall with op == " << op << endl;
      }
      assert(op == callOp);

      std::vector<Register> srcs;
      std::vector<Register> saves;

      //  Sanity check for NULL address arg
      if(!callee) {
        char msg[256];
        sprintf(msg,
                "%s[%d]:  internal error:  emitFuncCall called w/out"
                "callee argument",
                __FILE__, __LINE__);
        showErrorCallback(80, msg);
        assert(0);
      }

      vector<int> savedRegs;

      // save r0-r7
      for(int id = 0; id < gen.rs()->numGPRs(); id++) {
        registerSlot *reg = gen.rs()->GPRs()[id];

        // We must save if:
        // refCount > 0 (and not a source register)
        // keptValue == true (keep over the call)
        // liveState == live (technically, only if not saved by the callee)

        if((reg->refCount > 0) || reg->keptValue || (reg->liveState == registerSlot::live)) {
          insnCodeGen::saveRegister(gen, registerSpace::r0 + id, -2 * GPRSIZE_64,
                                    insnCodeGen::Post);
          savedRegs.push_back(reg->number);
        }
      }

      // Passing operands to registers
      for(size_t id = 0; id < operands.size(); id++) {
        Register reg = Null_Register;
        if(gen.rs()->allocateSpecificRegister(gen, registerSpace::r0 + id)) {
          reg = registerSpace::r0 + id;
        }

        Address unnecessary = ADDR_NULL;
        if(!operands[id]->generateCode_phase2(gen, unnecessary, reg)) {
          assert(0);
        }
        assert(reg != Null_Register);
      }

      assert(gen.rs());

      // Address of function to call in scratch register
      Register scratch = gen.rs()->getScratchRegister(gen);
      assert(scratch != Null_Register && "cannot get a scratch register");
      gen.markRegDefined(scratch);

      if(gen.addrSpace()->edit() != NULL &&
         (gen.func()->obj() != callee->obj() || gen.addrSpace()->needsPIC())) {
        // gen.as.edit() checks if we are in rewriter mode
        Address dest = getInterModuleFuncAddr(callee, gen);

        // emit ADR instruction

        long disp = dest - gen.currAddr();
        instruction insn;
        insn.clear();
        INSN_SET(insn, 31, 31, 0);
        INSN_SET(insn, 28, 28, 1);
        INSN_SET(insn, 5, 23, disp >> 2);
        INSN_SET(insn, 0, 4, scratch);
        insnCodeGen::generate(gen, insn);

        insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, scratch, scratch, 0, 8,
                                       insnCodeGen::Offset);
      } else {
        insnCodeGen::loadImmIntoReg(gen, scratch, callee->addr());
      }

      instruction branchInsn;
      branchInsn.clear();

      // Set bits which are 0 for both BR and BLR
      INSN_SET(branchInsn, 0, 4, 0);
      INSN_SET(branchInsn, 10, 15, 0);

      // Set register
      INSN_SET(branchInsn, 5, 9, scratch);

      // Set other bits. Basically, these are the opcode bits.
      // The only difference between BR and BLR is that bit 21 is 1 for BLR.
      INSN_SET(branchInsn, 16, 31, BRegOp);
      INSN_SET(branchInsn, 21, 21, 1);
      insnCodeGen::generate(gen, branchInsn);

      /*
       * Restoring registers
       */

      // r7-r0
      for(signed int ui = savedRegs.size() - 1; ui >= 0; ui--) {
        insnCodeGen::restoreRegister(gen, registerSpace::r0 + savedRegs[ui], 2 * GPRSIZE_64,
                                     insnCodeGen::Post);
      }

      return 0;
    }

    // Generates call instruction sequence for all AARCH64-based systems
    // under dynamic instrumentation.
    //
    // This should be able to stomp on the link register (LR) and TOC
    // register (r2), as they were saved by Emitter::emitCall() as necessary.
    bool EmitterAarch64::emitCallInstruction(codeGen &, func_instance *, bool, Address) {
      assert(0); // Not implemented
      return true;
    }

    bool EmitterAarch64::emitCallRelative(Register, Address, Register, codeGen &) {
      assert(0); // Not implemented
      return true;
    }

    Register EmitterAarch64::emitCallReplacement(opCode, codeGen &, func_instance *) {
      assert(0); // Not implemented
      return 0;
    }

    void EmitterAarch64::emitCountSpecLoad(const BPatch_countSpec_NP *, Dyninst::Register,
                                           codeGen &) {
      assert(0);
    }

    void EmitterAarch64::emitCSload(int, int, int, long, Register, codeGen &) {
      assert(0);
    }

    void EmitterAarch64::emitDiv(Register, Register, Register, codeGen &, bool) {
      assert(0);
    }

    void EmitterAarch64::emitDivImm(Register, Register, RegValue, codeGen &, bool) {
      assert(0);
    }

    // #sasha Fix parameters number
    void EmitterAarch64::emitGetParam(Register, Register param_num, instPoint::Type, opCode op,
                                      bool, codeGen &gen) {
      registerSlot *regSlot = NULL;
      switch(op) {
        case getParamOp:
          if(param_num <= 3) {
            // param_num is 0..8 - it's a parameter number, not a register
            regSlot = (*(gen.rs()))[registerSpace::r0 + param_num];
            break;
          } else {
            assert(0);
          }
          break;
        default:
          assert(0);
          break;
      } // end of swich(op)

      assert(regSlot);
      // Register reg = regSlot->number;

      // return reg;
    }

    void EmitterAarch64::emitGetRetAddr(Register, codeGen &) {
      assert(0);
    }

    void EmitterAarch64::emitGetRetVal(Register, bool, codeGen &) {
      assert(0);
    }

    codeBufIndex_t EmitterAarch64::emitIf(Register expr_reg, Register target,
                                          Dyninst::DyninstAPI::RegControl /*rc*/, codeGen &gen) {
      instruction insn;
      insn.clear();

      // compare to 0 and branch
      // register number, its value is compared to 0.
      INSN_SET(insn, 0, 4, expr_reg);
      INSN_SET(insn, 5, 23, (target + 4) / 4);
      INSN_SET(insn, 25, 30, 0x1a); // CBZ
      INSN_SET(insn, 31, 31, 1);

      insnCodeGen::generate(gen, insn);

      // Retval: where the jump is in this sequence
      codeBufIndex_t retval = gen.getIndex();
      return retval;
    }

    // TODO: 32-/64-bit regs?
    void EmitterAarch64::emitImm(opCode op, Register src1, RegValue src2imm, Register dest,
                                 codeGen &gen, bool s) {
      switch(op) {
        case plusOp:
        case minusOp: {
          Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
          insnCodeGen::generateAddSubShifted(
              gen, op == plusOp ? insnCodeGen::Add : insnCodeGen::Sub, 0, 0, rm, src1, dest, true);
        } break;
        case timesOp: {
          Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
          insnCodeGen::generateMul(gen, rm, src1, dest, true);
        } break;
        case divOp: {
          Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
          insnCodeGen::generateDiv(gen, rm, src1, dest, true, s);
        } break;
        case xorOp: {
          Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
          insnCodeGen::generateBitwiseOpShifted(gen, insnCodeGen::Eor, 0, rm, 0, src1, dest, true);
        } break;
        case orOp: {
          Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
          insnCodeGen::generateBitwiseOpShifted(gen, insnCodeGen::Or, 0, rm, 0, src1, dest, true);
        } break;
        case andOp: {
          Register rm = insnCodeGen::moveValueToReg(gen, src2imm);
          insnCodeGen::generateBitwiseOpShifted(gen, insnCodeGen::And, 0, rm, 0, src1, dest, true);
        } break;
        case eqOp: {
          Register scratch = gen.rs()->getScratchRegister(gen);
          gen.emitter()->emitVload(loadConstOp, src2imm, 0, scratch, gen);
          emitV(op, src1, scratch, dest, gen);
        } break;
        case neOp:
        case lessOp:
        case leOp:
        case greaterOp:
        case geOp:
          // note that eqOp could be grouped here too.
          // There's two ways to implement this.
          gen.codeEmitter()->emitRelOpImm(op, dest, src1, src2imm, gen, s);
          return;
        default:
          assert(0); // not implemented or not valid
          break;
      }
    }

    void EmitterAarch64::emitLoad(Register dest, Address addr, int size, codeGen &gen) {
      Register scratch = gen.rs()->getScratchRegister(gen);

      insnCodeGen::loadImmIntoReg(gen, scratch, addr);
      insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, dest, scratch, 0, size,
                                     insnCodeGen::Post);

      gen.rs()->freeRegister(scratch);
      gen.markRegDefined(dest);
    }

    void EmitterAarch64::emitLoadConst(Register dest, Address imm, codeGen &gen) {
      insnCodeGen::loadImmIntoReg(gen, dest, imm);
    }

    void EmitterAarch64::emitLoadFrameAddr(Register, Address, codeGen &) {
      assert(0);
    }

    void EmitterAarch64::emitLoadIndir(Register dest, Register addr_src, int size, codeGen &gen) {
      insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, dest, addr_src, 0, size,
                                     insnCodeGen::Post);

      gen.markRegDefined(dest);
    }

    void EmitterAarch64::emitLoadOrigFrameRelative(Register, Address, codeGen &) {
      assert(0);
    }

    void EmitterAarch64::emitLoadOrigRegister(Address register_num, Register destination,
                                              codeGen &gen) {
      registerSlot *src = (*gen.rs())[register_num];
      assert(src);
      registerSlot *dest = (*gen.rs())[destination];
      assert(dest);

      if(src->name == "sp") {
        insnCodeGen::generateAddSubImmediate(gen, insnCodeGen::Add, 0, TRAMP_FRAME_SIZE_64, REG_SP,
                                             destination, true);

        return;
      }

      if(src->spilledState == registerSlot::unspilled) {
        // not on the stack. Directly move the value
        insnCodeGen::generateMove(gen, destination, (Register)register_num, true);
        return;
      }

      int offset = TRAMP_GPR_OFFSET(gen.width());
      // its on the stack so load it.
      insnCodeGen::restoreRegister(gen, destination, offset + (register_num * gen.width()),
                                   insnCodeGen::Offset);
    }

    void EmitterAarch64::emitLoadOrigRegRelative(Register dest, Address offset, Register base,
                                                 codeGen &gen, bool deref) {
      gen.markRegDefined(dest);
      Register scratch = gen.rs()->getScratchRegister(gen);
      assert(scratch);
      gen.markRegDefined(scratch);

      // either load the address or the contents at that address
      if(deref) {
        // load the stored register 'base' into scratch
        emitLoadOrigRegister(base, scratch, gen);
        // move offset(%scratch), %dest
        insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, dest, scratch, offset,
                                       /*size==8?true:false*/ 4, insnCodeGen::Offset);
      } else {
        // load the stored register 'base' into dest
        emitLoadOrigRegister(base, scratch, gen);
        insnCodeGen::loadImmIntoReg(gen, dest, offset);
        insnCodeGen::generateAddSubShifted(gen, insnCodeGen::Add, 0, 0, dest, scratch, dest, true);
      }
    }

    bool EmitterAarch64::emitLoadRelative(Register dest, Address offset, Register baseReg, int size,
                                          codeGen &gen) {
      signed long long sOffset = (signed long long)offset;
      if(sOffset >= -256 && sOffset <= 255) {
        insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, dest, baseReg, sOffset, size,
                                       insnCodeGen::Pre);
      } else {
        std::vector<Register> exclude;
        exclude.push_back(baseReg);
        // mov sOffset to a reg
        auto addReg = insnCodeGen::moveValueToReg(gen, labs(sOffset), &exclude);
        // add/sub sOffset to baseReg
        insnCodeGen::generateAddSubShifted(gen, sOffset > 0 ? insnCodeGen::Add : insnCodeGen::Sub,
                                           0, 0, addReg, baseReg, baseReg, true);
        insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, dest, baseReg, 0, size,
                                       insnCodeGen::Pre);
      }

      gen.markRegDefined(dest);
      return true;
    }

    void EmitterAarch64::emitLoadShared(opCode op, Register dest, const image_variable *var,
                                        bool is_local, int size, codeGen &gen, Address offset) {
      // create or retrieve jump slot
      Address addr;
      int stackSize = 0;

      if(!var) {
        addr = offset;
      } else if(!is_local) {
        addr = getInterModuleVarAddr(var, gen);
      } else {
        addr = (Address)var->getOffset();
      }

      // load register with address from jump slot
      Register baseReg = gen.rs()->getScratchRegister(gen);
      assert(baseReg != Null_Register && "cannot get a scratch register");

      emitMovePCToReg(baseReg, gen);
      Address varOffset = addr - gen.currAddr() + 4;

      if(op == loadOp) {
        if(!is_local && (var != NULL)) {
          emitLoadRelative(dest, varOffset, baseReg, gen.width(), gen);
          // Deference the pointer to get the variable
          // emitLoadRelative(dest, 0, dest, size, gen);
          // Offset mode to load back to itself
          insnCodeGen::generateMemAccess(gen, insnCodeGen::Load, dest, dest, 0, 8,
                                         insnCodeGen::Offset);
        } else {
          emitLoadRelative(dest, varOffset, baseReg, size, gen);
        }
      } else { // loadConstop
        if(!is_local && (var != NULL)) {
          emitLoadRelative(dest, varOffset, baseReg, gen.width(), gen);
        } else {
          std::vector<Register> exclude;
          exclude.push_back(baseReg);
          auto addReg =
              insnCodeGen::moveValueToReg(gen, labs(static_cast<long int>(varOffset)), &exclude);
          insnCodeGen::generateAddSubShifted(
              gen, (signed long long)varOffset > 0 ? insnCodeGen::Add : insnCodeGen::Sub, 0, 0,
              addReg, baseReg, baseReg, true);
          insnCodeGen::generateMove(gen, dest, baseReg, true);
        }
      }

      assert(stackSize <= 0 && "stack not empty at the end");
    }

    Address EmitterAarch64::emitMovePCToReg(Register dest, codeGen &gen) {
      instruction insn;
      insn.clear();

      INSN_SET(insn, 28, 28, 1);
      INSN_SET(insn, 0, 4, dest);

      insnCodeGen::generate(gen, insn);
      Address ret = gen.currAddr();
      return ret;
    }

    bool EmitterAarch64::emitMoveRegToReg(Register, Register, codeGen &) {
      assert(0);
      return false;
    }

    bool EmitterAarch64::emitMoveRegToReg(registerSlot *, registerSlot *, codeGen &) {
      assert(0); // Not implemented
      return true;
    }

    void EmitterAarch64::emitOp(unsigned opcode, Register dest, Register src1, Register src2,
                                codeGen &gen) {
      // dest = src1 + src2
      if(opcode == plusOp) {
        insnCodeGen::generateAddSubShifted(gen, insnCodeGen::Add, 0, 0, src1, src2, dest, true);
      }

      // dest = src1 - src2
      else if(opcode == minusOp) {
        insnCodeGen::generateAddSubShifted(gen, insnCodeGen::Sub, 0, 0, src2, src1, dest, true);
      }

      // dest = src1 * src2
      else if(opcode == timesOp) {
        insnCodeGen::generateMul(gen, src1, src2, dest, true);
      }

      // dest = src1 & src2
      else if(opcode == andOp) {
        insnCodeGen::generateBitwiseOpShifted(gen, insnCodeGen::And, 0, src1, 0, src2, dest, true);
      }

      // dest = src1 | src2
      else if(opcode == orOp) {
        insnCodeGen::generateBitwiseOpShifted(gen, insnCodeGen::Or, 0, src1, 0, src2, dest, true);
      }

      // dest = src1 ^ src2
      else if(opcode == xorOp) {
        insnCodeGen::generateBitwiseOpShifted(gen, insnCodeGen::Eor, 0, src1, 0, src2, dest, true);
      }

      else {
        assert(0);
      }
    }

    void EmitterAarch64::emitOpImm(unsigned, unsigned, Register, Register, RegValue, codeGen &) {
      assert(0);
    }

    bool EmitterAarch64::emitPop(codeGen &, Register) {
      assert(0);
      return false;
    }

    bool EmitterAarch64::emitPush(codeGen &, Register) {
      assert(0);
      return false;
    }

    void EmitterAarch64::emitPushFlags(codeGen &) {
      assert(0);
    }

    Register EmitterAarch64::emitR(opCode op, Register src1, Register src2, Register dest,
                                   codeGen &gen, const instPoint *) {
      registerSlot *regSlot = NULL;
      unsigned addrWidth = gen.width();

      switch(op) {
        case getRetValOp:
          regSlot = (*(gen.rs()))[registerSpace::r0];
          break;
        case getParamOp:
          // src1 is the number of the argument
          // dest is a register where we can store the value
          // gen.codeEmitter()->emitGetParam(dest, src1, location->type(), op,
          //        false, gen);

          if(src1 <= 7) {
            // src1 is 0..7 - it's a parameter order number, not a register
            regSlot = (*(gen.rs()))[registerSpace::r0 + src1];
            break;

          } else {
            int stkOffset = TRAMP_FRAME_SIZE_64 + (src1 - 8) * sizeof(long);
            // printf("TRAMP_FRAME_SIZE_64: %d\n", TRAMP_FRAME_SIZE_64);
            // printf("stdOffset = TRAMP_xxx_64 + (argc - 8) * 8 = { %d }\n", stkOffset);
            // TODO: PARAM_OFFSET(addrWidth) is currently not used
            // should delete that macro if it's useless

            if(src2 != Null_Register) {
              insnCodeGen::saveRegister(gen, src2, stkOffset);
            }
            insnCodeGen::restoreRegister(gen, dest, stkOffset);

            return dest;
          }
          break;
        default:
          assert(0);
      }

      assert(regSlot);
      Register reg = regSlot->number;

      switch(regSlot->liveState) {
        case registerSlot::spilled: {
          int offset = TRAMP_GPR_OFFSET(addrWidth);
          // its on the stack so load it.
          // if (src2 != Null_Register) saveRegister(gen, src2, reg, offset);
          insnCodeGen::restoreRegister(gen, dest, offset + (reg * gen.width()));
          return (dest);
        }
        case registerSlot::live: {
          // its still in a register so return the register it is in.
          cerr << "emitR state:" << reg << " live" << endl;
          assert(0);
          return (reg);
        }
        case registerSlot::dead: {
          cerr << "emitR state" << reg << ": dead" << endl;
          // Uhhh... wha?
          assert(0);
        }
      }
      return reg;
    }

    void EmitterAarch64::emitRelOp(unsigned opcode, Register dest, Register src1, Register src2,
                                   codeGen &gen, bool s) {
      // CMP is an alias to SUBS;
      // dest here has src1-src2, which it's not important because the flags are
      // used for the comparison, not the subtration value.
      // Besides that dest must contain 1 for true or 0 for false, and the content
      // of dest is gonna be changed as follow.
      insnCodeGen::generateAddSubShifted(gen, insnCodeGen::Sub, 0, 0, src2, src1, dest, true);

      // make dest = 1, meaning true
      insnCodeGen::loadImmIntoReg(gen, dest, 0x1);

      // insert conditional jump to skip dest=0 in case the comparison resulted true
      // therefore keeping dest=1
      insnCodeGen::generateConditionalBranch(gen, 8, opcode, s);

      // make dest = 0, in case it fails the branch
      insnCodeGen::loadImmIntoReg(gen, dest, 0x0);
    }

    void EmitterAarch64::emitRelOpImm(unsigned opcode, Register dest, Register src1,
                                      RegValue src2imm, codeGen &gen, bool s) {
      // Register src2 = gen.rs()->allocateRegister(gen);
      Register src2 = gen.rs()->getScratchRegister(gen);
      emitLoadConst(src2, src2imm, gen);

      // CMP is an alias to SUBS;
      // dest here has src1-src2, which it's not important because the flags are
      // used for the comparison, not the subtration value.
      // Besides that dest must contain 1 for true or 0 for false, and the content
      // of dest is gonna be changed as follow.
      insnCodeGen::generateAddSubShifted(gen, insnCodeGen::Sub, 0, 0, src2, src1, dest, true);

      // make dest = 1, meaning true
      insnCodeGen::loadImmIntoReg(gen, dest, 0x1);

      // insert conditional jump to skip dest=0 in case the comparison resulted true
      // therefore keeping dest=1
      insnCodeGen::generateConditionalBranch(gen, 8, opcode, s);

      // make dest = 0, in case it fails the branch
      insnCodeGen::loadImmIntoReg(gen, dest, 0x0);

      gen.rs()->freeRegister(src2);
      gen.markRegDefined(dest);
    }

    void EmitterAarch64::emitRestoreFlags(codeGen &, unsigned) {
      assert(0);
    }

    void EmitterAarch64::emitRestoreFlagsFromStackSlot(codeGen &) {
      assert(0);
    }

    void EmitterAarch64::emitStore(Address addr, Register src, int size, codeGen &gen) {
      Register scratch = gen.rs()->getScratchRegister(gen);

      insnCodeGen::loadImmIntoReg(gen, scratch, addr);
      insnCodeGen::generateMemAccess(gen, insnCodeGen::Store, src, scratch, 0, size,
                                     insnCodeGen::Pre);

      gen.rs()->freeRegister(scratch);
      gen.markRegDefined(src);
    }

    void EmitterAarch64::emitStoreFrameRelative(Address, Register, Register, int, codeGen &) {
      assert(0);
    }

    void EmitterAarch64::emitStoreImm(Address, int, codeGen &) {
      assert(0);
    }

    void EmitterAarch64::emitStoreIndir(Register addr_reg, Register src, int size, codeGen &gen) {
      insnCodeGen::generateMemAccess(gen, insnCodeGen::Store, src, addr_reg, 0, size,
                                     insnCodeGen::Pre);

      gen.markRegDefined(addr_reg);
    }

    void EmitterAarch64::emitStoreOrigRegister(Address, Register, codeGen &) {
      assert(0);
    }

    void EmitterAarch64::emitStoreRelative(Register source, Address offset, Register base, int size,
                                           codeGen &gen) {
      if((signed long long)offset <= 255 && (signed long long)offset >= -256) {
        insnCodeGen::generateMemAccess(gen, insnCodeGen::Store, source, base, offset, size,
                                       insnCodeGen::Pre);
      } else {
        assert(0 && "offset in emitStoreRelative not in (-256,255)");
      }
    }

    void EmitterAarch64::emitStoreShared(Register source, const image_variable *var, bool is_local,
                                         int size, codeGen &gen) {
      // create or retrieve jump slot
      Address addr;
      int stackSize = 0;
      if(!is_local) {
        addr = getInterModuleVarAddr(var, gen);
      } else {
        addr = (Address)var->getOffset();
      }

      // load register with address from jump slot
      Register baseReg = gen.rs()->getScratchRegister(gen);
      assert(baseReg != Null_Register && "cannot get a scratch register");

      emitMovePCToReg(baseReg, gen);
      Address varOffset = addr - gen.currAddr() + 4;

      if(!is_local) {
        std::vector<Register> exclude;
        exclude.push_back(baseReg);
        Register scratchReg1 = gen.rs()->getScratchRegister(gen, exclude);
        assert(scratchReg1 != Null_Register && "cannot get a scratch register");
        emitLoadRelative(scratchReg1, varOffset, baseReg, gen.width(), gen);
        emitStoreRelative(source, 0, scratchReg1, size, gen);
      } else {
        std::vector<Register> exclude;
        exclude.push_back(baseReg);
        // mov offset to a reg
        auto addReg =
            insnCodeGen::moveValueToReg(gen, labs(static_cast<long int>(varOffset)), &exclude);
        // add/sub offset to baseReg
        insnCodeGen::generateAddSubShifted(
            gen, (signed long long)varOffset > 0 ? insnCodeGen::Add : insnCodeGen::Sub, 0, 0,
            addReg, baseReg, baseReg, true);
        insnCodeGen::generateMemAccess(gen, insnCodeGen::Store, source, baseReg, 0, size,
                                       insnCodeGen::Pre);
      }

      assert(stackSize <= 0 && "stack not empty at the end");
    }

    void EmitterAarch64::emitTimesImm(Register, Register, RegValue, codeGen &) {
      assert(0);
    }

    bool EmitterAarch64::emitTOCCall(block_instance *, codeGen &) {
      assert(0);
      return false;
    }

    bool EmitterAarch64::emitTOCJump(block_instance *, codeGen &) {
      assert(0);
      return false;
    }

    void EmitterAarch64::emitV(opCode op, Register src1, Register src2, Register dest, codeGen &gen,
                               int size, AddressSpace *proc, bool s) {
      switch(op) {
        case plusOp:
        case minusOp:
        case timesOp:
        case orOp:
        case andOp:
        case xorOp:
          gen.codeEmitter()->emitOp(op, dest, src1, src2, gen);
          break;
        case divOp:
          insnCodeGen::generateDiv(gen, src2, src1, dest, true, s);
          break;
        case lessOp:
        case leOp:
        case greaterOp:
        case geOp:
        case eqOp:
        case neOp:
          gen.codeEmitter()->emitRelOp(op, dest, src1, src2, gen, s);
          break;
        case loadIndirOp:
          size = !size ? proc->getAddressWidth() : size;
          // same as loadOp, but the value to load is already in a register
          gen.codeEmitter()->emitLoadIndir(dest, src1, size, gen);
          break;
        case storeIndirOp:
          size = !size ? proc->getAddressWidth() : size;
          gen.codeEmitter()->emitStoreIndir(dest, src1, size, gen);
          break;
        default:
          // std::cout << "operation not implemented= " << op << endl;
          assert(0); // Not implemented
          break;
      }
      return;
    }

    void EmitterAarch64::emitVload(opCode op, Address src1, Register src2, Register dest,
                                   codeGen &gen, int size, AddressSpace *) {
      switch(op) {
        case loadConstOp:
          // dest is a temporary
          // src1 is an immediate value
          // dest = src1:imm32
          gen.codeEmitter()->emitLoadConst(dest, src1, gen);
          break;
        case loadOp:
          // dest is a temporary
          // src1 is the address of the operand
          // dest = [src1]
          gen.codeEmitter()->emitLoad(dest, src1, size, gen);
          break;
        case loadRegRelativeAddr:
          // (readReg(src2) + src1)
          // dest is a temporary
          // src2 is the register
          // src1 is the offset from the address in src2
          gen.codeEmitter()->emitLoadOrigRegRelative(dest, src1, src2, gen, false);
          break;
        case loadRegRelativeOp:
          // *(readReg(src2) + src1)
          // dest is a temporary
          // src2 is the register
          // src1 is the offset from the address in src2
          gen.codeEmitter()->emitLoadOrigRegRelative(dest, src1, src2, gen, true);
          break;
        default:
          assert(0); // Not implemented
          break;
      }
    }

    void EmitterAarch64::emitVstore(opCode op, Register src1, Register /*src2*/, Address dest,
                                    codeGen &gen, int size, AddressSpace *) {
      if(op == storeOp) {
        // [dest] = src1
        // dest has the address where src1 is to be stored
        // src1 is a temporary
        // src2 is a "scratch" register, we don't need it in this architecture
        gen.codeEmitter()->emitStore(dest, src1, size, gen);
      } else {
        assert(0); // Not implemented
      }
      return;
    }

    Address EmitterAarch64::getInterModuleFuncAddr(func_instance *func, codeGen &gen) {
      // from POWER64 getInterModuleFuncAddr

      AddressSpace *addrSpace = gen.addrSpace();
      if(!addrSpace) {
        assert(0 && "No AddressSpace associated with codeGen object");
      }

      BinaryEdit *binEdit = addrSpace->edit();
      Address relocation_address;

      unsigned int jump_slot_size;
      switch(addrSpace->getAddressWidth()) {
        case 4:
          jump_slot_size = 4;
          break; // l: not needed
        case 8:
          jump_slot_size = 24;
          break;
        default:
          assert(0 && "Encountered unknown address width");
      }

      if(!binEdit || !func) {
        assert(!"Invalid function call (function info is missing)");
      }

      // find the Symbol corresponding to the func_instance
      std::vector<SymtabAPI::Symbol *> syms;
      func->ifunc()->func()->getSymbols(syms);

      if(syms.size() == 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "%s[%d]:  internal error:  cannot find symbol %s", __FILE__,
                 __LINE__, func->symTabName().c_str());
        showErrorCallback(80, msg);
        assert(0);
      }

      // try to find a dynamic symbol
      // (take first static symbol if none are found)
      SymtabAPI::Symbol *referring = syms[0];
      for(unsigned k = 0; k < syms.size(); k++) {
        if(syms[k]->isInDynSymtab()) {
          referring = syms[k];
          break;
        }
      }
      // have we added this relocation already?
      relocation_address = binEdit->getDependentRelocationAddr(referring);

      if(!relocation_address) {
        // inferiorMalloc addr location and initialize to zero
        relocation_address = binEdit->inferiorMalloc(jump_slot_size);
        unsigned char dat[24] = {0};
        binEdit->writeDataSpace((void *)relocation_address, jump_slot_size, dat);
        // add write new relocation symbol/entry
        binEdit->addDependentRelocation(relocation_address, referring);
      }
      return relocation_address;
    }

    Address EmitterAarch64::getInterModuleVarAddr(const image_variable *var, codeGen &gen) {
      AddressSpace *addrSpace = gen.addrSpace();
      if(!addrSpace) {
        assert(0 && "No AddressSpace associated with codeGen object");
      }

      BinaryEdit *binEdit = addrSpace->edit();
      Address relocation_address;

      unsigned int jump_slot_size;
      switch(addrSpace->getAddressWidth()) {
        case 4:
          jump_slot_size = 4;
          break;
        case 8:
          jump_slot_size = 8;
          break;
        default:
          assert(0 && "Encountered unknown address width");
      }

      if(!binEdit || !var) {
        assert(!"Invalid variable load (variable info is missing)");
      }

      // find the Symbol corresponding to the int_variable
      std::vector<SymtabAPI::Symbol *> syms;
      var->svar()->getSymbols(syms);

      if(syms.size() == 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "%s[%d]:  internal error:  cannot find symbol %s", __FILE__,
                 __LINE__, var->symTabName().c_str());
        showErrorCallback(80, msg);
        assert(0);
      }

      // try to find a dynamic symbol
      // (take first static symbol if none are found)
      SymtabAPI::Symbol *referring = syms[0];
      for(unsigned k = 0; k < syms.size(); k++) {
        if(syms[k]->isInDynSymtab()) {
          referring = syms[k];
          break;
        }
      }

      // have we added this relocation already?
      relocation_address = binEdit->getDependentRelocationAddr(referring);

      if(!relocation_address) {
        // inferiorMalloc addr location and initialize to zero
        relocation_address = binEdit->inferiorMalloc(jump_slot_size);
        unsigned char dat[8] = {0};
        binEdit->writeDataSpace((void *)relocation_address, jump_slot_size, dat);

        // add write new relocation symbol/entry
        binEdit->addDependentRelocation(relocation_address, referring);
      }

      return relocation_address;
    }

}}
