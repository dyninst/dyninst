/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * inst-x86.C - x86 dependent functions and code generator
 * $Id: inst-x86.C,v 1.289 2008/09/11 20:14:14 mlam Exp $
 */
#include <iomanip>
#include <cstdint>
#include <limits.h>
#include "common/src/headers.h"
#include "compiler_annotations.h"
#include "compiler_diagnostics.h"
#include <unordered_map>
#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/codegen.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/instPoint.h" // includes instPoint-x86.h

#include "dyninstAPI/src/addressSpace.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/dynProcess.h"

#include "dyninstAPI/src/registerSpace.h"

#include "dyninstAPI/src/instP.h" // class returnInstance
#include "mapped_module.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "IAPI_to_AST.h"
#include "Expression.h"
#include "Instruction.h"
#include <sstream>
#include <assert.h>
#include "unaligned_memory_access.h"

class ExpandInstruction;
class InsertNops;

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

void registerSpace::initialize32() {
    static bool done = false;
    if (done) return;
    done = true;

    // On 32-bit x86 we use stack slots as "registers"; therefore we can
    // create an arbitrary number, and use them. However, this can bite us
    // if we want to use actual registers. Any ideas?
    
    std::vector<registerSlot *> registers;

    // When we use 
    registerSlot *eax = new registerSlot(REGNUM_EAX,
                                        "eax",
                                        false, // Off-limits due to our "stack slot" register mechanism
                                        registerSlot::liveAlways,
                                        registerSlot::realReg);
    registerSlot *ecx = new registerSlot(REGNUM_ECX,
                                        "ecx",
                                        false,
                                        registerSlot::liveAlways,
                                        registerSlot::realReg);
    registerSlot *edx = new registerSlot(REGNUM_EDX,
                                        "edx",
                                        false,
                                        registerSlot::liveAlways,
                                        registerSlot::realReg);
    registerSlot *ebx = new registerSlot(REGNUM_EBX,
                                        "ebx",
                                        false,
                                        registerSlot::liveAlways,
                                        registerSlot::realReg);
    registerSlot *esp = new registerSlot(REGNUM_ESP,
                                        "esp",
                                        true, // Off-limits...
                                        registerSlot::liveAlways,
                                        registerSlot::realReg); // I'd argue the SP is a special-purpose reg
    registerSlot *ebp = new registerSlot(REGNUM_EBP,
                                        "ebp",
                                        true,
                                        registerSlot::liveAlways,
                                        registerSlot::realReg);
    registerSlot *esi = new registerSlot(REGNUM_ESI,
                                        "esi",
                                        false,
                                        registerSlot::liveAlways,
                                        registerSlot::realReg);
    registerSlot *edi = new registerSlot(REGNUM_EDI,
                                        "edi",
                                        false,
                                        registerSlot::liveAlways,
                                        registerSlot::realReg);
    
    registers.push_back(eax);
    registers.push_back(ecx);
    registers.push_back(edx);
    registers.push_back(ebx);
    registers.push_back(esp);
    registers.push_back(ebp);
    registers.push_back(esi);
    registers.push_back(edi);

    // FPRs...

    // SPRs...
    registerSlot *gs = new registerSlot(REGNUM_GS,
            "gs",
            false,
            registerSlot::liveAlways,
            registerSlot::SPR);

    registers.push_back(gs);

    // "Virtual" registers
    for (unsigned i = 1; i <= NUM_VIRTUAL_REGISTERS; i++) {
		char buf[128];
        sprintf(buf, "virtGPR%u", i);

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
                                         registerSlot::liveAlways, // because we check this via overapproximation and not the
                                         // regular liveness algorithm, start out *dead* and set live if written
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

}

#if defined arch_x86_64
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
					  // TODO FIXME but I need it...
                                          false, // We use it implicitly _everywhere_
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
                                          registerSlot::GPR); 
    registerSlot * rbp = new registerSlot(REGNUM_RBP,
                                          "rbp",
                                          true,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
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

    std::vector<registerSlot *> registers;
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

    registers.push_back(new registerSlot(REGNUM_EFLAGS,
                                         "eflags",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));

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
                                         registerSlot::liveAlways, // because we check this via overapproximation and not the
                                         // regular liveness algorithm, start out *dead* and set live if written
                                         registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM0,
					 "MM0/ST(0)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM1,
					 "MM1/ST(1)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM2,
					 "MM2/ST(2)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM3,
					 "MM3/ST(3)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM4,
					 "MM4/ST(4)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM5,
					 "MM5/ST(5)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM6,
					 "MM6/ST(6)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM7,
					 "MM7/ST(7)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM0,
					 "XMM0",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM1,
					 "XMM1",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM2,
					 "XMM2",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM3,
					 "XMM3",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM4,
					 "XMM4",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM5,
					 "XMM5",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM6,
					 "XMM6",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM7,
					 "XMM7",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM8,
                        "XMM8",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM9,
                        "XMM9",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM10,
                        "XMM10",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM11,
                        "XMM11",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM12,
                        "XMM12",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM13,
                        "XMM13",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM14,
                        "XMM14",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM15,
                        "XMM15",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));

    registers.push_back(new registerSlot(REGNUM_FS,
                        "FS",
                        false,
                        registerSlot::liveAlways,
                        registerSlot::SPR));



    // For registers that we really just don't care about.
    registers.push_back(new registerSlot(REGNUM_IGNORED,
                                         "ignored",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));

    registerSpace::createRegisterSpace64(registers);


}
#endif

void registerSpace::initialize()
{
    static bool inited = false;
    
    if (inited) return;
    inited = true;
    if(xmmCapable())
    {
      hasXMM = true;
    }
    

    initialize32();
#if defined arch_x86_64
    initialize64();
#endif
}

/* This makes a call to the cpuid instruction, which returns an int where each bit is 
   a feature.  Bit 24 contains whether fxsave is possible, meaning that xmm registers
   are saved. */
#if defined(os_windows)
int cpuidCall() {
#ifdef _WIN64
    int result[4];
    __cpuid(result, 1);
    return result[4]; // edx
#else
    DWORD result = 0;
    // Note: mov <target> <source>, so backwards from what gnu uses
    _asm {
        push ebx
        mov eax, 1
        cpuid
        pop ebx
        mov result, edx
    }
    return result;
#endif
}
#endif

#if !defined(x86_64_unknown_linux2_4)              \
 && !(defined(os_freebsd) && defined(arch_x86_64))
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

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void emitJccR8(int condition_code, char jump_offset,
               codeGen &gen) {
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, condition_code);
    append_memory_as_byte(insn, jump_offset);
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
      append_memory_as_byte(insn, opcode);
      append_memory_as_byte(insn, offset & 0xFF);
   }
   else { // jcc near rel32
      opcode = 0x80 | (unsigned char)condition;
      append_memory_as_byte(insn, 0x0F);
      append_memory_as_byte(insn, opcode);
      append_memory_as(insn, int32_t{offset});
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

int tramp_pre_frame_size_32 = 36; //Stack space allocated by 'pushf; pusha'

int tramp_pre_frame_size_64 = 8 + 16 * 8 + AMD64_RED_ZONE; // stack space allocated by pushing flags and 16 GPRs
                                                // and skipping the 128-byte red zone

bool can_do_relocation(PCProcess *proc,
                       const std::vector<std::vector<Frame> > &stackWalks,
                       func_instance *instrumented_func)
{
   bool can_do_reloc = true;

   // for every vectors of frame, ie. thread stack walk, make sure can do
   // relocation
   Address begAddr = instrumented_func->addr();
   for (unsigned walk_itr = 0; walk_itr < stackWalks.size(); walk_itr++) {
     std::vector<func_instance *> stack_funcs =
       proc->pcsToFuncs(stackWalks[walk_itr]);
     
     // for every frame in thread stack walk
     for(unsigned i=0; i<stack_funcs.size(); i++) {
       func_instance *stack_func = stack_funcs[i];
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




#define MAX_BRANCH	(static_cast<uint32_t>(1)<<31)

Address getMaxBranch() {
  return (Address)MAX_BRANCH;
}


bool doNotOverflow(int64_t value)
{
    if (value <= INT_MAX && value >= INT_MIN) return true;
    return false;
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
void emitAddressingMode(unsigned base, RegValue disp,
                        unsigned reg_opcode, codeGen &gen)
{
   // MT linux uses ESP+4
   // we need an SIB in that case
   if (base == REGNUM_ESP) {
      emitAddressingMode(REGNUM_ESP, Null_Register, 0, disp, reg_opcode, gen);
      return;
   }
   GET_PTR(insn, gen);
   if (base == Null_Register) {
      append_memory_as_byte(insn, makeModRMbyte(0, reg_opcode, 5));
      assert(numeric_limits<int32_t>::lowest() <= disp  && disp <= numeric_limits<int32_t>::max() && "disp more than 32 bits");
      append_memory_as(insn, static_cast<int32_t>(disp));
   } else if (disp == 0 && base != REGNUM_EBP) {
      append_memory_as_byte(insn, makeModRMbyte(0, reg_opcode, base));
   } else if (disp >= -128 && disp <= 127) {
      append_memory_as_byte(insn, makeModRMbyte(1, reg_opcode, base));
      append_memory_as(insn, static_cast<int8_t>(disp));
   } else {
      append_memory_as_byte(insn, makeModRMbyte(2, reg_opcode, base));
      assert(numeric_limits<int32_t>::lowest() <= disp  && disp <= numeric_limits<int32_t>::max() && "disp more than 32 bits");
      append_memory_as(insn, static_cast<int32_t>(disp));
   }
   SET_PTR(insn, gen);
}

// VG(7/30/02): emit a fully fledged addressing mode: base+index<<scale+disp
void emitAddressingMode(unsigned base, unsigned index,
                        unsigned int scale, RegValue disp,
                        int reg_opcode, codeGen &gen)
{
   bool needSIB = (base == REGNUM_ESP) || (index != Null_Register);

   if(!needSIB) {
      emitAddressingMode(base, disp, reg_opcode, gen);
      return;
   }
   
   // This isn't true for AMD-64...
   //assert(index != REGNUM_ESP);
   
   if(index == Null_Register) {
      assert(base == REGNUM_ESP); // not necessary, but sane
      index = 4;           // (==REGNUM_ESP) which actually means no index in SIB
   }

   GET_PTR(insn, gen);
   
   if(base == Null_Register) { // we have to emit [index<<scale+disp32]
      append_memory_as_byte(insn, makeModRMbyte(0, reg_opcode, 4));
      append_memory_as_byte(insn, makeSIBbyte(scale, index, 5));
      assert(numeric_limits<int32_t>::lowest() <= disp  && disp <= numeric_limits<int32_t>::max() && "disp more than 32 bits");
      append_memory_as(insn, static_cast<int32_t>(disp));
   }
   else if(disp == 0 && base != REGNUM_EBP) { // EBP must have 0 disp8; emit [base+index<<scale]
       append_memory_as_byte(insn, makeModRMbyte(0, reg_opcode, 4));
       append_memory_as_byte(insn, makeSIBbyte(scale, index, base));
   }
   else if (disp >= -128 && disp <= 127) { // emit [base+index<<scale+disp8]
      append_memory_as_byte(insn, makeModRMbyte(1, reg_opcode, 4));
      append_memory_as_byte(insn, makeSIBbyte(scale, index, base));
      append_memory_as(insn, static_cast<int8_t>(disp));
   }
   else { // emit [base+index<<scale+disp32]
      append_memory_as_byte(insn, makeModRMbyte(2, reg_opcode, 4));
      append_memory_as_byte(insn, makeSIBbyte(scale, index, base));
      assert(numeric_limits<int32_t>::lowest() <= disp  && disp <= numeric_limits<int32_t>::max() && "disp more than 32 bits");
      append_memory_as(insn, static_cast<int32_t>(disp));
   }

   SET_PTR(insn, gen);
}


/* emit a simple one-byte instruction */
void emitSimpleInsn(unsigned op, codeGen &gen) {
    GET_PTR(insn, gen);
    append_memory_as(insn, static_cast<uint8_t>(op));
    SET_PTR(insn, gen);
}

void emitPushImm(unsigned int imm, codeGen &gen)
{
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0x68);
    append_memory_as(insn, uint32_t{imm});
    SET_PTR(insn, gen);
    if (gen.rs())
       gen.rs()->incStack(gen.addrSpace()->getAddressWidth());
}

// emit a simple register to register instruction: OP dest, src
// opcode is one or two byte
void emitOpRegReg(unsigned opcode, RealRegister dest, RealRegister src,
                  codeGen &gen)
{
    GET_PTR(insn, gen);
    if (opcode <= 0xFF)
       append_memory_as_byte(insn,opcode);
    else {
       append_memory_as_byte(insn, opcode >> 8);
       append_memory_as_byte(insn, opcode & 0xFF);
    }
    // ModRM byte define the operands: Mod = 3, Reg = dest, RM = src
    append_memory_as_byte(insn, makeModRMbyte(3, dest.reg(), src.reg()));
    SET_PTR(insn, gen);
}

void emitOpRegImm(int opcode, RealRegister dest, int imm,
                  codeGen &gen) {
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x81);
   append_memory_as_byte(insn, makeModRMbyte(3, opcode, dest.reg()));
   append_memory_as(insn, int32_t{imm});
   SET_PTR(insn, gen);
}

void emitOpSegRMReg(unsigned opcode, RealRegister dest, RealRegister, int disp, codeGen &gen)
{
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, opcode);
    append_memory_as_byte(insn, makeModRMbyte(0, dest.reg(), 4));
    append_memory_as_byte(insn, 0x25);
    append_memory_as(insn, int32_t{disp});
    SET_PTR(insn, gen);
}


// emit OP reg, r/m
void emitOpRegRM(unsigned opcode, RealRegister dest, RealRegister base,
		 int disp, codeGen &gen)
{
    GET_PTR(insn, gen);
    if (opcode <= 0xff) {
       append_memory_as_byte(insn, opcode);
    } else {
       append_memory_as_byte(insn, opcode >> 8);
       append_memory_as_byte(insn, opcode & 0xff);
    }
    SET_PTR(insn, gen);
    emitAddressingMode(base.reg(), disp, dest.reg(), gen);
}

// emit OP r/m, reg
void emitOpRMReg(unsigned opcode, RealRegister base, int disp,
                 RealRegister src, codeGen &gen) {
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, opcode);
   SET_PTR(insn, gen);
   emitAddressingMode(base.reg(), disp, src.reg(), gen);
}

// emit OP reg, imm32
void emitOpExtRegImm(int opcode, int ext, RealRegister dest, int imm,
                     codeGen &gen) 
{
  GET_PTR(insn, gen);
   append_memory_as_byte(insn, opcode);
   append_memory_as_byte(insn, makeModRMbyte(3, (char) ext, dest.reg()));
   append_memory_as(insn, int32_t{imm});
   SET_PTR(insn, gen);
}

void emitOpExtRegImm8(int opcode, char ext, RealRegister dest, unsigned char imm,
                     codeGen &gen) 
{
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, opcode);
   append_memory_as_byte(insn, makeModRMbyte(3, ext, dest.reg()));
   append_memory_as_byte(insn, imm);
   SET_PTR(insn, gen);
}

void emitOpExtReg(unsigned opcode, unsigned char ext, RealRegister reg, codeGen &gen)
{
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, opcode);
   append_memory_as_byte(insn, makeModRMbyte(3, ext, reg.reg()));
   SET_PTR(insn, gen);
}

void emitMovRegToReg(RealRegister dest, RealRegister src, codeGen &gen)
{
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x8B);
   append_memory_as_byte(insn, makeModRMbyte(3, dest.reg(), src.reg()));
   SET_PTR(insn, gen);
}

void emitOpRegRegImm(unsigned opcode, RealRegister dest, RealRegister src, unsigned imm, codeGen &gen)
{
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, opcode);
   append_memory_as_byte(insn, makeModRMbyte(3, dest.reg(), src.reg()));
   append_memory_as(insn, uint32_t{imm});
   SET_PTR(insn, gen);
}

// emit MOV reg, (reg)
void emitMovIRegToReg(RealRegister dest, RealRegister src,
                      codeGen &gen) {
    GET_PTR(insn, gen);
    append_memory_as_byte(insn, 0x8B);
    append_memory_as_byte(insn, makeModRMbyte(0, dest.reg(), src.reg()));
    SET_PTR(insn, gen);
    gen.markRegDefined(dest.reg());
}

// VG(07/30/02): Emit a lea dest, [base + index * scale + disp]; dest is a
// real GPR
void emitLEA(RealRegister base, RealRegister index, unsigned int scale,
             RegValue disp, RealRegister dest, codeGen &gen)
{
   if (dest.reg() != REGNUM_ESP)
      gen.markRegDefined(dest.reg());
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x8D);
   SET_PTR(insn, gen);
   emitAddressingMode(base.reg(), index.reg(), scale, disp, (int)dest.reg(), gen);
}

void emitLEA(RealRegister base, unsigned displacement, RealRegister dest, 
             codeGen &gen)
{
   gen.markRegDefined(dest.reg());
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x8D);
   append_memory_as_byte(insn, makeModRMbyte(2, dest.reg(), base.reg()));
   append_memory_as(insn, uint32_t{displacement});
   SET_PTR(insn, gen);
}

// emit MOV reg, (offset(%eip))
void emitMovPCRMToReg(RealRegister dest, int offset, codeGen &gen, bool deref_result)
{
    // call next instruction (relative 0x0) and pop PC (EIP) into register
   GET_PTR(insn, gen);

   if (!gen.addrSpace()->needsPIC())
   {
      Address target = gen.currAddr() + offset;
      if (deref_result) {
         emitMovMToReg(dest, target, gen);
      }
      else {
         emitMovImmToReg(dest, target, gen);
      }
      return;
   }


   int used = gen.used();
   RealRegister pc_reg(0);
   if (gen.rs()->pc_rel_offset() == -1) {

      //assert(!gen.rs()->pc_rel_use_count);
      if (gen.getPCRelUseCount() == 1) {
         //We know there's only one getPC instruction.  We won't setup
         // a stored register for the PC.  Just use dest since we're
         // about to write to it anyways.
         pc_reg = dest;
      }
      else {
         gen.rs()->pc_rel_reg = gen.rs()->allocateRegister(gen, true);
         pc_reg = gen.rs()->loadVirtualForWrite(gen.rs()->pc_rel_reg, gen);
      }
      gen.rs()->pc_rel_offset() = used + 5;
      append_memory_as_byte(insn, 0xE8);
      append_memory_as_byte(insn, 0x00);
      append_memory_as_byte(insn, 0x00);
      append_memory_as_byte(insn, 0x00);
      append_memory_as_byte(insn, 0x00);
      append_memory_as_byte(insn, 0x58 + pc_reg.reg());
      SET_PTR(insn, gen);
   }
   else {
      pc_reg = gen.rs()->loadVirtual(gen.rs()->pc_rel_reg, gen);   
   }
   gen.rs()->pc_rel_use_count++;

   offset += used - gen.rs()->pc_rel_offset();
   if (deref_result) {
      emitMovRMToReg(dest, pc_reg, offset, gen);
   }
   else {
      emitLEA(pc_reg, offset, dest, gen);
   }   

   if (gen.getPCRelUseCount() > 1 && 
       gen.rs()->pc_rel_use_count == gen.getPCRelUseCount())
   {
      //We've made the last use of getPC.  Free the register that stores the PC
      //Don't do if getPCRelUseCount() is 0, because we don't know how many uses
      // there are.
      //Don't do if getPCRelUseCount() is 1, because it was special cased above
      gen.rs()->freeRegister(gen.rs()->pc_rel_reg);
      gen.rs()->pc_rel_reg = Null_Register;
      gen.rs()->pc_rel_offset() = -1;
   }
}

// emit MOV reg, r/m
void emitMovRMToReg(RealRegister dest, RealRegister base, int disp,
                    codeGen &gen) 
{
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x8B);
   SET_PTR(insn, gen);
   emitAddressingMode(base.reg(), disp, dest.reg(), gen);
}

// emit MOV r/m, reg
void emitMovRegToRM(RealRegister base, int disp, RealRegister src,
                    codeGen &gen) 
{
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x89);
   SET_PTR(insn, gen);
   emitAddressingMode(base.reg(), disp, src.reg(), gen);
}

// emit MOV m, reg
void emitMovRegToM(int disp, RealRegister src, codeGen &gen)
{
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x89);
   SET_PTR(insn, gen);
   emitAddressingMode(Null_Register, disp, src.reg(), gen);
}

// emit MOV m, reg
void emitMovRegToMB(int disp, RealRegister src, codeGen &gen)
{
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x88);
   append_memory_as_byte(insn, makeModRMbyte(0, src.reg(), 5));
   append_memory_as(insn, int32_t{disp});
   SET_PTR(insn, gen);
}

// emit MOV m, reg
void emitMovRegToMW(int disp, RealRegister src, codeGen &gen)
{
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x66);
   append_memory_as_byte(insn, 0x88);
   append_memory_as_byte(insn, makeModRMbyte(0, src.reg(), 5));
   append_memory_as(insn, int32_t{disp});
   SET_PTR(insn, gen);
}

// emit MOV reg, m
void emitMovMToReg(RealRegister dest, int disp, codeGen &gen)
{
   gen.markRegDefined(dest.reg());
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x8B);
   SET_PTR(insn, gen);
   emitAddressingMode(Null_Register, disp, dest.reg(), gen);
}

// emit MOVSBL reg, m
void emitMovMBToReg(RealRegister dest, int disp, codeGen &gen)
{
   gen.markRegDefined(dest.reg());
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x0F);
   append_memory_as_byte(insn, 0xBE);
   SET_PTR(insn, gen);
   emitAddressingMode(Null_Register, disp, dest.reg(), gen);
}

// emit MOVSWL reg, m
void emitMovMWToReg(RealRegister dest, int disp, codeGen &gen)
{
   gen.markRegDefined(dest.reg());
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x0F);
   append_memory_as_byte(insn, 0xBF);
   SET_PTR(insn, gen);
   emitAddressingMode(Null_Register, disp, dest.reg(), gen);
}

// emit MOV reg, imm32
void emitMovImmToReg(RealRegister dest, int imm, codeGen &gen)
{
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0xB8 + dest.reg());
   append_memory_as(insn, int32_t{imm});
   SET_PTR(insn, gen);
}

// emit MOV r/m32, imm32
void emitMovImmToRM(RealRegister base, int disp, int imm,
                    codeGen &gen) {
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0xC7);
   SET_PTR(insn, gen);
   emitAddressingMode(base.reg(), disp, 0, gen);
   REGET_PTR(insn, gen);
   append_memory_as(insn, int32_t{imm});
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
    append_memory_as_byte(insn, 0xC7);

    // FIXME: To adhere strictly to the x86 and x86_64 ISAs, we specify an
    // absolute (32-bit) address by emitting a ModRM and SIB byte of the
    // following form:
    //     Mod = 00b, Reg = (doesn't matter?), R/M = 100b
    //     base = 101b, index = 100b, scale = (doesn't matter?)
    // Current forms of emitAddressingMode() do not allow for this, and so
    // we do it manually here.  emitAddressingMode() should be made more
    // robust.
    append_memory_as_byte(insn, makeModRMbyte(0, 0, 4));
    append_memory_as_byte(insn, makeSIBbyte(0, 4, 5));
    assert(maddr <= numeric_limits<uint32_t>::max() && "maddr more than 32 bits");
    append_memory_as(insn, static_cast<uint32_t>(maddr));

    append_memory_as(insn, int32_t{imm});
    SET_PTR(insn, gen);
}

// emit Add dword ptr DS:[addr], imm
void emitAddMemImm32(Address addr, int imm, codeGen &gen)
{
    GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x81);
   append_memory_as_byte(insn, 0x05);
   assert(addr <= numeric_limits<uint32_t>::max() && "addr more than 32 bits");
   append_memory_as(insn, static_cast<uint32_t>(addr));
   append_memory_as(insn, int32_t{imm});
    SET_PTR(insn, gen);
}

// emit Add reg, imm32
void emitAddRegImm32(RealRegister reg, int imm, codeGen &gen)
{
   GET_PTR(insn, gen);
   if (imm >= -128 && imm <= 127) {
      append_memory_as_byte(insn, 0x83);
      append_memory_as_byte(insn, makeModRMbyte(3, 0, reg.reg()));
      append_memory_as_byte(insn, static_cast<int8_t>(imm));
   }
   else {
      append_memory_as_byte(insn, 0x81);
      append_memory_as_byte(insn, makeModRMbyte(3, 0, reg.reg()));
      append_memory_as(insn, int32_t{imm});
   }
   SET_PTR(insn, gen);
}

// emit Sub reg, reg
void emitSubRegReg(RealRegister dest, RealRegister src, codeGen &gen)
{
   gen.markRegDefined(dest.reg());
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x2B);
   append_memory_as_byte(insn, makeModRMbyte(3, dest.reg(), src.reg()));
   SET_PTR(insn, gen);
}

unsigned char cmovOpcodeFromRelOp(unsigned op, bool s)
{
   switch (op) {
      case eqOp: return 0x44; //cmove
      case neOp: return 0x45; //cmovne
      case lessOp: 
        if (s) return 0x4c; //cmovl
        else return 0x42; //cmovb                         
      case leOp: 
        if (s) return 0x4e; //cmovle
        else return 0x46; //cmovbe        
      case greaterOp:
        if (s) return 0x4f; //cmovg
        else return 0x47; //cmova                           
      case geOp: 
        if (s) return 0x4d; //cmovge
        else return 0x43; //cmovae
     default: assert(0);
   }
   return 0x0;
}
// help function to select appropriate jcc opcode for a relOp
unsigned char jccOpcodeFromRelOp(unsigned op, bool s)
{
   switch (op) {
     case eqOp: return JNE_R8;
     case neOp: return JE_R8;
     case lessOp: 
       if (s) return JGE_R8; else return JAE_R8;
     case leOp: 
       if (s) return JG_R8; else return JA_R8;
     case greaterOp: 
       if (s) return JLE_R8; else return JBE_R8;
     case geOp: 
       if (s) return JL_R8; else return JB_R8;
     default: assert(0);
   }
   return 0x0;
}

Dyninst::Register emitFuncCall(opCode, codeGen &, std::vector<AstNodePtr> &, bool, Address) {
	assert(0);
	return 0;
}

// this function just multiplexes between the 32-bit and 64-bit versions
Dyninst::Register emitFuncCall(opCode op,
                      codeGen &gen,
                      std::vector<AstNodePtr> &operands, 
                      bool noCost,
                      func_instance *callee)
{
    Dyninst::Register reg = gen.codeEmitter()->emitCall(op, gen, operands, noCost, callee);
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
                                      func_instance *callee)
		   
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


Dyninst::Register EmitterIA32::emitCall(opCode op,
                               codeGen &gen,
                               const std::vector<AstNodePtr> &operands, 
                               bool noCost, func_instance *callee) {
    bool inInstrumentation = true;
#if 0
    if (gen.obj() &&
        dynamic_cast<replacedInstruction *>(gen.obj())) {
        // We're replacing an instruction - so don't do anything
        // that requires a base tramp.
        inInstrumentation = false;
    }
#endif

    if (op != callOp) {
      cerr << "ERROR: emitCall with op == " << op << endl;
    }
    assert(op == callOp);
    std::vector <Dyninst::Register> srcs;
    int param_size;
    std::vector<Dyninst::Register> saves;
    
    //  Sanity check for NULL address arg
    if (!callee) {
        char msg[256];
        sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out"
                "callee argument", __FILE__, __LINE__);
        showErrorCallback(80, msg);
        assert(0);
    }

   param_size = emitCallParams(gen, operands, callee, saves, noCost);

   //Dyninst::Register ret = gen.rs()->allocateRegister(gen, noCost);
   Dyninst::Register ret = REGNUM_EAX;

   emitCallInstruction(gen, callee, ret);

   emitCallCleanup(gen, callee, param_size, saves);

   if (!inInstrumentation) return Null_Register;

   // allocate a (virtual) register to store the return value
   // Virtual register

   return ret;
}

/*
 * emit code for op(src1,src2, dest)
 * ibuf is an instruction buffer where instructions are generated
 * base is the next free position on ibuf where code is to be generated
 */

codeBufIndex_t emitA(opCode op, Dyninst::Register src1, Dyninst::Register /*src2*/, long dest,
                     codeGen &gen, RegControl rc, bool /*noCost*/)
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
         retval = gen.codeEmitter()->emitIf(src1, dest, rc, gen);
         break;
      }
      case branchOp: {
         // dest is the displacement from the current value of insn
         // this will need to work for both 32-bits and 64-bits
         // (since there is no JMP rel64)
         retval = gen.getIndex();
         insnCodeGen::generateBranch(gen, dest);
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

Dyninst::Register emitR(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Register dest,
               codeGen &gen, bool noCost,
               const instPoint *location, bool /*for_multithreaded*/)
{
    //bperr("emitR(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

   bool get_addr_of = (src2 != Null_Register);
   switch (op) {
       case getRetValOp:
          // dest is a register where we can store the value
          // the return value is in the saved EAX
          gen.codeEmitter()->emitGetRetVal(dest, get_addr_of, gen);
          if (!get_addr_of)
             return dest;
          break;
       case getRetAddrOp: 
          // dest is a register where we can store the return address
          gen.codeEmitter()->emitGetRetAddr(dest, gen);
          return dest;
          break;
       case getParamOp:
       case getParamAtCallOp:
       case getParamAtEntryOp:
          // src1 is the number of the argument
          // dest is a register where we can store the value
          gen.codeEmitter()->emitGetParam(dest, src1, location->type(), op,
                                          get_addr_of, gen);
          if (!get_addr_of)
             return dest;
          break;
       case loadRegOp:
          assert(src1 == 0);
          assert(0);
          return dest;
       default:
          abort();                  // unexpected op for this emit!
    }
    assert(get_addr_of);
    emitV(storeIndirOp, src2, 0, dest, gen, noCost, gen.rs(), 
          gen.addrSpace()->getAddressWidth(), gen.point(), gen.addrSpace());
    return(dest);
}

void emitSHL(RealRegister dest, unsigned char pos, codeGen &gen)
{
  //bperr( "Emiting SHL\n");
   gen.markRegDefined(dest.reg());
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0xC1);
   append_memory_as_byte(insn, makeModRMbyte(3 /* rm gives register */,
                           4 /* opcode ext. */, dest.reg()));
   append_memory_as_byte(insn, pos);
   SET_PTR(insn, gen);
}

void EmitterIA32::emitPushFlags(codeGen &gen) {
    // These crank the saves forward
    emitSimpleInsn(PUSHFD, gen);
}

void EmitterIA32::emitRestoreFlags(codeGen &, unsigned )
{
    assert(!"never use this!");
    return;
//   emitOpRMReg(PUSH_RM_OPC1, RealRegister(REGNUM_ESP), offset*4, RealRegister(PUSH_RM_OPC2), gen);
//    emitSimpleInsn(POPFD, gen); // popfd
}

void EmitterIA32::emitRestoreFlagsFromStackSlot(codeGen &gen)
{
    // if the flags aren't on the stack, they're already restored...
    if((*gen.rs())[IA32_FLAG_VIRTUAL_REGISTER]->liveState == registerSlot::spilled)
    {
        stackItemLocation loc = getHeightOf(stackItem(RealRegister(IA32_FLAG_VIRTUAL_REGISTER)), gen);
        assert(loc.offset % 4 == 0);
        ::emitPush(RealRegister(REGNUM_EAX), gen);
        emitMovRMToReg(RealRegister(REGNUM_EAX), loc.reg, loc.offset, gen);
        emitRestoreO(gen);
        emitSimpleInsn(0x9E, gen); // SAHF
        ::emitPop(RealRegister(REGNUM_EAX), gen);
    }
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

stackItemLocation getHeightOf(stackItem sitem, codeGen &gen)
{
   int offset = 0;
   RealRegister reg;

   int addr_width = gen.addrSpace()->getAddressWidth();

   // Suppress warning (for compilers where it is a false positive)
   // The value of REGNUM_EBP and REGNUM_RBP are identical
   DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_DUPLICATED_BRANCHES

   RealRegister plat_bp(addr_width == 4 ? REGNUM_EBP : REGNUM_RBP);
   RealRegister plat_sp(addr_width == 4 ? REGNUM_ESP : REGNUM_RSP); 
 
   DYNINST_DIAGNOSTIC_END_SUPPRESS_DUPLICATED_BRANCHES

   if (sitem.item == stackItem::reg_item && sitem.reg.reg() == plat_sp.reg())
   {
      sitem.item = stackItem::stacktop;
   }

   switch (sitem.item)
   {
      case stackItem::reg_item:
      {
         registerSlot *r = NULL;
         std::vector<registerSlot *> &regs = gen.rs()->trampRegs();
         for (unsigned i=0; i<regs.size(); i++) {
            if (regs[i]->number == (unsigned) sitem.reg.reg()) {
               r = regs[i];
               break;
            }
         }
         if((unsigned)sitem.reg.reg() == IA32_FLAG_VIRTUAL_REGISTER)
         {
             r = (*gen.rs())[sitem.reg.reg()];
         }
         if (!r && addr_width == 8) {
            r = (*gen.rs())[sitem.reg.reg()];
         }
         assert(r);
         offset = r->saveOffset * addr_width;
         if (!gen.bt() || gen.bt()->createdFrame) {
            reg = plat_bp;
            return stackItemLocation(plat_bp, offset);
         }
         
         offset += gen.rs()->getStackHeight();
         return stackItemLocation(plat_sp, offset);
      }

      // NOTE: We can no longer return a direct offset for the top of our
      // instrumentation stack.  The stack pointer was forcibly aligned,
      // which created a variable sized padding hole.
      //
      // Instead, we'll return where in memory we stored the original
      // stack pointer.
      case stackItem::stacktop:
      {
         offset = gen.rs()->getInstFrameSize();
         if (!gen.bt() || gen.bt()->createdFrame) {
            return stackItemLocation(plat_bp, offset);
         }

         offset += gen.rs()->getStackHeight();
         return stackItemLocation(plat_sp, offset);
      }
      case stackItem::framebase: {
         if (!gen.bt() || gen.bt()->createdFrame) {
            return stackItemLocation(plat_bp, 0);
         }
         offset = gen.rs()->getStackHeight();
         return stackItemLocation(plat_sp, offset);
      }
   }
   assert(0);
   return stackItemLocation(RealRegister(Null_Register), 0);
}

// Restore mutatee value of GPR reg to dest (real) GPR
Dyninst::Register restoreGPRtoReg(RealRegister reg, codeGen &gen, RealRegister *dest_to_use)
{
   Dyninst::Register dest = Null_Register;
   RealRegister dest_r(-1);
   if (dest_to_use) {
      dest_r = *dest_to_use;
   }
   else {
      if (gen.inInstrumentation()) {
         dest = gen.rs()->getScratchRegister(gen);
      }
      else {
         dest = gen.rs()->getScratchRegister(gen, false, true);
      }
   }
   
   if (reg.reg() == REGNUM_EBP) {
      //Special handling for EBP with and without instr stack frame
      if (dest_r.reg() == -1)
         dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
      if (gen.bt() && gen.bt()->createdFrame) {
         emitMovRMToReg(dest_r, RealRegister(REGNUM_EBP), 0, gen);
      }
      else {
         if (reg.reg() != dest_r.reg()) {
            //TODO: Future optimization here, allow ebp to be used
            // though not allocated
            emitMovRegToReg(dest_r, reg, gen);
         }
      }
      return dest;
   }

   if (reg.reg() == REGNUM_ESP) {
      //Special handling for ESP 
      if (dest_r.reg() == -1)
          dest_r = gen.rs()->loadVirtualForWrite(dest, gen);

      stackItemLocation loc = getHeightOf(stackItem::stacktop, gen);
      if (!gen.bt() || gen.bt()->alignedStack) {
          emitMovRMToReg(dest_r, loc.reg, loc.offset, gen);
      } else {
          emitLEA(loc.reg, RealRegister(Null_Register), 0,
                  loc.offset, dest_r, gen);
      }
      return dest;
   }

   registerSlot *r = gen.rs()->trampRegs()[reg.reg()];
   if (r->spilledState == registerSlot::unspilled ||
       !gen.isRegDefined(reg.reg())) 
   {
      //Dyninst::Register is still in its pristine state from app, leave it.
      if (dest_r.reg() == -1)
	      gen.rs()->noteVirtualInReal(dest, reg);
      else if (dest_r.reg() != reg.reg())
         emitMovRegToReg(dest_r, reg, gen);
      return dest;
   }

   //Load register from its saved location
   stackItemLocation loc = getHeightOf(stackItem(reg), gen);
   if(dest_r.reg() == -1)
   {
       dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
   }
   emitMovRMToReg(dest_r, loc.reg, loc.offset, gen);
   return dest;
}

void restoreGPRtoGPR(RealRegister src, RealRegister dest, codeGen &gen)
{
   restoreGPRtoReg(src, gen, &dest);
}

// VG(11/07/01): Load in destination the effective address given
// by the address descriptor. Used for memory access stuff.
void emitASload(const BPatch_addrSpec_NP *as, Dyninst::Register dest, int stackShift, codeGen &gen, bool /* noCost */)
{
    // TODO 16-bit registers, rep hacks
    long imm = as->getImm();
    int ra  = as->getReg(0);
    int rb  = as->getReg(1);
    int sc  = as->getScale();

    gen.codeEmitter()->emitASload(ra, rb, sc, imm, dest, stackShift, gen);
}

void EmitterIA32::emitASload(int ra, int rb, int sc, long imm, Dyninst::Register dest, int stackOffset, codeGen &gen)
{
    bool havera = ra > -1, haverb = rb > -1;
   
   // assuming 32-bit addressing (for now)
   
   if (ra == REGNUM_ESP && !haverb && sc == 0 && gen.bt()) {
      //Optimization, common for push/pop
      RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
      stackItemLocation loc = getHeightOf(stackItem::stacktop, gen);
      if (!gen.bt() || gen.bt()->alignedStack) {
          emitMovRMToReg(dest_r, loc.reg, loc.offset, gen);
          if (imm) ::emitLEA(dest_r, RealRegister(Null_Register), 0, imm, dest_r, gen);
      }
      else
          ::emitLEA(loc.reg, RealRegister(Null_Register), 0,
                  loc.offset, dest_r, gen);
      return;
   }

   RealRegister src1_r(-1);
   Dyninst::Register src1 = Null_Register;
   if (havera) {
      if (gen.inInstrumentation()) {
       src1 = restoreGPRtoReg(RealRegister(ra), gen);
       src1_r = gen.rs()->loadVirtual(src1, gen);
      gen.rs()->markKeptRegister(src1);
     }
     else {
       // Don't have a base tramp - use only reals
       src1_r = RealRegister(ra);
	   // If this is a stack pointer, modify imm to compensate
	   // for any changes in the stack pointer
	   if (ra == REGNUM_ESP) {
		   imm -= stackOffset;
	   }
     }
   }

   RealRegister src2_r(-1);
   Dyninst::Register src2 = Null_Register;
   if (haverb) {
      if (ra == rb) {
         src2_r = src1_r;
      }
      else if (gen.inInstrumentation()) {
	src2 = restoreGPRtoReg(RealRegister(rb), gen);
	src2_r = gen.rs()->loadVirtual(src2, gen);
	gen.rs()->markKeptRegister(src2);
      }
      else {
		  src2_r = RealRegister(rb);
	      // If this is a stack pointer, modify imm to compensate
	      // for any changes in the stack pointer
	      if (rb == REGNUM_ESP) {
			  imm -= (stackOffset*sc);
		  }
      }
   }
   
   if (havera && !haverb && !sc && !imm) {
      //Optimized case, just use the existing src1_r
      if (gen.inInstrumentation()) {
       gen.rs()->unKeepRegister(src1);
       gen.rs()->freeRegister(src1);
       gen.rs()->noteVirtualInReal(dest, src1_r);
       return;
     }
     else {
       // No base tramp, no virtual registers - emit a move?
       emitMovRegToReg(RealRegister(dest), src1_r, gen);
       return;
     }
   }

   // Emit the lea to do the math for us:
   // e.g. lea eax, [eax + edx * sc + imm] if both ra and rb had to be
   // restored
   RealRegister dest_r; 
   if (gen.inInstrumentation()) {
     dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
   }
   else {
     dest_r = RealRegister(dest);
   }
   ::emitLEA(src1_r, src2_r, sc, (long) imm, dest_r, gen);

   if (src1 != Null_Register) {
       gen.rs()->unKeepRegister(src1);
       gen.rs()->freeRegister(src1);
   }
   if (src2 != Null_Register) {
       gen.rs()->unKeepRegister(src2);
       gen.rs()->freeRegister(src2);
   }
}

void emitCSload(const BPatch_countSpec_NP *as, Dyninst::Register dest,
		codeGen &gen, bool /* noCost */ )
{
   // VG(7/30/02): different from ASload on this platform, no LEA business

   long imm = as->getImm();
   int ra  = as->getReg(0);
   int rb  = as->getReg(1);
   int sc  = as->getScale();

   gen.codeEmitter()->emitCSload(ra, rb, sc, imm, dest, gen);
}

void EmitterIA32::emitCSload(int ra, int rb, int sc, long imm, Dyninst::Register dest, codeGen &gen)
{
   // count is at most 1 register or constant or hack (aka pseudoregister)
   assert((ra == -1) &&
          ((rb == -1) ||
            ((imm == 0) && (rb == 1 /*REGNUM_ECX */ || rb >= IA32_EMULATE))));

   if(rb >= IA32_EMULATE) {
      bool neg = false;
      //bperr( "!!!In case rb >= IA32_EMULATE!!!\n");
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
           restoreGPRtoGPR(RealRegister(REGNUM_ECX), RealRegister(REGNUM_EAX), gen); // old ecx -> eax
           emitSubRegReg(RealRegister(REGNUM_EAX), RealRegister(REGNUM_ECX), gen); // eax = eax - ecx
           gen.markRegDefined(REGNUM_EAX);
           if(sc > 0)
              emitSHL(RealRegister(REGNUM_EAX), static_cast<unsigned char>(sc), gen); // shl eax, scale
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
           restoreGPRtoGPR(RealRegister(REGNUM_ECX), RealRegister(REGNUM_EAX), gen); // old ecx -> eax
           emitSubRegReg(RealRegister(REGNUM_EAX), RealRegister(REGNUM_ECX), gen); // eax = eax - ecx
           if(sc > 0)
              emitSHL(RealRegister(REGNUM_EAX), static_cast<unsigned char>(sc), gen); // shl eax, scale
           RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
           emitMovRegToReg(dest_r, RealRegister(REGNUM_EAX), gen);

           break;
        }
        default:
           assert(!"Wrong emulation!");
      }
   }
   else if(rb > -1) {
      //bperr( "!!!In case rb > -1!!!\n");
      // TODO: 16-bit pseudoregisters
      assert(rb < 8); 
      RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
      restoreGPRtoGPR(RealRegister(rb), dest_r, gen); // mov dest, [saved_rb]
      if(sc > 0)
         emitSHL(dest_r, static_cast<unsigned char>(sc), gen); // shl eax, scale
   }
   else {
      RealRegister dest_r = gen.rs()->loadVirtualForWrite(dest, gen);
      emitMovImmToReg(dest_r, imm, gen);
   }
}

void emitVload(opCode op, Address src1, Dyninst::Register src2, Dyninst::Register dest,
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

void emitVstore(opCode op, Dyninst::Register src1, Dyninst::Register src2, Address dest,
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

void emitV(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Register dest,
           codeGen &gen, bool /*noCost*/, 
           registerSpace * /*rs*/, int size,
           const instPoint * /* location */, AddressSpace * /* proc */, bool s)
{
    //bperr( "emitV(op=%d,src1=%d,src2=%d,dest=%d)\n", op, src1,
    //        src2, dest);
    
    assert ((op!=branchOp) && (op!=ifOp) &&
            (op!=trampPreamble));         // !emitA
    assert ((op!=getRetValOp) && (op!=getRetAddrOp) && 
            (op!=getParamOp));                                  // !emitR
    assert ((op!=loadOp) && (op!=loadConstOp));                 // !emitVload
    assert ((op!=storeOp));                                     // !emitVstore
    assert ((op!=updateCostOp));                                // !emitVupdate
    
    if (op ==  loadIndirOp) {
        // same as loadOp, but the value to load is already in a register
       gen.codeEmitter()->emitLoadIndir(dest, src1, size, gen);
    } 
    else if (op ==  storeIndirOp) {
        // same as storeOp, but the address where to store is already in a
        // register
       gen.codeEmitter()->emitStoreIndir(dest, src1, size, gen);
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
            
        case xorOp:
            opcode = 0x33; // XOR
            break;
            
        case timesOp:
            if (s)
                opcode = 0x0FAF; // IMUL
            else
                opcode = 0x0F74; // Unsigned Multiply
            break;
        case divOp: {
           gen.codeEmitter()->emitDiv(dest, src1, src2, gen, s);
           return;
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
            gen.codeEmitter()->emitRelOp(op, dest, src1, src2, gen, s);
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

void emitImm(opCode op, Dyninst::Register src1, RegValue src2imm, Dyninst::Register dest,
             codeGen &gen, bool, registerSpace *, bool s)
{
   if (op ==  storeOp) {
       // this doesn't seem to ever be called from ast.C (or anywhere) - gq

      // [dest] = src1
      // dest has the address where src1 is to be stored
      // src1 is an immediate value
      // src2 is a "scratch" register, we don't need it in this architecture
      emitMovImmToReg(RealRegister(REGNUM_EAX), dest, gen);
      emitMovImmToRM(RealRegister(REGNUM_EAX), 0, src1, gen);
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
         case xorOp:
            opcode1 = 0x81;
            opcode2 = 0x6; // XOR
            break;            
         case timesOp:
            gen.codeEmitter()->emitTimesImm(dest, src1, src2imm, gen);
            return;
         case divOp:
            gen.codeEmitter()->emitDivImm(dest, src1, src2imm, gen, s);
            return;
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
         case geOp:
            gen.codeEmitter()->emitRelOpImm(op, dest, src1, src2imm, gen, s);
            return;
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
   } else if (op == noOp) {
      return(1);
   } else if (op == getRetValOp) {
      return (1+1);
   } else if (op == getRetAddrOp) { 
      return (1); 
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
        case xorOp:
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
   return 0;
}

bool EmitterIA32::emitPush(codeGen &gen, Dyninst::Register reg) {
    RealRegister real_reg = gen.rs()->loadVirtual(reg, gen);
    return ::emitPush(real_reg, gen);
}

bool EmitterIA32::emitPop(codeGen &gen, Dyninst::Register reg) {
    RealRegister real_reg = gen.rs()->loadVirtual(reg, gen);
    return ::emitPop(real_reg, gen);
}

bool emitPush(RealRegister reg, codeGen &gen) {
    GET_PTR(insn, gen);
    int r = reg.reg();
    assert(r < 8);

    append_memory_as_byte(insn, 0x50 + r); // 0x50 is push EAX, and it increases from there.

    SET_PTR(insn, gen);
    if (gen.inInstrumentation()) {
       gen.rs()->incStack(4);
    }
    return true;
}

bool emitPop(RealRegister reg, codeGen &gen) {
    GET_PTR(insn, gen);
    int r = reg.reg();
    assert(r < 8);
    append_memory_as_byte(insn, 0x58 + r);
    SET_PTR(insn, gen);
    if (gen.inInstrumentation()) {
       gen.rs()->incStack(-4);
    }
    return true;
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


void emitLoadPreviousStackFrameRegister(Address register_num,
                                        Dyninst::Register dest,
                                        codeGen &gen,
                                        int,
                                        bool){
    gen.codeEmitter()->emitLoadOrigRegister(register_num, dest, gen);
}

void emitStorePreviousStackFrameRegister(Address register_num,
                                        Dyninst::Register src,
                                        codeGen &gen,
                                        int,
                                        bool) {
    gen.codeEmitter()->emitStoreOrigRegister(register_num, src, gen);
}

// First AST node: target of the call
// Second AST node: source of the call
// This can handle indirect control transfers as well 
bool AddressSpace::getDynamicCallSiteArgs(InstructionAPI::Instruction insn,
                                          Address addr,
                                          std::vector<AstNodePtr> &args)
{
   using namespace Dyninst::InstructionAPI;        
   Expression::Ptr cft = insn.getControlFlowTarget();
   ASTFactory f;
   cft->apply(&f);
   assert(f.m_stack.size() == 1);
   args.push_back(f.m_stack[0]);
   args.push_back(AstNode::operandNode(AstNode::operandType::Constant,
                                       (void *) addr));
   inst_printf("%s[%d]:  Inserting dynamic call site instrumentation for %s\n",
               FILE__, __LINE__, cft->format(insn.getArch()).c_str());
   return true;
}

/****************************************************************************/
/****************************************************************************/

int getMaxJumpSize()
{
  return JUMP_REL32_SZ;
}

// TODO: fix this so we don't screw future instrumentation of this
// function. It's a cute little hack, but jeez.
bool func_instance::setReturnValue(int val)
{
    codeGen gen(16);

    emitMovImmToReg(RealRegister(REGNUM_EAX), val, gen);
    emitSimpleInsn(0xc3, gen); //ret
    
    return proc()->writeTextSpace((void *) addr(), gen.used(), gen.start_ptr());
}

unsigned saveRestoreRegistersInBaseTramp(AddressSpace * /*proc*/, 
                                         baseTramp * /*bt*/,
                                         registerSpace * /*rs*/)
{
  return 0;
}

/**
 * Fills in an indirect function pointer at 'addr' to point to 'f'.
 **/
bool writeFunctionPtr(AddressSpace *p, Address addr, func_instance *f)
{
   Address val_to_write = f->addr();
   return p->writeDataSpace((void *) addr, sizeof(Address), &val_to_write);   
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

#if defined(arch_x86_64)
int registerSpace::framePointer() { 
   // Suppress warning (for compilers where it is a false positive)
   // The value of REGNUM_EBP and REGNUM_RBP are identical
   DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_DUPLICATED_BRANCHES

   return addr_width == 8 ? REGNUM_RBP : REGNUM_EBP; 

   DYNINST_DIAGNOSTIC_END_SUPPRESS_DUPLICATED_BRANCHES
}
#elif defined(arch_x86)
int registerSpace::framePointer() { 
   return REGNUM_EBP; 
}
#endif

void registerSpace::initRealRegSpace()
{
   for (unsigned i=0; i<regStateStack.size(); i++) {
      if (regStateStack[i])
         delete regStateStack[i];
   }
   regStateStack.clear();

   regState_t *new_regState = new regState_t();
   regStateStack.push_back(new_regState);
   regs_been_spilled.clear();
   
   pc_rel_reg = Null_Register;
   pc_rel_use_count = 0;
}

void registerSpace::movRegToReg(RealRegister dest, RealRegister src, codeGen &gen)
{
   gen.markRegDefined(dest.reg());
   emitMovRegToReg(dest, src, gen);
}

void registerSpace::spillToVReg(RealRegister reg, registerSlot *v_reg, codeGen &gen)
{
   stackItemLocation loc = getHeightOf(stackItem::framebase, gen);
   loc.offset += -4*v_reg->encoding();
   emitMovRegToRM(loc.reg, loc.offset, reg, gen);
}

void registerSpace::movVRegToReal(registerSlot *v_reg, RealRegister r, codeGen &gen)
{
   stackItemLocation loc = getHeightOf(stackItem::framebase, gen);
   loc.offset += -4*v_reg->encoding();
   gen.markRegDefined(r.reg());
   emitMovRMToReg(r, loc.reg, loc.offset, gen);
}

void emitBTRegRestores32(baseTramp *bt, codeGen &gen)
{
   int numRegsUsed = bt ? bt->numDefinedRegs() : -1;
   if (numRegsUsed == -1 || 
       numRegsUsed > X86_REGS_SAVE_LIMIT) {
      emitSimpleInsn(POPAD, gen);
   }
   else {
      registerSlot *reg;
      std::vector<registerSlot *> &regs = gen.rs()->trampRegs();
      for (int i=regs.size()-1; i>=0; i--) {
         reg = regs[i];          
         if (reg->encoding() != REGNUM_ESP && 
             reg->encoding() != REGNUM_EBP &&
             reg->spilledState != registerSlot::unspilled)
         {
            emitPop(RealRegister(reg->encoding()), gen);
         }
      }
   }
   
   gen.rs()->restoreVolatileRegisters(gen);
}

regState_t::regState_t() : 
   pc_rel_offset(-1), 
   timeline(0), 
   stack_height(0) 
{
   for (unsigned i=0; i<8; i++) {
      RealRegsState r;
      r.is_allocatable = (i != REGNUM_ESP && i != REGNUM_EBP);
      r.been_used = false;
      r.last_used = 0;
      r.contains = NULL;
      registerStates.push_back(r);
   }
}

void emitSaveO(codeGen &gen)
{
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x0f);
   append_memory_as_byte(insn, 0x90);
   append_memory_as_byte(insn, 0xC0);
   SET_PTR(insn, gen);
}

void emitRestoreO(codeGen &gen)
{
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0x80);
   append_memory_as_byte(insn, 0xC0);
   append_memory_as_byte(insn, 0x7f);
   SET_PTR(insn, gen);
}

void emitCallRel32(unsigned disp32, codeGen &gen)
{
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0xE8);
   append_memory_as(insn, uint32_t{disp32});
   SET_PTR(insn, gen);
}

void emitJump(unsigned disp32, codeGen &gen)
{
   GET_PTR(insn, gen);
   append_memory_as_byte(insn, 0xE9);
   append_memory_as(insn, uint32_t{disp32});
   SET_PTR(insn, gen);
}

#if defined(os_linux)   \
 || defined(os_freebsd)

// These functions were factored from linux-x86.C because
// they are identical on Linux and FreeBSD

int EmitterIA32::emitCallParams(codeGen &gen, 
                              const std::vector<AstNodePtr> &operands,
                              func_instance */*target*/, 
                              std::vector<Dyninst::Register> &/*extra_saves*/,
                              bool noCost)
{
    std::vector <Dyninst::Register> srcs;
    unsigned frame_size = 0;
    unsigned u;
    for (u = 0; u < operands.size(); u++) {
        Address unused = ADDR_NULL;
        Dyninst::Register reg = Null_Register;
        if (!operands[u]->generateCode_phase2(gen,
                                              noCost,
                                              unused,
                                              reg)) assert(0); // ARGH....
        assert (reg != Null_Register); // Give me a real return path!
        srcs.push_back(reg);
    }
    
    // push arguments in reverse order, last argument first
    // must use int instead of unsigned to avoid nasty underflow problem:
    for (int i=srcs.size() - 1; i >= 0; i--) {
       RealRegister r = gen.rs()->loadVirtual(srcs[i], gen);
       ::emitPush(r, gen);
       frame_size += 4;
       if (operands[i]->decRefCount())
          gen.rs()->freeRegister(srcs[i]);
    }
    return frame_size;
}

bool EmitterIA32::emitCallCleanup(codeGen &gen,
                                func_instance * /*target*/, 
                                int frame_size, 
                                std::vector<Dyninst::Register> &/*extra_saves*/)
{
   if (frame_size)
      emitOpRegImm(0, RealRegister(REGNUM_ESP), frame_size, gen); // add esp, frame_size
   gen.rs()->incStack(-1 * frame_size);
   return true;
}
#endif
