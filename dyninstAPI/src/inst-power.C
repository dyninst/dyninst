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
 * inst-power.C - Identify instrumentation points for a RS6000/PowerPCs
 * $Id: inst-power.C,v 1.283 2008/02/20 22:34:26 legendre Exp $
 */

#include "common/h/headers.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/inst-power.h"
#include "dyninstAPI/src/arch-power.h"
#if defined(os_aix)
#include "dyninstAPI/src/aix.h"
#endif
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/instPoint.h" // class instPoint
#include "dyninstAPI/src/debug.h"
#include "common/h/debugOstream.h"
#include "dyninstAPI/src/InstrucIter.h"
#include "dyninstAPI/src/rpcMgr.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/multiTramp.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/src/BPatch_collections.h"
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/registerSpace.h"

#include "InstrucIter.h"

#include "emitter.h"
#include "emit-power.h"

#include <sstream>

#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

extern bool isPowerOf2(int value, int &result);

#define DISTANCE(x,y)   ((x<y) ? (y-x) : (x-y))

Address getMaxBranch() {
  return MAX_BRANCH;
}

const char *registerNames[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
			"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
			"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
			"r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"};

dictionary_hash<pdstring, unsigned> funcFrequencyTable(pdstring::hash);

void initDefaultPointFrequencyTable()
{
#ifdef notdef
    funcFrequencyTable[EXIT_NAME] = 1;

    FILE *fp;
    float value;
    char name[512];

    funcFrequencyTable["main"] = 1;
    funcFrequencyTable["DYNINSTsampleValues"] = 1;
    // try to read file.
    fp = fopen("freq.input", "r");
    if (!fp) {
	bperr("no freq.input file\n");
	return;
    }
    while (!feof(fp)) {
	fscanf(fp, "%s %f\n", name, &value);
	funcFrequencyTable[name] = (int) value;
	bperr("adding %s %f\n", name, value);
    }
    fclose(fp);
#endif
}

/*
 * Get an etimate of the frequency for the passed instPoint.  
 *    This is not (always) the same as the function that contains the point.
 * 
 *  The function is selected as follows:
 *
 *  If the point is an entry or an exit return the function name.
 *  If the point is a call and the callee can be determined, return the called
 *     function.
 *  else return the funcation containing the point.
 *
 *  WARNING: This code contins arbitray values for func frequency (both user 
 *     and system).  This should be refined over time.
 *
 * Using 1000 calls sec to be one SD from the mean for most FPSPEC apps.
 *	-- jkh 6/24/94
 *
 */
float getPointFrequency(instPoint *point)
{

    int_function *func;

    func = point->findCallee();
    if (!func)
        func = point->func();
    
    if (!funcFrequencyTable.defines(func->prettyName().c_str())) {
        // Changing this value from 250 to 100 because predictedCost was
        // too high - naim 07/18/96
        return(100);
    } else {
        return (funcFrequencyTable[func->prettyName().c_str()]);
    }
}


int instPoint::liveRegSize()
{
  return maxGPR;
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
              // How the heck does a base tramp only cost 35 cycles?
              // -- bernat, 4OCT03
              // 35 cycles for base tramp
              // + 70 cyles for MT version (assuming 1 cycle per instruction)
              worstCost = 105; // Magic constant from before time
          }
      }
      else {
          // No multiTramp, so still free (we're not instrumenting here).
      }
  }
  return worstCost;
}

unsigned baseTramp::getBTCost() {
    // Ummm... check this
    return 105;
}

/*
 * Given and instruction, relocate it to a new address, patching up
 *   any relative addressing that is present.
 *
 */

unsigned relocatedInstruction::maxSizeRequired() {
    return insn->spaceToRelocate();
}

Register floatingLiveRegList[] = {13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
unsigned int floatingLiveRegListSize = 14;

// It appears as though 32-bit AIX and 32-bit Linux have compatible
// calling conventions, and thus we share the initialization code
// for both. 

// Note that while we have register definitions for r13..r31, we only
// use r0..r12 (well, r3..r12). I believe this is to reduce the number
// of saves that we execute. 


void registerSpace::initialize32() {
    static bool done = false;
    if (done) return;
    done = true;

    pdvector<registerSlot *> registers;

    // At ABI boundary: R0 and R12 are dead, others are live.
    // Also, define registers in reverse order - it helps with
    // function calls
    
    registers.push_back(new registerSlot(r12,
                                         false,
                                         registerSlot::deadABI,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r11,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r10,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r9,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r8,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r7,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r6,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r5,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r4,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r3,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    /// Aaaand the off-limits ones.

    registers.push_back(new registerSlot(r0,
                                         true, // Don't use r0 - it has all sorts
                                         // of implicit behavior.
                                         registerSlot::deadABI,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r1,
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r2,
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));

    for (unsigned i = fpr0; i <= fpr13; i++) {
        registers.push_back(new registerSlot(i,
                                             false,
                                             registerSlot::liveAlways,
                                             registerSlot::FPR));
    }

    registers.push_back(new registerSlot(lr,
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(cr,
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(ctr,
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(mq,
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registerSpace::createRegisterSpace(registers);

    // Initialize the sets that encode which registers
    // are used/defined by calls, returns, and syscalls. 
    // These all assume the ABI, of course. 

    // TODO: Linux/PPC needs these set as well.
    
#if defined(cap_liveness)
    returnRead_ = getBitArray();
    // Return reads r3, r4, fpr1, fpr2
    returnRead_[r3] = true;
    returnRead_[r4] = true;
    returnRead_[fpr1] = true;
    returnRead_[fpr2] = true;

    // Calls
    callRead_ = getBitArray();
    // Calls read r3 -> r10 (parameters), fpr1 -> fpr13 (volatile FPRs)
    for (unsigned i = r3; i <= r10; i++) 
        callRead_[i] = true;
    for (unsigned i = fpr1; i <= fpr13; i++) 
        callRead_[i] = true;
    callWritten_ = getBitArray();
    // Calls write to pretty much every register we use for code generation
    callWritten_[r0] = true;
    for (unsigned i = r3; i <= r12; i++)
        callWritten_[i] = true;
    // FPRs 0->13 are volatile
    for (unsigned i = fpr0; i <= fpr13; i++)
        callWritten_[i] = true;

    // Syscall - assume the same as call
    syscallRead_ = callRead_;
    syscallWritten_ = syscallWritten_;
#endif
}

void registerSpace::initialize64() {
    static bool done = false;
    if (done) return;
    done = true;

    pdvector<registerSlot *> registers;

    // At ABI boundary: R0 and R12 are dead, others are live.
    // Also, define registers in reverse order - it helps with
    // function calls
    
    registers.push_back(new registerSlot(r12,
                                         false,
                                         registerSlot::deadABI,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r11,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r10,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r9,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r8,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r7,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r6,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r5,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r4,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r3,
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    /// Aaaand the off-limits ones.

    registers.push_back(new registerSlot(r0,
                                         true, // Don't use r0 - it has all sorts
                                         // of implicit behavior.
                                         registerSlot::deadABI,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r1,
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r2,
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));

    for (unsigned i = fpr0; i <= fpr13; i++) {
        registers.push_back(new registerSlot(i,
                                             false,
                                             registerSlot::liveAlways,
                                             registerSlot::FPR));
    }

    registers.push_back(new registerSlot(lr,
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(cr,
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(ctr,
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(mq,
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registerSpace::createRegisterSpace64(registers);

    // Initialize the sets that encode which registers
    // are used/defined by calls, returns, and syscalls. 
    // These all assume the ABI, of course. 

    // TODO: Linux/PPC needs these set as well.
    
#if defined(cap_liveness)
    returnRead64_ = getBitArray();
    // Return reads r3, r4, fpr1, fpr2
    returnRead64_[r3] = true;
    returnRead64_[r4] = true;
    returnRead64_[fpr1] = true;
    returnRead64_[fpr2] = true;

    // Calls
    callRead64_ = getBitArray();
    // Calls read r3 -> r10 (parameters), fpr1 -> fpr13 (volatile FPRs)
    for (unsigned i = r3; i <= r10; i++) 
        callRead64_[i] = true;
    for (unsigned i = fpr1; i <= fpr13; i++) 
        callRead64_[i] = true;
    callWritten64_ = getBitArray();
    // Calls write to pretty much every register we use for code generation
    callWritten64_[r0] = true;
    for (unsigned i = r3; i <= r12; i++)
        callWritten64_[i] = true;
    // FPRs 0->13 are volatile
    for (unsigned i = fpr0; i <= fpr13; i++)
        callWritten64_[i] = true;

    // Syscall - assume the same as call
    syscallRead64_ = callRead_;
    syscallWritten64_ = syscallWritten_;
#endif
}

void registerSpace::initialize() {
    initialize32();
    initialize64();
}

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

    ////////////////////////////////////////////////////////////////////
    //Generates instructions to save a special purpose register onto
    //the stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
// NOTE: the bit layout of the mfspr instruction is as follows:
// opcode:6 ; RT: 5 ; SPR: 10 ; const 339:10 ; Rc: 1
// However, the two 5-bit halves of the SPR field are reversed
// so just using the xfxform will not work
void saveSPR(codeGen &gen,     //Instruction storage pointer
             Register    scratchReg, //Scratch register
             int         sprnum,     //SPR number
             int         stkOffset) //Offset from stack pointer
{
    instruction insn;

    (*insn).raw = 0;                    //mfspr:  mflr scratchReg
    (*insn).xform.op = EXTop;
    (*insn).xform.rt = scratchReg;
    (*insn).xform.ra = sprnum & 0x1f;
    (*insn).xform.rb = (sprnum >> 5) & 0x1f;
    (*insn).xform.xo = MFSPRxop;
    insn.generate(gen);

    if (gen.addrSpace()->getAddressWidth() == 4) {
	instruction::generateImm(gen, STop,
                                 scratchReg, 1, stkOffset);
    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
	instruction::generateMemAccess64(gen, STDop, STDxop,
                                         scratchReg, 1, stkOffset);
    }
}

    ////////////////////////////////////////////////////////////////////
    //Generates instructions to restore a special purpose register from
    //the stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
void restoreSPR(codeGen &gen,       //Instruction storage pointer
                Register      scratchReg, //Scratch register
                int           sprnum,     //SPR number
                int           stkOffset)  //Offset from stack pointer
{
    if (gen.addrSpace()->getAddressWidth() == 4) {
        instruction::generateImm(gen, Lop,
                                 scratchReg, 1, stkOffset);
    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
        instruction::generateMemAccess64(gen, LDop, LDxop,
                                         scratchReg, 1, stkOffset);
    }

    instruction insn;
    (*insn).raw = 0;                    //mtspr:  mtlr scratchReg
    (*insn).xform.op = EXTop;
    (*insn).xform.rt = scratchReg;
    (*insn).xform.ra = sprnum & 0x1f;
    (*insn).xform.rb = (sprnum >> 5) & 0x1f;
    (*insn).xform.xo = MTSPRxop;
    insn.generate(gen);
}

           ////////////////////////////////////////////////////////////////////
	   //Generates instructions to save link register onto stack.
	   //  Returns the number of bytes needed to store the generated
	   //    instructions.
	   //  The instruction storage pointer is advanced the number of 
	   //    instructions generated.
void saveLR(codeGen &gen,       //Instruction storage pointer
            Register      scratchReg, //Scratch register
            int           stkOffset)  //Offset from stack pointer
{
    saveSPR(gen, scratchReg, SPR_LR, stkOffset);
}

           ////////////////////////////////////////////////////////////////////
           //Generates instructions to restore link register from stack.
           //  Returns the number of bytes needed to store the generated
	   //    instructions.
	   //  The instruction storage pointer is advanced the number of 
	   //    instructions generated.
	   //
void restoreLR(codeGen &gen,       //Instruction storage pointer
               Register      scratchReg, //Scratch register
               int           stkOffset)  //Offset from stack pointer
{
    restoreSPR(gen, scratchReg, SPR_LR, stkOffset);
}

           ////////////////////////////////////////////////////////////////////
           //Generates instructions to place a given value into link register.
	   //  The entire instruction sequence consists of the generated
	   //    instructions followed by a given (tail) instruction.
	   //  Returns the number of bytes needed to store the entire
	   //    instruction sequence.
	   //  The instruction storage pointer is advanced the number of 
	   //    instructions in the sequence.
	   //
void setBRL(codeGen &gen,        //Instruction storage pointer
            Register      scratchReg,  //Scratch register
            long          val,         //Value to set link register to
            unsigned      ti)          //Tail instruction
{
    instruction::loadImmIntoReg(gen, scratchReg, val);

    instruction insn;
    (*insn).raw = 0;                 //mtspr:  mtlr scratchReg
    (*insn).xform.op = EXTop;
    (*insn).xform.rt = scratchReg;
    (*insn).xform.ra = SPR_LR;
    (*insn).xform.xo = MTSPRxop;
    insn.generate(gen);

    (*insn).raw = ti;
    insn.generate(gen);
}

     //////////////////////////////////////////////////////////////////////////
     //Writes out instructions to place a value into the link register.
     //  If val == 0, then the instruction sequence is followed by a `nop'.
     //  If val != 0, then the instruction sequence is followed by a `brl'.
     //
void resetBRL(process  *p,   //Process to write instructions into
	      Address   loc, //Address in process to write into
	      unsigned  val) //Value to set link register
{
    codeGen gen(10*instruction::size());
    Register scratch = 10;
    if (val) {
        setBRL(gen, scratch, val, BRLraw);
    }
    else {
        setBRL(gen, scratch, val, NOOPraw);
    }
    p->writeTextSpace((void *)loc, gen.used(), gen.start_ptr());
}

    /////////////////////////////////////////////////////////////////////////
    //Generates instructions to save the condition codes register onto stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
void saveCR(codeGen &gen,       //Instruction storage pointer
            Register      scratchReg, //Scratch register
            int           stkOffset)  //Offset from stack pointer
{
    instruction insn;

  (*insn).raw = 0;                    //mfcr:  mflr scratchReg
  (*insn).xfxform.op = EXTop;
  (*insn).xfxform.rt = scratchReg;
  (*insn).xfxform.xo = MFCRxop;
  insn.generate(gen);

  if (gen.addrSpace()->getAddressWidth() == 4) {
      instruction::generateImm(gen, STop,
			       scratchReg, 1, stkOffset);
  } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
      instruction::generateMemAccess64(gen, STDop, STDxop,
				       scratchReg, 1, stkOffset);
  }
}

    ///////////////////////////////////////////////////////////////////////////
    //Generates instructions to restore the condition codes register from stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
void restoreCR(codeGen &gen,       //Instruction storage pointer
               Register      scratchReg, //Scratch register
               int           stkOffset)  //Offset from stack pointer
{
    instruction insn;

    if (gen.addrSpace()->getAddressWidth() == 4) {
        instruction::generateImm(gen, Lop,
                                 scratchReg, 1, stkOffset);
    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
        instruction::generateMemAccess64(gen, LDop, LDxop,
                                         scratchReg, 1, stkOffset);
    }

    (*insn).raw = 0;                    //mtcrf:  scratchReg
    (*insn).xfxform.op  = EXTop;
    (*insn).xfxform.rt  = scratchReg;
    (*insn).xfxform.spr = 0xff << 1;
    (*insn).xfxform.xo  = MTCRFxop;
    insn.generate(gen);
}

    /////////////////////////////////////////////////////////////////////////
    //Generates instructions to save the floating point status and control
    //register on the stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
void saveFPSCR(codeGen &gen,       //Instruction storage pointer
               Register      scratchReg, //Scratch fp register
               int           stkOffset)  //Offset from stack pointer
{
    instruction mffs;

    (*mffs).raw = 0;                    //mffs scratchReg
    (*mffs).xform.op = X_FP_EXTENDEDop;
    (*mffs).xform.rt = scratchReg;
    (*mffs).xform.xo = MFFSxop;
    mffs.generate(gen);

    //st:     st scratchReg, stkOffset(r1)
    instruction::generateImm(gen, STFDop, scratchReg, 1, stkOffset);
}

    ///////////////////////////////////////////////////////////////////////////
    //Generates instructions to restore the floating point status and control
    //register from the stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
void restoreFPSCR(codeGen &gen,       //Instruction storage pointer
                  Register      scratchReg, //Scratch fp register
                  int           stkOffset)  //Offset from stack pointer
{
    instruction::generateImm(gen, LFDop, scratchReg, 1, stkOffset);

    instruction mtfsf;
    (*mtfsf).raw = 0;                    //mtfsf:  scratchReg
    (*mtfsf).xflform.op  = X_FP_EXTENDEDop;
    (*mtfsf).xflform.flm = 0xff;
    (*mtfsf).xflform.frb = scratchReg;
    (*mtfsf).xflform.xo  = MTFSFxop;
    mtfsf.generate(gen);
}

     //////////////////////////////////////////////////////////////////////////
     //Writes out a `br' instruction
     //
void resetBR(process  *p,    //Process to write instruction into
	     Address   loc)  //Address in process to write into
{
    instruction i;

    (*i).raw = BRraw;
  
    if (!p->writeDataSpace((void *)loc, instruction::size(), i.ptr()))
        fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
}

void saveRegister(codeGen &gen,
                  Register reg,
                  int save_off)
{
    if (gen.addrSpace()->getAddressWidth() == 4) {
        instruction::generateImm(gen, STop,
                                 reg, 1, save_off + reg*GPRSIZE_32);
    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
        instruction::generateMemAccess64(gen, STDop, STDxop,
                                         reg, 1, save_off + reg*GPRSIZE_64);
    }
    //  bperr("Saving reg %d at 0x%x off the stack\n", reg, offset + reg*GPRSIZE);
}

// Dest != reg : optimizate away a load/move pair
void restoreRegister(codeGen &gen,
                     Register source,
                     Register dest, int saved_off)
{
    if (gen.addrSpace()->getAddressWidth() == 4) {
        instruction::generateImm(gen, Lop, 
                                 dest, 1, saved_off + source*GPRSIZE_32);
    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
        instruction::generateMemAccess64(gen, LDop, LDxop,
                                         dest, 1, saved_off + source*GPRSIZE_64);
    }
    //bperr( "Loading reg %d (into reg %d) at 0x%x off the stack\n", 
    //  reg, dest, offset + reg*GPRSIZE);
}

void restoreRegister(codeGen &gen,
                     Register reg,
                     int save_off)
{
    restoreRegister(gen, reg, reg, save_off);
}

void saveFPRegister(codeGen &gen, 
                    Register reg,
                    int save_off)
{
    instruction::generateImm(gen, STFDop, 
                             reg, 1, save_off + reg*FPRSIZE);
    //bperr( "Saving FP reg %d at 0x%x off the stack\n", 
    //  reg, offset + reg*FPRSIZE);
}

void restoreFPRegister(codeGen &gen,
                       Register source,
                       Register dest,
                       int save_off)
{
    instruction::generateImm(gen, LFDop, 
                             dest, 1, save_off + source*FPRSIZE);
    //  bperr("Loading FP reg %d (into %d) at 0x%x off the stack\n", 
    //  reg, dest, offset + reg*FPRSIZE);
}

void restoreFPRegister(codeGen &gen,
                       Register reg,
                       int save_off)
{
    restoreFPRegister(gen, reg, reg, save_off);
}	

/*
 * Emit code to push down the stack, AST-generate style
 */
void pushStack(codeGen &gen)
{
    if (gen.addrSpace()->getAddressWidth() == 4) {
	instruction::generateImm(gen, STUop,
				 REG_SP, REG_SP, -TRAMP_FRAME_SIZE_32);
    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
	instruction::generateMemAccess64(gen, STDop, STDUxop,
                                  REG_SP, REG_SP, -TRAMP_FRAME_SIZE_64);
    }
}

void popStack(codeGen &gen)
{
    if (gen.addrSpace()->getAddressWidth() == 4) {
	instruction::generateImm(gen, CALop, 
				 REG_SP, REG_SP, TRAMP_FRAME_SIZE_32);

    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
	instruction::generateImm(gen, CALop,
                                 REG_SP, REG_SP, TRAMP_FRAME_SIZE_64);
    }
}

/*
 * Save necessary registers on the stack
 * insn, base: for code generation. Offset: regs saved at offset + reg
 * Returns: number of registers saved.
 * Side effects: instruction pointer and base param are shifted to 
 *   next free slot.
 */
unsigned saveGPRegisters(codeGen &gen,
                         registerSpace *theRegSpace,
                         int save_off)
{
    unsigned numRegs = 0;
    for(int i = 0; i < theRegSpace->numGPRs(); i++) {
        registerSlot *reg = theRegSpace->GPRs()[i];
        if (reg->liveState == registerSlot::live) {
	    saveRegister(gen, reg->encoding(), save_off);
	    gen.rs()->markSavedRegister(reg->number, save_off + reg->number*gen.addrSpace()->getAddressWidth());	
	    numRegs++;
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

unsigned restoreGPRegisters(codeGen &gen,
                            registerSpace *theRegSpace,
                            int save_off)
{
    unsigned numRegs = 0;
    for(int i = 0; i < theRegSpace->numGPRs(); i++) {
        registerSlot *reg = theRegSpace->GPRs()[i];
        if (reg->liveState == registerSlot::spilled) {
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

unsigned saveFPRegisters(codeGen &gen,
                         registerSpace * theRegSpace,
                         int save_off)
{
  unsigned numRegs = 0;
  for(int i = 0; i < theRegSpace->numFPRs(); i++) {
      registerSlot *reg = theRegSpace->FPRs()[i];
      if (reg->liveState == registerSlot::live) {
          saveFPRegister(gen, reg->encoding(), save_off);
          reg->liveState = registerSlot::spilled;
          numRegs++;
      }
  }  
  
  return numRegs;
}

/*
 * Restore FPR registers from the stack. (0-13)
 * insn, base: for code generation. Offset: regs restored from offset + reg
 * Returns: number of regs restored.
 */

unsigned restoreFPRegisters(codeGen &gen, 
                            registerSpace *theRegSpace,
                            int save_off)
{
  
  unsigned numRegs = 0;
  for(int i = 0; i < theRegSpace->numFPRs(); i++) {
      registerSlot *reg = theRegSpace->FPRs()[i];
      if (reg->liveState == registerSlot::spilled) {
          restoreFPRegister(gen, reg->encoding(), save_off);
          numRegs++;
      }
  }
  
  return numRegs;
}

/*
 * Save the special purpose registers (for Dyninst conservative tramp)
 * CTR, CR, XER, SPR0, FPSCR
 */
unsigned saveSPRegisters(codeGen &gen,
			 registerSpace * theRegSpace,
                         int save_off)
{
    unsigned num_saved = 0;
    int cr_off, ctr_off, xer_off, spr0_off, fpscr_off;
    
    if (gen.addrSpace()->getAddressWidth() == 4) {
	cr_off    = STK_CR_32;
	ctr_off   = STK_CTR_32;
	xer_off   = STK_XER_32;
	spr0_off  = STK_SPR0_32;
	fpscr_off = STK_FP_CR_32;
    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
	cr_off    = STK_CR_64;
	ctr_off   = STK_CTR_64;
	xer_off   = STK_XER_64;
	spr0_off  = STK_SPR0_64;
	fpscr_off = STK_FP_CR_64;
    }
    
    saveCR(gen, 10, save_off + cr_off); num_saved++;
    saveSPR(gen, 10, SPR_CTR, save_off + ctr_off); num_saved++;
    saveSPR(gen, 10, SPR_XER, save_off + xer_off); num_saved++;
    
#if defined(os_aix) && !defined(rs6000_ibm_aix64)
    // MQ only exists on POWER, not PowerPC. Right now that's correlated
    // to AIX vs Linux, but we _really_ should fix that...
    // We need to dynamically determine the CPU and emit code based on that.
    //
    // Apparently, AIX64 doesn't use the MQ register either.
    registerSlot *mq = ((*theRegSpace)[registerSpace::mq]);
    if (mq->liveState == registerSlot::live) {
        saveSPR(gen, 10, SPR_SPR0, save_off + spr0_off); num_saved++;
        mq->liveState = registerSlot::spilled;
    }
#endif
    saveFPSCR(gen, 10, save_off + fpscr_off); num_saved++;
    return num_saved;
}

/*
 * Restore the special purpose registers (for Dyninst conservative tramp)
 * CTR, CR, XER, SPR0, FPSCR
 */

unsigned restoreSPRegisters(codeGen &gen,
			    registerSpace *theRegSpace,
                            int save_off)
{
    int cr_off, ctr_off, xer_off, spr0_off, fpscr_off;
    unsigned num_restored = 0;

    if (gen.addrSpace()->getAddressWidth() == 4) {
	cr_off    = STK_CR_32;
	ctr_off   = STK_CTR_32;
	xer_off   = STK_XER_32;
	spr0_off  = STK_SPR0_32;
	fpscr_off = STK_FP_CR_32;
    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
	cr_off    = STK_CR_64;
	ctr_off   = STK_CTR_64;
	xer_off   = STK_XER_64;
	spr0_off  = STK_SPR0_64;
	fpscr_off = STK_FP_CR_64;
    }

    restoreCR(gen, 10, save_off + cr_off); num_restored++;
    restoreSPR(gen, 10, SPR_CTR, save_off + ctr_off); num_restored++;
    restoreSPR(gen, 10, SPR_XER, save_off + xer_off); num_restored++;

#if defined(os_aix) && !defined(rs6000_ibm_aix64)
    // See comment in saveSPRegisters
    registerSlot *mq = ((*theRegSpace)[registerSpace::mq]);
    if (mq->liveState == registerSlot::spilled) {
        restoreSPR(gen, 10, SPR_SPR0, save_off + spr0_off); num_restored++;
    }
#endif
    restoreFPSCR(gen, 10, save_off + fpscr_off); num_restored++;
    return num_restored;
}


bool baseTrampInstance::finalizeGuardBranch(codeGen &gen,
                                            int disp) {
    // Assumes that preCode is generated
    // and we're now finalizing the jump to go
    // past whatever miniTramps may have been.

    // Note: must be a conditional jump

    assert(disp > 0);

    // Conditional jump if not equal.
    instruction jumpInsn;

    (*jumpInsn).raw = 0;
    (*jumpInsn).bform.op = BCop;
    (*jumpInsn).bform.bo = BFALSEcond;
    (*jumpInsn).bform.bi = EQcond;
    (*jumpInsn).bform.bd = disp >> 2;
    (*jumpInsn).bform.aa = 0; 
    (*jumpInsn).bform.lk = 0;
    jumpInsn.generate(gen);

    return true;
}
       

bool baseTramp::generateSaves(codeGen &gen,
                              registerSpace *)
{
    regalloc_printf("========== baseTramp::generateSaves\n");
    
    int gpr_off, fpr_off, ctr_off;
    if (gen.addrSpace()->getAddressWidth() == 4) {
        gpr_off = TRAMP_GPR_OFFSET_32;
        fpr_off = TRAMP_FPR_OFFSET_32;
        ctr_off = STK_CTR_32;
    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
        gpr_off = TRAMP_GPR_OFFSET_64;
        fpr_off = TRAMP_FPR_OFFSET_64;
        ctr_off = STK_CTR_64;
    }

    // Make a stack frame.
    pushStack(gen);

    // Save GPRs
    saveGPRegisters(gen, gen.rs(), gpr_off);

    if(BPatch::bpatch->isSaveFPROn()) // Save FPRs
	saveFPRegisters(gen, gen.rs(), fpr_off);

    // Save LR
    saveLR(gen, REG_SCRATCH /* register to use */, TRAMP_SPR_OFFSET + STK_LR);

    // No more cookie. FIX aix stackwalking.
    if (isConservative())
        saveSPRegisters(gen, gen.rs(), TRAMP_SPR_OFFSET);
    else if (isCallsite())
        saveSPR(gen, REG_SCRATCH, SPR_CTR, TRAMP_SPR_OFFSET + ctr_off);

    return true;
}

bool baseTramp::generateRestores(codeGen &gen,
                                 registerSpace *)
{
    int gpr_off, fpr_off, ctr_off;
    if (gen.addrSpace()->getAddressWidth() == 4) {
        gpr_off = TRAMP_GPR_OFFSET_32;
        fpr_off = TRAMP_FPR_OFFSET_32;
        ctr_off = STK_CTR_32;
    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
        gpr_off = TRAMP_GPR_OFFSET_64;
        fpr_off = TRAMP_FPR_OFFSET_64;
        ctr_off = STK_CTR_64;
    }

    // Restore possible SPR saves
    if (isConservative())
        restoreSPRegisters(gen, gen.rs(), TRAMP_SPR_OFFSET);
    else if (isCallsite())
        restoreSPR(gen, REG_SCRATCH, SPR_CTR, TRAMP_SPR_OFFSET + ctr_off);


    // LR
    restoreLR(gen, REG_SCRATCH, TRAMP_SPR_OFFSET + STK_LR);

    if (BPatch::bpatch->isSaveFPROn()) // FPRs
	restoreFPRegisters(gen, gen.rs(), fpr_off);

    // GPRs
    restoreGPRegisters(gen, gen.rs(), gpr_off);

    /*
    // Multithread GPR -- always save
    restoreRegister(gen, REG_MT_POS, TRAMP_GPR_OFFSET);
    */

    popStack(gen);

    return true;
}

/*
 *            Get the hashed thread ID on the stack and in REG_MT_POS
 *            hashed thread ID * sizeof(int) in REG_GUARD_OFFSET
 *
 * So: call DYNINSTthreadIndex, which returns the INDEX value. This is
 *     done automatically (AST), the rest by hand. Save the INDEX.
 *
 */
 
// This is 99% identical with the version for each other platform;
// the result move is different. 

bool baseTramp::generateMTCode(codeGen &gen,
                               registerSpace *) {

    AstNodePtr threadPOS;
    pdvector<AstNodePtr> dummy;
    Register src = Null_Register;
    
    dyn_thread *thr = gen.thread();
    if (!threaded()) {
        /* Get the hashed value of the thread */
        emitVload(loadConstOp, 0, REG_MT_POS, REG_MT_POS, gen, false);
    }
    else if (thr) {
        // Override 'normal' index value...
        emitVload(loadConstOp, thr->get_index(), REG_MT_POS, REG_MT_POS, gen, false);
    }
    else {
        threadPOS = AstNode::funcCallNode("DYNINSTthreadIndex", dummy);
        if (!threadPOS->generateCode(gen,
                                     false, // noCost 
                                     src)) return false;
        if ((src) != REG_MT_POS) {
            // This is always going to happen... we reserve REG_MT_POS, so the
            // code generator will never use it as a destination
            instruction::generateImm(gen, ORILop, src, REG_MT_POS, 0);
			gen.rs()->freeRegister(src);
        }
    }

    return true;
}

bool baseTramp::generateGuardPreCode(codeGen &gen,
                                     codeBufIndex_t &guardJumpIndex,
                                     registerSpace *) {
    assert(guarded());

    Address trampGuardFlagAddr = proc()->trampGuardBase();
    if (!trampGuardFlagAddr) {
        guardJumpIndex = 0;
        return false;
    }

    // Load; check; branch; modify; store.
    emitVload(loadConstOp, trampGuardFlagAddr, REG_GUARD_ADDR, REG_GUARD_ADDR,
              gen, false, gen.rs(), sizeof(unsigned));
    // MT; if we're MTing we have a base (hence trampGuardBase()) but
    // need to index it to get our base
    if (threaded()) {
        // Build the offset in a spare register
        emitImm(timesOp, (Register) REG_MT_POS, (RegValue) sizeof(unsigned),
                REG_GUARD_OFFSET, gen, false);
        // And add it back in.
        emitV(plusOp, REG_GUARD_ADDR, REG_GUARD_OFFSET, REG_GUARD_ADDR,
              gen, false);
    }
    // We have the abs. addr in REG_GUARD_ADDR. We keep it around,
    // so load into a different reg.
    emitV(loadIndirOp, REG_GUARD_ADDR, 0, // Unused -- always 0.
          REG_GUARD_VALUE, gen, false, gen.rs(), sizeof(unsigned));
    instruction::generateImm(gen, CMPIop, 0, REG_GUARD_VALUE, 1);

    // Next thing we do is jump; store that in guardJumpOffset
    guardJumpIndex = gen.getIndex();

    // Drop an illegal in here for now.
    instruction::generateIllegal(gen);
    
    // Back to the base/offset style. 
    emitVload(loadConstOp, 0, REG_GUARD_VALUE, REG_GUARD_VALUE,
              gen, false, gen.rs(), sizeof(unsigned));
    // And store
    emitV(storeIndirOp, REG_GUARD_VALUE, 0, REG_GUARD_ADDR, 
          gen, false, gen.rs(), sizeof(unsigned));

    // And we store the addr so we don't have to recalculate later.
    instruction::generateImm(gen, STop, REG_GUARD_ADDR, 1, // SP
                             STK_GUARD); 
    return true;
}

bool baseTramp::generateGuardPostCode(codeGen &gen, codeBufIndex_t &index,
                                      registerSpace *) {
    assert(guarded());

    Address trampGuardFlagAddr = proc()->trampGuardBase();
    if (!trampGuardFlagAddr) {
        return false;
    }

    // Load addr; modify; store.
    instruction::generateImm(gen, Lop, REG_GUARD_ADDR, 1, STK_GUARD);
    
    emitVload(loadConstOp, 1, REG_GUARD_VALUE, REG_GUARD_VALUE, 
              gen, false, gen.rs(), sizeof(unsigned));
    // And store
    emitV(storeIndirOp, REG_GUARD_VALUE, 0, REG_GUARD_ADDR, 
          gen, false, gen.rs(), sizeof(unsigned));

    index = gen.getIndex();

    return true;
}

bool baseTramp::generateCostCode(codeGen &gen,
                                 unsigned &costUpdateOffset,
                                 registerSpace *) {
    // Load; modify; store.
    Address costAddr = proc()->getObservedCostAddr();
    if (!costAddr) return false;

    emitVload(loadConstOp, costAddr, 0, // unused
              REG_COST_ADDR, gen, false);
    emitV(loadIndirOp, REG_COST_ADDR, 0, 
          REG_COST_VALUE, gen, false);
    // We assume cost will be less than an immediate...
    // This insn does the math.
    costUpdateOffset = gen.used();
    emitImm(plusOp, REG_COST_VALUE, 0, // No cost yet
            REG_COST_VALUE, gen, false);

    // And store
    emitV(storeIndirOp, REG_COST_VALUE, 0, REG_COST_ADDR, 
          gen, false);

    return true;
}

void baseTrampInstance::updateTrampCost(unsigned cost) {
    // We only need to overwrite the cost addition
    // Though we also need to overwrite memory.
    if (baseT->costSize == 0) return;

    Address trampCostAddr = trampPreAddr() + baseT->costValueOffset;

    codeGen gen(instruction::size());

    emitImm(plusOp, REG_COST_VALUE, cost,
            REG_COST_VALUE, gen, false);

    // And write
    if (!proc()->writeDataSpace((void *)trampCostAddr,
                           gen.used(),
                           gen.start_ptr()))
     fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
}

void emitImm(opCode op, Register src1, RegValue src2imm, Register dest, 
             codeGen &gen, bool noCost, registerSpace * /* rs */)
{
        //bperr("emitImm(op=%d,src=%d,src2imm=%d,dest=%d)\n",
        //        op, src1, src2imm, dest);
    int iop=-1;
    int result=-1;
    switch (op) {
        // integer ops
    case plusOp:
        iop = CALop;
        instruction::generateImm(gen, iop, dest, src1, src2imm);
        return;
        break;
        
    case minusOp:
        iop = SIop;
        instruction::generateImm(gen, iop, dest, src1, src2imm);
        return;
        break;
        
    case timesOp:
       if (isPowerOf2(src2imm,result) && (result < (int) (gen.addrSpace()->getAddressWidth() * 8))) {
            instruction::generateLShift(gen, src1, result, dest);
            return;
        }
        else {
            Register dest2 = gen.rs()->getScratchRegister(gen, noCost);
            emitVload(loadConstOp, src2imm, dest2, dest2, gen, noCost);
            emitV(op, src1, dest2, dest, gen, noCost);
            return;
        }
        break;
        
    case divOp:
        if (isPowerOf2(src2imm,result) && (result < (int) (gen.addrSpace()->getAddressWidth() * 8))) {
            instruction::generateRShift(gen, src1, result, dest);
            return;
        }
        else {
            Register dest2 = gen.rs()->getScratchRegister(gen, noCost);
            emitVload(loadConstOp, src2imm, dest2, dest2, gen, noCost);
            emitV(op, src1, dest2, dest, gen, noCost);
            return;
        }
        break;
        
        // Bool ops
    case orOp:
        iop = ORILop;
        // For some reason, the destField is 2nd for ORILop and ANDILop
        instruction::generateImm(gen, iop, src1, dest, src2imm);
        return;
        break;
        
    case andOp:
        iop = ANDILop;
        // For some reason, the destField is 2nd for ORILop and ANDILop
        instruction::generateImm(gen, iop, src1, dest, src2imm);
        return;
        break;
    default:
        Register dest2 = gen.rs()->getScratchRegister(gen, noCost);
        emitVload(loadConstOp, src2imm, dest2, dest2, gen, noCost);
        emitV(op, src1, dest2, dest, gen, noCost);
        return;
        break;
    }
}

void cleanUpAndExit(int status);

/* Recursive function that goes to where our instrumentation is calling
to figure out what registers are clobbered there, and in any function
that it calls, to a certain depth ... at which point we clobber everything

Update-12/06, njr, since we're going to a cached system we are just going to 
look at the first level and not do recursive, since we would have to also
store and reexamine every call out instead of doing it on the fly like before*/
bool EmitterPOWER::clobberAllFuncCall( registerSpace *rs,
                                       int_function * callee)
		   
{
  unsigned i;
  if (!callee) return true;

  stats_codegen.startTimer(CODEGEN_LIVENESS_TIMER);
    
  /* usedRegs does calculations if not done before and returns
     whether or not the callee is a leaf function.
     if it is, we use the register info we gathered,
     otherwise, we punt and save everything */
  //     bool isLeafFunc = callee->ifunc()->usedRegs();

  if (callee->ifunc()->isLeafFunc()) {
      std::set<Register> * gprs = callee->ifunc()->usedGPRs();
      std::set<Register>::iterator It = gprs->begin();
      for(i = 0; i < gprs->size(); i++) {
          //while (It != gprs->end()){
          rs->GPRs()[*(It++)]->beenUsed = true;
      }
      
      std::set<Register> * fprs = callee->ifunc()->usedFPRs();
      std::set<Register>::iterator It2 = fprs->begin();
      for(i = 0; i < fprs->size(); i++)
      {
          //while (It2 != fprs->end()){
          rs->FPRs()[*(It2++)]->beenUsed = true;
      }
    }
  else {
      for (int i = 0; i < rs->numGPRs(); i++) {
          rs->GPRs()[i]->beenUsed = true;
      }
      for (int i = 0; i < rs->numFPRs(); i++) {
          rs->FPRs()[i]->beenUsed = true;
      }
  }
  stats_codegen.stopTimer(CODEGEN_LIVENESS_TIMER);
  return false;
}


//
// Author: Jeff Hollingsworth (3/26/96)
//
// Emit a function call.
//   It saves registers as needed.
//   copy the passed arguments into the canonical argument registers (r3-r10)
//   AIX and 64-bit ELF Linux ONLY: 
//     Locate the TOC entry of the callee module and copy it into R2
//   generate a branch and link the destination
//   AIX and 64-bit ELF Linux ONLY:
//     Restore the original TOC into R2
//   restore the saved registers.
//
// Parameters:
//   op - unused parameter (to be compatible with sparc)
//   srcs - vector of ints indicating the registers that contain the parameters
//   dest - the destination address (should be Address not reg). 
//   insn - pointer to the code we are generating
//   based - offset into the code generated.
//

Register emitFuncCall(opCode, codeGen &, pdvector<AstNodePtr> &, bool, Address) {
	assert(0);
        return 0;
}

Register emitFuncCall(opCode op,
                      codeGen &gen,
                      pdvector<AstNodePtr> &operands, bool noCost,
                      int_function *callee) {
    return gen.emitter()->emitCall(op, gen, operands, noCost, callee);
}


Register EmitterPOWERDyn::emitCall(opCode /* ocode */, 
                                   codeGen &gen,
                                   const pdvector<AstNodePtr> &operands, bool noCost,
                                   int_function *callee) {

    Address toc_anchor = 0;
    pdvector <Register> srcs;

    //  Sanity check for NULL address argument
    if (!callee) {
        char msg[256];
        sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out"
                "callee argument", __FILE__, __LINE__);
        showErrorCallback(80, msg);
        assert(0);
    }

    // Now that we have the destination address (unique, hopefully) 
    // get the TOC anchor value for that function
    // The TOC offset is stored in the Object. 
    // file() -> pdmodule "parent"
    // exec() -> image "parent"
    toc_anchor = gen.addrSpace()->proc()->getTOCoffsetInfo(callee);

    // Note: For 32-bit ELF PowerPC Linux (and other SYSV ABI followers)
    // r2 is described as "reserved for system use and is not to be 
    // changed by application code".
    // On these platforms, we return 0 when getTOCoffsetInfo is called.

    pdvector<int> savedRegs;

    //  Save the link register.
    // mflr r0
    instruction mflr0(MFLR0raw);
    mflr0.generate(gen);

    // Register 0 is actually the link register, now. However, since we
    // don't want to overwrite the LR slot, save it as "register 0"
    saveRegister(gen, 0, FUNC_CALL_SAVE);
    // Add 0 to the list of saved registers
    savedRegs.push_back(0);

    if (toc_anchor) {
        // Save register 2 (TOC)
        saveRegister(gen, 2, FUNC_CALL_SAVE);
        savedRegs.push_back(2);
    }

    // see what others we need to save.
    for (int i = 0; i < gen.rs()->numGPRs(); i++) {
       registerSlot *reg = gen.rs()->GPRs()[i];

       // We must save if:
       // refCount > 0 (and not a source register)
       // keptValue == true (keep over the call)
       // liveState == live (technically, only if not saved by the callee) 
       
       if ((reg->refCount > 0) || 
           reg->keptValue ||
           (reg->liveState == registerSlot::live)) {
          saveRegister(gen, reg->number, FUNC_CALL_SAVE);
          savedRegs.push_back(reg->number);
       }
    }

    // Generate the code for all function parameters, and keep a list
    // of what registers they're in.
    for (unsigned u = 0; u < operands.size(); u++) {
/*
	if (operands[u]->getSize() == 8) {
	    // What does this do?
	    bperr( "in weird code\n");
	    Register dummyReg = gen.rs()->allocateRegister(gen, noCost);
	    srcs.push_back(dummyReg);

	    instruction::generateImm(gen, CALop, dummyReg, 0, 0);
	}
*/
	//Register src = REG_NULL;
        // Try to target the code generation
        
        Register reg = REG_NULL;
        // Try to allocate the correct parameter register
        if (gen.rs()->allocateSpecificRegister(gen, registerSpace::r3 + u, true))
            reg = registerSpace::r3 + u;

	Address unused = ADDR_NULL;
	if (!operands[u]->generateCode_phase2( gen, false, unused, reg)) assert(0);
	assert(reg != REG_NULL);
	srcs.push_back(reg);
	//bperr( "Generated operand %d, base %d\n", u, base);
    }
  
    if(srcs.size() > 8) {
	// This is not necessarily true; more then 8 arguments could be passed,
	// the first 8 need to be in registers while the others need to be on
	// the stack, -- sec 3/1/97
	pdstring msg = "Too many arguments to function call in instrumentation code:"
	    " only 8 arguments can (currently) be passed on the POWER architecture.\n";
	bperr( msg.c_str());
	showErrorCallback(94,msg);
	cleanUpAndExit(-1);
    }

    // If we got the wrong register, we may need to do a 3-way swap. 

    int scratchReg[8];
    for (int a = 0; a < 8; a++) {
	scratchReg[a] = -1;
    }

    // Now load the parameters into registers.
    for (unsigned u=0; u<srcs.size(); u++){

        // Parameters start at register 3 - so we're already done
        // in this case
	if (srcs[u] == (registerSpace::r3+u)) {
	    gen.rs()->freeRegister(srcs[u]);
	    continue;
	}

	int whichSource = -1;
	bool hasSourceBeenCopied = true;
        

        // If the parameter we want exists in a scratch register...
	if (scratchReg[u] != -1) {
	    instruction::generateImm(gen, ORILop, scratchReg[u], u+3, 0);
	    gen.rs()->freeRegister(scratchReg[u]);
            // We should check to make sure the one we want isn't occupied?
	} else {
	    for (unsigned v=u; v < srcs.size(); v++) {
		if (srcs[v] == u+3) {
                    // Okay, so the source we want is actuall in srcs[v]
		    hasSourceBeenCopied = false;
		    whichSource = v;
                    break;
		}
	    }
            // Ummm... we didn't find it? Ah, so copying us (since we're wrong)
            // into scratch.
	    if (!hasSourceBeenCopied) {
                Register scratch = gen.rs()->getScratchRegister(gen);
		instruction::generateImm(gen, ORILop, u+3, scratch, 0);
		gen.rs()->freeRegister(u+3);
		scratchReg[whichSource] = scratch;
		hasSourceBeenCopied = true;

		instruction::generateImm(gen, ORILop, srcs[u], u+3, 0);
		gen.rs()->freeRegister(srcs[u]);

	    } else {
		instruction::generateImm(gen, ORILop, srcs[u], u+3, 0);
		gen.rs()->freeRegister(srcs[u]);
                // Not sure why this was needed
		//gen.rs()->clobberRegister(u+3);
	    }
	} 
    }

    if (toc_anchor) {
        // Set up the new TOC value
        emitVload(loadConstOp, toc_anchor, 2, 2, gen, false);
        //inst_printf("toc setup (%d)...");
    }

    // generate a branch to the subroutine to be called.
    // load r0 with address, then move to link reg and branch and link.

    emitVload(loadConstOp, callee->getAddress(), 0, 0, gen, false);
    //inst_printf("addr setup (%d)....");
  
    // Move to link register
    instruction mtlr0(MTLR0raw);
    mtlr0.generate(gen);
   
    //inst_printf("mtlr0 (%d)...");

    // brl - branch and link through the link reg.

    instruction brl(BRLraw);
    brl.generate(gen);

    // get a register to keep the return value in.
    Register retReg = gen.rs()->allocateRegister(gen, noCost);

    // put the return value from register 3 to the newly allocated register.
    instruction::generateImm(gen, ORILop, 3, retReg, 0);

    // restore saved registers.
    for (u_int ui = 0; ui < savedRegs.size(); ui++) {
	restoreRegister(gen, savedRegs[ui], FUNC_CALL_SAVE);
    }
  
    // mtlr	0 (aka mtspr 8, rs) = 0x7c0803a6
    // Move to link register
    // Reused from above. instruction mtlr0(MTLR0raw);
    mtlr0.generate(gen);
    /*
      gen = (instruction *) gen;
      for (unsigned foo = initBase/4; foo < base/4; foo++)
      bperr( "0x%x,\n", gen[foo].raw);
    */
    // return value is the register with the return value from the called function
    return(retReg);
}

 
codeBufIndex_t emitA(opCode op, Register src1, Register /*src2*/, Register dest,
	      codeGen &gen, bool /*noCost*/)
{
    //bperr("emitA(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);
    codeBufIndex_t retval = 0;
    switch (op) {
      case ifOp: {
	// cmpi 0,0,src1,0
          instruction insn;
          (*insn).raw = 0;
          (*insn).dform.op = CMPIop;
          (*insn).dform.ra = src1;
          (*insn).dform.d_or_si = 0;
          insn.generate(gen);
          retval = gen.getIndex();
          
          // be 0, dest
          (*insn).raw = 0;
          (*insn).bform.op = BCop;
          (*insn).bform.bo = BTRUEcond;
          (*insn).bform.bi = EQcond;
          (*insn).bform.bd = dest/4;
          (*insn).bform.aa = 0;
          (*insn).bform.lk = 0;
          
          insn.generate(gen);
          break;
      }
    case branchOp: {
        retval = gen.getIndex();
        instruction::generateBranch(gen, dest);
        break;
    }
    case trampPreamble: {
        // nothing to do in this platform
        return(0);              // let's hope this is expected!
    }        
    case trampTrailer: {
        retval = gen.getIndex();
        unsigned addr_width = gen.addrSpace()->getAddressWidth();
        gen.fill(instruction::maxJumpSize(addr_width), codeGen::cgNOP);
        instruction::generateIllegal(gen);
        break;
      }
    default:
        assert(0);        // unexpected op for this emit!
    }
    return retval;
}

Register emitR(opCode op, Register src1, Register /*src2*/, Register dest,
               codeGen &gen, bool /*noCost*/,
               const instPoint * /*location*/, bool /*for_MT*/)
{
    //bperr("emitR(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

    registerSlot *regSlot = NULL;

    switch (op) {
    case getRetValOp: {
	regSlot = (*(gen.rs()))[registerSpace::r3];
        break;
    }
    case getParamOp:
      // the first 8 parameters (0-7) are stored in registers (r3-r10) upon entering
      // the function and then saved above the stack upon entering the trampoline;
      // in emit functional call the stack pointer is moved so the saved registers
      // are not over-written
      // the other parameters > 8 are stored on the caller's stack at an offset.
      // 
      // src1 is the argument number 0..X, the first 8 are stored in registers
      // r3 and 
      if(src1 < 8) {
          // Note that src1 is 0..8 - it's not a register, it's a parameter number
          regSlot = (*(gen.rs()))[registerSpace::r3 + src1];
          break;

      } else {
          // Registers from 11 (src = 8) and beyond are saved on the
          // stack. On AIX this is +56 bytes; for ELF it's something different.
	  if (gen.addrSpace()->getAddressWidth() == 4) {
	      instruction::generateImm(gen, Lop, dest, 1,
				       TRAMP_FRAME_SIZE_32 +
				       ((src1 - 8)*sizeof(int)) +
				       PARAM_OFFSET(gen.addrSpace()->getAddressWidth()));
	  } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
	      instruction::generateMemAccess64(gen, LDop, LDxop, dest, 1,
					       TRAMP_FRAME_SIZE_64 +
					       ((src1 - 8)*sizeof(long)) +
					       PARAM_OFFSET(gen.addrSpace()->getAddressWidth()));
	  }
          return(dest);
      }
      break;
    default:
        assert(0);
        break;
    }

    assert(regSlot);
    Register reg = regSlot->number;

    switch(regSlot->liveState) {
    case registerSlot::spilled: {
        int offset;
        if (gen.addrSpace()->getAddressWidth() == 4)
            offset = TRAMP_GPR_OFFSET_32;
        else /* gen.addrSpace()->getAddressWidth() == 8 */
            offset = TRAMP_GPR_OFFSET_64;
        // its on the stack so load it.
        restoreRegister(gen, reg, dest, offset);
        return(dest);
    }
    case registerSlot::live: {
        // its still in a register so return the register it is in.
        return(reg);
    }
    case registerSlot::dead: {
        // Uhhh... wha?
        assert(0);
    }
    }
    assert(0);
    return REG_NULL;
}

void emitJmpMC(int /*condition*/, int /*offset*/, codeGen &)
{
  // Not needed for memory instrumentation, otherwise TBD
}


// VG(11/16/01): Say if we have to restore a register to get its original value
// VG(03/15/02): Sync'd with the new AIX tramp
static inline bool needsRestore(Register x)
{
  //return (x == 0) || ((x >= 3) && (x <= 12)) || (x == POWER_XER2531);
  return ((x <= 12) && !(x==2)) || (x == POWER_XER2531);
}

// VG(03/15/02): Restore mutatee value of GPR reg to dest GPR
static inline void restoreGPRtoGPR(codeGen &gen,
                                   Register reg, Register dest)
{
    int frame_size, gpr_size, gpr_off;
    if (gen.addrSpace()->getAddressWidth() == 4) {
	frame_size = TRAMP_FRAME_SIZE_32;
	gpr_size   = GPRSIZE_32;
	gpr_off    = TRAMP_GPR_OFFSET_32;
    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
	frame_size = TRAMP_FRAME_SIZE_64;
	gpr_size   = GPRSIZE_64;
	gpr_off    = TRAMP_GPR_OFFSET_64;
    }

    if (reg == 1) // SP is in a different place, but we don't need to
                  // restore it, just subtract the stack frame size
        instruction::generateImm(gen, CALop, dest, REG_SP, frame_size);

    else if((reg == 0) || ((reg >= 3) && (reg <=12)))
        instruction::generateImm(gen, Lop, dest, 1, gpr_off + reg*gpr_size);

    else {
        bperr( "GPR %d should not be restored...", reg);
        assert(0);
    }
    //bperr( "Loading reg %d (into reg %d) at 0x%x off the stack\n", 
    //  reg, dest, offset + reg*GPRSIZE);
}

// VG(03/15/02): Restore mutatee value of XER to dest GPR
static inline void restoreXERtoGPR(codeGen &gen, Register dest)
{
    if (gen.addrSpace()->getAddressWidth() == 4) {
        instruction::generateImm(gen, Lop, dest, 1,
                                 TRAMP_SPR_OFFSET + STK_XER_32);
    } else /* gen.addrSpace()->getAddressWidth() == 8 */ {
        instruction::generateMemAccess64(gen, LDop, LDxop, dest, 1,
                                         TRAMP_SPR_OFFSET + STK_XER_64);
    }
}

// VG(03/15/02): Move bits 25:31 of GPR reg to GPR dest
static inline void moveGPR2531toGPR(codeGen &gen,
                                    Register reg, Register dest)
{
  // keep only bits 32+25:32+31; extrdi dest, reg, 7 (n bits), 32+25 (start at b)
  // which is actually: rldicl dest, reg, 32+25+7 (b+n), 64-7 (64-n)
  // which is the same as: clrldi dest,reg,57 because 32+25+7 = 64
    instruction rld;
    (*rld).raw = 0;
    (*rld).mdform.op  = RLDop;
    (*rld).mdform.rs  = reg;
    (*rld).mdform.ra  = dest;
    (*rld).mdform.sh  = 0;  //(32+25+7) % 32;
    (*rld).mdform.mb  = (64-7) % 32;
    (*rld).mdform.mb2 = (64-7) / 32;
    (*rld).mdform.xo  = ICLxop;
    (*rld).mdform.sh2 = 0; //(32+25+7) / 32;
    (*rld).mdform.rc  = 0;
    rld.generate(gen);
}

// VG(11/16/01): Emit code to add the original value of a register to
// another. The original value may need to be restored from stack...
// VG(03/15/02): Made functionality more obvious by adding the above functions
static inline void emitAddOriginal(Register src, Register acc, 
                                   codeGen &gen, bool noCost)
{
    bool nr = needsRestore(src);
    Register temp;
    
    if(nr) {
        // this needs gen because it uses emitV...
        temp = gen.rs()->allocateRegister(gen, noCost);
        
        // Emit code to restore the original ra register value in temp.
        // The offset compensates for the gap 0, 3, 4, ...
        // This writes at insn, and updates insn and base.
        
        if(src == POWER_XER2531) { // hack for XER_25:31
            //bperr( "XER_25:31\n");
            restoreXERtoGPR(gen, temp);
            moveGPR2531toGPR(gen, temp, temp);
        }
        else
            restoreGPRtoGPR(gen, src, temp);
    }
    else
        temp = src;
    
    // add temp to dest;
    // writes at gen+base and updates base, we must update insn...
    emitV(plusOp, temp, acc, acc, gen, noCost, 0);
    
    if(nr)
        gen.rs()->freeRegister(temp);
}

// VG(11/07/01): Load in destination the effective address given
// by the address descriptor. Used for memory access stuff.
void emitASload(const BPatch_addrSpec_NP *as, Register dest, codeGen &gen,
		bool noCost)
{
  //instruction *insn = (instruction *) ((void*)&gen[base]);
  int imm = as->getImm();
  int ra  = as->getReg(0);
  int rb  = as->getReg(1);
  // TODO: optimize this to generate the minimum number of
  // instructions; think about schedule

  // emit code to load the immediate (constant offset) into dest; this
  // writes at gen+base and updates base, we must update insn...
  emitVload(loadConstOp, (Address)imm, dest, dest, gen, noCost);
  
  // If ra is used in the address spec, allocate a temp register and
  // get the value of ra from stack into it
  if(ra > -1)
      emitAddOriginal(ra, dest, gen, noCost);

  // If rb is used in the address spec, allocate a temp register and
  // get the value of ra from stack into it
  if(rb > -1)
    emitAddOriginal(rb, dest, gen, noCost);

}

void emitCSload(const BPatch_addrSpec_NP *as, Register dest, codeGen &gen,               
		bool noCost)
{
    emitASload(as, dest, gen, noCost);
}

void emitVload(opCode op, Address src1, Register /*src2*/, Register dest,
               codeGen &gen, bool /*noCost*/, 
               registerSpace * /*rs*/, int size,
               const instPoint * /* location */, AddressSpace *proc)
{
    if (op == loadConstOp) {
        instruction::loadImmIntoReg(gen, dest, (long)src1);

    } else if (op == loadOp) {
        instruction::loadPartialImmIntoReg(gen, dest, (long)src1);

	// really load dest, (dest)imm
        if (size == 1)
            instruction::generateImm(gen, LBZop, dest, dest, LOW(src1));
        else if (size == 2)
            instruction::generateImm(gen, LHZop, dest, dest, LOW(src1));
        else if ((size == 4) ||
		 (size == 8 && proc->getAddressWidth() == 4)) // Override bogus size
            instruction::generateImm(gen, Lop,   dest, dest, LOW(src1));
        else if (size == 8)
            instruction::generateMemAccess64(gen, LDop, LDxop,
                                             dest, dest, (int16_t)LOW(src1));
        else assert(0 && "Incompatible loadOp size");

    } else if (op == loadFrameRelativeOp) {
	long offset = (long)src1;
	if (gen.addrSpace()->getAddressWidth() == 4)
	    offset += TRAMP_FRAME_SIZE_32;
	else /* gen.addrSpace()->getAddressWidth() == 8 */
	    offset += TRAMP_FRAME_SIZE_64;

	// return the value that is FP offset from the original fp
	if (size == 1)
	    instruction::generateImm(gen, LBZop, dest, REG_SP, offset);
	else if (size == 2)
	    instruction::generateImm(gen, LHZop, dest, REG_SP, offset);
	else if ((size == 4) ||
		 (size == 8 && proc->getAddressWidth() == 4)) // Override bogus size
	    instruction::generateImm(gen, Lop,   dest, REG_SP, offset);
	else if (size == 8)
	    instruction::generateMemAccess64(gen, LDop, LDxop,
					     dest, REG_SP, offset);
	else assert(0 && "Incompatible loadFrameRelativeOp size");

    } else if (op == loadFrameAddr) {
	// offsets are signed!
        long offset = (long)src1;
        offset += (gen.addrSpace()->getAddressWidth() == 4 ? TRAMP_FRAME_SIZE_32
                                                      : TRAMP_FRAME_SIZE_64);

        if (offset < MIN_IMM16 || MAX_IMM16 < offset) assert(0);
        instruction::generateImm(gen, CALop, dest, REG_SP, offset);

    } else {
        assert(0);
    }
}

void emitVstore(opCode op, Register src1, Register /*src2*/, Address dest,
		codeGen &gen, bool noCost, 
                registerSpace * /* rs */, int size,
                const instPoint * /* location */, AddressSpace *proc)
{
    if (op == storeOp) {
	// temp register to hold base address for store (added 6/26/96 jkh)
	Register temp = gen.rs()->getScratchRegister(gen, noCost);

        instruction::loadPartialImmIntoReg(gen, temp, (long)dest);
        if (size == 1)
            instruction::generateImm(gen, STBop, src1, temp, LOW(dest));
        else if (size == 2)
            instruction::generateImm(gen, STHop, src1, temp, LOW(dest));
        else if ((size == 4) ||
		 (size == 8 && proc->getAddressWidth() == 4)) // Override bogus size
            instruction::generateImm(gen, STop,  src1, temp, LOW(dest));
        else if (size == 8)
            instruction::generateMemAccess64(gen, STDop, STDxop,
                                             src1, temp, (int16_t)BOT_LO(dest));
        else assert(0 && "Incompatible storeOp size");

    } else if (op == storeFrameRelativeOp) {
        long offset = (long)dest;
        offset += (gen.addrSpace()->getAddressWidth() == 4
		   ? TRAMP_FRAME_SIZE_32
		   : TRAMP_FRAME_SIZE_64);

        if (size == 1)
            instruction::generateImm(gen, STBop, src1, REG_SP, offset);
        else if (size == 2)
            instruction::generateImm(gen, STHop, src1, REG_SP, offset);
        else if ((size == 4) ||
		 (size == 8 || proc->getAddressWidth() == 4)) // Override bogus size
            instruction::generateImm(gen, STop,  src1, REG_SP, offset);
        else if (size == 8)
            instruction::generateMemAccess64(gen, STDop, STDxop, src1,
                                             REG_SP, offset);
        else assert(0 && "Incompatible storeFrameRelativeOp size");

    } else assert(0 && "Unknown op passed to emitVstore");
}

void emitV(opCode op, Register src1, Register src2, Register dest,
           codeGen &gen, bool /*noCost*/,
           registerSpace * /*rs*/, int size,
           const instPoint * /* location */, AddressSpace *proc)
{
    //bperr("emitV(op=%d,src1=%d,src2=%d,dest=%d)\n",op,src1,src2,dest);

    assert ((op!=branchOp) && (op!=ifOp) && 
            (op!=trampTrailer) && (op!=trampPreamble));         // !emitA
    assert ((op!=getRetValOp) && (op!=getParamOp));             // !emitR
    assert ((op!=loadOp) && (op!=loadConstOp));                 // !emitVload
    assert ((op!=storeOp));                                     // !emitVstore
    assert ((op!=updateCostOp));                                // !emitVupdate

    instruction insn;

    if (op == loadIndirOp) {
       // really load dest, (dest)imm
       assert(src2 == 0); // Since we don't use it.
       if (!size)
          size = proc->getAddressWidth();
       if (size == 1)
          instruction::generateImm(gen, LBZop, dest, src1, 0);
       else if (size == 2)
          instruction::generateImm(gen, LHZop, dest, src1, 0);
       else if ((size == 4) ||
                (size == 8 && proc->getAddressWidth() == 4)) // Override bogus size
          instruction::generateImm(gen, Lop,   dest, src1, 0);
       else if (size == 8) {
          instruction::generateMemAccess64(gen, LDop, LDxop,
                                           dest, src1, 0);
       } 
       else 
          assert(0 && "Incompatible loadOp size");
    } else if (op == storeIndirOp) {
        // generate -- st src1, dest
        if (size == 1)
            instruction::generateImm(gen, STBop, src1, dest, 0);
        else if (size == 2)
            instruction::generateImm(gen, STHop, src1, dest, 0);
        else if ((size == 4) ||
		 (size == 8 && proc->getAddressWidth() == 4)) // Override bogus size
            instruction::generateImm(gen, STop,  src1, dest, 0);
        else if (size == 8)
            instruction::generateMemAccess64(gen, STDop, STDxop,
                                             src1, dest, 0);
        else assert(0 && "Incompatible storeOp size");

    } else if (op == noOp) {
        instruction::generateNOOP(gen);

    } else if (op == saveRegOp) {
        saveRegister(gen,src1,8);

    } else {
        int instXop=-1;
        int instOp=-1;
        switch (op) {
            // integer ops
            case plusOp:
                instOp = CAXop;
                instXop = CAXxop;
                break;

            case minusOp:
                Register temp;
                // need to flip operands since this computes ra-rb not rb-ra
                temp = src1;
                src1 = src2;
                src2 = temp;
                instOp = SFop;
                instXop = SFxop;
                break;

            case timesOp:
                instOp = MULSop;
                instXop = MULSxop;
                break;

            case divOp:
                instOp = DIVSop;   // POWER divide instruction
                                   // Same as DIVWop for PowerPC
#if defined(os_aix)                // Should use runtime CPU detection ...
		if (gen.addrSpace()->getAddressWidth() == 8)
		    instXop = DIVWxop; // divs instruction deleted on 64-bit
		else
		    instXop = DIVSxop;
#else
                instXop = DIVWxop; // PowerPC
#endif
                break;

            // Bool ops
            case orOp:
                //genSimple(gen, ORop, src1, src2, dest);
                (*insn).raw = 0;
                (*insn).xoform.op = ORop;
                // operation is ra <- rs | rb (or rs,ra,rb)
                (*insn).xoform.ra = dest;
                (*insn).xoform.rt = src1;
                (*insn).xoform.rb = src2;
                (*insn).xoform.xo = ORxop;
                insn.generate(gen);
                return;
                break;

            case andOp:
                //genSimple(gen, ANDop, src1, src2, dest);
                // This is a Boolean and with true == 1 so bitwise is OK
                (*insn).raw = 0;
                (*insn).xoform.op = ANDop;
                // operation is ra <- rs & rb (and rs,ra,rb)
                (*insn).xoform.ra = dest;
                (*insn).xoform.rt = src1;
                (*insn).xoform.rb = src2;
                (*insn).xoform.xo = ANDxop;
                insn.generate(gen);
                return;
                break;

            // rel ops
            case eqOp:
                instruction::generateRelOp(gen, EQcond, BTRUEcond, src1, src2, dest);
                return;
                break;

            case neOp:
                instruction::generateRelOp(gen, EQcond, BFALSEcond, src1, src2, dest);
                return;
                break;

            case lessOp:
                instruction::generateRelOp(gen, LTcond, BTRUEcond, src1, src2, dest);
                return;
                break;

            case greaterOp:
                instruction::generateRelOp(gen, GTcond, BTRUEcond, src1, src2, dest);
                return;
                break;

            case leOp:
                instruction::generateRelOp(gen, GTcond, BFALSEcond, src1, src2, dest);
                return;
                break;

            case geOp:
                instruction::generateRelOp(gen, LTcond, BFALSEcond, src1, src2, dest);
                return;
                break;

            default:
                // internal error, invalid op.
                bperr( "Invalid op passed to emit, instOp = %d\n", instOp);
                assert(0 && "Invalid op passed to emit");
                break;
        }

        
        assert((instOp != -1) && (instXop != -1));
        (*insn).raw = 0;
        (*insn).xoform.op = instOp;
        (*insn).xoform.rt = dest;
        (*insn).xoform.ra = src1;
        (*insn).xoform.rb = src2;
        (*insn).xoform.xo = instXop;
        insn.generate(gen);
    }
  return;
}


//
// I don't know how to compute cycles for POWER instructions due to 
//   multiple functional units.  However, we can compute the number of
//   instructions and hope that is fairly close. - jkh 1/30/96
//
int getInsnCost(opCode op)
  {
    int cost = 0;

    /* XXX Need to add branchOp */
    switch (op) {
      
    case noOp:
    case loadIndirOp:
    case saveRegOp:

      // worse case is it is on the stack and takes one instruction.
    case getParamOp:

      // integer ops
    case plusOp:
    case minusOp:
    case timesOp:
    case divOp:
      cost = 1;
      break;

    case loadConstOp:
      // worse case is addi followed by ori

    case loadOp:
      // addis, l
      
    case storeOp:
    case storeIndirOp:
      cost = 2;
      break;

    case ifOp:
      // cmpi 0,0,src1,0
      // be 0, dest
      // nop
      cost = 3;
      break;

      // rel ops
    case eqOp:
    case neOp:
    case lessOp:
    case greaterOp:
    case leOp:
    case geOp:
      cost = 4;
      break;
      
    case callOp:
      // mflr r0
      // st r0, (r1)
      cost += 2;
      
      // Should compute the cost to save registers here.  However, we lack 
      //   sufficient information to compute this value. We need to be 
      //   inside the code generator to know this amount.
      //
      // We know it is at *least* every live register (i.e. parameter reg)
      cost += 13;
      
      // clr r5
      // clr r6
      // decrement stack pointer -- STUop
      // load r0 with address, then move to link reg and branch and link.
      // ori dest,dest,LOW(src1)
      // mtlr	0 (aka mtspr 8, rs) = 0x7c0803a6
      // brl - branch and link through the link reg.
      cost += 7;
      
      // now cleanup.
      
      // increment stack pointer -- CALop
      // restore the saved register 0.
      cost += 2;
      
      // Should compute the cost to restore registers here.  However, we lack 
      //   sufficient information to compute this value. We need to be 
      //   inside the code generator to know this amount.
      //
      // We know it is at *least* every live register (i.e. parameter reg)
      cost += 13;
      
      // mtlr	0 
      cost++;
      break;

    case updateCostOp:
      // In most cases this cost is 4, but if we can't use CALop because
      // the value is to large, then we'll need 2 additional operations to
      // load that value - naim
      // Why the +=? Isn't it equivalent to an = here? -- bernat
      cost += 4;
      break;
      
    case trampPreamble:
      // Generate code to update the observed cost.
      // generated code moved to the base trampoline.
      cost += 0;
      break;

    case trampTrailer:
      // Should compute the cost to restore registers here.  However, we lack 
      //   sufficient information to compute this value. We need to be 
      //   inside the code generator to know this amount.
      //
      
      // branch
      // nop
      cost += 2;
      break;

    default:
      cost = 0;
      break;
    }
    return (cost);
}

#if 0
// What does this do???
void registerSpace::saveClobberInfo(const instPoint *location)
{
  registerSlot *regSlot = NULL;
  registerSlot *regFPSlot = NULL;
  if (location == NULL)
    return;
  if (location->actualGPRLiveSet_ != NULL && location->actualFPRLiveSet_ != NULL)
    {
      
      // REG guard registers, if live, must be saved
      if (location->actualGPRLiveSet_[ REG_GUARD_ADDR ] == LIVE_REG)
	location->actualGPRLiveSet_[ REG_GUARD_ADDR ] = LIVE_CLOBBERED_REG;
      
      if (location->actualGPRLiveSet_[ REG_GUARD_OFFSET ] == LIVE_REG)
	location->actualGPRLiveSet_[ REG_GUARD_OFFSET ] = LIVE_CLOBBERED_REG;

      // GPR and FPR scratch registers, if live, must be saved
      if (location->actualGPRLiveSet_[ REG_SCRATCH ] == LIVE_REG)
	location->actualGPRLiveSet_[ REG_SCRATCH ] = LIVE_CLOBBERED_REG;

      if (location->actualFPRLiveSet_[ REG_SCRATCH ] == LIVE_REG)
	location->actualFPRLiveSet_[ REG_SCRATCH ] = LIVE_CLOBBERED_REG;

      // Return func call register, since we make a call because
      // of multithreading (regardless if it's threaded) from BT
      // we must save return register
      if (location->actualGPRLiveSet_[ 3 ] == LIVE_REG)
	location->actualGPRLiveSet_[ 3 ] = LIVE_CLOBBERED_REG;

    
      for (u_int i = 0; i < getRegisterCount(); i++)
	{
	  regSlot = getRegSlot(i);

	  if (  location->actualGPRLiveSet_[ (int) registers[i].number ] == LIVE_REG )
	    {
	      if (!registers[i].beenClobbered)
		location->actualGPRLiveSet_[ (int) registers[i].number ] = LIVE_UNCLOBBERED_REG;
	      else
		location->actualGPRLiveSet_[ (int) registers[i].number ] = LIVE_CLOBBERED_REG;
	    }


	  if (  location->actualGPRLiveSet_[ (int) registers[i].number ] == LIVE_UNCLOBBERED_REG ) 
	    {
	      if (registers[i].beenClobbered)
		location->actualGPRLiveSet_[ (int) registers[i].number ] = LIVE_CLOBBERED_REG;
	    }
	}
	  
      for (u_int i = 0; i < getFPRegisterCount(); i++)
	{
	  regFPSlot = getFPRegSlot(i);
	  
	  if (  location->actualFPRLiveSet_[ (int) fpRegisters[i].number ] == LIVE_REG )
	    {
	      if (!fpRegisters[i].beenClobbered)
		location->actualFPRLiveSet_[ (int) fpRegisters[i].number ] = LIVE_UNCLOBBERED_REG;
	      else
		location->actualFPRLiveSet_[ (int) fpRegisters[i].number ] = LIVE_CLOBBERED_REG;
	    }
	  
	  if (  location->actualFPRLiveSet_[ (int) fpRegisters[i].number ] == LIVE_UNCLOBBERED_REG )
	    {
	      if (fpRegisters[i].beenClobbered)
		location->actualFPRLiveSet_[ (int) fpRegisters[i].number ] = LIVE_CLOBBERED_REG;
	    }
	}
    }
}
#endif


bool doNotOverflow(int value)
{
  // we are assuming that we have 15 bits to store the immediate operand.
  if ( (value <= 32767) && (value >= -32768) ) return(true);
  else return(false);
}

// hasBeenBound: returns false
// On AIX (at least what we handle so far), all symbols are bound
// at load time. This kind of obviates the need for a hasBeenBound
// function: of _course_ the symbol has been bound. 
// So the idea of having a relocation entry for the function doesn't
// quite make sense. Given the target address, we can scan the function
// lists until we find the desired function.

bool process::hasBeenBound(const relocationEntry &,int_function *&, Address ) {
  // What needs doing:
  // Locate call instruction
  // Decipher call instruction (static/dynamic call, global linkage code)
  // Locate target
  // Lookup target
  return false; // Haven't patched this up yet
}

// Emit code to jump to function CALLEE without linking.  (I.e., when
// CALLEE returns, it returns to the current caller.)

// 18FEB00 -- I removed the parameter names to get rid of compiler
// warnings. They are copied below.
// opCode op, codeGen &gen, const int_function *callee, AddressSpace *proc

void emitFuncJump(opCode              op, 
                  codeGen            &gen,
                  const int_function *func,
                  AddressSpace       * /*proc*/,
                  const instPoint    *point,
                  bool)
{
    // Performs the following steps:
    // 1) Unwinds the base tramp that we're in; equivalent to generateRestores.
    // 2) Generates a jump to a new function. We don't call, since we're 
    //    not planning on returning to where we are. 

    assert (op == funcJumpOp);
    assert (point);

    // Leave base tramp. Functional equivalent of baseTramp::generateRestores.
    // Assume we're pre-tramp instrumentation for the instPoint...
    baseTramp *tramp = point->preBaseTramp();
    if (!tramp) {
        tramp = point->postBaseTramp();
    }
    if (!tramp) {
        tramp = point->targetBaseTramp();
    }

    // Better have one by now...
    assert(tramp);
    
    // Generate restores
    // Good thing for us the registerSpace isn't used. Because we don't 
    // have one (???)
    tramp->generateRestores(gen, NULL);
    
    Address fromAddr = gen.currAddr();
    Address toAddr = func->getAddress();

    instruction::generateInterFunctionBranch(gen,
                                             fromAddr,
                                             toAddr);
    return;
}

// atch AIX register numbering (sys/reg.h)
#define REG_LR  131
#define REG_CTR 132

void emitLoadPreviousStackFrameRegister(Address register_num, 
                                        Register dest,
                                        codeGen &gen,
                                        int /*size*/,
                                        bool noCost)
{
    // As of 10/24/2007, the size parameter is still incorrect.
    // Luckily, we know implicitly what size they actually want.

    // Offset if needed
    int offset;
    // Unused, 3OCT03
    //instruction *insn_ptr = (instruction *)insn;
    // We need values to define special registers.
    switch ( (int) register_num) {
    case REG_LR:
        // LR is saved on the stack
        // Note: this is only valid for non-function entry/exit instru. 
        // Once we've entered a function, the LR is stomped to point
        // at the exit tramp!
        offset = TRAMP_SPR_OFFSET + STK_LR; 

        // Get address (SP + offset) and stick in register dest.
        emitImm(plusOp ,(Register) REG_SP, (RegValue) offset, dest,
                gen, noCost, gen.rs());
        // Load LR into register dest
        emitV(loadIndirOp, dest, 0, dest, gen, noCost, gen.rs(),
              gen.addrSpace()->getAddressWidth(), gen.point(), gen.addrSpace());
        break;

    case REG_CTR:
        // CTR is saved down the stack
        if (gen.addrSpace()->getAddressWidth() == 4)
            offset = TRAMP_SPR_OFFSET + STK_CTR_32;
        else
            offset = TRAMP_SPR_OFFSET + STK_CTR_64;

        // Get address (SP + offset) and stick in register dest.
        emitImm(plusOp ,(Register) REG_SP, (RegValue) offset, dest,
                gen, noCost, gen.rs());
        // Load LR into register dest
        emitV(loadIndirOp, dest, 0, dest, gen, noCost, gen.rs(),
              gen.addrSpace()->getAddressWidth(), gen.point(), gen.addrSpace());
      break;

    default:
        cerr << "Fallthrough in emitLoadPreviousStackFrameRegister" << endl;
        cerr << "Unexpected register " << register_num << endl;
        abort();
        break;
    }
}

bool AddressSpace::getDynamicCallSiteArgs(instPoint *callSite,
                                    pdvector<AstNodePtr> &args)
{

    const instruction &i = callSite->insn();
    Register branch_target;

    // Is this a branch conditional link register (BCLR)
    // BCLR uses the xlform (6,5,5,5,10,1)
    if((*i).xlform.op == BCLRop) // BLR/BCR, or bcctr/bcc. Same opcode.
        {
            if ((*i).xlform.xo == BCLRxop) // BLR (bclr)
                {
                    //bperr( "Branch target is the link register\n");
                    branch_target = REG_LR;
                }
            else if ((*i).xlform.xo == BCCTRxop)
                {
                    // We handle global linkage branches (BCTR) as static call
                    // sites. They're currently registered when the static call
                    // graph is built (Paradyn), after all objects have been read
                    // and parsed.
                    //bperr( "Branch target is the count register\n");
                    branch_target = REG_CTR;
                }
            else
                {
                    // Used to print an error, but the opcode (19) is also used
                    // for other instructions, and errors could confuse people.
                    // So just return false instead.
                    return false;
                }


            // Where we're jumping to (link register, count register)
            args.push_back( AstNode::operandNode(AstNode::PreviousStackFrameDataReg,
                                                 (void *) branch_target));

            // Where we are now
            args.push_back( AstNode::operandNode(AstNode::Constant,
                                                 (void *) callSite->addr()));

            return true;
        }
    else if ((*i).xlform.op == Bop) {
        /// Why didn't we catch this earlier? In any case, don't print an error
        
        // I have seen this legally -- branches to the FP register saves.
        // Since we ignore the save macros, we have no idea where the branch
        // goes. For now, return true -- means no error.
        
        //return true;
        
        //  since we do not fill in args array, return false ??
        return false;
    }
    else
        {
            cerr << "MonitorCallSite: Unknown opcode " << (*i).xlform.op << endl;
            cerr << "opcode extension: " << (*i).xlform.xo << endl;
            bperr( "Address is 0x%x, insn 0x%x\n",
                   callSite->addr(),
                   (*i).raw);
            return false;
        }
}

bool writeFunctionPtr(AddressSpace *p, Address addr, int_function *f)
{
#if defined(os_aix)
    Address buffer[3];
    Address val_to_write = f->getAddress();
    Address toc = p->proc()->getTOCoffsetInfo(val_to_write);
    buffer[0] = val_to_write;
    buffer[1] = toc;
    buffer[2] = 0x0;

    if (!p->writeDataSpace((void *) addr, sizeof(buffer), buffer))
        fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
    return true;
#else
    // 64-bit ELF PowerPC Linux uses r2 (same as AIX) for TOC base register
    if (p->getAddressWidth() == sizeof(uint64_t)) {
        Address buffer[3];
        Address val_to_write = f->getAddress();
        assert(p->proc());
        Address toc = p->proc()->getTOCoffsetInfo(val_to_write);
        buffer[0] = val_to_write;
        buffer[1] = toc;
        buffer[2] = 0x0;

        if (!p->writeDataSpace((void *) addr, sizeof(buffer), buffer))
            fprintf(stderr, "%s[%d]:  writeDataSpace failed\n",
                            FILE__, __LINE__);
        return true;
    }
    else {
        // Originally copied from inst-x86.C
        // 32-bit ELF PowerPC Linux mutatee
        uint32_t val_to_write = (uint32_t)f->getAddress();
        return p->writeDataSpace((void *) addr,
                                 sizeof(val_to_write), &val_to_write);
    }
#endif
}


Emitter *AddressSpace::getEmitter() 
{
    static EmitterPOWER32Dyn emitter32Dyn;
    static EmitterPOWER64Dyn emitter64Dyn;
    static EmitterPOWER32Stat emitter32Stat;
    static EmitterPOWER64Stat emitter64Stat;

    if (getAddressWidth() == 8) {
        if (proc()) {
            return &emitter64Dyn;
        }
        else return &emitter64Stat;
    }
    if (proc())
        return &emitter32Dyn;
    else
        return &emitter32Stat;

    assert(0);
    return NULL;
}

Register EmitterPOWER32Stat::emitCall(opCode, codeGen &,
                                      const pdvector<AstNodePtr> &,
                                      bool, int_function *) 
{ 
   assert(0); 
   return 0;
}

Register EmitterPOWER64Stat::emitCall(opCode, codeGen &,
                                      const pdvector<AstNodePtr> &,
                                      bool, int_function *) 
{ 
   assert(0); 
   return 0; 
}

