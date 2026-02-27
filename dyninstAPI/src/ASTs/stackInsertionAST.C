#include "Architecture.h"
#include "ast_helpers.h"
#include "codegen.h"
#include "RegisterConversion.h"
#include "registers/MachRegister.h"
#include "stackInsertionAST.h"

#include <iomanip>
#include <sstream>

namespace Dyninst { namespace DyninstAPI {

std::string stackInsertionAST::format(std::string indent) {
  std::stringstream ret;
  ret << indent << "StackInsert/" << std::hex << this;
  ret << "(size " << size << ")";
  if(type == CANARY_AST) {
    ret << " (is canary)";
  }
  ret << std::endl;
  return ret.str();
}

#ifndef cap_stack_mods

bool stackInsertionAST::generateCode_phase2(codeGen &, bool, Address &, Dyninst::Register &) {
  return false;
}

#else

bool stackInsertionAST::generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &,
                                             Dyninst::Register &) {
  // Turn off default basetramp instrumentation saves & restores
  gen.setInsertNaked(true);
  gen.setModifiedStackFrame(true);

  Dyninst::Register reg_sp = convertRegID(MachRegister::getStackPointer(gen.getArch()));

  Emitterx86 *emitter = dynamic_cast<Emitterx86 *>(gen.codeEmitter());
  assert(emitter);

  if(type == stackAST::GENERIC_AST) {
    /* We're going to use a MOV to insert the new value, and a LEA to update the SP
     * This is instead of using a push, which requires a free register */

    /* Move stack pointer to accomodate new value */
    if(gen.getArch() == Arch_x86) {
      emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -size, reg_sp, gen);
    } else if(gen.getArch() == Arch_x86_64) {
      emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -size, reg_sp, gen);
    }

  } else if(type == stackAST::CANARY_AST) {

    // Find a register to use
    Dyninst::Register canaryReg = Dyninst::Null_Register;
    bool needSaveAndRestore = true;

    // 64-bit requires stack alignment
    // We'll do this BEFORE we push the canary for two reasons:
    // 1. Easier to pop the canary in the check at the end of the function
    // 2. Ensures that the canary will get overwritten (rather than empty alignment space) in case
    // of an overflow
    if(gen.getArch() == Arch_x86_64) {
      allocateCanaryRegister(gen, noCost, canaryReg, needSaveAndRestore);

      int canarySize = 8;
      int off = AMD64_STACK_ALIGNMENT - canarySize; // canary
      emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -off, reg_sp, gen);
    } else {
      canaryReg = REGNUM_EAX;
      needSaveAndRestore = true;
    }

    // Save canaryReg value if necessary
    if(needSaveAndRestore) {
      if(gen.getArch() == Arch_x86) {
        int disp = 4;
        emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -disp, reg_sp, gen);
        gen.codeEmitter()->emitPush(gen, canaryReg);
        emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, 2 * disp, reg_sp, gen);
      } else if(gen.getArch() == Arch_x86_64) {
        int disp = 8;
        emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -disp, reg_sp, gen);
        gen.codeEmitter()->emitPush(gen, canaryReg);
        emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, 2 * disp, reg_sp, gen);
      }
    }

    // Set up canary value
    // CURRENT USES GLIBC-PROVIDED VALUE;
    // from gcc/config/i386/gnu-user64.h:
    //  #ifdef TARGET_LIBC_PROVIDES_SSP
    //  /* i386 glibc provides __stack_chk_guard in %gs:0x14,
    //      x32 glibc provides it in %fs:0x18.
    //      x86_64 glibc provides it in %fs:0x28.  */
    //  #define TARGET_THREAD_SSP_OFFSET (TARGET_64BIT ? (TARGET_X32 ? 0x18 : 0x28) : 0x14))) */

    // goal:
    //      x86:    mov %fs:0x14, canaryReg
    //      x86_64: mov %gs:0x28, canaryReg
    if(gen.getArch() == Arch_x86) {
      emitter->emitLoadRelativeSegReg(canaryReg, 0x14, REGNUM_GS, 4, gen);
    } else if(gen.getArch() == Arch_x86_64) {
      emitter->emitLoadRelativeSegReg(canaryReg, 0x28, REGNUM_FS, 8, gen);
    }

    // Push the canary value
    gen.codeEmitter()->emitPush(gen, canaryReg);

    // Clear canary register to prevent info leaking
    emitter->emitXorRegReg(canaryReg, canaryReg, gen);

    // Restore canaryReg value if necessary
    if(needSaveAndRestore) {
      if(gen.getArch() == Arch_x86) {
        int disp = 4;
        emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -disp, reg_sp, gen);
        gen.codeEmitter()->emitPop(gen, canaryReg);
      } else if(gen.getArch() == Arch_x86_64) {
        int disp = 8;
        emitter->emitLEA(reg_sp, Dyninst::Null_Register, 0, -disp, reg_sp, gen);
        emitter->emitPop(gen, canaryReg);
      }
    }

    // C&P from nullNode
    decUseCount(gen);
  }

  return true;
}

#endif

}}
