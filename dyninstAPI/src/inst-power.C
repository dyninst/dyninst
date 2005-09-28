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
 * $Id: inst-power.C,v 1.233 2005/09/28 17:03:06 bernat Exp $
 */

#include "common/h/headers.h"

#ifndef BPATCH_LIBRARY
#include "rtinst/h/rtinst.h"
#else
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#endif
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/inst-power.h"
#include "dyninstAPI/src/arch-power.h"
#include "dyninstAPI/src/aix.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/instPoint.h" // class instPoint
#include "dyninstAPI/src/showerror.h"
#include "common/h/debugOstream.h"
#include "dyninstAPI/src/InstrucIter.h"
#include "dyninstAPI/src/rpcMgr.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/src/multiTramp.h"
#include "dyninstAPI/src/miniTramp.h"

#include "InstrucIter.h"

#include <sstream>

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
    
    if (!funcFrequencyTable.defines(func->prettyName())) {
        // Changing this value from 250 to 100 because predictedCost was
        // too high - naim 07/18/96
        return(100);
    } else {
        return (funcFrequencyTable[func->prettyName()]);
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

Register deadRegs[] = {10, REG_GUARD_ADDR, REG_GUARD_VALUE, REG_GUARD_OFFSET};

registerSpace *regSpace;

registerSpace *floatRegSpace;

// This register space should be used with the conservative base trampoline.
// Right now it's only used for purposes of determining which registers must
// be saved and restored in the base trampoline.
registerSpace *conservativeRegSpace;

// allocate in reverse order since we use them to build arguments.
Register liveRegList[] = { 11, 10, 9, 8, 7, 6, 5, 4, 3 };

// If we're being conservative, we don't assume that any registers are dead.
#if defined( __XLC__) || defined(__xlC__)
//  XLC does not like empty initializer of unbounded array so this is init'd in initTramps
Register * conservativeDeadRegList;
#else
Register conservativeDeadRegList[] = { };
#endif

// The registers that aren't preserved by called functions are considered live.
Register conservativeLiveRegList[] = { 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 0 };

#if defined(__XLC__) || defined(__xlC__)
Register *floatingDeadRegList;
#else
Register floatingDeadRegList[] = { };
#endif

Register floatingLiveRegList[] = {13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

void initTramps(bool is_multithreaded)
{
    static bool inited=false;
    if (inited) return;
    inited = true;

    
    // reg 12 is defined not to have a live value at procedure call points.
    // reg 11 is the static chain register, used to hold an environment pointer
    //   for languages like Pascal and PL/1; it is also apparently used by
    //   OpenMP.  It should be considered live at entry points and call
    //   sites.
    // reg 3-10 are used to pass arguments to functions.
    //   We must save them before we can use them.
    Register deadRegList[1];
    unsigned dead_reg_count = 0;
    if(! is_multithreaded) {
       dead_reg_count++;
       deadRegList[0] = 12;
    }

    
    regSpace = 
       new registerSpace(dead_reg_count, deadRegList, 
                         sizeof(liveRegList)/sizeof(Register), liveRegList,
                         is_multithreaded);

    /*
      floatRegSpace = 
      new registerSpace(0, floatingDeadRegList,
      sizeof(floatingLiveRegList)/sizeof(Register), floatingLiveRegList,
      is_multithreaded); */

    regSpace->initFloatingPointRegisters(sizeof(floatingLiveRegList)/sizeof(Register), 
					floatingLiveRegList);
    
#if defined (__XLC__) || defined(__xlC__)
    conservativeDeadRegList = new Register[0]; //  is this just too weird?
#endif

    // Note that we don't always use this with the conservative base tramp --
    // see the message where we declare conservativeRegSpace.
    conservativeRegSpace =
      new registerSpace(sizeof(conservativeDeadRegList)/sizeof(Register),
		        conservativeDeadRegList, 
		        sizeof(conservativeLiveRegList)/sizeof(Register),
			conservativeLiveRegList);

    conservativeRegSpace->initFloatingPointRegisters(sizeof(floatingLiveRegList)/sizeof(Register), 
					floatingLiveRegList);

}

/*
 * Saving and restoring registers
 * We create a new stack frame in the base tramp and save registers
 * above it. Currently, the plan is this:
 *                 < 220 bytes as per system spec      >
 *                 < 14 GPR slots @ 4 bytes each       >
 *                 < 14 FPR slots @ 8 bytes each       >
 *                 < 6 SPR slots @ 4 bytes each        >
 *                 < 1 FP SPR slot @ 8 bytes           >
 *                 < Space to save live regs at func call >
 *                 < Func call overflow area, 32 bytes > 
 *                 < Linkage area, 24 bytes            >
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
    (*insn).xform.op = 31;
    (*insn).xform.rt = scratchReg;
    (*insn).xform.ra = sprnum & 0x1f;
    (*insn).xform.rb = (sprnum >> 5) & 0x1f;
    (*insn).xform.xo = 339;
    insn.generate(gen);
    
    (*insn).raw = 0;                    //st:     st scratchReg, stkOffset(r1)
    (*insn).dform.op      = 36;
    (*insn).dform.rt      = scratchReg;
    (*insn).dform.ra      = 1;
    (*insn).dform.d_or_si = stkOffset;

    insn.generate(gen);

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
    instruction insn;
    
    (*insn).raw = 0;                    //l:      l scratchReg, stkOffset(r1)
    (*insn).dform.op      = 32;
    (*insn).dform.rt      = scratchReg;
    (*insn).dform.ra      = 1;
    (*insn).dform.d_or_si = stkOffset;
    insn.generate(gen);
    
    (*insn).raw = 0;                    //mtspr:  mtlr scratchReg
    (*insn).xform.op = 31;
    (*insn).xform.rt = scratchReg;
    (*insn).xform.ra = sprnum & 0x1f;
    (*insn).xform.rb = (sprnum >> 5) & 0x1f;
    (*insn).xform.xo = 467;
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
    instruction insn;

    (*insn).raw = 0;                    //mfspr:  mflr scratchReg
    (*insn).xform.op = 31;
    (*insn).xform.rt = scratchReg;
    (*insn).xform.ra = 8;
    (*insn).xform.xo = 339;
    insn.generate(gen);
    
    (*insn).raw = 0;                    //st:     st scratchReg, stkOffset(r1)
    (*insn).dform.op      = 36;
    (*insn).dform.rt      = scratchReg;
    (*insn).dform.ra      = 1;
    (*insn).dform.d_or_si = stkOffset;
    insn.generate(gen);
    
    
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
    instruction insn;
    (*insn).raw = 0;                    //l:      l scratchReg, stkOffset(r1)
    (*insn).dform.op      = 32;
    (*insn).dform.rt      = scratchReg;
    (*insn).dform.ra      = 1;
    (*insn).dform.d_or_si = stkOffset;
    insn.generate(gen);
    
    (*insn).raw = 0;                    //mtspr:  mtlr scratchReg
    (*insn).xform.op = 31;
    (*insn).xform.rt = scratchReg;
    (*insn).xform.ra = 8;
    (*insn).xform.xo = 467;
    insn.generate(gen);

    
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
            unsigned      val,         //Value to set link register to
            unsigned      ti)          //Tail instruction
 {
     instruction insn;
    
    
    (*insn).raw =0;                  //cau:  cau scratchReg, 0, HIGH(val)
    (*insn).dform.op      = 15;
    (*insn).dform.rt      = scratchReg;
    (*insn).dform.ra      = 0;
    (*insn).dform.d_or_si = ((val >> 16) & 0x0000ffff);
    insn.generate(gen);
    
    (*insn).raw = 0;                 //oril:  oril scratchReg, scratchReg, LOW(val)
    (*insn).dform.op      = 24;
    (*insn).dform.rt      = scratchReg;
    (*insn).dform.ra      = scratchReg;
    (*insn).dform.d_or_si = (val & 0x0000ffff);
    insn.generate(gen);
    
    (*insn).raw = 0;                 //mtspr:  mtlr scratchReg
    (*insn).xform.op = 31;
    (*insn).xform.rt = scratchReg;
    (*insn).xform.ra = 8;
    (*insn).xform.xo = 467;
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
  (*insn).xform.op = 31;
  (*insn).xform.rt = scratchReg;
  (*insn).xform.xo = 19;
  insn.generate(gen);

  (*insn).raw = 0;                    //st:     st scratchReg, stkOffset(r1)
  (*insn).dform.op      = 36;
  (*insn).dform.rt      = scratchReg;
  (*insn).dform.ra      = 1;
  (*insn).dform.d_or_si = stkOffset;
  insn.generate(gen);

  
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

  (*insn).raw = 0;                    //l:      l scratchReg, stkOffset(r1)
  (*insn).dform.op      = 32;
  (*insn).dform.rt      = scratchReg;
  (*insn).dform.ra      = 1;
  (*insn).dform.d_or_si = stkOffset;
  insn.generate(gen);

  (*insn).raw = 0;                    //mtcrf:  scratchReg
  (*insn).xfxform.op  = 31;
  (*insn).xfxform.rt  = scratchReg;
  (*insn).xfxform.spr = 0xff << 1;
  (*insn).xfxform.xo  = 144;
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
    (*mffs).xform.op = 63;
    (*mffs).xform.rt = scratchReg;
    (*mffs).xform.xo = 583;
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
    (*mtfsf).xflform.op  = 63;
    (*mtfsf).xflform.flm = 0xff;
    (*mtfsf).xflform.frb = scratchReg;
    (*mtfsf).xflform.xo  = 711;
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
  
  p->writeDataSpace((void *)loc, instruction::size(), i.ptr());
}

void saveRegister(codeGen &gen,
                  Register reg,
                  int save_off)
{
    instruction::generateImm(gen, STop, 
                             reg, 1, 
                             save_off + reg*GPRSIZE);
    //  bperr("Saving reg %d at 0x%x off the stack\n", reg, offset + reg*GPRSIZE);
}

// Dest != reg : optimizate away a load/move pair

void restoreRegister(codeGen &gen,
                     Register source,
                     Register dest, int saved_off)
{
    instruction::generateImm(gen, Lop, 
                             dest, 1, saved_off + source*GPRSIZE);
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
    instruction::generateImm(gen, 
                             STFDop, 
                             reg, 
                             1, 
                             save_off + reg*FPRSIZE);
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
    instruction::generateImm(gen, STUop, 
                             REG_SP, REG_SP, -TRAMP_FRAME_SIZE);
}

void popStack(codeGen &gen)
{
    instruction::generateImm(gen, CALop, 
                             REG_SP, REG_SP, TRAMP_FRAME_SIZE);
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
#if 0
    // Clever version; check must_save. Not doing for now, saving everything.
    unsigned numRegs = 0;
    for(u_int i = 0; i < theRegSpace->getRegisterCount(); i++) {
        registerSlot *reg = theRegSpace->getRegSlot(i);
        if ((reg->number >= 10 && reg->number <= 12) || 
            reg->number == REG_GUARD_ADDR || reg->number == REG_GUARD_VALUE
            || reg->number == 0) {
            saveRegister(gen, reg->number, save_off);
            numRegs++;
        }
    }
#else

    unsigned numRegs = 0;
    for(u_int i = 0; i < theRegSpace->getRegisterCount(); i++) {
        registerSlot *reg = theRegSpace->getRegSlot(i);
        if (reg->startsLive) {
	  saveRegister(gen, reg->number, save_off);
	  numRegs++;
        }
    }
#endif
    
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
#if 0
    unsigned numRegs = 0;
    for(u_int i = 0; i < theRegSpace->getRegisterCount(); i++) {
        registerSlot *reg = theRegSpace->getRegSlot(i);
        if ((reg->number >= 10 && reg->number <= 12) || 
            reg->number == REG_GUARD_ADDR || reg->number == REG_GUARD_VALUE
            || reg->number == 0) 
            {
                restoreRegister(gen, reg->number, save_off);
                numRegs++;
            }
        else
            {
                if (reg->beenClobbered)
                    {
                        restoreRegister(gen, reg->number, TRAMP_GPR_OFFSET);
                        regSpace->unClobberRegister(reg->number);
                        numRegs++;
                    }
            }
    }
#else
    unsigned numRegs = 0;
    for(u_int i = 0; i < theRegSpace->getRegisterCount(); i++) {
        registerSlot *reg = theRegSpace->getRegSlot(i);
        if (reg->startsLive) {
            restoreRegister(gen, reg->number, save_off);
            numRegs++;
        }
    }
#endif

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
  for(u_int i = 0; i < theRegSpace->getFPRegisterCount(); i++) {
    registerSlot *reg = theRegSpace->getFPRegSlot(i);
    if (reg->startsLive) {
      saveFPRegister(gen, reg->number, save_off);
      numRegs++;
    }
  }
  
  return numRegs;
  
  /*
  unsigned numRegs = 0;
    for (unsigned i = 0; i <= 13; i++) {
        numRegs++;
        saveFPRegister(gen, i, save_off);
    }
    return numRegs;
  */
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
  for(u_int i = 0; i < theRegSpace->getFPRegisterCount(); i++) {
    registerSlot *reg = theRegSpace->getFPRegSlot(i);
    if (reg->startsLive) {
      restoreFPRegister(gen, reg->number, save_off);
      numRegs++;
    }
  }
  
  return numRegs;

  
  /*
    unsigned numRegs = 0;
    for (unsigned i = 0; i <= 13; i++) {
        numRegs++;
        restoreFPRegister(gen, i, save_off);
    }
    return numRegs;
  */


    /*
    unsigned numRegs = 0;
    for(u_int i = 0; i < theRegSpace->getFPRegisterCount(); i++) {
        registerSlot *reg = theRegSpace->getFPRegSlot(i);
        if (reg->number == 10)
            {
                restoreFPRegister(gen, reg->number, save_off, offset);
                numRegs++;
            }
        else
            {
                if (reg->beenClobbered)
                    {
                        restoreFPRegister(gen, reg->number, TRAMP_FPR_OFFSET, offset);
                        regSpace->unClobberFPRegister(reg->number);
                        numRegs++;
                    }
            }
    }
  return numRegs;
    */    
  
}

/*
 * Save the special purpose registers (for Dyninst conservative tramp)
 * CTR, CR, XER, SPR0, FPSCR
 */

unsigned saveSPRegisters(codeGen &gen,
			 registerSpace * theRegSpace,
                         int save_off)
{
    saveCR(gen, 10, save_off + STK_CR);           
    saveSPR(gen, 10, SPR_CTR, save_off + STK_CTR);
    saveSPR(gen, 10, SPR_XER, save_off + STK_XER);
    if (theRegSpace->getSPFlag())
      saveSPR(gen, 10, SPR_SPR0, save_off + STK_SPR0);
    saveFPSCR(gen, 10, save_off + STK_FP_CR);
    return 4; // register saved
}

/*
 * Restore the special purpose registers (for Dyninst conservative tramp)
 * CTR, CR, XER, SPR0, FPSCR
 */

unsigned restoreSPRegisters(codeGen &gen,
			    registerSpace *theRegSpace,
                            int save_off)
{
    restoreCR(gen, 10, save_off + STK_CR);
    restoreSPR(gen, 10, SPR_CTR, save_off + STK_CTR);
    restoreSPR(gen, 10, SPR_XER, save_off + STK_XER);
    if (theRegSpace->getSPFlag())
      restoreSPR(gen, 10, SPR_SPR0, save_off + STK_SPR0);
    
    restoreFPSCR(gen, 10, save_off + STK_FP_CR);
    return 4; // restored
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
                              registerSpace *) {
    // Make a stack frame.
    pushStack(gen);

    // Save GPRs
    // Reverting to "save all" behavior; Nick's going to fix
    // this.
    saveGPRegisters(gen,
                    theRegSpace,
                    TRAMP_GPR_OFFSET);
    // Save FPRs
    saveFPRegisters(gen,
                    theRegSpace,
                    TRAMP_FPR_OFFSET);
    
    // Save LR
    saveLR(gen, REG_SCRATCH, // register to use
           TRAMP_SPR_OFFSET + STK_LR);
    
    // No more cookie. FIX aix stackwalking.
    if (isConservative()) {
        saveSPRegisters(gen,
			theRegSpace,
                        TRAMP_SPR_OFFSET);
    }
    // If we're at a callsite (or unknown) save the count register
    if (isConservative() || isCallsite()) {
        saveSPR(gen, REG_SCRATCH, // scratchreg
                SPR_CTR, TRAMP_SPR_OFFSET + STK_CTR);
    }
    return true;
}

bool baseTramp::generateRestores(codeGen &gen,
                                 registerSpace *) {
    // Restore possible SPR saves
    if (isConservative() || isCallsite()) {
        restoreSPR(gen, REG_SCRATCH, 
                   SPR_CTR, TRAMP_SPR_OFFSET + STK_CTR);
    }
    if (isConservative()) {
        restoreSPRegisters(gen, theRegSpace, TRAMP_SPR_OFFSET);
    }

    // LR
    restoreLR(gen, REG_SCRATCH, TRAMP_SPR_OFFSET + STK_LR);
    // FPRs
    restoreFPRegisters(gen, theRegSpace, TRAMP_FPR_OFFSET);
    // GPRs
    restoreGPRegisters(gen, theRegSpace, TRAMP_GPR_OFFSET);

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

    AstNode *threadPOS;
    pdvector<AstNode *> dummy;
    Register src = Null_Register;
    
    // registers cleanup
    regSpace->resetSpace();
    
    /* Get the hashed value of the thread */
    if (threaded()) 
       threadPOS = new AstNode("DYNINSTreturnZero", dummy);
    else
       threadPOS = new AstNode("DYNINSTthreadIndex", dummy);
    src = threadPOS->generateCode(proc(), regSpace, gen,
                                  false, // noCost 
                                  true); // root node
    if ((src) != REG_MT_POS) {
        // This is always going to happen... we reserve REG_MT_POS, so the
        // code generator will never use it as a destination
        instruction::generateImm(gen, ORILop, src, REG_MT_POS, 0);
    }
    regSpace->resetSpace();
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
              gen, false);
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
          REG_GUARD_VALUE,
          gen, false);
    instruction::generateImm(gen, CMPIop, 0, REG_GUARD_VALUE, 1);

    // Next thing we do is jump; store that in guardJumpOffset
    guardJumpIndex = gen.getIndex();

    // Drop an illegal in here for now.
    instruction::generateIllegal(gen);
    
    // Back to the base/offset style. 
    emitVload(loadConstOp, 0, REG_GUARD_VALUE, REG_GUARD_VALUE,
              gen, false);
    // And store
    emitV(storeIndirOp, REG_GUARD_VALUE, 0, REG_GUARD_ADDR, 
          gen, false);

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
              gen, false);
    // And store
    emitV(storeIndirOp, REG_GUARD_VALUE, 0, REG_GUARD_ADDR, 
          gen, false);

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
    proc()->writeDataSpace((void *)trampCostAddr,
                           gen.used(),
                           gen.start_ptr());
}

void emitImm(opCode op, Register src1, RegValue src2imm, Register dest, 
             codeGen &gen, bool noCost, registerSpace *rs)
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
        if (isPowerOf2(src2imm,result) && (result<32)) {
            instruction::generateLShift(gen, src1, result, dest);
            return;
        }
        else {
            Register dest2 = regSpace->allocateRegister(gen, noCost);
            emitVload(loadConstOp, src2imm, dest2, dest2, gen, noCost);
            emitV(op, src1, dest2, dest, gen, noCost);
            regSpace->freeRegister(dest2);
            return;
        }
        break;
        
    case divOp:
        if (isPowerOf2(src2imm,result) && (result<32)) {
            instruction::generateRShift(gen, src1, result, dest);
            return;
        }
        else {
            Register dest2 = regSpace->allocateRegister(gen, noCost);
            emitVload(loadConstOp, src2imm, dest2, dest2, gen, noCost);
            emitV(op, src1, dest2, dest, gen, noCost);
            regSpace->freeRegister(dest2);
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
        Register dest2 = regSpace->allocateRegister(gen, noCost);
        emitVload(loadConstOp, src2imm, dest2, dest2, gen, noCost);
        emitV(op, src1, dest2, dest, gen, noCost);
        regSpace->freeRegister(dest2);
        return;
        break;
    }
}


//static int dummy[3];
//
//void
//initTocOffset(int toc_offset) {
//    //  st r2,20(r1)  ; 0x90410014 save toc register  
//    dummy[0] = 0x90410014; 
//
//    //  liu r2, 0x0000     ;0x3c40abcd reset the toc value to 0xabcdefgh
//    dummy[1] = (0x3c400000 | (toc_offset >> 16));
//
//    //  oril    r2, r2,0x0000   ;0x6042efgh
//    dummy[2] = (0x60420000 | (toc_offset & 0x0000ffff));
//}


void cleanUpAndExit(int status);

//
// Author: Jeff Hollingsworth (3/26/96)
//
// Emit a function call.
//   It saves registers as needed.
//   copy the passed arguments into the canonical argument registers (r3-r10)
//   Locate the TOC entry of the callee module and copy it into R2
//   generate a branch and link the destination
//   Restore the original TOC into R2
//   restore the saved registers.
//
// Parameters:
//   op - unused parameter (to be compatible with sparc)
//   srcs - vector of ints indicating the registers that contain the parameters
//   dest - the destination address (should be Address not reg). 
//   insn - pointer to the code we are generating
//   based - offset into the code generated.
//


Register emitFuncCall(opCode /* ocode */, 
		      registerSpace *rs,
                      codeGen &gen,
		      pdvector<AstNode *> &operands, 
		      process *proc, bool noCost,
		      Address callee_addr,
		      const pdvector<AstNode *> &ifForks,
		      const instPoint *location)
{
    Address toc_anchor;
    bool clobberAll = false;
    pdvector <Register> srcs;

    //  Sanity check for NULL address argument
    if (!callee_addr) {
        char msg[256];
        sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out"
                "callee_addr argument", __FILE__, __LINE__);
        showErrorCallback(80, msg);
        assert(0);
    }
 
   // Now that we have the destination address (unique, hopefully) 
   // get the TOC anchor value for that function
   // The TOC offset is stored in the Object. 
   // file() -> pdmodule "parent"
   // exec() -> image "parent"
   //toc_anchor = ((int_function *)calleefunc)->file()->exec()->getObject().getTOCoffset();
   toc_anchor = proc->getTOCoffsetInfo(callee_addr);
   
   // Generate the code for all function parameters, and keep a list
   // of what registers they're in.
   for (unsigned u = 0; u < operands.size(); u++) {
      if (operands[u]->getSize() == 8) {
         // What does this do?
         bperr( "in weird code\n");
         Register dummyReg = rs->allocateRegister(gen, noCost);
	 rs->clobberRegister(dummyReg);
         srcs.push_back(dummyReg);
         
         instruction::generateImm(gen, CALop, dummyReg, 0, 0);
      }
      srcs.push_back(operands[u]->generateCode_phase2(proc, rs, gen,
                                                      false, ifForks, location));
      //bperr( "Generated operand %d, base %d\n", u, base);
   }

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
  
   // Save register 2 (TOC)
   saveRegister(gen, 2, FUNC_CALL_SAVE);
   savedRegs.push_back(2);

   if(proc->multithread_capable()) {
      // save REG_MT_POS
      saveRegister(gen, REG_MT_POS, FUNC_CALL_SAVE);
      savedRegs += REG_MT_POS;
   }

   // see what others we need to save.
   for (u_int i = 0; i < rs->getRegisterCount(); i++) {
      registerSlot *reg = rs->getRegSlot(i);
      if (reg->needsSaving) {
         // needsSaving -> caller saves register
         // we MUST save restore this and the end of the function call
         //     rather than delay it to the end of the tramp due to:
         //        (1) we could be in a conditional & the restores would
         //            be unconditional (i.e. restore bad data)
         //        (2) $arg[n] code depends on paramters being in registers
         //
         // MT_AIX: we are not saving registers on demand on the power
         // architecture anymore - naim
         // saveRegister(gen,reg->number,8+(46*4));
         // savedRegs += reg->number;
      } else if (reg->refCount > 0 && !reg->mustRestore) {
         // inUse && !mustRestore -> in use scratch register 
         //		(i.e. part of an expression being evaluated).

         // no reason to save the register if we are going to free it in a bit
         // we should keep it free, otherwise we might overwrite the register
         // we allocate for the return value -- we can't request that register
         // before, since we then might run out of registers
         unsigned u;
         for(u=0; u < srcs.size(); u++) {
            if(reg->number == srcs[u]) break;
         }
         // since the register should be free
         // assert((u == srcs.size()) || (srcs[u] != (int) (u+3)));
         if(u == srcs.size()) {
             saveRegister(gen, reg->number, FUNC_CALL_SAVE);
            savedRegs.push_back(reg->number);
            //cerr << "Saved inUse && ! mustRestore reg " << reg->number << endl;
         }
      } else if (reg->refCount > 0) {
         // only inuse registers permitted here are the parameters.
         unsigned u;
         for (u=0; u<srcs.size(); u++){
            if (reg->number == srcs[u]) break;
         }
         if (u == srcs.size()) {
            // XXXX - caller saves register that is in use.  We have no
            //    place to save this, but we must save it!!!.  Should
            //    find a place to push this on the stack - jkh 7/31/95
            pdstring msg = "Too many registers required for MDL expression\n";
            bpfatal( msg.c_str());
            showErrorCallback(94,msg);
            cleanUpAndExit(-1);
         }
      }
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
  


   int scratchReg[8];
   for (int a = 0; a < 8; a++)
     {
       scratchReg[a] = -1;
     }

   // Now load the parameters into registers.
   for (unsigned u=0; u<srcs.size(); u++){
      // check that is is not already in the register
   
     /*

     if (srcs[u] == (unsigned int) u+3) {
         rs->freeRegister(srcs[u]);
         continue;
      }
      assert(rs->isFreeRegister(u+3));

      // internal error we expect this register to be free here
      // if (!rs->isFreeRegister(u+3)) abort();
    
      rs->clobberRegister(u+3);
      //if(rs->clobberRegister(u+3))
      //	saveRegister(gen, u+3, TRAMP_GPR_OFFSET);

      instruction::generateImm(gen, ORILop, 
                               srcs[u], u+3, 0);
      inst_printf("move reg %d to %d (%d)...", srcs[u], u+3, base);

      // source register is now free.
      rs->freeRegister(srcs[u]);
     */

     
     if (srcs[u] == (unsigned int) u+3) 
       {
	 rs->freeRegister(srcs[u]);
	 continue;
       }

     int whichSource = -1;
     bool hasSourceBeenCopied = true;
     Register scratch = 13;

     if (scratchReg[u] != -1)
       {
           instruction::generateImm(gen, ORILop, scratchReg[u], u+3, 0);
           rs->freeRegister(scratchReg[u]);
       }
     else
         {
             for (unsigned v=u; v < srcs.size(); v++)
                 {
                     if (srcs[v] == u+3)
                         {
                             hasSourceBeenCopied = false;
                             whichSource = v;
                         }
                 }
             if (!hasSourceBeenCopied)
                 {
                     scratch = rs->allocateRegister(gen, noCost);
                     rs->clobberRegister(scratch);
                     instruction::generateImm(gen, ORILop, u+3, scratch, 0);
                     rs->freeRegister(u+3);
                     scratchReg[whichSource] = scratch;
                     hasSourceBeenCopied = true;
                     
                     instruction::generateImm(gen, ORILop, srcs[u], u+3, 0);
                     rs->freeRegister(srcs[u]);
                 }
             else
                 {
                     instruction::generateImm(gen, ORILop, srcs[u], u+3, 0);
                     rs->freeRegister(srcs[u]);
                     rs->clobberRegister(u+3);
                 }
         }
     
     
   }

   // Set up the new TOC value

   emitVload(loadConstOp, toc_anchor, 2, 2, gen, false);
   inst_printf("toc setup (%d)...");

   // generate a branch to the subroutine to be called.
   // load r0 with address, then move to link reg and branch and link.

   emitVload(loadConstOp, callee_addr, 0, 0, gen, false);
   inst_printf("addr setup (%d)....");
  
   // Move to link register
   instruction mtlr0(MTLR0raw);
   mtlr0.generate(gen);
   
   inst_printf("mtlr0 (%d)...");

   // Linear Scan on the functions to see which registers get clobbered
     
   //clobberAll = true;

   
   int_function *funcc;
   
   codeRange *range = proc->findCodeRangeByAddress(callee_addr);
   
   if (range)
     {
       funcc = range->is_function();
       //cout << "Function Name is " << funcc->prettyName() << endl;
       
       if (funcc) {           
	 InstrucIter ah(funcc);
	 
	 //while there are still instructions to check for in the
	 //address space of the function
	 
	 while (ah.hasMore()) {

	   if (ah.isA_RT_WriteInstruction())
	     {
	       
	       rs->clobberRegister(ah.getRTValue());
	     }
	   if (ah.isA_RA_WriteInstruction())
	     {
	       rs->clobberRegister(ah.getRAValue());
	     }
	   if (ah.isA_FRT_WriteInstruction())
	     {
	       rs->clobberFPRegister(ah.getRTValue());
	     }
	   if (ah.isA_FRA_WriteInstruction())
	     {
	       
	       //cout<<"Writes to FRA for ";
	       //ah.printOpCode();
	       //cout<<" and register ";
	       //cout<<ah.getRAValue()<<endl;
	       
	       rs->clobberFPRegister(ah.getRAValue());
	     }
	   if (ah.isAReturnInstruction()){
	   }
	   else if (ah.isACondBranchInstruction()){
	   }
	   else if (ah.isAJumpInstruction()){
	   }
	   else if (ah.isACallInstruction()){
	     clobberAll = true;
	   }
	   else if (ah.isAnneal()){
	   }
	   else{
	   }
	   ah++;
	 }
       }
     }
   
   
   
   /////////////// Clobber the registers if needed

   if (clobberAll)
     {
       for(u_int i = 0; i < rs->getRegisterCount(); i++){
	 registerSlot * reg = rs->getRegSlot(i);
	 rs->clobberRegister(reg->number);
       }
       
       for(u_int i = 0; i < rs->getFPRegisterCount(); i++){
	 registerSlot * reg = rs->getFPRegSlot(i);
	 rs->clobberFPRegister(reg->number);
       }
     }
   

   // brl - branch and link through the link reg.

   instruction brl(BRLraw);
   brl.generate(gen);

   // get a register to keep the return value in.
   Register retReg = rs->allocateRegister(gen, noCost);
   rs->clobberRegister(retReg);

   // put the return value from register 3 to the newly allocated register.
   instruction::generateImm(gen, ORILop, 3, retReg, 0);

   // Restore floating point registers
   //restoreFPRegisters(gen, TRAMP_FPR_OFFSET);
   
   //Restore the registers used to save  parameters
   /*
   for(u_int i = 0; i < rs->getRegisterCount(); i++) {
   registerSlot *reg = rs->getRegSlot(i);
   if (reg->beenClobbered) {
   restoreRegister(gen, reg->number, TRAMP_GPR_OFFSET);
   regSpace->unClobberRegister(reg->number);
   }
   }
   */
   
   // restore saved registers.
   for (u_int ui = 0; ui < savedRegs.size(); ui++) {
       restoreRegister(gen,
                       savedRegs[ui],FUNC_CALL_SAVE);
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
        gen.fill(instruction::maxJumpSize(), codeGen::cgNOP);
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
               const instPoint * /* location */, bool /*for_MT*/)
{
    //bperr("emitR(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

    switch (op) {
    case getRetValOp: {
	// return value is in register 3
	Register reg = 3;
	registerSlot *regSlot = NULL;

	// find the registerSlot for this register.
	for (unsigned i = 0; i < regSpace->getRegisterCount(); i++) {
	    regSlot = regSpace->getRegSlot(i);
	    if (regSlot->number == reg) {
		break;
	    }
	}
        // FIXME
	if (regSlot->mustRestore || 1) {
            // its on the stack so load it.
            restoreRegister(gen, reg, dest, TRAMP_GPR_OFFSET);
            return(dest);
	} else {
	    // its still in a register so return the register it is in.
	    return(reg);
	}
      }
    case getParamOp: {
      // the first 8 parameters (0-7) are stored in registers (r3-r10) upon entering
      // the function and then saved above the stack upon entering the trampoline;
      // in emit functional call the stack pointer is moved so the saved registers
      // are not over-written
      // the other parameters > 8 are stored on the caller's stack at an offset.
      // 
      // src1 is the argument number 0..X, the first 8 are stored in registers
      // r3 and 
      if(src1 < 8) {
	registerSlot * regSlot = NULL;
	// find the registerSlot for this register.
	for (unsigned i = 0; i < regSpace->getRegisterCount(); i++) {
	  regSlot = regSpace->getRegSlot(i);
	  if (regSlot->number == src1+3) 
	    {
                // FIXME
                if (regSpace->beenSaved(regSlot->number) || 1) {
                    restoreRegister(gen, src1+3, dest, 
                                    TRAMP_GPR_OFFSET);
                }
                else {
                    instruction::generateImm(gen, ORILop,
                                             src1+3, dest, 0);
		}
	    }
	}    
	return(dest);
      } else {
          // Registers from 11 (src = 8) and beyond are saved starting at stack+56
          instruction::generateImm(gen, Lop, 
                                   dest, 1, 
                                   TRAMP_FRAME_SIZE+((src1-8)*sizeof(unsigned))+56);
          return(dest);
      }
    }
    default:
        assert(0);        // unexpected op for this emit!
    }
    return 0; // Compiler happiness
}

#ifdef BPATCH_LIBRARY
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
  if(reg == 1) // SP is in a different place, but we don't need to
               // restore it, just subtract the stack frame size
      instruction::generateImm(gen, CALop, 
                               dest, REG_SP, TRAMP_FRAME_SIZE);

  else if((reg == 0) || ((reg >= 3) && (reg <=12)))
      instruction::generateImm(gen, Lop, dest, 1, 
                               TRAMP_GPR_OFFSET + reg*GPRSIZE);
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
    instruction::generateImm(gen, Lop, dest, 
                             1, TRAMP_SPR_OFFSET + STK_XER);
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
    (*rld).mdform.op = RLDop;
    (*rld).mdform.rs = reg;
    (*rld).mdform.ra = dest;
    (*rld).mdform.sh = 0;  //(32+25+7) % 32;
    (*rld).mdform.mb_or_me = (64-7) % 32;
    (*rld).mdform.mb_or_me2 = (64-7) / 32;
    (*rld).mdform.xo = 0;  // rldicl
    (*rld).mdform.sh2 = 0; //(32+25+7) / 32;
    (*rld).mdform.rc = 0;
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
        temp = regSpace->allocateRegister(gen, noCost);
        
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
        regSpace->freeRegister(temp);
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
#endif

void emitVload(opCode op, Address src1, Register /*src2*/, Register dest,
               codeGen &gen, bool /*noCost*/, 
               registerSpace * /*rs*/, int size,
               const instPoint * /* location */, process * /* proc */)
{
    if (op == loadConstOp) {
	unsigned int constValue = (int) src1;
	unsigned int top_half = ((constValue & 0xffff0000) >> 16);
	unsigned int bottom_half = (constValue & 0x0000ffff);
	assert (constValue == ((top_half << 16) + bottom_half));
	// AIX sign-extends. So if top_half is 0, and the top bit of
	// bottom_half is 0, then we can use a single instruction. Otherwise
	// do it the hard way.
	if (!top_half && !(bottom_half & 0x8000)) {
            // single instruction (CALop)
            instruction::generateImm(gen, 
                                     CALop, dest, 0, bottom_half);
	}
	else {
	  instruction::generateImm(gen, CAUop, 
                                   dest, 0, top_half);
	  // ori dest,dest,LOW(src1)
	  instruction::generateImm(gen, ORILop, 
                                   dest, dest, bottom_half);
	}
    } else if (op ==  loadOp) {
	int high;

	// load high half word of address into dest.
	// really addis 0,dest,HIGH(src1) aka lis dest, HIGH(src1)
	if (LOW(src1) & 0x8000) {
  	    // high bit of low is set so the sign extension of the load
	    // will cause the wrong effective addr to be computed.
	    // so we subtract the sign ext value from HIGH.
	    // sounds odd, but works and saves an instruction - jkh 5/25/95
	    high = HIGH(src1) - 0xffff;
        } else {
   	    high = HIGH(src1);
	}
	instruction::generateImm(gen,
                                 CAUop, dest, 0, high);

	// really load dest, (dest)imm
        if (size == 1)
           instruction::generateImm(gen, LBZop, 
                                    dest, dest, LOW(src1));
        else if (size == 2)
           instruction::generateImm(gen, LHZop, 
                                    dest, dest, LOW(src1));
        else 
           instruction::generateImm(gen, Lop, 
                                    dest, dest, LOW(src1));
        
    } else if (op == loadFrameRelativeOp) {
	// return the value that is FP offset from the original fp
	int offset = (int) src1;

	if ((offset < MIN_IMM16) || (offset > MAX_IMM16)) {
	    assert(0);
	} else {
	    instruction::generateImm(gen, Lop, dest, 
                                     REG_SP, offset + TRAMP_FRAME_SIZE);
	}
    } else if (op == loadFrameAddr) {
	// offsets are signed!
	int offset = (int) src1;
        
	if ((offset < MIN_IMM16) || (offset > MAX_IMM16)) {
            assert(0);
	} else {
	    instruction::generateImm(gen, 
                                     CALop, dest, REG_SP,
                                     offset + TRAMP_FRAME_SIZE);
	}
    } else {
      assert(0);
    }
}

void emitVstore(opCode op, Register src1, Register /*src2*/, Address dest,
	      codeGen &gen, bool noCost, 
                registerSpace *rs, int /* size */,
                const instPoint * /* location */, process * /* proc */)
{
    if (op == storeOp) {
	int high;

	// load high half word of address into dest.
	// really addis 0,dest,HIGH(src1) aka lis dest, HIGH(src1)
	if (LOW(dest) & 0x8000) {
  	    // high bit of low is set so the sign extension of the load
	    // will cause the wrong effective addr to be computed.
	    // so we subtract the sign ext value from HIGH.
	    // sounds odd, but works and saves an instruction - jkh 5/25/95
	    high = HIGH(dest) - 0xffff;
        } else {
   	    high = HIGH(dest);
	}

	// temp register to hold base address for store (added 6/26/96 jkh)
	Register temp = regSpace->allocateRegister(gen, noCost);
	rs->clobberRegister(temp);

	// set upper 16 bits of  temp to be the top high.
	instruction::generateImm(gen, CAUop, 
                                 temp, 0, high);

	// low == LOW(dest)
	// generate -- st src1, low(temp)
	instruction::generateImm(gen, STop, 
                                 src1, temp, LOW(dest));
	regSpace->freeRegister(temp);
        return;
    } else if (op == storeFrameRelativeOp) {
	// offsets are signed!
	int offset = (int) dest;
	if ((offset < MIN_IMM16) || (offset > MAX_IMM16)) {
	  assert(0);
	} else {
	    instruction::generateImm(gen, STop, 
                                     src1, REG_SP, offset + TRAMP_FRAME_SIZE);
	}
    } else {
        assert(0);       // unexpected op for this emit!
    }
}

void emitV(opCode op, Register src1, Register src2, Register dest,
           codeGen &gen, bool /*noCost*/,
           registerSpace * /*rs*/, int /* size */,
           const instPoint * /* location */, process * /* proc */)
{
    //bperr("emitV(op=%d,src1=%d,src2=%d,dest=%d)\n",op,src1,src2,dest)
;

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
        instruction::generateImm(gen,
                                 Lop, dest, src1, 0);

    } else if (op == storeIndirOp) {

        // generate -- st src1, dest
        (*insn).raw = 0;
        (*insn).dform.op = STop;
        (*insn).dform.rt = src1;
        (*insn).dform.ra = dest;
        (*insn).dform.d_or_si = 0;
        insn.generate(gen);

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
                //#if defined(_POWER_PC)
                // xop = DIVWxop; // PowerPC 32 bit divide instruction
                //#else 
                instOp = DIVSop;   // Power divide instruction
                instXop = DIVSxop;
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
      cost += sizeof(liveRegList)/sizeof(int);
      
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
      cost += sizeof(liveRegList)/sizeof(int);
      
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

//Checks to see if register has been clobbered
//If return true, it hasn't and we need to save register
//If return false, it's been clobbered and we do nothing
bool registerSpace::clobberRegister(Register reg) 
{
    if (reg == 0 || reg == 2 || reg == deadRegs[0] || reg == deadRegs[1] || 
        reg == deadRegs[2] || reg == 11 || reg == 12 )
        return false;
    for (u_int i=0; i < numRegisters; i++) {
        if (registers[i].number == reg) {
            if(registers[i].beenClobbered == true)
                return false;
            else if (registers[i].startsLive == false)
                return false;
            else{
                registers[i].beenClobbered = true;
                return true;
            }	
        }
    }
    //assert(0 && "Unreachable");
    return false;
}

//Checks to see if register has been clobbered
//If return true, it hasn't and we need to save register
//If return false, it's been clobbered and we do nothing
bool registerSpace::clobberFPRegister(Register reg) 
{
  if (reg == 10)
    return false;
  for (u_int i=0; i < numFPRegisters; i++) {
    if (fpRegisters[i].number == reg) {
      if(fpRegisters[i].beenClobbered == true)
	return false;
      else{
	fpRegisters[i].beenClobbered = true;
	return true;
      }	
    }
  }
  //assert(0 && "Unreachable");
  return false;
}

// Takes information from instPoint and resets
// regSpace liveness information accordingly
// Right now, all the registers are assumed to be live by default
void registerSpace::resetLiveDeadInfo(const int * liveRegs, 
				      const int * liveFPRegs,
				      const int * liveSPRegs)
{
  registerSlot *regSlot = NULL;
  registerSlot *regFPSlot = NULL;

  if (liveRegs != NULL & liveFPRegs != NULL)
    {
      /*
      printf("GPR  ");
      for (int a = 0; a < 32; a++)
	printf("%d ",liveRegs[a]);
      printf("\n");
      
      printf("FPR  ");
      for (int a = 0; a < 32; a++)
	printf("%d ",liveFPRegs[a]);
	printf("\n");
      */

      for (u_int i = 0; i < regSpace->getRegisterCount(); i++)
	{
	  regSlot = regSpace->getRegSlot(i);
	  if (  liveRegs[ (int) registers[i].number ] == 1 )
	    {
	      registers[i].needsSaving = true;
	      registers[i].startsLive = true;
	    }
	  else
	    {
	      registers[i].needsSaving = false;
	      registers[i].startsLive = false;
	    }
	}

       for (u_int i = 0; i < regSpace->getFPRegisterCount(); i++)
	{
	  regFPSlot = regSpace->getFPRegSlot(i);
	  if (  liveFPRegs[ (int) fpRegisters[i].number ] == 1 )
	    {
	      fpRegisters[i].needsSaving = true;
	      fpRegisters[i].startsLive = true;
	    }
	  else
	    {
	      fpRegisters[i].needsSaving = false;
	      fpRegisters[i].startsLive = false;
	    }
	}
    }
  if (liveSPRegs != NULL)
    spFlag = liveSPRegs[0];
}


bool registerSpace::beenSaved(Register reg)
{
  if (reg == 10 || reg == deadRegs[1] || reg == deadRegs[2] || reg == 11 || reg == 12)
    return true;
  for (u_int i = 0; i < numRegisters; i++){
    if (registers[i].number == reg){
      if (registers[i].beenClobbered == true)
	return true;
      else
	return false;
    }
  }
  assert(0 && "Unreachable");
  return false;
}

bool registerSpace::beenSavedFP(Register reg)
{
  if (reg == 10)
    return true;
  for (u_int i = 0; i < numFPRegisters; i++){
    if (fpRegisters[i].number == reg){
      if (fpRegisters[i].beenClobbered == true)
	return true;
      else
	return false;
    }
  }
  assert(0 && "Unreachable");
  return false;
}


//
// This is specific to some processors that have info in registers that we
//   can read, but should not write.
//   On power, registers r3-r11 are parameters that must be read only.
//     However, sometimes we have spilled to paramter registers back to the
//     stack to use them as scratch registers.  In this case they are writeable.
//
bool registerSpace::readOnlyRegister(Register reg_number) 
{
    registerSlot *regSlot = NULL;

    // it's not a parameter registers so it is read/write
    if ((reg_number > 11) || (reg_number < 3)) return false;

    // find the registerSlot for this register.
    for (u_int i = 0; i < regSpace->getRegisterCount(); i++) {
	regSlot = regSpace->getRegSlot(i);
	if (regSlot->number == reg_number) {
	    break;
	}
    }

    if (regSlot->mustRestore) {
	// we have already wrriten this to the stack so its OK to clobber it.
	return(false);
    } else {
	// its a live parameter register.
        return true;
    }
}

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

// process::replaceFunctionCall
//
// Replace the function call at the given instrumentation point with a call to
// a different function, or with a NOOP.  In order to replace the call with a
// NOOP, pass NULL as the parameter "func."
// Returns true if sucessful, false if not.  Fails if the site is not a call
// site, or if the site has already been instrumented using a base tramp.
bool process::replaceFunctionCall(instPoint *point,
				  const int_function *func) {
   // Must be a call site
   if (point->getPointType() != callSite)
      return false;

   inst_printf("Function replacement, point func %s, new func %s, point primary addr 0x%x\n",
               point->func()->symTabName().c_str(), func ? func->symTabName().c_str() : "<NULL>",
               point->addr());

  instPointIter ipIter(point);
  instPointInstance *ipInst;
  while ((ipInst = ipIter++)) {  
      // Multiple replacements. Wheee...
      Address pointAddr = ipInst->addr();
      inst_printf("... replacing 0x%x", pointAddr);
      codeRange *range;

      if (modifiedAreas_.find(pointAddr, range)) {
          multiTramp *multi = range->is_multitramp();
          if (multi) {
              // We pre-create these guys... so check to see if
              // there's anything there
              if (!multi->generated()) {
                  removeMultiTramp(multi);
              }
              else {
                  // TODO: modify the callsite in the multitramp.
                  assert(0);
              }
          }
          if (dynamic_cast<functionReplacement *>(range)) {
              // We overwrote this in a function replacement...
              continue; 
          }
      }
      
      codeGen gen(instruction::size());
      if (func == NULL) {	// Replace with a NOOP
          instruction::generateNOOP(gen);
      } else {			// Replace with a new call instruction
          instruction::generateCall(gen, pointAddr, func->getAddress());
      }
      
      // Before we replace, track the code.
      // We could be clever with instpoints keeping instructions around, but
      // it's really not worth it.
      replacedFunctionCall *newRFC = new replacedFunctionCall();
      newRFC->callAddr = pointAddr;

      newRFC->callSize = instruction::size();
      if (func)
          newRFC->newTargetAddr = func->getAddress();
      else
          newRFC->newTargetAddr = 0;
      
      codeGen old(instruction::size());
      old.copy(point->insn().ptr(), instruction::size());
      newRFC->oldCall = old;
      newRFC->newCall = gen;
      
      replacedFunctionCalls_[pointAddr] = newRFC;
      
      
      writeTextSpace((caddr_t)pointAddr, gen.used(), gen.start_ptr());
      inst_printf("...done\n");
  }
  return true;
}

// Emit code to jump to function CALLEE without linking.  (I.e., when
// CALLEE returns, it returns to the current caller.)

// 18FEB00 -- I removed the parameter names to get rid of compiler
// warnings. They are copied below.
// opCode op, codeGen &gen, const int_function *callee, process *proc

void emitFuncJump(opCode, 
                  codeGen &gen,
		  const int_function * /*unused*/, process *,
		  const instPoint *, bool)
{
     /* Unimplemented on this platform! */
     assert(0);
}

// atch AIX register numbering (sys/reg.h)
#define REG_LR  131
#define REG_CTR 132

void emitLoadPreviousStackFrameRegister(Address register_num, 
					Register dest,
                                        codeGen &gen,
					int size,
					bool noCost)
{
  // Offset if needed
  int offset;
  // Unused, 3OCT03
  //instruction *insn_ptr = (instruction *)insn;
  // We need values to define special registers.
  switch ( (int) register_num)
    {
    case REG_LR:
      // LR is saved on the stack
      // Note: this is only valid for non-function entry/exit instru. 
      // Once we've entered a function, the LR is stomped to point
      // at the exit tramp!
      offset = TRAMP_SPR_OFFSET + STK_LR; 
      // Get address (SP + offset) and stick in register dest.
      emitImm(plusOp ,(Register) REG_SP, (RegValue) offset, dest,
	      gen, noCost, regSpace);
      // Load LR into register dest
      emitV(loadIndirOp, dest, 0, dest, gen, noCost, regSpace, size);
      break;
    case REG_CTR:
      // CTR is saved down the stack
        offset = TRAMP_SPR_OFFSET + STK_CTR;
        // Get address (SP + offset) and stick in register dest.
        emitImm(plusOp ,(Register) REG_SP, (RegValue) offset, dest,
                gen, noCost, regSpace);
        // Load LR into register dest
        emitV(loadIndirOp, dest, 0, dest, gen, noCost, regSpace, size);
      break;
    default:
      cerr << "Fallthrough in emitLoadPreviousStackFrameRegister" << endl;
      cerr << "Unexpected register " << register_num << endl;
      abort();
      break;
    }
}

bool process::getDynamicCallSiteArgs(instPoint *callSite,
                                    pdvector<AstNode *> &args)
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
            args.push_back( new AstNode(AstNode::PreviousStackFrameDataReg,
                                        (void *) branch_target));
            // Where we are now
            args.push_back( new AstNode(AstNode::Constant,
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

#ifdef NOTDEF // PDSEP
bool process::MonitorCallSite(instPoint *callSite){
  instruction i = callSite->originalInstruction;
  pdvector<AstNode *> the_args(2);
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
      the_args[0] = new AstNode(AstNode::PreviousStackFrameDataReg,
                                (void *) branch_target); 
      // Where we are now
      the_args[1] = new AstNode(AstNode::Constant,
                                (void *) callSite->absPointAddr(this));
      // Monitoring function
      AstNode *func = new AstNode("DYNINSTRegisterCallee", 
                                  the_args);
      miniTramp *mtHandle;
      addInstFunc(this, mtHandle, callSite, func, callPreInsn,
                  orderFirstAtPoint,
                  true,                        /* noCost flag   */
                  false,                       /* trampRecursiveDesired flag */
                  true);                       /* allowTrap */
      return true;
  }
  else if ((*i).xlform.op == Bop) {
      /// Why didn't we catch this earlier? In any case, don't print an error

      // I have seen this legally -- branches to the FP register saves.
      // Since we ignore the save macros, we have no idea where the branch
      // goes. For now, return true -- means no error.

      return true;
  }
  else
  {
      cerr << "MonitorCallSite: Unknown opcode " << (*i).xlform.op << endl; 
      cerr << "opcode extension: " << (*i).xlform.xo << endl;
      bperr( "Address is 0x%x, insn 0x%x\n", 
              callSite->absPointAddr(this), 
              (*i).raw);
      return false;
  }
}
#endif // NOTDEF // PDSEP

bool writeFunctionPtr(process *p, Address addr, int_function *f)
{
   Address buffer[3];
   Address val_to_write = f->getAddress();
   Address toc = p->getTOCoffsetInfo(val_to_write);
   buffer[0] = val_to_write;
   buffer[1] = toc;
   buffer[2] = 0x0;

   p->writeDataSpace((void *) addr, sizeof(buffer), buffer);
   return true;
}
