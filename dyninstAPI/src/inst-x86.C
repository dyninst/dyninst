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
 * $Id: inst-x86.C,v 1.213 2005/08/11 21:20:14 bernat Exp $
 */
#include <iomanip>

#include <limits.h>
#include "common/h/headers.h"

#ifndef BPATCH_LIBRARY
#include "rtinst/h/rtinst.h"
#endif
#include "common/h/Dictionary.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/dyn_lwp.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/showerror.h"

#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/instPoint.h" // includes instPoint-x86.h

#include "dyninstAPI/src/instP.h" // class returnInstance
#include "dyninstAPI/src/rpcMgr.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "InstrucIter.h"

#include <sstream>

class ExpandInstruction;
class InsertNops;

extern bool relocateFunction(process *proc, instPoint *&location);

extern bool isPowerOf2(int value, int &result);
void BaseTrampTrapHandler(int); //siginfo_t*, ucontext_t*);

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/



/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

/* A quick model for the "we're done, branch back/ILL" tramp end */

unsigned trampEnd::maxSizeRequired() {
    // +2 for illegal insn
    return JUMP_REL32_SZ + 2;
}

/*
 * Worst-case scenario for how much room it will take to relocate
 * an instruction -- used for allocating the new area
 */
unsigned relocatedInstruction::maxSizeRequired() {

    // List of instructions that might be painful:
    // jumps (displacement will change)
    // call (displacement will change)
    // PC-relative ops

    // TODO: pc-relative ops

   const unsigned char *origInsn = insn.ptr();

   unsigned maxSize = 0;

   if (insn.type() & REL_B) {
       /* replace with rel32 instruction, opcode is one byte. */
       if (*origInsn == JCXZ) {
           // Replace with:
           // jcxz 2    2 bytes
           //  jmp 5    2 bytes
           // jmp rel32 5 bytes
           
           return 9;
      }
      else {
          if (insn.type() & IS_JCC) {
              /* Change a Jcc rel8 to Jcc rel32.  Must generate a new opcode: a
                 0x0F followed by (old opcode + 16) */
              return 6;
          }
          else if (insn.type() & IS_JUMP) {
              /* change opcode to 0xE9 */
              return 5;
          }
          assert(0);
      }
   }
   else if (insn.type() & REL_W) {
      /* Skip prefixes */
      if (insn.type() & PREFIX_OPR)
          maxSize++;
      if (insn.type() & PREFIX_SEG)
          maxSize++;
      /* opcode is unchanged, just relocate the displacement */
      if (*origInsn == (unsigned char)0x0F)
          maxSize++;

      // opcode, 32-bit displacement
      maxSize += 5;

      return maxSize;
   } else if (insn.type() & REL_D) {
       if (insn.type() & PREFIX_OPR)
           maxSize++;
       if (insn.type() & PREFIX_SEG)
           maxSize++;
       
       /* opcode is unchanged, just relocate the displacement */
       if (*origInsn == 0x0F)
           maxSize++;
       
       maxSize += 5;
       return maxSize;
   }
   else {
       return insn.size();
   }
   assert(0);
   return 0;
}

/*
 * Clear the vector
 */

bool relocatedInstruction::generateCode(codeGen &gen,
                                        Address baseInMutatee) {
    if (alreadyGenerated(gen, baseInMutatee))
        return true;
    generateSetup(gen, baseInMutatee);

    // We grab the maximum space we might need
    GET_PTR(insnBuf, gen);
    size_ = 0;
    
    /* 
       Relative address instructions need to be modified. The relative address
       can be a 8, 16, or 32-byte displacement relative to the next instruction.
       Since we are relocating the instruction to a different area, we have
       to replace 8 and 16-byte displacements with 32-byte displacements.
       
       All relative address instructions are one or two-byte opcode followed
       by a displacement relative to the next instruction:
       
       CALL rel16 / CALL rel32
       Jcc rel8 / Jcc rel16 / Jcc rel32
       JMP rel8 / JMP rel16 / JMP rel32
       
       The only two-byte opcode instructions are the Jcc rel16/rel32,
       all others have one byte opcode.
       
       The instruction JCXZ/JECXZ rel8 does not have an equivalent with rel32
       displacement. We must generate code to emulate this instruction:
       
       JCXZ rel8
       
       becomes
       
       A0: JCXZ 2 (jump to A4)
       A2: JMP 5  (jump to A9)
       A4: JMP rel32 (relocated displacement)
       A9: ...
       
   */

    const unsigned char *origInsn = insn.ptr();
    unsigned insnType = insn.type();
    unsigned insnSz = insn.size();
    // This moves as we emit code
    unsigned char *newInsn = insnBuf;

    int oldDisp;
    int newDisp;

    bool done = false;
    
    // Check to see if we're doing the "get my PC" via a call
    // We do this first as those aren't "real" jumps.
    if (insn.isCall() && !insn.isCallIndir()) {
        // A possibility...
        // Two types: call(0) (AKA call(me+5));
        // or call to a move/return combo.

        // First, call(me)
        Address target = insn.getTarget(origAddr);
        inst_printf("Target of insn at 0x%x: 0x%x, insn size %d\n",
                    origAddr, target, insn.size());
        // Big if tree: "if we go to the next insn"
        // Also could do with an instrucIter... or even have
        // parsing label it for us. 
        // TODO: label in parsing (once)
        
        if (target == (origAddr + insn.size())) {
            inst_printf("Found get EIP call\n");

            *newInsn = 0x68; // What opcode is this?
            newInsn++;

            Address EIP = origAddr + insn.size();
            inst_printf("Adding immediate load of 0x%x\n", EIP);
            unsigned int *temp = (unsigned int *) newInsn;
            *temp = EIP;
            // No 9-byte jumps...
            assert(sizeof(unsigned int *) == 4);
            newInsn += sizeof(unsigned int *);
            assert((newInsn - insnBuf) == 5);
            done = true;
        }
        else {
            // Get us an instrucIter
            InstrucIter callTarget(target, multiT->proc());
            instruction firstInsn = callTarget.getInstruction();
            instruction secondInsn = callTarget.getNextInstruction();
            inst_printf("Checking for call to thunk...\n");
            if (firstInsn.isMoveRegMemToRegMem() &&
                secondInsn.isReturn()) {
                inst_printf("Found mov/ret...\n");
                // We need to fake this by figuring out the register
                // target (assuming we're moving stack->reg),
                // and constructing an immediate with the value of the
                // original address of the call (+size)
                // This was copied from function relocation code... I 
                // don't understand most of it -- bernat
                const unsigned char *ptr = firstInsn.ptr();
                unsigned char modrm = *(ptr + 1);
                unsigned char reg = (modrm >> 3) & 0x3;
                // Source register... 
                if ((modrm == 0x0c) || (modrm == 0x1c)) {
                    inst_printf("Passed modrm\n");
                    // Check source register (%esp == 0x24)
                    if ((*(ptr + 2) == 0x24)) {
                        inst_printf("And source register right\n");
                        // Okay, put the PC into the 'reg'
                        Address EIP = origAddr + insn.size();
                        *newInsn = 0xb8 + reg; // Opcode???
                        newInsn++;
                        unsigned int *temp = (unsigned int *)newInsn;
                        *temp = EIP;
                        //assert(sizeof(unsigned int *)==4);
                        //newInsn += sizeof(unsigned int *);
                        newInsn += 4;  // fix for AMD64
                        done = true;
                    }
                }
            }
        }
    }
    if (!done) {
        // If call-specialization didn't take care of us
        if (insnType & REL_B) {
            /* replace with rel32 instruction, opcode is one byte. */
            if (*origInsn == JCXZ) {
                if (!targetOverride_) {
                    oldDisp = (int)*(const char *)(origInsn+1);
                    newDisp = (origAddr + 2) + oldDisp - (addrInMutatee_ + 9);
                }
                else {
                    // Override the target to go somewhere else
                    oldDisp = 0;
                    newDisp = targetOverride_ - (addrInMutatee_ + 9);
                }
                *newInsn++ = *origInsn; *(newInsn++) = 2; // jcxz 2
                *newInsn++ = 0xEB; *newInsn++ = 5;        // jmp 5
                *newInsn++ = 0xE9;                        // jmp rel32
                *((int *)newInsn) = newDisp;
                newInsn += sizeof(int);
            }
            else {
                unsigned newSz=UINT_MAX;
                if (insnType & IS_JCC) {
                    /* Change a Jcc rel8 to Jcc rel32.  Must generate a new opcode: a
                       0x0F followed by (old opcode + 16) */
                    unsigned char opcode = *origInsn++;
                    *newInsn++ = 0x0F;
                    *newInsn++ = opcode + 0x10;
                    newSz = 6;
                }
                else if (insnType & IS_JUMP) {
                    /* change opcode to 0xE9 */
                    origInsn++;
                    *newInsn++ = 0xE9;
                    newSz = 5;
                }
                assert(newSz!=UINT_MAX);
                if (!targetOverride_) {
                    oldDisp = (int)*(const char *)origInsn;
                    newDisp = (origAddr + 2) + oldDisp - (addrInMutatee_ + newSz);
                }
                else {
                    oldDisp = 0;
                    newDisp = targetOverride_ - (addrInMutatee_ + newSz);
                }
                *((int *)newInsn) = newDisp;
                newInsn += sizeof(int);
            }
        }
        else if (insnType & REL_W) {
            /* Skip prefixes */
            if (insnType & PREFIX_OPR)
                origInsn++;
            if (insnType & PREFIX_SEG)
                origInsn++;
            /* opcode is unchanged, just relocate the displacement */
            if (*origInsn == (unsigned char)0x0F)
                *newInsn++ = *origInsn++;
            *newInsn++ = *origInsn++;
            if (!targetOverride_) {
                oldDisp = *((const short *)origInsn);
                newDisp = (origAddr + 5) + oldDisp - (addrInMutatee_ + 3);
            }
            else {
                oldDisp = 0;
                newDisp = targetOverride_ -  (addrInMutatee_ + 3);
            }
            *((int *)newInsn) = newDisp;
            newInsn += sizeof(int);
        } else if (insnType & REL_D || insnType & REL_D_DATA) {

            // Skip prefixes
            unsigned nPrefixes = 0;
            if (insnType & PREFIX_OPR)
                nPrefixes++;
            if (insnType & PREFIX_SEG)
                nPrefixes++;
	    if (insnType & PREFIX_OPCODE)
		nPrefixes++;
            if (insnType & PREFIX_REX)
                nPrefixes++;

            for (unsigned u = 0; u < nPrefixes; u++)
                *newInsn++ = *origInsn++;
            
            /* opcode is unchanged, just relocate the displacement */
            if (*origInsn == 0x0F)
                *newInsn++ = *origInsn++;
            *newInsn++ = *origInsn++;

	    // skip over ModRM byte for RIP-relative data
	    if (insnType & REL_D_DATA)
		*newInsn++ = *origInsn++;

            if (!targetOverride_) {
                oldDisp = *((const int *)origInsn);
                newDisp = (origAddr + insnSz) + oldDisp - (addrInMutatee_ + insnSz);
            }
            else {
                oldDisp = 0;
                newDisp = targetOverride_ - (addrInMutatee_ + insnSz);
            }
            *((int *)newInsn) = newDisp;
            newInsn += sizeof(int);

	    // copy the rest of the instruction for RIP-relative data (there may be an immediate)
	    // we can do this since the insn size will always be the same
	    if (insnType & REL_D_DATA) {
		origInsn += sizeof(int);
		while (newInsn - insnBuf < (int)insnSz)
		    *newInsn++ = *origInsn++;
	    }
        }
        else {
            /* instruction is unchanged */
            for (unsigned u = 0; u < insnSz; u++)
                *newInsn++ = *origInsn++;
        }
    }

    SET_PTR(newInsn, gen);

    inst_printf("Emitted new instruction; size is %d\n", size_);
    size_ = gen.currAddr(baseInMutatee) - addrInMutatee_;
    generated_ = true;

    return true;
}



registerSpace *regSpace32;
#if defined(arch_x86_64)
registerSpace *regSpace64;
#endif
registerSpace *regSpace;

bool registerSpace::readOnlyRegister(Register) {
  return false;
}

Register deadList32[NUM_VIRTUAL_REGISTERS];
int deadList32Size = sizeof(deadList32);

#if defined(arch_x86_64)
// we do non-arg registers here first - followed by arg registers in reverse order
Register deadList64[] = {RBX, R10, R11, R12, R13, R14, R15, R9, R8, RCX, RDX, RSI, RDI};
int deadList64Size = sizeof(deadList64);
#endif

void initTramps(bool is_multithreaded)
{
    static bool inited = false;

    if (inited) return;
    inited = true;

    unsigned regs_to_loop_over;
    if(is_multithreaded)
	regs_to_loop_over = NUM_VIRTUAL_REGISTERS - 1;
    else
	regs_to_loop_over = NUM_VIRTUAL_REGISTERS;

    for (unsigned u = 0; u < regs_to_loop_over; u++) {
	deadList32[u] = u+1;
    }

    regSpace32 = new registerSpace(deadList32Size/sizeof(Register), deadList32,
				   0, NULL, is_multithreaded);

#if defined(arch_x86_64)
    assert(!is_multithreaded);
    regSpace64 = new registerSpace(deadList64Size/sizeof(Register), deadList64,
				   0, NULL, is_multithreaded);
#endif

    // default to 32-bit
    regSpace = regSpace32;
}


unsigned generateAndWriteBranch(process *proc, 
                                Address fromAddr, 
                                Address newAddr,
                                unsigned fillSize)
{
    if (fillSize == 0) fillSize = BASE_TO_MINI_JMP_SIZE;
    codeGen gen(fillSize);

#if defined(arch_x86_64)
    long disp = newAddr - (fromAddr + JUMP_REL32_SZ);
    if (disp > I32_MAX || disp < I32_MIN) {

	// need to use an indirect jmp
	emitMovImmToReg64(RAX, newAddr, true, gen);
	emitSimpleInsn(0xff, gen); // group 5
	emitSimpleInsn(0xe0, gen); // mod = 11, reg = 4 (call Ev), r/m = 0 (RAX)
    }
    else {
	// can use a direct jump
	instruction::generateBranch(gen, fromAddr, newAddr);
	gen.fillRemaining(codeGen::cgNOP);
    }
#else
    // we can always use a direct jump on x86
    instruction::generateBranch(gen, fromAddr, newAddr);
    gen.fillRemaining(codeGen::cgNOP);
#endif
    
    proc->writeTextSpace((caddr_t)fromAddr, gen.used(), gen.start_ptr());
    return gen.used();
}

// Should move...
unsigned miniTramp::interJumpSize() {

    return BASE_TO_MINI_JMP_SIZE;
}

unsigned multiTramp::maxSizeRequired() {
    // A jump to the multiTramp, 
    // todo: in-line :)

    return JUMP_REL32_SZ;
}

/* Generate a jump to a base tramp. Return the size of the instruction
   generated at the instrumentation point. */

// insn: buffer to generate code

bool multiTramp::generateBranchToTramp(codeGen &gen)
{
    /* There are three ways to get to the base tramp:
       1. Ordinary 5-byte jump instruction.
       2. Using a short jump to hit nearby space, and long-jumping to the multiTramp. 
       3. Trap instruction.
       
       We currently support #s 1 and 3.
    */
    
    assert(instAddr_);
    assert(trampAddr_);
    unsigned origUsed = gen.used();

    /* Ordinary 5-byte jump */
    if (instSize_ >= JUMP_REL32_SZ) {
        instruction::generateBranch(gen, instAddr_, trampAddr_);
    }
    /* Extra slot */
    else { /* Trap */
        /*
        fprintf(stderr, "MultiTramp requires trap at 0x%x to 0x%x, func %s\n",
                instAddr(), instAddr()+instSize(), func()->prettyName().c_str());
        */
        if (!canUseTrap_) {
            return false;
        }
        // We're doing a trap. Now, we know that trap addrs are reported
        // as "finished" address... so use that one (not instAddr_)
        proc()->trampTrapMapping[gen.currAddr(instAddr_)+1] = trampAddr_;
        instruction::generateTrap(gen);
    }

    branchSize_ = gen.used() - origUsed;
    // We play a cute trick with the rest of the overwritten space: fill it with
    // traps. If one is hit, we can transfer the PC into the multiTramp without
    // further problems. Cute, eh?
    while (gen.used() < instSize()) {
        Address origAddr = gen.currAddr(instAddr_);
        Address addrInMulti = uninstToInstAddr(origAddr);
        // x86: traps read at PC + 1
        if (addrInMulti) {
            // addrInMulti may be 0 if our trap instruction does not
            // map onto a real instruction
            proc()->trampTrapMapping[origAddr+1] = addrInMulti;
        }
        instruction::generateTrap(gen);
    }

    return true;
}

bool baseTramp::generateSaves(codeGen& gen, registerSpace*) {

    return x86_emitter->emitBTSaves(this, gen);
}

bool baseTramp::generateRestores(codeGen &gen, registerSpace*) {

    return x86_emitter->emitBTRestores(this, gen);
}

bool baseTramp::generateMTCode(codeGen &gen, registerSpace*) {

    return x86_emitter->emitBTMTCode(this, gen);
}

bool baseTramp::generateGuardPreCode(codeGen &gen,
                                     codeBufIndex_t &guardJumpOffset,
				     registerSpace*) {

    return x86_emitter->emitBTGuardPreCode(this, gen, guardJumpOffset);
}

bool baseTramp::generateGuardPostCode(codeGen &gen,
				      codeBufIndex_t &guardTargetIndex,
				      registerSpace *) {

    return x86_emitter->emitBTGuardPostCode(this, gen, guardTargetIndex);
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

    // 2 for the jump size; TODO fixme
    // Should handle longer branches
    if (disp < 256) {
        emitJccR8(JE_R8, (disp- 2), gen);
        // Moves gen...
        jumpSize = gen.used() - start;
    }
    else {
        // We need a bigger jump buffer.
        assert(0); // TODO
    }

    gen.fill(baseT->guardBranchSize - jumpSize,
             codeGen::cgNOP);
        
    return true;
}
       

bool baseTramp::generateCostCode(codeGen &gen, unsigned &costUpdateOffset,
                                 registerSpace *) {
    Address costAddr = proc()->getObservedCostAddr();
    inst_printf("costAddr is 0x%x\n", costAddr);
    if (!costAddr) return false;

    return x86_emitter->emitBTCostCode(this, gen, costUpdateOffset);
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
#if defined(arch_x86_64)
	emitMovImmToReg64(RAX, costAddr, true, gen);
	emitOpRMImm(0x81, 0, RAX, 0, cost, gen);
#else
	assert(0);
#endif
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
    *insn++ = condition_code;
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
static inline void emitJcc(int condition, int offset,
                           codeGen &gen, bool willRegen=true)
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

int tramp_pre_frame_size_32 = 36; //Stack space allocated by 'pushf; pusha'

int tramp_pre_frame_size_64 = 8 + 16 * 8 + 128; // stack space allocated by pushing flags and 16 GPRs
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
   return ((Mod & 0x3) << 6) + ((Reg & 0x7) << 3) + (RM & 0x7);
}

// VG(7/30/02): Build the SIB byte of an instruction */
static inline unsigned char makeSIBbyte(unsigned Scale, unsigned Index,
                                        unsigned Base)
{
  return ((Scale & 0x3) << 6) + ((Index & 0x7) << 3) + (Base & 0x7);
}

static void emitAddressingMode(Register base, Register index,
                               unsigned int scale, RegValue disp,
                               int reg_opcode, codeGen &gen);


/* 
   Emit the ModRM byte and displacement for addressing modes.
   base is a register (EAX, ECX, EDX, EBX, EBP, ESI, EDI)
   disp is a displacement
   reg_opcode is either a register or an opcode
*/
static inline void emitAddressingMode(Register base, RegValue disp,
                                      int reg_opcode, codeGen &gen)
{
    // MT linux uses ESP+4
    // we need an SIB in that case
    if (base == ESP) {
	emitAddressingMode(ESP, Null_Register, 0, disp, reg_opcode, gen);
	return;
    }
    GET_PTR(insn, gen);
    if (base == Null_Register) {
	*insn++ = makeModRMbyte(0, reg_opcode, 5);
	*((int *)insn) = disp;
	insn += sizeof(int);
    } else if (disp == 0 && base != EBP) {
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
static void emitAddressingMode(Register base, Register index,
			       unsigned int scale, RegValue disp,
			       int reg_opcode, codeGen &gen)
{
   bool needSIB = (base == ESP) || (index != Null_Register);

   if(!needSIB) {
      emitAddressingMode(base, disp, reg_opcode, gen);
      return;
   }
   
   assert(index != ESP);
   
   if(index == Null_Register) {
      assert(base == ESP); // not necessary, but sane
      index = 4;           // (==ESP) which actually means no index in SIB
   }

   GET_PTR(insn, gen);
   
   if(base == Null_Register) { // we have to emit [index<<scale+disp32]
      *insn++ = makeModRMbyte(0, reg_opcode, 4);
      *insn++ = makeSIBbyte(scale, index, 5);
      *((int *)insn) = disp;
      insn += sizeof(int);
   }
   else if(disp == 0 && base != EBP) { // EBP must have 0 disp8; emit [base+index<<scale]
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
    *insn++ = op;
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
	*insn++ = opcode;
    else {
	*insn++ = opcode >> 8;
	*insn++ = opcode & 0xFF;
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
	*insn++ = opcode;
    } else {
	*insn++ = opcode >> 8;
	*insn++ = opcode & 0xff;
    }
    SET_PTR(insn, gen);
    emitAddressingMode(base, disp, dest, gen);
}

// emit OP r/m, reg
void emitOpRMReg(unsigned opcode, Register base, int disp,
                               Register src, codeGen &gen) {
    GET_PTR(insn, gen);
   *insn++ = opcode;
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
    *insn++ = opcode1;
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
    *insn++ = opcode1;
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
    *insn++ = opcode;
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
void emitMovImmToReg(Register dest, int imm,
		     codeGen &gen) {
    GET_PTR(insn, gen);
   *insn++ = 0xB8 + dest;
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
    GET_PTR(insn, gen);
   *insn++ = 0xC7;
   // emit the ModRM byte: we use a 32-bit displacement for the address,
   // the ModRM value is 0x05
   *insn++ = 0x05;
   *((unsigned *)insn) = maddr;
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
}

static inline void emitEnter(short imm16, codeGen &gen) {
    GET_PTR(insn, gen);
   *insn++ = 0xC8;
   *((short*)insn) = imm16;
   insn += sizeof(short);
   *insn++ = 0;
    SET_PTR(insn, gen);
}

// this function just multiplexes between the 32-bit and 64-bit versions
Register emitFuncCall(opCode op, 
                      registerSpace *rs,
                      codeGen &gen,
                      pdvector<AstNode *> &operands, 
                      process *proc,
                      bool noCost,
		      Address callee_addr,
                      const pdvector<AstNode *> &ifForks,
                      const instPoint *location)
{
    return x86_emitter->emitCall(op, rs, gen, operands, proc, noCost, callee_addr, ifForks, location);
}

Register Emitter32::emitCall(opCode op, 
                             registerSpace *rs,
                             codeGen &gen,
                             const pdvector<AstNode *> &operands, 
                             process *proc,
                             bool noCost, Address callee_addr,
                             const pdvector<AstNode *> &ifForks,
                             const instPoint *location)
{
   assert(op == callOp);
   pdvector <Register> srcs;

   //  Sanity check for NULL address arg
   if (!callee_addr) {
      char msg[256];
      sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out"
              "callee_addr argument", __FILE__, __LINE__);
      showErrorCallback(80, msg);
      assert(0);
   }
 
   for (unsigned u = 0; u < operands.size(); u++)
       srcs.push_back((Register)operands[u]->generateCode_phase2(proc, rs, gen,
                                                                 noCost, 
                                                                 ifForks, 
                                                                 location));

   // push arguments in reverse order, last argument first
   // must use int instead of unsigned to avoid nasty underflow problem:
   for (int i=srcs.size() - 1 ; i >= 0; i--) {
       emitOpRMReg(PUSH_RM_OPC1, EBP, -(srcs[i]*4), PUSH_RM_OPC2, gen);
       rs->freeRegister(srcs[i]);
   }

   // make the call
   // we are using an indirect call here because we don't know the
   // address of this instruction, so we can't use a relative call.
   // TODO: change this to use a direct call
   emitMovImmToReg(EAX, callee_addr, gen);       // mov eax, addr
   emitOpRegReg(CALL_RM_OPC1, CALL_RM_OPC2, EAX, gen);   // call *(eax)

   // reset the stack pointer
   if (srcs.size() > 0)
      emitOpRegImm(0, ESP, srcs.size()*4, gen); // add esp, srcs.size()*4

   // allocate a (virtual) register to store the return value
   Register ret = rs->allocateRegister(gen, noCost);
   emitMovRegToRM(EBP, -(ret*4), EAX, gen);

   return ret;
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
	 retval = x86_emitter->emitIf(src1, dest, gen);
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
	 instruction::generateNOOP(gen, BASE_TO_MINI_JMP_SIZE);
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
          x86_emitter->emitGetRetVal(dest, gen);
          return dest;
      }
      case getParamOp: {
         // src1 is the number of the argument
         // dest is a register where we can store the value
          x86_emitter->emitGetParam(dest, src1, location->getPointType(), gen);
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

#ifdef BPATCH_LIBRARY
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
    
    emitMovRMToReg(EAX, EBP, SAVED_EFLAGS_OFFSET, gen); // mov eax, offset[ebp]
    emitSimpleInsn(0x50, gen);  // push eax
    emitSimpleInsn(POPFD, gen); // popfd
    emitJcc(condition, offset, gen);
}

// VG(07/30/02): Restore mutatee value of GPR reg to dest (real) GPR
static inline void restoreGPRtoGPR(Register reg, Register dest,
                                   codeGen &gen)
{
    // NOTE: I don't use emitLoadPreviousStackFrameRegister because it saves
    // the value to a virtual (stack based) register, which is not what I want!
    
    // mov dest, offset[ebp]
    emitMovRMToReg(dest, EBP, SAVED_EAX_OFFSET-(reg<<2), gen);
}

// VG(11/07/01): Load in destination the effective address given
// by the address descriptor. Used for memory access stuff.
void emitASload(const BPatch_addrSpec_NP *as, Register dest, codeGen &gen, bool /* noCost */) {
   // TODO 16-bit registers, rep hacks
   int imm = as->getImm();
   int ra  = as->getReg(0);
   int rb  = as->getReg(1);
   int sc  = as->getScale();

   bool havera = ra > -1, haverb = rb > -1;

   // VG(7/30/02): given that we use virtual (stack allocated) registers for
   // our inter-snippet temporaries, I assume all real registers to be fair
   // game.  So, we restore the original registers in EAX and EDX - this
   // allows us to generate a lea (load effective address instruction) that
   // will make the cpu do the math for us.

   // assuming 32-bit addressing (for now)

   //bperr( "ASLOAD: ra=%d rb=%d sc=%d imm=%d\n", ra, rb, sc, imm);

   if(havera)
      restoreGPRtoGPR(ra, EAX, gen);        // mov eax, [saved_ra]

   if(haverb)
      restoreGPRtoGPR(rb, EDX, gen);        // mov edx, [saved_rb]

   // Emit the lea to do the math for us:

   // e.g. lea eax, [eax + edx * sc + imm] if both ra and rb had to be
   // restored
   emitLEA((havera ? EAX : Null_Register), (haverb ? EDX : Null_Register),
           sc, imm, EAX, gen);

   emitMovRegToRM(EBP, -(dest<<2), EAX, gen); // mov (virtual reg) dest, eax
}

void emitCSload(const BPatch_countSpec_NP *as, Register dest,
		codeGen &gen, bool /* noCost */ )
{
   // VG(7/30/02): different from ASload on this platform, no LEA business

   int imm = as->getImm();
   int ra  = as->getReg(0);
   int rb  = as->getReg(1);
   int sc  = as->getScale();

   // count is at most 1 register or constant or hack (aka pseudoregister)
   assert((ra == -1) &&
          ((rb == -1) ||
            ((imm == 0) && (rb == 1 /*ECX */ || rb >= IA32_EMULATE))));

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
           emitMovRMToReg(EAX, EBP, SAVED_EFLAGS_OFFSET, gen);

           emitSimpleInsn(0x50, gen);  // push eax
           emitSimpleInsn(POPFD, gen); // popfd
           restoreGPRtoGPR(EAX, EAX, gen);
           restoreGPRtoGPR(ECX, ECX, gen);
           restoreGPRtoGPR(EDI, EDI, gen);
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
           restoreGPRtoGPR(ECX, EAX, gen); // old ecx -> eax
           emitSubRegReg(EAX, ECX, gen); // eax = eax - ecx
           if(sc > 0)
              emitSHL(EAX, sc, gen);              // shl eax, scale

           // mov (virtual reg) dest, eax
           emitMovRegToRM(EBP, -(dest<<2), EAX, gen);

           break;
        case IA32_NECMPS:
           neg = true;
        case IA32_ECMPS:
           // plan: restore flags, esi, edi, ecx; do rep(n)e cmps(b/w);
           // compute (saved_ecx - ecx) << sc;

           // mov eax, offset[ebp]
           emitMovRMToReg(EAX, EBP, SAVED_EFLAGS_OFFSET, gen);

           emitSimpleInsn(0x50, gen);  // push eax
           emitSimpleInsn(POPFD, gen); // popfd
           restoreGPRtoGPR(ECX, ECX, gen);
           restoreGPRtoGPR(ESI, ESI, gen);
           restoreGPRtoGPR(EDI, EDI, gen);
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
           restoreGPRtoGPR(ECX, EAX, gen); // old ecx -> eax
           emitSubRegReg(EAX, ECX, gen); // eax = eax - ecx
           if(sc > 0)
              emitSHL(EAX, sc, gen);              // shl eax, scale

           // mov (virtual reg) dest, eax
           emitMovRegToRM(EBP, -(dest<<2), EAX, gen);

           break;
        default:
           assert(!"Wrong emulation!");
      }
   }
   else if(rb > -1) {
      //bperr( "!!!In case rb > -1!!!\n");
      // TODO: 16-bit pseudoregisters
      assert(rb < 8); 
      restoreGPRtoGPR(rb, EAX, gen);        // mov eax, [saved_rb]
      if(sc > 0)
         emitSHL(EAX, sc, gen);              // shl eax, scale

      // mov (virtual reg) dest, eax
      emitMovRegToRM(EBP, -(dest<<2), EAX, gen);
   }
   else
      emitMovImmToRM(EBP, -(dest<<2), imm, gen);
}
#endif


void emitVload(opCode op, Address src1, Register /*src2*/, Register dest, 
               codeGen &gen, bool /*noCost*/, 
               registerSpace * /*rs*/, int size,
               const instPoint * /* location */, process * /* proc */)
{
   if (op == loadConstOp) {
      // dest is a temporary
      // src1 is an immediate value 
      // dest = src1:imm32
       x86_emitter->emitLoadConst(dest, src1, gen);
      return;
   } else if (op ==  loadOp) {
      // dest is a temporary
      // src1 is the address of the operand
      // dest = [src1]
       x86_emitter->emitLoad(dest, src1, size, gen);
      return;
   } else if (op == loadFrameRelativeOp) {
      // dest is a temporary
      // src1 is the offset of the from the frame of the variable
       x86_emitter->emitLoadFrameRelative(dest, src1, gen);
       return;
   } else if (op == loadFrameAddr) {
       x86_emitter->emitLoadFrameAddr(dest, src1, gen);
       return;
   } else {
      abort();                // unexpected op for this emit!
   }
}

void emitVstore(opCode op, Register src1, Register src2, Address dest,
                codeGen &gen, bool /*noCost*/, registerSpace * /*rs*/, 
                int /* size */,
                const instPoint * /* location */, process * /* proc */)
{
   if (op ==  storeOp) {
      // [dest] = src1
      // dest has the address where src1 is to be stored
      // src1 is a temporary
      // src2 is a "scratch" register, we don't need it in this architecture
       x86_emitter->emitStore(dest, src1, gen);
      return;
   } else if (op == storeFrameRelativeOp) {
       // src1 is a temporary
       // src2 is a "scratch" register, we don't need it in this architecture
       // dest is the frame offset 
       x86_emitter->emitStoreFrameRelative(dest, src1, src2, gen);
       return;
   } else {
       abort();                // unexpected op for this emit!
   }
}

void emitV(opCode op, Register src1, Register src2, Register dest, 
           codeGen &gen, bool /*noCost*/, 
           registerSpace * /*rs*/, int /* size */,
           const instPoint * /* location */, process * /* proc */)
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
        x86_emitter->emitLoadIndir(dest, src1, gen);
    } 
    else if (op ==  storeIndirOp) {
        // same as storeOp, but the address where to store is already in a
        // register
        x86_emitter->emitStoreIndir(dest, src1, gen);
    } else if (op == noOp) {
        emitSimpleInsn(NOP, gen); // nop
    } else if (op == saveRegOp) {
        // should not be used on this platform
        assert(0);
        
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
	   x86_emitter->emitDiv(dest, src1, src2, gen);
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
            x86_emitter->emitRelOp(op, dest, src1, src2, gen);
            return;
            break;
        }
        default:
            abort();
            break;
        }
        x86_emitter->emitOp(opcode, dest, src1, src2, gen);
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
      emitMovImmToReg(EAX, dest, gen);
      emitMovImmToRM(EAX, 0, src1, gen);
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
          x86_emitter->emitTimesImm(dest, src1, src2imm, gen);
          return;
          break;
      }
      case divOp: {
          x86_emitter->emitDivImm(dest, src1, src2imm, gen);
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
          x86_emitter->emitRelOpImm(op, dest, src1, src2imm, gen);
          return;
          break;
      }
      default:
          abort();
          break;
      }
      x86_emitter->emitOpImm(opcode1, opcode2, dest, src1, src2imm, gen);
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


dictionary_hash<pdstring, unsigned> funcFrequencyTable(pdstring::hash);

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
    
    if (!funcFrequencyTable.defines(func->prettyName())) {
        // Changing this value from 250 to 100 because predictedCost was
        // too high - naim 07/18/96
        return(100.0);       
    } else {
        return ((float)funcFrequencyTable[func->prettyName()]);
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

// process::replaceFunctionCall
//
// Replace the function call at the given instrumentation point with a call to
// a different function, or with a NOOP.  In order to replace the call with a
// NOOP, pass NULL as the parameter "func."
// Returns true if sucessful, false if not.  Fails if the site is not a call
// site, or if the site has already been instrumented using a base tramp.
//
// Note that right now we can only replace a call instruction that is five
// bytes long (like a call to a 32-bit relative address).
// 18APR05: don't see why we can't fix up a base tramp, it's just a little more
// complicated.

// By the way, we want to be able to disable these, which means keeping
// track of what we put in. Since addresses are unique, we can do it with
// a dictionary in the process class.
bool process::replaceFunctionCall(instPoint *point,
                                  const int_function *func) {
    // Must be a call site
  if (point->getPointType() != callSite)
    return false;

  instPointIter ipIter(point);
  instPointInstance *ipInst;
  while ((ipInst = ipIter++)) {  
      // Multiple replacements. Wheee...
      Address pointAddr = ipInst->addr();

      codeRange *range;
      if (multiTramps_.find(pointAddr, range)) {
          // We instrumented this... not handled yet
          assert(0);
      }
      else {
          codeGen gen(point->insn().size());
          // Uninstrumented
          // Replace the call
          if (func == NULL) {	// Replace with NOOPs
              gen.fillRemaining(codeGen::cgNOP);
          } else { // Replace with a call to a different function
              // XXX Right only, call has to be 5 bytes -- sometime, we should make
              // it work for other calls as well.
              assert(point->insn().size() == CALL_REL32_SZ);
              instruction::generateCall(gen, pointAddr, func->getAddress());
          }

          // Before we replace, track the code.
          // We could be clever with instpoints keeping instructions around, but
          // it's really not worth it.
          replacedFunctionCall *newRFC = new replacedFunctionCall();
          newRFC->callAddr = pointAddr;

          codeGen old(point->insn().size());
          old.copy(point->insn().ptr(), point->insn().size());
                      
          newRFC->oldCall = old;
          newRFC->newCall = gen;
          
          replacedFunctionCalls_[pointAddr] = newRFC;
          
          writeTextSpace((void *)pointAddr, gen.used(), gen.start_ptr());


      }
  }
  return true;
}

// Emit code to jump to function CALLEE without linking.  (I.e., when
// CALLEE returns, it returns to the current caller.)
void emitFuncJump(opCode op, 
                  codeGen &gen,
		  const int_function *callee, process *,
		  const instPoint *loc, bool)
{
    // This must mimic the generateRestores baseTramp method. 
    // TODO: make this work better :)

    assert(op == funcJumpOp);

    Address addr = callee->getOriginalAddress();
    instPointType_t ptType = loc->getPointType();
    x86_emitter->emitFuncJump(addr, ptType, gen);
}

void Emitter32::emitFuncJump(Address addr, instPointType_t ptType, codeGen &gen)
{       
    if (ptType == otherPoint)
        emitOpRegRM(FRSTOR, FRSTOR_OP, EBP, -TRAMP_FRAME_SIZE - FSAVE_STATE_SIZE, gen);
    emitSimpleInsn(LEAVE, gen);     // leave
    emitSimpleInsn(POP_EAX, gen);
    emitSimpleInsn(POPAD, gen);     // popad
    emitSimpleInsn(POPFD, gen);
    
    GET_PTR(insn, gen);
    *insn++ = 0x68; /* push 32 bit immediate */
    *((int *)insn) = addr; /* the immediate */
    insn += 4;
    *insn++ = 0xc3; /* ret */
    SET_PTR(insn, gen);

    instruction::generateIllegal(gen);
}

void emitLoadPreviousStackFrameRegister(Address register_num,
                                        Register dest,
                                        codeGen &gen,
                                        int,
                                        bool){
  
  x86_emitter->emitLoadPreviousStackFrameRegister(register_num, dest, gen);
}

// First AST node: target of the call
// Second AST node: source of the call

bool process::getDynamicCallSiteArgs(instPoint *callSite,
                                     pdvector<AstNode *> &args)
{
   Register base_reg, index_reg;
   int displacement;
   unsigned scale;
   int addr_mode;
   unsigned Mod;

   const instruction &i = callSite->insn();
   if(i.isCallIndir()){
      addr_mode = get_instruction_operand(i.ptr(), base_reg, index_reg,
                                          displacement, scale, Mod);
      switch(addr_mode){

	  // casting first to long, then void* in calls to the AstNode
	  // constructor below avoids a mess of compiler warnings on AMD64
        case REGISTER_DIRECT:
           {
              args.push_back( new AstNode(AstNode::PreviousStackFrameDataReg,
                                        (void *)(long) base_reg));
              break;
           }
        case REGISTER_INDIRECT:
           {
              AstNode *prevReg =
                 new AstNode(AstNode::PreviousStackFrameDataReg,
                             (void *)(long) base_reg);
              args.push_back( new AstNode(AstNode::DataIndir, prevReg));
              break;
           }
        case REGISTER_INDIRECT_DISPLACED:
           {
              AstNode *prevReg =
                 new AstNode(AstNode::PreviousStackFrameDataReg,
                             (void *)(long) base_reg);
              AstNode *offset = new AstNode(AstNode::Constant,
                                            (void *)(long) displacement);
              AstNode *sum = new AstNode(plusOp, prevReg, offset);

              args.push_back( new AstNode(AstNode::DataIndir, sum));
              break;
           }
        case DISPLACED:
           {
              AstNode *offset = new AstNode(AstNode::Constant,
                                            (void *)(long) displacement);
              args.push_back( new AstNode(AstNode::DataIndir, offset));
              break;
           }
        case SIB:
           {
              AstNode *effective_address;
              if(index_reg != 4) { //We use a scaled index
                 bool useBaseReg = true;
                 if(Mod == 0 && base_reg == 5){
                    cerr << "Inserting untested call site monitoring "
                         << "instrumentation at address " << std::hex
                         << callSite->addr() << std::dec << endl;
                    useBaseReg = false;
                 }

                 AstNode *index =
                    new AstNode(AstNode::PreviousStackFrameDataReg,
                                (void *)(long) index_reg);
                 AstNode *base =
                    new AstNode(AstNode::PreviousStackFrameDataReg,
                                (void *)(long) base_reg);

                 AstNode *disp = new AstNode(AstNode::Constant,
                                             (void *)(long) displacement);

                 if(scale == 1){ //No need to do the multiplication
                    if(useBaseReg){
                       AstNode *base_index_sum = new AstNode(plusOp, index,
                                                             base);
                       effective_address = new AstNode(plusOp, base_index_sum,
                                                       disp);
                    }
                    else
                       effective_address = new AstNode(plusOp, index, disp);

                    args.push_back( new AstNode(AstNode::DataIndir,
                                              effective_address));

                 }
                 else {
                    AstNode *scale_factor
                       = new AstNode(AstNode::Constant, (void *)(long) scale);
                    AstNode *index_scale_product = new AstNode(timesOp, index,
                                                               scale_factor);
                    if(useBaseReg){
                       AstNode *base_index_sum =
                          new AstNode(plusOp, index_scale_product, base);
                       effective_address = new AstNode(plusOp, base_index_sum,
                                                       disp);
                    }
                    else
                       effective_address = new AstNode(plusOp,
                                                       index_scale_product,
                                                       disp);
                    args.push_back( new AstNode(AstNode::DataIndir,
                                              effective_address));

                 }
              }
              else { //We do not use a scaled index.
                 cerr << "Inserting untested call site monitoring "
                      << "instrumentation at address " << std::hex
                      << callSite->addr() << std::dec << endl;
                 AstNode *base =
                    new AstNode(AstNode::PreviousStackFrameDataReg,
                                (void *)(long) base_reg);
                 AstNode *disp = new AstNode(AstNode::Constant,
                                             (void *)(long) displacement);
                 AstNode *effective_address =  new AstNode(plusOp, base,
                                                           disp);
                 args.push_back( new AstNode(AstNode::DataIndir,
                                           effective_address));
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
      args.push_back( new AstNode(AstNode::Constant,
				  (void *) callSite->addr()));

   }
   else if(i.isCall()){
      //Regular callees are statically determinable, so no need to
      //instrument them
      //return true;
      fprintf(stderr, "%s[%d]:  FIXME,  dynamic call is statically determinable\n at address %x (%s)",
	      __FILE__, __LINE__, callSite->addr(), callSite->func()->prettyName().c_str());
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
    emitMovImmToReg(EAX, val, gen);
    emitSimpleInsn(0xc3, gen); //ret
    
    return proc()->writeTextSpace((void *) addr, gen.used(), gen.start_ptr());
}


/* For AIX liveness analysis */
void registerSpace::resetLiveDeadInfo(const int *)
{}

bool registerSpace::clobberRegister(Register /*reg*/) 
{
  return false;
}

bool registerSpace::clobberFPRegister(Register /*reg*/)
{
  return false;
}

unsigned saveRestoreRegistersInBaseTramp(process */*proc*/, 
                                         baseTramp */*bt*/,
                                         registerSpace */*rs*/)
{
  return 0;
}


