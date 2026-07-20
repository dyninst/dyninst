#include "arch-power.h"
#include "binaryEdit.h"
#include "codegen/codegen-power.h"
#include "codegen/emitters/PowerPC/generators.h"
#include "common/src/bitmath.h"
#include "debug.h"
#include "dynproc/dynProcess.h"
#include "EmitterPowerPC.h"
#include "inst-power.h"
#include "RegisterConversion.h"
#include "registerSpace/registerSpace.h"

struct parsed_regs {
  std::set<Dyninst::Register> gprs, fprs;
};

static parsed_regs calcUsedRegs(parse_func *func);
static bool can_optimize_as_shift(RegValue val, uint8_t max_num_bits);

namespace Dyninst { namespace DyninstAPI {

    bool EmitterPowerPC::clobberAllFuncCall(registerSpace *rs, func_instance *callee) {
      if(!callee) {
        return true;
      }

      if(clobbered_functions.contains(callee)) {
        return true;
      }
      clobbered_functions.insert(callee);

      /* usedRegs does calculations if not done before and returns
         whether or not the callee is a leaf function.
         if it is, we use the register info we gathered,
         otherwise, we punt and save everything */
      if(callee->ifunc()->isLeafFunc()) {
        auto used_regs = calcUsedRegs(callee->ifunc());
        for(Dyninst::Register r : used_regs.gprs) {
          rs->GPRs()[r]->beenUsed = true;
        }

        for(Dyninst::Register r : used_regs.fprs) {
          rs->FPRs()[registerSpace::FPR(r)]->beenUsed = true;
        }
      } else {
        for(int i = 0; i < rs->numGPRs(); i++) {
          rs->GPRs()[i]->beenUsed = true;
        }
        for(int i = 0; i < rs->numFPRs(); i++) {
          rs->FPRs()[i]->beenUsed = true;
        }
      }
      return false;
    }

    codeBufIndex_t EmitterPowerPC::emitA(opCode op, Dyninst::Register src1, long dest, codeGen &gen,
                                         Dyninst::DyninstAPI::RegControl) {
      codeBufIndex_t retval = 0;
      switch(op) {
        case ifOp: {
          // cmpi 0,0,src1,0
          instruction insn;
          insn.clear();
          DFORM_OP_SET(insn, CMPIop);
          DFORM_RA_SET(insn, src1);
          DFORM_SI_SET(insn, 0);
          insnCodeGen::generate(gen, insn);
          retval = gen.getIndex();

          // be 0, dest
          insn.clear();
          BFORM_OP_SET(insn, BCop);
          BFORM_BO_SET(insn, BTRUEcond);
          BFORM_BI_SET(insn, EQcond);
          BFORM_BD_SET(insn, dest / 4);
          BFORM_AA_SET(insn, 0);
          BFORM_LK_SET(insn, 0);

          insnCodeGen::generate(gen, insn);
          break;
        }
        case branchOp: {
          retval = gen.getIndex();
          insnCodeGen::generateBranch(gen, dest);
          break;
        }
        default:
          assert(0); // unexpected op for this emit!
      }
      return retval;
    }

    // VG(11/07/01): Load in destination the effective address given
    // by the address descriptor. Used for memory access stuff.
    void EmitterPowerPC::emitAddrSpecLoad(const BPatch_addrSpec_NP *as, Dyninst::Register dest,
                                          int stackShift, codeGen &gen) {
      // Haven't implemented non-zero shifts yet
      assert(stackShift == 0);
      // instruction *insn = (instruction *) ((void*)&gen[base]);
      int imm = as->getImm();
      int ra = as->getReg(0);
      int rb = as->getReg(1);
      // TODO: optimize this to generate the minimum number of
      // instructions; think about schedule

      // emit code to load the immediate (constant offset) into dest; this
      // writes at gen+base and updates base, we must update insn...
      gen.emitter()->emitVload(loadConstOp, (Address)imm, dest, dest, gen);

      // If ra is used in the address spec, allocate a temp register and
      // get the value of ra from stack into it
      if(ra > -1) {
        ppc::emitAddOriginal(ra, dest, gen);
      }

      // If rb is used in the address spec, allocate a temp register and
      // get the value of ra from stack into it
      if(rb > -1) {
        ppc::emitAddOriginal(rb, dest, gen);
      }
    }

    void EmitterPowerPC::emitAddSignedImm(Address, int, codeGen &) {
      assert(0);
    }

    bool EmitterPowerPC::emitAdjustStackPointer(int, codeGen &) {
      assert(0);
      return true;
    }

    void EmitterPowerPC::emitASload(int, int, int, long, Register, int, codeGen &) {
      assert(0);
    }

    bool EmitterPowerPC::emitBTRestores(baseTramp *, codeGen &) {
      assert(0);
      return true;
    }

    bool EmitterPowerPC::emitBTSaves(baseTramp *, codeGen &) {
      assert(0);
      return true;
    }

    // There are four "axes" going on here:
    // 32 bit vs 64 bit
    // Instrumentation vs function call replacement
    // Static vs. dynamic
    Dyninst::Register EmitterPowerPC::emitCall(opCode ocode, codeGen &gen,
                                               const std::vector<codeGenASTPtr> &operands,
                                               func_instance *callee) {
      bool inInstrumentation = true;

      // If inInstrumentation is true we're in instrumentation;
      // if false we're in function call replacement

      if(ocode == funcJumpOp) {
        return emitCallReplacement(ocode, gen, callee);
      }

      //  Sanity check for NULL address argument
      if(!callee) {
        char msg[256];
        sprintf(msg,
                "%s[%d]:  internal error:  emitFuncCall called w/out"
                "callee argument",
                __FILE__, __LINE__);
        showErrorCallback(80, msg);
        assert(0);
      }

      // Now that we have the destination address (unique, hopefully)
      // get the TOC anchor value for that function
      // The TOC offset is stored in the Object.
      // file() -> pdmodule "parent"
      // exec() -> image "parent"

      Address toc_anchor = 0;
      Address caller_toc = 0;
      std::vector<Dyninst::Register> srcs;

      // Linux, 64, static/dynamic, inst/repl
      // DYN
      toc_anchor = gen.addrSpace()->getTOCoffsetInfo(callee);

      // Instead of saving the TOC (if we can't), just reset it afterwards.
      if(gen.func()) {
        caller_toc = gen.addrSpace()->getTOCoffsetInfo(gen.func());
      } else if(gen.point()) {
        caller_toc = gen.addrSpace()->getTOCoffsetInfo(gen.point()->func());
      } else {
        // Don't need it, and this might be an iRPC
      }

      inst_printf("Caller TOC 0x%lx; callee 0x%lx\n", caller_toc, toc_anchor);
      // ALL
      bool needToSaveLR = false;
      registerSlot *regLR = (*(gen.rs()))[registerSpace::lr];
      if(regLR && regLR->liveState == registerSlot::live) {
        needToSaveLR = true;
        inst_printf("... need to save LR\n");
      }

      // Note: For 32-bit ELF PowerPC Linux (and other SYSV ABI followers)
      // r2 is described as "reserved for system use and is not to be
      // changed by application code".
      // On these platforms, we return 0 when getTOCoffsetInfo is called.

      std::vector<int> savedRegs;

      //  Save the link register.
      // mflr r0
      // Linux, 32/64, stat/dynamic, instrumentation
      if(needToSaveLR) {
        assert(inInstrumentation);
        insnCodeGen::generateMoveFromLR(gen, 0);
        ppc::saveRegister(gen, 0, FUNC_CALL_SAVE(gen.width()));
        savedRegs.push_back(0);
        inst_printf("saved LR in 0\n");
      }

      if(inInstrumentation && (toc_anchor != caller_toc)) {
        // Save register 2 (TOC)
        ppc::saveRegister(gen, 2, FUNC_CALL_SAVE(gen.width()));
        savedRegs.push_back(2);
      }

      // see what others we need to save.
      for(int i = 0; i < gen.rs()->numGPRs(); i++) {
        registerSlot *reg = gen.rs()->GPRs()[i];

        // We must save if:
        // refCount > 0 (and not a source register)
        // keptValue == true (keep over the call)
        // liveState == live (technically, only if not saved by the callee)

        if(inInstrumentation &&
           ((reg->refCount > 0) || reg->keptValue || (reg->liveState == registerSlot::live))) {
          ppc::saveRegister(gen, reg->number, FUNC_CALL_SAVE(gen.width()));
          savedRegs.push_back(reg->number);
        }
      }

      // Generate the code for all function parameters, and keep a list
      // of what registers they're in.
      for(unsigned u = 0; u < operands.size(); u++) {
        // Note: if we're in function replacement, we can assert operands.empty()
        // Try to target the code generation

        Dyninst::Register reg = Dyninst::Null_Register;
        // Try to allocate the correct parameter register
        if(gen.rs()->allocateSpecificRegister(gen, registerSpace::r3 + u)) {
          reg = registerSpace::r3 + u;
        }
        Address unused = ADDR_NULL;
        if(!operands[u]->generateCode_phase2(gen, unused, reg)) {
          assert(0);
        }
        assert(reg != Dyninst::Null_Register);
        srcs.push_back(reg);
      }

      if(srcs.size() > 8) {
        // This is not necessarily true; more then 8 arguments could be passed,
        // the first 8 need to be in registers while the others need to be on
        // the stack, -- sec 3/1/97
        std::string msg =
            "Too many arguments to function call in instrumentation code:"
            " only 8 arguments can (currently) be passed on the POWER architecture.\n";
        bperr("%s", msg.c_str());
        showErrorCallback(94, msg);
        exit(-1);
      }

      // If we got the wrong register, we may need to do a 3-way swap.

      int scratchRegs[8];
      for(int a = 0; a < 8; a++) {
        scratchRegs[a] = -1;
      }

      // Now load the parameters into registers.
      for(unsigned u = 0; u < srcs.size(); u++) {

        // Parameters start at register 3 - so we're already done
        // in this case
        if(srcs[u] == (registerSpace::r3 + u)) {
          gen.rs()->freeRegister(srcs[u]);
          continue;
        }

        int whichSource = -1;
        bool hasSourceBeenCopied = true;

        // If the parameter we want exists in a scratch register...
        if(scratchRegs[u] != -1) {
          insnCodeGen::generateImm(gen, ORILop, scratchRegs[u], u + 3, 0);
          gen.rs()->freeRegister(scratchRegs[u]);
          // We should check to make sure the one we want isn't occupied?
        } else {
          for(unsigned v = u; v < srcs.size(); v++) {
            if(srcs[v] == u + 3) {
              // Okay, so the source we want is actuall in srcs[v]
              hasSourceBeenCopied = false;
              whichSource = v;
              break;
            }
          }
          // Ummm... we didn't find it? Ah, so copying us (since we're wrong)
          // into scratch.
          if(!hasSourceBeenCopied) {
            Dyninst::Register scratch = gen.rs()->getScratchRegister(gen);
            insnCodeGen::generateImm(gen, ORILop, u + 3, scratch, 0);
            gen.rs()->freeRegister(u + 3);
            scratchRegs[whichSource] = scratch;
            hasSourceBeenCopied = true;

            insnCodeGen::generateImm(gen, ORILop, srcs[u], u + 3, 0);
            gen.rs()->freeRegister(srcs[u]);

          } else {
            insnCodeGen::generateImm(gen, ORILop, srcs[u], u + 3, 0);
            gen.rs()->freeRegister(srcs[u]);
          }
        }
      }

      // Call generation time.
      bool setTOC = false;

      // Linux, 64, stat/dyn, inst/repl
      if(toc_anchor != caller_toc) {
        // fprintf(stderr, "info: %s:%d: \n", __FILE__, __LINE__);
        setTOC = true;
      }

      emitCallInstruction(gen, callee, setTOC, toc_anchor);
      // ALL instrumentation
      Dyninst::Register retReg = Dyninst::Null_Register;
      if(inInstrumentation) {
        // get a register to keep the return value in.
        retReg = gen.rs()->allocateRegister(gen);
        // put the return value from register 3 to the newly allocated register.
        insnCodeGen::generateImm(gen, ORILop, 3, retReg, 0);
      }

      // Otherwise we're replacing a call and so we don't want to move
      // anything.

      // restore saved registers.
      // If inInstrumentation == false then this vector should be empty...
      // ALL instrumentation

      if(!inInstrumentation) {
        assert(savedRegs.size() == 0);
      }
      for(u_int ui = 0; ui < savedRegs.size(); ui++) {
        ppc::restoreRegister(gen, savedRegs[ui], FUNC_CALL_SAVE(gen.width()));
      }

      // mtlr 0 (aka mtspr 8, rs) = 0x7c0803a6
      // Move to link register
      // Reused from above. instruction mtlr0(MTLR0raw);
      if(needToSaveLR) {
        // We only use register 0 to save LR.
        insnCodeGen::generateMoveToLR(gen, 0);
      }

      if(!inInstrumentation && setTOC) {
        // Need to reset the TOC
        gen.emitter()->emitVload(loadConstOp, caller_toc, 2, 2, gen);

        // Also store toc_orig [r2] into the TOC save area [40(r1)].
        // Subsequent code will look for it there.
        ppc::saveRegisterAtOffset(gen, 2, 40);
      }

      // return value is the register with the return value from the called function
      return (retReg);
    }

    // Generates call instruction sequence for all POWER-based systems
    // under dynamic instrumentation.
    //
    // This should be able to stomp on the link register (LR) and TOC
    // register (r2), as they were saved by Emitter::emitCall() as necessary.
    bool EmitterPowerPC::emitCallInstruction(codeGen &gen, func_instance *callee, bool setTOC,
                                             Address toc_anchor) {

      bool needLongBranch = false;
      if(gen.startAddr() == (Address)-1) { // Unset...
        needLongBranch = true;
        inst_printf("Unknown generation addr, long call required\n");
      } else {
        long displacement = callee->addr() - gen.currAddr();
        // Increase the displacement to be conservative.
        // We use fewer than 6 instructions, too. But again,
        // conservative.
        if((ABS(displacement) + 6 * instruction::size()) > MAX_BRANCH) {
          needLongBranch = true;
          inst_printf("Need long call to get from 0x%lx to 0x%lx\n", gen.currAddr(),
                      callee->addr());
        }
      }
      // Need somewhere to put the destination calculation...
      int scratchReg = 12;
      if(needLongBranch) {
        // Use scratchReg to set destination of the call...
        inst_printf(
            "[EmitterPowerPC::EmitCallInstruction] needLongBranch, Emitting VLOAD  "
            "Callee: 0x%lx, "
            "ScratchReg: %u\n",
            callee->addr(), (unsigned)scratchReg);
        gen.emitter()->emitVload(loadConstOp, callee->addr(), scratchReg, scratchReg, gen);
        insnCodeGen::generateMoveToLR(gen, scratchReg);

        inst_printf("Generated LR value in %d\n", scratchReg);
      }

      // Linux 64
      if(setTOC) {
        inst_printf(
            "[EmitterPowerPC::EmitCallInstruction] Setting TOC anchor, toc_anchor: "
            "%lx, register: "
            "2(fixed)\n",
            (uint64_t)toc_anchor);
        // Set up the new TOC value
        gen.emitter()->emitVload(loadConstOp, toc_anchor, 2, 2, gen);
        inst_printf("Set new TOC\n");
      }

      // ALL dynamic; call instruction generation
      if(needLongBranch) {
        gen.emitter()->emitVload(loadConstOp, callee->addr(), 12, 12, gen);
        instruction brl(BRLraw);
        insnCodeGen::generate(gen, brl);
        inst_printf("Generated BRL\n");
      } else {
        inst_printf(
            "[EmitterPowerPC::EmitCallInstruction] Generating Call, curAddress: "
            "%lx, calleeAddr: "
            "%lx\n",
            gen.currAddr(), callee->addr());
        gen.emitter()->emitVload(loadConstOp, callee->addr(), 12, 12, gen);
        insnCodeGen::generateCall(gen, gen.currAddr(), callee->addr());

        inst_printf("Generated short call from 0x%lx to 0x%lx\n", gen.currAddr(), callee->addr());
      }

      return true;
    }

    bool EmitterPowerPC::emitCallRelative(Dyninst::Register dest, Address offset,
                                          Dyninst::Register base, codeGen &gen) {
      // Loads a saved register from the stack.
      int imm = offset;
      if(gen.width() == 4) {
        if(((signed)MIN_IMM16 <= (signed)imm) && ((signed)imm <= (signed)MAX_IMM16)) {
          insnCodeGen::generateImm(gen, CALop, dest, base, imm);

        } else if(((signed)MIN_IMM32 <= (signed)imm) && ((signed)imm <= (signed)MAX_IMM32)) {
          insnCodeGen::generateImm(gen, CAUop, dest, 0, BOT_HI(offset));
          insnCodeGen::generateImm(gen, ORILop, dest, dest, BOT_LO(offset));
          insnCodeGen::generateAddReg(gen, CAXop, dest, dest, base);
        } else {
          assert(0);
        }
      }
      return true;
    }

    Dyninst::Register EmitterPowerPC::emitCallReplacement(opCode ocode, codeGen &gen,
                                                          func_instance *callee) {
      // This takes care of the special case where we are replacing an existing
      // linking branch instruction.
      //
      // This code makes two crucial assumptions:
      // 1) LR is free: Linking branch instructions place pre-branch IP in LR.
      // 2) TOC (r2) is free: r2 should hold TOC of destination.  So use it
      //    as scratch, and set it to destination module's TOC upon return.
      //    This works for both the inter and intra module call cases.
      // In the 32-bit case where we can't use r2, stomp on r0 and pray...

      //  Sanity check for opcode.
      assert(ocode == funcJumpOp);

      Dyninst::Register freeReg = 0;
      instruction mtlr(MTLR0raw);

      // 64-bit Mutatees
      if(gen.addrSpace()->proc()->getAddressWidth() == 8) {
        freeReg = 2;
        mtlr = instruction(MTLR2raw);
      }

      // Load register with address.
      gen.emitter()->emitVload(loadConstOp, callee->addr(), freeReg, freeReg, gen);

      // Move to link register.
      insnCodeGen::generate(gen, mtlr);

      Address toc_new = gen.addrSpace()->proc()->getTOCoffsetInfo(callee);
      if(toc_new) {
        // Set up the new TOC value
        gen.emitter()->emitVload(loadConstOp, toc_new, freeReg, freeReg, gen);
      }

      // blr - branch through the link reg.
      instruction blr(BRraw);
      insnCodeGen::generate(gen, blr);

      func_instance *caller = gen.point()->func();
      Address toc_orig = gen.addrSpace()->proc()->getTOCoffsetInfo(caller);
      if(toc_new) {
        // Restore the original TOC value.
        gen.emitter()->emitVload(loadConstOp, toc_orig, freeReg, freeReg, gen);
      }

      // What to return here?
      return Dyninst::Null_Register;
    }

    void EmitterPowerPC::emitCallWithSaves(codeGen &gen, Address dest, bool saveToc, bool saveLR,
                                           bool saveR12) {
      // Save the values onto the stack.... (might be needed).
      if(saveToc) {
      }
      if(saveLR) {
      }
      if(saveR12) {
      }

      gen.emitter()->emitVload(loadConstOp, dest, 0, 0, gen);
      insnCodeGen::generateMoveToLR(gen, 0);
      gen.emitter()->emitVload(loadConstOp, dest, 12, 12, gen);
      instruction brl(BRLraw);
      insnCodeGen::generate(gen, brl);
      inst_printf("Generated BRL\n");
      // Retore the original
      if(saveToc) {
      }
      if(saveLR) {
      }
      if(saveR12) {
      }
    }

    void EmitterPowerPC::emitCountSpecLoad(const BPatch_addrSpec_NP *as, Dyninst::Register dest,
                                           codeGen &gen) {
      gen.emitter()->emitAddrSpecLoad(as, dest, 0, gen);
    }

    void EmitterPowerPC::emitCSload(int, int, int, long, Register, codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitDiv(Register, Register, Register, codeGen &, bool) {
      assert(0);
    }

    void EmitterPowerPC::emitDivImm(Register, Register, RegValue, codeGen &, bool) {
      assert(0);
    }

    void EmitterPowerPC::emitGetParam(Register, Register, instPoint::Type, opCode, bool,
                                      codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitGetRetAddr(Register, codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitGetRetVal(Register, bool, codeGen &) {
      assert(0);
    }

    codeBufIndex_t EmitterPowerPC::emitIf(Register, Register, Dyninst::DyninstAPI::RegControl,
                                          codeGen &) {
      assert(0);
      return 0;
    }

    void EmitterPowerPC::emitImm(opCode op, Dyninst::Register src1, RegValue src2imm,
                                 Dyninst::Register dest, codeGen &gen, bool s) {
      int iop = -1;
      switch(op) {
          // integer ops
        case plusOp:
          iop = CALop;
          insnCodeGen::generateImm(gen, iop, dest, src1, src2imm);
          return;
          break;

        case minusOp:
          iop = SIop;
          insnCodeGen::generateImm(gen, iop, dest, src1, src2imm);
          return;
          break;
        case timesOp: {
          if(can_optimize_as_shift(src2imm, gen.width() * 8)) {
            const uint8_t num_bits_to_shift = *Dyninst::ilog2(src2imm);
            insnCodeGen::generateLShift(gen, src1, num_bits_to_shift, dest);
            return;
          } else {
            Dyninst::Register dest2 = gen.rs()->getScratchRegister(gen);
            gen.emitter()->emitVload(loadConstOp, src2imm, dest2, dest2, gen);
            emitV(op, src1, dest2, dest, gen, gen.width(), gen.addrSpace(), s);
            return;
          }
          break;
        }
        case divOp: {
          Dyninst::Register dest2 = gen.rs()->getScratchRegister(gen);
          gen.emitter()->emitVload(loadConstOp, src2imm, dest2, dest2, gen);
          emitV(op, src1, dest2, dest, gen, gen.width(), gen.addrSpace(), s);
          return;
        }
          // Bool ops
        case orOp:
          iop = ORILop;
          // For some reason, the destField is 2nd for ORILop and ANDILop
          insnCodeGen::generateImm(gen, iop, src1, dest, src2imm);
          return;
          break;

        case andOp:
          iop = ANDILop;
          // For some reason, the destField is 2nd for ORILop and ANDILop
          insnCodeGen::generateImm(gen, iop, src1, dest, src2imm);
          return;
          break;
        default:
          Dyninst::Register dest2 = gen.rs()->getScratchRegister(gen);
          gen.emitter()->emitVload(loadConstOp, src2imm, dest2, dest2, gen);
          emitV(op, src1, dest2, dest, gen, gen.width(), gen.addrSpace(), s);
          return;
          break;
      }
    }

    void EmitterPowerPC::emitLoad(Register, Address, int, codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitLoadConst(Register, Address, codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitLoadFrameAddr(Register, Address, codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitLoadIndir(Register, Register, int, codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitLoadOrigFrameRelative(Register, Address, codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitLoadOrigRegister(Address, Register, codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitLoadOrigRegRelative(Register, Address, Register, codeGen &, bool) {
      assert(0);
    }

    bool EmitterPowerPC::emitLoadRelative(Dyninst::Register dest, Address offset,
                                          Dyninst::Register base, int size, codeGen &gen) {
      if(((long)MIN_IMM16 <= (long)offset) && ((long)offset <= (long)MAX_IMM16)) {
        int ocode = Lop;
        switch(size) {
          case 1:
            ocode = LBZop;
            break;
          case 2:
            ocode = LHZop;
            break;
          case 4:
            ocode = Lop;
            break;
          case 8:
            ocode = LDop;
            break;
          default:
            return false;
            break;
        }
        insnCodeGen::generateImm(gen, ocode, dest, base, offset);
      } else {
        // Add the offset to the base register, which holds
        // the current PC
        insnCodeGen::generateImm(gen, CAUop, base, base, HA(offset));
        insnCodeGen::generateImm(gen, CALop, base, base, LOW(offset));

        int ocode = LXop;
        int xcode = 0;
        switch(size) {
          case 1:
            xcode = LBZXxop;
            break;
          case 2:
            xcode = LHZXxop;
            break;
          case 4:
            xcode = LXxop;
            break;
          case 8:
            xcode = LDXxop;
            break;
          default:
            printf(" Unrecognized size for load operation(%d). Assuming size of 4 \n", size);
            return false;
            break;
        }

        instruction insn;
        insn.clear();
        XFORM_OP_SET(insn, ocode);
        XFORM_RT_SET(insn, dest);
        XFORM_RA_SET(insn, 0);
        XFORM_RB_SET(insn, base);
        XFORM_XO_SET(insn, xcode);
        XFORM_RC_SET(insn, 0);
        insnCodeGen::generate(gen, insn);
      }
      return true;
    }

    void EmitterPowerPC::emitLoadShared(opCode op, Dyninst::Register dest,
                                        const image_variable *var, bool is_local, int size,
                                        codeGen &gen, Address offset) {
      // create or retrieve jump slot
      Address addr;
      int stackSize = 0;

      if(var == NULL) {
        addr = offset;
      } else if(!is_local) {
        addr = getInterModuleVarAddr(var, gen);
      } else {
        addr = (Address)var->getOffset();
      }

      // load register with address from jump slot

      inst_printf("emitLoadShared addr 0x%lx curr adress 0x%lx offset %lu 0x%lx size %d\n", addr,
                  gen.currAddr(), addr - gen.currAddr() + 4, addr - gen.currAddr() + 4, size);
      Dyninst::Register scratchReg = gen.rs()->getScratchRegister(gen);

      if(scratchReg == Dyninst::Null_Register) {
        std::vector<Dyninst::Register> freeReg;
        std::vector<Dyninst::Register> excludeReg;
        stackSize = insnCodeGen::createStackFrame(gen, 1, freeReg, excludeReg);
        assert(stackSize == 1);
        scratchReg = freeReg[0];
        inst_printf(
            "emitLoadrelative - after new stack frame - addr 0x%lx curr adress "
            "0x%lx offset %lu "
            "0x%lx size %d\n",
            addr, gen.currAddr(), addr - gen.currAddr() + 4, addr - gen.currAddr() + 4, size);
      }

      emitMovePCToReg(scratchReg, gen);
      Address varOffset = addr - gen.currAddr() + 4;

      if(op == loadOp) {
        if(!is_local && (var != NULL)) {

          emitLoadRelative(dest, varOffset, scratchReg, gen.width(), gen);
          // Deference the pointer to get the variable
          emitLoadRelative(dest, 0, dest, size, gen);
        } else {

          emitLoadRelative(dest, varOffset, scratchReg, size, gen);
        }
      } else { // loadConstop
        if(!is_local && (var != NULL)) {

          emitLoadRelative(dest, varOffset, scratchReg, gen.width(), gen);
        } else {

          // Move address of the variable into the register - load effective address
          // dest = effective address of pc+offset ;
          insnCodeGen::generateImm(gen, CAUop, dest, 0, BOT_HI(varOffset));
          insnCodeGen::generateImm(gen, ORILop, dest, dest, BOT_LO(varOffset));
          insnCodeGen::generateAddReg(gen, CAXop, dest, dest, scratchReg);
        }
      }

      if(stackSize > 0) {
        insnCodeGen::removeStackFrame(gen);
      }

      return;
    }

    Address EmitterPowerPC::emitMovePCToReg(Dyninst::Register dest, codeGen &gen) {
      insnCodeGen::generateBranch(gen, gen.currAddr(), gen.currAddr() + 4, true); // blrl
      Address ret = gen.currAddr();
      insnCodeGen::generateMoveFromLR(gen, dest); // mflr
      return ret;
    }

    bool EmitterPowerPC::emitMoveRegToReg(Register, Register, codeGen &) {
      assert(0);
      return false;
    }

    bool EmitterPowerPC::emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen) {
      assert(dest->type == registerSlot::GPR);

      switch(src->type) {
        case registerSlot::GPR:
          insnCodeGen::generateImm(gen, ORILop, src->encoding(), dest->encoding(), 0);
          break;
        case registerSlot::SPR: {
          instruction insn;

          switch(src->number) {
            case registerSpace::lr:
            case registerSpace::xer:
            case registerSpace::ctr:
            case registerSpace::mq:
              insn.clear();
              XFORM_OP_SET(insn, EXTop);
              XFORM_RT_SET(insn, dest->encoding());
              XFORM_RA_SET(insn, src->encoding() & 0x1f);
              XFORM_RB_SET(insn, (src->encoding() >> 5) & 0x1f);
              XFORM_XO_SET(insn, MFSPRxop);
              insnCodeGen::generate(gen, insn);
              break;
            case registerSpace::cr:
              insn.clear(); // mtcrf:  scratchReg
              XFXFORM_OP_SET(insn, EXTop);
              XFXFORM_RT_SET(insn, dest->encoding());
              XFXFORM_XO_SET(insn, MFCRxop);
              insnCodeGen::generate(gen, insn);
              break;
            default:
              assert(0);
              break;
          }
          break;
        }

        default:
          assert(0);
          break;
      }
      return true;
    }

    void EmitterPowerPC::emitOp(unsigned, Register, Register, Register, codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitOpImm(unsigned, unsigned, Register, Register, RegValue, codeGen &) {
      assert(0);
    }

    bool EmitterPowerPC::emitPop(codeGen &, Register) {
      assert(0);
      return true;
    }

    bool EmitterPowerPC::emitPush(codeGen &, Register) {
      assert(0);
      return true;
    }

    void EmitterPowerPC::emitPushFlags(codeGen &) {
      assert(0);
    }

    Dyninst::Register EmitterPowerPC::emitR(opCode op, Dyninst::Register src1,
                                            Dyninst::Register src2, Dyninst::Register dest,
                                            codeGen &gen, const instPoint * /*location*/) {

      registerSlot *regSlot = NULL;
      unsigned addrWidth = gen.width();

      switch(op) {
        case getRetValOp: {
          regSlot = (*(gen.rs()))[registerSpace::r3];
          break;
        }

        case getParamOp: {
          // The first 8 parameters (0-7) are stored in registers (r3-r10) upon
          // entering the function and then saved above the stack upon entering
          // the trampoline; in emit functional call the stack pointer is moved
          // so the saved registers are not over-written the other parameters >
          // 8 are stored on the caller's stack at an offset.
          //
          // src1 is the argument number 0..X, the first 8 are stored in regs
          // src2 (if not Dyninst::Null_Register) holds the value to be written into src1

          if(src1 < 8) {
            // src1 is 0..8 - it's a parameter number, not a register
            regSlot = (*(gen.rs()))[registerSpace::r3 + src1];
            break;

          } else {
            // Registers from 11 (src = 8) and beyond are saved on the stack.

            int stkOffset;
            if(addrWidth == 4) {
              stkOffset = TRAMP_FRAME_SIZE_32 + (src1 - 8) * sizeof(int) + PARAM_OFFSET(addrWidth);
            } else {
              // Linux ABI says:
              // Parameters go in the "argument save area", which starts at
              // PARAM_OFFSET(...). However, we'd save argument _0_ at the base
              // of it, so the first 8 slots are normally empty (as they go in
              // registers). To get the 9th, etc. argument you want
              // PARAM_OFFSET(...) + (8 * arg number) instead of
              // 8 * (arg_number - 8)
              int stackSlot = src1;
              stkOffset = TRAMP_FRAME_SIZE_64 + stackSlot * sizeof(long) + PARAM_OFFSET(addrWidth);
            }

            if(src2 != Dyninst::Null_Register) {
              ppc::saveRegisterAtOffset(gen, src2, stkOffset);
            }
            ppc::restoreRegisterAtOffset(gen, dest, stkOffset);
            return (dest);
          }
          break;
        }

        case getRetAddrOp: {
          regSlot = (*(gen.rs()))[registerSpace::lr];
          break;
        }

        default:
          assert(0);
          break;
      }

      assert(regSlot);
      Dyninst::Register reg = regSlot->number;

      switch(regSlot->liveState) {
        case registerSlot::spilled: {
          int offset = TRAMP_GPR_OFFSET(addrWidth);

          // its on the stack so load it.
          if(src2 != Dyninst::Null_Register) {
            ppc::saveRegister(gen, src2, reg, offset);
          }
          ppc::restoreRegister(gen, reg, dest, offset);
          return (dest);
        }
        case registerSlot::live: {
          // its still in a register so return the register it is in.

          return (reg);
        }
        case registerSlot::dead: {
          // Uhhh... wha?
          assert(0);
        }
      }

      assert(0);
      return Dyninst::Null_Register;
    }

    void EmitterPowerPC::emitRelOp(unsigned, Register, Register, Register, codeGen &, bool) {
      assert(0);
    }

    void EmitterPowerPC::emitRelOpImm(unsigned, Register, Register, RegValue, codeGen &, bool) {
      assert(0);
    }

    void EmitterPowerPC::emitRestoreFlags(codeGen &, unsigned) {
      assert(0);
    }

    void EmitterPowerPC::emitRestoreFlagsFromStackSlot(codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitStore(Address, Register, int, codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitStoreFrameRelative(Address, Register, Register, int, codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitStoreImm(Address, int, codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitStoreIndir(Register, Register, int, codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitStoreOrigRegister(Address, Register, codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitStoreRelative(Dyninst::Register source, Address offset,
                                           Dyninst::Register base, int size, codeGen &gen) {
      if(((long)MIN_IMM16 <= (long)offset) && ((long)offset <= (long)MAX_IMM16)) {
        int ocode = STop;
        switch(size) {
          case 1:
            ocode = STBop;
            break;
          case 2:
            ocode = STHop;
            break;
          case 4:
            ocode = STop;
            break;
          case 8:
            ocode = STDop;
            break;
          default:
            assert(0);
            break;
        }
        insnCodeGen::generateImm(gen, ocode, source, base, offset);
      } else {

        // Add the offset to the base register, which holds
        // the current PC
        insnCodeGen::generateImm(gen, CAUop, base, base, HA(offset));
        insnCodeGen::generateImm(gen, CALop, base, base, LOW(offset));

        int ocode = STXop;
        int xcode = 0;
        switch(size) {
          case 1:
            xcode = STBXxop;
            break;
          case 2:
            xcode = STHXxop;
            break;
          case 4:
            xcode = STXxop;
            break;
          case 8:
            xcode = STDXxop;
            break;
          default:
            printf(" Unrecognized size for load operation(%d). Assuming size of 4 \n", size);
            assert(0);
            break;
        }

        instruction insn;
        insn.clear();
        XFORM_OP_SET(insn, ocode);
        XFORM_RT_SET(insn, source);
        XFORM_RA_SET(insn, 0);
        XFORM_RB_SET(insn, base);
        XFORM_XO_SET(insn, xcode);
        XFORM_RC_SET(insn, 0);
        insnCodeGen::generate(gen, insn);
      }
    }

    void EmitterPowerPC::emitStoreShared(Dyninst::Register source, const image_variable *var,
                                         bool is_local, int size, codeGen &gen) {
      // create or retrieve jump slot
      Address addr;
      int stackSize = 0;
      if(!is_local) {
        addr = getInterModuleVarAddr(var, gen);
      } else {
        addr = (Address)var->getOffset();
      }

      inst_printf("emitStoreRelative addr 0x%lx curr adress 0x%lx offset %lu 0x%lx size %d\n", addr,
                  gen.currAddr(), addr - gen.currAddr() + 4, addr - gen.currAddr() + 4, size);

      // load register with address from jump slot
      Dyninst::Register scratchReg = gen.rs()->getScratchRegister(gen);
      if(scratchReg == Dyninst::Null_Register) {
        std::vector<Dyninst::Register> freeReg;
        std::vector<Dyninst::Register> excludeReg;
        stackSize = insnCodeGen::createStackFrame(gen, 1, freeReg, excludeReg);
        assert(stackSize == 1);
        scratchReg = freeReg[0];

        inst_printf(
            "emitStoreRelative - after new stack frame- addr 0x%lx curr adress "
            "0x%lx offset %lu "
            "0x%lx size %d\n",
            addr, gen.currAddr(), addr - gen.currAddr() + 4, addr - gen.currAddr() + 4, size);
      }

      emitMovePCToReg(scratchReg, gen);
      Address varOffset = addr - gen.currAddr() + 4;

      if(!is_local) {
        std::vector<Dyninst::Register> exclude;
        exclude.push_back(scratchReg);
        Dyninst::Register scratchReg1 = gen.rs()->getScratchRegister(gen, exclude);
        if(scratchReg1 == Dyninst::Null_Register) {
          std::vector<Dyninst::Register> freeReg;
          std::vector<Dyninst::Register> excludeReg;
          stackSize = insnCodeGen::createStackFrame(gen, 1, freeReg, excludeReg);
          assert(stackSize == 1);
          scratchReg1 = freeReg[0];

          inst_printf(
              "emitStoreRelative - after new stack frame- addr 0x%lx curr adress "
              "0x%lx offset %lu "
              "0x%lx size %d\n",
              addr, gen.currAddr(), addr - gen.currAddr() + 4, addr - gen.currAddr() + 4, size);
        }
        emitLoadRelative(scratchReg1, varOffset, scratchReg, gen.width(), gen);
        emitStoreRelative(source, 0, scratchReg1, size, gen);
      } else {
        emitStoreRelative(source, varOffset, scratchReg, size, gen);
      }

      if(stackSize > 0) {
        insnCodeGen::removeStackFrame(gen);
      }

      return;
    }

    void EmitterPowerPC::emitTimesImm(Register, Register, RegValue, codeGen &) {
      assert(0);
    }

    void EmitterPowerPC::emitV(opCode op, Dyninst::Register src1, Dyninst::Register src2,
                               Dyninst::Register dest, codeGen &gen, int size, AddressSpace *proc,
                               bool s) {

      assert((op != branchOp) && (op != ifOp));          // !emitA
      assert((op != getRetValOp) && (op != getParamOp)); // !emitR
      assert((op != loadOp) && (op != loadConstOp));     // !emitVload
      assert((op != storeOp));                           // !emitVstore

      instruction insn;

      if(op == loadIndirOp) {
        // really load dest, (dest)imm
        assert(src2 == 0); // Since we don't use it.
        if(!size) {
          size = proc->getAddressWidth();
        }
        if(size == 1) {
          insnCodeGen::generateImm(gen, LBZop, dest, src1, 0);
        } else if(size == 2) {
          insnCodeGen::generateImm(gen, LHZop, dest, src1, 0);
        } else if((size == 4) || (size == 8 && proc->getAddressWidth() == 4)) {
          // Override bogus size
          insnCodeGen::generateImm(gen, Lop, dest, src1, 0);
        } else if(size == 8) {
          insnCodeGen::generateMemAccess64(gen, LDop, LDxop, dest, src1, 0);
        } else {
          assert(0 && "Incompatible loadOp size");
        }
      } else if(op == storeIndirOp) {
        // generate -- st src1, dest
        if(size == 1) {
          insnCodeGen::generateImm(gen, STBop, src1, dest, 0);
        } else if(size == 2) {
          insnCodeGen::generateImm(gen, STHop, src1, dest, 0);
        } else if((size == 4) || (size == 8 && proc->getAddressWidth() == 4)) {
          // Override bogus size
          insnCodeGen::generateImm(gen, STop, src1, dest, 0);
        } else if(size == 8) {
          insnCodeGen::generateMemAccess64(gen, STDop, STDxop, src1, dest, 0);
        } else {
          assert(0 && "Incompatible storeOp size");
        }

      } else if(op == noOp) {
        insnCodeGen::generateNOOP(gen);

      } else if(op == saveRegOp) {
        ppc::saveRegister(gen, src1, 8);

      } else {
        int instXop = -1;
        int instOp = -1;
        switch(op) {
          // integer ops
          case plusOp:
            instOp = CAXop;
            instXop = CAXxop;
            break;

          case minusOp: {
            Register temp;
            // need to flip operands since this computes ra-rb not rb-ra
            temp = src1;
            src1 = src2;
            src2 = temp;
            instOp = SFop;
            instXop = SFxop;
            break;
          }

          case timesOp:
            instOp = MULSop;
            // For 64-bit integer multiplication,
            // signed and unsigned are the same.
            //
            // Signed and unsigned are different if we start to
            // psas upper 64-bit of the results back to users.
            instXop = MULLxop;
            break;

          case divOp:
            instOp = DIVSop; // POWER divide instruction
                             // Same as DIVWop for PowerPC
            if(s) {
              instXop = DIVLSxop; // Extended opcode for signed division
            } else {
              instXop = DIVLUxop; // Extended opcode for unsigned division
            }
            break;

          // Bool ops
          case orOp:
            insn.clear();
            XOFORM_OP_SET(insn, ORop);
            // operation is ra <- rs | rb (or rs,ra,rb)
            XOFORM_RA_SET(insn, dest);
            XOFORM_RT_SET(insn, src1);
            XOFORM_RB_SET(insn, src2);
            XOFORM_XO_SET(insn, ORxop);
            insnCodeGen::generate(gen, insn);
            return;
            break;

          case andOp:
            // This is a Boolean and with true == 1 so bitwise is OK
            insn.clear();
            XOFORM_OP_SET(insn, ANDop);
            // operation is ra <- rs & rb (and rs,ra,rb)
            XOFORM_RA_SET(insn, dest);
            XOFORM_RT_SET(insn, src1);
            XOFORM_RB_SET(insn, src2);
            XOFORM_XO_SET(insn, ANDxop);
            insnCodeGen::generate(gen, insn);
            return;
            break;

          case xorOp:
            insn.clear();
            XOFORM_OP_SET(insn, XORop);
            // operation is ra <- rs ^ rb (xor rs,ra,rb)
            XOFORM_RA_SET(insn, dest);
            XOFORM_RT_SET(insn, src1);
            XOFORM_RB_SET(insn, src2);
            XOFORM_XO_SET(insn, XORxop);
            insnCodeGen::generate(gen, insn);
            return;
            break;

          // rel ops
          case eqOp:
            insnCodeGen::generateRelOp(gen, EQcond, BTRUEcond, src1, src2, dest, s);
            return;
            break;

          case neOp:
            insnCodeGen::generateRelOp(gen, EQcond, BFALSEcond, src1, src2, dest, s);
            return;
            break;

          case lessOp:
            insnCodeGen::generateRelOp(gen, LTcond, BTRUEcond, src1, src2, dest, s);
            return;
            break;

          case greaterOp:
            insnCodeGen::generateRelOp(gen, GTcond, BTRUEcond, src1, src2, dest, s);
            return;
            break;

          case leOp:
            insnCodeGen::generateRelOp(gen, GTcond, BFALSEcond, src1, src2, dest, s);
            return;
            break;

          case geOp:
            insnCodeGen::generateRelOp(gen, LTcond, BFALSEcond, src1, src2, dest, s);
            return;
            break;

          default:
            // internal error, invalid op.
            bperr("Invalid op passed to emit, instOp = %d\n", instOp);
            assert(0 && "Invalid op passed to emit");
            break;
        }

        assert((instOp != -1) && (instXop != -1));
        insn.clear();
        XOFORM_OP_SET(insn, instOp);
        XOFORM_RT_SET(insn, dest);
        XOFORM_RA_SET(insn, src1);
        XOFORM_RB_SET(insn, src2);
        XOFORM_XO_SET(insn, instXop);
        insnCodeGen::generate(gen, insn);
      }
      return;
    }

    void EmitterPowerPC::emitVload(opCode op, Address src1, Dyninst::Register src2,
                                   Dyninst::Register dest, codeGen &gen, int size,
                                   AddressSpace *proc) {
      switch(op) {
        case loadConstOp:
          insnCodeGen::loadImmIntoReg(gen, dest, (long)src1);
          break;
        case loadOp:
          insnCodeGen::loadPartialImmIntoReg(gen, dest, (long)src1);

          // really load dest, (dest)imm
          if(size == 1) {
            insnCodeGen::generateImm(gen, LBZop, dest, dest, LOW(src1));
          } else if(size == 2) {
            insnCodeGen::generateImm(gen, LHZop, dest, dest, LOW(src1));
          } else if((size == 4) ||
                    (size == 8 && proc->getAddressWidth() == 4)) { // Override bogus size
            insnCodeGen::generateImm(gen, Lop, dest, dest, LOW(src1));
          } else if(size == 8) {
            insnCodeGen::generateMemAccess64(gen, LDop, LDxop, dest, dest, (int16_t)LOW(src1));
          } else {
            assert(0 && "Incompatible loadOp size");
          }
          break;
        case loadFrameRelativeOp: {
          long offset = (long)src1;
          if(gen.width() == 4) {
            offset += TRAMP_FRAME_SIZE_32;
          } else { /* gen.width() == 8 */
            offset += TRAMP_FRAME_SIZE_64;
          }

          // return the value that is FP offset from the original fp
          if(size == 1) {
            insnCodeGen::generateImm(gen, LBZop, dest, REG_SP, offset);
          } else if(size == 2) {
            insnCodeGen::generateImm(gen, LHZop, dest, REG_SP, offset);
          } else if((size == 4) ||
                    (size == 8 && proc->getAddressWidth() == 4)) { // Override bogus size
            insnCodeGen::generateImm(gen, Lop, dest, REG_SP, offset);
          } else if(size == 8) {
            insnCodeGen::generateMemAccess64(gen, LDop, LDxop, dest, REG_SP, offset);
          } else {
            assert(0 && "Incompatible loadFrameRelativeOp size");
          }
        } break;
        case loadFrameAddr: {
          // offsets are signed!
          long offset = (long)src1;
          offset += (gen.width() == 4 ? TRAMP_FRAME_SIZE_32 : TRAMP_FRAME_SIZE_64);

          if(offset < MIN_IMM16 || MAX_IMM16 < offset) {
            assert(0);
          }
          insnCodeGen::generateImm(gen, CALop, dest, REG_SP, offset);
        } break;
        case loadRegRelativeAddr:
          gen.rs()->readProgramRegister(gen, src2, dest);
          gen.emitter()->emitImm(plusOp, dest, src1, dest, gen);
          break;
        case loadRegRelativeOp:
          gen.rs()->readProgramRegister(gen, src2, dest);

          if(size == 1) {
            insnCodeGen::generateImm(gen, LBZop, dest, dest, src1);
          } else if(size == 2) {
            insnCodeGen::generateImm(gen, LHZop, dest, dest, src1);
          } else if((size == 4) ||
                    (size == 8 && proc->getAddressWidth() == 4)) { // Override bogus size
            insnCodeGen::generateImm(gen, Lop, dest, dest, src1);
          } else if(size == 8) {
            insnCodeGen::generateMemAccess64(gen, LDop, LDxop, dest, dest, src1);
          }
          break;
        default:

          cerr << "Unknown op " << op << endl;
          assert(0);
      }
    }

    void EmitterPowerPC::emitVstore(opCode op, Dyninst::Register src1, Dyninst::Register /*src2*/,
                                    Address dest, codeGen &gen, int size, AddressSpace *proc) {
      if(op == storeOp) {
        // temp register to hold base address for store (added 6/26/96 jkh)
        Dyninst::Register temp = gen.rs()->getScratchRegister(gen);

        insnCodeGen::loadPartialImmIntoReg(gen, temp, (long)dest);
        if(size == 1) {
          insnCodeGen::generateImm(gen, STBop, src1, temp, LOW(dest));
        } else if(size == 2) {
          insnCodeGen::generateImm(gen, STHop, src1, temp, LOW(dest));
        } else if((size == 4) ||
                  (size == 8 && proc->getAddressWidth() == 4)) { // Override bogus size
          insnCodeGen::generateImm(gen, STop, src1, temp, LOW(dest));
        } else if(size == 8) {
          insnCodeGen::generateMemAccess64(gen, STDop, STDxop, src1, temp, (int16_t)BOT_LO(dest));
        } else {
          assert(0 && "Incompatible storeOp size");
        }

      } else if(op == storeFrameRelativeOp) {
        long offset = (long)dest;
        offset += (gen.width() == 4 ? TRAMP_FRAME_SIZE_32 : TRAMP_FRAME_SIZE_64);

        if(size == 1) {
          insnCodeGen::generateImm(gen, STBop, src1, REG_SP, offset);
        } else if(size == 2) {
          insnCodeGen::generateImm(gen, STHop, src1, REG_SP, offset);
        } else if((size == 4) ||
                  (size == 8 || proc->getAddressWidth() == 4)) { // Override bogus size
          insnCodeGen::generateImm(gen, STop, src1, REG_SP, offset);
        } else if(size == 8) {
          insnCodeGen::generateMemAccess64(gen, STDop, STDxop, src1, REG_SP, offset);
        } else {
          assert(0 && "Incompatible storeFrameRelativeOp size");
        }

      } else {
        assert(0 && "Unknown op passed to emitVstore");
      }
    }

    Address EmitterPowerPC::getInterModuleFuncAddr(func_instance *func, codeGen &gen) {
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
        sprintf(msg, "%s[%d]:  internal error:  cannot find symbol %s", __FILE__, __LINE__,
                func->symTabName().c_str());
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

    Address EmitterPowerPC::getInterModuleVarAddr(const image_variable *var, codeGen &gen) {
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
        sprintf(msg, "%s[%d]:  internal error:  cannot find symbol %s", __FILE__, __LINE__,
                var->symTabName().c_str());
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

static parsed_regs calcUsedRegs(parse_func *func) {
  parsed_regs usedRegisters;
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

static bool can_optimize_as_shift(RegValue val, uint8_t max_num_bits) {
  if(!Dyninst::isPowerOf2(val)) {
    return false;
  }

  // number of bits, n, such that src2imm = 2^n
  boost::optional<uint8_t> n = Dyninst::ilog2(val);

  if(!n) {
    return false;
  }

  // Can the result fit in a single register without overflowing?
  return *n < max_num_bits;
}
