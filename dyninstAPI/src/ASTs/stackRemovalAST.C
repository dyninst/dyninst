#include "ast_helpers.h"
#include "RegisterConversion.h"
#include "stackRemovalAST.h"

#include <iomanip>
#include <sstream>

namespace Dyninst { namespace DyninstAPI {

std::string stackRemovalAST::format(std::string indent) {
  std::stringstream ret;
  ret << indent << "StackRemove/" << std::hex << this;
  ret << "(size " << size << ")";
  if(type == stackAST::CANARY_AST) {
    ret << "(is canary)";
  }
  ret << std::endl;
  return ret.str();
}

#ifndef cap_stack_mods

bool stackRemovalAST::generateCode_phase2(codeGen &, bool, Address &, Dyninst::Register &) {
  (void)func_;
  (void)canaryAfterPrologue_;
  (void)canaryHeight_;
  return false;
}

#else

#include "Architecture.h"
#include "codegen.h"
#include "RegisterConversion.h"
#include "registers/MachRegister.h"

bool stackRemovalAST::generateCode_phase2(codeGen &gen, bool noCost, Address &,
                                             Dyninst::Register &) {
  // Turn off default basetramp instrumentation saves & restores
  gen.setInsertNaked(true);
  gen.setModifiedStackFrame(true);

  Dyninst::Register reg_sp = convertRegID(MachRegister::getStackPointer(gen.getArch()));

  Emitterx86 *emitter = dynamic_cast<Emitterx86 *>(gen.codeEmitter());
  assert(emitter);

  if(type == stackAST::GENERIC_AST) {
    /* Adjust stack pointer by size */
    int disp = size;
    if(gen.getArch() == Arch_x86) {
      emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, disp, reg_sp, gen);
    } else if(gen.getArch() == Arch_x86_64) {
      emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, disp, reg_sp, gen);
    }
  } else if(type == stackAST::CANARY_AST) {
    //        gen.setCanary(true);

    // Find a register to use
    Dyninst::Register canaryReg = Dyninst::Null_Register;
    bool needSaveAndRestore = true;
    if(gen.getArch() == Arch_x86_64) {
      allocateCanaryRegister(gen, noCost, canaryReg, needSaveAndRestore);
    } else {
      canaryReg = REGNUM_EDX;
      gen.rs()->noteVirtualInReal(canaryReg, RealRegister(canaryReg));
      needSaveAndRestore = true;
    }
    // Save canaryReg value if necessary
    if(needSaveAndRestore) {
      if(gen.getArch() == Arch_x86) {
        int disp = 4;
        gen.codeEmitter()->emitPush(gen, canaryReg);
        emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, disp, reg_sp, gen);
      } else if(gen.getArch() == Arch_x86_64) {
        int disp = 8;
        emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, disp, reg_sp, gen);
      }
    }

    // Retrieve canary value and verify its integrity
    if(!canaryAfterPrologue_) {
      gen.codeEmitter()->emitPop(gen, canaryReg);
    } else {
      Address canaryOffset = -1 * (Address)(canaryHeight_);
      if(gen.getArch() == Arch_x86) {
        Dyninst::Register destReg = reg_sp;
        RealRegister canaryReg_r = gen.rs()->loadVirtualForWrite(canaryReg, gen);
        emitMovRMToReg(canaryReg_r, RealRegister(destReg), canaryOffset, gen);
      } else if(gen.getArch() == Arch_x86_64) {
        Dyninst::Register destReg = reg_sp;
        gen.codeEmitter()->emitLoadRelative(canaryReg, canaryOffset, destReg, 0, gen);
      }
    }

    // CURRENTLY USES GLIBC-PROVIDED VALUE;
    //  /* i386 glibc provides __stack_chk_guard in %gs:0x14,
    //      x32 glibc provides it in %fs:0x18.
    //      x86_64 glibc provides it in %fs:0x28.  */
    // goal:
    //      x86: xor %fs:0x14, canaryReg
    //      x86_64: xor %gs:0x28, canaryReg
    if(gen.getArch() == Arch_x86) {
      emitter->emitXorRegSegReg(canaryReg, REGNUM_GS, 0x14, gen);
    } else if(gen.getArch() == Arch_x86_64) {
      emitter->emitXorRegSegReg(canaryReg, REGNUM_FS, 0x28, gen);
    }

    // Restore canaryReg if necessary
    if(needSaveAndRestore) {
      if(gen.getArch() == Arch_x86) {
        int disp = 4;
        emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -disp, reg_sp, gen);
        gen.codeEmitter()->emitPop(gen, canaryReg);
      } else if(gen.getArch() == Arch_x86_64) {
        int disp = 8;
        emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -disp, reg_sp, gen);
        gen.codeEmitter()->emitPop(gen, canaryReg);
      }
    }

    // Fix up the stack in the canaryAfterPrologue case
    if(canaryAfterPrologue_) {
      if(gen.getArch() == Arch_x86) {
        int disp = 4;
        emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -1 * canaryHeight_ + disp, reg_sp, gen);
      } else if(gen.getArch() == Arch_x86_64) {
        int disp = 8;
        emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -1 * canaryHeight_ + disp, reg_sp, gen);
      }
    }

    // Re-align the stack
    if(gen.getArch() == Arch_x86_64) {
      // 64-bit requires stack alignment (this will include canary cleanup)
      int canarySize = 8;
      int off = AMD64_STACK_ALIGNMENT - canarySize;
      emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, off, reg_sp, gen);
    }

    // If the canary value is valid, jmp to next expected instruction
    int condition = 0x4;
    int offset = 1;        // This is just the placeholder
    bool willRegen = true; // This gets the longer form of the jcc, so we can patch it up
    codeBufIndex_t jccIndex = gen.getIndex();
    emitJcc(condition, offset, gen, willRegen);

    // Otherwise, call specified failure function
    // NOTE: Skipping saving live registers that will be clobbered by the call because this will be
    // non-returning
    std::vector<codeGenASTPtr> operands;
    func_instance *func = func_; // c&p'd from AstCallNode
    codeBufIndex_t preCallIndex = gen.getIndex();
    emitter->emitCallInstruction(gen, func, canaryReg);
    codeBufIndex_t postCallIndex = gen.getIndex();

    // Fix-up the jcc
    offset = postCallIndex - preCallIndex;
    gen.setIndex(jccIndex);
    emitJcc(condition, offset, gen, willRegen);
    gen.setIndex(postCallIndex);

    decUseCount(gen);
  }

  return true;
}

#endif

}}
