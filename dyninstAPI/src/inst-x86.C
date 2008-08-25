/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/*
 * inst-x86.C - x86 dependent functions and code generator
 * $Id: inst-x86.C,v 1.288 2008/08/25 16:21:36 mlam Exp $
 */
#include <iomanip>

#include <limits.h>
#include "common/h/headers.h"
#include "common/h/Dictionary.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/h/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/instPoint.h" // includes instPoint-x86.h

#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/binaryEdit.h"

#include "dyninstAPI/src/registerSpace.h"

#include "dyninstAPI/src/instP.h" // class returnInstance
#include "dyninstAPI/src/rpcMgr.h"
#include "dyninstAPI/src/dyn_thread.h"
//#include "InstrucIter.h"

#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

#include <sstream>

class ExpandInstruction;
class InsertNops;

extern bool relocateFunction(process *proc, instPoint *&location);

extern bool isPowerOf2(int value, int &result);
void BaseTrampTrapHandler(int); //siginfo_t*, ucontext_t*);


extern "C" int cpuidCall();

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/



/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/* A quick model for the "we're done, branch back/ILL" tramp end */


/*
 * Worst-case scenario for how much room it will take to relocate
 * an instruction -- used for allocating the new area
 */
unsigned relocatedInstruction::maxSizeRequired() {
    return insn->spaceToRelocate();
}

void registerSpace::initialize32() {
    static bool done = false;
    if (done) return;
    done = true;

    // On 32-bit x86 we use stack slots as "registers"; therefore we can
    // create an arbitrary number, and use them. However, this can bite us
    // if we want to use actual registers. Any ideas?
    
    pdvector<registerSlot *> registers;

#if 0
    // When we use 
    registerSlot eax = registerSlot(REGNUM_EAX,
                                    true, // Off-limits due to our "stack slot" register mechanism
                                    false, // Dead at function call?
                                    registerSlot::GPR);
    registerSlot ecx = registerSlot(REGNUM_ECX,
                                    true,
                                    false,                                    
                                    registerSlot::GPR);
    registerSlot edx = registerSlot(REGNUM_EDX,
                                    true,
                                    false,
                                    registerSlot::GPR);
    registerSlot ebx = registerSlot(REGNUM_EBX,
                                    true,
                                    false,
                                    registerSlot::GPR);
    registerSlot esp = registerSlot(REGNUM_ESP,
                                    true, // Off-limits...
                                    true,
                                    registerSlot::SPR); // I'd argue the SP is a special-purpose reg
    registerSlot ebp = registerSlot(REGNUM_EBP,
                                    true,
                                    true,
                                    registerSlot::SPR);
    registerSlot esi = registerSlot(REGNUM_ESI,
                                    true,
                                    false,
                                    registerSlot::GPR);
    registerSlot edi = registerSlot(REGNUM_EDI,
                                    true,
                                    false,
                                    registerSlot::GPR);
    
    registers.push_back(eax);
    registers.push_back(ecx);
    registers.push_back(edx);
    registers.push_back(ebx);
    registers.push_back(esp);
    registers.push_back(ebp);
    registers.push_back(esi);
    registers.push_back(edi);
#endif
    // FPRs...

    // SPRs...
    
    // "Virtual" registers
    for (unsigned i = 1; i <= NUM_VIRTUAL_REGISTERS; i++) {
        char buf[128];
        sprintf(buf, "virtGPR%d", i);

        registerSlot *virt = new registerSlot(i,
                                              buf,
                                              false,
                                              registerSlot::deadAlways,
                                              registerSlot::GPR);
        registers.push_back(virt);
    }

    // Create a single FPR representation to represent
    // whether any FPR is live
    registerSlot *fpr = new registerSlot(IA32_FPR_VIRTUAL_REGISTER,
                                         "virtFPR",
                                         true, // off-limits...
                                         registerSlot::liveAlways,
                                         registerSlot::FPR);
    registers.push_back(fpr);

    // And a "do we save the flags" "register"
    registers.push_back(new registerSlot(IA32_FLAG_VIRTUAL_REGISTER,
                                         "virtFlags",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    // Create the global register space
    registerSpace::createRegisterSpace(registers);

    // Define:
    // callRead
    // callWritten
    // Fortunately, both are basically zero...
    
    // callRead: no change
    // callWritten: write to the flags
    // TODO FIXME

    // Define:
    // callRead - argument registers
    // callWritten - RAX

    // Can't use numRegisters here because we're depending
    // on the REGNUM_FOO numbering
#if defined(cap_liveness)
    returnRead_ = getBitArray();
    // Return reads no registers

    callRead_ = getBitArray();
    // CallRead reads no registers

    // TODO: Fix this for platform-specific calling conventions

    // Assume calls write flags
    callWritten_ = callRead_;
    for (unsigned i = REGNUM_OF; i <= REGNUM_RF; i++) 
        callWritten_[i] = true;


    // And assume a syscall reads or writes _everything_
    syscallRead_ = getBitArray().set();
    syscallWritten_ = syscallRead_;
#endif
}

#if defined(cap_32_64)
void registerSpace::initialize64() {
    static bool done = false;
    if (done) return;
    done = true;

    // Create the 64-bit registers
    // Well, let's just list them....

    // Calling ABI:
    // rax, rcx, rdx, r8, r9, r10, r11 are not preserved across a call
    // However, rcx, rdx, r8, and r9 are used for arguments, and therefore
    // should be assumed live. 
    // So rax, r10, r11 are dead at a function call.

    registerSlot * rax = new registerSlot(REGNUM_RAX,
                                          "rax",
                                          true, // We use it implicitly _everywhere_
                                          registerSlot::deadABI,
                                          registerSlot::GPR);
    registerSlot * rcx = new registerSlot(REGNUM_RCX,
                                          "rcx",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * rdx = new registerSlot(REGNUM_RDX,
                                          "rdx",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * rbx = new registerSlot(REGNUM_RBX,
                                          "rbx",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * rsp = new registerSlot(REGNUM_RSP,
                                          "rsp",
                                          true, // Off-limits...
                                          registerSlot::liveAlways,
                                          registerSlot::SPR); 
    registerSlot * rbp = new registerSlot(REGNUM_RBP,
                                          "rbp",
                                          true,
                                          registerSlot::liveAlways,
                                          registerSlot::SPR);
    registerSlot * rsi = new registerSlot(REGNUM_RSI,
                                          "rsi",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * rdi = new registerSlot(REGNUM_RDI,
                                          "rdi",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * r8 = new registerSlot(REGNUM_R8,
                                         "r8",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR);
    registerSlot * r9 = new registerSlot(REGNUM_R9,
                                         "r9",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR);
    registerSlot * r10 = new registerSlot(REGNUM_R10,
                                          "r10",
                                          false,
                                          registerSlot::deadABI,
                                          registerSlot::GPR);
    registerSlot * r11 = new registerSlot(REGNUM_R11,
                                          "r11",
                                          false,
                                          registerSlot::deadABI,
                                          registerSlot::GPR);
    registerSlot * r12 = new registerSlot(REGNUM_R12,
                                          "r12",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * r13 = new registerSlot(REGNUM_R13,
                                          "r13",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * r14 = new registerSlot(REGNUM_R14,
                                          "r14",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * r15 = new registerSlot(REGNUM_R15,
                                          "r15",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);

    pdvector<registerSlot *> registers;
    registers.push_back(rax);
    registers.push_back(rbx);
    registers.push_back(rsp);
    registers.push_back(rbp);
    registers.push_back(r10);
    registers.push_back(r11);
    registers.push_back(r12);
    registers.push_back(r13);
    registers.push_back(r14);
    registers.push_back(r15);

    // Put the call parameter registers last so that we are not
    // likely to allocate them for general purposes
    registers.push_back(r8);
    registers.push_back(r9);
    registers.push_back(rcx);
    registers.push_back(rdx);
    registers.push_back(rsi);
    registers.push_back(rdi);


    registers.push_back(new registerSlot(REGNUM_OF,
                                         "of",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_SF,
                                         "sf",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_ZF,
                                         "zf",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_AF,
                                         "af",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_PF,
                                         "pf",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_CF,
                                         "cf",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_TF,
                                         "tf",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_IF,
                                         "if",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_DF,
                                         "df",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_NT,
                                         "nt",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_RF,
                                         "rf",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));

    registers.push_back(new registerSlot(REGNUM_DUMMYFPR,
                                         "dummyFPR",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::FPR));

    // For registers that we really just don't care about.
    registers.push_back(new registerSlot(REGNUM_IGNORED,
                                         "ignored",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));

    registerSpace::createRegisterSpace64(registers);

    // Define:
    // callRead - argument registers
    // callWritten - RAX

#if defined(cap_liveness)
    returnRead64_ = getBitArray();
    returnRead64_[REGNUM_RAX] = true;
    // Returns also "read" any callee-saved registers
    returnRead64_[REGNUM_RBX] = true;
    returnRead64_[REGNUM_R12] = true;
    returnRead64_[REGNUM_R13] = true;
    returnRead64_[REGNUM_R14] = true;
    returnRead64_[REGNUM_R15] = true;

    //returnRead64_[REGNUM_R10] = true;
    

    callRead64_ = getBitArray();
    callRead64_[REGNUM_RCX] = true;
    callRead64_[REGNUM_RDX] = true;
    callRead64_[REGNUM_R8] = true;
    callRead64_[REGNUM_R9] = true;
    callRead64_[REGNUM_RDI] = true;
    callRead64_[REGNUM_RSI] = true;

    // Anything in those four is not preserved across a call...
    // So we copy this as a shorthand then augment it
    callWritten64_ = callRead64_;

    // As well as RAX, R10, R11
    callWritten64_[REGNUM_RAX] = true;
    callWritten64_[REGNUM_R10] = true;
    callWritten64_[REGNUM_R11] = true;
    // And flags
    for (unsigned i = REGNUM_OF; i <= REGNUM_RF; i++) 
        callWritten64_[i] = true;

    // What about floating point?

    // And assume a syscall reads or writes _everything_
    syscallRead64_ = getBitArray().set();
    syscallWritten64_ = syscallRead_;

#endif

}
#endif

void registerSpace::initialize()
{
    static bool inited = false;
    
    if (inited) return;
    inited = true;

    initialize32();
#if defined(cap_32_64)
    initialize64();
#endif
}

/* This makes a call to the cpuid instruction, which returns an int where each bit is 
   a feature.  Bit 24 contains whether fxsave is possible, meaning that xmm registers
   are saved. */
#if defined(os_windows)
int cpuidCall() {
    DWORD result;
    _asm {
        xor eax, eax
        cpuid
        mov result, eax
    }
    return result;
}
#endif
#if !defined(x86_64_unknown_linux2_4)
bool xmmCapable()
{
  int features = cpuidCall();
  char * ptr = (char *)&features;
  ptr += 3;
  if (0x1 & (*ptr))
    return true;
  else
    return false;
}
#else
bool xmmCapable()
{
  return true;
}
#endif


bool baseTramp::generateSaves(codeGen& gen, registerSpace*) {
    return gen.codeEmitter()->emitBTSaves(this, gen);
}

bool baseTramp::generateRestores(codeGen &gen, registerSpace*) {

    return gen.codeEmitter()->emitBTRestores(this, gen);
}

bool baseTramp::generateMTCode(codeGen &gen, registerSpace*) {
    return gen.codeEmitter()->emitBTMTCode(this, gen);
}

bool baseTramp::generateGuardPreCode(codeGen &gen,
                                     codeBufIndex_t &guardJumpOffset,
				     registerSpace*) {

    return gen.codeEmitter()->emitBTGuardPreCode(this, gen, guardJumpOffset);
}

bool baseTramp::generateGuardPostCode(codeGen &gen,
				      codeBufIndex_t &guardTargetIndex,
				      registerSpace *) {

    return gen.codeEmitter()->emitBTGuardPostCode(this, gen, guardTargetIndex);
}

bool baseTrampInstance::finalizeGuardBranch(codeGen &gen,
                                            int disp) {

    // This would be a bad thing...
    assert(disp > 0);
    // Assumes that preCode is generated
    // and we're now finalizing the jump to go
    // past whatever miniTramps may have been.
    
    // x86: we use the smallest jump we can and
    // noop the rest.
    
    // Note: must be a conditional jump
    
    // Gen is at the branch point
    
    unsigned start = gen.used();
    int jumpSize = 0;

    emitJcc(0x04, disp-2, gen, false);

    jumpSize = gen.used() - start;

    gen.fill(baseT->guardBranchSize - jumpSize,
             codeGen::cgNOP);
        
    return true;
}
       

bool baseTramp::generateCostCode(codeGen &gen, unsigned &costUpdateOffset,
                                 registerSpace *) {
    Address costAddr = proc()->getObservedCostAddr();
    if (!costAddr) return false;

    return gen.codeEmitter()->emitBTCostCode(this, gen, costUpdateOffset);
}

// And update the same in an atomic action
void baseTrampInstance::updateTrampCost(unsigned cost) {
    if (!baseT->costSize) return;

    assert(baseT->costSize);

    Address trampCostAddr = trampPreAddr() + baseT->costValueOffset;

    codeGen gen(baseT->costSize);

    Address costAddr = proc()->getObservedCostAddr();

    if (proc()->getAddressWidth() == 4) {
       emitAddMemImm32(costAddr, cost, gen);    
    }
    else {
       emitMovImmToReg64(REGNUM_RAX, costAddr, true, gen);
       emitOpRMImm(0x81, 0, REGNUM_RAX, 0, cost, gen);
    }

    // We can assert this here as we regenerate the entire
    // cost section
    assert(gen.used() == baseT->costSize);
    
    proc()->writeDataSpace((void *)trampCostAddr,
                           gen.used(),
                           (void *)gen.start_ptr());
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void emitJccR8(int condition_code, char jump_offset,
               codeGen &gen) {
    GET_PTR(insn, gen);
    *insn++ = static_cast<unsigned char>(condition_code);
    *insn++ = jump_offset;
    SET_PTR(insn, gen);
}

// VG(8/15/02): nicer jcc: condition is the tttn field.
// Because we generate jumps twice, once with bogus 0
// offset, and then with the right offset, the instruction
// may be longer (and overwrite something else) the 2nd time.
// So willRegen defaults to true and always generates jcc near
// (the longer form)

// TODO: generate JEXCZ as well
void emitJcc(int condition, int offset,
             codeGen &gen, bool willRegen) /* = true */
{
   unsigned char opcode;
   GET_PTR(insn, gen);
   
   assert(condition >= 0 && condition <= 0x0F);
   
   if(!willRegen && (offset >= -128 && offset <= 127)) { // jcc rel8
      opcode = 0x70 | (unsigned char)condition;
      *insn++ = opcode;
      *insn++ = (unsigned char) (offset & 0xFF);
   }
   else { // jcc near rel32
      opcode = 0x80 | (unsigned char)condition;
      *insn++ = 0x0F;
      *insn++ = opcode;
      *((int*)insn) = offset;
      insn += sizeof(int);
   }
   SET_PTR(insn, gen);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/**
 * tramp_pre_frame_size is the amount of space the base trampoline allocates
 * on the stack before setting up a stack frame.  It's needed to stack
 * walk out of base tramps.  Should be treated as a constant, but the
 * C++ scoping rules for const are stupid.
 **/

int tramp_pre_frame_size_32 = 36 + STACK_PAD_CONSTANT; //Stack space allocated by 'pushf; pusha'

int tramp_pre_frame_size_64 = 8 + 16 * 8 + STACK_PAD_CONSTANT; // stack space allocated by pushing flags and 16 GPRs
                                                // and skipping the 128-byte red zone

bool can_do_relocation(process *proc,
                       const pdvector<pdvector<Frame> > &stackWalks,
                       int_function *instrumented_func)
{
   bool can_do_reloc = true;

   // for every vectors of frame, ie. thread stack walk, make sure can do
   // relocation
   Address begAddr = instrumented_func->getAddress();
   for (unsigned walk_itr = 0; walk_itr < stackWalks.size(); walk_itr++) {
     pdvector<int_function *> stack_funcs =
       proc->pcsToFuncs(stackWalks[walk_itr]);
     
     // for every frame in thread stack walk
     for(unsigned i=0; i<stack_funcs.size(); i++) {
       int_function *stack_func = stack_funcs[i];
       Address pc = stackWalks[walk_itr][i].getPC();
       
       if( stack_func == instrumented_func ) {
	 // Catchup doesn't occur on instPoinst in relocated function when
	 // the original function is on the stack.  This leads to the
	 // timer never being called for timer metrics.  A solution still
	 // needs to be worked out.
	 if(pc >= begAddr && pc <= begAddr+JUMP_REL32_SZ) {
	   // can't relocate since within first five bytes
	   can_do_reloc = false;
	 } else {
             // Need to check whether each entry point has enough room
             // to patch in a jump; technically, this is only needed
             // if we're _in_ the function as control may transfer to
             // the middle of the jump(s) out.

             assert(0);
         }
	 break;
       }
     }
   }
   
   return can_do_reloc;
}

/**************************************************************
 *
 *  code generator for x86
 *
 **************************************************************/




#define MAX_BRANCH	(0x1<<31)

Address getMaxBranch() {
  return (Address)MAX_BRANCH;
}


bool doNotOverflow(int)
{
   //
   // this should be changed by the correct code. If there isn't any case to
   // be checked here, then the function should return TRUE. If there isn't
   // any immediate code to be generated, then it should return FALSE - naim
   //
   // any int value can be an immediate on the pentium
    return(true);
}



/* build the MOD/RM byte of an instruction */
static inline unsigned char makeModRMbyte(unsigned Mod, unsigned Reg,
                                          unsigned RM)
{
   return static_cast<unsigned char>(((Mod & 0x3) << 6) + ((Reg & 0x7) << 3) + (RM & 0x7));
}

// VG(7/30/02): Build the SIB byte of an instruction */
static inline unsigned char makeSIBbyte(unsigned Scale, unsigned Index,
                                        unsigned Base)
{
   return static_cast<unsigned char>(((Scale & 0x3) << 6) + ((Index & 0x7) << 3) + (Base & 0x7));
}

/* 
   Emit the ModRM byte and displacement for addressing modes.
   base is a register (EAX, ECX, REGNUM_EDX, EBX, EBP, REGNUM_ESI, REGNUM_EDI)
   disp is a displacement
   reg_opcode is either a register or an opcode
*/
void emitAddressingMode(Register base, RegValue disp,
                        int reg_opcode, codeGen &gen)
{
   // MT linux uses ESP+4
   // we need an SIB in that case
   if (base == REGNUM_ESP) {
      emitAddressingMode(REGNUM_ESP, Null_Register, 0, disp, reg_opcode, gen);
      return;
   }
   GET_PTR(insn, gen);
   if (base == Null_Register) {
      *insn++ = makeModRMbyte(0, reg_opcode, 5);
      *((int *)insn) = disp;
      insn += sizeof(int);
   } else if (disp == 0 && base != REGNUM_EBP) {
      *insn++ = makeModRMbyte(0, reg_opcode, base);
   } else if (disp >= -128 && disp <= 127) {
      *insn++ = makeModRMbyte(1, reg_opcode, base);
      *((char *)insn++) = (char) disp;
   } else {
      *insn++ = makeModRMbyte(2, reg_opcode, base);
      *((int *)insn) = disp;
      insn += sizeof(int);
   }
   SET_PTR(insn, gen);
}

// VG(7/30/02): emit a fully fledged addressing mode: base+index<<scale+disp
void emitAddressingMode(Register base, Register index,
                        unsigned int scale, RegValue disp,
                        int reg_opcode, codeGen &gen)
{
   bool needSIB = (base == REGNUM_ESP) || (index != Null_Register);

   if(!needSIB) {
      emitAddressingMode(base, disp, reg_opcode, gen);
      return;
   }
   
   assert(index != REGNUM_ESP);
   
   if(index == Null_Register) {
      assert(base == REGNUM_ESP); // not necessary, but sane
      index = 4;           // (==REGNUM_ESP) which actually means no index in SIB
   }

   GET_PTR(insn, gen);
   
   if(base == Null_Register) { // we have to emit [index<<scale+disp32]
      *insn++ = makeModRMbyte(0, reg_opcode, 4);
      *insn++ = makeSIBbyte(scale, index, 5);
      *((int *)insn) = disp;
      insn += sizeof(int);
   }
   else if(disp == 0 && base != REGNUM_EBP) { // EBP must have 0 disp8; emit [base+index<<scale]
       *insn++ = makeModRMbyte(0, reg_opcode, 4);
       *insn++ = makeSIBbyte(scale, index, base);
   }
   else if (disp >= -128 && disp <= 127) { // emit [base+index<<scale+disp8]
      *insn++ = makeModRMbyte(1, reg_opcode, 4);
      *insn++ = makeSIBbyte(scale, index, base);
      *((char *)insn++) = (char) disp;
   }
   else { // emit [base+index<<scale+disp32]
      *insn++ = makeModRMbyte(2, reg_opcode, 4);
      *insn++ = makeSIBbyte(scale, index, base);
      *((int *)insn) = disp;
      insn += sizeof(int);
   }

   SET_PTR(insn, gen);
}


/* emit a simple one-byte instruction */
void emitSimpleInsn(unsigned op, codeGen &gen) {
    GET_PTR(insn, gen);
    *insn++ = static_cast<unsigned char>(op);
    SET_PTR(insn, gen);
}

void emitPushImm(unsigned int imm, codeGen &gen)
{
    GET_PTR(insn, gen);
    *insn++ = 0x68;
    *((unsigned int *)insn) = imm;
    insn += sizeof(unsigned int);
    SET_PTR(insn, gen);
}

// emit a simple register to register instruction: OP dest, src
// opcode is one or two byte
void emitOpRegReg(unsigned opcode, Register dest, Register src,
                  codeGen &gen)
{
    GET_PTR(insn, gen);
    if (opcode <= 0xFF)
       *insn++ = static_cast<unsigned char>(opcode);
    else {
       *insn++ = static_cast<unsigned char>(opcode >> 8);
       *insn++ = static_cast<unsigned char>(opcode & 0xFF);
    }
    // ModRM byte define the operands: Mod = 3, Reg = dest, RM = src
    *insn++ = makeModRMbyte(3, dest, src);
    SET_PTR(insn, gen);
}

// emit OP reg, r/m
void emitOpRegRM(unsigned opcode, Register dest, Register base,
		 int disp, codeGen &gen)
{
    GET_PTR(insn, gen);
    if (opcode <= 0xff) {
       *insn++ = static_cast<unsigned char>(opcode);
    } else {
       *insn++ = static_cast<unsigned char>(opcode >> 8);
       *insn++ = static_cast<unsigned char>(opcode & 0xff);
    }
    SET_PTR(insn, gen);
    emitAddressingMode(base, disp, dest, gen);
}

// emit OP r/m, reg
void emitOpRMReg(unsigned opcode, Register base, int disp,
                               Register src, codeGen &gen) {
   GET_PTR(insn, gen);
   *insn++ = static_cast<unsigned char>(opcode);
   SET_PTR(insn, gen);
   emitAddressingMode(base, disp, src, gen);
}

// emit OP reg, imm32
void emitOpRegImm(int opcode, Register dest, int imm,
                                codeGen &gen) {
    GET_PTR(insn, gen);
   *insn++ = 0x81;
   *insn++ = makeModRMbyte(3, opcode, dest);
   *((int *)insn) = imm;
   insn+= sizeof(int);
   SET_PTR(insn, gen);
}

/*
// emit OP r/m, imm32
void emitOpRMImm(unsigned opcode, Register base, int disp, int imm,
                 codeGen &gen) {
  *insn++ = 0x81;
  emitAddressingMode(base, disp, opcode, insn);
  *((int *)insn) = imm;
  insn += sizeof(int);
}
*/

// emit OP r/m, imm32
void emitOpRMImm(unsigned opcode1, unsigned opcode2,
		 Register base, int disp, int imm,
		 codeGen &gen) {
    GET_PTR(insn, gen);
    *insn++ = static_cast<unsigned char>(opcode1);
    SET_PTR(insn, gen);
    emitAddressingMode(base, disp, opcode2, gen);
    REGET_PTR(insn, gen);
    *((int *)insn) = imm;
    insn += sizeof(int);
    SET_PTR(insn, gen);
}

// emit OP r/m, imm8
void emitOpRMImm8(unsigned opcode1, unsigned opcode2,
		  Register base, int disp, char imm,
		  codeGen &gen) {
    GET_PTR(insn, gen);
    *insn++ = static_cast<unsigned char>(opcode1);
    SET_PTR(insn, gen);
    emitAddressingMode(base, disp, opcode2, gen);
    REGET_PTR(insn, gen);
    *insn++ = imm;
    SET_PTR(insn, gen);
}

// emit OP reg, r/m, imm32
void emitOpRegRMImm(unsigned opcode, Register dest,
		    Register base, int disp, int imm,
		    codeGen &gen) {
    GET_PTR(insn, gen);
    *insn++ = static_cast<unsigned char>(opcode);
    SET_PTR(insn, gen);
    emitAddressingMode(base, disp, dest, gen);
    REGET_PTR(insn, gen);
    *((int *)insn) = imm;
    insn += sizeof(int);
    SET_PTR(insn, gen);
}

// emit MOV reg, reg
void emitMovRegToReg(Register dest, Register src,
                                   codeGen &gen) {
    GET_PTR(insn, gen);
    *insn++ = 0x8B;
    *insn++ = makeModRMbyte(3, dest, src);
    SET_PTR(insn, gen);
}

// emit MOV reg, r/m
void emitMovRMToReg(Register dest, Register base, int disp,
                                  codeGen &gen) {
    GET_PTR(insn, gen);
   *insn++ = 0x8B;
    SET_PTR(insn, gen);
    emitAddressingMode(base, disp, dest, gen);
}

// emit MOV r/m, reg
void emitMovRegToRM(Register base, int disp, Register src,
                                  codeGen &gen) {
    GET_PTR(insn, gen);
   *insn++ = 0x89;
    SET_PTR(insn, gen);
   emitAddressingMode(base, disp, src, gen);
}

// emit MOV m, reg
void emitMovRegToM(int disp, Register src, codeGen &gen)
{
    GET_PTR(insn, gen);
   *insn++ = 0x89;
    SET_PTR(insn, gen);
   emitAddressingMode(Null_Register, disp, src, gen);
}

// emit MOV reg, m
void emitMovMToReg(Register dest, int disp, codeGen &gen)
{
    GET_PTR(insn, gen);
   *insn++ = 0x8B;
    SET_PTR(insn, gen);
   emitAddressingMode(Null_Register, disp, dest, gen);
}

// emit MOVSBL reg, m
void emitMovMBToReg(Register dest, int disp, codeGen &gen)
{
    GET_PTR(insn, gen);
   *insn++ = 0x0F;
   *insn++ = 0xBE;
    SET_PTR(insn, gen);
   emitAddressingMode(Null_Register, disp, dest, gen);
}

// emit MOVSWL reg, m
void emitMovMWToReg(Register dest, int disp, codeGen &gen)
{
    GET_PTR(insn, gen);
   *insn++ = 0x0F;
   *insn++ = 0xBF;
    SET_PTR(insn, gen);
   emitAddressingMode(Null_Register, disp, dest, gen);
}

// emit MOV reg, imm32
void emitMovImmToReg(Register dest, int imm, codeGen &gen)
{
   GET_PTR(insn, gen);
   *insn++ = static_cast<unsigned char>(0xB8 + dest);
   *((int *)insn) = imm;
   insn += sizeof(int);
   SET_PTR(insn, gen);
}

// emit MOV r/m32, imm32
void emitMovImmToRM(Register base, int disp, int imm,
                                  codeGen &gen) {
   GET_PTR(insn, gen);
   *insn++ = 0xC7;
   SET_PTR(insn, gen);
   emitAddressingMode(base, disp, 0, gen);
   REGET_PTR(insn, gen);
   *((int*)insn) = imm;
   insn += sizeof(int);
    SET_PTR(insn, gen);
}

// emit MOV mem32, imm32
void emitMovImmToMem(Address maddr, int imm,
                                   codeGen &gen) {
    // In x86_64, the meaning of the ModRM byte for disp32 has changed.
    // Now, it implies an [RIP] + disp32 address.  To get an absolute
    // address operand (in both x86 and x86_64), the full ModRM + SIB
    // syntax must be used.
    GET_PTR(insn, gen);
    *insn++ = 0xC7;

    // FIXME: To adhere strictly to the x86 and x86_64 ISAs, we specify an
    // absolute (32-bit) address by emitting a ModRM and SIB byte of the
    // following form:
    //     Mod = 00b, Reg = (doesn't matter?), R/M = 100b
    //     base = 101b, index = 100b, scale = (doesn't matter?)
    // Current forms of emitAddressingMode() do not allow for this, and so
    // we do it manually here.  emitAddressingMode() should be made more
    // robust.
    *insn++ = makeModRMbyte(0, 0, 4);
    *insn++ = makeSIBbyte(0, 4, 5);
    *((int *)insn) = maddr;
    insn += sizeof(unsigned);

    *((int*)insn) = imm;
    insn += sizeof(int);
    SET_PTR(insn, gen);
}

// emit Add dword ptr DS:[addr], imm
void emitAddMemImm32(Address addr, int imm, codeGen &gen)
{
    GET_PTR(insn, gen);
   *insn++ = 0x81;
   *insn++ = 0x05;
   *((unsigned *)insn) = addr;
   insn += sizeof(unsigned);
   *((int *)insn) = imm;
   insn += sizeof(int);
    SET_PTR(insn, gen);
}

// emit Add reg, imm32
void emitAddRegImm32(Register reg, int imm, codeGen &gen)
{
    GET_PTR(insn, gen);
   *insn++ = 0x81;
   *insn++ = makeModRMbyte(3, 0, reg);
   *((int *)insn) = imm;
   insn += sizeof(int);
    SET_PTR(insn, gen);
}

// emit Sub reg, reg
static inline void emitSubRegReg(Register dest, Register src,
                                 codeGen &gen)
{
    GET_PTR(insn, gen);
   *insn++ = 0x2B;
   *insn++ = makeModRMbyte(3, dest, src);
    SET_PTR(insn, gen);
}

// help function to select appropriate jcc opcode for a relOp
unsigned char jccOpcodeFromRelOp(unsigned op)
{
   switch (op) {
     case eqOp: return JNE_R8;
     case neOp: return JE_R8;
     case lessOp: return JGE_R8;
     case leOp: return JG_R8;
     case greaterOp: return JLE_R8;
     case geOp: return JL_R8;
     default: assert(0);
   }
   return 0x0;
}

static inline void emitEnter(short imm16, codeGen &gen) {
    GET_PTR(insn, gen);
   *insn++ = 0xC8;
   *((short*)insn) = imm16;
   insn += sizeof(short);
   *insn++ = 0;
    SET_PTR(insn, gen);
}

Register emitFuncCall(opCode, codeGen &, pdvector<AstNodePtr> &, bool, Address) {
	assert(0);
	return 0;
}

// this function just multiplexes between the 32-bit and 64-bit versions
Register emitFuncCall(opCode op, 
                      codeGen &gen,
                      pdvector<AstNodePtr> &operands, 
                      bool noCost,
                      int_function *callee)
{
    Register reg = gen.codeEmitter()->emitCall(op, gen, operands, noCost, callee);
    return reg;
}



/* Recursive function that goes to where our instrumentation is calling
to figure out what registers are clobbered there, and in any function
that it calls, to a certain depth ... at which point we clobber everything

Update-12/06, njr, since we're going to a cached system we are just going to 
look at the first level and not do recursive, since we would have to also
store and reexamine every call out instead of doing it on the fly like before*/

// Should be a member of the registerSpace class?

bool EmitterIA32::clobberAllFuncCall( registerSpace *rs,
                                      int_function *callee)
		   
{
  if (callee == NULL) return false;

  /* This will calculate the values if the first time around, otherwise
     will check preparsed, stored values.
     True - FP Writes are present
     False - No FP Writes
  */

  stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);  
  if (callee->ifunc()->writesFPRs()) {
      for (unsigned i = 0; i < rs->FPRs().size(); i++) {
          rs->FPRs()[i]->beenUsed = true;
      }
  }
  stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);
  return true;
}


void EmitterIA32::setFPSaveOrNot(const int * liveFPReg,bool saveOrNot)
{
   if (liveFPReg != NULL)
   {
      if (liveFPReg[0] == 0 && saveOrNot)
      {
         int * temp = const_cast<int *>(liveFPReg);
         temp[0] = 1;
      }
   }
}


Register EmitterIA32::emitCall(opCode op, 
                               codeGen &gen,
                               const pdvector<AstNodePtr> &operands, 
                               bool noCost, int_function *callee) {
    assert(op == callOp);
    pdvector <Register> srcs;
    int param_size;
    pdvector<Register> saves;
    
    //  Sanity check for NULL address arg
    if (!callee) {
        char msg[256];
        sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out"
                "callee argument", __FILE__, __LINE__);
        showErrorCallback(80, msg);
        assert(0);
    }

   /*
      int emitCallParams(registerSpace *rs, codeGen &gen, 
                   const pdvector<AstNodePtr> &operands, process *proc,
                   int_function *target, const pdvector<AstNodePtr> &ifForks,
                   pdvector<Register> &extra_saves, const instPoint *location,
                   bool noCost);
                   */

   param_size = emitCallParams(gen, operands, callee, saves, noCost);

   emitCallInstruction(gen, callee);

   /*
   // reset the stack pointer
   if (srcs.size() > 0)
      emitOpRegImm(0, REGNUM_ESP, srcs.size()*4, gen); // add esp, srcs.size()*4
  */
   emitCallCleanup(gen, callee, param_size, saves);

   // allocate a (virtual) register to store the return value
   Register ret = gen.rs()->allocateRegister(gen, noCost);
   emitMovRegToRM(REGNUM_EBP, -1*(ret*4), REGNUM_EAX, gen);

   return ret;
}

bool EmitterIA32Dyn::emitCallInstruction(codeGen &gen, int_function *callee) 
{
    // make the call
    // we are using an indirect call here because we don't know the
    // address of this instruction, so we can't use a relative call.
    // TODO: change this to use a direct call
    Register ptr = gen.rs()->allocateRegister(gen, false);
    emitMovImmToReg(ptr, callee->getAddress(), gen);  // mov e_x, addr
    emitOpRegReg(CALL_RM_OPC1, CALL_RM_OPC2, ptr, gen);   // call *(e_x)
    gen.rs()->freeRegister(ptr);
    return true;
}

// TODO: we need to know if we're doing an inter-module call. This
// means we need to know where we're _coming_ from, which currently
// isn't possible. Time to add more to the codeGen structure, such as
// "function where we're inserting new code."  Also, we'll need a
// std::string version that doesn't take a callee...

bool EmitterIA32Stat::emitCallInstruction(codeGen &gen, int_function *callee) {
#ifdef BINEDIT_DEBUG
    fprintf(stdout, "at emitCallInstruction: callee=%s\n", callee->prettyName().c_str());
#endif
    AddressSpace *addrSpace = gen.addrSpace();
    BinaryEdit *binEdit = addrSpace->edit();
    Address dest;
    Register ptr = gen.rs()->allocateRegister(gen, false);

    // find int_function reference in address space
    // (refresh func_map)
    pdvector<int_function *> funcs;
    addrSpace->findFuncsByAll(callee->prettyName(), funcs);

    // test to see if callee is in a shared module
    if (callee->obj()->isSharedLib() && binEdit != NULL) {

        // find the Symbol corresponding to the int_function
        std::vector<Symbol *> syms;
        callee->obj()->parse_img()->getObject()->findSymbolByType(
                syms, callee->symTabName(), Symbol::ST_FUNCTION,
                true, false, true);
        if (syms.size() == 0) {
            char msg[256];
            sprintf(msg, "%s[%d]:  internal error:  cannot find symbol %s"
                    , __FILE__, __LINE__, callee->symTabName().c_str());
            showErrorCallback(80, msg);
            assert(0);
        }
        Symbol *referring = syms[0];

        // have we added this relocation already?
        dest = binEdit->getDependentRelocationAddr(referring);

        if (!dest) {
            // inferiorMalloc addr location and initialize to zero
            dest = binEdit->inferiorMalloc(4);
            unsigned int dat = 0;
            binEdit->writeDataSpace((void*)dest, 4, &dat);

            // add write new relocation symbol/entry
            binEdit->addDependentRelocation(dest, referring);
        }

        // load register with address from jump table
        emitMovMToReg(ptr, dest, gen);                      // mov e_x, *(addr)

    } else {
        dest = callee->getAddress();

        // load register with function address
        emitMovImmToReg(ptr, dest, gen);                    // mov e_x, addr
    }

    // emit call
    emitOpRegReg(CALL_RM_OPC1, CALL_RM_OPC2, ptr, gen);     // call *(e_x)

    gen.rs()->freeRegister(ptr);

    return true;
}
/*
 * emit code for op(src1,src2, dest)
 * ibuf is an instruction buffer where instructions are generated
 * base is the next free position on ibuf where code is to be generated
 */

codeBufIndex_t emitA(opCode op, Register src1, Register /*src2*/, Register dest,
                     codeGen &gen, bool /*noCost*/)
{
   //bperr("emitA(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

    // retval is the address of the jump (if one is created). 
    // It's always the _start_ of the jump, which means that if we need
    // to offset (like x86 (to - (from + insnsize))) we do it later.
    codeBufIndex_t retval = 0;

   switch (op) {
     case ifOp: {
	 // if src1 == 0 jump to dest
	 // src1 is a temporary
	 // dest is a target address
	 retval = gen.codeEmitter()->emitIf(src1, dest, gen);
	 break;
     }
     case branchOp: {
	// dest is the displacement from the current value of insn
	// this will need to work for both 32-bits and 64-bits
	// (since there is no JMP rel64)
	 instruction::generateBranch(gen, dest);
	 retval = gen.getIndex();
	 break;
     }
     case trampTrailer: {
         // generate the template for a jump -- actual jump is generated
         // elsewhere
         retval = gen.getIndex();
         gen.fill(instruction::maxJumpSize(gen.addrSpace()->getAddressWidth()),
                  codeGen::cgNOP);
         instruction::generateIllegal(gen);
         break;
     }
     case trampPreamble: {
	 break;
     }
     default:
        abort();        // unexpected op for this emit!
   }

   return retval;
}

Register emitR(opCode op, Register src1, Register /*src2*/, Register dest,
               codeGen &gen, bool /*noCost*/,
               const instPoint *location, bool /*for_multithreaded*/)
{
    //bperr("emitR(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

    switch (op) {
      case getRetValOp: {
         // dest is a register where we can store the value
         // the return value is in the saved EAX
          gen.codeEmitter()->emitGetRetVal(dest, gen);
          return dest;
      }
      case getParamOp: {
         // src1 is the number of the argument
         // dest is a register where we can store the value
          gen.codeEmitter()->emitGetParam(dest, src1, location->getPointType(), gen);
          return dest;
      }
    case loadRegOp: {
        assert(src1 == 0);

        assert(0);
        return dest;
    }
    default:
        abort();                  // unexpected op for this emit!
    }
    return(Null_Register);        // should never be reached!
}

// VG(07/30/02): Emit a lea dest, [base + index * scale + disp]; dest is a
// real GPR
void emitLEA(Register base, Register index, unsigned int scale,
                           RegValue disp, Register dest, codeGen &gen)
{
    GET_PTR(insn, gen);
    *insn++ = 0x8D;
    SET_PTR(insn, gen);
    emitAddressingMode(base, index, scale, disp, (int)dest, gen);
}

static inline void emitSHL(Register dest, unsigned char pos,
                           codeGen &gen)
{
  //bperr( "Emiting SHL\n");
    GET_PTR(insn, gen);
    *insn++ = 0xC1;
    *insn++ = makeModRMbyte(3 /* rm gives register */,
                            4 /* opcode ext. */, dest);
    *insn++ = pos;
    SET_PTR(insn, gen);
}

void EmitterIA32::emitPushFlags(codeGen &gen) {
    // These crank the saves forward
    emitSimpleInsn(PUSHFD, gen);
}

void EmitterIA32::emitRestoreFlags(codeGen &gen, unsigned offset)
{
    emitOpRMReg(PUSH_RM_OPC1, REGNUM_ESP, offset*4, PUSH_RM_OPC2, gen);
    emitSimpleInsn(POPFD, gen); // popfd
}

void EmitterIA32::emitRestoreFlagsFromStackSlot(codeGen &gen)
{
    emitRestoreFlags(gen, SAVED_EFLAGS_OFFSET);
}

// VG(8/15/02): Emit the jcc over a conditional snippet
void emitJmpMC(int condition, int offset, codeGen &gen)
{
    // What we want: 
    //   mov eax, [original EFLAGS]
    //   push eax
    //   popfd
    //   jCC target   ; CC = !condition (we jump on the negated condition)
    
    assert(condition >= 0 && condition <= 0x0F);
    
    //bperr("OC: %x, NC: %x\n", condition, condition ^ 0x01);
    condition ^= 0x01; // flip last bit to negate the tttn condition
    
    gen.codeEmitter()->emitRestoreFlagsFromStackSlot(gen);
    emitJcc(condition, offset, gen);
}

// VG(07/30/02): Restore mutatee value of GPR reg to dest (real) GPR
static inline void restoreGPRtoGPR(Register reg, Register dest,
                                   codeGen &gen)
{
    // NOTE: I don't use emitLoadPreviousStackFrameRegister because it saves
    // the value to a virtual (stack based) register, which is not what I want!
    
    // mov dest, offset[ebp]
  emitMovRMToReg(dest, REGNUM_EBP, SAVED_EAX_OFFSET-(reg<<2), gen);
}

// VG(11/07/01): Load in destination the effective address given
// by the address descriptor. Used for memory access stuff.
void emitASload(const BPatch_addrSpec_NP *as, Register dest, codeGen &gen, bool /* noCost */)
{
    // TODO 16-bit registers, rep hacks
    long imm = as->getImm();
    int ra  = as->getReg(0);
    int rb  = as->getReg(1);
    int sc  = as->getScale();

    gen.codeEmitter()->emitASload(ra, rb, sc, imm, dest, gen);
}

void EmitterIA32::emitASload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen)
{
   bool havera = ra > -1, haverb = rb > -1;

   // VG(7/30/02): given that we use virtual (stack allocated) registers for
   // our inter-snippet temporaries, I assume all real registers to be fair
   // game.  So, we restore the original registers in EAX and REGNUM_EDX - this
   // allows us to generate a lea (load effective address instruction) that
   // will make the cpu do the math for us.

   // assuming 32-bit addressing (for now)

   //bperr( "ASLOAD: ra=%d rb=%d sc=%d imm=%d\n", ra, rb, sc, imm);

   if(havera)
      restoreGPRtoGPR(ra, REGNUM_EAX, gen);        // mov eax, [saved_ra]

   if(haverb)
      restoreGPRtoGPR(rb, REGNUM_EDX, gen);        // mov edx, [saved_rb]

   // Emit the lea to do the math for us:

   // e.g. lea eax, [eax + edx * sc + imm] if both ra and rb had to be
   // restored
   emitLEA((havera ? REGNUM_EAX : Null_Register), (haverb ? REGNUM_EDX : Null_Register),
           sc, (long)imm, REGNUM_EAX, gen);

   emitMovRegToRM(REGNUM_EBP, -1*(dest<<2), REGNUM_EAX, gen); // mov (virtual reg) dest, eax
}

void emitCSload(const BPatch_countSpec_NP *as, Register dest,
		codeGen &gen, bool /* noCost */ )
{
   // VG(7/30/02): different from ASload on this platform, no LEA business

   long imm = as->getImm();
   int ra  = as->getReg(0);
   int rb  = as->getReg(1);
   int sc  = as->getScale();

   gen.codeEmitter()->emitCSload(ra, rb, sc, imm, dest, gen);
}

void EmitterIA32::emitCSload(int ra, int rb, int sc, long imm, Register dest, codeGen &gen)
{
   // count is at most 1 register or constant or hack (aka pseudoregister)
   assert((ra == -1) &&
          ((rb == -1) ||
            ((imm == 0) && (rb == 1 /*REGNUM_ECX */ || rb >= IA32_EMULATE))));

   if(rb >= IA32_EMULATE) {
      // TODO: firewall code to ensure that direction is up
      bool neg = false;
      //bperr( "!!!In case rb >= IA32_EMULATE!!!\n");
      switch(rb) {
        case IA32_NESCAS:
           neg = true;
        case IA32_ESCAS:
           // plan: restore flags, edi, eax, ecx; do rep(n)e scas(b/w);
           // compute (saved_ecx - ecx) << sc;

           // mov eax, offset[ebp]
           emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, SAVED_EFLAGS_OFFSET, gen);

           emitSimpleInsn(0x50, gen);  // push eax
           emitSimpleInsn(POPFD, gen); // popfd
           restoreGPRtoGPR(REGNUM_EAX, REGNUM_EAX, gen);
           restoreGPRtoGPR(REGNUM_ECX, REGNUM_ECX, gen);
           restoreGPRtoGPR(REGNUM_EDI, REGNUM_EDI, gen);
           emitSimpleInsn(neg ? 0xF2 : 0xF3, gen); // rep(n)e
           switch(sc) {
             case 0:
                emitSimpleInsn(0xAE, gen); // scasb
                break;
             case 1:
                emitSimpleInsn(0x66, gen); // operand size override for scasw;
             case 2:
                emitSimpleInsn(0xAF, gen); // scasw/d
                break;
             default:
                assert(!"Wrong scale!");
           }
           restoreGPRtoGPR(REGNUM_ECX, REGNUM_EAX, gen); // old ecx -> eax
           emitSubRegReg(REGNUM_EAX, REGNUM_ECX, gen); // eax = eax - ecx
           if(sc > 0)
              emitSHL(REGNUM_EAX, static_cast<unsigned char>(sc), gen); // shl eax, scale

           // mov (virtual reg) dest, eax
           emitMovRegToRM(REGNUM_EBP, -1*(dest<<2), REGNUM_EAX, gen);

           break;
        case IA32_NECMPS:
           neg = true;
        case IA32_ECMPS:
           // plan: restore flags, esi, edi, ecx; do rep(n)e cmps(b/w);
           // compute (saved_ecx - ecx) << sc;

           // mov eax, offset[ebp]
           emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, SAVED_EFLAGS_OFFSET, gen);

           emitSimpleInsn(0x50, gen);  // push eax
           emitSimpleInsn(POPFD, gen); // popfd
           restoreGPRtoGPR(REGNUM_ECX, REGNUM_ECX, gen);
           restoreGPRtoGPR(REGNUM_ESI, REGNUM_ESI, gen);
           restoreGPRtoGPR(REGNUM_EDI, REGNUM_EDI, gen);
           emitSimpleInsn(neg ? 0xF2 : 0xF3, gen); // rep(n)e
           switch(sc) {
             case 0:
                emitSimpleInsn(0xA6, gen); // cmpsb
                break;
             case 1:
                emitSimpleInsn(0x66, gen); // operand size override for cmpsw;
             case 2:
                emitSimpleInsn(0xA7, gen); // cmpsw/d
                break;
             default:
                assert(!"Wrong scale!");
           }
           restoreGPRtoGPR(REGNUM_ECX, REGNUM_EAX, gen); // old ecx -> eax
           emitSubRegReg(REGNUM_EAX, REGNUM_ECX, gen); // eax = eax - ecx
           if(sc > 0)
              emitSHL(REGNUM_EAX, static_cast<unsigned char>(sc), gen); // shl eax, scale

           // mov (virtual reg) dest, eax
           emitMovRegToRM(REGNUM_EBP, -1*(dest<<2), REGNUM_EAX, gen);

           break;
        default:
           assert(!"Wrong emulation!");
      }
   }
   else if(rb > -1) {
      //bperr( "!!!In case rb > -1!!!\n");
      // TODO: 16-bit pseudoregisters
      assert(rb < 8); 
      restoreGPRtoGPR(rb, REGNUM_EAX, gen);        // mov eax, [saved_rb]
      if(sc > 0)
         emitSHL(REGNUM_EAX, static_cast<unsigned char>(sc), gen); // shl eax, scale

      // mov (virtual reg) dest, eax
      emitMovRegToRM(REGNUM_EBP, -1*(dest<<2), REGNUM_EAX, gen);
   }
   else
      emitMovImmToRM(REGNUM_EBP, -1*(dest<<2), (int)imm, gen);
}


void emitVload(opCode op, Address src1, Register src2, Register dest, 
               codeGen &gen, bool /*noCost*/, 
               registerSpace * /*rs*/, int size,
               const instPoint * /* location */, AddressSpace * /* proc */)
{
   if (op == loadConstOp) {
      // dest is a temporary
      // src1 is an immediate value 
      // dest = src1:imm32
       gen.codeEmitter()->emitLoadConst(dest, src1, gen);
      return;
   } else if (op ==  loadOp) {
      // dest is a temporary
      // src1 is the address of the operand
      // dest = [src1]
       gen.codeEmitter()->emitLoad(dest, src1, size, gen);
      return;
   } else if (op == loadFrameRelativeOp) {
      // dest is a temporary
      // src1 is the offset of the from the frame of the variable
       gen.codeEmitter()->emitLoadOrigFrameRelative(dest, src1, gen);
       return;
   } else if (op == loadRegRelativeOp) {
      // dest is a temporary
      // src2 is the register 
      // src1 is the offset from the address in src2
      gen.codeEmitter()->emitLoadOrigRegRelative(dest, src1, src2, gen, true);
      return;
   } else if (op == loadRegRelativeAddr) {
      // dest is a temporary
      // src2 is the register 
      // src1 is the offset from the address in src2
      gen.codeEmitter()->emitLoadOrigRegRelative(dest, src1, src2, gen, false);
      return;
   } else if (op == loadFrameAddr) {
       gen.codeEmitter()->emitLoadFrameAddr(dest, src1, gen);
       return;
   } else {
      abort();                // unexpected op for this emit!
   }
}

void emitVstore(opCode op, Register src1, Register src2, Address dest,
                codeGen &gen, bool /*noCost*/, registerSpace * /*rs*/, 
                int size,
                const instPoint * /* location */, AddressSpace * /* proc */)
{
   if (op ==  storeOp) {
      // [dest] = src1
      // dest has the address where src1 is to be stored
      // src1 is a temporary
      // src2 is a "scratch" register, we don't need it in this architecture
       gen.codeEmitter()->emitStore(dest, src1, size, gen);
      return;
   } else if (op == storeFrameRelativeOp) {
       // src1 is a temporary
       // src2 is a "scratch" register, we don't need it in this architecture
       // dest is the frame offset 
       gen.codeEmitter()->emitStoreFrameRelative(dest, src1, src2, size, gen);
       return;
   } else {
       abort();                // unexpected op for this emit!
   }
}

void emitV(opCode op, Register src1, Register src2, Register dest, 
           codeGen &gen, bool /*noCost*/, 
           registerSpace * /*rs*/, int /* size */,
           const instPoint * /* location */, AddressSpace * /* proc */)
{
    //bperr( "emitV(op=%d,src1=%d,src2=%d,dest=%d)\n", op, src1,
    //        src2, dest);
    
    assert ((op!=branchOp) && (op!=ifOp) &&
            (op!=trampTrailer) && (op!=trampPreamble));         // !emitA
    assert ((op!=getRetValOp) && (op!=getParamOp));             // !emitR
    assert ((op!=loadOp) && (op!=loadConstOp));                 // !emitVload
    assert ((op!=storeOp));                                     // !emitVstore
    assert ((op!=updateCostOp));                                // !emitVupdate
    
    if (op ==  loadIndirOp) {
        // same as loadOp, but the value to load is already in a register
        gen.codeEmitter()->emitLoadIndir(dest, src1, gen);
    } 
    else if (op ==  storeIndirOp) {
        // same as storeOp, but the address where to store is already in a
        // register
        gen.codeEmitter()->emitStoreIndir(dest, src1, gen);
    } else if (op == noOp) {
        emitSimpleInsn(NOP, gen); // nop
    } else if (op == saveRegOp) {
        // Push....
        assert(src2 == 0);
        assert(dest == 0);
        gen.codeEmitter()->emitPush(gen, src1);
    } else if (op == loadRegOp) {
        assert(src1 == 0);
        assert(src2 == 0);
        gen.codeEmitter()->emitPop(gen, dest);
    } else {
        unsigned opcode = 0;//initialize to placate gcc warnings
        switch (op) {
            // integer ops
        case plusOp:
            // dest = src1 + src2
            // mv eax, src1
            // add eax, src2
            // mov dest, eax
            opcode = 0x03; // ADD
            break;
            
        case minusOp:
            opcode = 0x2B; // SUB
            break;
            
        case timesOp:
            opcode = 0x0FAF; // IMUL
            break;

        case divOp: {
           // dest = src1 div src2
	   gen.codeEmitter()->emitDiv(dest, src1, src2, gen);
           return;
           break;
        }
           // Bool ops
        case orOp:
           opcode = 0x0B; // OR 
           break;

        case andOp:
           opcode = 0x23; // AND
           break;

           // rel ops
           // dest = src1 relop src2
        case eqOp:
        case neOp:
        case lessOp:
        case leOp:
        case greaterOp:
        case geOp: {
            gen.codeEmitter()->emitRelOp(op, dest, src1, src2, gen);
            return;
            break;
        }
        default:
            abort();
            break;
        }
        gen.codeEmitter()->emitOp(opcode, dest, src1, src2, gen);
    }
    return;
}

void emitImm(opCode op, Register src1, RegValue src2imm, Register dest, 
             codeGen &gen, bool, registerSpace *)
{
   if (op ==  storeOp) {
       // this doesn't seem to ever be called from ast.C (or anywhere) - gq

      // [dest] = src1
      // dest has the address where src1 is to be stored
      // src1 is an immediate value
      // src2 is a "scratch" register, we don't need it in this architecture
      emitMovImmToReg(REGNUM_EAX, dest, gen);
      emitMovImmToRM(REGNUM_EAX, 0, src1, gen);
   } else {
      unsigned opcode1;
      unsigned opcode2;
      switch (op) {
         // integer ops
        case plusOp:
	        opcode1 = 0x81;
	        opcode2 = 0x0; // ADD
           break;

        case minusOp:
           opcode1 = 0x81;
           opcode2 = 0x5; // SUB
           break;

      case timesOp: {
          gen.codeEmitter()->emitTimesImm(dest, src1, src2imm, gen);
          return;
          break;
      }
      case divOp: {
          gen.codeEmitter()->emitDivImm(dest, src1, src2imm, gen);
           return;
           break;
      }
           // Bool ops
        case orOp:
           opcode1 = 0x81;
           opcode2 = 0x1; // OR 
           break;

        case andOp:
           opcode1 = 0x81;
           opcode2 = 0x4; // AND
           break;

           // rel ops
           // dest = src1 relop src2
        case eqOp:
        case neOp:
        case lessOp:
        case leOp:
        case greaterOp:
      case geOp: {
          gen.codeEmitter()->emitRelOpImm(op, dest, src1, src2imm, gen);
          return;
          break;
      }
      default:
          abort();
          break;
      }
      gen.codeEmitter()->emitOpImm(opcode1, opcode2, dest, src1, src2imm, gen);
   }
   return;
}


// TODO: mux this between x86 and AMD64
int getInsnCost(opCode op)
{
   if (op == loadConstOp) {
      return(1);
   } else if (op ==  loadOp) {
      return(1+1);
   } else if (op ==  loadIndirOp) {
      return(3);
   } else if (op ==  storeOp) {
      return(1+1); 
   } else if (op ==  storeIndirOp) {
      return(3);
   } else if (op ==  ifOp) {
      return(1+2+1);
   } else if (op ==  ifMCOp) { // VG(8/15/02): No clue if this is right or not
      return(1+2+1);
   } else if (op ==  whileOp) {
      return(1+2+1+1); /* Need to find out about this */
   } else if (op == branchOp) {
      return(1);	/* XXX Need to find out what value this should be. */
   } else if (op ==  callOp) {
      // cost of call only
      return(1+2+1+1);
   } else if (op == funcJumpOp) {
      // copy callOp
      return(1+2+1+1);
   } else if (op == updateCostOp) {
      return(3);
   } else if (op ==  trampPreamble) {
      return(0);
   } else if (op ==  trampTrailer) {
      return(1);
   } else if (op == noOp) {
      return(1);
   } else if (op == getRetValOp) {
      return (1+1);
   } else if (op == getParamOp) {
      return(1+1);
   } else {
      switch (op) {
         // rel ops
        case eqOp:
        case neOp:
        case lessOp:
        case leOp:
        case greaterOp:
        case geOp:
	        return(1+1+2+1+1+1);
	        break;
        case divOp:
           return(1+2+46+1);
        case timesOp:
           return(1+10+1);
        case plusOp:
        case minusOp:
        case orOp:
        case andOp:
           return(1+2+1);
        case getAddrOp:
           return(0);	// doesn't add anything to operand
        default:
           assert(0);
           return 0;
           break;
      }
   }
}


dictionary_hash<std::string, unsigned> funcFrequencyTable(::Dyninst::hash);

//
// initDefaultPointFrequencyTable - define the expected call frequency of
//    procedures.  Currently we just define several one shots with a
//    frequency of one, and provide a hook to read a file with more accurate
//    information.
//
void initDefaultPointFrequencyTable()
{
    FILE *fp;
    float value;
    char name[512];

    funcFrequencyTable["main"] = 1;
    funcFrequencyTable["DYNINSTsampleValues"] = 1;
    funcFrequencyTable[EXIT_NAME] = 1;

    // try to read file.
    fp = fopen("freq.input", "r");
    if (!fp) {
        return;
    } else {
        bperr("found freq.input file\n");
    }
    while (!feof(fp)) {
        fscanf(fp, "%s %f\n", name, &value);
        funcFrequencyTable[name] = (int) value;
        bperr("adding %s %f\n", name, value);
    }
    fclose(fp);
}

/*
 * Get an estimate of the frequency for the passed instPoint.  
 *    This is not (always) the same as the function that contains the point.
 * 
 *  The function is selected as follows:
 *
 *  If the point is an entry or an exit return the function name.
 *  If the point is a call and the callee can be determined, return the called
 *     function.
 *  else return the funcation containing the point.
 *
 *  WARNING: This code contains arbitrary values for func frequency (both user 
 *     and system).  This should be refined over time.
 *
 * Using 1000 calls sec to be one SD from the mean for most FPSPEC apps.
 *	-- jkh 6/24/94
 *
 */
float getPointFrequency(instPoint *point)
{
    int_function *func = point->findCallee();
    
    if (!func) {
        // Just need a name
        func = point->func();
    }
    
	if (!funcFrequencyTable.defines(func->prettyName().c_str())) {
        // Changing this value from 250 to 100 because predictedCost was
        // too high - naim 07/18/96
        return(100.0);       
    } else {
		return ((float)funcFrequencyTable[func->prettyName().c_str()]);
    }
}

//
// return cost in cycles of executing at this point.  This is the cost
//   of the base tramp if it is the first at this point or 0 otherwise.
//
// We now have multiple versions of instrumentation... so, how does this work?
// We look for a local maximum.
int instPoint::getPointCost()
{
  unsigned worstCost = 0;
  for (unsigned i = 0; i < instances.size(); i++) {
      if (instances[i]->multi()) {
          if (instances[i]->multi()->usesTrap()) {
              // Stop right here
              // Actually, probably don't want this if the "always
              // delivered" instrumentation happens
              return 9000; // Estimated trap cost
          }
          else {
              // Base tramp cost if we're first at point, otherwise
              // free (someone else paid)
              // Which makes no sense, since we're talking an entire instPoint.
              // So if there is a multitramp hooked up we use the base tramp cost.
              worstCost = 83; // Magic constant from before time
          }
      }
      else {
          // No multiTramp, so still free (we're not instrumenting here).
      }
  }
  return worstCost;
}

unsigned baseTramp::getBTCost() {
    // Check this...
    return 83;
}

// Emit code to jump to function CALLEE without linking.  (I.e., when
// CALLEE returns, it returns to the current caller.)
void emitFuncJump(opCode op, 
                  codeGen &gen,
		  const int_function *callee, AddressSpace *,
		  const instPoint *loc, bool)
{
    // This must mimic the generateRestores baseTramp method. 
    // TODO: make this work better :)

    // TODO: what about multiple entry points? Heh.

    assert(op == funcJumpOp);

    Address addr = callee->getAddress();
    instPointType_t ptType = loc->getPointType();
    gen.codeEmitter()->emitFuncJump(addr, ptType, gen);
}

void EmitterIA32::emitFuncJump(Address addr, instPointType_t ptType, codeGen &gen)
{       
    if (ptType == otherPoint)
        emitOpRegRM(FRSTOR, FRSTOR_OP, REGNUM_EBP, -TRAMP_FRAME_SIZE - FSAVE_STATE_SIZE, gen);
    emitSimpleInsn(LEAVE, gen);     // leave
    emitSimpleInsn(POP_EAX, gen);
    emitSimpleInsn(POPAD, gen);     // popad

    gen.rs()->restoreVolatileRegisters(gen);
    //emitSimpleInsn(POPFD, gen);

    // Red zone skip - see comment in emitBTsaves
    if (STACK_PAD_CONSTANT)
        emitLEA(REGNUM_ESP, Null_Register, 0, STACK_PAD_CONSTANT, REGNUM_ESP, gen);    
    
    GET_PTR(insn, gen);
    *insn++ = 0x68; /* push 32 bit immediate */
    *((int *)insn) = addr; /* the immediate */
    insn += 4;
    *insn++ = 0xc3; /* ret */
    SET_PTR(insn, gen);

    instruction::generateIllegal(gen);
}

bool EmitterIA32::emitPush(codeGen &gen, Register r) {
    GET_PTR(insn, gen);   
    if (r >= 8) {
        fprintf(stderr, "ERROR: attempt to push register %d\n", r);
    }
    assert(r < 8);

    *insn++ = static_cast<unsigned char>(0x50 + r); // 0x50 is push EAX, and it increases from there.

    SET_PTR(insn, gen);
    return true;
}

bool EmitterIA32::emitPop(codeGen &gen, Register r) {
    GET_PTR(insn, gen);
    assert(r < 8);
    *insn++ = static_cast<unsigned char>(0x58 + r);
    
    SET_PTR(insn, gen);
    return true;
}

bool EmitterIA32::emitAdjustStackPointer(int index, codeGen &gen) {
	// The index will be positive for "needs popped" and negative
	// for "needs pushed". However, positive + SP works, so don't
	// invert.
	int popVal = index * gen.addrSpace()->getAddressWidth();
	emitOpRegImm(EXTENDED_0x81_ADD, REGNUM_ESP, popVal, gen);
	return true;
}


void emitLoadPreviousStackFrameRegister(Address register_num,
                                        Register dest,
                                        codeGen &gen,
                                        int,
                                        bool){
    gen.codeEmitter()->emitLoadOrigRegister(register_num, dest, gen);
}

void emitStorePreviousStackFrameRegister(Address register_num,
                                        Register src,
                                        codeGen &gen,
                                        int,
                                        bool) {
    gen.codeEmitter()->emitStoreOrigRegister(register_num, src, gen);
}

// First AST node: target of the call
// Second AST node: source of the call
// This can handle indirect control transfers as well 
bool AddressSpace::getDynamicCallSiteArgs(instPoint *callSite,
                                          pdvector<AstNodePtr> &args)
{
   Register base_reg, index_reg;
   int displacement;
   unsigned scale;
   int addr_mode;
   unsigned Mod;

   const instruction &i = callSite->insn();

   if (i.type() & PREFIX_SEG) {
      return false;
   }

   if(i.isCallIndir() || i.isJumpIndir()){
      addr_mode = get_instruction_operand(i.ptr(), base_reg, index_reg,
                                          displacement, scale, Mod);
      switch(addr_mode){

         // casting first to long, then void* in calls to the AstNode
         // constructor below avoids a mess of compiler warnings on AMD64
         case REGISTER_DIRECT:
         {
            args.push_back(AstNode::operandNode(AstNode::origRegister, (void *)(long)base_reg));
            break;
         }
         case REGISTER_INDIRECT:
         {
            args.push_back(AstNode::operandNode(AstNode::DataIndir,
                                                AstNode::operandNode(AstNode::origRegister, 
                                                                     (void *)(long)base_reg)));
            break;
         }
         case REGISTER_INDIRECT_DISPLACED:
         {
            args.push_back(AstNode::operandNode(AstNode::DataIndir, 
                              AstNode::operatorNode(plusOp,
                                                    AstNode::operandNode(AstNode::Constant,
                                                                         (void *)(long)displacement),
                                                    AstNode::operandNode(AstNode::origRegister,
                                                                         (void *)(long)base_reg))));
            break;
         }
         case DISPLACED:
         {
            args.push_back(AstNode::operandNode(AstNode::DataIndir,
                                                AstNode::operandNode(AstNode::Constant,
                                                                     (void *)(long) displacement)));
            break;
         }
         case SIB:
         {
            AstNodePtr effective_address;
            if(index_reg != 4) { //We use a scaled index
               bool useBaseReg = true;
               if(Mod == 0 && base_reg == 5){
                  cerr << "Inserting untested call site monitoring "
                       << "instrumentation at address " << std::hex
                       << callSite->addr() << std::dec << endl;
                  useBaseReg = false;
               }
               
               AstNodePtr index = AstNode::operandNode(AstNode::origRegister, 
                                                       (void *)(long) index_reg);
               AstNodePtr base = AstNode::operandNode(AstNode::origRegister,
                                                      (void *)(long) base_reg);
               
               AstNodePtr disp = AstNode::operandNode(AstNode::Constant,
                                                      (void *)(long) displacement);
                 
               if(scale == 1){ //No need to do the multiplication
                  if(useBaseReg){
                     effective_address = AstNode::operatorNode(plusOp,
                                                               AstNode::operatorNode(plusOp,
                                                                                     index,
                                                                                     base),
                                                               disp);
                  }
                  else
                     effective_address = AstNode::operatorNode(plusOp, index, disp);
                  
                  args.push_back(AstNode::operandNode(AstNode::DataIndir,
                                                      effective_address));
                  
               }
               else {
                  AstNodePtr scale_factor = AstNode::operandNode(AstNode::Constant, 
                                                                 (void *)(long) scale);
                  
                  AstNodePtr index_scale_product = AstNode::operatorNode(timesOp,
                                                                         index,
                                                                         scale_factor);
                  if(useBaseReg){
                     effective_address = AstNode::operatorNode(
                                                    plusOp,
                                                    AstNode::operatorNode(plusOp,
                                                                          index_scale_product,
                                                                          base),
                                                    disp);
                  }
                     else
                        effective_address = AstNode::operatorNode(plusOp,
                                                                  index_scale_product,
                                                                  disp);
                     args.push_back( AstNode::operandNode(AstNode::DataIndir,
                                                          effective_address));
                  }
               }
               else { //We do not use a scaled index.
                  args.push_back(AstNode::operandNode(AstNode::DataIndir,
                                                      AstNode::operatorNode(
                                                         plusOp,
                                                         AstNode::operandNode(
                                                                 AstNode::Constant,
                                                                 (void *)(long)displacement),
                                                         AstNode::operandNode(
                                                                 AstNode::origRegister,
                                                                 (void *)(long)base_reg))));
               }
         }
         break;
         
         default:
            cerr << "Unexpected addressing type " << addr_mode 
                 << " in MonitorCallSite at addr:"
                 << std::hex << callSite->addr() << std::dec
                 << "The daemon declines the monitoring request of"
                 << " this call site." << endl;
            break;
      }
      
      // Second AST
      args.push_back( AstNode::operandNode(AstNode::Constant,
                                           (void *) callSite->addr()));
   }
   else if(i.isCall()){ 
      //Regular callees are statically determinable, so no need to
      //instrument them
      //return true;
      fprintf(stderr, "%s[%d]:  FIXME,  dynamic call is statically determinable\n"
              "at address %p (%s)", FILE__, __LINE__, 
              (void *)callSite->addr(), callSite->func()->prettyName().c_str());
      return false; // but we generate no args.
   }
   else {
      cerr << "Unexpected instruction in MonitorCallSite()!!!\n";
   }
   return true;
}


#if defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
#include <sys/signal.h>
//#include <sys/ucontext.h>

void
BaseTrampTrapHandler (int)//, siginfo_t*, ucontext_t*)
{
   cout << "In BaseTrampTrapHandler()" << endl;
   // unset trap handler, so that DYNINSTtrapHandler can take place
   if (sigaction(SIGTRAP, NULL, NULL) != 0) {
      perror("sigaction(SIGTRAP)");
      assert(0);
      abort();
   }
}
#endif

/****************************************************************************/
/****************************************************************************/

int getMaxJumpSize()
{
  return JUMP_REL32_SZ;
}

// TODO: fix this so we don't screw future instrumentation of this
// function. It's a cute little hack, but jeez.
bool int_function::setReturnValue(int val)
{
    codeGen gen(16);

    Address addr = getAddress();
    emitMovImmToReg(REGNUM_EAX, val, gen);
    emitSimpleInsn(0xc3, gen); //ret
    
    return proc()->writeTextSpace((void *) addr, gen.used(), gen.start_ptr());
}

unsigned saveRestoreRegistersInBaseTramp(process * /*proc*/, 
                                         baseTramp * /*bt*/,
                                         registerSpace * /*rs*/)
{
  return 0;
}

/**
 * Fills in an indirect function pointer at 'addr' to point to 'f'.
 **/
bool writeFunctionPtr(AddressSpace *p, Address addr, int_function *f)
{
   Address val_to_write = f->getAddress();
   return p->writeDataSpace((void *) addr, sizeof(Address), &val_to_write);   
}

int instPoint::liveRegSize()
{
  return maxGPR;
}

bool emitStoreConst(Address addr, int imm, codeGen &gen, bool noCost) {
   gen.codeEmitter()->emitStoreImm(addr, imm, gen, noCost);
   return true;
}

bool emitAddSignedImm(Address addr, long int imm, codeGen &gen, bool noCost) {
   gen.codeEmitter()->emitAddSignedImm(addr, imm, gen, noCost);
   return true;
}

bool emitSubSignedImm(Address addr, long int imm, codeGen &gen, bool noCost) {
   gen.codeEmitter()->emitAddSignedImm(addr, imm * -1, gen, noCost);
   return true;
}

Emitter *AddressSpace::getEmitter() 
{
   static EmitterIA32Dyn emitter32Dyn;
   static EmitterIA32Stat emitter32Stat;

#if defined(arch_x86_64)
   static EmitterAMD64Dyn emitter64Dyn;
   static EmitterAMD64Stat emitter64Stat;

   if (getAddressWidth() == 8) {
       if (proc()) {
           return &emitter64Dyn;
       }
       else {
           assert(edit());
           return &emitter64Stat;
       }
   }
#endif
   if (proc()) {
       return &emitter32Dyn;
   }
   else {
       assert(edit());
       return &emitter32Stat;
   }
}

bool image::isAligned(const Address/* where*/) const 
{
   return true;
}

