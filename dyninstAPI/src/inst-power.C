/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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
 * $Id: inst-power.C,v 1.95 2000/08/04 19:49:48 hollings Exp $
 */

#include "common/h/headers.h"

#ifndef BPATCH_LIBRARY
#include "rtinst/h/rtinst.h"
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

// The following vrbles were defined in process.C:
extern debug_ostream attach_cerr;
extern debug_ostream inferiorrpc_cerr;
extern debug_ostream shmsample_cerr;
extern debug_ostream forkexec_cerr;
extern debug_ostream metric_cerr;

#define perror(a) P_abort();

extern bool isPowerOf2(int value, int &result);

#define ABS(x)		((x) > 0 ? x : -x)
#define MAX_BRANCH	0x1<<23
#define MAX_CBRANCH	0x1<<13

#define MAX_IMM		0x1<<15		/* 15 plus sign == 16 bits */

#define SPR_XER	1
#define SPR_LR	8
#define SPR_CTR	9
#define DISTANCE(x,y)   ((x<y) ? (y-x) : (x-y))

Address getMaxBranch() {
  return MAX_BRANCH;
}

const char *registerNames[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
			"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
			"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
			"r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"};

dictionary_hash<string, unsigned> funcFrequencyTable(string::hash);

inline void generateBranchInsn(instruction *insn, int offset)
{
    if (ABS(offset) > MAX_BRANCH) {
	logLine("a branch too far\n");
	showErrorCallback(52, "Internal error: branch too far");
	abort();
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
      abort();
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


instPoint::instPoint(pd_Function *f, const instruction &instr, 
		     const image *, Address adr, bool, ipFuncLoc fLoc) 
		      : addr(adr), originalInstruction(instr), 
			callIndirect(false), callee(NULL), func(f), ipLoc(fLoc)
{
   // inDelaySlot = false;
   // isDelayed = false;
   // callAggregate = false;
}


// Determine if the called function is a "library" function or a "user" function
// This cannot be done until all of the functions have been seen, verified, and
// classified
//
void pd_Function::checkCallPoints() {
  unsigned int i;
  instPoint *p;
  Address loc_addr;

  vector<instPoint*> non_lib;

  for (i=0; i<calls.size(); ++i) {
    /* check to see where we are calling */
    p = calls[i];
    assert(p);

    if(isCallInsn(p->originalInstruction)) {
      loc_addr = p->addr + (p->originalInstruction.iform.li << 2);
      pd_Function *pdf = (file_->exec())->findFunction(loc_addr);
      if (pdf) {
        p->callee = pdf;
        non_lib += p;
      } else {
	delete p;
      }
    } else {
      // Indirect call -- be conservative, assume it is a call to
      // an unnamed user function
      assert(!p->callee); assert(p->callIndirect);
      p->callee = NULL;
      non_lib += p;
    }
  }
  calls = non_lib;
}

// TODO we cannot find the called function by address at this point in time
// because the called function may not have been seen.
//
Address pd_Function::newCallPoint(const Address adr, const instruction instr,
				 const image *owner, bool &err)
{
    Address ret=adr;
    instPoint *point;
    err = true;

    point = new instPoint(this, instr, owner, adr, false, ipFuncCallPoint);

    if (!isCallInsn(instr)) {
      point->callIndirect = true;
      point->callee = NULL;
    } else
      point->callIndirect = false;

    // point->callAggregate = false;

    calls += point;
    err = false;
    return ret;
}

void initDefaultPointFrequencyTable()
{
    funcFrequencyTable[EXIT_NAME] = 1;

#ifdef notdef
    FILE *fp;
    float value;
    char name[512];

    funcFrequencyTable["main"] = 1;
    funcFrequencyTable["DYNINSTsampleValues"] = 1;
    // try to read file.
    fp = fopen("freq.input", "r");
    if (!fp) {
	printf("no freq.input file\n");
	return;
    }
    while (!feof(fp)) {
	fscanf(fp, "%s %f\n", name, &value);
	funcFrequencyTable[name] = (int) value;
	printf("adding %s %f\n", name, value);
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

    pd_Function *func;

    if (point->callee)
        func = point->callee;
    else
        func = point->func;

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
	abort();
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
      abort();
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
	abort();
      } else {
	insn->iform.li = newOffset >> 2;
      }
    } else if (isInsnType(*insn, Bmask, BCmatch)) {
      // conditional pc relative branch.
      newOffset = origAddr - targetAddr + (insn->bform.bd << 2);
      if (ABS(newOffset) > MAX_CBRANCH) {
	logLine("a branch too far\n");
	abort();
      } else {
	insn->bform.bd = newOffset >> 2;
      }
    } else if (insn->iform.op == SVCop) {
      logLine("attempt to relocate a system call\n");
      abort();
    } 
    /* The rest of the instructions should be fine as is */
}
#endif

trampTemplate baseTemplate;

// New version of the base tramp -- will not enter instrumentation recursively
trampTemplate baseTemplateNonRecursive;

#ifdef BPATCH_LIBRARY
trampTemplate conservativeTemplate;
#endif

extern "C" void baseTramp();
#ifdef BPATCH_LIBRARY
extern "C" void conservativeTramp();
#endif

void initATramp(trampTemplate *thisTemp, instruction *tramp, bool guardDesired = true)
{
    instruction *temp;

    // TODO - are these offsets always positive?
    thisTemp->trampTemp = (void *) tramp;
    for (temp = tramp; temp->raw != END_TRAMP; temp++) {
	switch (temp->raw) {
	    case LOCAL_PRE_BRANCH:
		thisTemp->localPreOffset = ((char*)temp - (char*)tramp);
		thisTemp->localPreReturnOffset = thisTemp->localPreOffset
		                                 + 4 * sizeof(temp->raw);
		break;
	    case GLOBAL_PRE_BRANCH:
		thisTemp->globalPreOffset = ((char*)temp - (char*)tramp);
		break;
	    case LOCAL_POST_BRANCH:
		thisTemp->localPostOffset = ((char*)temp - (char*)tramp);
		thisTemp->localPostReturnOffset = thisTemp->localPostOffset
		                                  + 4 * sizeof(temp->raw);
		break;
	    case GLOBAL_POST_BRANCH:
		thisTemp->globalPostOffset = ((char*)temp - (char*)tramp);
		break;
	    case SKIP_PRE_INSN:
                thisTemp->skipPreInsOffset = ((char*)temp - (char*)tramp);
                break;
	    case UPDATE_COST_INSN:
		thisTemp->updateCostOffset = ((char*)temp - (char*)tramp);
		break;
	    case SKIP_POST_INSN:
                thisTemp->skipPostInsOffset = ((char*)temp - (char*)tramp);
                break;
	    case RETURN_INSN:
                thisTemp->returnInsOffset = ((char*)temp - (char*)tramp);
                break;
	    case EMULATE_INSN:
                thisTemp->emulateInsOffset = ((char*)temp - (char*)tramp);
                break;
	    case SAVE_PRE_INSN:
                thisTemp->savePreInsOffset = ((char*)temp - (char*)tramp);
                break;
	    case RESTORE_PRE_INSN:
                thisTemp->restorePreInsOffset = ((char*)temp - (char*)tramp);
                break;
	    case SAVE_POST_INSN:
                thisTemp->savePostInsOffset = ((char*)temp - (char*)tramp);
                break;
	    case RESTORE_POST_INSN:
                thisTemp->restorePostInsOffset = ((char*)temp - (char*)tramp);
                break;
	    case REENTRANT_PRE_INSN_JUMP:
	        thisTemp->recursiveGuardPreJumpOffset = ((char*)temp - (char*)tramp);
	        break;
   	    case REENTRANT_POST_INSN_JUMP:
	        thisTemp->recursiveGuardPostJumpOffset = ((char*)temp - (char*)tramp);
       	        break;
	    default:
	        break;
  	}	
    }
    thisTemp->cost = 8;
    thisTemp->prevBaseCost = 20;
    thisTemp->postBaseCost = 30;
    if (!guardDesired)
      {
	thisTemp->recursiveGuardPreJumpOffset = 0;
	thisTemp->recursiveGuardPostJumpOffset = 0;
      }
    else // Update the costs
      {
	thisTemp->prevBaseCost += 11;  
	thisTemp->postBaseCost += 11;
      }
    thisTemp->prevInstru = thisTemp->postInstru = false;
    thisTemp->size = (int) temp - (int) tramp;
}

registerSpace *regSpace;
#ifdef BPATCH_LIBRARY
// This register space should be used with the conservative base trampoline.
// Right now it's only used for purposes of determining which registers must
// be saved and restored in the base trampoline.
registerSpace *conservativeRegSpace;
#endif

// 11-12 are defined not to have live values at procedure call points.
// reg 3-10 are used to pass arguments to functions.
//   We must save them before we can use them.
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
Register deadRegList[] = { 11 };
#else
Register deadRegList[] = { 11, 12 };
#endif

// allocate in reverse order since we use them to build arguments.
Register liveRegList[] = { 10, 9, 8, 7, 6, 5, 4, 3 };

#ifdef BPATCH_LIBRARY
// If we're being conservative, we don't assume that any registers are dead.
Register conservativeDeadRegList[] = { };
// The registers that aren't preserved by called functions are considered live.
Register conservativeLiveRegList[] = { 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 0 };
#endif

void initTramps()
{
    static bool inited=false;

    if (inited) return;
    inited = true;

    initATramp(&baseTemplateNonRecursive, (instruction *) baseTramp);
    initATramp(&baseTemplate, (instruction *) baseTramp, false);
#ifdef BPATCH_LIBRARY
    initATramp(&conservativeTemplate, (instruction *) conservativeTramp);
#endif

    regSpace = new registerSpace(sizeof(deadRegList)/sizeof(Register), deadRegList, 
				 sizeof(liveRegList)/sizeof(Register), liveRegList);

#ifdef BPATCH_LIBRARY
    // Note that we don't always use this with the conservative base tramp --
    // see the message where we declare conservativeRegSpace.
    conservativeRegSpace =
      new registerSpace(sizeof(conservativeDeadRegList)/sizeof(Register),
		        conservativeDeadRegList, 
		        sizeof(conservativeLiveRegList)/sizeof(Register),
			conservativeLiveRegList);
#endif
}


	///////////////////////////////////////////////////////////////////////
	//Our base trampolines save and restore registers onto the stack using
	//  negative offsets from the stack pointer.  These offsets were
	//  colliding with offsets used by instructions in the application.
	//  STKPAD is an extra offset to translate our offsets further down
	//  the stack to reduce the chance of collision.
	//
	//  -24 to -248    We were colliding in this zone.  STKPAD used here.
	//
	//    -24  to -52    Saves GPR 3 - 10
	//    -56            Saves LR.  STKLR is set to this offset.
	//    -144 to -248   Saves FPR 0 - 13
	//
	//  Above -248     Used by mini trampolines.  STKPAD avoids this zone.
	//
	//  STKPAD values of 0.5KB, 1KB, 2KB, 4KB, 8KB were too small.
	//
#define STKPAD ( 24 * 1024 )

#ifdef BPATCH_LIBRARY
#define STKLR    ( -(8 + (13+1)*4) )
#define STKCR    (STKLR - 4)
#define STKXER	 (STKLR - 8)
#define STKCTR	 (STKLR - 12)
#define STKSPR0  (STKLR - 16)
#define STKFPSCR (STKLR - 24)
#define STKFP    (- STKFPSCR)
#define STKFCALLREGS (STKFP+(14*8))
#define STKKEEP (STKFCALLREGS+((13+1)*4)+STKPAD)
#else
#define STKLR  ( -(8 + (11+1)*4) )
#define STKFP  (8+(32*4))
#define STKFCALLREGS (8+(32*4)+(14*8))
#define STKKEEP 0
#endif

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
  insn->dform.d_or_si = stkOffset - STKPAD;
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
  insn->dform.d_or_si = stkOffset - STKPAD;
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
  insn->dform.d_or_si = stkOffset - STKPAD;
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
  insn->dform.d_or_si = stkOffset - STKPAD;
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
  insn->dform.d_or_si = stkOffset - STKPAD;
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
  insn->dform.d_or_si = stkOffset - STKPAD;
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
  genImmInsn(insn, STFDop, scratchReg, 1, stkOffset - STKPAD);
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
  genImmInsn(insn, LFDop, scratchReg, 1, stkOffset - STKPAD);
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
  genImmInsn(insn, STop, reg, 1, -1*((reg+1)*4 + offset + STKPAD));
  insn++;
  base += sizeof(instruction);
}

static void restoreRegister(instruction *&insn, Address &base, Register reg,
		      int dest, int offset)
{
  genImmInsn(insn, Lop, dest, 1, -1*((reg+1)*4 + offset + STKPAD));
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
  genImmInsn(insn, STFDop, reg, 1, -1*((reg+1)*8 + offset + STKPAD));
  insn++;
  base += sizeof(instruction);
}

static void restoreFPRegister(instruction *&insn, Address &base, Register reg,
		        int dest, int offset)
{
  genImmInsn(insn, LFDop, dest, 1, -1*((reg+1)*8 + offset + STKPAD));
  insn++;
  base += sizeof(instruction);
}

static void restoreFPRegister(instruction *&insn, Address &base, Register reg,
		       int offset)
{
  restoreFPRegister(insn, base, reg, reg, offset);
}	

void saveAllRegistersThatNeedsSaving(instruction *insn, Address &base)
{
   unsigned long numInsn=0;
   for (u_int i = 0; i < regSpace->getRegisterCount(); i++) {
     registerSlot *reg = regSpace->getRegSlot(i);
     if (reg->startsLive) {
       saveRegister(insn,numInsn,reg->number,8);
     }
   }
   base += numInsn/sizeof(instruction);
}

void restoreAllRegistersThatNeededSaving(instruction *insn, Address &base)
{
   unsigned long numInsn=0;
   for (u_int i = 0; i < regSpace->getRegisterCount(); i++) {
     registerSlot *reg = regSpace->getRegSlot(i);
     if (reg->startsLive) {
       restoreRegister(insn,numInsn,reg->number,8);
     }
   }
   base += numInsn/sizeof(instruction);
}

#if defined(MT_THREAD)
void generateMTpreamble(char *insn, Address &base, process *proc)
{
  AstNode *t1,*t2,*t3,*t4,*t5;
  vector<AstNode *> dummy;
  Address tableAddr;
  int value; 
  bool err;
  Register src = Null_Register;

  // registers cleanup
  regSpace->resetSpace();

  /* t3=DYNINSTthreadTable[thr_self()] */
  t1 = new AstNode("DYNINSTthreadPos", dummy);
  value = sizeof(unsigned);
  t4 = new AstNode(AstNode::Constant,(void *)value);
  t2 = new AstNode(timesOp, t1, t4);
  removeAst(t1);
  removeAst(t4);

  tableAddr = proc->findInternalAddress("DYNINSTthreadTable",true,err);
  assert(!err);
  t5 = new AstNode(AstNode::Constant, (void *)tableAddr);
  t3 = new AstNode(plusOp, t2, t5);
  removeAst(t2);
  removeAst(t5);
  src = t3->generateCode(proc, regSpace, insn, base, false, true);
  removeAst(t3);
  instruction *tmp_insn = (instruction *) ((void*)&insn[base]);
  genImmInsn(tmp_insn, ORILop, src, REG_MT, 0);  
  base += sizeof(instruction);
  regSpace->freeRegister(src);
}
#endif


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
trampTemplate *installBaseTramp(const instPoint *location, process *proc,
				bool trampRecursiveDesired = false,
				trampTemplate *templateType = NULL,
                                Address exitTrampAddr = 0,
				Address baseAddr = 0)
{
    unsigned long trampGuardFlagAddr = proc->getTrampGuardFlagAddr();
    Address currAddr;
    instruction *code;
    instruction *temp;
    unsigned offset;
    unsigned long numInsn;
    trampTemplate *theTemplate = NULL;
    bool isReinstall = (baseAddr != 0);
#ifdef BPATCH_LIBRARY
    registerSpace *theRegSpace = NULL;
    if (location->ipLoc == ipOther)
      theRegSpace = conservativeRegSpace;
    else theRegSpace = regSpace;
#endif

    if (templateType) // Already have a preferred template type
      {
	theTemplate = templateType;
      }
    else
      {
#ifdef BPATCH_LIBRARY
	if (location->ipLoc == ipOther) {
	  theTemplate = &conservativeTemplate;
	}
	else
#endif
	{
	  if (trampRecursiveDesired)
	    theTemplate = &baseTemplate;
	  else
	    theTemplate = &baseTemplateNonRecursive;
        }
      }

    // Have we allocated space for the flag yet?
    if (trampGuardFlagAddr == 0)
      {
	int zeroval = 0;
        trampGuardFlagAddr = inferiorMalloc(proc, sizeof(int), dataHeap);
        // Zero out the new value
        proc->writeDataSpace((void *)trampGuardFlagAddr, sizeof(int), &zeroval);
	// Keep track in the process space
	proc->setTrampGuardFlagAddr(trampGuardFlagAddr);
      }

    if (! isReinstall) 
      baseAddr = inferiorMalloc(proc, theTemplate->size, textHeap, location->addr);
    // InferiorMalloc can ignore our "hints" when necessary, but that's bad here.
    assert(DISTANCE(location->addr, baseAddr) <= MAX_BRANCH);

    code = new instruction[theTemplate->size / sizeof(instruction)];
    memcpy((char *) code, (char*) theTemplate->trampTemp, theTemplate->size);
    // bcopy(theTemplate->trampTemp, code, theTemplate->size);

    for (temp = code, currAddr = baseAddr; 
	(int) (currAddr - baseAddr) < theTemplate->size;
	temp++, currAddr += sizeof(instruction)) {
      switch (temp->raw) {
	
      case UPDATE_LR:
	if(location->ipLoc == ipFuncReturn) {
	  // loading the old LR from the 4th word from the stack, 
	  // in the link-area
	  genImmInsn(&code[0], Lop, 0, 1, 12);
	  code[1].raw = MTLR0raw;
	  // the rest of the instrs are already NOOPs
	} else if((location->ipLoc == ipFuncEntry) && (exitTrampAddr != 0)) {
	  code[0].raw = MFLR0raw;
	  // storing the old LR in the 4th word from the stack, 
	  // in the link-area
	  genImmInsn(&code[1], STop, 0, 1, 12); 
	  genImmInsn(&code[2], CAUop, 0, 0, HIGH(exitTrampAddr));
	  genImmInsn(&code[3], ORILop, 0, 0, LOW(exitTrampAddr));
	  code[4].raw = MTLR0raw;
	} else {
	  generateNOOP(temp);
            // the rest of the instrs are already NOOPs
	}
	break;
      case EMULATE_INSN:
	if(location->ipLoc == ipFuncReturn) {
	  generateNOOP(temp);
	} else {
	  *temp = location->originalInstruction;
	  relocateInstruction(temp, location->addr, currAddr);
	}
	break;
      case RETURN_INSN:
	if(location->ipLoc == ipFuncReturn) {
	  temp->raw = BRraw;
	} else {
	  generateBranchInsn(temp, (location->addr + 
				    sizeof(instruction) - currAddr));
	}
	break;
      case SKIP_PRE_INSN:
	//offset = baseAddr+theTemplate->updateCostOffset-currAddr;
	offset = baseAddr+theTemplate->emulateInsOffset-currAddr;
	generateBranchInsn(temp,offset);
	break;
      case SKIP_POST_INSN:
	offset = baseAddr+theTemplate->returnInsOffset-currAddr;
	generateBranchInsn(temp,offset);
	break;
      case UPDATE_COST_INSN:
	theTemplate->costAddr = currAddr;
	generateNOOP(temp);
	break;
      case SAVE_PRE_INSN:
      case SAVE_POST_INSN:
	// Note: we're saving the registers 24K down the stack.
	// This is necessary since the near area around the stack pointer
	// can contain useful data, and we can't tell when this is.
	// So we can't shift the SP down a little ways, and in most
	// cases we don't care about shifting it down a long way here.
	// If we make a function call (emitFuncCall), it handles
	// shifting the SP down.

#ifdef BPATCH_LIBRARY
	for(u_int i = 0; i < theRegSpace->getRegisterCount(); i++) {
	  registerSlot *reg = theRegSpace->getRegSlot(i);
#else
	  for(u_int i = 0; i < regSpace->getRegisterCount(); i++) {
	    registerSlot *reg = regSpace->getRegSlot(i);
#endif
	    if (reg->startsLive) {
	      numInsn = 0;
	      saveRegister(temp,numInsn,reg->number,8);
	      assert(numInsn > 0);
	      currAddr += numInsn;
	    }
	  }

	  // A commented-out closing brace to satisfy the auto-indentation
	  // programs -- uncomment if they get picky
	  //}
		  
	  currAddr += saveLR(temp, 10, STKLR);   //Save link register on stack

#ifdef BPATCH_LIBRARY
	  // If this is a point where we're using a conservative base tramp,
	  // we need to save some more registers.
	  if (location->ipLoc == ipOther) {
	    currAddr += saveCR(temp, 10, STKCR); // Save the condition codes
	    currAddr += saveSPR(temp, 10, SPR_XER, STKXER); // & XER
	    currAddr += saveSPR(temp, 10, SPR_CTR, STKCTR); // & count reg
	    currAddr += saveSPR(temp, 10, 0, STKSPR0);      // & SPR0
	  }
#endif
	  
	  // Also save the floating point registers which could
	  // be modified, f0-r13
	  for(unsigned i=0; i <= 13; i++) {
	    numInsn = 0;
	    saveFPRegister(temp,numInsn,i,STKFP);
	    currAddr += numInsn;
	  }
	  
#ifdef BPATCH_LIBRARY
	  if (location->ipLoc == ipOther) {
	    // Save the floating point status and control register
	    // (we have to do it after saving the floating point registers,
	    // because we need to use one as a temporary)
	    currAddr += saveFPSCR(temp, 13, STKFPSCR);
	  }
#endif
	  
	  temp--;
	  currAddr -= sizeof(instruction);
	  
	  break;
	case RESTORE_PRE_INSN:
	case RESTORE_POST_INSN:
	  // See comment above for why we don't shift SP.
	  // However, the LR is stored in its normal place (at SP+8)
          currAddr += restoreLR(temp, 10, STKLR); //Restore link register from
	  
#ifdef BPATCH_LIBRARY
	  // If this is a point where we're using a conservative base tramp,
	  // we need to restore some more registers.
	  if (location->ipLoc == ipOther) {
	    currAddr += restoreCR(temp, 10, STKCR); // Restore condition codes
	    currAddr += restoreSPR(temp, 10, SPR_XER, STKXER); // & XER
	    currAddr += restoreSPR(temp, 10, SPR_CTR, STKCTR); // & count reg
	    currAddr += restoreSPR(temp, 10, 0, STKSPR0); // & SPR0
	      currAddr += restoreFPSCR(temp, 13, STKFPSCR); // & FPSCR
	  }
#endif
	  
#ifdef BPATCH_LIBRARY
	  for (u_int i = 0; i < theRegSpace->getRegisterCount(); i++) {
	    registerSlot *reg = theRegSpace->getRegSlot(i);
#else
          for (u_int i = 0; i < regSpace->getRegisterCount(); i++) {
	    registerSlot *reg = regSpace->getRegSlot(i);
#endif
	    if (reg->startsLive) {
	      numInsn = 0;
	      restoreRegister(temp,numInsn,reg->number,8);
	      assert(numInsn > 0);
	      currAddr += numInsn;
	    }
	  }
	  // A commented-out closing brace to satisfy the auto-indentation
	  // programs -- uncomment if they get picky
	  //}
	
	  // Also load the floating point registers which were saved
	  // since they could have been modified, f0-r13
	  for(u_int i=0; i <= 13; i++) {
	    numInsn = 0;
	    restoreFPRegister(temp,numInsn,i,STKFP);
	    currAddr += numInsn;
	  }
	  
	  // "Undo" the for loop step, since we've advanced the FP also.
	  temp--;
	  currAddr -= sizeof(instruction);
	  break;
	case LOCAL_PRE_BRANCH:
	case GLOBAL_PRE_BRANCH:
	case LOCAL_POST_BRANCH:
	case GLOBAL_POST_BRANCH:
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
	  if ((temp->raw == LOCAL_PRE_BRANCH) ||
	      (temp->raw == LOCAL_POST_BRANCH)) {
	    temp -= NUM_INSN_MT_PREAMBLE;
	    unsigned numIns=0;
	    generateMTpreamble((char *)temp, numIns, proc);
	    numIns = numIns/sizeof(instruction);
	    if(numIns != NUM_INSN_MT_PREAMBLE) {
	      cerr << "numIns = " << numIns << endl;
	      assert(numIns==NUM_INSN_MT_PREAMBLE);
	    }
	    // if numIns!=NUM_INSN_MT_PREAMBLE then we should update the
	    // space reserved for generateMTpreamble in the base-tramp
	    // template file (tramp-power.S) - naim
	    temp += NUM_INSN_MT_PREAMBLE;
	  }
#endif
	  currAddr += setBRL(temp, 10, 0, NOOPraw); //Basically `nop'
	  
	  temp--;                                   //`for' loop compensate
	  currAddr -= sizeof(instruction);          //`for' loop compensate
	  break;
        case REENTRANT_GUARD_ADDR:
	  if (theTemplate->recursiveGuardPreJumpOffset)
	    {
	      // Need to write the following instructions:
	      // cau 7,0,HIGH(baseAddr)
	      genImmInsn(temp,
			 CAUop,   // Add immediate and shift
			 6,       // Destination
			 0,       // Source
			 HIGH((int)trampGuardFlagAddr));
	      temp++; currAddr += sizeof(instruction); // Step for loop
	      // oril 6,7,LOW(baseAddr)
	      genImmInsn(temp,
			 ORILop,  // OR immediate
			 6,       // Destination
			 6,       // Source
			 LOW((int)trampGuardFlagAddr));
	    }
	  else // No guard wanted, so overwrite with noop
	    temp->raw = NOOPraw;
	  break;
	case REENTRANT_GUARD_LOAD:
	  if (theTemplate->recursiveGuardPreJumpOffset)
	    {
	      // load (addr: 6, dest: 5)
	      genImmInsn(temp,   // instruction
			 Lop,    // Load
			 5,      // Into register 5
			 6,      // Addr in register 6
			 0);     // No offset
	    }
	  else
	    temp->raw = NOOPraw;
	  break;
        case REENTRANT_PRE_INSN_JUMP:
	  if (theTemplate->recursiveGuardPreJumpOffset)
	    {
	      // Need to write the following instruction:
	      // cmp (addr: 5 != 0)
	      genImmInsn(temp,   // Instruction
			 CMPIop, // Compare immediate
			 0,      // CR 0, L=0
			 5,      // Source,
			 0);     // Compare to 0
	      temp++; currAddr += sizeof(instruction);
	      // bcxxx (4, 2, reentrant(Pre|Post)GuardJump);
	      temp->raw = 0;
	      temp->bform.op = BCop; // Branch conditional
	      temp->bform.bo = BFALSEcond; // Branch if false
	      temp->bform.bi = EQcond;
	      temp->bform.bd = ( theTemplate->restorePreInsOffset - 
				 theTemplate->recursiveGuardPreJumpOffset) / 4;
	      temp->bform.aa = 0; // Relative branch
	      temp->bform.lk = 0; // No link req'd
	    }
	  else
	    temp->raw = NOOPraw;
	  break;
        case REENTRANT_POST_INSN_JUMP:
	  if (theTemplate->recursiveGuardPostJumpOffset)
	    {
	      // Need to write the following instruction:
	      // cmp (addr: 5 != 0)
	      genImmInsn(temp,   // Instruction
			 CMPIop, // Compare immediate
			 0,      // CR 0, L=0
			 5,      // Source,
			 0);     // Compare to 0
	      temp++; currAddr += sizeof(instruction);
	      // bcxxx (4, 2, reentrant(Pre|Post)GuardJump);
	      temp->raw = 0;
	      temp->bform.op = BCop; // Branch conditional
	      temp->bform.bo = BFALSEcond; // Branch if false
	      temp->bform.bi = EQcond;
	      temp->bform.bd = ( theTemplate->restorePostInsOffset - 
				 theTemplate->recursiveGuardPostJumpOffset) / 4;
	      temp->bform.aa = 0; // Relative branch
	      temp->bform.lk = 0; // No link req'd
	    }
	  else
	    temp->raw = NOOPraw;
	  break;
        case REENTRANT_GUARD_INC:
	  if (theTemplate->recursiveGuardPreJumpOffset)
	    {
	      // Need to write:
	      // cal 5,1(0) -- set register 5 to value 1
	      genImmInsn(temp,  // instruction 
			 CALop, // add immediate
			 5,     // Destination
			 0,     // Source
			 1);    // Value of 1
	    }
	  else
	    temp->raw = NOOPraw;
	  break;
        case REENTRANT_GUARD_DEC:
	  if (theTemplate->recursiveGuardPreJumpOffset)
	    {
	      // Need to write:
	      // cal 5,0(0)
	      genImmInsn(temp,  // instruction,
			 CALop, // Add immediate
			 5,     // Destination 
			 0,     // Source
			 0);    // Set to 0
	    }
	  else
	    temp->raw = NOOPraw;
	  break;
	case REENTRANT_GUARD_STORE: 
	  if (theTemplate->recursiveGuardPreJumpOffset)
	    {
	      // stw 5,0(6)
	      genImmInsn(temp,
			 STop,
			 5,         // Register 5 
			 6,         // Dest addr in reg 6
			 0);        // 0 offset
	    }
	  else
	    temp->raw = NOOPraw;
	  break;
	default:
          break;
      }
    }
	/*
    for (temp = code, currAddr = baseAddr; 
	(int) (currAddr - baseAddr) < theTemplate->size;
	temp++, currAddr += sizeof(instruction))
      fprintf(stderr, "Op: %u; RT: %u; RA: %u; D: %u\n", temp->dform.op,
	      temp->dform.rt, temp->dform.ra, temp->dform.d_or_si);
	*/
    // TODO cast
    proc->writeDataSpace((caddr_t)baseAddr, theTemplate->size, (caddr_t) code);

    free(code);

    if (isReinstall) return NULL;

    trampTemplate *baseInst = new trampTemplate;
    *baseInst = *theTemplate;
    baseInst->baseAddr = baseAddr;
    return baseInst;
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

     //////////////////////////////////////////////////////////////////////////
     //Given a process and a vector of instPoint's, reinstall all the 
     //  base trampolines that have been damaged by an AIX load.
     //
void findAndReinstallBaseTramps(process                  *p,
				vector<const instPoint*> &allInstPoints)
{
  if (! p) return;

  for (unsigned u = 0; u < allInstPoints.size(); u++) {
    const instPoint *ip = allInstPoints[u];
    trampTemplate   *bt = p->baseMap[ip];                       //Base tramp
    if (baseTrampExists(p, bt->baseAddr)) continue;

    if ((ip->ipLoc == ipFuncEntry) || (ip->ipLoc == ipFuncReturn)) {
      const instPoint *rp = ip->iPgetFunction()->funcExits(p)[0]; //Return point
      trampTemplate *rt = p->baseMap[rp];                       //Return tramp
      // Note: we can safely ignore the return value of installBaseTramp
      // since it is NULL for the reinstallation case. 
      installBaseTramp(rp, p, true, rt, 0, rt->baseAddr);
      rt->updateTrampCost(p, 0);
                                                              
      const instPoint *ep = ip->iPgetFunction()->funcEntry(p);  //Entry point
      trampTemplate *et = p->baseMap[ep];                       //Entry tramp
      installBaseTramp(ep, p, true, et, rt->baseAddr, et->baseAddr);
      et->updateTrampCost(p, 0);
      generateBranch(p, ep->iPgetAddress(), et->baseAddr);
    }
    else {
      installBaseTramp(ip, p, true, bt, 0, bt->baseAddr);
      bt->updateTrampCost(p, 0);
      generateBranch(p, ip->iPgetAddress(), bt->baseAddr);
    }
  }
}



trampTemplate *findAndInstallBaseTramp(process *proc, 
				       instPoint *&location,
				       returnInstance *&retInstance,
				       bool trampRecursiveDesired,
				       bool /*noCost*/)
{
    trampTemplate *ret = NULL;
    process *globalProc;
    retInstance = NULL;

    globalProc = proc;
    if (!globalProc->baseMap.defines(location)) {      
        if((location->ipLoc == ipFuncEntry) ||
	   (location->ipLoc == ipFuncReturn)) {
	  trampTemplate* exTramp;
	  //instruction    code[5];
	  
	  const instPoint* newLoc1 = location->func->funcExits(globalProc)[0];
	  exTramp = installBaseTramp(newLoc1, globalProc, trampRecursiveDesired);
	  globalProc->baseMap[newLoc1] = exTramp;

	  const instPoint* newLoc2 = location->func->funcEntry(globalProc);
	  assert(newLoc2->ipLoc == ipFuncEntry);
          assert(exTramp->baseAddr != 0);
	  ret = installBaseTramp(newLoc2, globalProc, trampRecursiveDesired,
				 NULL, // Don't have a previous trampType to pass in
				 exTramp->baseAddr);

	  instruction *insn = new instruction;
	  generateBranchInsn(insn, ret->baseAddr - newLoc2->addr);
	  globalProc->baseMap[newLoc2] = ret;
	  retInstance = new returnInstance((instruction *)insn, 
					   sizeof(instruction), newLoc2->addr,
					   sizeof(instruction));
	  
	  if(location->ipLoc == ipFuncReturn) {
	    ret = exTramp;
	  }
        } else {
	  ret = installBaseTramp(location, globalProc, trampRecursiveDesired);
	  instruction *insn = new instruction;
	  generateBranchInsn(insn, ret->baseAddr - location->addr);
	  globalProc->baseMap[location] = ret;
	  retInstance = new returnInstance((instruction *)insn, 
					   sizeof(instruction), location->addr,
					   sizeof(instruction));
	}
    } else {
        ret = globalProc->baseMap[location];
    }
      
    return(ret);
}


     //////////////////////////////////////////////////////////////////////////
     //Given a process and a vector of instInstance's, reconstruct branches 
     //  from base trampolines to their mini trampolines
     //
void reattachMiniTramps(process               *p,
			vector<instInstance*> &allInstInstances)
{
  if (! p) return;

  for (unsigned u = 0; u < allInstInstances.size(); u++) {
    instInstance *ii = allInstInstances[u];
    if (ii->prevAtPoint) continue;              //Not first at inst point

    trampTemplate *bt       = ii->baseInstance; //Base trampoline
    Address        skipAddr = bt->baseAddr;
    if (ii->when == callPreInsn)
      skipAddr += bt->skipPreInsOffset;
    else
      skipAddr += bt->skipPostInsOffset;
    generateNoOp(p, skipAddr);                  //Clear "skip" branch
                                                //Restore branch from base tramp
    extern int getBaseBranchAddr(process *, instInstance *);
    resetBRL(p, getBaseBranchAddr(p, ii), ii->trampBase);
  }
}



/*
 * Install a single tramp.
 *
 */
void installTramp(instInstance *inst, char *code, int codeSize) 
{
    totalMiniTramps++;
    insnGenerated += codeSize/sizeof(int);
    
    // TODO cast
    (inst->proc)->writeDataSpace((caddr_t)inst->trampBase, codeSize, code);

#ifdef BPATCH_LIBRARY
    trampTemplate *theTemplate;

    if (inst->location->ipLoc == ipOther)
	theTemplate = &conservativeTemplate;
    else
	theTemplate = &baseTemplate;
#else
    const trampTemplate *theTemplate = &baseTemplate;
#endif

    Address atAddr;
    if (inst->when == callPreInsn) {
	if (inst->baseInstance->prevInstru == false) {
	    atAddr = inst->baseInstance->baseAddr+theTemplate->skipPreInsOffset;
	    inst->baseInstance->cost += inst->baseInstance->prevBaseCost;
	    inst->baseInstance->prevInstru = true;
	    generateNoOp(inst->proc, atAddr);
	}
    }
    else {
	if (inst->baseInstance->postInstru == false) {
	    atAddr = inst->baseInstance->baseAddr+theTemplate->skipPostInsOffset; 
	    inst->baseInstance->cost += inst->baseInstance->postBaseCost;
	    inst->baseInstance->postInstru = true;
	    generateNoOp(inst->proc, atAddr);
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
            point->func->file->fullName, point->func->prettyName, point->addr);
	logLine(errorLine);
#endif
        return(true);
    } else {
	if (point->callee) {
	    return(true);
	} else {
	    return(false);
	}
    }
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
pd_Function *getFunction(instPoint *point)
{
    return(point->callee ? point->callee : point->func);
}


void emitImm(opCode op, Register src1, RegValue src2imm, Register dest, 
                 char *i, Address &base, bool noCost)
{
        //fprintf(stderr,"emitImm(op=%d,src=%d,src2imm=%d,dest=%d)\n",
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
//   generate a branch and link the destination
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
		      const vector<AstNode *> &operands, 
		      const string &callee, process *proc, bool noCost,
		      const function_base *calleefunc)
{
  Address dest;
  bool err;
  vector <Register> srcs;
  
  if (calleefunc)
       dest = calleefunc->getEffectiveAddress(proc);
  else {
       dest = proc->findInternalAddress(callee, false, err);
       if (err) {
	    function_base *func = proc->findOneFunction(callee);
	    if (!func) {
		 ostrstream os(errorLine, 1024, ios::out);
		 os << "Internal error: unable to find addr of " << callee << endl;
		 logLine(errorLine);
		 showErrorCallback(80, (const char *) errorLine);
		 P_abort();
	    }
	    dest = func->getAddress(0);
       }
  }

  // Generate the code for all function parameters, and keep a list
  // of what registers they're in.
  for (unsigned u = 0; u < operands.size(); u++) {
    srcs += operands[u]->generateCode(proc, rs, iPtr, base, false, false);
  }
  
  // generateCode can shift the instruction pointer, so reset insn
  instruction *insn = (instruction *) ((void*)&iPtr[base]);
  vector<int> savedRegs;
  
  //     Save the link register.
  // mflr r0
  insn->raw = MFLR0raw;
  insn++;
  base += sizeof(instruction);
  
  // Register 0 is actually the link register, now. However, since we
  // don't want to overwrite the LR slot, save it as "register 0"
  saveRegister(insn, base, 0, STKFCALLREGS);
  // Add 0 to the list of saved registers
  savedRegs += 0;
  
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
  // save REG_MT
  saveRegister(insn,base,REG_MT,STKFCALLREGS);
  savedRegs += REG_MT;
#endif
  
  // see what others we need to save.
  for (u_int i = 0; i < regSpace->getRegisterCount(); i++) {
    registerSlot *reg = regSpace->getRegSlot(i);
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
    } else if (reg->inUse && !reg->mustRestore) {
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
	saveRegister(insn, base, reg->number, STKFCALLREGS);
	savedRegs += reg->number;
      }
    } else if (reg->inUse) {
      // only inuse registers permitted here are the parameters.
      unsigned u;
      for (u=0; u<srcs.size(); u++){
	if (reg->number == srcs[u]) break;
      }
      if (u == srcs.size()) {
	// XXXX - caller saves register that is in use.  We have no
	//    place to save this, but we must save it!!!.  Should
	//    find a place to push this on the stack - jkh 7/31/95
	string msg = "Too many registers required for MDL expression\n";
	fprintf(stderr, msg.string_of());
	showErrorCallback(94,msg);
	cleanUpAndExit(-1);
      }
    }
  }
  
  
  if(srcs.size() > 8) {
    // This is not necessarily true; more then 8 arguments could be passed,
    // the first 8 need to be in registers while the others need to be on
    // the stack, -- sec 3/1/97
    string msg = "Too many arguments to function call in instrumentation code:"
                " only 8 arguments can (currently) be passed on the POWER architecture.\n";
    fprintf(stderr, msg.string_of());
    showErrorCallback(94,msg);
    cleanUpAndExit(-1);
  }
  
  // Now load the parameters into registers.
  for (unsigned u=0; u<srcs.size(); u++){
    // check that is is not already in the register
    if (srcs[u] == (unsigned int) u+3) {
      regSpace->freeRegister(srcs[u]);
      continue;
    }

    assert(regSpace->isFreeRegister(u+3));

    // internal error we expect this register to be free here
    // if (!regSpace->isFreeRegister(u+3)) abort();
    
    genImmInsn(insn, ORILop, srcs[u], u+3, 0);
    insn++;
    base += sizeof(instruction);
    
    // source register is now free.
    regSpace->freeRegister(srcs[u]);
  }
  
  // 
  // Update the stack pointer
  //   argarea  =  8 words  (8 registers)
  //   linkarea =  6 words  (6 registers)
  //   nfuncrs  =  32 words (area we use when saving the registers above 
  //                         ngprs at the beginning of this function call; 
  //                         this area saves the registers already being
  //                         used in the mini trampoline, registers which
  //                         need to be accessed after this function call;
  //                         this area is used in emitFuncCall--not all the
  //                         registers are being saved, only the necessary
  //                         ones)
  //   nfprs    =  14 words (area we use to save the floatpoing registers,
  //                         not all the fprs, only the volatile ones 0-13)
  //   ngprs    =  32 words (area we use when saving the registers above the
  //                         stack pointer, at the beginning of the base
  //                         trampoline--not all the gprs are saved, only the
  //   ???      =  2  words  (two extra word was added on for extra space, 
  //                          and to make sure the floatpings are aligned)
  // OLD STACK POINTER
  //   szdsa    =  4*(argarea+linkarea+nfuncrs+nfprs+ngprs) = 368 + 8 = 376
  // TODO: move this to arch-power. Here for compiling speed reasons.
#define emitFuncCallStackSpace 376

  // So as of now, we have a big chunk of space at SP-24K (approx) which
  // is used. This is not a good situation, as we have _no clue_ what 
  // our instrumentation is going to do. Simple answer: shift the stack
  // pointer past our saved area. Not efficient in terms of space (at 24K
  // a pop), but safe.
  // Take current SP, decrease by frame amount, and store (both) in
  // register 1 and in memory. Back chain to "caller"
  genImmInsn(insn, STUop, REG_SP, REG_SP, -emitFuncCallStackSpace - STKKEEP);
  insn++;
  base += sizeof(instruction);
    
  // save and reset the value of TOC 
  vector<int> toc_info = proc->getTOCoffsetInfo();
  assert(toc_info.size()==3);
  insn->raw = toc_info[0]; insn++; base += sizeof(instruction);
  insn->raw = toc_info[1]; insn++; base += sizeof(instruction);
  insn->raw = toc_info[2]; insn++; base += sizeof(instruction);
  // 0x90410014 0x3c402000 0x6042fd7c 
  // cout << "Dummy" << dummy[0] << "\t" << dummy[1]<< "\t" << dummy[2]<<endl;

  // generate a to the subroutine to be called.
  // load r0 with address, then move to link reg and branch and link.
  
  // TODO: can we do this with a branch to fixed, instead of a
  // branch-to-link-register?
  // Yes, but we then have to put all of the minitramps in
  // the text heap (which is limited), not the data heap (which isn't).
  // We can't guarantee that we can reach a function from the data
  // heap, and odds are against us. 
  // Set the upper half of the link register
  genImmInsn(insn, CAUop, 0, 0, HIGH(dest));
  insn++;
  base += sizeof(instruction);
  
  // Set lower half
  genImmInsn(insn, ORILop, 0, 0, LOW(dest));
  insn++;
  base += sizeof(instruction);
  
  // Move to link register
  insn->raw = MTLR0raw;
  insn++;
  base += sizeof(instruction);
  
  // brl - branch and link through the link reg.
  insn->raw = BRLraw;
  insn++;
  base += sizeof(instruction);
  
  // should there be a nop of some sort here?, sec
  // No: AFAIK, the nop is an artifact of the linker, which we bypass.
  // -- bernat
  //insn->raw = 0x4ffffb82;  // nop
  //insn++;
  //base += sizeof(instruction);
  
  // now cleanup.
  genImmInsn(insn, CALop, REG_SP, REG_SP, emitFuncCallStackSpace + STKKEEP);
  insn++;
  base += sizeof(instruction);
  
  // sec
  // Probably want this in -- bernat
  // But currently it makes test 1.12 fail in the Dyninst suite
  // (insert/remove and malloc/free, but I don't know why
  //  // restore TOC
  //genImmInsn(insn, Lop, 2, 1, 20);
  //insn++;
  //base += sizeof(instruction);

  // get a register to keep the return value in.
  Register retReg = regSpace->allocateRegister(iPtr, base, noCost);

  // allocateRegister can generate code. Reset insn
  insn = (instruction *) ((void*)&iPtr[base]);

  // put the return value from register 3 to the newly allocated register.
  genImmInsn(insn, ORILop, 3, retReg, 0);
  insn++;
  base += sizeof(instruction);
  
  // restore saved registers.
  for (u_int ui = 0; ui < savedRegs.size(); ui++) {
    restoreRegister(insn,base,savedRegs[ui],STKFCALLREGS);
  }
  
  // mtlr	0 (aka mtspr 8, rs) = 0x7c0803a6
  insn->raw = MTLR0raw;
  insn++;
  base += sizeof(instruction);
  
  // return value is the register with the return value from the called function
  return(retReg);
}

 
Address emitA(opCode op, Register src1, Register /*src2*/, Register dest,
	      char *baseInsn, Address &base, bool /*noCost*/)
{
    //fprintf(stderr,"emitA(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

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
        abort();        // unexpected op for this emit!
  }
}

Register emitR(opCode op, Register src1, Register /*src2*/, Register dest,
	      char *baseInsn, Address &base, bool /*noCost*/)
{
    //fprintf(stderr,"emitR(op=%d,src1=%d,src2=XX,dest=%d)\n",op,src1,dest);

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
	restoreRegister(insn, base, src1+3, dest, 8);
	return(dest);
      } else {
        genImmInsn(insn, Lop, dest, 1, (src1+6)*4);
        insn++;	
        base += sizeof(instruction);
        return(dest);
      }
    }
    default:
        abort();        // unexpected op for this emit!
  }
}

void emitVload(opCode op, Address src1, Register /*src2*/, Register dest,
	      char *baseInsn, Address &base, bool /*noCost*/, int /* size */)
{
    instruction *insn = (instruction *) ((void*)&baseInsn[base]);

    if (op == loadConstOp) {
	// constants are signed
	int constValue = (int) src1;
	if (ABS(constValue) > MAX_IMM) {
	    // really addis dest,0,HIGH(src1) aka lis dest, HIGH(src1)
	    genImmInsn(insn, CAUop, dest, 0, HIGH(constValue));
	    insn++;

	    // ori dest,dest,LOW(src1)
	    genImmInsn(insn, ORILop, dest, dest, LOW(constValue));
	    base += 2 * sizeof(instruction);
	} else {
	    // really add regd,0,imm
	    genImmInsn(insn, CALop, dest, 0, constValue);
	    base += sizeof(instruction);
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
	genImmInsn(insn, Lop, dest, dest, LOW(src1));

        base += sizeof(instruction)*2;
    } else if (op == loadFrameRelativeOp) {
	// return the value that is FP offset from the original fp
	int offset = (int) src1;

	if ((offset < MIN_IMM16) || (offset > MAX_IMM16)) {
	    abort();
	} else {
	    genImmInsn(insn, Lop, dest, REG_SP, offset);
	    insn++;
            base += sizeof(instruction)*2;
	}
    } else if (op == loadFrameAddr) {
	// offsets are signed!
	int offset = (int) src1;

	if ((offset < MIN_IMM16) || (offset > MAX_IMM16)) {
	    abort();
	} else {
	    genImmInsn(insn, CALop, dest, REG_SP, offset);
	    insn++;
	    base += sizeof(instruction);
	}
    } else {
        abort();       // unexpected op for this emit!
    }
}

void emitVstore(opCode op, Register src1, Register /*src2*/, Address dest,
	      char *baseInsn, Address &base, bool noCost, int /* size */)
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
	    abort();
	} else {
	    genImmInsn(insn, STop, src1, REG_SP, offset);
	    base += sizeof(instruction);
	    insn++;
	}
    } else {
        abort();       // unexpected op for this emit!
    }
}

void emitVupdate(opCode op, RegValue src1, Register /*src2*/, Address dest,
	      char *baseInsn, Address &base, bool noCost)
{
    instruction *insn = (instruction *) ((void*)&baseInsn[base]);

    if (op == updateCostOp) {
        if (!noCost) {

           regSpace->resetSpace();

	   // add in the cost to the passed pointer variable.

	   // high order bits of the address of the cummulative cost.
	   Register obsCostAddr = regSpace->allocateRegister(baseInsn, base, noCost);

	   // actual cost.
	   Register obsCostValue = regSpace->allocateRegister(baseInsn, base, noCost);

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
        abort();       // unexpected op for this emit!
    }
}

void emitV(opCode op, Register src1, Register src2, Register dest,
              char *baseInsn, Address &base, bool /*noCost*/, int /* size */)
{
    //fprintf(stderr,"emitV(op=%d,src1=%d,src2=%d,dest=%d)\n",op,src1,src2,dest)
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
                fprintf(stderr, "Invalid op passed to emit, instOp = %d\n", 
                        instOp);
                assert(0 && "Invalid op passed to emit");
                abort();
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


bool isCallInsn(const instruction i)
{
  #define CALLmatch 0x48000001 /* bl */

  // Only look for 'bl' instructions for now, although a branch
  // could be a call function, and it doesn't need to set the link
  // register if it is the last function call
  return(isInsnType(i, OPmask | AALKmask, CALLmatch));
}

/*
 * We need to also find calls that don't go to a known location.
 * AFAIK, POWER has no "branch to register" calls. It does, however,
 * have branch to link register and branch to count register calls.
 */

bool isDynamicCall(const instruction i)
{
  #define BRLCmatch 0x4C000001 /* bLR and bCR have opcode 19 */
  if (isInsnType(i, OPmask | LLmask, BRLCmatch))
    return true;
  return false;
}

/* ************************************************************************
   --  This function, isReturnInsn, is no longer needed.  -- sec

// Check for a real return instruction.  Unfortunatly, on RS6000 there are
//    several factors that make this difficult:
//
//    br - is the "normal" return instruction
//    b <cleanupRoutine> is sometimes used to restore fp state.
//    bctr - can be used, but it is also the instruction used for jump tables.
//  
//    However, due to the traceback table convention the instruction after 
//    a real return is always all zeros (i.e. the flag for the start of a
//    tracback record).
//    Also, jump tables never seem to have a 0 offset so the sequence:
//		bctr
//		.long 0
//    doesn't ever appear in a case statement.
//
//    We use the following heuristics.  
//        1.) Any br is a return from function
//        2.) bctr or b insn followed by zero insn is a return from a function.
//
// WARNING:  We use the heuristic that any return insns followed by a 0
//   is the last return in the current function.
//
bool isReturnInsn(const image *owner, Address adr)
{
  bool        ret;
  instruction instr;
  instruction nextInstr;

#ifdef ndef
  // Currently there is a problem with finding the return insn on AIX,
  // which was found once we started using xlc to compile programs
  // xlc uses a br branch to branch inside the function, in addition with
  // optmize flags, xlc using conditional branch returns; a conditional branch
  // which could branch to somewhere inside of the function or leave the
  // function we are working on a new way to handle this, but for it has not
  // been implemented, tested, etc. as of now, so it is not here.
  // if you have any questins, contact sec@cs.wisc.edu

  // Finding a return instruction is becomming harder then predicted,
  // right now be safe and allow a bctr or br followed by a 
  // 0, raw, instruction--won't always work, but will never fail
  // this will be fixed soon, being worked on now...  -- sec
  instr.raw = owner->get_instruction(adr);
  nextInstr.raw = owner->get_instruction(adr+4);
  if(isInsnType(instr, FULLmask, BRraw) ||
     isInsnType(instr, FULLmask, BCTRraw)) {     // bctr, br, or b
    if(nextInstr.raw == 0) {
      ret = true;
    } else {
      ret = false;
    }
  } else {
    ret = false;
  }
#else
  instr.raw = owner->get_instruction(adr);
  nextInstr.raw = owner->get_instruction(adr+4);
  
  if(isInsnType(instr, FULLmask, BRraw)) {      // br instruction
    ret = true;
  } else if (isInsnType(instr, FULLmask, BCTRraw) ||
	     isInsnType(instr, Bmask, Bmatch)) {  // bctr or b 
    if (nextInstr.raw == 0) {
      ret = true;
    } else {
      ret = false;
    }
  } else {
    ret = false;
  }
#endif

  return ret;
}
 * ************************************************************************ */

bool pd_Function::findInstPoints(const image *owner) 
{  
  Address adr = getAddress(0);
  instruction instr;
  bool err;

  instr.raw = owner->get_instruction(adr);
  if (!IS_VALID_INSN(instr)) {
    return false;
  }

  funcEntry_ = new instPoint(this, instr, owner, adr, true, ipFuncEntry);
  assert(funcEntry_);

  // instead of finding the return instruction, we are creating a 
  // return base trampoline which will be called when the function really
  // exits.  what will happen is that the link register, which holds the
  // pc for the return branch, will be changed to point ot this trampoline,
  //
  // !!! Note that because there is no specific instruction that corresponds
  //     to this point, we don't have an address to pass to the instPoint
  //     constructor.  Because Dyninst maintains a mapping from addresses to
  //     instrumentation points, it requires that each point has a unique
  //     address.  In order to provide this, we pass the address of the
  //     entry point + 1 as the address of the exit point.  Since an
  //     instruction must be on a word boundary, this address is guaranteed
  //     not to be used by any other point in the function.
  instruction retInsn;
  retInsn.raw = 0;
  funcReturns+= new instPoint(this, retInsn, owner, adr+1, false, ipFuncReturn);

  instr.raw = owner->get_instruction(adr);
  while(instr.raw != 0x0) {
    if (isCallInsn(instr)) {
      // Define the call point
      adr = newCallPoint(adr, instr, owner, err);
      if (err) return false;
    }
    else if (isDynamicCall(instr)) {
      // Define the call point
      adr = newCallPoint(adr, instr, owner, err);
      if (err) return false;
    }

    // now do the next instruction
    adr += sizeof(instruction);
    instr.raw = owner->get_instruction(adr);
  }

  return(true);
}



//
// Each processor may have a different heap layout.
//   So we make this processor specific.
//
// find all DYNINST symbols that are data symbols
bool process::heapIsOk(const vector<sym_data> &find_us) {
  Address instHeapStart;
  Symbol sym;
  string str;

  // find the main function
  // first look for main or _main
  if (!((mainFunction = findOneFunction("main")) 
        || (mainFunction = findOneFunction("_main")))) {
     string msg = "Cannot find main. Exiting.";
     statusLine(msg.string_of());
     showErrorCallback(50, msg);
     return false;
  }

  for (unsigned i=0; i<find_us.size(); i++) {
    str = find_us[i].name;
    if (!symbols->symbol_info(str, sym)) {
      string str1 = string("_") + str.string_of();
      if (!symbols->symbol_info(str1, sym) && find_us[i].must_find) {
	string msg;
        msg = string("Cannot find ") + str + string(". Exiting");
	statusLine(msg.string_of());
	showErrorCallback(50, msg);
	return false;
      }
    }
  }

#if 0
  //string ghb = "_DYNINSTtext";
  //aix.C does not change the leading "." of function names to "_" anymore.
  //  Instead, the "." is simply skipped.
  string ghb = "DYNINSTtext";
  if (!symbols->symbol_info(ghb, sym)) {
      string msg;
      msg = string("Cannot find ") + ghb + string(". Exiting");
      statusLine(msg.string_of());
      showErrorCallback(50, msg);
      return false;
  }
  instHeapStart = sym.addr();

  // check that we can get to our heap.
  if (instHeapStart > getMaxBranch() + symbols->codeOffset()) {
    logLine("*** FATAL ERROR: Program text + data too big for dyninst\n");
    sprintf(errorLine, "    heap starts at 0x%lx\n", instHeapStart);
    logLine(errorLine);
    sprintf(errorLine, "    max reachable at 0x%lx\n", 
	getMaxBranch() + symbols->codeOffset());
    logLine(errorLine);
    showErrorCallback(53,(const char *) errorLine);
    return false;
  } else if (instHeapStart + SYN_INST_BUF_SIZE > 
	     getMaxBranch() + symbols->codeOffset()) {
    logLine("WARNING: Program text + data could be too big for dyninst\n");
    showErrorCallback(54,(const char *) errorLine);
    return false;
  }
  string hd = "DYNINSTdata";
  if (!symbols->symbol_info(hd, sym)) {
      string msg;
      msg = string("Cannot find ") + hd + string(". Exiting");
      statusLine(msg.string_of());
      showErrorCallback(50, msg);
      return false;
  }
#endif
  return true;
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


bool returnInstance::checkReturnInstance(const vector<Address> &/*adr*/,
					 u_int &/*index*/)
{
#ifdef ndef  
// TODO: implement this.  This stuff is not implemented for this platform 
    for(u_int i=0; i < adr.size(); i++){
        index = i;
        if ((adr[i] > addr_) && ( adr[i] <= addr_+size_)){
	     return false;
        }
    }
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

void generateBreakPoint(instruction &insn) { // instP.h
    insn.raw = BREAK_POINT_INSN;
}

void generateIllegalInsn(instruction &insn) { // instP.h
   insn.raw = 0; // I think that on power, this is an illegal instruction (someone check this please) --ari
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

bool completeTheFork(process *parentProc, int childpid) {
   // no "process" structure yet exists for the child.
   // a fork syscall has just happened.  On AIX, this means that
   // the text segment of the child has been reset instead of copied
   // from the parent process.  This routine "completes" the fork so that
   // it behaves like a normal UNIX fork.

   // First, we copy everything from the parent's inferior text heap
   // to the child.  To do this, we loop thru every allocated item in

   forkexec_cerr << "WELCOME to completeTheFork parent pid is " << parentProc->getPid()
                 << ", child pid is " << childpid << endl;

   vector<heapItem*> srcAllocatedBlocks = parentProc->heap.heapActive.values();

   char buffer[2048];
   const unsigned max_read = 1024;

   for (unsigned lcv=0; lcv < srcAllocatedBlocks.size(); lcv++) {
      const heapItem &srcItem = *srcAllocatedBlocks[lcv];

      assert(srcItem.status == HEAPallocated);
      unsigned addr = srcItem.addr;
      int      len  = srcItem.length;
      const unsigned last_addr = addr + len - 1;

      unsigned start_addr = addr;
      while (start_addr <= last_addr) {
	 unsigned this_time_len = len;
	 if (this_time_len > max_read)
	    this_time_len = max_read;

	 if (!parentProc->readDataSpace((const void*)addr, this_time_len, buffer, true))
	    assert(false);

	 // now write "this_time_len" bytes from "buffer" into the inferior process,
	 // starting at "addr".
	 if (-1 == ptrace(PT_WRITE_BLOCK, childpid, (int*)addr, this_time_len,
			  (int*)buffer))
	    assert(false);

	 start_addr += this_time_len;
      }
   }

   // Okay that completes the first part; the inferior text heap contents have
   // been copied.  In other words, the base and mini tramps have been copied.
   // But now we need to update parts where the code that jumped to the base
   // tramps.

   // How do we do this?  We loop thru all instInstance's of the parent process.
   // Fields of interest are:
   // 1) location (type instPoint*) -- where the code was put
   // 2) trampBase (type unsigned)  -- base of code.
   // 3) baseInstance (type trampTemplate*) -- base trampoline instance

   // We can use "location" as the index into dictionary "baseMap"
   // of the parent process to get a "trampTemplate".

   vector<instInstance*> allParentInstInstances;
   getAllInstInstancesForProcess(parentProc, allParentInstInstances);

   for (unsigned lcv=0; lcv < allParentInstInstances.size(); lcv++) {
      instInstance *inst = allParentInstInstances[lcv];
      assert(inst);

      instPoint *theLocation = inst->location;
      unsigned addr = theLocation->addr;

      // I don't think we need these - naim
      //unsigned   theTrampBase = inst->trampBase;
      //trampTemplate *theBaseInstance = inst->baseInstance;

      // I had to comment out the following line because it was causing 
      // problems. Also, I don't understand why do we want to overwrite the
      // content of the baseAddr field in the parent - naim
      //if (theBaseInstance) theBaseInstance->baseAddr = theTrampBase;

      if (theLocation->addr==0) {
	// This happens when we are only instrumenting the return point of
	// a function, so we need to find the address where to insert the
	// jump to the base trampoline somewhere else. Actually, if we have
	// instrumentation at the entry point, this is not necessary, but
	// it looks easier this way - naim
	const function_base *base_f = theLocation->iPgetFunction();
	assert(base_f);
	const instPoint *ip = base_f->funcEntry(parentProc);
	assert(ip);
	addr = ip->addr;
      }
      assert(addr);

      // Now all we need is a "returnInstance", which contains
      // "instructionSeq", a sequence of instructions to be installed,
      // and "addr_", an address to write to.

      // But for now, since we always relocate exactly ONE
      // instruction on AIX, we can hack our way around without
      // the returnInstance.

      // So, we need to copy one word.  The word to copy can be found
      // at address "theLocation->addr" for AIX.  The original instruction
      // can be found at "theLocation->originalInstruction", but we don't
      // need it.  So, we ptrace-read 1 word @ theLocation->addr from
      // the parent process, and then ptrace-write it to the same location
      // in the child process.

      int data; // big enough to hold 1 instr

      errno = 0;
      data = ptrace(PT_READ_I, parentProc->getPid(), 
		    (int*)addr, 0, 0);
      if (data == -1 && errno != 0)
	 assert(false);

      errno = 0;
      if (-1 == ptrace(PT_WRITE_I, childpid, (int*)addr, data, 0) &&
	  errno != 0)
	 assert(false);
   }

   return true;
}


// hasBeenBound: returns false
// dynamic linking not implemented on this platform
bool process::hasBeenBound(const relocationEntry ,pd_Function *&, Address ) {
    return false;
}

// findCallee: returns false unless callee is already set in instPoint
// dynamic linking not implemented on this platform
bool process::findCallee(instPoint &instr, function_base *&target){

    if((target = const_cast<function_base *>(instr.iPgetCallee()))) {
       return true; // callee already set
    }
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
				  const function_base *newFunc) {

    // Must be a call site
    if (point->ipLoc != ipFuncCallPoint)
	return false;

    // Cannot already be instrumented with a base tramp
    if (baseMap.defines(point))
	return false;

    instruction newInsn;
    if (newFunc == NULL) {	// Replace with a NOOP
	generateNOOP(&newInsn);
    } else {			// Replace with a new call instruction
	generateBranchInsn(&newInsn, newFunc->addr()-point->addr);
	newInsn.iform.lk = 1;
    }

    writeTextSpace((caddr_t)point->addr, sizeof(instruction), &newInsn);

    return true;
}

// Emit code to jump to function CALLEE without linking.  (I.e., when
// CALLEE returns, it returns to the current caller.)

// 18FEB00 -- I removed the parameter names to get rid of compiler
// warnings. They are copied below.
// opCode op, char *i, Address &base, const function_base *callee, process *proc

void emitFuncJump(opCode, 
		  char *, Address &, 
		  const function_base *unused, process *)
{
  // Get rid of a warning about the unused parameter
     if (unused) ;
     /* Unimplemented on this platform! */
     assert(0);
}

#define REG_LR 64
#define REG_CTR 65

void emitLoadPreviousStackFrameRegister(Address register_num, 
					Register dest,
					char *insn,
					Address &base,
					int size,
					bool noCost)
{
  // Offset if needed
  int offset;
  // Pointer to instruction space if needed
  instruction *temp = (instruction *) ((void*)&insn[base]);
  // We need values to define special registers.
  switch ( (int) register_num)
    {
    case REG_LR:
      // LR is saved down the stack
      offset = STKLR - STKPAD; 
      // Get address (SP + offset) and stick in register dest.
      emitImm(plusOp ,(Register) REG_SP, (RegValue) offset, dest, insn, 
	      base, noCost);
      // Load LR into register dest
      emitV(loadIndirOp, dest, 0, dest, insn, base, noCost, size);
      break;
    case REG_CTR:
      // Not saved on the stack, still in a register
      // CTR in SPR 9
      temp->raw = 0;                    //mfspr:  mflr scratchReg
      temp->xfxform.op = 31;
      temp->xfxform.rt = dest;
      temp->xfxform.spr = 9;
      temp->xfxform.xo = 339;
      base += sizeof(instruction);
      break;
    default:
      cerr << "Fallthrough in emitLoadPreviousStackFrameRegister" << endl;
      cerr << "Unexpected register " << register_num << endl;
      abort();
      break;
    }
}

#ifndef BPATCH_LIBRARY
bool process::isDynamicCallSite(instPoint *callSite){
  function_base *temp;
  if(!findCallee(*(callSite),temp)){
    return true;
  }
  return false;
}

bool process::MonitorCallSite(instPoint *callSite){
  instruction i = callSite->originalInstruction;
  vector<AstNode *> the_args(2);
  Register branch_target;
  // Is this a branch conditional link register (BCLR)
  // BCLR uses the xlform (6,5,5,5,10,1)
  if(i.xlform.op == BCLRop) // BLR or BCR
    {
      if (i.xlform.xo == BCLRxop) // BLR (bclr)
	{
	  branch_target = REG_LR;
	}
      else if (i.xlform.xo == BCCTRxop)
	{
	  branch_target = REG_CTR;
	}
      else
	{
	  cerr << "MonitorCallSite: Unknown extended opcode " << i.xlform.xo << endl;
	  return false;
	}
      // Where we're jumping to (link register)
      the_args[0] = new AstNode(AstNode::PreviousStackFrameDataReg,
				(void *) branch_target); 
      // Where we are now
      the_args[1] = new AstNode(AstNode::Constant,
				(void *) callSite->iPgetAddress());
      // Monitoring function
      AstNode *func = new AstNode("DYNINSTRegisterCallee", 
				  the_args);
      addInstFunc(this, callSite, func, callPreInsn,
		  orderFirstAtPoint,
		  true,                              /* noCost flag                */
		  false);                            /* trampRecursiveDesired flag */
      return true;
    }
  else
    {
      cerr << "MonitorCallSite: Unknown opcode " << i.xlform.op << endl; 
      return false;
    }

  
}

#endif


#ifdef BPATCH_LIBRARY
/*
 * createInstructionInstPoint
 *
 * Create a BPatch_point instrumentation point at the given address, which
 * is guaranteed not be one of the "standard" inst points.
 *
 * proc         The process in which to create the inst point.
 * address      The address for which to create the point.
 */
BPatch_point *createInstructionInstPoint(process *proc, void *address)
{
    int i;

    function_base *func = proc->findFunctionIn((Address)address);

    if (!isAligned((Address)address))
	return NULL;

    instruction instr;
    proc->readTextSpace(address, sizeof(instruction), &instr.raw);

    instPoint *newpt = new instPoint((pd_Function *)func,
				     instr,
				     NULL, // image *owner - this is ignored
				     (Address)address,
				     false, // bool delayOk - this is ignored
				     ipOther);

    return proc->findOrCreateBPPoint(NULL, newpt, BPatch_instruction);
}

/*
 * BPatch_point::getDisplacedInstructions
 *
 * Returns the instructions to be relocated when instrumentation is inserted
 * at this point.  Returns the number of bytes taken up by these instructions.
 *
 * maxSize      The maximum number of bytes of instructions to return.
 * insns        A pointer to a buffer in which to return the instructions.
 */
int BPatch_point::getDisplacedInstructions(int maxSize, void *insns)
{
    if (maxSize >= sizeof(instruction))
        memcpy(insns, &point->originalInstruction.raw, sizeof(instruction));

    return sizeof(instruction);
}

#endif
