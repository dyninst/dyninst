#include "addressSpace.h"
#include "arch-regs-x86.h"
#include "arch-x86.h"
#include "binaryEdit.h"
#include "BPatch.h"
#include "codegen/emitters/x86/generators.h"
#include "codegen/emitters/x86/IA32/EmitterIA32.h"
#include "codegen/RegControl.h"
#include "debug.h"
#include "function.h"
#include "image.h"
#include "inst-x86.h"
#include "parse_func.h"
#include "registerSpace/RealRegister.h"
#include "registerSpace/registerSpace.h"
#include "Symbol.h"
#include "unaligned_memory_access.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <limits>

extern bool isPowerOf2(int value, int &result);

static int extra_space_check{};

namespace Dyninst { namespace DyninstAPI {

  bool EmitterIA32::clobberAllFuncCall(registerSpace *rs, func_instance *callee) {
    if(callee == NULL) {
      return false;
    }

    if(writesFPRs(callee->ifunc())) {
      for(unsigned i = 0; i < rs->FPRs().size(); i++) {
        rs->FPRs()[i]->beenUsed = true;
      }
    }
    return true;
  }

  void EmitterIA32::emitAddSignedImm(Address addr, int imm, codeGen &gen, bool /*noCost*/) {
    x86::emitAddMem(addr, imm, gen);
  }

  bool EmitterIA32::emitAdjustStackPointer(int index, codeGen &gen) {
    // The index will be positive for "needs popped" and negative
    // for "needs pushed". However, positive + SP works, so don't
    // invert.
    int popVal = index * gen.addrSpace()->getAddressWidth();
    emitOpExtRegImm(0x81, EXTENDED_0x81_ADD, RealRegister(REGNUM_ESP), popVal, gen);
    gen.rs()->incStack(-1 * popVal);
    return true;
  }

  void EmitterIA32::emitASload(int ra, int rb, int sc, long imm, Dyninst::Register dest,
                               int stackOffset, codeGen &gen) {
    bool havera = ra > -1, haverb = rb > -1;

    // assuming 32-bit addressing (for now)

    if(ra == REGNUM_ESP && !haverb && sc == 0 && gen.bt()) {
      // Optimization, common for push/pop
      RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
      stackItemLocation loc = getHeightOf(stackItem::stacktop, gen);
      if(!gen.bt() || gen.bt()->alignedStack) {
        emitMovRMToReg(dest_r, loc.reg, loc.offset, gen);
        if(imm) {
          ::emitLEA(dest_r, RealRegister(Null_Register), 0, imm, dest_r, gen);
        }
      } else {
        ::emitLEA(loc.reg, RealRegister(Null_Register), 0, loc.offset, dest_r, gen);
      }
      return;
    }

    RealRegister src1_r(-1);
    Dyninst::Register src1 = Null_Register;
    if(havera) {
      if(gen.inInstrumentation()) {
        src1 = restoreGPRtoReg(RealRegister(ra), gen);
        src1_r = gen.rs()->loadVirtual(src1, gen);
        gen.rs()->markKeptRegister(src1);
      } else {
        // Don't have a base tramp - use only reals
        src1_r = RealRegister(ra);
        // If this is a stack pointer, modify imm to compensate
        // for any changes in the stack pointer
        if(ra == REGNUM_ESP) {
          imm -= stackOffset;
        }
      }
    }

    RealRegister src2_r(-1);
    Dyninst::Register src2 = Null_Register;
    if(haverb) {
      if(ra == rb) {
        src2_r = src1_r;
      } else if(gen.inInstrumentation()) {
        src2 = restoreGPRtoReg(RealRegister(rb), gen);
        src2_r = gen.rs()->loadVirtual(src2, gen);
        gen.rs()->markKeptRegister(src2);
      } else {
        src2_r = RealRegister(rb);
        // If this is a stack pointer, modify imm to compensate
        // for any changes in the stack pointer
        if(rb == REGNUM_ESP) {
          imm -= (stackOffset * sc);
        }
      }
    }

    if(havera && !haverb && !sc && !imm) {
      // Optimized case, just use the existing src1_r
      if(gen.inInstrumentation()) {
        gen.rs()->unKeepRegister(src1);
        gen.rs()->freeRegister(src1);
        gen.rs()->noteVirtualInReal(dest, src1_r);
        return;
      } else {
        // No base tramp, no virtual registers - emit a move?
        emitMovRegToReg(RealRegister(dest), src1_r, gen);
        return;
      }
    }

    // Emit the lea to do the math for us:
    // e.g. lea eax, [eax + edx * sc + imm] if both ra and rb had to be
    // restored
    RealRegister dest_r;
    if(gen.inInstrumentation()) {
      dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    } else {
      dest_r = RealRegister(dest);
    }
    ::emitLEA(src1_r, src2_r, sc, (long)imm, dest_r, gen);

    if(src1 != Null_Register) {
      gen.rs()->unKeepRegister(src1);
      gen.rs()->freeRegister(src1);
    }
    if(src2 != Null_Register) {
      gen.rs()->unKeepRegister(src2);
      gen.rs()->freeRegister(src2);
    }
  }

  bool EmitterIA32::emitBTRestores(baseTramp *bt, codeGen &gen) {
    bool useFPRs;
    bool createFrame;
    bool saveOrigAddr;
    bool alignStack;
    if(bt) {
      useFPRs = bt->savedFPRs;
      createFrame = bt->createdFrame;
      saveOrigAddr = bt->savedOrigAddr;
      alignStack = bt->alignedStack;
    } else {
      useFPRs = BPatch::bpatch->isForceSaveFPROn() ||
                (BPatch::bpatch->isSaveFPROn() && gen.rs()->anyLiveFPRsAtEntry());
      createFrame = true;
      saveOrigAddr = false;
      alignStack = true;
    }

    if(useFPRs) {
      // restore saved FP state
      // fxrstor (%rsp) ; 0x0f 0xae 0x04 0x24
      GET_PTR(insn, gen);
      append_memory_as_byte(insn, 0x0f);
      append_memory_as_byte(insn, 0xae);
      append_memory_as_byte(insn, 0x0c);
      append_memory_as_byte(insn, 0x24);
      SET_PTR(insn, gen);
    }

    // Remove extra space allocated for temporaries and floating-point state
    int extra_space = gen.rs()->getStackHeight();
    assert(extra_space == extra_space_check);
    if(!createFrame && extra_space) {
      ::emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0, extra_space,
                RealRegister(REGNUM_ESP), gen);
    }

    if(createFrame) {
      emitSimpleInsn(LEAVE, gen);
    }
    if(saveOrigAddr) {
      ::emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0, 4,
                RealRegister(REGNUM_ESP), gen);
    }

    // popa or pop each register, plus optional popf
    emitBTRegRestores32(bt, gen);

    // Restore the (possibly unaligned) stack pointer.
    if(alignStack) {
      emitMovRMToReg(RealRegister(REGNUM_ESP), RealRegister(REGNUM_ESP), 0, gen);
    } else {
      int funcJumpSlotSize = 0;
      if(bt && bt->funcJumpSlotSize()) {
        funcJumpSlotSize = bt->funcJumpSlotSize() * 4;
      }
      if(funcJumpSlotSize) {
        ::emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0, funcJumpSlotSize,
                  RealRegister(REGNUM_ESP), gen);
      }
    }

    gen.setInInstrumentation(false);
    return true;
  }

  bool EmitterIA32::emitBTSaves(baseTramp *bt, codeGen &gen) {
    // x86 linux platforms do not allow for writing to memory
    // below the stack pointer.  No need to skip a "red zone."

    gen.setInInstrumentation(true);

    int instFrameSize = 0; // Tracks how much we are moving %rsp
    int funcJumpSlotSize = 0;
    if(bt) {
      funcJumpSlotSize = bt->funcJumpSlotSize() * 4;
    }

    // Align the stack now to avoid having a padding hole in the middle of
    // our instrumentation stack.  Referring to anything on the stack above
    // this point will require an indirect reference.
    //
    // There are two cases that require a 16-byte aligned stack pointer:
    //
    //    - Any time we need to save the FP registers
    //    - Any time we may execute SSE/SSE2 instructions
    //
    // The second case is only possible if we generate a function call
    // so search the ASTs for function call generation.
    //
    bool useFPRs = BPatch::bpatch->isForceSaveFPROn() ||
                   (BPatch::bpatch->isSaveFPROn() && gen.rs()->anyLiveFPRsAtEntry() &&
                    bt->saveFPRs() && bt->makesCall());
    bool alignStack = useFPRs || !bt || bt->checkForFuncCalls();

    if(alignStack) {
      emitStackAlign(funcJumpSlotSize, gen);

    } else if(funcJumpSlotSize > 0) {
      // Just move %esp to make room for the funcJump.
      // Use LEA to avoid flag modification.
      ::emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0, -funcJumpSlotSize,
                RealRegister(REGNUM_ESP), gen);
      instFrameSize += funcJumpSlotSize;
    }

    bool flags_saved = gen.rs()->saveVolatileRegisters(gen);
    // makesCall was added because our code spills registers around function
    // calls, and needs somewhere for those spills to go
    bool createFrame = !bt || bt->needsFrame() || useFPRs || bt->makesCall();
    bool saveOrigAddr = createFrame && bt->instP();
    bool localSpace =
        createFrame || useFPRs || (bt && bt->validOptimizationInfo() && bt->spilledRegisters);

    if(bt) {
      bt->savedFPRs = useFPRs;
      bt->createdFrame = createFrame;
      bt->savedOrigAddr = saveOrigAddr;
      bt->createdLocalSpace = localSpace;
      bt->alignedStack = alignStack;
      bt->savedFlags = flags_saved;
    }

    int flags_saved_i = flags_saved ? 1 : 0;
    int base_i = (saveOrigAddr ? 1 : 0) + (createFrame ? 1 : 0);

    int num_saved = 0;
    int numRegsUsed = bt ? bt->numDefinedRegs() : -1;
    if(numRegsUsed == -1 || numRegsUsed > X86_REGS_SAVE_LIMIT) {
      emitSimpleInsn(PUSHAD, gen);
      gen.rs()->incStack(8 * 4);
      num_saved = 8;

      gen.rs()->markSavedRegister(RealRegister(REGNUM_EAX), 7 + flags_saved_i + base_i);
      if(flags_saved) {
        gen.rs()->markSavedRegister(IA32_FLAG_VIRTUAL_REGISTER, 7 + base_i);
      }
      gen.rs()->markSavedRegister(RealRegister(REGNUM_ECX), 6 + base_i);
      gen.rs()->markSavedRegister(RealRegister(REGNUM_EDX), 5 + base_i);
      gen.rs()->markSavedRegister(RealRegister(REGNUM_EBX), 4 + base_i);
      gen.rs()->markSavedRegister(RealRegister(REGNUM_ESP), 3 + base_i);
      if(!createFrame) {
        gen.rs()->markSavedRegister(RealRegister(REGNUM_EBP), 2 + base_i);
      }
      gen.rs()->markSavedRegister(RealRegister(REGNUM_ESI), 1 + base_i);
      gen.rs()->markSavedRegister(RealRegister(REGNUM_EDI), 0 + base_i);
    } else {
      std::vector<registerSlot *> &regs = gen.rs()->trampRegs();
      for(unsigned i = 0; i < regs.size(); i++) {
        registerSlot *reg = regs[i];
        if(bt->definedRegs[reg->encoding()]) {
          ::emitPush(RealRegister(reg->encoding()), gen);
          int eax_flags = (reg->encoding() == REGNUM_EAX) ? flags_saved_i : 0;
          gen.rs()->markSavedRegister(RealRegister(reg->encoding()),
                                      numRegsUsed - num_saved + base_i - 1 + eax_flags);
          if(eax_flags) {
            gen.rs()->markSavedRegister(IA32_FLAG_VIRTUAL_REGISTER,
                                        numRegsUsed - num_saved + base_i - 1);
          }
          num_saved++;
        }
      }
      assert(num_saved == numRegsUsed);
    }

    if(saveOrigAddr) {
      emitPushImm(bt->instP()->addr_compat(), gen);
    }
    if(createFrame) {
      // For now, we'll do all saves then do the guard. Could inline
      // Return addr for stack frame walking; for lack of a better idea,
      // we grab the original instPoint address
      emitSimpleInsn(PUSH_EBP, gen);
      gen.rs()->incStack(4);
      emitMovRegToReg(RealRegister(REGNUM_EBP), RealRegister(REGNUM_ESP), gen);
      gen.rs()->markSavedRegister(RealRegister(REGNUM_EBP), 0);
    }

    // Not sure liveness touches this yet, so not using
    // bool liveFPRs = (gen.rs()->FPRs()[0]->liveState == registerSlot:live);

    // Prepare our stack bookkeeping data structures.
    instFrameSize += (flags_saved_i + num_saved + base_i) * 4;
    if(bt) {
      bt->stackHeight = instFrameSize;
    }
    gen.rs()->setInstFrameSize(instFrameSize);
    gen.rs()->setStackHeight(0);

    // Pre-calculate space for temporaries and floating-point state.
    int extra_space = 0;
    if(useFPRs) {
      extra_space += TRAMP_FRAME_SIZE + 512;
    } else if(localSpace) {
      extra_space += TRAMP_FRAME_SIZE;
    }

    // Make sure that we're still aligned when we add extra_space to the stack.
    if(alignStack) {
      if((instFrameSize + extra_space) % IA32_STACK_ALIGNMENT) {
        extra_space +=
            IA32_STACK_ALIGNMENT - ((instFrameSize + extra_space) % IA32_STACK_ALIGNMENT);
      }
    }

    if(extra_space) {
      ::emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0, -extra_space,
                RealRegister(REGNUM_ESP), gen);
      gen.rs()->incStack(extra_space);
    }
    extra_space_check = extra_space;

    if(useFPRs) {
      // need to save the floating point state (x87, MMX, SSE)
      // We're guaranteed to be 16-byte aligned now, so just
      // emit the fxsave.

      // fxsave (%esp) ; 0x0f 0xae 0x04 0x24
      GET_PTR(insn, gen);
      append_memory_as_byte(insn, 0x0f);
      append_memory_as_byte(insn, 0xae);
      append_memory_as_byte(insn, 0x04);
      append_memory_as_byte(insn, 0x24);
      SET_PTR(insn, gen);
    }

    return true;
  }

  Dyninst::Register EmitterIA32::emitCall(opCode op, codeGen &gen,
                                          const std::vector<codeGenASTPtr> &operands, bool noCost,
                                          func_instance *callee) {
    bool inInstrumentation = true;
    if(op != callOp) {
      cerr << "ERROR: emitCall with op == " << op << endl;
    }
    assert(op == callOp);
    std::vector<Dyninst::Register> srcs;
    int param_size;
    std::vector<Dyninst::Register> saves;

    //  Sanity check for NULL address arg
    if(!callee) {
      char msg[256];
      sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out callee argument", __FILE__,
              __LINE__);
      showErrorCallback(80, msg);
      assert(0);
    }

    param_size = emitCallParams(gen, operands, callee, saves, noCost);

    Dyninst::Register ret = REGNUM_EAX;

    emitCallInstruction(gen, callee, ret);

    emitCallCleanup(gen, callee, param_size, saves);

    if(!inInstrumentation) {
      return Null_Register;
    }

    return ret;
  }

  int EmitterIA32::emitCallParams(codeGen &gen, const std::vector<codeGenASTPtr> &operands,
                                  func_instance * /*target*/,
                                  std::vector<Dyninst::Register> & /*extra_saves*/, bool noCost) {
    std::vector<Dyninst::Register> srcs;
    unsigned frame_size = 0;
    unsigned u;
    for(u = 0; u < operands.size(); u++) {
      Address unused = ADDR_NULL;
      Dyninst::Register reg = Null_Register;
      if(!operands[u]->generateCode_phase2(gen, noCost, unused, reg)) {
        assert(0);
      }
      assert(reg != Null_Register);
      srcs.push_back(reg);
    }

    // push arguments in reverse order, last argument first
    // must use int instead of unsigned to avoid nasty underflow problem:
    for(int i = srcs.size() - 1; i >= 0; i--) {
      RealRegister r = gen.rs()->loadVirtual(srcs[i], gen);
      ::emitPush(r, gen);
      frame_size += 4;
      gen.rs()->freeRegister(srcs[i]);
    }
    return frame_size;
  }

  bool EmitterIA32::emitCallCleanup(codeGen &gen, func_instance * /*target*/, int frame_size,
                                    std::vector<Dyninst::Register> & /*extra_saves*/) {
    if(frame_size) {
      emitOpRegImm(0, RealRegister(REGNUM_ESP), frame_size, gen); // add esp, frame_size
    }
    gen.rs()->incStack(-1 * frame_size);
    return true;
  }

  bool EmitterIA32::emitCallRelative(Register, Address, Register, codeGen &) {
    assert(0);
    return false;
  }

  void EmitterIA32::emitCSload(int ra, int rb, int sc, long imm, Dyninst::Register dest,
                               codeGen &gen) {
    // count is at most 1 register or constant or hack (aka pseudoregister)
    assert((ra == -1) &&
           ((rb == -1) || ((imm == 0) && (rb == 1 /*REGNUM_ECX */ || rb >= IA32_EMULATE))));

    if(rb >= IA32_EMULATE) {
      bool neg = false;
      switch(rb) {
        case IA32_NESCAS:
          neg = true;
          DYNINST_FALLTHROUGH;
        case IA32_ESCAS: {
          // plan: restore flags, edi, eax, ecx; do rep(n)e scas(b/w);
          // compute (saved_ecx - ecx) << sc;

          gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EAX), gen);
          gen.rs()->makeRegisterAvail(RealRegister(REGNUM_ECX), gen);
          gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EDI), gen);

          // mov eax<-offset[ebp]
          emitRestoreFlagsFromStackSlot(gen);
          restoreGPRtoGPR(RealRegister(REGNUM_EAX), RealRegister(REGNUM_EAX), gen);
          restoreGPRtoGPR(RealRegister(REGNUM_ECX), RealRegister(REGNUM_ECX), gen);
          restoreGPRtoGPR(RealRegister(REGNUM_EDI), RealRegister(REGNUM_EDI), gen);
          gen.markRegDefined(REGNUM_EAX);
          gen.markRegDefined(REGNUM_ECX);
          gen.markRegDefined(REGNUM_EDI);
          emitSimpleInsn(neg ? 0xF2 : 0xF3, gen); // rep(n)e
          switch(sc) {
            case 0:
              emitSimpleInsn(0xAE, gen); // scasb
              break;
            case 1:
              emitSimpleInsn(0x66, gen); // operand size override for scasw;
              DYNINST_FALLTHROUGH;
            case 2:
              emitSimpleInsn(0xAF, gen); // scasw/d
              break;
            default:
              assert(!"Wrong scale!");
          }
          restoreGPRtoGPR(RealRegister(REGNUM_ECX), RealRegister(REGNUM_EAX),
                          gen);                                                   // old ecx -> eax
          emitSubRegReg(RealRegister(REGNUM_EAX), RealRegister(REGNUM_ECX), gen); // eax = eax - ecx
          gen.markRegDefined(REGNUM_EAX);
          if(sc > 0) {
            emitSHL(RealRegister(REGNUM_EAX), static_cast<unsigned char>(sc),
                    gen); // shl eax, scale
          }
          RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
          emitMovRegToReg(dest_r, RealRegister(REGNUM_EAX), gen);
          break;
        }
        case IA32_NECMPS:
          neg = true;
          DYNINST_FALLTHROUGH;
        case IA32_ECMPS: {
          // plan: restore flags, esi, edi, ecx; do rep(n)e cmps(b/w);
          // compute (saved_ecx - ecx) << sc;

          gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EAX), gen);
          gen.rs()->makeRegisterAvail(RealRegister(REGNUM_ESI), gen);
          gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EDI), gen);
          gen.rs()->makeRegisterAvail(RealRegister(REGNUM_ECX), gen);

          // mov eax<-offset[ebp]
          emitRestoreFlagsFromStackSlot(gen);
          restoreGPRtoGPR(RealRegister(REGNUM_ECX), RealRegister(REGNUM_ECX), gen);
          gen.markRegDefined(REGNUM_ECX);
          restoreGPRtoGPR(RealRegister(REGNUM_ESI), RealRegister(REGNUM_ESI), gen);
          gen.markRegDefined(REGNUM_ESI);
          restoreGPRtoGPR(RealRegister(REGNUM_EDI), RealRegister(REGNUM_EDI), gen);
          gen.markRegDefined(REGNUM_EDI);
          emitSimpleInsn(neg ? 0xF2 : 0xF3, gen); // rep(n)e
          switch(sc) {
            case 0:
              emitSimpleInsn(0xA6, gen); // cmpsb
              break;
            case 1:
              emitSimpleInsn(0x66, gen); // operand size override for cmpsw;
              DYNINST_FALLTHROUGH;
            case 2:
              emitSimpleInsn(0xA7, gen); // cmpsw/d
              break;
            default:
              assert(!"Wrong scale!");
          }
          restoreGPRtoGPR(RealRegister(REGNUM_ECX), RealRegister(REGNUM_EAX),
                          gen);                                                   // old ecx -> eax
          emitSubRegReg(RealRegister(REGNUM_EAX), RealRegister(REGNUM_ECX), gen); // eax = eax - ecx
          if(sc > 0) {
            emitSHL(RealRegister(REGNUM_EAX), static_cast<unsigned char>(sc),
                    gen); // shl eax, scale
          }
          RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
          emitMovRegToReg(dest_r, RealRegister(REGNUM_EAX), gen);

          break;
        }
        default:
          assert(!"Wrong emulation!");
      }
    } else if(rb > -1) {
      // TODO: 16-bit pseudoregisters
      assert(rb < 8);
      RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
      restoreGPRtoGPR(RealRegister(rb), dest_r, gen); // mov dest, [saved_rb]
      if(sc > 0) {
        emitSHL(dest_r, static_cast<unsigned char>(sc), gen); // shl eax, scale
      }
    } else {
      RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
      emitMovImmToReg(dest_r, imm, gen);
    }
  }

  void EmitterIA32::emitDiv(Register dest, Register src1, Register src2, codeGen &gen, bool s) {
    Register scratch = gen.rs()->allocateRegister(gen, true);
    gen.rs()->loadVirtualToSpecific(src1, RealRegister(REGNUM_EAX), gen);
    gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EDX), gen);
    gen.rs()->noteVirtualInReal(scratch, RealRegister(REGNUM_EDX));
    RealRegister src2_r = gen.rs()->loadVirtual(src2, gen);
    gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EAX), gen);
    emitSimpleInsn(0x99, gen); // cdq (src1 -> eax:edx)
    if(s) {
      emitOpExtReg(0xF7, 0x7, src2_r, gen); // idiv eax:edx,src2 -> eax
    } else {
      emitOpExtReg(0xF7, 0x6, src2_r, gen); // div eax:edx,src2 -> eax
    }
    gen.rs()->noteVirtualInReal(dest, RealRegister(REGNUM_EAX));
    gen.rs()->freeRegister(scratch);
  }

  void EmitterIA32::emitDivImm(Register dest, Register src1, RegValue src2imm, codeGen &gen,
                               bool s) {
    int result;
    if(src2imm == 1) {
      return;
    }

    if(isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
      RealRegister src1_r = gen.rs()->loadVirtual(src1, gen);
      RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);

      if(src1 != dest) {
        emitMovRegToReg(dest_r, src1_r, gen);
      }
      if(s) {
        // sar dest, result
        emitOpExtRegImm8(0xC1, 7, dest_r, static_cast<unsigned char>(result), gen);
      } else {
        // shr dest, result
        emitOpExtRegImm8(0xC1, 5, dest_r, static_cast<unsigned char>(result), gen);
      }

    } else {
      Register src2 = gen.rs()->allocateRegister(gen, true);
      emitLoadConst(src2, src2imm, gen);
      emitDiv(dest, src1, src2, gen, s);
      gen.rs()->freeRegister(src2);
    }
  }

  void EmitterIA32::emitGetParam(Register dest, Register param_num, instPoint::Type pt_type,
                                 opCode op, bool addr_of, codeGen &gen) {
    // Parameters are addressed by a positive offset from ebp,
    // the first is PARAM_OFFSET[ebp]
    stackItemLocation loc = getHeightOf(stackItem::stacktop, gen);
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    if(!gen.bt() || gen.bt()->alignedStack) {
      // Load the original %esp value into dest_r
      emitMovRMToReg(dest_r, loc.reg, loc.offset, gen);
      loc.offset = 0;
      loc.reg = dest_r;
    }

    switch(op) {
      case getParamOp:
        // guess whether we're at the call or the function entry point,
        // in which case we need to skip the return value
        if(pt_type == instPoint::FuncEntry) {
          loc.offset += 4;
        }
        break;
      case getParamAtCallOp:
        break;
      case getParamAtEntryOp:
        loc.offset += 4;
        break;
      default:
        assert(0);
        break;
    }

    loc.offset += param_num * 4;

    // Prepare a real destination register.

    if(!addr_of) {
      emitMovRMToReg(dest_r, loc.reg, loc.offset, gen);
    } else {
      ::emitLEA(loc.reg, RealRegister(Null_Register), 0, loc.offset, dest_r, gen);
    }
  }

  void EmitterIA32::emitGetRetAddr(Register dest, codeGen &gen) {
    // Parameters are addressed by a positive offset from ebp,
    // the first is PARAM_OFFSET[ebp]
    stackItemLocation loc = getHeightOf(stackItem::stacktop, gen);
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    if(!gen.bt() || gen.bt()->alignedStack) {
      // Load the original %esp value into dest_r
      emitMovRMToReg(dest_r, loc.reg, loc.offset, gen);
      loc.offset = 0;
      loc.reg = dest_r;
    }
    emitMovRMToReg(dest_r, loc.reg, loc.offset, gen);
  }

  void EmitterIA32::emitGetRetVal(Register dest, bool addr_of, codeGen &gen) {
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    if(!addr_of) {
      restoreGPRtoGPR(RealRegister(REGNUM_EAX), dest_r, gen);
      return;
    }

    // EAX isn't really defined here, but this will make the code generator
    // put it onto the stack and thus guarentee that we'll have an
    // address to access it at.
    gen.markRegDefined(REGNUM_EAX);
    stackItemLocation loc = getHeightOf(stackItem::framebase, gen);

    std::vector<registerSlot *> &regs = gen.rs()->trampRegs();
    registerSlot *eax = NULL;
    for(unsigned i = 0; i < regs.size(); i++) {
      if(regs[i]->encoding() == REGNUM_EAX) {
        eax = regs[i];
        break;
      }
    }
    assert(eax);

    loc.offset += (eax->saveOffset * 4);
    ::emitLEA(loc.reg, RealRegister(Null_Register), 0, loc.offset, dest_r, gen);
  }

  codeBufIndex_t EmitterIA32::emitIf(Register expr_reg, Register target,
                                     Dyninst::DyninstAPI::RegControl rc, codeGen &gen) {
    RealRegister r = gen.rs()->loadVirtual(expr_reg, gen);
    emitOpRegReg(TEST_EV_GV, r, r, gen);

    // Retval: where the jump is in this sequence
    codeBufIndex_t retval = gen.getIndex();

    // Jump displacements are from the end of the insn, not start. The
    // one we're emitting has a size of 6.
    int32_t disp = 0;
    if(target) {
      disp = target - 6;
    }

    if(rc == Dyninst::DyninstAPI::RegControl::rc_before_jump) {
      gen.rs()->pushNewRegState();
    }
    GET_PTR(insn, gen);
    // je dest
    append_memory_as_byte(insn, 0x0F);
    append_memory_as_byte(insn, 0x84);
    write_memory_as(insn, int32_t{disp});
    if(disp == 0) {
      SET_PTR(insn, gen);
      gen.addPatch(gen.getIndex(), NULL, sizeof(int), relocPatch::patch_type_t::pcrel,
                   gen.used() + sizeof(int));
      REGET_PTR(insn, gen);
    }
    insn += sizeof(int);
    SET_PTR(insn, gen);

    return retval;
  }

  void EmitterIA32::emitLEA(Register base, Register index, unsigned int scale, int disp,
                            Register dest, codeGen &gen) {
    Register tmp_base = base;
    Register tmp_index = index;
    Register tmp_dest = dest;
    ::emitLEA(RealRegister(tmp_base), RealRegister(tmp_index), scale, disp, RealRegister(tmp_dest),
              gen);
    gen.markRegDefined(dest);
  }

  void EmitterIA32::emitLoad(Register dest, Address addr, int size, codeGen &gen) {
    RealRegister r = gen.rs()->loadVirtualForWrite(dest, gen);
    if(size == 1) {
      emitMovMBToReg(r, addr, gen); // movsbl eax, addr
    } else if(size == 2) {
      emitMovMWToReg(r, addr, gen); // movswl eax, addr
    } else {
      emitMovMToReg(r, addr, gen); // mov eax, addr
    }
  }

  void EmitterIA32::emitLoadConst(Register dest, Address imm, codeGen &gen) {
    RealRegister r = gen.rs()->loadVirtualForWrite(dest, gen);
    emitMovImmToReg(r, imm, gen);
  }

  void EmitterIA32::emitLoadFrameAddr(Register dest, Address offset, codeGen &gen) {
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    restoreGPRtoReg(RealRegister(REGNUM_EBP), gen, &dest_r);
    emitAddRegImm32(dest_r, offset, gen);
  }

  void EmitterIA32::emitLoadIndir(Register dest, Register addr_reg, int /*size*/, codeGen &gen) {
    RealRegister dest_r(-1);
    RealRegister src_r = gen.rs()->loadVirtual(addr_reg, gen);
    if(dest != addr_reg) {
      dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    } else {
      dest_r = src_r;
    }
    emitMovRMToReg(dest_r, src_r, 0, gen);
  }

  void EmitterIA32::emitLoadOrigFrameRelative(Register dest, Address offset, codeGen &gen) {
    if(gen.bt()->createdFrame) {
      Register scratch = gen.rs()->allocateRegister(gen, true);
      RealRegister scratch_r = gen.rs()->loadVirtualForWrite(scratch, gen);
      RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
      emitMovRMToReg(scratch_r, RealRegister(REGNUM_EBP), 0, gen);
      emitMovRMToReg(dest_r, scratch_r, offset, gen);
      gen.rs()->freeRegister(scratch);
      return;
    }

    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    emitMovRMToReg(dest_r, RealRegister(REGNUM_EBP), offset, gen);
  }

  void EmitterIA32::emitLoadOrigRegister(Address register_num, Register dest, codeGen &gen) {
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    restoreGPRtoGPR(RealRegister(register_num), dest_r, gen);
  }

  void EmitterIA32::emitLoadOrigRegRelative(Register dest, Address offset, Register base,
                                            codeGen &gen, bool store) {

    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    restoreGPRtoGPR(RealRegister(base), dest_r, gen);
    // either load the address or the contents at that address
    if(store) {
      // dest = [reg](offset)
      emitMovRMToReg(dest_r, dest_r, offset, gen);
    } else // calc address
    {
      // add offset,eax
      emitAddRegImm32(dest_r, offset, gen);
    }
  }

  bool EmitterIA32::emitLoadRelative(Register /*dest*/, Address /*offset*/, Register /*base*/,
                                     int /*size*/, codeGen & /*gen*/) {
    assert(0);
    return false;
  }

  bool EmitterIA32::emitLoadRelativeSegReg(Register /*dest*/, Address offset, Register base,
                                           int /*size*/, codeGen &gen) {
    // WARNING: dest is hard-coded to EAX currently
    x86::emitSegPrefix(base, gen);
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0xa1);
    append_memory_as_byte(insn, offset);
    append_memory_as_byte(insn, 0x00);
    append_memory_as_byte(insn, 0x00);
    append_memory_as_byte(insn, 0x00);
    SET_PTR(insn, gen);
    return true;
  }

  void EmitterIA32::emitLoadShared(opCode op, Register dest, const image_variable *var,
                                   bool is_local, int /*size*/, codeGen &gen, Address offset) {
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);

    // create or retrieve jump slot
    Address addr;
    if(var == NULL) {
      addr = offset;
    } else if(!is_local) {
      addr = getInterModuleVarAddr(var, gen);
    } else {
      addr = (Address)var->getOffset();
    }

    emitMovPCRMToReg(dest_r, addr - gen.currAddr(), gen, (!is_local && var != NULL));
    if(op == loadOp) {
      emitLoadIndir(dest, dest, 4, gen);
    }
  }

  bool EmitterIA32::emitMoveRegToReg(Register src, Register dest, codeGen &gen) {
    RealRegister src_r = gen.rs()->loadVirtual(src, gen);
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    emitMovRegToReg(dest_r, src_r, gen);
    return true;
  }

  bool EmitterIA32::emitMoveRegToReg(registerSlot *, registerSlot *, codeGen &) {
    assert(0);
    return true;
  }

  void EmitterIA32::emitOp(unsigned opcode, Register dest, Register src1, Register src2,
                           codeGen &gen) {
    RealRegister src1_r = gen.rs()->loadVirtual(src1, gen);
    RealRegister src2_r = gen.rs()->loadVirtual(src2, gen);
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    emitMovRegToReg(dest_r, src1_r, gen);
    emitOpRegReg(opcode, dest_r, src2_r, gen);
  }

  void EmitterIA32::emitOpImm(unsigned opcode1, unsigned opcode2, Register dest, Register src1,
                              RegValue src2imm, codeGen &gen) {
    RealRegister src1_r = gen.rs()->loadVirtual(src1, gen);
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    if(src1 != dest) {
      emitMovRegToReg(dest_r, src1_r, gen);
    }
    emitOpExtRegImm(opcode1, (char)opcode2, dest_r, src2imm, gen);
  }

  bool EmitterIA32::emitPop(codeGen &gen, Dyninst::Register reg) {
    RealRegister real_reg = gen.rs()->loadVirtual(reg, gen);
    return ::emitPop(real_reg, gen);
  }

  bool EmitterIA32::emitPush(codeGen &gen, Dyninst::Register reg) {
    RealRegister real_reg = gen.rs()->loadVirtual(reg, gen);
    return ::emitPush(real_reg, gen);
  }

  void EmitterIA32::emitPushFlags(codeGen &gen) {
    // These crank the saves forward
    emitSimpleInsn(PUSHFD, gen);
  }

  void EmitterIA32::emitRelOp(unsigned op, Register dest, Register src1, Register src2,
                              codeGen &gen, bool s) {
    RealRegister src1_r = gen.rs()->loadVirtual(src1, gen);
    RealRegister src2_r = gen.rs()->loadVirtual(src2, gen);
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    Register scratch = gen.rs()->allocateRegister(gen, true);
    RealRegister scratch_r = gen.rs()->loadVirtualForWrite(scratch, gen);

    emitOpRegReg(XOR_R32_RM32, dest_r, dest_r, gen); // XOR dest,dest
    emitMovImmToReg(scratch_r, 0x1, gen);            // MOV $2,scratch
    emitOpRegReg(CMP_GV_EV, src1_r, src2_r, gen);    // CMP src1, src2

    unsigned char opcode = cmovOpcodeFromRelOp(op, s);
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0x0f);
    SET_PTR(insn, gen);
    emitOpRegReg(opcode, dest_r, scratch_r, gen); // CMOVcc scratch,dest
    gen.rs()->freeRegister(scratch);
  }

  void EmitterIA32::emitRelOpImm(unsigned op, Register dest, Register src1, RegValue src2imm,
                                 codeGen &gen, bool s) {

    Register src2 = gen.rs()->allocateRegister(gen, true);
    emitLoadConst(src2, src2imm, gen);
    emitRelOp(op, dest, src1, src2, gen, s);
    gen.rs()->freeRegister(src2);
  }

  void EmitterIA32::emitRestoreFlags(codeGen &, unsigned) {
    assert(!"never use this!");
    return;
  }

  void EmitterIA32::emitRestoreFlagsFromStackSlot(codeGen &gen) {
    // if the flags aren't on the stack, they're already restored...
    if((*gen.rs())[IA32_FLAG_VIRTUAL_REGISTER]->liveState == registerSlot::spilled) {
      stackItemLocation loc = getHeightOf(stackItem(RealRegister(IA32_FLAG_VIRTUAL_REGISTER)), gen);
      assert(loc.offset % 4 == 0);
      ::emitPush(RealRegister(REGNUM_EAX), gen);
      emitMovRMToReg(RealRegister(REGNUM_EAX), loc.reg, loc.offset, gen);
      emitRestoreO(gen);
      emitSimpleInsn(0x9E, gen); // SAHF
      ::emitPop(RealRegister(REGNUM_EAX), gen);
    }
  }

  // Moves stack pointer by offset and aligns it to IA32_STACK_ALIGNMENT
  // with the following sequence:
  //
  //     lea    -off(%esp) => %esp           # move %esp down
  //     mov    %eax => saveSlot1(%esp)      # save %eax onto stack
  //     lahf                                # save %eflags byte into %ah
  //     seto   %al                          # save overflow flag into %al
  //     mov    %eax => saveSlot2(%esp)      # save flags %eax onto stack
  //     lea    off(%esp) => %eax            # store original %esp in %eax
  //     and    -$IA32_STACK_ALIGNMENT,%esp  # align %esp
  //     mov    %eax => (%esp)               # store original %esp on stack
  //     mov    -off+saveSlot2(%eax) => %eax # restore flags %eax from stack
  //     add    $0x7f,%al                    # restore overflow flag from %al
  //     sahf                                # restore %eflags byte from %ah
  //     mov    (%esp) => %eax               # re-load old %esp into %eax to ...
  //     mov    -off+saveSlot1(%eax) => %eax # ... restore %eax from stack
  //
  // This sequence has three important properties:
  //     1) It never *directly* writes to memory below %esp.  It always begins
  //        by moving %esp down, then writing to locations above it.  This way,
  //        if the kernel decides to interrupt, it won't stomp all over our
  //        values before we get a chance to use them.
  //     2) It is designed to support easy de-allocation of this space by
  //        ending with %esp pointing to where we stored the original %esp.
  //     3) Care has been taken to properly restore both %eax and %eflags
  //        by using "lea" instead of "add" or "sub," and saving the necessary
  //        flags around the "and" instruction.
  //
  // Saving of the flags register can be skipped if the register is not live.
  void EmitterIA32::emitStackAlign(int offset, codeGen &gen) {
    int off = offset + 4 + IA32_STACK_ALIGNMENT;
    int saveSlot1 = 0 + IA32_STACK_ALIGNMENT;
    int saveSlot2 = 4 + IA32_STACK_ALIGNMENT;
    RealRegister esp = RealRegister(REGNUM_ESP);
    RealRegister eax = RealRegister(REGNUM_EAX);
    RealRegister enull = RealRegister(Null_Register);

    bool saveFlags = false;
    if(gen.rs()->checkVolatileRegisters(gen, registerSlot::live)) {
      saveFlags = true; // We need to save the flags register
      off += 4;         // Allocate stack space to store the flags
    }

    ::emitLEA(esp, enull, 0, -off, esp, gen);
    emitMovRegToRM(esp, saveSlot1, eax, gen);
    if(saveFlags) {
      emitSimpleInsn(0x9f, gen);
      emitSaveO(gen);
      emitMovRegToRM(esp, saveSlot2, eax, gen);
    }
    ::emitLEA(esp, enull, 0, off, eax, gen);
    emitOpExtRegImm8(0x83, EXTENDED_0x83_AND, esp, -IA32_STACK_ALIGNMENT, gen);
    emitMovRegToRM(esp, 0, eax, gen);
    if(saveFlags) {
      emitMovRMToReg(eax, eax, -off + saveSlot2, gen);
      emitRestoreO(gen);
      emitSimpleInsn(0x9e, gen);
      emitMovRMToReg(eax, esp, 0, gen);
    }
    emitMovRMToReg(eax, eax, -off + saveSlot1, gen);
  }

  void EmitterIA32::emitStore(Address addr, Register src, int size, codeGen &gen) {
    RealRegister r = gen.rs()->loadVirtual(src, gen);
    if(size == 1) {
      emitMovRegToMB(addr, r, gen);
    } else if(size == 2) {
      emitMovRegToMW(addr, r, gen);
    } else {
      emitMovRegToM(addr, r, gen);
    }
  }

  void EmitterIA32::emitStoreFrameRelative(Address offset, Register src, Register scratch,
                                           int /*size*/, codeGen &gen) {
    if(gen.bt()->createdFrame) {
      RealRegister src_r = gen.rs()->loadVirtual(src, gen);
      RealRegister scratch_r = gen.rs()->loadVirtual(scratch, gen);
      emitMovRMToReg(scratch_r, RealRegister(REGNUM_EBP), 0, gen);
      emitMovRegToRM(scratch_r, offset, src_r, gen);
      return;
    }
    RealRegister src_r = gen.rs()->loadVirtual(src, gen);
    emitMovRegToRM(RealRegister(REGNUM_EBP), offset, src_r, gen);
  }

  void EmitterIA32::emitStoreImm(Address addr, int imm, codeGen &gen, bool /*noCost*/) {
    emitMovImmToMem(addr, imm, gen);
  }

  void EmitterIA32::emitStoreIndir(Register addr_reg, Register src, int /*size*/, codeGen &gen) {
    RealRegister src_r = gen.rs()->loadVirtual(src, gen);
    RealRegister addr_r = gen.rs()->loadVirtual(addr_reg, gen);
    emitMovRegToRM(addr_r, 0, src_r, gen);
  }

  void EmitterIA32::emitStoreOrigRegister(Address, Register, codeGen &) {
    assert(0);
  }

  void EmitterIA32::emitStoreRelative(Register, Address, Register, int, codeGen &) {
    assert(0);
    return;
  }

  void EmitterIA32::emitStoreShared(Register source, const image_variable *var, bool is_local,
                                    int /*size*/, codeGen &gen) {
    // create or retrieve jump slot
    // Address addr = getInterModuleVarAddr(var, gen);
    Address addr;
    if(!is_local) {
      addr = getInterModuleVarAddr(var, gen);
    } else {
      addr = (Address)var->getOffset();
    }

    // temporary virtual register for storing destination address
    Register dest = gen.rs()->allocateRegister(gen, false);
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    emitMovPCRMToReg(dest_r, addr - gen.currAddr(), gen, !is_local);
    emitStoreIndir(dest, source, 4, gen);
    gen.rs()->freeRegister(dest);
  }

  void EmitterIA32::emitTimesImm(Register dest, Register src1, RegValue src2imm, codeGen &gen) {
    int result;

    RealRegister src1_r = gen.rs()->loadVirtual(src1, gen);
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);

    if(src2imm == 1) {
      emitMovRegToReg(dest_r, src1_r, gen);
      return;
    }

    if(isPowerOf2(src2imm, result) && result <= MAX_IMM8) {
      // sal dest, result
      if(src1 != dest) {
        emitMovRegToReg(dest_r, src1_r, gen);
      }
      emitOpExtRegImm8(0xC1, 4, dest_r, static_cast<char>(result), gen);
    } else {
      // imul src1 * src2imm -> dest_r
      emitOpRegRegImm(0x69, dest_r, src1_r, src2imm, gen);
    }
  }

  bool EmitterIA32::emitXorRegImm(Register dest, int imm, codeGen &gen) {
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    emitOpRegImm(0x6, dest_r, imm, gen);
    return true;
  }

  bool EmitterIA32::emitXorRegReg(Register dest, Register base, codeGen &gen) {
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    emitOpRegReg(XOR_R32_RM32, dest_r, RealRegister(base), gen);
    return true;
  }

  bool EmitterIA32::emitXorRegRM(Register dest, Register base, int disp, codeGen &gen) {
    RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
    emitOpRegRM(XOR_R32_RM32 /*0x33*/, dest_r, RealRegister(base), disp, gen);
    return true;
  }

  bool EmitterIA32::emitXorRegSegReg(Register /*dest*/, Register base, int disp, codeGen &gen) {
    // WARNING: dest is hard-coded to EDX currently
    x86::emitSegPrefix(base, gen);
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0x33);
    append_memory_as_byte(insn, 0x15);
    append_memory_as_byte(insn, disp);
    append_memory_as_byte(insn, 0x00);
    append_memory_as_byte(insn, 0x00);
    append_memory_as_byte(insn, 0x00);
    SET_PTR(insn, gen);
    return true;
  }

}}
