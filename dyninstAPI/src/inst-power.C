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
 * $Id: inst-power.C,v 1.222 2005/07/11 19:35:22 rutar Exp $
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

#include "InstrucIter.h"

#include <sstream>

extern bool isPowerOf2(int value, int &result);

#define ABS(x)		((x) > 0 ? x : -x)
//#define MAX_BRANCH	0x1<<23
#define MAX_BRANCH      0x01fffffc
#define MAX_CBRANCH	0x1<<13

#define MAX_IMM		0x1<<15		/* 15 plus sign == 16 bits */

#define SPR_XER	1
#define SPR_LR	8
#define SPR_CTR	9
#define SPR_SPR0 0 
#define DISTANCE(x,y)   ((x<y) ? (y-x) : (x-y))

Address getMaxBranch() {
  return MAX_BRANCH;
}


const char *registerNames[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
			"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
			"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
			"r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"};

dictionary_hash<pdstring, unsigned> funcFrequencyTable(pdstring::hash);

inline void generateBranchInsn(instruction *insn, int offset)
{
    if (ABS(offset) > MAX_BRANCH) {
      bperr( "Error: attempted a branch of 0x%x\n", offset);
	logLine("a branch too far\n");
	showErrorCallback(52, "Internal error: branch too far");
	bperr( "Attempted to make a branch of offset %x\n", offset);
	while (1) ;
	return;	
    }

    insn->raw = 0;
    insn->iform.op = Bop;
    insn->iform.aa = 0;
    insn->iform.lk = 0;
    insn->iform.li = offset >> 2;

    // logLine("ba,a %x\n", offset);
}

inline void genImmInsn(instruction *insn, int op, Register rt, Register ra, int immd)
{
  // something should be here to make sure immd is within bounds
  // bound check really depends on op since we have both signed and unsigned
  //   opcodes.
  // We basically check if the top bits are 0 (unsigned, or positive signed)
  // or 0xffff (negative signed)
  // This is because we don't enforce calling us with LOW(immd), and
  // signed ints come in with 0xffff set. C'est la vie.
  // TODO: This should be a check that the high 16 bits are equal to bit 15,
  // really.
  assert (((immd & 0xffff0000) == (0xffff0000)) ||
          ((immd & 0xffff0000) == (0x00000000)));

  insn->raw = 0;
  insn->dform.op = op;
  insn->dform.rt = rt;
  insn->dform.ra = ra;
  if (op==SIop) immd = -immd;
  insn->dform.d_or_si = immd;
}

// rlwinm ra,rs,n,0,31-n
inline void generateLShift(instruction *insn, int rs, int offset, int ra)
{
    assert(offset<32);
    insn->raw = 0;
    insn->mform.op = RLINMxop;
    insn->mform.rs = rs;
    insn->mform.ra = ra;
    insn->mform.sh = offset;
    insn->mform.mb = 0;
    insn->mform.me = 31-offset;
    insn->mform.rc = 0;
}

// rlwinm ra,rs,32-n,n,31
inline void generateRShift(instruction *insn, int rs, int offset, int ra)
{
    assert(offset<32);
    insn->raw = 0;
    insn->mform.op = RLINMxop;
    insn->mform.rs = rs;
    insn->mform.ra = ra;
    insn->mform.sh = 32-offset;
    insn->mform.mb = offset;
    insn->mform.me = 31;
    insn->mform.rc = 0;
}

//
// generate an instruction that does nothing and has to side affect except to
//   advance the program counter.
//
inline void generateNOOP(instruction *insn)
{
  insn->raw = NOOPraw;
}

inline void genSimpleInsn(instruction *insn, int op, 
                Register src1, Register src2, Register dest, Address &base)
{
    int xop=-1;
    insn->raw = 0;
    insn->xform.op = op;
    insn->xform.rt = src1;
    insn->xform.ra = dest;
    insn->xform.rb = src2;
    if (op==ANDop) {
      xop=ANDxop;
    } else if (op==ORop) {
      xop=ORxop;
    } else {
      // only AND and OR are currently designed to use genSimpleInsn
      assert(0);
    }
    insn->xform.xo = xop;
    base += sizeof(instruction);
}

inline void genRelOp(instruction *insn, int cond, int mode, Register rs1,
		     Register rs2, Register rd, Address &base)
{
    // cmp rs1, rs2
    insn->raw = 0;
    insn->xform.op = CMPop;
    insn->xform.rt = 0;    // really bf & l sub fields of rt we care about
    insn->xform.ra = rs1;
    insn->xform.rb = rs2;
    insn->xform.xo = CMPxop;
    insn++;

    // li rd, 1
    genImmInsn(insn, CALop, rd, 0, 1);
    insn++;

    // b??,a +2
    insn->bform.op = BCop;
    insn->bform.bi = cond;
    insn->bform.bo = mode;
    insn->bform.bd = 2;		// + two instructions */
    insn->bform.aa = 0;
    insn->bform.lk = 0;
    insn++;

    // clr rd
    genImmInsn(insn, CALop, rd, 0, 0);
    base += 4 * sizeof(instruction);
}

// Given a value, load it into a register (two operations, CAU and ORIL)

inline void loadImmIntoReg(instruction *&insn, Register rt, unsigned value)
{
  unsigned high16 = (value & 0xffff0000) >> 16;
  unsigned low16 = (value & 0x0000ffff);
  assert((high16+low16)==value);

  if (high16 == 0x0) { // We can save an instruction by not using CAU
    genImmInsn(insn, CALop, rt, 0, low16);
    insn++;
    return;
  }
  else if (low16 == 0x0) { // we don't have to ORIL the low bits
    genImmInsn(insn, CAUop, rt, 0, high16);
    insn++;
    return;
  }
  else {
    genImmInsn(insn, CAUop, rt, 0, high16);
    insn++;
    genImmInsn(insn, ORILop, rt, rt, low16);
    insn++;
    return;
  }
}

// VG(11/06/01): Shouldn't this be placed in instPoint-power.h?
instPoint::instPoint(int_function *f, const instruction &instr, 
                     const image *, Address adr, bool, instPointType type) :
   instPointBase(type, adr, f), originalInstruction(instr), 
   callIndirect(false)
{
  //printf("Address is 0x%x\n",adr);
  //liveRegisters[0] = -1;
   // inDelaySlot = false;
   // isDelayed = false;
   // callAggregate = false;
}

bool isCallInsn(const instruction i)
{
#define CALLmatch 0x48000001 /* bl */
    
    // Only look for 'bl' instructions for now, although a branch
    // could be a call function, and it doesn't need to set the link
    // register if it is the last function call
    return(isInsnType(i, OPmask | AALKmask, CALLmatch));
}

// Determine if the called function is a "library" function or a "user" function
// This cannot be done until all of the functions have been seen, verified, and
// classified
//
void int_function::checkCallPoints() {
  unsigned int i;
  instPoint *p;
  Address loc_addr;
  if (call_points_have_been_checked) return;

  image *owner = mod_->exec();

  pdvector<instPoint*> non_lib;

  for (i=0; i<calls.size(); ++i) {
      /* check to see where we are calling */
      p = calls[i];
      assert(p);
      
      if(isCallInsn(p->originalInstruction)) {
          loc_addr = p->pointAddr() + (p->originalInstruction.iform.li << 2);
          
          int_function *pdf = owner->findFuncByOffset(loc_addr);
          if (pdf) {
              p->setCallee(pdf);
              non_lib.push_back(p);
          } else {
              p->callIndirect = true;
              p->setCallee(NULL);
              non_lib.push_back(p);
          }
      } else 
      {
          if ((this->prettyName()).suffixed_by("_linkage"))
          {
              // Inter-module call. See note below.
              // Assert the call is a bctr. Otherwise this code is FUBARed.
              p->callIndirect = true;
              p->setCallee(NULL);
              non_lib.push_back(p);
          }
          else 
          {
              // Indirect call -- be conservative, assume it is a call to
              // an unnamed user function
              assert(!p->getCallee()); assert(p->callIndirect);
              p->setCallee(NULL);
              non_lib.push_back(p);
          }
      }
  }
  
  
  calls = non_lib;
  call_points_have_been_checked = true;
}

// TODO we cannot find the called function by address at this point in time
// because the called function may not have been seen.
//
Address int_function::newCallPoint(const Address adr, const instruction instr,
                                  const image *owner, bool &err)
{
    Address ret=adr;
    instPoint *point;
    err = true;
    
    point = new instPoint(this, instr, owner, adr, false, callSite);
    
    if (!isCallInsn(instr)) {
        point->callIndirect = true;
        point->setCallee(NULL);
    } else
        point->callIndirect = false;
    
    // point->callAggregate = false;
    
    calls.push_back(point);
    err = false;
    return ret;
}

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

    if (point->getCallee())
        func = point->getCallee();
    else
        func = point->pointFunc();

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
int getPointCost(process *proc, const instPoint *point)
{
    if (proc->baseMap.defines(point)) {
	return(0);
    } else {
        // How the heck does a base tramp only cost 35 cycles?
        // -- bernat, 4OCT03
        // 35 cycles for base tramp
        // + 70 cyles for MT version (assuming 1 cycle per instruction)
        return(105);
    }
}

/*
 * Given and instruction, relocate it to a new address, patching up
 *   any relative addressing that is present.
 *
 */
#ifdef BPATCH_LIBRARY
void relocateInstruction(instruction *insn, Address origAddr, Address targetAddr)
{
    int newOffset;

    if (isInsnType(*insn, Bmask, Bmatch)) {
      // unconditional pc relative branch.
      newOffset = origAddr  - targetAddr + (insn->iform.li << 2);
      if (ABS(newOffset) >= MAX_BRANCH) {
	logLine("a branch too far\n");
	assert(0);
      } else {
	insn->iform.li = newOffset >> 2;
      }
    } else if (isInsnType(*insn, Bmask, BCmatch)) {
      // conditional pc relative branch.
      newOffset = origAddr - targetAddr + (insn->bform.bd << 2);
      if (ABS(newOffset) >= MAX_CBRANCH) {
	unsigned lk = insn->bform.lk;
	insn->bform.lk = 0;

	if ((insn->bform.bo & BALWAYSmask) == BALWAYScond) {
	    assert(insn->bform.bo == BALWAYScond);
	    generateBranchInsn(insn, newOffset);
	    if (lk) insn->iform.lk = 1;
	} else {
	    // Figure out if the original branch was predicted as taken or not
	    // taken.  We'll set up our new branch to be predicted the same way
	    // the old one was.
	    bool predict_taken;
	    if (insn->bform.bd < 0)
		predict_taken = true;
	    else
		predict_taken = false;
	    if (insn->bform.bo & BPREDICTbit)
		predict_taken = !predict_taken;
	    insn->bform.bo &= ~BPREDICTbit;

	    // Change the branch to move one instruction ahead
	    insn->bform.bd = 2;
	    if (predict_taken) insn->bform.bo |= BPREDICTbit;
	    insn++;
	    assert(insn->raw == NOOPraw);
	    generateBranchInsn(insn, (origAddr + sizeof(instruction)) -
				     (targetAddr + sizeof(instruction)));
	    insn++;
	    assert(insn->raw == NOOPraw);
	    generateBranchInsn(insn, newOffset - 2*sizeof(instruction));
	    if (lk) insn->iform.lk = 1;
	}
      } else {
	insn->bform.bd = newOffset >> 2;
      }
    } else if (insn->iform.op == SVCop) {
      logLine("attempt to relocate a system call\n");
      assert(0);
    } 
    /* The rest of the instructions should be fine as is */
}
#else
void relocateInstruction(instruction *insn, Address origAddr, Address targetAddr)
{
    int newOffset;

    if (isInsnType(*insn, Bmask, Bmatch)) {
      // unconditional pc relative branch.
      newOffset = origAddr  - targetAddr + (insn->iform.li << 2);
      if (ABS(newOffset) > MAX_BRANCH) {
	logLine("a branch too far\n");
	assert(0);
      } else {
	insn->iform.li = newOffset >> 2;
      }
    } else if (isInsnType(*insn, Bmask, BCmatch)) {
      // conditional pc relative branch.
      newOffset = origAddr - targetAddr + (insn->bform.bd << 2);
      if (ABS(newOffset) > MAX_CBRANCH) {
	logLine("a branch too far\n");
	assert(0);
      } else {
	insn->bform.bd = newOffset >> 2;
      }
    } else if (insn->iform.op == SVCop) {
      logLine("attempt to relocate a system call\n");
      assert(0);
    } 
    /* The rest of the instructions should be fine as is */
}
#endif

Register deadRegs[] = {10, REG_GUARD_ADDR, REG_GUARD_VALUE, REG_GUARD_OFFSET};
//deadRegs[0] = 10;
//deadRegs[1] = REG_GUARD_ADDR;
//deadRegs[2] = REG_GUARD_VALUE;
//deadRegs[3] = REG_GUARD_OFFSET;

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
static int saveSPR(instruction *&insn,     //Instruction storage pointer
		   Register    scratchReg, //Scratch register
		   int         sprnum,     //SPR number
		   int         stkOffset)  //Offset from stack pointer
{
  insn->raw = 0;                    //mfspr:  mflr scratchReg
  insn->xform.op = 31;
  insn->xform.rt = scratchReg;
  insn->xform.ra = sprnum & 0x1f;
  insn->xform.rb = (sprnum >> 5) & 0x1f;
  insn->xform.xo = 339;
  insn++;

  insn->raw = 0;                    //st:     st scratchReg, stkOffset(r1)
  insn->dform.op      = 36;
  insn->dform.rt      = scratchReg;
  insn->dform.ra      = 1;
  insn->dform.d_or_si = stkOffset;
  insn++;

  return 2 * sizeof(instruction);
}

    ////////////////////////////////////////////////////////////////////
    //Generates instructions to restore a special purpose register from
    //the stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
static int restoreSPR(instruction *&insn,       //Instruction storage pointer
		      Register      scratchReg, //Scratch register
		      int           sprnum,     //SPR number
		      int           stkOffset)  //Offset from stack pointer
{
  insn->raw = 0;                    //l:      l scratchReg, stkOffset(r1)
  insn->dform.op      = 32;
  insn->dform.rt      = scratchReg;
  insn->dform.ra      = 1;
  insn->dform.d_or_si = stkOffset;
  insn++;

  insn->raw = 0;                    //mtspr:  mtlr scratchReg
  insn->xform.op = 31;
  insn->xform.rt = scratchReg;
  insn->xform.ra = sprnum & 0x1f;
  insn->xform.rb = (sprnum >> 5) & 0x1f;
  insn->xform.xo = 467;
  insn++;

  return 2 * sizeof(instruction);
}
           ////////////////////////////////////////////////////////////////////
	   //Generates instructions to save link register onto stack.
	   //  Returns the number of bytes needed to store the generated
	   //    instructions.
	   //  The instruction storage pointer is advanced the number of 
	   //    instructions generated.
static int saveLR(instruction *&insn,       //Instruction storage pointer
		  Register      scratchReg, //Scratch register
		  int           stkOffset)  //Offset from stack pointer
{
  insn->raw = 0;                    //mfspr:  mflr scratchReg
  insn->xform.op = 31;
  insn->xform.rt = scratchReg;
  insn->xform.ra = 8;
  insn->xform.xo = 339;
  insn++;

  insn->raw = 0;                    //st:     st scratchReg, stkOffset(r1)
  insn->dform.op      = 36;
  insn->dform.rt      = scratchReg;
  insn->dform.ra      = 1;
  insn->dform.d_or_si = stkOffset;
  insn++;

  return 2 * sizeof(instruction);
}

           ////////////////////////////////////////////////////////////////////
           //Generates instructions to restore link register from stack.
           //  Returns the number of bytes needed to store the generated
	   //    instructions.
	   //  The instruction storage pointer is advanced the number of 
	   //    instructions generated.
	   //
static int restoreLR(instruction *&insn,       //Instruction storage pointer
		     Register      scratchReg, //Scratch register
		     int           stkOffset)  //Offset from stack pointer
{
  insn->raw = 0;                    //l:      l scratchReg, stkOffset(r1)
  insn->dform.op      = 32;
  insn->dform.rt      = scratchReg;
  insn->dform.ra      = 1;
  insn->dform.d_or_si = stkOffset;
  insn++;

  insn->raw = 0;                    //mtspr:  mtlr scratchReg
  insn->xform.op = 31;
  insn->xform.rt = scratchReg;
  insn->xform.ra = 8;
  insn->xform.xo = 467;
  insn++;

  return 2 * sizeof(instruction);
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
static int setBRL(instruction *&insn,        //Instruction storage pointer
		  Register      scratchReg,  //Scratch register
		  unsigned      val,         //Value to set link register to
		  unsigned      ti)          //Tail instruction
{
  insn->raw =0;                  //cau:  cau scratchReg, 0, HIGH(val)
  insn->dform.op      = 15;
  insn->dform.rt      = scratchReg;
  insn->dform.ra      = 0;
  insn->dform.d_or_si = ((val >> 16) & 0x0000ffff);
  insn++;

  insn->raw = 0;                 //oril:  oril scratchReg, scratchReg, LOW(val)
  insn->dform.op      = 24;
  insn->dform.rt      = scratchReg;
  insn->dform.ra      = scratchReg;
  insn->dform.d_or_si = (val & 0x0000ffff);
  insn++;
 
  insn->raw = 0;                 //mtspr:  mtlr scratchReg
  insn->xform.op = 31;
  insn->xform.rt = scratchReg;
  insn->xform.ra = 8;
  insn->xform.xo = 467;
  insn++;

  insn->raw = ti;
  insn++;

  return 4 * sizeof(instruction);
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
  instruction  i[8];                          //8 just to be safe
  instruction *t        = i;                  //To avoid side-effects on i
  int          numBytes = val ? setBRL(t, 10, val, BRLraw)
                              : setBRL(t, 10, val, NOOPraw);

  p->writeTextSpace((void *)loc, numBytes, i);
}

    /////////////////////////////////////////////////////////////////////////
    //Generates instructions to save the condition codes register onto stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
static int saveCR(instruction *&insn,       //Instruction storage pointer
		  Register      scratchReg, //Scratch register
		  int           stkOffset)  //Offset from stack pointer
{
  insn->raw = 0;                    //mfcr:  mflr scratchReg
  insn->xform.op = 31;
  insn->xform.rt = scratchReg;
  insn->xform.xo = 19;
  insn++;

  insn->raw = 0;                    //st:     st scratchReg, stkOffset(r1)
  insn->dform.op      = 36;
  insn->dform.rt      = scratchReg;
  insn->dform.ra      = 1;
  insn->dform.d_or_si = stkOffset;
  insn++;

  return 2 * sizeof(instruction);
}

    ///////////////////////////////////////////////////////////////////////////
    //Generates instructions to restore the condition codes register from stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
static int restoreCR(instruction *&insn,       //Instruction storage pointer
		     Register      scratchReg, //Scratch register
		     int           stkOffset)  //Offset from stack pointer
{
  insn->raw = 0;                    //l:      l scratchReg, stkOffset(r1)
  insn->dform.op      = 32;
  insn->dform.rt      = scratchReg;
  insn->dform.ra      = 1;
  insn->dform.d_or_si = stkOffset;
  insn++;

  insn->raw = 0;                    //mtcrf:  scratchReg
  insn->xfxform.op  = 31;
  insn->xfxform.rt  = scratchReg;
  insn->xfxform.spr = 0xff << 1;
  insn->xfxform.xo  = 144;
  insn++;

  return 2 * sizeof(instruction);
}
    /////////////////////////////////////////////////////////////////////////
    //Generates instructions to save the floating point status and control
    //register on the stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
static int saveFPSCR(instruction *&insn,       //Instruction storage pointer
		     Register      scratchReg, //Scratch fp register
		     int           stkOffset)  //Offset from stack pointer
{
  insn->raw = 0;                    //mffs scratchReg
  insn->xform.op = 63;
  insn->xform.rt = scratchReg;
  insn->xform.xo = 583;
  insn++;

  //st:     st scratchReg, stkOffset(r1)
  genImmInsn(insn, STFDop, scratchReg, 1, stkOffset);
  insn++;

  return 2 * sizeof(instruction);
}


    ///////////////////////////////////////////////////////////////////////////
    //Generates instructions to restore the floating point status and control
    //register from the stack.
    //  Returns the number of bytes needed to store the generated
    //    instructions.
    //  The instruction storage pointer is advanced the number of 
    //    instructions generated.
    //
static int restoreFPSCR(instruction *&insn,       //Instruction storage pointer
		        Register      scratchReg, //Scratch fp register
		        int           stkOffset)  //Offset from stack pointer
{
  genImmInsn(insn, LFDop, scratchReg, 1, stkOffset);
  insn++;

  insn->raw = 0;                    //mtfsf:  scratchReg
  insn->xflform.op  = 63;
  insn->xflform.flm = 0xff;
  insn->xflform.frb = scratchReg;
  insn->xflform.xo  = 711;
  insn++;

  return 2 * sizeof(instruction);
}
     //////////////////////////////////////////////////////////////////////////
     //Writes out a `br' instruction
     //
void resetBR(process  *p,    //Process to write instruction into
	     Address   loc)  //Address in process to write into
{
  instruction i;

  i.raw = BRraw;

  p->writeDataSpace((void *)loc, sizeof(instruction), &i);
}

static void saveRegister(instruction *&insn, Address &base, Register reg,
			 int offset)
{
  genImmInsn(insn, STop, reg, 1, offset + reg*GPRSIZE);
  //  bperr("Saving reg %d at 0x%x off the stack\n", reg, offset + reg*GPRSIZE);
  insn++;
  base += sizeof(instruction);
}

// Dest != reg : optimizate away a load/move pair

static void restoreRegister(instruction *&insn, Address &base, Register reg,
			    int dest, int offset)
{
  genImmInsn(insn, Lop, dest, 1, offset + reg*GPRSIZE);
  //bperr( "Loading reg %d (into reg %d) at 0x%x off the stack\n", 
  //  reg, dest, offset + reg*GPRSIZE);
  insn++;
  base += sizeof(instruction);
}

static void restoreRegister(instruction *&insn, Address &base, Register reg,
			    int offset)
{
  restoreRegister(insn, base, reg, reg, offset);
}

static void saveFPRegister(instruction *&insn, Address &base, Register reg,
		    int offset)
{
  genImmInsn(insn, STFDop, reg, 1, offset + reg*FPRSIZE);
  //bperr( "Saving FP reg %d at 0x%x off the stack\n", 
  //  reg, offset + reg*FPRSIZE);
  insn++;
  base += sizeof(instruction);
}

static void restoreFPRegister(instruction *&insn, Address &base, Register reg,
			      int dest, int offset)
{
  genImmInsn(insn, LFDop, dest, 1, offset + reg*FPRSIZE);
  //  bperr("Loading FP reg %d (into %d) at 0x%x off the stack\n", 
  //  reg, dest, offset + reg*FPRSIZE);
  insn++;
  base += sizeof(instruction);
}

static void restoreFPRegister(instruction *&insn, Address &base, Register reg,
		       int offset)
{
  restoreFPRegister(insn, base, reg, reg, offset);
}	

/*
 *            Get the hashed thread ID on the stack and in REG_MT_POS
 *            hashed thread ID * sizeof(int) in REG_GUARD_OFFSET
 *
 * So: call DYNINSTthreadIndex, which returns the INDEX value. This is
 *     done automatically (AST), the rest by hand. Save the INDEX.
 *
 */

unsigned generateMTpreamble(char *insn, Address &base, process *proc)
{
  AstNode *threadPOS;
  pdvector<AstNode *> dummy;
  Register src = Null_Register;

  // registers cleanup
  regSpace->resetSpace();
 
  /* Get the hashed value of the thread */
  if (!proc->multithread_ready()) {
    // Uh oh... we're not ready to build an MT tramp yet. 
      // Probably haven't loaded the RT library
    threadPOS = new AstNode("DYNINSTreturnZero", dummy);
  }
  else 
    threadPOS = new AstNode("DYNINSTthreadIndex", dummy);
  src = threadPOS->generateCode(proc, regSpace, insn,
				base, 
				false, // noCost 
				true); // root node
  instruction *tmp_insn = (instruction *) &(insn[base]);
  if ((src) != REG_MT_POS) {
    // This is always going to happen... we reserve REG_MT_POS, so the
    // code generator will never use it as a destination
    genImmInsn(tmp_insn, ORILop, src, REG_MT_POS, 0);
    tmp_insn++; base+=sizeof(instruction);
  }
  
  regSpace->resetSpace();
  return 0;
}

/*
 * Emit code to push down the stack, AST-generate style
 */

void pushStack(char *i, Address &base)
{
  instruction *insn = (instruction *) ((void*)&i[base]);
  genImmInsn(insn, STUop, REG_SP, REG_SP, -TRAMP_FRAME_SIZE);
  base += sizeof(instruction);
}

void popStack(char *i, Address &base)
{
  instruction *insn = (instruction *) ((void*)&i[base]);
  genImmInsn(insn, CALop, REG_SP, REG_SP, TRAMP_FRAME_SIZE);
  base += sizeof(instruction);
}

/*
 * Save necessary registers on the stack
 * insn, base: for code generation. Offset: regs saved at offset + reg
 * Returns: number of registers saved.
 * Side effects: instruction pointer and base param are shifted to 
 *   next free slot.
 */

unsigned saveGPRegisters(instruction *&insn, Address &base, Address offset, registerSpace *theRegSpace)
{
  unsigned numRegs = 0;
  for(u_int i = 0; i < theRegSpace->getRegisterCount(); i++) {
    registerSlot *reg = theRegSpace->getRegSlot(i);
    if ((reg->number >= 10 && reg->number <= 12) || 
	reg->number == deadRegs[1] || reg->number == deadRegs[2]
	|| reg->number == 0) {
      saveRegister(insn, base, reg->number, offset);
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

unsigned restoreGPRegisters(instruction *&insn, Address &base, Address offset, registerSpace *theRegSpace)
{
  unsigned numRegs = 0;
  for(u_int i = 0; i < theRegSpace->getRegisterCount(); i++) {
    registerSlot *reg = theRegSpace->getRegSlot(i);
    if ((reg->number >= 10 && reg->number <= 12) || 
	reg->number == deadRegs[1] || reg->number == deadRegs[2]
	|| reg->number == 0) 
      {
	restoreRegister(insn, base, reg->number, offset);
	numRegs++;
      }
    else
      {
	if (reg->beenClobbered)
	  {
	    restoreRegister(insn, base, reg->number, TRAMP_GPR_OFFSET);
	    regSpace->unClobberRegister(reg->number);
	    numRegs++;
	  }
      }
  }
  return numRegs;
}

/*
 * Save FPR registers on the stack. (0-13)
 * insn, base: for code generation. Offset: regs saved at offset + reg
 * Returns: number of regs saved.
 */

unsigned saveFPRegisters(instruction *&insn, Address &base, Address offset)
{
  
  unsigned numRegs = 0;
  for (unsigned i = 0; i <= 9; i++) {
    numRegs++;
    saveFPRegister(insn, base, i, offset);
  }
  for (unsigned i = 11; i <= 13; i++) {
    numRegs++;
    saveFPRegister(insn, base, i, offset);
  }
  return numRegs;
  
}

/*
 * Restore FPR registers from the stack. (0-13)
 * insn, base: for code generation. Offset: regs restored from offset + reg
 * Returns: number of regs restored.
 */

unsigned restoreFPRegisters(instruction *&insn, Address &base, 
			    Address offset, registerSpace *theRegSpace)
{
  /*
  unsigned numRegs = 0;
  for (unsigned i = 0; i <= 9; i++) {
    numRegs++;
    restoreFPRegister(insn, base, i, offset);
  }
  for (unsigned i = 11; i <= 13; i++) {
    numRegs++;
    restoreFPRegister(insn, base, i, offset);
  }
  return numRegs;
  */

  unsigned numRegs = 0;
  for(u_int i = 0; i < theRegSpace->getFPRegisterCount(); i++) {
    registerSlot *reg = theRegSpace->getFPRegSlot(i);
    if (reg->number == 10)
      {
	restoreFPRegister(insn, base, reg->number, offset);
	numRegs++;
      }
    else
      {
	if (reg->beenClobbered)
	  {
	    restoreFPRegister(insn, base, reg->number, TRAMP_FPR_OFFSET);
	    regSpace->unClobberFPRegister(reg->number);
	    numRegs++;
	  }
      }
  }
  
  return numRegs;
  
}

/*
 * Save the special purpose registers (for Dyninst conservative tramp)
 * CTR, CR, XER, SPR0, FPSCR
 */

unsigned saveSPRegisters(instruction *&insn, Address &base, Address offset)
{
  base += saveCR(insn, deadRegs[0], offset + STK_CR);
  base += saveSPR(insn, deadRegs[0], SPR_XER, offset + STK_XER);
  base += saveSPR(insn, deadRegs[0], SPR_SPR0, offset + STK_SPR0);
  base += saveFPSCR(insn, 10, offset + STK_FP_CR);
  return 4;
}

/*
 * Restore the special purpose registers (for Dyninst conservative tramp)
 * CTR, CR, XER, SPR0, FPSCR
 */

unsigned restoreSPRegisters(instruction *&insn, Address &base, Address offset)
{
  base += restoreCR(insn, deadRegs[0], offset + STK_CR);
  base += restoreSPR(insn, deadRegs[0], SPR_XER, offset + STK_XER);
  base += restoreSPR(insn, deadRegs[0], SPR_SPR0, offset + STK_SPR0);
  base += restoreFPSCR(insn, 10, offset + STK_FP_CR);
  return 4;
}

/*
 * Restore the LR from the stored location (stack + 12)
 */
void loadOrigLR(instruction *&insn, Address &base)
{
    // We had previously set a "cookie" instructing our stackwalk
    // code to look at the saved LR spot. Remove this value
    // for correctness.
    genImmInsn(insn, CALop, 0, 0, 0x0000);
    insn++; base += sizeof(instruction);
    genImmInsn(insn, STop, 0, 1, 16);
    insn++; base += sizeof(instruction);
    genImmInsn(insn, Lop, 0, 1, 12);
    insn++; base += sizeof(instruction);
    // Save the LR in the right place on the stack for
    // walk purposes
    genImmInsn(insn, STop, 0, 1, 8);
    insn++; base += sizeof(instruction);
    insn->raw = MTLR0raw;
    insn++; base += sizeof(instruction);
}

/*
 * Modify the LR to point to our exit tramp instead of the caller
 */
void stompLRForExit(instruction *&insn, Address &base, Address newReturnAddr)
{
  // First, save the old LR
  insn->raw = MFLR0raw;
  insn++; base += sizeof(instruction);
  genImmInsn(insn, STop, 0, 1, 12);
  insn++; base += sizeof(instruction);
  // We set a "cookie" letting our stackwalk code know that it needs
  // to load stackwalk info from the saved location (SP + 12). 
  genImmInsn(insn, CALop, 0, 0, MODIFIED_LR);
  insn++; base += sizeof(instruction);
  genImmInsn(insn, STop, 0, 1, 16);
  insn++; base += sizeof(instruction);
  // Calculate and store the new LR
  Address newBase = 0;
  emitVload(loadConstOp, newReturnAddr /* src */, 0 /* ignored */,
	    0 /* dest */, (char *)insn, newBase, false, 0 /*ignored */);
  base += newBase; insn += (newBase/sizeof(instruction));
  insn->raw = MTLR0raw;
  insn++; base += sizeof(instruction);
  

}

/*
 * Write out enough noops for the cost section to fill later
 */
void generateCostSection(instruction *&insn, Address &base)
{
  for (unsigned i = 0; i < 6; i++) {
    insn->raw = NOOPraw;
    insn++; base += sizeof(instruction);
  }
}

/*
 * Install a base tramp -- fill calls with nop's for now.
 *
 * This can be called for two different reasons. First, and most often (I 
 * believe) is to write in a new base tramp. We decide which one to use
 * (conservative/regular, guarded/unguarded) and write it in. The template
 * used is then returned, and assumedly stored. 
 *
 * Option 2: rewriting an existing tramp. The tramp has been scrambled
 * for some reason, but the allocated storage still exists. This rewrites
 * the existing tramp. 
 *
 */

unsigned saveRestoreRegistersInBaseTramp(process *proc, trampTemplate * bt, 
					 registerSpace * rs)
{
  instruction iS[25];
  instruction iR[25];
  Address currAddrSave = 0;
  Address currAddrRestore = 0;

  instruction *iSave = (instruction *)iS;
  instruction *iRestore = (instruction *)iR;

  bool change = false;
  

  /* Save and Restore GPRs */
  for(u_int i = 0; i < rs->getRegisterCount(); i++) {
    registerSlot *reg = rs->getRegSlot(i);
    if ((reg->number >= 10 && reg->number <= 12) || 
	reg->number == deadRegs[1] || reg->number == deadRegs[2]
	|| reg->number == 0) 
      continue;
    if(reg->beenClobbered && bt->clobberedGPR[reg->number] == 0)
      {
	saveRegister(iSave, currAddrSave, reg->number, TRAMP_GPR_OFFSET);
	restoreRegister(iRestore, currAddrRestore, reg->number, TRAMP_GPR_OFFSET);
	bt->clobberedGPR[reg->number] = 1;
	change = true;
	bt->totalClobbered = bt->totalClobbered + 1;
      }
  }


  /* Save and Restore FPRs */
  for(u_int i = 0; i < rs->getFPRegisterCount(); i++) {
    registerSlot *reg = rs->getFPRegSlot(i);
    if (reg->number == 10)
      continue;
    if(reg->beenClobbered && bt->clobberedFPR[reg->number] == 0)
      {
	saveFPRegister(iSave, currAddrSave, reg->number, TRAMP_FPR_OFFSET);
	restoreFPRegister(iRestore, currAddrRestore, reg->number, TRAMP_FPR_OFFSET);
	bt->clobberedFPR[reg->number] = 1;
	change = true;
	bt->totalClobbered = bt->totalClobbered + 1;
      }
  }


  if (change)
    {
      proc->writeDataSpace((caddr_t)(bt->baseAddr + bt->saveRegOffset), 
		       currAddrSave, (caddr_t) iS );
      
      proc->writeDataSpace((caddr_t)(bt->baseAddr + bt->saveRegOffset2), 
			 currAddrSave, (caddr_t) iS );

      proc->writeDataSpace((caddr_t)(bt->baseAddr + bt->restRegOffset), 
		       currAddrRestore, (caddr_t) iR );

      proc->writeDataSpace((caddr_t)(bt->baseAddr + bt->restRegOffset2), 
			 currAddrRestore, (caddr_t) iR );

      bt->saveRegOffset += currAddrSave;
      bt->saveRegOffset2 += currAddrSave;
      bt->restRegOffset += currAddrRestore;
      bt->restRegOffset2 += currAddrRestore;

    }

  if (bt->totalClobbered < NUM_LO_REGISTERS &&
      (change || bt->totalClobbered==0))
    {
      generateBranch(proc, bt->baseAddr + bt->saveRegOffset, 
		     bt->baseAddr + bt->saveRegOffset + 
		     (NUM_LO_REGISTERS - bt->totalClobbered)*sizeof(instruction));
      
      generateBranch(proc, bt->baseAddr + bt->saveRegOffset2, 
		     bt->baseAddr + bt->saveRegOffset2 + 
		     (NUM_LO_REGISTERS - bt->totalClobbered)*sizeof(instruction));
      
      generateBranch(proc, bt->baseAddr + bt->restRegOffset, 
		     bt->baseAddr + bt->restRegOffset + 
		     (NUM_LO_REGISTERS - bt->totalClobbered)*sizeof(instruction));
      
      generateBranch(proc, bt->baseAddr + bt->restRegOffset2, 
		     bt->baseAddr + bt->restRegOffset2 + 
		     (NUM_LO_REGISTERS - bt->totalClobbered)*sizeof(instruction));
      
    }

  return 0;
}

trampTemplate* installBaseTramp(const instPoint *location, process *proc,
                                bool trampRecursiveDesired = false,
                                trampTemplate *oldTemplate = NULL,
                                Address exitTrampAddr = 0,
                                Address baseAddr = 0)
{
  trampTemplate *theTemplate;
  Address trampGuardFlagAddr = proc->trampGuardAddr();
  assert(trampGuardFlagAddr);
  registerSpace *theRegSpace;
  if (location->getPointType() == otherPoint)
    theRegSpace = conservativeRegSpace;
  else
    theRegSpace = regSpace;

  theRegSpace->resetClobbers();
  theRegSpace->resetLiveDeadInfo(location->liveRegisters);

  bool wantTrampGuard = true;
  if (oldTemplate && oldTemplate->recursiveGuardPreJumpOffset == -1) {
    wantTrampGuard = false;
  }
  else if (trampRecursiveDesired == true) {
    wantTrampGuard = false;
  }
  if (oldTemplate) // Already have a preferred template type
    theTemplate = oldTemplate;
  else {
      theTemplate = new trampTemplate(location, proc);
      // Initialize some bits of the template
      theTemplate->prevInstru = false;
      theTemplate->postInstru = false;
      theTemplate->prevBaseCost = 0;
      theTemplate->postBaseCost = 0;
      theTemplate->cost = 0;
      theTemplate->pre_minitramps = NULL;
      theTemplate->post_minitramps = NULL;
      theTemplate->clobberedGPR = new int[13];
      theTemplate->clobberedFPR = new int[14];
      theTemplate->totalClobbered = 0;
      theTemplate->isMerged = false;
  }
  
  for (int i = 0; i < 13; i++)
    theTemplate->clobberedGPR[i] = 0;
  for (int i = 0; i < 14; i++)
    theTemplate->clobberedFPR[i] = 0;


  // New model: build the tramp from code segments. Get rid of the
  // tramp-power.S file.
  /*
   * Tramp bits:
   * If at function entry: stomp the LR for function exit
   * If at function exit: restore LR
   * If no instrumentation: pre insn skip
   *   Make new stack frame & save LR
   *   Save all registers (+ possibly conservative ones)
   *   If MT, get the POS (hashed thread ID)
   *   If not, set POS to 0
   *   Pretramp guard code
   *   Minitramp (at last!)
   *   Posttramp guard code
   *   Update costs
   *   Restore
   * Emulated insn
   *   If not exit, do it all over again (post-instrumentation)
   */
  
  // Generate the tramp, figure out how big it is, (possibly)
  // allocate memory for it in the inferior.

  instruction tramp[2048]; // Should be big enough, right?
  instruction *insn = (instruction *)tramp;
  Address currAddr = 0;
  Address spareAddr = 0;

  // Pad the start of the base tramp with a noop -- used for
  // system call trap emulation
#if !defined(AIX_PROC)
  // NOTE: PTRACE ONLY!
  generateNOOP(insn);
  insn++; currAddr += sizeof(instruction);
#endif
  
  // Put in the function stomping at the beginning
  switch (location->getPointType()) {
  case functionExit:
    //bperr( "Base tramp at function entry\n");
    loadOrigLR(insn, currAddr);
    break;
  case functionEntry:
    //bperr( "Base tramp at function exit\n");
    stompLRForExit(insn, currAddr, exitTrampAddr);
    break;
  case otherPoint:
    //bperr( "Base tramp at arbitrary point\n");
    break;
  case callSite:
      //bperr( "Base tramp at function call\n");
    break;
 default:
     assert(0);
     break;
  }

  /////////////////////////////////////////////////////////////
  ////////////////////// PRE //////////////////////////////////
  /////////////////////////////////////////////////////////////


  // Jump past save/restore if there's no instru
  // But we don't know how far to jump. Stick a placeholder here and fix
  // later
  
  theTemplate->skipPreInsOffset = currAddr;
  insn++; currAddr += sizeof(instruction);

  theTemplate->savePreInsOffset = currAddr;

  // Push a new stack frame
  genImmInsn(insn, STUop, REG_SP, REG_SP, -TRAMP_FRAME_SIZE);
  insn++; currAddr += sizeof(instruction);


  // Save registers
  saveGPRegisters(insn, currAddr, TRAMP_GPR_OFFSET, theRegSpace);
  
  saveFPRegister(insn, currAddr, 10, TRAMP_FPR_OFFSET);

  theTemplate->saveRegOffset = currAddr;

  /* Noops, we will fill these in with restore registers */
  for (unsigned i = 0; i < NUM_LO_REGISTERS; i++) {
    generateNOOP(insn);
    insn++; currAddr += sizeof(instruction);
  }

  currAddr += saveLR(insn, deadRegs[0], TRAMP_SPR_OFFSET + STK_LR);

  // Let the stack walk code/anyone else know we're in a base tramp
  // via cookie writing.
  if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite)  {
      // But... we only want to do this if a frame has already been constructed
      // by the function we're instrumenting. Since we can't tell, we assume that
      // if we're at callSite or otherPoint then a frame exists
      unsigned int cookie_value = IN_TRAMP;
      
      // Load previous cookie value
      // add (OR) our cookie
      genImmInsn(insn, CALop, deadRegs[0], 0, cookie_value);
      insn++; currAddr += sizeof(instruction);
      // Store it back
      genImmInsn(insn, STop, deadRegs[0], 1, 16);
      insn++; currAddr += sizeof(instruction);
  }
  

  if (location->getPointType() == otherPoint) {
    // Save special purpose registers
    saveSPRegisters(insn, currAddr, TRAMP_SPR_OFFSET);
    // Save GPR0 here also? 
  }
  
  // Save the count register if we're at an arbitrary point (otherPoint)
  // or a call site (callSite)
  if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite) {
    currAddr += saveSPR(insn, deadRegs[0], SPR_CTR, TRAMP_SPR_OFFSET + STK_CTR);
  }


  if (proc->multithread_capable()) {
    Address scratch = 0;
    //at this point we may have not yet loaded the
    //paradyn lib, so the DYNINSTthreadPos symbols may not
    //be found
    // generateMT places code at insn[addr]. We want it at insn[0],
    // so the scratch address is used.
    // MT: thread POS calculation. If not, stick a 0 here
    generateMTpreamble((char *)insn, scratch, proc);
    // GenerateMT will push forward the currAddr, but not insn
    currAddr += scratch;
    insn = &tramp[currAddr/4];
  }

  if (wantTrampGuard) {
    // Tramp guard
    spareAddr = 0;
    // Load the base address of the guard
    emitVload(loadConstOp, trampGuardFlagAddr, deadRegs[1], deadRegs[1],
	      (char *)insn, spareAddr, false);

    // POS is in REG_MT_POS, we need to multiply by sizeof(unsigned) 
    // and add to the guard address
    if (proc->multithread_capable()) {
      emitImm(timesOp, (Register) REG_MT_POS, (RegValue) sizeof(unsigned), 
	      deadRegs[3], (char *)insn, spareAddr, false, regSpace);
      
      // Add on the offset (for MT)
      emitV(plusOp, deadRegs[1], deadRegs[3], deadRegs[1],
	    (char *)insn, spareAddr, false);
    }
    // Load value
    emitV(loadIndirOp, deadRegs[1], 0, deadRegs[2],
	  (char *)insn, spareAddr, false);
    // Previous functions increased spareAddr, but not insn. Recalculate
    currAddr += spareAddr; insn = &tramp[currAddr/4];
    // Compare with 0
    genImmInsn(insn, CMPIop, 0, deadRegs[2], 1);
    insn++; currAddr += sizeof(instruction);
    
    // And record the offset for a later jump instruction
    theTemplate->recursiveGuardPreJumpOffset = currAddr;
    insn++; currAddr += sizeof(instruction);
    
    // Store 0 in the slot
    spareAddr = 0;
    emitVload(loadConstOp, 0, deadRegs[2], deadRegs[2],
	      (char *)insn, spareAddr, false);
    currAddr += spareAddr; spareAddr = 0; insn = &tramp[currAddr/4];
    // Store
    genImmInsn(insn, STop, deadRegs[2], deadRegs[1], 0);
    insn++; currAddr += sizeof(instruction);
    // Store the flag addr?
    genImmInsn(insn, STop, deadRegs[1], 1, STK_GUARD); // 16 is offset up from SP
    insn++; currAddr += sizeof(instruction);
  }
  else
    theTemplate->recursiveGuardPreJumpOffset = -1;



  // At last we come to the fun bit: the minitramp! Of course,
  // this is just a noop... sigh.
  // Four instructions: generateBranch(null)
  ////////////////////////////////////////////////////////////////
  // Pre mini tramp
  ////////////////////////////////////////////////////////////////
  
  theTemplate->localPreOffset = currAddr;
  currAddr += setBRL(insn, deadRegs[0] /*scratch reg*/, 0, NOOPraw);
  theTemplate->localPreReturnOffset = currAddr;
  
  // Tramp guard shtuff. 
  // Reload the addr of the guard from the stack
  if (wantTrampGuard) {
    genImmInsn(insn, Lop, deadRegs[1], 1, STK_GUARD);
    insn++; currAddr += sizeof(instruction);
    // store immediate
    spareAddr = 0;
    emitVload(loadConstOp, 1, deadRegs[2], deadRegs[2],
  	      (char *)insn, spareAddr, false);
    currAddr += spareAddr; spareAddr = 0; insn = &tramp[currAddr/4];
    
    genImmInsn(insn, STop, deadRegs[2], deadRegs[1], 0);
    insn++; currAddr += sizeof(instruction);
   }
  // Update the cost: a series of noops for now
  theTemplate->updateCostOffset = currAddr; 
  generateCostSection(insn, currAddr);




  // Register restore. 
  theTemplate->restorePreInsOffset = currAddr;


  currAddr += restoreLR(insn, deadRegs[0], TRAMP_SPR_OFFSET + STK_LR);
  if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite) {
    currAddr += restoreSPR(insn, deadRegs[0], SPR_CTR, TRAMP_SPR_OFFSET + STK_CTR);
  }  

  if (location->getPointType() == otherPoint)
    restoreSPRegisters(insn, currAddr, TRAMP_SPR_OFFSET);
  
  
  restoreFPRegister(insn, currAddr, 10, TRAMP_FPR_OFFSET);


  // Not in a base tramp any more
    if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite) {
      unsigned int cookie_value = 0x0;
      genImmInsn(insn, CALop, deadRegs[0], 0, cookie_value);
      insn++; currAddr += sizeof(instruction);
      // Store it back
       genImmInsn(insn, STop, deadRegs[0], 1, 16);
      insn++; currAddr += sizeof(instruction);
   }

  restoreGPRegisters(insn, currAddr, TRAMP_GPR_OFFSET, theRegSpace);
  
  theTemplate->restRegOffset = currAddr;

  /* Noops, we will fill these in with restore registers */
  for (unsigned i = 0; i < NUM_LO_REGISTERS; i++) {
    generateNOOP(insn);
    insn++; currAddr += sizeof(instruction);
  }


  // Pop stack flame, could also be a load indirect R1->R1
  genImmInsn(insn, CALop, REG_SP, REG_SP, TRAMP_FRAME_SIZE);
  insn++; currAddr += sizeof(instruction);



  // FINALLY, we get to the original instruction! W00T!
  generateNOOP(insn);
  theTemplate->emulateInsOffset = currAddr;  
  insn++; currAddr += sizeof(instruction);
  // The fun isn't over. If we're dyninst, we stick four more
  // noops here to cover all eventualities
  if (location->getPointType() == otherPoint)
    for (unsigned i = 0; i < 4; i++) {
      generateNOOP(insn);
      insn++; currAddr += sizeof(instruction);
    }
  
  // That was it? I somehow expected... more. Now handle the post-bits

  /////////////////////////////////////////////////////////////
  ////////////////////// POST /////////////////////////////////
  /////////////////////////////////////////////////////////////


  // Jump past save/restore if there's no instru
  // But we don't know how far to jump. Stick a placeholder here and fix
  // later
  theTemplate->skipPostInsOffset = currAddr;
  insn++; currAddr += sizeof(instruction);

  theTemplate->savePostInsOffset = currAddr;

  // Push a new stack frame
  genImmInsn(insn, STUop, REG_SP, REG_SP, -TRAMP_FRAME_SIZE);
  insn++; currAddr += sizeof(instruction);

  // Save registers
  saveGPRegisters(insn, currAddr, TRAMP_GPR_OFFSET, theRegSpace);

  saveFPRegister(insn, currAddr, 10, TRAMP_FPR_OFFSET);

  theTemplate->saveRegOffset2 = currAddr; 

  /* Noops are place-holders, will put save registers here later */
  for (unsigned i = 0; i < NUM_LO_REGISTERS; i++) {
    generateNOOP(insn);
    insn++; currAddr += sizeof(instruction);
  }
  
    
  currAddr += saveLR(insn, deadRegs[0], TRAMP_SPR_OFFSET + STK_LR);

  // Let the stack walk code/anyone else know we're in a base tramp
  // via cookie writing.
  if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite)  {
      unsigned int cookie_value = IN_TRAMP;
      
      // Load previous cookie value
      // add (OR) our cookie
      genImmInsn(insn, CALop, deadRegs[0], 0, cookie_value);
      insn++; currAddr += sizeof(instruction);
      // Store it back
      genImmInsn(insn, STop, deadRegs[0], 1, 16);
      insn++; currAddr += sizeof(instruction);
  }
  
  //saveFPRegisters(insn, currAddr, TRAMP_FPR_OFFSET);
 
    
  if (location->getPointType() == otherPoint) {
    // Save special purpose registers    
    saveSPRegisters(insn, currAddr, TRAMP_SPR_OFFSET);
    // Save GPR0 here also? 
  }
  
  // Save the count register if we're at an arbitrary point (otherPoint)
  // or a call site (ipFuncCallPoint)
  if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite) {
    currAddr += saveSPR(insn, deadRegs[0], SPR_CTR, TRAMP_SPR_OFFSET + STK_CTR);
  }

  // MT: thread POS calculation. If not, stick a 0 here
  if (proc->multithread_capable()) {
    Address scratch = 0;
    //be found
    // generateMT places code at insn[addr]. We want it at insn[0],
    // so the scratch address is used.
    // MT: thread POS calculation. If not, stick a 0 here
    generateMTpreamble((char *)insn, scratch, proc);
    // GenerateMT will push forward the currAddr, but not insn
    currAddr += scratch;
    insn = &tramp[currAddr/4];
  }
  
  // Tramp guard
  // Load the base address of the guard
  if (wantTrampGuard) {
    spareAddr = 0;
    emitVload(loadConstOp, trampGuardFlagAddr, deadRegs[1], deadRegs[1],
	      (char *)insn, spareAddr, false);
    // POS is in REG_MT_POS, we need to multiply by sizeof(unsigned) 
    // and add to the guard address
    if (proc->multithread_capable()) {
      emitImm(timesOp, (Register) REG_MT_POS, (RegValue) sizeof(unsigned), 
	      deadRegs[3], (char *)insn, spareAddr, false, regSpace);
      
      // Add on the offset (for MT)
      emitV(plusOp, deadRegs[1], deadRegs[3], deadRegs[1],
	    (char *)insn, spareAddr, false);
    }
    // Load value
    emitV(loadIndirOp, deadRegs[1], 0, deadRegs[2],
	  (char *)insn, spareAddr, false);
    // Previous functions increased spareAddr, but not insn. Recalculate
    currAddr += spareAddr; insn = &tramp[currAddr/4];
    // Compare with 1
    genImmInsn(insn, CMPIop, 0, deadRegs[2], 1);
    insn++; currAddr += sizeof(instruction);
    
    // And record the offset for a later jump instruction
    theTemplate->recursiveGuardPostJumpOffset = currAddr;
    insn++; currAddr += sizeof(instruction);
    
    // Store 0 in the slot
    spareAddr = 0;
    emitVload(loadConstOp, 0, deadRegs[2], deadRegs[2], 
	      (char *)insn, spareAddr, false);
    currAddr += spareAddr; spareAddr = 0; insn = &tramp[currAddr/4];
    // Store
    genImmInsn(insn, STop, deadRegs[2], deadRegs[1], 0);
    insn++; currAddr += sizeof(instruction);
    // Store the flag addr?
    genImmInsn(insn, STop, deadRegs[1], 1, STK_GUARD); // 16 is offset up from SP
    insn++; currAddr += sizeof(instruction);
  }
  else
    theTemplate->recursiveGuardPostJumpOffset = -1;
  // At last we come to the fun bit: the minitramp! Of course,
  // this is just a noop... sigh.
  // Four instructions: generateBranch(null)
  theTemplate->localPostOffset = currAddr;
  currAddr += setBRL(insn, deadRegs[0] /*scratch reg*/, 0, NOOPraw);
  theTemplate->localPostReturnOffset = currAddr;

  // Tramp guard shtuff. 
  // Reload the addr of the guard from the stack
  if (wantTrampGuard) {
    genImmInsn(insn, Lop, deadRegs[1], 1, STK_GUARD);
    insn++; currAddr += sizeof(instruction);
    // store immediate
    spareAddr = 0;
    emitVload(loadConstOp, 1, deadRegs[2], deadRegs[2], 
	      (char *)insn, spareAddr, false);
    currAddr += spareAddr; spareAddr = 0; insn = &tramp[currAddr/4];
    
    genImmInsn(insn, STop, deadRegs[2], deadRegs[1], 0);
    insn++; currAddr += sizeof(instruction);
  }

  // Register restore. 
  theTemplate->restorePostInsOffset = currAddr;

  currAddr += restoreLR(insn, deadRegs[0], TRAMP_SPR_OFFSET + STK_LR);
  if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite) {
    currAddr += restoreSPR(insn, deadRegs[0], SPR_CTR, TRAMP_SPR_OFFSET + STK_CTR);
  }  

  if (location->getPointType() == otherPoint)
    restoreSPRegisters(insn, currAddr, TRAMP_SPR_OFFSET);
  
  
  //restoreFPRegisters(insn, currAddr, TRAMP_FPR_OFFSET, theRegSpace);

  restoreFPRegister(insn, currAddr, 10, TRAMP_FPR_OFFSET);

  // Not in a base tramp any more
  if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite)  {
      unsigned int cookie_value = 0x0;
      genImmInsn(insn, CALop, deadRegs[0], 0, cookie_value);
      insn++; currAddr += sizeof(instruction);
      // Store it back
      genImmInsn(insn, STop, deadRegs[0], 1, 16);
      insn++; currAddr += sizeof(instruction);
  }


  restoreGPRegisters(insn, currAddr, TRAMP_GPR_OFFSET, theRegSpace);

  theTemplate->restRegOffset2 = currAddr;

  /* Noops, we will fill these in with restore registers */
  for (unsigned i = 0; i < NUM_LO_REGISTERS; i++) {
    generateNOOP(insn);
    insn++; currAddr += sizeof(instruction);
  }


  // Pop stack flame, could also be a load indirect R1->R1
  genImmInsn(insn, CALop, REG_SP, REG_SP, TRAMP_FRAME_SIZE);
  insn++; currAddr += sizeof(instruction);

  theTemplate->returnInsOffset = currAddr;
  if (location->getPointType() == functionExit)
    insn->raw = BRraw;
  // Otherwise will get handled below

  insn++; currAddr += sizeof(instruction);

  theTemplate->size = currAddr;

  ////////////////////////////////////////////////////////////////////
  //////////////////////// FIXUP /////////////////////////////////////
  ////////////////////////////////////////////////////////////////////

  // We have the following jumps that need to be written in:
  // 1) skipPreIns (from theTemplate->skipPreInsOffset to
  //      emulateInsOffset
  // 2) skipPostIns (from theTemplate->skipPostInsOffset to
  //      returnInsOffset
  // 3) preTrampGuard (recursiveGuardPreJumpOffset to restorePreInsn)
  // 4) postTrampGuard (recursiveGuardPostJumpOffset to restorePostInsn)
  // 5) Return jump (returnInsOffset to location->pointAddr)
  // 6) Emulated instruction (well, kinda)

  // 1
  instruction *temp = &(tramp[theTemplate->skipPreInsOffset/sizeof(instruction)]);
  generateBranchInsn(temp, theTemplate->emulateInsOffset - theTemplate->skipPreInsOffset);
  // 2
  temp = &(tramp[theTemplate->skipPostInsOffset/sizeof(instruction)]);
  generateBranchInsn(temp, theTemplate->returnInsOffset - theTemplate->skipPostInsOffset);
  // 3
  if (theTemplate->recursiveGuardPreJumpOffset != -1) {
    temp = &(tramp[theTemplate->recursiveGuardPreJumpOffset/sizeof(instruction)]);
    Address offset = ((theTemplate->restorePreInsOffset -
		       theTemplate->recursiveGuardPreJumpOffset))/4;
    // Set it up by hand
    temp->raw = 0; temp->bform.op = BCop; // conditional
    temp->bform.bo = BFALSEcond; // Branch if false
    temp->bform.bi = EQcond; temp->bform.bd = offset; 
    temp->bform.aa = 0; temp->bform.lk = 0;

  }
  // 4
  if (theTemplate->recursiveGuardPostJumpOffset != -1) {
    temp = &(tramp[theTemplate->recursiveGuardPostJumpOffset/sizeof(instruction)]);
    Address offset = ((theTemplate->restorePostInsOffset -
		      theTemplate->recursiveGuardPostJumpOffset))/4;
    // Set it up by hand
    temp->raw = 0; temp->bform.op = BCop; // conditional
    temp->bform.bo = BFALSEcond; // Branch if false
    temp->bform.bi = EQcond; temp->bform.bd = offset; 
    temp->bform.aa = 0; temp->bform.lk = 0;
  }
  // Okay, actually build this sucker.
  bool isReinstall = true;
#if defined(bug_aix_proc_broken_fork)
      // We need the base tramp to be in allocated heap space, not scavenged
      // text space, because scavenged text space is _not_ copied on fork.
  if (!baseAddr) {
      if (location->pointFunc()->prettyName() == pdstring("__fork")) {
          baseAddr = proc->inferiorMalloc(theTemplate->size, (inferiorHeapType) (textHeap | dataHeap),
                                          location->absPointAddr(proc));
          isReinstall = false;
      }
  }
#endif      
  if (!baseAddr) {
      
      baseAddr = proc->inferiorMalloc(theTemplate->size, anyHeap,
                                      location->absPointAddr(proc));
      isReinstall = false;
  }
  // InferiorMalloc can ignore our "hints" when necessary, but that's 
  // bad here because we don't have a fallback position
  if (DISTANCE(location->absPointAddr(proc), baseAddr) > MAX_BRANCH)
    {
      bperr( "Instrumentation point %x too far from tramp location %x, trying to instrument function %s\n",
	      (unsigned) location->absPointAddr(proc), (unsigned) baseAddr,
              location->pointFunc()->prettyName().c_str());
      bperr( "If this is a library function or the instrumentation point address begins with 0xd...\n");
      bperr( "please ensure that the text instrumentation library (libDyninstText.a) is compiled and\n");
      bperr( "in one of the directories contained in the LIBPATH environment variable. If it is not,\n");
      bperr( "contact paradyn@cs.wisc.edu for further assistance.\n");
      
      delete theTemplate;
      return NULL;
    }
  
  theTemplate->baseAddr = baseAddr;
  theTemplate->costAddr = baseAddr + theTemplate->updateCostOffset;

  // Relocate now
  if (location->getPointType() != functionExit) {
    // If it was func return, we slapped in a noop earlier.
    temp = &(tramp[theTemplate->emulateInsOffset/sizeof(instruction)]);
    // Copy in the original instruction
    *temp = location->originalInstruction;
    relocateInstruction(temp, location->absPointAddr(proc), 
                        baseAddr + theTemplate->emulateInsOffset);
  }

  // 5
  if (location->getPointType() != functionExit) {
    temp = &(tramp[theTemplate->returnInsOffset/sizeof(instruction)]);
    generateBranchInsn(temp, location->absPointAddr(proc) + sizeof(instruction)
                       - (baseAddr + theTemplate->returnInsOffset));
    /*
    bperr( "Installing jump at 0%x, offset 0x%x, going to 0x%x\n",
	    (baseAddr + theTemplate->returnInsOffset), 
	    location->absPointAddr(proc) - (baseAddr + theTemplate->returnInsOffset),
	    location->absPointAddr(proc));
    */
  }

#if defined(DEBUG)
  bperr( "------------\n");
  for (int i = 0; i < (theTemplate->size/4); i++)
    bperr( "0x%x,\n", tramp[i].raw);
  bperr( "------------\n");
  bperr( "\n\n\n");
#endif
#if defined(DEBUG)
  fprintf(stderr, "Dumping template: localPre %d, preReturn %d, localPost %d, postReturn %d\n",
	  theTemplate->localPreOffset, theTemplate->localPreReturnOffset, 
	  theTemplate->localPostOffset, theTemplate->localPostReturnOffset);
  fprintf(stderr, "returnIns %d, skipPre %d, skipPost %d, emulate %d, updateCost %d\n",
	  theTemplate->returnInsOffset, theTemplate->skipPreInsOffset, theTemplate->skipPostInsOffset,
	  theTemplate->emulateInsOffset,theTemplate->updateCostOffset);
  fprintf(stderr,"savePre %d, restorePre %d, savePost %d, restorePost %d, guardPre %d, guardPost %d\n",
	  theTemplate->savePreInsOffset, theTemplate->restorePreInsOffset, theTemplate->savePostInsOffset, theTemplate->restorePostInsOffset, theTemplate->recursiveGuardPreJumpOffset,
	  theTemplate->recursiveGuardPostJumpOffset);
  fprintf(stderr, "baseAddr = 0x%x\n", theTemplate->baseAddr);
#endif
  // TDODO cast
  proc->writeDataSpace((caddr_t)baseAddr, theTemplate->size, (caddr_t) tramp);
#if defined(DEBUG)
  fprintf(stderr,  "Base tramp from 0x%x to 0x%x, from 0x%x in function %s\n",
          baseAddr, baseAddr+theTemplate->size, location->absPointAddr(proc), 
          location->pointFunc()->prettyName().c_str());
#endif


  if (!isReinstall) {
      proc->addCodeRange(baseAddr, theTemplate);
      return theTemplate;
  }
  else 
      return NULL;
}






/* This installs the base tramp for the merged tramps 
   Most of the code is the same as the installBaseTramp
   function above.  As I add more stuff, I will go through
   and clean up/comment the code more. -Nick
*/
trampTemplate* installMergedBaseTramp(const instPoint *location, process *proc,
				      char * mTCode, Address count,
				      registerSpace *regS,
				      callWhen when,
				      bool trampRecursiveDesired = false,
				      trampTemplate *oldTemplate = NULL,
				      Address exitTrampAddr = 0,
				      Address baseAddr = 0)
{
  trampTemplate *theTemplate;
  Address trampGuardFlagAddr = proc->trampGuardAddr();
  assert(trampGuardFlagAddr);
  registerSpace *theRegSpace;
  if (location->getPointType() == otherPoint)
    theRegSpace = conservativeRegSpace;
  else
    theRegSpace = regSpace;

  theRegSpace->resetClobbers();

  bool wantTrampGuard = true;
  
  if (trampRecursiveDesired == true) {
    wantTrampGuard = false;
  }
  
  theTemplate = new trampTemplate(location, proc);
  
  // Initialize some bits of the template
  theTemplate->prevInstru = false;
  theTemplate->postInstru = false;
  theTemplate->prevBaseCost = 0;
  theTemplate->postBaseCost = 0;
  theTemplate->cost = 0;
  theTemplate->pre_minitramps = NULL;
  theTemplate->post_minitramps = NULL;
  theTemplate->totalClobbered = 0;
  theTemplate->isMerged = true;
    
  // Generate the tramp, figure out how big it is, (possibly)
  // allocate memory for it in the inferior.

  instruction tramp[2048]; // Should be big enough, right?
  instruction *insn = (instruction *)tramp;
  Address currAddr = 0;
  Address spareAddr = 0;

  // Pad the start of the base tramp with a noop -- used for
  // system call trap emulation
#if !defined(AIX_PROC)
  // NOTE: PTRACE ONLY!
  generateNOOP(insn);
  insn++; currAddr += sizeof(instruction);
#endif
  
  // Put in the function stomping at the beginning
  switch (location->getPointType()) {
  case functionExit:
    //bperr( "Base tramp at function entry\n");
    loadOrigLR(insn, currAddr);
    break;
  case functionEntry:
    //bperr( "Base tramp at function exit\n");
    stompLRForExit(insn, currAddr, exitTrampAddr);
    break;
  case otherPoint:
    //bperr( "Base tramp at arbitrary point\n");
    break;
  case callSite:
      //bperr( "Base tramp at function call\n");
    break;
 default:
     assert(0);
     break;
  }

  /////////////////////////////////////////////////////////////
  ////////////////////// PRE //////////////////////////////////
  /////////////////////////////////////////////////////////////


  // Jump past save/restore if there's no instru
  // But we don't know how far to jump. Stick a placeholder here and fix
  // later
  
  theTemplate->skipPreInsOffset = currAddr;
  insn++; currAddr += sizeof(instruction);

  theTemplate->savePreInsOffset = currAddr;

  // Push a new stack frame
  genImmInsn(insn, STUop, REG_SP, REG_SP, -TRAMP_FRAME_SIZE);
  insn++; currAddr += sizeof(instruction);


  // Save registers
  saveGPRegisters(insn, currAddr, TRAMP_GPR_OFFSET, theRegSpace);
  
  saveFPRegister(insn, currAddr, 10, TRAMP_FPR_OFFSET);

  //theTemplate->saveRegOffset = currAddr;


   //Save GPR
  for(u_int i = 0; i < regS->getRegisterCount(); i++) {
    registerSlot *reg = regS->getRegSlot(i);
    if ((reg->number >= 10 && reg->number <= 12) || 
	reg->number == deadRegs[1] || reg->number == deadRegs[2]
	|| reg->number == 0) 
      continue;
    if(reg->beenClobbered)
      {
	saveRegister(insn, currAddr, reg->number, TRAMP_GPR_OFFSET);
      }
  }

  // Save FPRs 
  for(u_int i = 0; i < regS->getFPRegisterCount(); i++) {
    registerSlot *reg = regS->getFPRegSlot(i);
    if (reg->number == 10)
      continue;
    if(reg->beenClobbered)
      {
	saveFPRegister(insn, currAddr, reg->number, TRAMP_FPR_OFFSET);
      }
  }

  currAddr += saveLR(insn, 10, TRAMP_SPR_OFFSET + STK_LR);

  // Let the stack walk code/anyone else know we're in a base tramp
  // via cookie writing.
  if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite)  {
      // But... we only want to do this if a frame has already been constructed
      // by the function we're instrumenting. Since we can't tell, we assume that
      // if we're at callSite or otherPoint then a frame exists
      unsigned int cookie_value = IN_TRAMP;
      
      // Load previous cookie value
      // add (OR) our cookie
      genImmInsn(insn, CALop, 10, 0, cookie_value);
      insn++; currAddr += sizeof(instruction);
      // Store it back
      genImmInsn(insn, STop, 10, 1, 16);
      insn++; currAddr += sizeof(instruction);
  }
  

  if (location->getPointType() == otherPoint) {
    // Save special purpose registers
      saveSPRegisters(insn, currAddr, TRAMP_SPR_OFFSET);
    // Save GPR0 here also? 
  }
  
  // Save the count register if we're at an arbitrary point (otherPoint)
  // or a call site (callSite)
  if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite) {
      currAddr += saveSPR(insn, 10, SPR_CTR, TRAMP_SPR_OFFSET + STK_CTR);
  }


  if (proc->multithread_capable()) {
    Address scratch = 0;
    //at this point we may have not yet loaded the
    //paradyn lib, so the DYNINSTthreadPos symbols may not
    //be found
    // generateMT places code at insn[addr]. We want it at insn[0],
    // so the scratch address is used.
    // MT: thread POS calculation. If not, stick a 0 here
    generateMTpreamble((char *)insn, scratch, proc);
    // GenerateMT will push forward the currAddr, but not insn
    currAddr += scratch;
    insn = &tramp[currAddr/4];
  }
  if (wantTrampGuard) {
    // Tramp guard
    spareAddr = 0;
    // Load the base address of the guard
    emitVload(loadConstOp, trampGuardFlagAddr, deadRegs[1], deadRegs[1],
	      (char *)insn, spareAddr, false);

    // POS is in REG_MT_POS, we need to multiply by sizeof(unsigned) 
    // and add to the guard address
    if (proc->multithread_capable()) {
      emitImm(timesOp, (Register) REG_MT_POS, (RegValue) sizeof(unsigned), 
	      deadRegs[3], (char *)insn, spareAddr, false, regSpace);
      
      // Add on the offset (for MT)
      emitV(plusOp, deadRegs[1], deadRegs[3], deadRegs[1],
	    (char *)insn, spareAddr, false);
    }
    // Load value
    emitV(loadIndirOp, deadRegs[1], 0, deadRegs[2],
	  (char *)insn, spareAddr, false);
    // Previous functions increased spareAddr, but not insn. Recalculate
    currAddr += spareAddr; insn = &tramp[currAddr/4];
    // Compare with 0
    genImmInsn(insn, CMPIop, 0, deadRegs[2], 1);
    insn++; currAddr += sizeof(instruction);
    
    // And record the offset for a later jump instruction
    theTemplate->recursiveGuardPreJumpOffset = currAddr;
    insn++; currAddr += sizeof(instruction);
    
    // Store 0 in the slot
    spareAddr = 0;
    emitVload(loadConstOp, 0, deadRegs[2], deadRegs[2],
	      (char *)insn, spareAddr, false);
    currAddr += spareAddr; spareAddr = 0; insn = &tramp[currAddr/4];
    // Store
    genImmInsn(insn, STop, deadRegs[2], deadRegs[1], 0);
    insn++; currAddr += sizeof(instruction);
    // Store the flag addr?
    genImmInsn(insn, STop, deadRegs[1], 1, STK_GUARD); // 16 is offset up from SP
    insn++; currAddr += sizeof(instruction);
  }
  else
    theTemplate->recursiveGuardPreJumpOffset = -1;




  // At last we come to the fun bit: the minitramp! Of course,
  // this is just a noop... sigh.
  // Four instructions: generateBranch(null)
  ////////////////////////////////////////////////////////////////
  // Pre mini tramp
  ////////////////////////////////////////////////////////////////

  //instruction *insn = (instruction *) ((void*)&baseInsn[base]);

  //printf("Insn - 0x%lx, MTCode - 0x%lx, Count - %d\n",insn, mTCode,count);

  
  //int * currI = (int *)mTCode;
  //for (unsigned i = 0; i < count/4; i++)
  //  {
      //printf("0x%lx\n",*currI);
  //  currI += sizeof(int);
      //printf( "0x%lx,\n", mTCode[i]);
  //  }


  memcpy(insn,mTCode,count-8);
  currAddr += (count-8);
  insn = &tramp[currAddr/4];

  //theTemplate->localPreOffset = currAddr;
  // currAddr += setBRL(insn, 10 // scratch reg //, 0, NOOPraw);
  //theTemplate->localPreReturnOffset = currAddr;
  // Tramp guard shtuff. 
  
  // Reload the addr of the guard from the stack
  if (wantTrampGuard) {
    genImmInsn(insn, Lop, deadRegs[1], 1, STK_GUARD);
    insn++; currAddr += sizeof(instruction);
    // store immediate
    spareAddr = 0;
    emitVload(loadConstOp, 1, deadRegs[2], deadRegs[2],
	      (char *)insn, spareAddr, false);
    currAddr += spareAddr; spareAddr = 0; insn = &tramp[currAddr/4];
    
    genImmInsn(insn, STop, deadRegs[2], deadRegs[1], 0);
    insn++; currAddr += sizeof(instruction);
  }
  
  // Update the cost: a series of noops for now
  //theTemplate->updateCostOffset = currAddr; 
  //generateCostSection(insn, currAddr);
  
  
  currAddr += restoreLR(insn, 10, TRAMP_SPR_OFFSET + STK_LR);
  if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite) {
      currAddr += restoreSPR(insn, 10, SPR_CTR, TRAMP_SPR_OFFSET + STK_CTR);
  }  

  if (location->getPointType() == otherPoint)
      restoreSPRegisters(insn, currAddr, TRAMP_SPR_OFFSET);
  
  
  restoreFPRegister(insn, currAddr, 10, TRAMP_FPR_OFFSET);


  // Not in a base tramp any more
  if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite) {
      unsigned int cookie_value = 0x0;
      genImmInsn(insn, CALop, 10, 0, cookie_value);
      insn++; currAddr += sizeof(instruction);
      // Store it back
      genImmInsn(insn, STop, 10, 1, 16);
      insn++; currAddr += sizeof(instruction);
  }

  restoreGPRegisters(insn, currAddr, TRAMP_GPR_OFFSET, theRegSpace);
  
  //theTemplate->restRegOffset = currAddr;


  //Save GPR
  for(u_int i = 0; i < regS->getRegisterCount(); i++) {
    registerSlot *reg = regS->getRegSlot(i);
    if ((reg->number >= 10 && reg->number <= 12) || 
	reg->number == deadRegs[1] || reg->number == deadRegs[2]
	|| reg->number == 0) 
      continue;
    if(reg->beenClobbered)
      {
	restoreRegister(insn, currAddr, reg->number, TRAMP_GPR_OFFSET);
      }
  }

  // Save FPRs 
  for(u_int i = 0; i < regS->getFPRegisterCount(); i++) {
    registerSlot *reg = regS->getFPRegSlot(i);
    if (reg->number == 10)
      continue;
    if(reg->beenClobbered)
      {
	restoreFPRegister(insn, currAddr, reg->number, TRAMP_FPR_OFFSET);
      }
  }





  // Noops, we will fill these in with restore registers 
  //for (unsigned i = 0; i < NUM_LO_REGISTERS; i++) {
  //  generateNOOP(insn);
  //  insn++; currAddr += sizeof(instruction);
  // }


  // Pop stack flame, could also be a load indirect R1->R1
  genImmInsn(insn, CALop, REG_SP, REG_SP, TRAMP_FRAME_SIZE);
  insn++; currAddr += sizeof(instruction);



  // FINALLY, we get to the original instruction! W00T!
  generateNOOP(insn);
  theTemplate->emulateInsOffset = currAddr;  
  insn++; currAddr += sizeof(instruction);
  // The fun isn't over. If we're dyninst, we stick four more
  // noops here to cover all eventualities
  if (location->getPointType() == otherPoint)
    for (unsigned i = 0; i < 4; i++) {
      generateNOOP(insn);
      insn++; currAddr += sizeof(instruction);
    }
  
  // That was it? I somehow expected... more. Now handle the post-bits

  /////////////////////////////////////////////////////////////
  ////////////////////// POST /////////////////////////////////
  /////////////////////////////////////////////////////////////


  // Jump past save/restore if there's no instru
  // But we don't know how far to jump. Stick a placeholder here and fix
  // later
  theTemplate->skipPostInsOffset = currAddr;
  insn++; currAddr += sizeof(instruction);

  theTemplate->savePostInsOffset = currAddr;

  // Push a new stack frame
  genImmInsn(insn, STUop, REG_SP, REG_SP, -TRAMP_FRAME_SIZE);
  insn++; currAddr += sizeof(instruction);

  // Save registers
  saveGPRegisters(insn, currAddr, TRAMP_GPR_OFFSET, theRegSpace);

  saveFPRegister(insn, currAddr, 10, TRAMP_FPR_OFFSET);

  //theTemplate->saveRegOffset2 = currAddr; 

  //Save GPR
  for(u_int i = 0; i < regS->getRegisterCount(); i++) {
    registerSlot *reg = regS->getRegSlot(i);
    if ((reg->number >= 10 && reg->number <= 12) || 
	reg->number == deadRegs[1] || reg->number == deadRegs[2]
	|| reg->number == 0) 
      continue;
    if(reg->beenClobbered)
      {
	saveRegister(insn, currAddr, reg->number, TRAMP_GPR_OFFSET);
      }
  }

  // Save FPRs 
  for(u_int i = 0; i < regS->getFPRegisterCount(); i++) {
    registerSlot *reg = regS->getFPRegSlot(i);
    if (reg->number == 10)
      continue;
    if(reg->beenClobbered)
      {
	saveFPRegister(insn, currAddr, reg->number, TRAMP_FPR_OFFSET);
      }
  }





  // Noops are place-holders, will put save registers here later //
  //for (unsigned i = 0; i < NUM_LO_REGISTERS; i++) {
  //  generateNOOP(insn);
  //  insn++; currAddr += sizeof(instruction);
  //}
  
    
  currAddr += saveLR(insn, 10, TRAMP_SPR_OFFSET + STK_LR);

  // Let the stack walk code/anyone else know we're in a base tramp
  // via cookie writing.
  if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite)  {
      unsigned int cookie_value = IN_TRAMP;
      
      // Load previous cookie value
      // add (OR) our cookie
      genImmInsn(insn, CALop, 10, 0, cookie_value);
      insn++; currAddr += sizeof(instruction);
      // Store it back
      genImmInsn(insn, STop, 10, 1, 16);
      insn++; currAddr += sizeof(instruction);
  }
  
    
  if (location->getPointType() == otherPoint) {
    // Save special purpose registers    
    saveSPRegisters(insn, currAddr, TRAMP_SPR_OFFSET);
    // Save GPR0 here also? 
  }
  
  // Save the count register if we're at an arbitrary point (otherPoint)
  // or a call site (ipFuncCallPoint)
  if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite) {
      currAddr += saveSPR(insn, 10, SPR_CTR, TRAMP_SPR_OFFSET + STK_CTR);
  }

  // MT: thread POS calculation. If not, stick a 0 here
  if (proc->multithread_capable()) {
    Address scratch = 0;
    //be found
    // generateMT places code at insn[addr]. We want it at insn[0],
    // so the scratch address is used.
    // MT: thread POS calculation. If not, stick a 0 here
    generateMTpreamble((char *)insn, scratch, proc);
    // GenerateMT will push forward the currAddr, but not insn
    currAddr += scratch;
    insn = &tramp[currAddr/4];
  }
  
  // Tramp guard
  // Load the base address of the guard
  if (wantTrampGuard) {
    spareAddr = 0;
    emitVload(loadConstOp, trampGuardFlagAddr, deadRegs[1], deadRegs[1],
	      (char *)insn, spareAddr, false);
    // POS is in REG_MT_POS, we need to multiply by sizeof(unsigned) 
    // and add to the guard address
    if (proc->multithread_capable()) {
      emitImm(timesOp, (Register) REG_MT_POS, (RegValue) sizeof(unsigned), 
	      deadRegs[3], (char *)insn, spareAddr, false, regSpace);
      
      // Add on the offset (for MT)
      emitV(plusOp, deadRegs[1], deadRegs[3], deadRegs[1],
	    (char *)insn, spareAddr, false);
    }
    // Load value
    emitV(loadIndirOp, deadRegs[1], 0, deadRegs[2],
	  (char *)insn, spareAddr, false);
    // Previous functions increased spareAddr, but not insn. Recalculate
    currAddr += spareAddr; insn = &tramp[currAddr/4];
    // Compare with 1
    genImmInsn(insn, CMPIop, 0, deadRegs[2], 1);
    insn++; currAddr += sizeof(instruction);
    
    // And record the offset for a later jump instruction
    theTemplate->recursiveGuardPostJumpOffset = currAddr;
    insn++; currAddr += sizeof(instruction);
    
    // Store 0 in the slot
    spareAddr = 0;
    emitVload(loadConstOp, 0, deadRegs[2], deadRegs[2], 
	      (char *)insn, spareAddr, false);
    currAddr += spareAddr; spareAddr = 0; insn = &tramp[currAddr/4];
    // Store
    genImmInsn(insn, STop, deadRegs[2], deadRegs[1], 0);
    insn++; currAddr += sizeof(instruction);
    // Store the flag addr?
    genImmInsn(insn, STop, deadRegs[1], 1, STK_GUARD); // 16 is offset up from SP
    insn++; currAddr += sizeof(instruction);
  }
  else
    theTemplate->recursiveGuardPostJumpOffset = -1;
  // At last we come to the fun bit: the minitramp! Of course,
  // this is just a noop... sigh.
  // Four instructions: generateBranch(null)
  
  //int * currI = (int *)mTCode;
  //for (unsigned i = 0; i < count/4; i++)
  //  {
  //    currI += sizeof(int);
  //  }

  memcpy(insn,mTCode,count-8);
  currAddr += (count-8);
  insn = &tramp[currAddr/4];
  
  //theTemplate->localPostOffset = currAddr;
  //currAddr += setBRL(insn, 10 // scratch reg , 0, NOOPraw);
  //theTemplate->localPostReturnOffset = currAddr;

  // Tramp guard shtuff. 
  // Reload the addr of the guard from the stack
  if (wantTrampGuard) {
    genImmInsn(insn, Lop, deadRegs[1], 1, STK_GUARD);
    insn++; currAddr += sizeof(instruction);
    // store immediate
    spareAddr = 0;
    emitVload(loadConstOp, 1, deadRegs[2], deadRegs[2], 
	      (char *)insn, spareAddr, false);
    currAddr += spareAddr; spareAddr = 0; insn = &tramp[currAddr/4];
    
    genImmInsn(insn, STop, deadRegs[2], deadRegs[1], 0);
    insn++; currAddr += sizeof(instruction);
  }

  // Register restore. 
  theTemplate->restorePostInsOffset = currAddr;

  currAddr += restoreLR(insn, 10, TRAMP_SPR_OFFSET + STK_LR);
  if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite) {
      currAddr += restoreSPR(insn, 10, SPR_CTR, TRAMP_SPR_OFFSET + STK_CTR);
  }  

  if (location->getPointType() == otherPoint)
    restoreSPRegisters(insn, currAddr, TRAMP_SPR_OFFSET);
  
  
  restoreFPRegister(insn, currAddr, 10, TRAMP_FPR_OFFSET);

  // Not in a base tramp any more
  if (location->getPointType() == otherPoint ||
      location->getPointType() == callSite)  {
      unsigned int cookie_value = 0x0;
      genImmInsn(insn, CALop, 10, 0, cookie_value);
      insn++; currAddr += sizeof(instruction);
      // Store it back
      genImmInsn(insn, STop, 10, 1, 16);
      insn++; currAddr += sizeof(instruction);
  }


  restoreGPRegisters(insn, currAddr, TRAMP_GPR_OFFSET, theRegSpace);

  //theTemplate->restRegOffset2 = currAddr;


  //TODO: Restore GPR/FPR Here

 //Save GPR
  for(u_int i = 0; i < regS->getRegisterCount(); i++) {
    registerSlot *reg = regS->getRegSlot(i);
    if ((reg->number >= 10 && reg->number <= 12) || 
	reg->number == deadRegs[1] || reg->number == deadRegs[2]
	|| reg->number == 0) 
      continue;
    if(reg->beenClobbered)
      {
	restoreRegister(insn, currAddr, reg->number, TRAMP_GPR_OFFSET);
      }
  }

  // Save FPRs 
  for(u_int i = 0; i < regS->getFPRegisterCount(); i++) {
    registerSlot *reg = regS->getFPRegSlot(i);
    if (reg->number == 10)
      continue;
    if(reg->beenClobbered)
      {
	restoreFPRegister(insn, currAddr, reg->number, TRAMP_FPR_OFFSET);
      }
  }








  // Noops, we will fill these in with restore registers //
  //for (unsigned i = 0; i < NUM_LO_REGISTERS; i++) {
  //  generateNOOP(insn);
  //  insn++; currAddr += sizeof(instruction);
  //}


  // Pop stack flame, could also be a load indirect R1->R1
  genImmInsn(insn, CALop, REG_SP, REG_SP, TRAMP_FRAME_SIZE);
  insn++; currAddr += sizeof(instruction);

  theTemplate->returnInsOffset = currAddr;
  if (location->getPointType() == functionExit)
    insn->raw = BRraw;
  // Otherwise will get handled below

  insn++; currAddr += sizeof(instruction);

  theTemplate->size = currAddr;

  ////////////////////////////////////////////////////////////////////
  //////////////////////// FIXUP /////////////////////////////////////
  ////////////////////////////////////////////////////////////////////

  // We have the following jumps that need to be written in:
  // 1) skipPreIns (from theTemplate->skipPreInsOffset to
  //      emulateInsOffset
  // 2) skipPostIns (from theTemplate->skipPostInsOffset to
  //      returnInsOffset
  // 3) preTrampGuard (recursiveGuardPreJumpOffset to restorePreInsn)
  // 4) postTrampGuard (recursiveGuardPostJumpOffset to restorePostInsn)
  // 5) Return jump (returnInsOffset to location->pointAddr)
  // 6) Emulated instruction (well, kinda)

  // 1
  instruction *temp = &(tramp[theTemplate->skipPreInsOffset/sizeof(instruction)]);
  generateBranchInsn(temp, theTemplate->emulateInsOffset - theTemplate->skipPreInsOffset);
  // 2
  temp = &(tramp[theTemplate->skipPostInsOffset/sizeof(instruction)]);
  generateBranchInsn(temp, theTemplate->returnInsOffset - theTemplate->skipPostInsOffset);
  // 3
  if (theTemplate->recursiveGuardPreJumpOffset != -1) {
    temp = &(tramp[theTemplate->recursiveGuardPreJumpOffset/sizeof(instruction)]);
    Address offset = ((theTemplate->restorePreInsOffset -
		       theTemplate->recursiveGuardPreJumpOffset))/4;
    // Set it up by hand
    temp->raw = 0; temp->bform.op = BCop; // conditional
    temp->bform.bo = BFALSEcond; // Branch if false
    temp->bform.bi = EQcond; temp->bform.bd = offset; 
    temp->bform.aa = 0; temp->bform.lk = 0;

  }
  // 4
  if (theTemplate->recursiveGuardPostJumpOffset != -1) {
    temp = &(tramp[theTemplate->recursiveGuardPostJumpOffset/sizeof(instruction)]);
    Address offset = ((theTemplate->restorePostInsOffset -
		      theTemplate->recursiveGuardPostJumpOffset))/4;
    // Set it up by hand
    temp->raw = 0; temp->bform.op = BCop; // conditional
    temp->bform.bo = BFALSEcond; // Branch if false
    temp->bform.bi = EQcond; temp->bform.bd = offset; 
    temp->bform.aa = 0; temp->bform.lk = 0;
  }
  // Okay, actually build this sucker.
  bool isReinstall = true;
#if defined(bug_aix_proc_broken_fork)
      // We need the base tramp to be in allocated heap space, not scavenged
      // text space, because scavenged text space is _not_ copied on fork.
  if (!baseAddr) {
      if (location->pointFunc()->prettyName() == pdstring("__fork")) {
          baseAddr = proc->inferiorMalloc(theTemplate->size, (inferiorHeapType) (textHeap | dataHeap),
                                          location->absPointAddr(proc));
          isReinstall = false;
      }
  }
#endif      
  if (!baseAddr) {
      
      baseAddr = proc->inferiorMalloc(theTemplate->size, anyHeap,
                                      location->absPointAddr(proc));
      isReinstall = false;
  }
  // InferiorMalloc can ignore our "hints" when necessary, but that's 
  // bad here because we don't have a fallback position
  if (DISTANCE(location->absPointAddr(proc), baseAddr) > MAX_BRANCH)
    {
      bperr( "Instrumentation point %x too far from tramp location %x, trying to instrument function %s\n",
	      (unsigned) location->absPointAddr(proc), (unsigned) baseAddr,
              location->pointFunc()->prettyName().c_str());
      bperr( "If this is a library function or the instrumentation point address begins with 0xd...\n");
      bperr( "please ensure that the text instrumentation library (libDyninstText.a) is compiled and\n");
      bperr( "in one of the directories contained in the LIBPATH environment variable. If it is not,\n");
      bperr( "contact paradyn@cs.wisc.edu for further assistance.\n");
      
      delete theTemplate;
      return NULL;
    }
  
  theTemplate->baseAddr = baseAddr;
  theTemplate->costAddr = baseAddr + theTemplate->updateCostOffset;

  // Relocate now
  if (location->getPointType() != functionExit) {
    // If it was func return, we slapped in a noop earlier.
    temp = &(tramp[theTemplate->emulateInsOffset/sizeof(instruction)]);
    // Copy in the original instruction
    *temp = location->originalInstruction;
    relocateInstruction(temp, location->absPointAddr(proc), 
                        baseAddr + theTemplate->emulateInsOffset);
  }

  // 5
  if (location->getPointType() != functionExit) {
    temp = &(tramp[theTemplate->returnInsOffset/sizeof(instruction)]);
    generateBranchInsn(temp, location->absPointAddr(proc) + sizeof(instruction)
                       - (baseAddr + theTemplate->returnInsOffset));
    //
    //bperr( "Installing jump at 0%x, offset 0x%x, going to 0x%x\n",
    //(baseAddr + theTemplate->returnInsOffset), 
    //	    location->absPointAddr(proc) - (baseAddr + theTemplate->returnInsOffset),
    //    location->absPointAddr(proc));
    //
  }

#if defined(DEBUG)
  bperr( "------------\n");
  for (int i = 0; i < (theTemplate->size/4); i++)
    bperr( "0x%x,\n", tramp[i].raw);
  bperr( "------------\n");
  bperr( "\n\n\n");
#endif
#if defined(DEBUG)
  fprintf(stderr, "Dumping template: localPre %d, preReturn %d, localPost %d, postReturn %d\n",
	  theTemplate->localPreOffset, theTemplate->localPreReturnOffset, 
	  theTemplate->localPostOffset, theTemplate->localPostReturnOffset);
  fprintf(stderr, "returnIns %d, skipPre %d, skipPost %d, emulate %d, updateCost %d\n",
	  theTemplate->returnInsOffset, theTemplate->skipPreInsOffset, theTemplate->skipPostInsOffset,
	  theTemplate->emulateInsOffset,theTemplate->updateCostOffset);
  fprintf(stderr,"savePre %d, restorePre %d, savePost %d, restorePost %d, guardPre %d, guardPost %d\n",
	  theTemplate->savePreInsOffset, theTemplate->restorePreInsOffset, theTemplate->savePostInsOffset, theTemplate->restorePostInsOffset, theTemplate->recursiveGuardPreJumpOffset,
	  theTemplate->recursiveGuardPostJumpOffset);
  fprintf(stderr, "baseAddr = 0x%x\n", theTemplate->baseAddr);
#endif
  // TODO cast
  proc->writeDataSpace((caddr_t)baseAddr, theTemplate->size, (caddr_t) tramp);
#if defined(DEBUG)
  fprintf(stderr,  "Base tramp from 0x%x to 0x%x, from 0x%x in function %s\n",
          baseAddr, baseAddr+theTemplate->size, location->absPointAddr(proc), 
          location->pointFunc()->prettyName().c_str());
#endif


Address atAddr;
// Kill the skip instructions
  if (when == callPreInsn) {
    if (theTemplate->prevInstru == false) {
      atAddr = theTemplate->baseAddr + theTemplate->skipPreInsOffset;
      theTemplate->prevInstru = true;
      generateNoOp(proc, atAddr);
    }
  }
  else {
    if (theTemplate->postInstru == false) {
      atAddr = theTemplate->baseAddr + theTemplate->skipPostInsOffset; 
      theTemplate->cost += theTemplate->postBaseCost;
      theTemplate->postInstru = true;
      generateNoOp(proc, atAddr);
    }
  }
  

  if (!isReinstall) {
      proc->addCodeRange(baseAddr, theTemplate);
      return theTemplate;
  }
  else 
      return NULL;
}


void generateNoOp(process *proc, Address addr)
{
    instruction insn;

    /* fill with no-op */
    /* ori 0,0,0 */
    insn.raw = 0;
    insn.dform.op = ORILop;

    proc->writeTextWord((caddr_t)addr, insn.raw);
}


     //////////////////////////////////////////////////////////////////////////
     //Returns `true' if a correct base trampoline exists; `false' otherwise
     //
bool baseTrampExists(process  *p,         //Process to check into
		     Address   baseAddr)  //Address at which to start checking
{
  if ((! p) || (! baseAddr)) return false;

  int data = ptrace(PT_READ_I, p->getPid(), (int *)baseAddr, 0, 0);
  if ((data == 0) || (data == -1))        //Bad instruction or ptrace error
    return false;
  else
    return true;
}



trampTemplate *findOrInstallBaseTramp(process *proc, 
                                      instPoint *&location,
                                      returnInstance *&retInstance,
                                      bool trampRecursiveDesired,
                                      bool /*noCost*/,
                                      bool& /*deferred*/,
                                      bool /*allowTrap*/)
{
  trampTemplate *ret = NULL;
  retInstance = NULL;

  // find fills in the second argument if it's found
  if (!proc->baseMap.find(location, ret)) {
      // Need to create base tramp
      // Note: entry/exit tramps are ALWAYS installed in pairs.
      // If one does not exist, the other MUST NOT exist.
      if((location->getPointType() == functionEntry) ||
         (location->getPointType() == functionExit)) {
          
          pdvector<instPoint *> exit_pts = location->pointFunc()->funcExits(proc);
          const instPoint* exitP = exit_pts[exit_pts.size() -1];
          assert(!proc->baseMap.defines(exitP));
          trampTemplate *exitT = installBaseTramp(exitP, proc, trampRecursiveDesired);
          
          if (!exitT) return NULL;
          proc->baseMap[exitP] = exitT;

          const instPoint* entryP = location->pointFunc()->funcEntry(proc);
          assert(!proc->baseMap.defines(entryP));
          trampTemplate *entryT = installBaseTramp(entryP, proc, trampRecursiveDesired,
                                                   NULL, exitT->baseAddr);
          if (!entryT) {
              // Delete the exit
              deleteBaseTramp(proc, exitT);
              return NULL;
          }          
          proc->baseMap[entryP] = entryT;

          if(location->getPointType() == functionExit) {
              ret = exitT;
          }
          else
              ret = entryT;
          
          instruction *insn = new instruction;
          
          generateBranchInsn(insn,
                             entryT->baseAddr - entryP->absPointAddr(proc));
          retInstance =
             new returnInstance(1, (instruction *)insn, sizeof(instruction),
                                entryP->absPointAddr(proc),
                                sizeof(instruction));          
      } else {
          // Arbitrary or function call site

          ret = installBaseTramp(location, proc, trampRecursiveDesired);
          if(!ret) return NULL;

          proc->baseMap[location] = ret;

          instruction *insn = new instruction;
          generateBranchInsn(insn,
                             ret->baseAddr - location->absPointAddr(proc));

          retInstance = new returnInstance(1, (instruction *)insn, 
                                           sizeof(instruction),
                                           location->absPointAddr(proc),
                                           sizeof(instruction));
      }
  }
  
  return(ret);
}




trampTemplate *installMergedTramp(process *proc, 
				  instPoint *&location,
				  char * i, Address count, 
				  registerSpace * regS, 
				  callWhen when,
				  returnInstance *&retInstance,
				  bool trampRecursiveDesired,
				  bool /*noCost*/,
				  bool& /*deferred*/,
				  bool /*allowTrap*/)
{
  trampTemplate *ret = NULL;
  retInstance = NULL;

  
  if((location->getPointType() == functionEntry) ||
     (location->getPointType() == functionExit)) {
    
    pdvector<instPoint *> exit_pts = location->pointFunc()->funcExits(proc);
    const instPoint* exitP = exit_pts[exit_pts.size() -1];
    assert(!proc->baseMap.defines(exitP));
    trampTemplate *exitT = installMergedBaseTramp(exitP, proc, i, count, regS, when, 
						  trampRecursiveDesired);
    
    if (!exitT) return NULL;
    proc->baseMap[exitP] = exitT;
    
    const instPoint* entryP = location->pointFunc()->funcEntry(proc);
    assert(!proc->baseMap.defines(entryP));
    trampTemplate *entryT = installMergedBaseTramp(entryP, proc, i, count, regS, when,
						   trampRecursiveDesired,
						   NULL, exitT->baseAddr);
    if (!entryT) {
      // Delete the exit
      deleteBaseTramp(proc, exitT);
      return NULL;
    }          
    proc->baseMap[entryP] = entryT;
    
    if(location->getPointType() == functionExit) {
      ret = exitT;
    }
    else
      ret = entryT;
    
    instruction *insn = new instruction;
    
    generateBranchInsn(insn,
		       entryT->baseAddr - entryP->absPointAddr(proc));
    retInstance =
      new returnInstance(1, (instruction *)insn, sizeof(instruction),
			 entryP->absPointAddr(proc),	
			 sizeof(instruction));          
  } else {
    
    // Arbitrary or function call site
    
    ret = installMergedBaseTramp(location, proc,i,count, regS, when, trampRecursiveDesired);
    if(!ret) return NULL;
    
    proc->baseMap[location] = ret;
    
    instruction *insn = new instruction;
    generateBranchInsn(insn,
		       ret->baseAddr - location->absPointAddr(proc));
    
    retInstance = new returnInstance(1, (instruction *)insn, 
				     sizeof(instruction),
				     location->absPointAddr(proc),
				     sizeof(instruction));
    }
  return(ret);
}


/*
 * Install a single tramp.
 *
 */
void installTramp(miniTrampHandle *mtHandle, process *proc, 
                  char *code, int codeSize)
{
    totalMiniTramps++;
    insnGenerated += codeSize/sizeof(int);
    // TODO cast
    proc->writeDataSpace((caddr_t)mtHandle->miniTrampBase, codeSize, code);
    
    Address atAddr;
    trampTemplate *bT = mtHandle->baseTramp;

    // Kill the skip instructions
    if (mtHandle->when == callPreInsn) {
        if (bT->prevInstru == false) {
            //bperr( "Base addr %x, skipPre %x\n",
            //  bT->baseAddr,
            //  bT->skipPreInsOffset);
            atAddr = bT->baseAddr + bT->skipPreInsOffset;

            bT->cost += bT->prevBaseCost;
            bT->prevInstru = true;
            generateNoOp(proc, atAddr);
        }
    }
    else {
        if (bT->postInstru == false) {
            atAddr = bT->baseAddr + bT->skipPostInsOffset; 
            bT->cost += bT->postBaseCost;
            bT->postInstru = true;
            generateNoOp(proc, atAddr);
        }
    }
}

/*
 * change the insn at addr to be a branch to newAddr.
 *   Used to add multiple tramps to a point.
 */
void generateBranch(process *proc, Address fromAddr, Address newAddr)
{
    int disp;
    instruction insn;

    disp = newAddr-fromAddr;
    generateBranchInsn(&insn, disp);
    // TODO cast
    proc->writeTextWord((caddr_t)fromAddr, insn.raw);
}

int callsTrackedFuncP(instPoint *point)
{
   if (point->callIndirect) {
#ifdef notdef
      // TODO this won't compile now
      // it's rare to call a library function as a parameter.
      sprintf(errorLine, "*** Warning call indirect\n from %s %s (addr %d)\n",
              point->func->file->fullName, point->func->prettyName,
              point->pointAddr());
      logLine(errorLine);
#endif
      return(true);
   } else {
      if (point->getCallee()) {
         return(true);
      } else {
         return(false);
      }
   }
}

void generateBreakPoint(instruction &insn) { // instP.h
    insn.raw = BREAK_POINT_INSN;
}

void generateIllegalInsn(instruction &insn) { // instP.h
   insn.raw = 0; // I think that on power, this is an illegal instruction (someone check this please) --ari
}


bool rpcMgr::emitInferiorRPCheader(void *insnPtr, Address &baseBytes) 
{

  // We save daemon-side...
  instruction *tmp = (instruction *)insnPtr;
  // A miracle of casting...
  instruction *insn = (instruction *) (&tmp[baseBytes/sizeof(instruction)]);
  
  // Make a stack frame
  genImmInsn(insn, STUop, REG_SP, REG_SP, -TRAMP_FRAME_SIZE);
  insn++; baseBytes += sizeof(instruction);
/*  
  // Save registers
  saveGPRegisters(insn, baseBytes, TRAMP_GPR_OFFSET, conservativeRegSpace);
*/
  return true;
}


bool rpcMgr::emitInferiorRPCtrailer(void *insnPtr, Address &baseBytes,
                                    unsigned &breakOffset,
                                    bool stopForResult,
                                    unsigned &stopForResultOffset,
                                    unsigned &justAfter_stopForResultOffset) {

  // The sequence we want is: (restore), trap, illegal,
  // where (restore) undoes anything done in emitInferiorRPCheader(), above.
  instruction *tmp = (instruction *)insnPtr;
  // A miracle of casting...
  instruction *insn = (instruction *) (&tmp[baseBytes/sizeof(instruction)]);
  
  if (stopForResult) {
    generateBreakPoint(*insn);
    stopForResultOffset = baseBytes;
    baseBytes += sizeof(instruction); insn++;
    justAfter_stopForResultOffset = baseBytes;
  }

/*
  // We save daemon-side...
  restoreGPRegisters(insn, baseBytes, TRAMP_GPR_OFFSET, conservativeRegSpace);
*/
  // Pop the stack
  genImmInsn(insn, CALop, REG_SP, REG_SP, TRAMP_FRAME_SIZE);
  insn++; baseBytes += sizeof(instruction);

  // but the code doesn't like justAfter == breakOffset. Insert
  // a noop
  generateNOOP(insn);
  insn++; baseBytes += sizeof(instruction);

  generateBreakPoint(*insn);
  breakOffset = baseBytes;
  baseBytes += sizeof(instruction); insn++;
  
  // And just to make sure that we don't continue, we put an illegal
  // insn here:
  generateIllegalInsn(*insn);
  baseBytes += sizeof(instruction); insn++;
    
  return true;
}

/*
 * return the function asociated with a point.
 *
 *     If the point is a funcation call, and we know the function being called,
 *          then we use that.  Otherwise it is the function that contains the
 *          point.
 *  
 *   This is done to return a better idea of which function we are using.
 */
int_function* getFunction(instPoint *point)
{
    return(point->getCallee() ? point->getCallee() : point->pointFunc());
}


void emitImm(opCode op, Register src1, RegValue src2imm, Register dest, 
                 char *i, Address &base, bool noCost, registerSpace * rs)
{
        //bperr("emitImm(op=%d,src=%d,src2imm=%d,dest=%d)\n",
        //        op, src1, src2imm, dest);
        instruction *insn = (instruction *) ((void*)&i[base]);
        int iop=-1;
        int result=-1;
	switch (op) {
	    // integer ops
	    case plusOp:
		iop = CALop;
		genImmInsn(insn, iop, dest, src1, src2imm);
		base += sizeof(instruction);
		return;
		break;

	    case minusOp:
		iop = SIop;
		genImmInsn(insn, iop, dest, src1, src2imm);
		base += sizeof(instruction);
		return;
		break;

	    case timesOp:
               if (isPowerOf2(src2imm,result) && (result<32)) {
                  generateLShift(insn, src1, result, dest);           
                  base += sizeof(instruction);
                  return;
	        }
                else {
                  Register dest2 = regSpace->allocateRegister(i, base, noCost);
		  rs->clobberRegister(dest2);
                  emitVload(loadConstOp, src2imm, dest2, dest2, i, base, noCost);
                  emitV(op, src1, dest2, dest, i, base, noCost);
                  regSpace->freeRegister(dest2);
                  return;
		}
		break;

	    case divOp:
                if (isPowerOf2(src2imm,result) && (result<32)) {
                  generateRShift(insn, src1, result, dest);           
                  base += sizeof(instruction);
                  return;
	        }
		else {
                  Register dest2 = regSpace->allocateRegister(i, base, noCost);
		  rs->clobberRegister(dest2);
                  emitVload(loadConstOp, src2imm, dest2, dest2, i, base, noCost);
                  emitV(op, src1, dest2, dest, i, base, noCost);
                  regSpace->freeRegister(dest2);
                  return;
		}
		break;

	    // Bool ops
	    case orOp:
		iop = ORILop;
		// For some reason, the destField is 2nd for ORILop and ANDILop
		genImmInsn(insn, iop, src1, dest, src2imm);
		base += sizeof(instruction);
		return;
		break;

	    case andOp:
		iop = ANDILop;
		// For some reason, the destField is 2nd for ORILop and ANDILop
		genImmInsn(insn, iop, src1, dest, src2imm);
		base += sizeof(instruction);
		return;
		break;
	
	    default:
                Register dest2 = regSpace->allocateRegister(i, base, noCost);
		rs->clobberRegister(dest2);
                emitVload(loadConstOp, src2imm, dest2, dest2, i, base, noCost);
                emitV(op, src1, dest2, dest, i, base, noCost);
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
		      char *iPtr, Address &base, 
		      const pdvector<AstNode *> &operands, 
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
         Register dummyReg = rs->allocateRegister(iPtr, base, noCost);
	 rs->clobberRegister(dummyReg);
         srcs.push_back(dummyReg);
         instruction *insn = (instruction *) ((void*)&iPtr[base]);
         genImmInsn(insn, CALop, dummyReg, 0, 0);
         base += sizeof(instruction);
      }
      srcs.push_back(operands[u]->generateCode_phase2(proc, rs, iPtr, base, 
                                                      false, ifForks, location));
      //bperr( "Generated operand %d, base %d\n", u, base);
   }
  
   // generateCode can shift the instruction pointer, so reset insn
   instruction *insn = (instruction *) ((void*)&iPtr[base]);
   pdvector<int> savedRegs;
  
   //  Save the link register.
   // mflr r0
   insn->raw = MFLR0raw;
   insn++;
   base += sizeof(instruction);
   // Register 0 is actually the link register, now. However, since we
   // don't want to overwrite the LR slot, save it as "register 0"
   saveRegister(insn, base, 0, FUNC_CALL_SAVE);
   // Add 0 to the list of saved registers
   savedRegs.push_back(0);
  
   // Save register 2 (TOC)
   saveRegister(insn, base, 2, FUNC_CALL_SAVE);
   savedRegs.push_back(2);

   if(proc->multithread_capable()) {
      // save REG_MT_POS
      saveRegister(insn,base,REG_MT_POS, FUNC_CALL_SAVE);
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
         // saveRegister(insn,base,reg->number,8+(46*4));
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
            saveRegister(insn, base, reg->number, FUNC_CALL_SAVE);
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
      //	saveRegister(insn,base,u+3, TRAMP_GPR_OFFSET);

      genImmInsn(insn, ORILop, srcs[u], u+3, 0);
      insn++;
      base += sizeof(instruction);
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
	 genImmInsn(insn, ORILop, scratchReg[u], u+3, 0);
	 insn++;
	 base += sizeof(instruction);
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
	     scratch = rs->allocateRegister(iPtr, base, noCost);
	     rs->clobberRegister(scratch);
	     genImmInsn(insn, ORILop, u+3, scratch, 0);
	     insn++;
	     base += sizeof(instruction);
	     rs->freeRegister(u+3);
	     scratchReg[whichSource] = scratch;
	     hasSourceBeenCopied = true;

	     genImmInsn(insn, ORILop, srcs[u], u+3, 0);
	     insn++;
	     base += sizeof(instruction);
	     rs->freeRegister(srcs[u]);
	   }
	 else
	   {
	     genImmInsn(insn, ORILop, srcs[u], u+3, 0);
	     insn++;
	     base += sizeof(instruction);
	     rs->freeRegister(srcs[u]);
	     rs->clobberRegister(u+3);
	   }
       }

     
   }

   // Set up the new TOC value

   genImmInsn(insn, CAUop, 2, 0, HIGH(toc_anchor));
   insn++;
   base += sizeof(instruction);
   genImmInsn(insn, ORILop, 2, 2, LOW(toc_anchor));
   insn++;
   base += sizeof(instruction);

   // generate a branch to the subroutine to be called.
   // load r0 with address, then move to link reg and branch and link.
  
   // Set the upper half of the link register
   genImmInsn(insn, CAUop, 0, 0, HIGH(callee_addr));
   insn++;
   base += sizeof(instruction);
  
   // Set lower half
   genImmInsn(insn, ORILop, 0, 0, LOW(callee_addr));
   insn++;
   base += sizeof(instruction);
  
   // Move to link register
   insn->raw = MTLR0raw;
   insn++;
   base += sizeof(instruction);
   
   // Linear Scan on the functions to see which registers get clobbered
     
   //clobberAll = true;

   
   int_function *funcc;
   
   codeRange *range = proc->findCodeRangeByAddress(callee_addr);
   
   if (range)
     {
       funcc = range->is_function();
       //cout << "Function Name is " << funcc->prettyName() << endl;
       
       if (funcc)
	 {
	   pdmodule * mod = funcc->pdmod();
	   
	   InstrucIter ah(funcc, proc, mod);
	   
	   //while there are still instructions to check for in the
	   //address space of the function
	   
	   while (ah.hasMore()) {
	     InstrucIter inst(ah);
	     Address pos = ah++;
	     
	     if (inst.isA_RT_WriteInstruction())
	       {
		 
		 //cout<<"Writes to RT for ";
		 //inst.printOpCode();
		 //cout<<" and register ";
		 //cout<< inst.getRTValue() << endl;
		 
		 rs->clobberRegister(inst.getRTValue());
	       }
	     if (inst.isA_RA_WriteInstruction())
	       {
		 
		 //cout<<"Writes to RA for ";
		 //inst.printOpCode();
		 //cout<<" and register ";
		 //cout<<inst.getRAValue()<<endl;
		 
		 rs->clobberRegister(inst.getRAValue());
	       }
	     if (inst.isA_FRT_WriteInstruction())
	       {
		 
		 //cout<<"Writes to FRT for ";
		 //inst.printOpCode();
		 //cout<<" and register ";
		 //cout<<inst.getRTValue()<<endl;
		 
		 rs->clobberFPRegister(inst.getRTValue());
	       }
	     if (inst.isA_FRA_WriteInstruction())
	       {
		 
		 //cout<<"Writes to FRA for ";
		 //inst.printOpCode();
		 //cout<<" and register ";
		 //cout<<inst.getRAValue()<<endl;
		 
		 rs->clobberFPRegister(inst.getRAValue());
	       }
	     if (inst.isAReturnInstruction()){
	     }
	     else if (inst.isACondBranchInstruction()){
	     }
	     else if (inst.isAJumpInstruction()){
	     }
	     else if (inst.isACallInstruction()){
	       clobberAll = true;
	     }
	     else if (inst.isAnneal()){
	     }
	     else{
	     }
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
   
   insn->raw = BRLraw;
   insn++;
   base += sizeof(instruction);

   // get a register to keep the return value in.
   Register retReg = rs->allocateRegister(iPtr, base, noCost);
   rs->clobberRegister(retReg);

   // allocateRegister can generate code. Reset insn
   insn = (instruction *) ((void*)&iPtr[base]);

   // put the return value from register 3 to the newly allocated register.
   genImmInsn(insn, ORILop, 3, retReg, 0);
   insn++;
   base += sizeof(instruction);
   
   // Restore floating point registers
   //restoreFPRegisters(insn, base, TRAMP_FPR_OFFSET);
   
   //Restore the registers used to save  parameters
   /*
   for(u_int i = 0; i < rs->getRegisterCount(); i++) {
   registerSlot *reg = rs->getRegSlot(i);
   if (reg->beenClobbered) {
   restoreRegister(insn, base, reg->number, TRAMP_GPR_OFFSET);
   regSpace->unClobberRegister(reg->number);
   }
   }
   */
   
   // restore saved registers.
   for (u_int ui = 0; ui < savedRegs.size(); ui++) {
      restoreRegister(insn,base,savedRegs[ui],FUNC_CALL_SAVE);
   }
  
   // mtlr	0 (aka mtspr 8, rs) = 0x7c0803a6
   insn->raw = MTLR0raw;
   insn++;
   base += sizeof(instruction);
   /*
     insn = (instruction *) iPtr;
     for (unsigned foo = initBase/4; foo < base/4; foo++)
     bperr( "0x%x,\n", insn[foo].raw);
   */  
   // return value is the register with the return value from the called function

   return(retReg);
}

 
Address emitA(opCode op, Register src1, Register /*src2*/, Register dest,
	      char *baseInsn, Address &base, bool /*noCost*/)
{
    //bperr("emitA(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

    // TODO cast
    instruction *insn = (instruction *) ((void*)&baseInsn[base]);

    switch (op) {
      case ifOp: {
	// cmpi 0,0,src1,0
        insn->raw = 0;
	insn->dform.op = CMPIop;
	insn->dform.ra = src1;
	insn->dform.d_or_si = 0;
	insn++;

	// be 0, dest
	insn->bform.op = BCop;
	insn->bform.bo = BTRUEcond;
	insn->bform.bi = EQcond;
	insn->bform.bd = dest/4;
	insn->bform.aa = 0;
	insn->bform.lk = 0;
	insn++;

	generateNOOP(insn);
	base += sizeof(instruction)*3;
	return(base - 2*sizeof(instruction));
        }
      case branchOp: {
	generateBranchInsn(insn, dest);
	insn++;

	generateNOOP(insn);
	base += sizeof(instruction)*2;
	return(base - 2*sizeof(instruction));
        }
      case trampPreamble: {
        // nothing to do in this platform
        return(0);              // let's hope this is expected!
        }        
      case trampTrailer: {
	/*
	for(u_int i = 0; i < regSpace->getRegisterCount(); i++) {
	  registerSlot *reg = regSpace->getRegSlot(i);
	  if (reg->beenClobbered) {
	    restoreRegister(insn, base, reg->number, TRAMP_GPR_OFFSET);
	    regSpace->unClobberRegister(reg->number);
	  }
	}
	*/
	
/*
	// restore the registers we have saved
	for (unsigned i = 0; i < regSpace->getRegisterCount(); i++) {
	    registerSlot *reg = regSpace->getRegSlot(i);
	    if (reg->mustRestore) {
		// cleanup register space for next trampoline.
		reg->needsSaving = true;
		reg->mustRestore = false;
		// actually restore the register.
		restoreRegister(insn,base,reg->number,8);
	    }
	}
*/
	generateBranchInsn(insn, dest);
	insn++;
	base += sizeof(instruction);

	generateNOOP(insn);
	insn++;
	base += sizeof(instruction);

	// return the relative address of the return statement.
	return(base - 2*sizeof(instruction));
      }
    default:
        assert(0);        // unexpected op for this emit!
  }
    return 0; // quiet the compiler
}

Register emitR(opCode op, Register src1, Register /*src2*/, Register dest,
               char *baseInsn, Address &base, bool /*noCost*/,
               const instPoint * /* location */, bool /*for_MT*/)
{
    //bperr("emitR(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

    // TODO cast
    instruction *insn = (instruction *) ((void*)&baseInsn[base]);

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

	if (regSlot->mustRestore) {
	  // its on the stack so load it.
	  restoreRegister(insn, base, reg, dest, 8);
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
	      if (regSpace->beenSaved(regSlot->number))
		{
		  restoreRegister(insn, base, src1+3, dest, TRAMP_GPR_OFFSET);
		}
	      else
		{
		  genImmInsn(insn, ORILop, src1+3, dest, 0);
		  insn++;
		  base += sizeof(instruction);
		}
	    }
	}    
	return(dest);
      } else {
	// Registers from 11 (src = 8) and beyond are saved starting at stack+56
        genImmInsn(insn, Lop, dest, 1, TRAMP_FRAME_SIZE+((src1-8)*sizeof(unsigned))+56);
        insn++;	
        base += sizeof(instruction);
        return(dest);
      }
    }
    default:
        assert(0);        // unexpected op for this emit!
  }
    return 0; // Compiler happiness
}

#ifdef BPATCH_LIBRARY
void emitJmpMC(int /*condition*/, int /*offset*/, char* /*baseInsn*/, Address &/*base*/)
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
static inline void restoreGPRtoGPR(instruction *&insn, Address &base,
                                   Register reg, Register dest)
{
  if(reg == 1) // SP is in a different place, but we don't need to
               // restore it, just subtract the stack frame size
    genImmInsn(insn, CALop, dest, REG_SP, TRAMP_FRAME_SIZE);
  else if((reg == 0) || ((reg >= 3) && (reg <=12)))
    genImmInsn(insn, Lop, dest, 1, TRAMP_GPR_OFFSET + reg*GPRSIZE);
  else {
    bperr( "GPR %d should not be restored...", reg);
    assert(0);
  }
  //bperr( "Loading reg %d (into reg %d) at 0x%x off the stack\n", 
  //  reg, dest, offset + reg*GPRSIZE);
  insn++;
  base += sizeof(instruction);
}

// VG(03/15/02): Restore mutatee value of XER to dest GPR
static inline void restoreXERtoGPR(instruction *&insn, Address &base, Register dest)
{
  genImmInsn(insn, Lop, dest, 1, TRAMP_SPR_OFFSET + STK_XER);
  insn++;
  base += sizeof(instruction);
}

// VG(03/15/02): Move bits 25:31 of GPR reg to GPR dest
static inline void moveGPR2531toGPR(instruction *&insn, Address &base,
                                    Register reg, Register dest)
{
  // keep only bits 32+25:32+31; extrdi dest, reg, 7 (n bits), 32+25 (start at b)
  // which is actually: rldicl dest, reg, 32+25+7 (b+n), 64-7 (64-n)
  // which is the same as: clrldi dest,reg,57 because 32+25+7 = 64
  insn->raw = 0;
  insn->mdform.op = RLDop;
  insn->mdform.rs = reg;
  insn->mdform.ra = dest;
  insn->mdform.sh = 0;  //(32+25+7) % 32;
  insn->mdform.mb_or_me = (64-7) % 32;
  insn->mdform.mb_or_me2 = (64-7) / 32;
  insn->mdform.xo = 0;  // rldicl
  insn->mdform.sh2 = 0; //(32+25+7) / 32;
  insn->mdform.rc = 0;
  ++insn;
  base += sizeof(instruction);
}

// VG(11/16/01): Emit code to add the original value of a register to
// another. The original value may need to be restored from stack...
// VG(03/15/02): Made functionality more obvious by adding the above functions
static inline void emitAddOriginal(Register src, Register acc, 
                                   char* baseInsn, Address &base, bool noCost)
{
  bool nr = needsRestore(src);
  instruction *insn = (instruction *) ((void*)&baseInsn[base]);
  Register temp;

  if(nr) {
    // this needs baseInsn because it uses emitV...
    temp = regSpace->allocateRegister(baseInsn, base, noCost);
    regSpace->clobberRegister(temp);

    // Looking at allocateRegister, I'd say that jkh's hack is no longer
    // useful on AIX (see naim's comment); no code is generated...
    /*insn = (instruction *) ((void*)&baseInsn[base]);*/

    // Emit code to restore the original ra register value in temp.
    // The offset compensates for the gap 0, 3, 4, ...
    // This writes at insn, and updates insn and base.

    if(src == POWER_XER2531) { // hack for XER_25:31
      //bperr( "XER_25:31\n");
      restoreXERtoGPR(insn, base, temp);
      moveGPR2531toGPR(insn, base, temp, temp);
    }
    else
      restoreGPRtoGPR(insn, base, src, temp);
  }
  else
    temp = src;

  // add temp to dest;
  // writes at baseInsn+base and updates base, we must update insn...
  emitV(plusOp, temp, acc, acc, baseInsn, base, noCost, 0);
  insn = (instruction *) ((void*)&baseInsn[base]);

  if(nr)
    regSpace->freeRegister(temp);
}

// VG(11/07/01): Load in destination the effective address given
// by the address descriptor. Used for memory access stuff.
void emitASload(BPatch_addrSpec_NP as, Register dest, char* baseInsn,
		Address &base, bool noCost)
{
  //instruction *insn = (instruction *) ((void*)&baseInsn[base]);
  int imm = as.getImm();
  int ra  = as.getReg(0);
  int rb  = as.getReg(1);
  // TODO: optimize this to generate the minimum number of
  // instructions; think about schedule

  // emit code to load the immediate (constant offset) into dest; this
  // writes at baseInsn+base and updates base, we must update insn...
  emitVload(loadConstOp, (Address)imm, dest, dest, baseInsn, base, noCost);
  //insn = (instruction *) ((void*)&baseInsn[base]);  
  
  // If ra is used in the address spec, allocate a temp register and
  // get the value of ra from stack into it
  if(ra > -1)
    emitAddOriginal(ra, dest, baseInsn, base, noCost);

  // If rb is used in the address spec, allocate a temp register and
  // get the value of ra from stack into it
  if(rb > -1)
    emitAddOriginal(rb, dest, baseInsn, base, noCost);

}

void emitCSload(BPatch_addrSpec_NP as, Register dest, char* baseInsn,
		Address &base, bool noCost)
{
  emitASload(as, dest, baseInsn, base, noCost);
}
#endif

void emitVload(opCode op, Address src1, Register /*src2*/, Register dest,
				char *baseInsn, Address &base, bool /*noCost*/, int size,
				const instPoint * /* location */, process * /* proc */,
				registerSpace * /* rs */ )
{
    instruction *insn = (instruction *) ((void*)&baseInsn[base]);

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
	  genImmInsn(insn, CALop, dest, 0, bottom_half);
	  base += sizeof(instruction);
	}
	else {
	  genImmInsn(insn, CAUop, dest, 0, top_half);
	  insn++;
	  // ori dest,dest,LOW(src1)
	  genImmInsn(insn, ORILop, dest, dest, bottom_half);
	  base += 2*sizeof(instruction);
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
	genImmInsn(insn, CAUop, dest, 0, high);
        insn++;

	// really load dest, (dest)imm
        if (size == 1)
           genImmInsn(insn, LBZop, dest, dest, LOW(src1));
        else if (size == 2)
           genImmInsn(insn, LHZop, dest, dest, LOW(src1));
        else 
           genImmInsn(insn, Lop, dest, dest, LOW(src1));

        base += sizeof(instruction)*2;
    } else if (op == loadFrameRelativeOp) {
	// return the value that is FP offset from the original fp
	int offset = (int) src1;

	if ((offset < MIN_IMM16) || (offset > MAX_IMM16)) {
	    assert(0);
	} else {
	    genImmInsn(insn, Lop, dest, REG_SP, offset + TRAMP_FRAME_SIZE);
	    insn++;
            base += sizeof(instruction)*2;
	}
    } else if (op == loadFrameAddr) {
	// offsets are signed!
	int offset = (int) src1;

	if ((offset < MIN_IMM16) || (offset > MAX_IMM16)) {
	  assert(0);
	} else {
	    genImmInsn(insn, CALop, dest, REG_SP, offset + TRAMP_FRAME_SIZE);
	    insn++;
	    base += sizeof(instruction);
	}
    } else {
      assert(0);
    }
}

void emitVstore(opCode op, Register src1, Register /*src2*/, Address dest,
	      char *baseInsn, Address &base, bool noCost, registerSpace * rs, int /* size */,
				const instPoint * /* location */, process * /* proc */)	      
{
    instruction *insn = (instruction *) ((void*)&baseInsn[base]);

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
	Register temp = regSpace->allocateRegister(baseInsn, base, noCost);
	rs->clobberRegister(temp);

	// This next line is a hack! - jkh 6/27/96
	//   It is required since allocateRegister can generate code.
	insn = (instruction *) ((void*)&baseInsn[base]);

	// set upper 16 bits of  temp to be the top high.
	genImmInsn(insn, CAUop, temp, 0, high);
	base += sizeof(instruction);
        insn++;

	// low == LOW(dest)
	// generate -- st src1, low(temp)
#ifdef notdef
	insn->dform.op = STop;
	insn->dform.rt = src1;
	insn->dform.ra = temp;
	insn->dform.d_or_si = LOW(dest);
#endif
	genImmInsn(insn, STop, src1, temp, LOW(dest));
	base += sizeof(instruction);
        insn++;

	regSpace->freeRegister(temp);
        return;
    } else if (op == storeFrameRelativeOp) {
	// offsets are signed!
	int offset = (int) dest;
	if ((offset < MIN_IMM16) || (offset > MAX_IMM16)) {
	  assert(0);
	} else {
	    genImmInsn(insn, STop, src1, REG_SP, offset + TRAMP_FRAME_SIZE);
	    base += sizeof(instruction);
	    insn++;
	}
    } else {
        assert(0);       // unexpected op for this emit!
    }
}

void emitVupdate(opCode op, RegValue src1, Register /*src2*/, Address dest,
	      char *baseInsn, Address &base, bool noCost, registerSpace * rs)
{
    instruction *insn = (instruction *) ((void*)&baseInsn[base]);

    if (op == updateCostOp) {
        if (!noCost) {

           regSpace->resetSpace();

	   // add in the cost to the passed pointer variable.

	   // high order bits of the address of the cummulative cost.
	   Register obsCostAddr = regSpace->allocateRegister(baseInsn, base, noCost);
	   rs->clobberRegister(obsCostAddr);

	   // actual cost.
	   Register obsCostValue = regSpace->allocateRegister(baseInsn, base, noCost);
	   rs->clobberRegister(obsCostValue);

	   // This next line is a hack! - jkh 6/27/96
	   //   It is required since allocateRegister can generate code.
	   insn = (instruction *) ((void*)&baseInsn[base]);

	   int high;

	   // load high half word of address into dest.
	   // really addis 0,dest,HIGH(dest) aka lis dest, HIGH(dest)
	   if (LOW(dest) & 0x8000) {
	      // high bit of low is set so the sign extension of the load
	      // will cause the wrong effective addr to be computed.
	      // so we subtract the sign ext value from HIGH.
	      // sounds odd, but works and saves an instruction - jkh 5/25/95
	      high = HIGH(dest) - 0xffff;
	   } else {
	      high = HIGH(dest);
	   }
	   genImmInsn(insn, CAUop, obsCostAddr, 0, high);
	   insn++;

	   // really load obsCostValue, (obsCostAddr)imm
	   genImmInsn(insn, Lop, obsCostValue, obsCostAddr, LOW(dest));
	   insn++;

           base += 2 * sizeof(instruction);

	   //assert(src1 <= MAX_IMM);
           if (src1 <= MAX_IMM) {
  	     genImmInsn(insn, CALop, obsCostValue, obsCostValue, LOW(src1));
	     insn++;
             base += sizeof(instruction);
	   } else {
             // If src1>MAX_IMM, we can't generate an instruction using an
             // an immediate operator. Therefore, we need to load the 
             // value first, which will cost 2 instructions - naim
             Register reg = regSpace->allocateRegister(baseInsn, base, noCost);
             rs->clobberRegister(reg);
	     genImmInsn(insn, CAUop, reg, 0, HIGH(src1));
             insn++;
             genImmInsn(insn, ORILop, reg, reg, LOW(src1));
             insn++;
             base += 2 * sizeof(instruction);
             emitV(plusOp, reg, obsCostValue, obsCostValue, baseInsn, 
                         base, noCost);        
             regSpace->freeRegister(reg);     
             insn++;
	   }

	   // now store it back.
	   // low == LOW(dest)
	   // generate -- st obsCostValue, obsCostAddr+low(dest)
	   insn->dform.op = STop;
	   insn->dform.rt = obsCostValue;
	   insn->dform.ra = obsCostAddr;
	   insn->dform.d_or_si = LOW(dest);
	   insn++;
	   base += sizeof(instruction);

	   regSpace->freeRegister(obsCostValue);
	   regSpace->freeRegister(obsCostAddr);
       } // if !noCost
    } else {
        assert(0);       // unexpected op for this emit!
    }
}

void emitV(opCode op, Register src1, Register src2, Register dest,
              char *baseInsn, Address &base, bool /*noCost*/, int /* size */,
              const instPoint * /* location */, process * /* proc */,
              registerSpace * /* rs */ )
{
    //bperr("emitV(op=%d,src1=%d,src2=%d,dest=%d)\n",op,src1,src2,dest)
;

    assert ((op!=branchOp) && (op!=ifOp) && 
            (op!=trampTrailer) && (op!=trampPreamble));         // !emitA
    assert ((op!=getRetValOp) && (op!=getParamOp));             // !emitR
    assert ((op!=loadOp) && (op!=loadConstOp));                 // !emitVload
    assert ((op!=storeOp));                                     // !emitVstore
    assert ((op!=updateCostOp));                                // !emitVupdate

    // TODO cast
    instruction *insn = (instruction *) ((void*)&baseInsn[base]);

    if (op == loadIndirOp) {
        // really load dest, (dest)imm
        genImmInsn(insn, Lop, dest, src1, 0);

        base += sizeof(instruction);

    } else if (op == storeIndirOp) {

        // generate -- st src1, dest
        insn->dform.op = STop;
        insn->dform.rt = src1;
        insn->dform.ra = dest;
        insn->dform.d_or_si = 0;
        base += sizeof(instruction);

    } else if (op == noOp) {
        generateNOOP(insn);
        base += sizeof(instruction);
    } else if (op == saveRegOp) {
        saveRegister(insn,base,src1,8);
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
                //genSimpleInsn(insn, ORop, src1, src2, dest, base);
                insn->raw = 0;
                insn->xoform.op = ORop;
                // operation is ra <- rs | rb (or rs,ra,rb)
                insn->xoform.ra = dest;
                insn->xoform.rt = src1;
                insn->xoform.rb = src2;
                insn->xoform.xo = ORxop;
                base += sizeof(instruction);
                return;
                break;

            case andOp:
                //genSimpleInsn(insn, ANDop, src1, src2, dest, base);
                // This is a Boolean and with true == 1 so bitwise is OK
                insn->raw = 0;
                insn->xoform.op = ANDop;
                // operation is ra <- rs & rb (and rs,ra,rb)
                insn->xoform.ra = dest;
                insn->xoform.rt = src1;
                insn->xoform.rb = src2;
                insn->xoform.xo = ANDxop;
                base += sizeof(instruction);
                return;
                break;

            // rel ops
            case eqOp:
                genRelOp(insn, EQcond, BTRUEcond, src1, src2, dest, base);
                return;
                break;

            case neOp:
                genRelOp(insn, EQcond, BFALSEcond, src1, src2, dest, base);
                return;
                break;

            case lessOp:
                genRelOp(insn, LTcond, BTRUEcond, src1, src2, dest, base);
                return;
                break;

            case greaterOp:
                genRelOp(insn, GTcond, BTRUEcond, src1, src2, dest, base);
                return;
                break;

            case leOp:
                genRelOp(insn, GTcond, BFALSEcond, src1, src2, dest, base);
                return;
                break;

            case geOp:
                genRelOp(insn, LTcond, BFALSEcond, src1, src2, dest, base);
                return;
                break;

            default:
                // internal error, invalid op.
                bperr( "Invalid op passed to emit, instOp = %d\n", instOp);
                assert(0 && "Invalid op passed to emit");
                break;
        }
        assert((instOp != -1) && (instXop != -1));
        insn->raw = 0;
        insn->xoform.op = instOp;
        insn->xoform.rt = dest;
        insn->xoform.ra = src1;
        insn->xoform.rb = src2;
        insn->xoform.xo = instXop;

        base += sizeof(instruction);
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

/*
 * We need to also find calls that don't go to a known location.
 * AFAIK, POWER has no "branch to register" calls. It does, however,
 * have branch to link register and branch to count register calls.
 */

bool isDynamicCall(const instruction i)
{
    // I'm going to break this up a little so that I can comment
    // it better. 
    
    if (i.xlform.op == BCLRop && i.xlform.xo == BCLRxop)
    {
        if (i.xlform.lk)
            // Type one: Branch-to-LR, save PC in LR
            // Standard function pointer call
            return true;
        else
            // Type two: Branch-to-LR, don't save PC
            // Haven't seen one of these around
            // It would be a return instruction, probably
        {
            return false;
        }
    }
    if (i.xlform.op == BCLRop && i.xlform.xo == BCCTRxop)
    {
        if (i.xlform.lk)
            // Type three: Branch-to-CR, save PC
        {
            return true;
        }
        else
            // Type four: Branch-to-CR, don't save PC
            // Used for global linkage code.
            // We handle those elsewhere.
        {
            return true;
        }
    }
    return false;
}

bool int_function::findInstPoints(const image *i_owner)
{
    if( parsed_ ) 
    {
        fprintf(stderr, "Error: multiple call of findInstPoints\n");
        return false;
    } 
    parsed_ = true;

    makesNoCalls_ = true;
    noStackFrame = true;
    int insnSize = sizeof( instruction );
    image *owner = const_cast<image *>(i_owner);
    BPatch_Set< Address > leaders;
    dictionary_hash< Address, BPatch_basicBlock* > leadersToBlock( addrHash );
       
    Address funcBegin = getAddress( 0 );
    Address funcEnd = funcBegin;

    instruction instr;
    instr.raw = owner->get_instruction( funcBegin );

    if (!IS_VALID_INSN(instr)) 
    {
        isInstrumentable_ = false;
        return false;
    }

    //define entry instpoint 
    funcEntry_ = new instPoint( this, instr, owner, funcBegin, 
                               true, functionEntry );
    assert( funcEntry_ );
    
    InstrucIter ah( funcBegin, funcBegin, owner );
    Address currAddr = funcBegin;
    
    instruction retInsn;
    retInsn.raw = 0;
    funcReturns.push_back( new instPoint( this, retInsn, owner, 
                                          funcBegin + get_size() - insnSize,
                                          false, functionExit));
    
    instr.raw = owner->get_instruction( currAddr );
    for (unsigned i = 0; i < 10; i++) 
    {
        if ((instr.dform.op == STUop) &&
            (instr.dform.rt == 1) &&
            (instr.dform.ra == 1)) 
        {
            noStackFrame = false;
        }
        if (instr.raw == MFLR0raw) 
        {
            makesNoCalls_ = false;
        }
        currAddr += insnSize;
        instr.raw = owner->get_instruction( currAddr );
    }
 
    //find all the basic blocks and define the instpoints for this function
    BPatch_Set< Address > visited;
    pdvector< Address > jmpTargets;
    jmpTargets.push_back( funcBegin );

    //entry basic block
    leaders += funcBegin;
    leadersToBlock[ funcBegin ] = new BPatch_basicBlock;
    leadersToBlock[ funcBegin ]->setRelStart( funcBegin );
    leadersToBlock[ funcBegin ]->isEntryBasicBlock = true;
    blockList->push_back( leadersToBlock[ funcBegin ] );
    jmpTargets.push_back( funcBegin );
    
    for( unsigned i = 0; i < jmpTargets.size(); i++ )
    {
         InstrucIter ah( jmpTargets[ i ], funcBegin, owner );
         BPatch_basicBlock* currBlk = leadersToBlock[ jmpTargets[ i ] ];
        
         while( true )
         {
             currAddr = *ah;
             if( visited.contains( currAddr ) )
                 break;
             else
                 visited += currAddr;

             if( ah.isACondBranchInstruction() )
             {
                 currBlk->setRelLast( currAddr );
                 currBlk->setRelEnd( currAddr + insnSize );
                 if( currAddr >= funcEnd )
                     funcEnd = currAddr + insnSize;
                 
                 Address target = ah.getBranchTargetAddress( currAddr );
                 if( target < funcBegin )
                 {
                     currBlk->setExitBlock( true );
                 }
                 else
                 {
                     jmpTargets.push_back( target );
                     
                     //check if a basicblock object has been 
                     //created for the target
                     if( !leaders.contains( target ) )
                     {
                         //if not, then create one
                         leadersToBlock[ target ] = new BPatch_basicBlock;
                         leaders += target;
                         blockList->push_back( leadersToBlock[ target ] );
                     }
                     leadersToBlock[ target ]->setRelStart( target );	
                     leadersToBlock[ target ]->addSource( currBlk );
                     currBlk->addTarget( leadersToBlock[ target ] );
                 }            
                
                 Address t2 = currAddr + insnSize;
                 jmpTargets.push_back( t2 );
                 if( !leaders.contains( t2 ) )
                 {
                     leadersToBlock[ t2 ] = new BPatch_basicBlock;
                     leaders += t2;
                     blockList->push_back( leadersToBlock[ t2 ] );
                 }                 
                 leadersToBlock[ t2 ]->setRelStart( t2 );
                 leadersToBlock[ t2 ]->addSource( currBlk );
                 currBlk->addTarget( leadersToBlock[ t2 ] );
                 break;
             }
             else if( ah.isAIndirectJumpInstruction( ah ) )
             {
                 InstrucIter ah2( ah );
                 currBlk->setRelLast( currAddr );
                 currBlk->setRelEnd( currAddr + insnSize );
                 
                 if( currAddr >= funcEnd )
                     funcEnd = currAddr + insnSize;
                 
                 BPatch_Set< Address > res;              
                 ah2.getMultipleJumpTargets( res );
                                  
                 BPatch_Set< Address >::iterator iter;
                 iter = res.begin();

                 while( iter != res.end() )
                 {
                     if( *iter < funcBegin )
                     {
                         currBlk->isExitBasicBlock = true;
                     }
                     else
                     {
                         if( !leaders.contains( *iter ) )
                         {
                             leadersToBlock[ *iter ] = new BPatch_basicBlock;
                             leadersToBlock[ *iter ]->setRelStart( *iter );
                             leaders += *iter;

                             jmpTargets.push_back( *iter );
                             blockList->push_back( leadersToBlock[ *iter] );
                         }                        
                         currBlk->addTarget( leadersToBlock[ *iter ] );
                         leadersToBlock[ *iter ]->addSource( currBlk );
                     }
                     iter++;
                 }                 
                 break;
             }             
             else if( ah.isAReturnInstruction() ||
                      ah.getInstruction().raw == 0x0 )
             {
                 //we catch exits by overwriting the return address
                 //we therefore do not need to make exit instPoints
                 currBlk->setRelLast( currAddr );
                 currBlk->setRelEnd( currAddr + insnSize );
                 currBlk->isExitBasicBlock = true;
                 
                 if( currAddr >= funcEnd )
                     funcEnd = currAddr + insnSize;
                 break;
             }
             else if( ah.isAJumpInstruction() )
             {
                 currBlk->setRelLast( currAddr );
                 currBlk->setRelEnd( currAddr + insnSize );
                 
                 if( currAddr >= funcEnd )
                     funcEnd = currAddr + insnSize;
                
                 Address target = ah.getBranchTargetAddress( *ah );

                 if( target < funcBegin )
                 {
                     currBlk->setExitBlock( true );
                 }
                 else
                 {
                     jmpTargets.push_back( target );
                     //check if a basicblock object has been 
                     //created for the target
                     if( !leaders.contains( target ) )
                     {
                         //if not, then create one
                         leadersToBlock[ target ] = new BPatch_basicBlock;
                         leaders += target;
                         blockList->push_back( leadersToBlock[ target ] );
                     }                     
                     leadersToBlock[ target ]->setRelStart( target );	
                     leadersToBlock[ target ]->addSource( currBlk );
                     currBlk->addTarget( leadersToBlock[ target ] );
                 }                 
                 break;                 
             }
             else if( ah.isACallInstruction() || 
                      ah.isADynamicCallInstruction())
             {
                 instr = ah.getInstruction();
                 bool err;
                 newCallPoint( currAddr, instr, owner, err);  
                 
                 if( err )
                 {
                     isInstrumentable_ = false;
                     return false;
                 }
             }            
             ah++;
         }                   
    }
    
    VECTOR_SORT( calls, instPointCompare );
    //check if basic blocks need to be split   
    VECTOR_SORT( (*blockList), basicBlockCompare );
     
    //maybe BPatch_flowGraph.C would be a better home for this bit of code?
    for( unsigned int iii = 0; iii < blockList->size(); iii++ )
    {
        (*blockList)[iii]->blockNumber = iii;
    }
  
    for( unsigned int r = 0; r + 1 < blockList->size(); r++ )
    {
        BPatch_basicBlock* b1 = (*blockList)[ r ];
        BPatch_basicBlock* b2 = (*blockList)[ r + 1 ];
              
        if( b2->getRelStart() < b1->getRelEnd() )
        {
            BPatch_Vector< BPatch_basicBlock* > out;
            b1->getTargets( out );
            
            for( unsigned j = 0; j < out.size(); j++ )
            {
                out[j]->sources.remove( b1 );
                out[j]->sources.insert( b2 );
            }        
          
            //set end address of higher block
            b2->setRelLast( b1->getRelLast());
            b2->setRelEnd( b1->getRelEnd() );
            b2->targets = b1->targets;	    
            b2->addSource( b1 );
            
            BPatch_Set< BPatch_basicBlock* > nt;
            nt += b2;
            b1->targets = nt;
            
            //find the end of the split block	       
            b1->setRelLast( b2->getRelStart() - insnSize );
            b1->setRelEnd( b2->getRelStart() );
                                    
            if( b1->isExitBasicBlock )
            {
                b1->setExitBlock( false );
                b2->setExitBlock( true );
            }
        }
    }

    for( unsigned q = 0; q + 1 < blockList->size(); q++ )
    {
        BPatch_basicBlock* b1 = (*blockList)[ q ];
        BPatch_basicBlock* b2 = (*blockList)[ q + 1 ];
        
        if( b1->getRelEnd() == 0 )
        {
            b1->setRelLast( b2->getRelStart() - insnSize );
            b1->setRelEnd( b2->getRelStart() );
            b1->addTarget( b2 );
            b2->addSource( b1 );	            
        }        
    }    
    
    isInstrumentable_ = true;
    return true;
}

//
// Each processor may have a different heap layout.
//   So we make this processor specific.
//
// find all DYNINST symbols that are data symbols
bool process::heapIsOk(const pdvector<sym_data> &find_us) {
  Address baseAddr;
  Symbol sym;
  pdstring str;

  // find the main function
  // first look for main or _main
  if (!((mainFunction = findOnlyOneFunction("main")) 
        || (mainFunction = findOnlyOneFunction("_main")))) {
     pdstring msg = "Cannot find main. Exiting.";
     statusLine(msg.c_str());
     showErrorCallback(50, msg);
     return false;
  }

  for (unsigned i=0; i<find_us.size(); i++) {
    const pdstring &str = find_us[i].name;
    if (!getSymbolInfo(str, sym, baseAddr)) {
      pdstring str1 = pdstring("_") + str.c_str();
      if (!getSymbolInfo(str1, sym, baseAddr) && find_us[i].must_find) {
        pdstring msg;
        msg = pdstring("Cannot find ") + str + pdstring(". Exiting");
        statusLine(msg.c_str());
        showErrorCallback(50, msg);
        return false;
      }
    }
  }
  return true;
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
void registerSpace::resetLiveDeadInfo(const int * liveRegs)
{
  registerSlot *regSlot = NULL;
  
  if (liveRegs != NULL)
    {
      /*
      for (int a = 0; a < 32; a++)
	printf("%d ",liveRegs[a]);
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
    }
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


bool returnInstance::checkReturnInstance(const pdvector<pdvector<Frame> > &/*stackWalks*/)
{
#ifdef ndef  
  // TODO: implement this.  This stuff is not implemented for this platform 
  // That's okay -- single instruction jumps mean that we can overwrite anywhere,
  // anytime. If we ever went to multi-instruction fallbacks, then we'd need this
  // -- bernat, 10OCT02
#endif
    return true;
}
 
void returnInstance::installReturnInstance(process *proc) {
    proc->writeTextSpace((caddr_t)addr_, instSeqSize, (caddr_t) instructionSeq); 
	installed = true;
}

void returnInstance::addToReturnWaitingList(Address , process * ) {
    int blah = 0;
    assert(blah);
    P_abort();
}

bool doNotOverflow(int value)
{
  // we are assuming that we have 15 bits to store the immediate operand.
  if ( (value <= 32767) && (value >= -32768) ) return(true);
  else return(false);
}

void instWaitingList::cleanUp(process * , Address ) {
    P_abort();
}

// hasBeenBound: returns false
// On AIX (at least what we handle so far), all symbols are bound
// at load time. This kind of obviates the need for a hasBeenBound
// function: of _course_ the symbol has been bound. 
// So the idea of having a relocation entry for the function doesn't
// quite make sense. Given the target address, we can scan the function
// lists until we find the desired function.

bool process::hasBeenBound(const relocationEntry ,int_function *&, Address ) {
  // What needs doing:
  // Locate call instruction
  // Decipher call instruction (static/dynamic call, global linkage code)
  // Locate target
  // Lookup target
  return false; // Haven't patched this up yet
}

// findCallee
bool process::findCallee(instPoint &instr, int_function *&target){
  if((target = instr.getCallee())) {
    return true; // callee already set
  }
  // Other possibilities: call through a function pointer,
  // or a inter-module call. We handle inter-module calls as
  // a static function call.
  const image *owner = instr.getOwner();
  const int_function *caller = instr.pointFunc();
  // Or module == glink.s == "Global_Linkage"
  if (caller->prettyName().suffixed_by("_linkage")) {
      // Make sure we're not mistaking a function named
      // *_linkage for global linkage code. 
      
      if (instr.originalInstruction.raw != 0x4e800420) // BCTR
          return false;
      Address TOC_addr = (owner->getObject()).getTOCoffset();
      instruction offset_instr;
      // We want the offset --so  pointAddr
      offset_instr.raw = owner->get_instruction(instr.pointAddr() - 20); // Five instructions up.
      if ((offset_instr.dform.op != Lop) ||
          (offset_instr.dform.rt != 12) ||
          (offset_instr.dform.ra != 2))
          // Not a l r12, <offset>(r2), so not actually linkage code.
          return false;
      // This should be the contents of R12 in the linkage function
      Address callee_TOC_entry = owner->get_instruction(TOC_addr + offset_instr.dform.d_or_si);
      // We need to find what object the callee TOC entry is defined in. This will be the
      // same place we find the function, later.
      Address callee_addr = 0;
      const image *callee_img = NULL;
      if ( (callee_addr = symbols->get_instruction(callee_TOC_entry) ))
          callee_img = symbols;
      else
          if (shared_objects) {
              for(u_int i=0; i < shared_objects->size(); i++){
                  const image *img_tmp = ((*shared_objects)[i])->getImage();
                  if ( (callee_addr = img_tmp->get_instruction(callee_TOC_entry)))
                  {
                      callee_img = img_tmp;
                      break;
                  }
              }
          }
      if (!callee_img) return false;
      // callee_addr: address of function called, contained in image callee_img
      // Sanity check on callee_addr
      if (
          ((callee_addr < 0x20000000) ||
           (callee_addr > 0xdfffffff)))
      {
          if (callee_addr != 0) { // unexpected -- where is this function call? Print it out.
              bperr( "Skipping illegal address 0x%x in function %s\n",
                      (unsigned) callee_addr, caller->prettyName().c_str());
          }
          return false;
      }
      
      // Again, by definition, the function is not in owner.
      // So look it up.
      int_function *pdf = 0;
      codeRange *range = findCodeRangeByAddress(callee_addr);
      pdf = range->is_function();
      
      if (pdf)
      {
          target = pdf;
          instr.setCallee(pdf);
          return true;
      }
      else
          bperr( "Couldn't find target function for address 0x%x, jump at 0x%x\n",
                  (unsigned) callee_addr,
                  (unsigned) instr.absPointAddr(this));
  }
  // Todo
  target = 0;
  return false;
  
}


// process::replaceFunctionCall
//
// Replace the function call at the given instrumentation point with a call to
// a different function, or with a NOOP.  In order to replace the call with a
// NOOP, pass NULL as the parameter "func."
// Returns true if sucessful, false if not.  Fails if the site is not a call
// site, or if the site has already been instrumented using a base tramp.
bool process::replaceFunctionCall(const instPoint *point,
				  const int_function *newFunc) {
   // Must be a call site
   if (point->getPointType() != callSite)
      return false;
   
   // Cannot already be instrumented with a base tramp
   if (baseMap.defines(point))
      return false;
   
   instruction newInsn;
   if (newFunc == NULL) {	// Replace with a NOOP
      generateNOOP(&newInsn);
   } else {			// Replace with a new call instruction
      generateBranchInsn(&newInsn,
                         newFunc->get_address()-point->absPointAddr(this));
      newInsn.iform.lk = 1;
   }
   
   writeTextSpace((caddr_t)point->absPointAddr(this), sizeof(instruction),
                  &newInsn);
   
   return true;
}

// Emit code to jump to function CALLEE without linking.  (I.e., when
// CALLEE returns, it returns to the current caller.)

// 18FEB00 -- I removed the parameter names to get rid of compiler
// warnings. They are copied below.
// opCode op, char *i, Address &base, const int_function *callee, process *proc

void emitFuncJump(opCode, 
		  char *, Address &, 
		  const int_function *unused, process *,
		  const instPoint *, bool)
{
  // Get rid of a warning about the unused parameter
     if (unused) ;
     /* Unimplemented on this platform! */
     assert(0);
}

// atch AIX register numbering (sys/reg.h)
#define REG_LR  131
#define REG_CTR 132

void emitLoadPreviousStackFrameRegister(Address register_num, 
					Register dest,
					char *insn,
					Address &base,
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
      emitImm(plusOp ,(Register) REG_SP, (RegValue) offset, dest, insn, 
	      base, noCost, regSpace);
      // Load LR into register dest
      emitV(loadIndirOp, dest, 0, dest, insn, base, noCost, size);
      break;
    case REG_CTR:
      // CTR is saved down the stack
        offset = TRAMP_SPR_OFFSET + STK_CTR;
        // Get address (SP + offset) and stick in register dest.
        emitImm(plusOp ,(Register) REG_SP, (RegValue) offset, dest, insn, 
                base, noCost, regSpace);
        // Load LR into register dest
        emitV(loadIndirOp, dest, 0, dest, insn, base, noCost, size);
      break;
    default:
      cerr << "Fallthrough in emitLoadPreviousStackFrameRegister" << endl;
      cerr << "Unexpected register " << register_num << endl;
      abort();
      break;
    }
}

bool process::isDynamicCallSite(instPoint *callSite){
    if (isDynamicCall(callSite->originalInstruction))
        return true;

    return false;
    
}

bool process::getDynamicCallSiteArgs(instPoint *callSite,
                                    pdvector<AstNode *> &args)
{

  instruction i = callSite->originalInstruction;
  Register branch_target;

  // Is this a branch conditional link register (BCLR)
  // BCLR uses the xlform (6,5,5,5,10,1)
  if(i.xlform.op == BCLRop) // BLR/BCR, or bcctr/bcc. Same opcode.
  {
      if (i.xlform.xo == BCLRxop) // BLR (bclr)
      {
          //bperr( "Branch target is the link register\n");
          branch_target = REG_LR;
      }
      else if (i.xlform.xo == BCCTRxop)
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
                                  (void *) callSite->absPointAddr(this)));
      return true;
  }
  else if (i.xlform.op == Bop) {
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
      cerr << "MonitorCallSite: Unknown opcode " << i.xlform.op << endl;
      cerr << "opcode extension: " << i.xlform.xo << endl;
      bperr( "Address is 0x%x, insn 0x%x\n",
              (unsigned) callSite->absPointAddr(this),
              (unsigned) i.raw);
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
  if(i.xlform.op == BCLRop) // BLR/BCR, or bcctr/bcc. Same opcode.
  {
      if (i.xlform.xo == BCLRxop) // BLR (bclr)
      {
          //bperr( "Branch target is the link register\n");
          branch_target = REG_LR;
      }
      else if (i.xlform.xo == BCCTRxop)
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
      miniTrampHandle *mtHandle;
      addInstFunc(this, mtHandle, callSite, func, callPreInsn,
                  orderFirstAtPoint,
                  true,                        /* noCost flag   */
                  false,                       /* trampRecursiveDesired flag */
                  true);                       /* allowTrap */
      return true;
  }
  else if (i.xlform.op == Bop) {
      /// Why didn't we catch this earlier? In any case, don't print an error

      // I have seen this legally -- branches to the FP register saves.
      // Since we ignore the save macros, we have no idea where the branch
      // goes. For now, return true -- means no error.

      return true;
  }
  else
  {
      cerr << "MonitorCallSite: Unknown opcode " << i.xlform.op << endl; 
      cerr << "opcode extension: " << i.xlform.xo << endl;
      bperr( "Address is 0x%x, insn 0x%x\n", 
              (unsigned) callSite->absPointAddr(this), 
              (unsigned) i.raw);
      return false;
  }
}
#endif // NOTDEF // PDSEP


bool deleteBaseTramp(process */*proc*/, trampTemplate *)
{
	cerr << "WARNING : deleteBaseTramp is unimplemented "
	     << "(after the last instrumentation deleted)" << endl;
	return false;
}


/*
 * createInstructionInstPoint
 *
 * Create a BPatch_point instrumentation point at the given address, which
 * is guaranteed not be one of the "standard" inst points.
 *
 * proc         The process in which to create the inst point.
 * address      The address for which to create the point.
 */
BPatch_point* createInstructionInstPoint(BPatch_process *proc, void *address,
                                         BPatch_point** /*alternative*/,
                                         BPatch_function* bpf)
{

    if (!isAligned((Address)address))
        return NULL;

    int_function *func;
    if(bpf)
        func = (int_function *)bpf->func;
    else {
        codeRange *range = proc->llproc->findCodeRangeByAddress((Address) address);
        func = range->is_function();
    }

    instruction instr;
    proc->llproc->readTextSpace(address, sizeof(instruction), &instr.raw);

    Address pointImageBase = 0;
    image* pointImage = func->pdmod()->exec();
    proc->llproc->getBaseAddress((const image*)pointImage,pointImageBase);

    instPoint *newpt = new instPoint(func,
                                     instr,
                                     NULL, // image *owner - this is ignored
                                     (Address)((Address)address-pointImageBase),
                                     false, // bool delayOk - this is ignored
                                     otherPoint);
    func->addArbitraryPoint(newpt,NULL);

    return proc->findOrCreateBPPoint(NULL, newpt, BPatch_arbitrary);
}

#ifdef BPATCH_LIBRARY
/*
 * BPatch_point::getDisplacedInstructions
 *
 * Returns the instructions to be relocated when instrumentation is inserted
 * at this point.  Returns the number of bytes taken up by these instructions.
 *
 * maxSize      The maximum number of bytes of instructions to return.
 * insns        A pointer to a buffer in which to return the instructions.
 */
int BPatch_point::getDisplacedInstructionsInt(int maxSize, void *insns)
{
  if ((unsigned) maxSize >= (unsigned) sizeof(instruction))
      memcpy(insns, &point->originalInstruction.raw, sizeof(instruction));
  
  return sizeof(instruction);
}

#endif


//XXX loop port
BPatch_point *
createInstructionEdgeInstPoint(process* /*proc*/, 
			       int_function */*func*/, 
			       BPatch_edge */*edge*/)
{
    return NULL;
}

//XXX loop port
void 
createEdgeTramp(process */*proc*/, image */*img*/, BPatch_edge */*edge*/)
{

}
