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
 * inst-hppa.C - Identify instrumentation points for PA-RISC processors.
 *
 * $Log: inst-hppa.C,v $
 * Revision 1.27  1996/11/11 01:53:08  lzheng
 * Moved the instructions which is used to caculate the observed cost
 * from the miniTramps to baseTramp
 *
 * Revision 1.26  1996/10/31 08:46:44  tamches
 * the shm-sampling commit
 *
 * Revision 1.25  1996/10/30 02:46:08  lzheng
 * Minor bug fixes for call site instrumentation and emitImm.
 *
 * Revision 1.24  1996/10/18 23:52:39  mjrg
 * Changed argument of findInstPoints
 *
 * Revision 1.23  1996/10/07 22:01:47  lzheng
 * Adding the function emitImm
 *
 * Revision 1.22  1996/10/04 14:57:58  naim
 * Moving save/restore instructions from mini-tramp to base-tramp. Also, changes
 * to the base-tramp to support arrays of counters and timers (multithreaded
 * case) - naim
 *
 * Revision 1.21  1996/10/03 22:12:09  mjrg
 * Removed multiple stop/continues when inserting instrumentation
 * Fixed bug on process termination
 * Removed machine dependent code from metric.C and process.C
 *
 * Revision 1.20  1996/09/13 21:41:53  mjrg
 * Implemented opcode ReturnVal for ast's to get the return value of functions.
 * Added missing calls to free registers in Ast.generateCode and emitFuncCall.
 * Removed architecture dependencies from inst.C.
 * Changed code to allow base tramps of variable size.
 *
 * Revision 1.19  1996/09/05 16:34:28  lzheng
 * Move the architecture dependent definations to the architecture dependent files
 *
 * Revision 1.18  1996/08/20 19:21:09  lzheng
 * Implementation of moving multiple instructions sequence and
 * splitting the instrumentation into two phases
 *
 * Revision 1.17  1996/08/16 21:18:49  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.16  1996/08/16 16:32:47  lzheng
 * Minor fixing related to the previous commits
 *
 * Revision 1.15  1996/08/16 04:06:00  lzheng
 * Minor changes for the change of paradyn.rc and bug fixing.
 *
 * Revision 1.14  1996/07/18 19:48:35  naim
 * Changing the "frequency" value from 250 to 100 - naim
 *
 * Revision 1.13  1996/07/09 04:19:26  lzheng
 * Implemented the multiple exit and relocation of conditional branch
 * instruction on HPUX
 *
 * Revision 1.12  1996/05/09 19:24:18  lzheng
 * Minor changes for the return value of procedure findInstPoint
 *
 * Revision 1.11  1996/04/29 22:18:42  mjrg
 * Added size to functions (get size from symbol table)
 * Use size to define function boundary
 * Find multiple return points for sparc
 * Instrument branches and jumps out of a function as return points (sparc)
 * Recognize tail-call optimizations and instrument them as return points (sparc)
 * Move instPoint to machine dependent files
 *
 * Revision 1.10  1996/04/26 20:38:47  lzheng
 * Some changes are moved so that function arguments registers are saved
 * in miniTrampoline.
 * Changes in emitFuncCall to remove the #ifdef in ast.C
 *
 */

#include "util/h/headers.h"

#include "rtinst/h/rtinst.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "inst-hppa.h"
#include "arch-hppa.h"
#include "ast.h"
#include "util.h"
#include "internalMetrics.h"
#include "stats.h"
#include "os.h"
#include "showerror.h"
#include <sys/mman.h>

#define perror(a) P_abort();

extern bool isPowerOf2(int value, int &result);

class instPoint {
public:
  instPoint(pdFunction *f, const instruction &instr, const image *owner,
	    const Address adr, const bool delayOK, bool &err,
	    instPointType pointType = noneType);

  ~instPoint() {  /* TODO */ }

  // can't set this in the constructor because call points can't be classified until
  // all functions have been seen -- this might be cleaned up
  void set_callee(pdFunction *to) { callee = to; }


  Address addr;                   /* address of inst point */
  instruction originalInstruction;    /* original instruction */
  instruction nextInstruction;
  instPointType ipType;

  instruction delaySlotInsn;  /* original instruction */
  instruction aggregateInsn;  /* aggregate insn */
  bool inDelaySlot;            /* Is the instruction in a delay slot */
  bool isDelayed;		/* is the instruction a delayed instruction */
  bool callIndirect;		/* is it a call whose target is rt computed ? */
  bool callAggregate;		/* calling a func that returns an aggregate
				   we need to reolcate three insns in this case
				   */
  pdFunction *callee;		/* what function is called */
  pdFunction *func;		/* what function we are inst */

};


#define ABS(x)		((x) > 0 ? x : -x)
//#define MAX_BRANCH      ((0x1<<18)+8) /* 17-signed bits, lshift by 2, +8 */
#define MAX_BRANCH         (0x01<<31)    /* assumed it is within one address
                                           space, maybe more complicated */ 
const unsigned MAX_IMM21 = 0x1fff;

unsigned getMaxBranch() {
  return MAX_BRANCH;
}

const char *registerNames[] =
    { "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
      "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
      "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
      "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31" };

dictionary_hash<string, unsigned> funcFrequencyTable(string::hash);

/* not called at all, why is this here
string getStrOp(int op)
{
    switch (op) {
	case ADDop:	return("add");
	case ANDop:	return("and");
	case BLop:	return("bl");
	case COMBTop:	return("combt");
	case LDILop:	return("ldil");
	case LDOop:	return("ldo");
	case LDWop:	return("ldw");
	case ORop:	return("or");
	case STWop:	return("stw");
	case SUBop:	return("sub");

	default: 	return("???");
    }
}
*/

inline unsigned assemble_21(unsigned offset) {
    unsigned ret = ((offset&0x100000)>>20)|((offset&0xeef00)>>8)
                   |((offset&0x180)<<7)|((offset&0x7c)<<14)
                   |((offset&0x3)<<12);
    return ret;
}

/*
inline int w_w1_w2_to_offset(unsigned w, unsigned w1, unsigned w2) {
    assemble_w_w1_w2 x;
    x.raw = (w == 0) ? (0) : (~0); // sign extend the offset

    x.w_w1_w2.w = w;
    x.w_w1_w2.w1 = w1;
    x.w_w1_w2.w2 = ((w2&0x01)<<10) | ((w2&0x07fe)>>1); 
    // it means cat(w,w1,w2{10},w2{0..9}) 
    return ((x.raw << 2)+8);
}

inline int w_w1_to_offset(unsigned w, unsigned w1) {
    assemble_w_w1 x;
    x.raw = (w == 0) ? (0) : (~0);

    x.w_w1.w = w;
    x.w_w1.w1 = ((w1&0x01)<<10) | ((w1&0x07fe)>>1);
    return ((x.raw << 2)+8);
}
*/

inline assemble_w_w1_w2 offset_to_w_w1_w2(int offset) {
    if (ABS(offset) > getMaxBranch()) {
	logLine("a branch too far\n");
	showErrorCallback(52, "");
	abort();
    }

    offset -= 8;
    offset >>= 2; /* undo lshift */

    assemble_w_w1_w2 ret;
    ret.raw = offset;
    ret.w_w1_w2.w2 = ((offset&0x03ff)<<1) | ((offset&0x0400)>>10);
    return ret;
}

inline void generateBranchInsn(instruction *insn, int offset, reg lr = 0)
{
    assemble_w_w1_w2 x = offset_to_w_w1_w2(offset);

    insn->raw = 0;
    insn->bi.op = BLop;
    insn->bi.r2_p_b_t = lr;
    insn->bi.r1_im5_w1_x = x.w_w1_w2.w1;
    insn->bi.c_s_ext3 = BLext;
    insn->bi.w1_w2 = x.w_w1_w2.w2;
    insn->bi.n = 1;
    insn->bi.w = x.w_w1_w2.w;

    // logLine("ba,a %x\n", offset);
}

inline void generateBackBranchInsn(instruction *insn, bool toText=true)
{

    insn->raw = 0;
    insn->bi.op = BEop;
    insn->bi.r2_p_b_t = 31;     // GR 31
    insn->bi.r1_im5_w1_x = 0;
    if (toText) {
       insn->bi.c_s_ext3 = 1;      // space register 4
    } else {
       insn->bi.c_s_ext3 = 3;
    }
    insn->bi.w1_w2 = 0;
    insn->bi.n = 0;
    insn->bi.w = 0;

}

// Branch instruction (from application to base trampoline) 
inline void generateToBranchInsn1(instruction *insn, unsigned dest) {

    insn->raw = 0;
    insn->li.op = LDILop;
    insn->li.tr = 31;               // GR 31          
    insn->li.im21 = assemble_21((dest >> 11) & 0x1fffff);

}

inline void generateToBranchInsn2(instruction *insn, unsigned dest, 
                                  bool toData=true, bool isDelayed=false) {

    int offset = dest&0x7ff;
    offset += 8;
    assemble_w_w1_w2 x = offset_to_w_w1_w2(offset);
    insn->raw = 0;
    insn->bi.op = BLEop;
    insn->bi.r2_p_b_t = 31;        // GR 31
    insn->bi.r1_im5_w1_x = x.w_w1_w2.w1;
    if (toData) {
      insn->bi.c_s_ext3 = 3;         // space register 5!! assemble_3()
    } else {
      insn->bi.c_s_ext3 = 1;         // space register 4
    }
    insn->bi.w1_w2 = x.w_w1_w2.w2;
    if (isDelayed) {
	insn->bi.n = 0;              // actually means not nullified
    }
    else { 
	insn->bi.n = 1;              // nullifiled
    }
    insn->bi.w = x.w_w1_w2.w;

}  


inline void generateCompareBranch(instruction* insn, reg rs1, reg rs2,
  int offset) {
    offset -= 8; /* 8 always added to branch */
    offset >>= 2; /* undo the lshift of assembly */
    unsigned w = (offset & 0x800)>>11 ; /* pull out the w1,w bits */
    unsigned w1 = (offset << 0x1)|((offset & 0x400)>>10);

    insn->raw = 0;
    insn->bi.op = COMBTop;
    insn->bi.r2_p_b_t = rs2;
    insn->bi.r1_im5_w1_x = rs1;
    insn->bi.c_s_ext3 = COMCLR_EQ_C;
    insn->bi.w1_w2 = w1;
    insn->bi.n = 1;
    insn->bi.w = w;
}

inline void genArithLogInsn(instruction *insn, unsigned op, unsigned ext7,
    reg rs1, reg rs2, reg rd)
{
    insn->raw = 0;
    insn->ci.al.op = op;
    insn->ci.al.r2 = rs2;
    insn->ci.al.r1 = rs1;
    insn->ci.al.ext7 = ext7;
    insn->ci.al.t = rd;

    // logLine("%s %%%s,%%%s,%%%s\n", getStrOp(op), registerNames[rs1],
    // 	registerNames[rs2], registerNames[rd]);
}

inline void genArithImmn(instruction *insn, unsigned op, reg r1, reg r2, 
       int im11)
{
    insn->raw = 0;
    insn->ci.ai.op = op;
    insn->ci.ai.r = r1;
    insn->ci.ai.t = r2;
    insn->ci.ai.c = 0;
    insn->ci.ai.f = 0;
    insn->ci.ai.e = 0;
    insn->ci.ai.im11 = ((im11&0x7ff)<<1)|((im11&0x400)>>10);
}

inline void generateNOOP(instruction *insn)
{
    // or 0,0,0
    genArithLogInsn(insn, ORop, ORext7, 0, 0, 0);

    // logLine("nop\n");
}

inline void genCmpOp(instruction *insn, unsigned cond, unsigned flow,
    reg rs1, reg rs2, reg rd)
{
    insn->raw = 0;
    insn->ci.al.op = COMCLRop;
    insn->ci.al.r2 = rs2;
    insn->ci.al.r1 = rs1;
    insn->ci.al.c = cond;
    insn->ci.al.f = flow;
    insn->ci.al.ext7 = COMCLRext7;
    insn->ci.al.t = rd;
}

// only numbers larger than 0 are handled  
inline void generateLoadConst(instruction *insn, int src1, int dest,
  unsigned& base) {

      if (ABS(src1) > MAX_IMM21) {
	insn->raw = 0;
	insn->li.op = LDILop;
	insn->li.tr = dest;
	insn->li.im21 = assemble_21((src1 >> 11) & 0x1fffff);
         
	insn++;

	insn->raw = 0;
	insn->mr.ls.op = LDOop;
	insn->mr.ls.b = dest;
	insn->mr.ls.tr = dest;
	insn->mr.ls.s = 0;
	insn->mr.ls.im14 = (src1&0x7ff)<<1;
  
	base += 2*sizeof(instruction);
      }
      else {
	insn->raw = 0;
	insn->mr.ls.op = LDOop;
	insn->mr.ls.b = 0;
	insn->mr.ls.tr = dest;
	insn->mr.ls.s = 0;
	insn->mr.ls.im14 = ((src1&0x1fff)<<1)|((src1&0x0000)>>13);

	base += sizeof(instruction);
      }
}

inline void generateLoad(instruction *insn, reg src, reg dest, int im14 = 0)
{
    insn->raw = 0;
    insn->mr.ls.op = LDWop;
    insn->mr.ls.b = src;
    insn->mr.ls.tr = dest;
    insn->mr.ls.s = 0;
    insn->mr.ls.im14 = ((im14&0x3fff)<<1)|((im14&0x2000)>>13);
}

inline void generateStore(instruction *insn, int rd, int rs1, int im14 = 0)
{
    insn->raw = 0;
    insn->mr.ls.op = STWop;
    insn->mr.ls.b = rs1;
    insn->mr.ls.tr = rd;
    insn->mr.ls.s = 0;
    insn->mr.ls.im14 = ((im14&0x3fff)<<1)|((im14&0x2000)>>13); 
}


void generateToBranch(process *proc, instPoint *location, unsigned dest )
{
    instruction *insn = new instruction;

    // Two instructions are needed to do the long jump
    generateToBranchInsn1(insn, dest);
    proc->writeTextWord((caddr_t)(location->addr), insn->raw);
    insn++;
    generateToBranchInsn2(insn, dest);
    proc->writeTextWord((caddr_t)(location->addr+4), insn->raw);

    // One more if the second instruction is branch and there's
    // a delayed slot for this one
    if ((location->ipType == functionEntry)&&(location->isDelayed)) {
	generateNOOP(insn);
	proc->writeTextWord((caddr_t)(location->addr+8), insn->raw);
    }

//    proc->writeDataSpace((caddr_t)(fromAddr),2*sizeof(instruction), 
//                          (caddr_t)insn);
}


instPoint::instPoint(pdFunction *f, const instruction &instr,
		     const image *owner, Address adr, bool delayOK, 
		     bool &err, instPointType pointType )
: addr(adr), originalInstruction(instr), ipType(pointType), inDelaySlot(false), isDelayed(false),
  callIndirect(false), callAggregate(false), callee(NULL), func(f)
{

  err = false;

  switch (pointType) {

    case functionEntry:
         nextInstruction.raw = owner->get_instruction(adr+4);
	 if (IS_BRANCH_INSN(nextInstruction)||
	     IS_CONDITIONAL_BRANCH(nextInstruction)) {
	     if (isBranchInsnTest(nextInstruction, adr+4, adr, adr+f->size())) {
		 if (nextInstruction.bi.n == 0) {
		     isDelayed = true;
		     delaySlotInsn.raw = owner->get_instruction(adr+8);
		 }
	     } else {
		 err = true;
	     }
	 }
     
         break;

    case functionExit:
         originalInstruction.raw = owner->get_instruction(adr-4);
         if (IS_BRANCH_INSN(originalInstruction)) {
	     err = true;
         }

         nextInstruction.raw = owner->get_instruction(adr);
	 assert(nextInstruction.bi.op == BVop);
	 if (nextInstruction.bi.r1_im5_w1_x != 0) {
             logLine("Internal Error: branch return instruction's strange.\n");  
	     err = true;
	 }
         nextInstruction.bi.c_s_ext3 = 1;
	 nextInstruction.bi.op = BEop;

         if (nextInstruction.bi.n == 0) {
	     isDelayed = true;
	     delaySlotInsn.raw = owner->get_instruction(adr+4);
	 }

	 addr-=4;
         break;

    case callSite:
         nextInstruction.raw = owner->get_instruction(adr+4);
	 if (IS_BRANCH_INSN(nextInstruction)) {
	     err = true;
	 } 
	 break;

	 // it might be an idea to put all these functions 
	 // in a file to say these functions are not 
	 // instrumentable. (i think those internal procedure 
	 // would never be instrumented indeed. --ling

         //if (IS_BRANCH_INSN(nextInstruction)) {
	 //    logLine("Internal Error: two adjacent branchs.\n");
	 //    abort();
         //}

    case noneType:
         break;
    default:
         logLine("Wrong pointType of instPoint(impossible to reach here).\n");
  }  

  //if (owner->isValidAddress(adr-4)) {
  //  instruction iplus1;
  //  iplus1.raw = owner->get_instruction(adr-4);
  //  if (IS_DELAYED_INST(iplus1) && !delayOK) {
      // ostrstream os(errorLine, 1024, ios::out);
      // os << "** inst point " << func->file->fullName << "/"
      //  << func->prettyName() << " at addr " << addr <<
      //	" in a delay slot\n";
      // logLine(errorLine);
   //   inDelaySlot = true;
   // }
   //}
}


// Determine if the called function is a "library" function or a "user" function
// This cannot be done until all of the functions have been seen, verified, and
// classified
//
void pdFunction::checkCallPoints() {
  instPoint *p;
  Address loc_addr;

  vector<instPoint*> non_lib;

  for (unsigned i=0; i<calls.size(); ++i) {
    /* check to see where we are calling */

    p = calls[i];
    assert(p);

    if (!isCallInsn(p->originalInstruction)) {
	logLine("what is a non-call (jump) doing in set of calls\n");
	continue;
    }

    if (isInsnType(p->originalInstruction, CALLmask, CALLmatch)) {
      // Direct call
      int offset = w_w1_w2_to_offset(p->originalInstruction.bi.w,
      		p->originalInstruction.bi.r1_im5_w1_x,
      		p->originalInstruction.bi.w1_w2);
      loc_addr = p->addr + offset;
      pdFunction *pdf = (file_->exec())->findFunction(loc_addr);
      if (pdf && !pdf->isLibTag()) {
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
Address pdFunction::newCallPoint(const Address adr, const instruction instr,
				 const image *owner, bool &err)
{
    Address ret=adr;
    instPoint *point;

    err = false;

    // modified 1/16/95 3:45pm
    //   same reason as in the inst-sparc.C
    // bool err = true;

    point = new instPoint(this, instr, owner, adr, false, err, callSite);

    if (!isCallInsn(instr)) {
	logLine("what is a non-call (jump) doing in newCallPoint\n");
	assert(0);
    }

    if (!isInsnType(instr, CALLmask, CALLmatch)) {
      point->callIndirect = true;
      point->callee = NULL;
    } else
      point->callIndirect = false;

    point->callAggregate = false;

    if (err == false) {
	calls += point;
    }

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
        return;
    } else {
        printf("found freq.input file\n");
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

    pdFunction *func;

    if (point->callee)
        func = point->callee;
    else
        func = point->func;

    if (!funcFrequencyTable.defines(func->prettyName())) {
      if (func->isLibTag()) {
	return(100);
      } else {
        // Changing this value from 250 to 100 because predictedCost was
        // too high - naim 07/18/96
	return(100);
      }
    } else {
      return (funcFrequencyTable[func->prettyName()]);
    }
}

//
// return cost in cycles of executing at this point.  This is the cost
//   of the base tramp if it is the first at this point or 0 otherwise.
//
int getPointCost(process *proc, instPoint *point)
{
    if (proc->baseMap.defines(point)) {
        return(0);
    } else {
        // 76 cycles for base tramp
        if (point->ipType == callSite)
	    return(76+28);
	else return(76);
    }
}

/*
 * Given and instruction, relocate it to a new address, patching up
 *   any relative addressing that is present.
 *
 * There are 6 types of unconditional branches
 * and 8 types of conditional branches on the PA-RISC.
 *
 * Most of the unconditional branches are base/index relative
 * and cannot (need not?) be relocated.  However, most of the
 * conditional branches can be relocated since they are PC relative.
 *
 * The simple solution is that for the instrumentation points we
 * can handle (procedure entry, procedure exit, and procedure
 * call), only certain types of branches can occur.  If anything
 * else occurs in these places, the compiler is screwed.
 *
 */
void relocateInstruction(process *proc, instruction *&insn, int origAddr, 
			 int targetAddr, instPoint *location)
{
    int dest;


    /* To relocate instruction of the procedure call, we need
       use a assemble routine as a middleman. The reason is that
       when the precedure called finish, it need to branch return
       to the tramoplines (in the data segment!!) */

    if (location->ipType == callSite) {

	int oldOffset = w_w1_w2_to_offset(insn->bi.w,
					  insn->bi.r1_im5_w1_x,
					  insn->bi.w1_w2);
	dest = origAddr + oldOffset;
	generateStore(insn, 31,  30, -20);
	insn++; 
        generateLoadConst(insn, dest, 28, 0);
	insn += 2;
        pdFunction *midfunc = proc->findOneFunction("baseCall");
	if (!midfunc) {
		ostrstream os(errorLine, 1024, ios::out);
		os << "Internal error: unable to find addr of miniCall" << endl;
		logLine(errorLine);
		showErrorCallback(80, (const char *) errorLine);
		P_abort();
	    }
	dest = midfunc->addr();
        generateToBranchInsn1(insn,dest); insn++;
        generateToBranchInsn2(insn,dest, location->isDelayed);insn++;
        generateNOOP(insn);
    } else if (location->ipType == functionEntry) {

	*insn = location -> nextInstruction;

	/* If it is an unconditional branch instruction, modify
	   the value the register 31 to let the base tramopline
	   jump back to the right place */ 
	if (isCallInsn(*insn)) {

	    int offset = w_w1_w2_to_offset(insn->bi.w,
					      insn->bi.r1_im5_w1_x,
					      insn->bi.w1_w2);
	    if (location->isDelayed) {
		*insn = location->delaySlotInsn;
		insn++;
	    }

	    genArithImmn(insn, ADDIop, 31, 31, offset-4);

	/* If it is an unconditional branch instruction, 
	   deal with it */     
	} else if (IS_CONDITIONAL_BRANCH(*insn)) {
	    int offset = w_w1_to_offset(insn->bi.w, insn->bi.w1_w2);
	    if (location->isDelayed) {
		*insn = location->delaySlotInsn;
		insn++;
	    }

	    genArithImmn(insn, ADDIop, 31, 31, offset-4);
	    insn++;

	    *insn = location->nextInstruction;
	    insn->bi.w = 0;
	    insn->bi.w1_w2 = 2;
	    insn += 2;
	    genArithImmn(insn, ADDIop, 31, 31, 8-offset);
	}
    }

    /* The rest of the instructions should be fine as is */
}

trampTemplate baseTemplate;

extern "C" void baseTramp();

void initATramp(trampTemplate *thisTemp, instruction *tramp)
{
    instruction *temp;

    // TODO - are these offset always positive?
    thisTemp->trampTemp = (void *) tramp;
    for (temp = tramp; temp->raw != END_TRAMP; temp++) {
	switch (temp->raw) {
	    case LOCAL_PRE_BRANCH:
		thisTemp->localPreOffset = ((void*)temp - (void*)tramp);
		thisTemp->localPreReturnOffset = thisTemp->localPreOffset 
		                                 + sizeof(temp->raw);
		break;
	    case GLOBAL_PRE_BRANCH:
		thisTemp->globalPreOffset = ((void*)temp - (void*)tramp);
		break;
	    case LOCAL_POST_BRANCH:
		thisTemp->localPostOffset = ((void*)temp - (void*)tramp);
		thisTemp->localPostReturnOffset = thisTemp->localPostOffset
		                                  + sizeof(temp->raw);
		break;
	    case GLOBAL_POST_BRANCH:
		thisTemp->globalPostOffset = ((void*)temp - (void*)tramp);
		break;
	    case SKIP_PRE_INSN:
                thisTemp->skipPreInsOffset = ((void*)temp - (void*)tramp);
                break;
	    case SKIP_POST_INSN:
                thisTemp->skipPostInsOffset = ((void*)temp - (void*)tramp);
                break;
	    case RETURN_INSN:
                thisTemp->returnInsOffset = ((void*)temp - (void*)tramp);
                break;
	    case EMULATE_INSN:
                thisTemp->emulateInsOffset = ((void*)temp - (void*)tramp);
                break;
	    case UPDATE_COST_INSN:
		thisTemp->updateCostOffset = ((void*)temp - (void*)tramp);
                break;
  	}
    }
    thisTemp->cost = 30;  
    thisTemp->prevBaseCost = 13;
    thisTemp->postBaseCost = 13;
    thisTemp->prevInstru = thisTemp->postInstru = false;
    thisTemp->size = (int) temp - (int) tramp;
}

registerSpace *regSpace;

// return values come via r28, r29; these should be dead at call point
// Not really, actually.
int deadRegList[] = { 29, 3, 2, 23, 24, 25, 26 };


// r26, r25, r24, r23 are call arguments (in that order)
int liveRegList[] = { 1, 19, 20, 21, 22, 31, };
    // all are caller save registers

void initTramps()
{
    static bool inited=false;

    if (inited) return;
    inited = true;

    initATramp(&baseTemplate, (instruction *) baseTramp);

    regSpace = new registerSpace(sizeof(deadRegList)/sizeof(int), deadRegList,
	sizeof(liveRegList)/sizeof(int), liveRegList);
}

/*
 * Install a base tramp -- fill calls with nop's for now.
 *
 */
trampTemplate *installBaseTramp(instPoint *location, process *proc)
{
    unsigned currAddr;
    instruction *code;
    instruction *temp;

    unsigned baseAddr = inferiorMalloc(proc, baseTemplate.size, textHeap);
    code = new instruction[baseTemplate.size];
    memcpy((char *) code, (char*) baseTemplate.trampTemp, baseTemplate.size);
    // bcopy(baseTemplate.trampTemp, code, baseTemplate.size);

    for (temp = code, currAddr = baseAddr;
	(currAddr - baseAddr) < baseTemplate.size;
	temp++, currAddr += sizeof(instruction)) {
	if (temp->raw == EMULATE_INSN) {
	    *temp = location->originalInstruction;

	    if (location->ipType == callSite) {
		relocateInstruction(proc, temp, location->addr, currAddr,
	     			location);
		temp++;
		if (location->isDelayed) {
		    *temp = location->nextInstruction;
		} 
		temp++;
		generateLoad(temp, 30, 31, -20);
                currAddr += 2*sizeof(instruction);  
	    } else if (location->ipType == functionExit) {
		temp++;
		assert(temp->raw == EMULATE_INSN_1);
		*temp = location->nextInstruction;
		currAddr += sizeof(instruction);  
		location->ipType == noneType;     /* hacking , what's mean?*/
		relocateInstruction(proc, temp, location->addr+4, currAddr, 
				    location);
			/*	    location->isDelayed, noneType);*/
		if (location->isDelayed) {
		    *(temp+1) = location->delaySlotInsn;
		}
	    } else if (location->ipType == functionEntry) {
		temp++;
		assert(temp->raw == EMULATE_INSN_1);
		relocateInstruction(proc, temp, location->addr+4, currAddr, 
				    location);
                currAddr += sizeof(instruction);
	    }
            // TODO: What happens if ipType is not any of the previous choices?
            // We should probably do something about it - naim
	} else if (temp->raw == RETURN_INSN) {
            genArithImmn(temp, ADDIop, 31, 31, -0x4);
        } else if (temp->raw == RETURN_INSN_1) {
	    generateBackBranchInsn(temp);
	} else if (temp->raw == PREAMBLE_0||temp->raw == PREAMBLE_00){
	    genArithImmn(temp, ADDIop, 30, 30, 128);  
        } else if (temp->raw == PREAMBLE_1||temp->raw == PREAMBLE_11){
            generateStore(temp, 31, 30, -124); 
	} else if (temp->raw == PREAMBLE_2||temp->raw == PREAMBLE_22){
            generateStore(temp, 2,  30, -120);
	} else if (temp->raw == PREAMBLE_3) {
	    if (location->ipType == callSite)
		generateStore(temp, 3,  30, -132);
	    else
		generateStore(temp, 3,  30, -116);
	} else if(temp->raw == PREAMBLE_33){
            generateStore(temp, 3,  30, -116);
	} else if (temp->raw == PREAMBLE_4||temp->raw == PREAMBLE_44){
            generateStore(temp, 28,  30, -112);
        } else if (temp->raw == TRAILER_4||temp->raw == TRAILER_44){
	    generateLoad(temp, 30,  28, -112);
        } else if (temp->raw == TRAILER_33) {
	    if (location->ipType == callSite) 
		generateLoad(temp, 30,   3, -132);
	    else
		generateLoad(temp, 30,   3, -116);
	} else if (temp->raw == TRAILER_3){
	    generateLoad(temp, 30,   3, -116);
        } else if (temp->raw == TRAILER_2||temp->raw == TRAILER_22){
            generateLoad(temp, 30,   2, -120); 
        } else if (temp->raw == TRAILER_1||temp->raw == TRAILER_11){
	    generateLoad(temp, 30,  31, -124);
	} else if (temp->raw == TRAILER_0||temp->raw == TRAILER_00)  { 
	    genArithImmn(temp, ADDIop, 30, 30, -128);
        } else if (temp->raw == SAVE_PRE || temp->raw == SAVE_POST) {
          int ind;
	  for (ind=0; ind < 4; ind++) {
	    generateStore(temp+ind, 26-ind,  30, -128-36-4*ind);
	  }
          ind--;
          temp += ind;
          currAddr += ind*sizeof(instruction);
        } else if (temp->raw == RESTORE_PRE || temp->raw == RESTORE_POST) { 
          int ind;
          for (ind=0; ind < 4; ind++) {
	    generateLoad(temp+ind, 30, 23+ind, -128-48+4*ind);
	  }
          ind--;
          temp += ind;
          currAddr += ind*sizeof(instruction);
        } else if (temp->raw == SKIP_PRE_INSN) {
          unsigned offset;
          offset = baseAddr+baseTemplate.updateCostOffset-currAddr;
          generateBranchInsn(temp,offset);
        } else if (temp->raw == SKIP_POST_INSN) {
	    unsigned offset;
	    offset = baseAddr+baseTemplate.returnInsOffset-currAddr;
	    generateBranchInsn(temp,offset);
	    
	} else if (temp->raw == UPDATE_COST_INSN) {
	    baseTemplate.costAddr = currAddr;
	    generateNOOP(temp);
   
        } else if ((temp->raw == LOCAL_PRE_BRANCH) ||
		   (temp->raw == LOCAL_PRE_BRANCH_1) ||
		   (temp->raw == GLOBAL_PRE_BRANCH) ||
		   (temp->raw == LOCAL_POST_BRANCH) ||
		   (temp->raw == LOCAL_POST_BRANCH_1) ||
		   (temp->raw == GLOBAL_POST_BRANCH)) {
	    /* fill with no-op */
	    generateNOOP(temp);
	}

}
    // TODO cast
    proc->writeDataSpace((caddr_t)baseAddr, baseTemplate.size, (caddr_t) code);

    free(code);

    trampTemplate *baseInst = new trampTemplate;
    *baseInst = baseTemplate;
    baseInst->baseAddr = baseAddr;
    return baseInst;
}

void generateNoOp(process *proc, int addr)
{
    instruction insn;

    generateNOOP(&insn);
    proc->writeDataSpace((caddr_t)addr, sizeof(int), (char *)&insn);
}


trampTemplate *findAndInstallBaseTramp(process *proc,
				       instPoint *location,
				       returnInstance *&retInstance,
				       bool noCost)
{
    trampTemplate *ret;
    process *globalProc;

    globalProc = proc;
    if (!globalProc->baseMap.defines(location)) {
	ret = installBaseTramp(location, globalProc);
	//generateToBranch(globalProc, location, ret);
	instruction *insn = new instruction[2];
	generateToBranchInsn1(insn, ret->baseAddr);
	generateToBranchInsn2(insn+1, ret->baseAddr);
	if ((location->ipType == functionEntry)&&(location->isDelayed)) {
	    generateNOOP(insn+2);
	    retInstance = new returnInstance((instruction *)insn,
					     3*sizeof(instruction), location->addr,
					     3*sizeof(instruction));
	} else {
	    retInstance = new returnInstance((instruction *)insn,
					     2*sizeof(instruction), location->addr, 
					     2*sizeof(instruction));
	}
	globalProc->baseMap[location] = ret;
    } else {
        ret = globalProc->baseMap[location];
	retInstance = NULL;
    }

    return(ret);
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

    unsigned atAddr;
    if (inst->when == callPreInsn) {
	if (inst->baseInstance->prevInstru == false) {
	    atAddr = inst->baseInstance->baseAddr+baseTemplate.skipPreInsOffset;
	    inst->baseInstance->cost += inst->baseInstance->prevBaseCost;
	    inst->baseInstance->prevInstru = true;
	    generateNoOp(inst->proc, atAddr);
	}
    }
    else {
	if (inst->baseInstance->postInstru == false) {
	    atAddr = inst->baseInstance->baseAddr+baseTemplate.skipPostInsOffset; 
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
void generateBranch(process *proc, unsigned fromAddr, unsigned newAddr)
{
    int disp;
    instruction insn;

    disp = newAddr-fromAddr;
    generateBranchInsn(&insn, disp);

    // TODO cast
    proc->writeDataSpace((caddr_t)fromAddr,sizeof(int),(char *)&insn);
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
	if (point->callee && !(point->callee->isLibTag())) {
	    return(true);
	} else {
	    return(false);
	}
    }
}


/* 
 *function Kludge
 */
unsigned functionKludge(pdFunction *func, process *proc)
{
    unsigned address; 

    if ((func->entryPoint).raw&&(func->entryPoint).mr.ls.tr == 2) {
	(func->entryPoint).mr.ls.tr = 31;
    } else if ((func->entryPoint).raw&&(func->entryPoint).mr.ls.tr == 31) {
    } else {
    }
    address = func->addr();
    proc->writeTextWord((caddr_t)address ,(func->entryPoint).raw);

    int index = 0;
    instruction loadReturn;
    while (index < (func->lr).size()) {
        loadReturn = (func->lr[index])->originalInstruction; 
	if (loadReturn.raw && loadReturn.mr.ls.tr == 2) {
	    loadReturn.mr.ls.tr = 31;
	} else if (loadReturn.raw && loadReturn.mr.ls.tr == 31) {
	} else {
	    printf("Internal Error: wrong with function load return\n");
	    abort();
	} 
	address = (func->lr[index])->addr;
	proc->writeTextWord((caddr_t)address ,loadReturn.raw);
        index++;
    }

    if ((func->exitPoint).raw&&(func->exitPoint).bi.op == BVop ) {
	int isDelayed = (func->exitPoint).bi.n;
	generateBackBranchInsn(&(func->exitPoint), false);
	(func->exitPoint).bi.n = isDelayed;
    } else if ((func->exitPoint).raw&&(func->exitPoint).bi.op == BEop ) {
    } else {
	printf("Internal Error: wrong with function return\n");
	abort();
    } 
    // warning: the following code assumes one exit point!
//    address = ((func->funcReturn())->addr)+4;
    address = func->funcReturns[func->funcReturns.size()-1]->addr + 4;
    proc->writeTextWord((caddr_t)address ,(func->exitPoint).raw);

    Address ret = func -> addr(); 
    return ret;
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
pdFunction *getFunction(instPoint *point)
{
    return(point->callee ? point->callee : point->func);
}

unsigned emitImm(opCode op, reg src1, reg src2, reg dest, char *i, 
                 unsigned &base, bool noCost)
{        
    instruction *insn = (instruction *) ((void*)&i[base]);
    int result;
    
    switch (op) {
	// integer ops
      case plusOp:
	genArithImmn(insn, ADDIop, src1, dest, src2);
	break;
	
      case minusOp:
	genArithImmn(insn, SUBIop, src1, dest, src2);
	break;
	
      case timesOp:
	if (isPowerOf2(src2,result))
	    genArithLogInsn(insn, SHDop, src1, 0, 32-result, dest); 
	else 
	    abort(); // emulated via "floating point!" multiply
	break;
	
      case divOp:
	if (isPowerOf2(src2,result))
	    genArithLogInsn(insn, SHDop, 0, src1, result, dest);
	else 
	    abort(); // not implemented
	break;
	
      default:
	abort();
	break;
    }
    
    base += sizeof(instruction);
    return(0);
}

unsigned emitFuncCall(opCode op, 
		      registerSpace *rs,
		      char *i, unsigned &base, 
		      const vector<AstNode> &operands, 
		      const string &callee, process *proc,
		      bool noCost)
{
        register srcs;

        for (unsigned u = 0; u < operands.size(); u++) {
	    if (u >= 4) {
                 string msg = "Too many arguments to function call in instrumentation code:
 only 4 arguments can be passed on the sparc architecture.\n";
                 fprintf(stderr, msg.string_of());
                 showErrorCallback(94,msg);
                 cleanUpAndExit(-1);
            }  
	    srcs = operands[u].generateCode(proc, rs, i, base, noCost);
	    emit(storeMemOp, srcs, 30, -36-4*u, i, base, noCost);
	    rs->freeRegister(srcs);
        }

	for (unsigned u = 0; u < operands.size(); u++) {
	    assert(u < 4);
	    emit(loadMemOp, 30, 26-u,-36-4*u, i, base, noCost);
	}

	instruction *insn = (instruction *) ((void*)&i[base]);
        unsigned dest;
        bool err;
        pdFunction *func;
        dest = proc->findInternalAddress(callee, false, err);
        if (!err) {
          func = proc->findOneFunction(callee);
          dest=functionKludge(func, proc); 
	}  
        else {
	    func = proc->findOneFunction(callee);
            if (!func) {
             ostrstream os(errorLine, 1024, ios::out);
             os << "Internal error: unable to find addr of " << callee << endl;
             logLine(errorLine);
             showErrorCallback(80, (const char *) errorLine);
             P_abort();
            }
	    dest = func->addr();
        
	    generateLoadConst(insn, dest, 28, base);
	    insn = (instruction *) ((void*)&i[base]);
	    pdFunction *midfunc = proc->findOneFunction("miniCall");
	    if (!midfunc) {
		ostrstream os(errorLine, 1024, ios::out);
		os << "Internal error: unable to find addr of miniCall" << endl;
		logLine(errorLine);
		showErrorCallback(80, (const char *) errorLine);
		P_abort();
	    }
	    dest = midfunc->addr();
	}

        generateToBranchInsn1(insn, dest); insn++;
        generateToBranchInsn2(insn, dest, false); insn++;
        generateNOOP(insn);insn++;
        base += 3*sizeof(instruction);   

        // return value is the register with the return value from the
        //   function.
        // This needs to be %o0 since it is back in the callers scope.
        return(28);
}
 
bool process::emitInferiorRPCheader(void *, unsigned &) {
   // Is anything needed here?
   return true;
}

void generateBreakPoint(instruction &insn) {
    insn.raw = BREAK_POINT_INSN;
}

void genIllegalInsn(instruction &insn) {
   // according to page D-2 (opcodes) of the hppa 1.1 manual,
   // an opcode with the value 7 in bits 2 thru 5 is illegal.
   // (where bit 0 is the MSB).
   // So the following binary value should be illegal:
   // xx0111xxx... (where x represents anything)
   // If we set x to 0 everywhere, we get 0x1c000000

   insn.raw = 0x1c000000;
}

bool process::emitInferiorRPCtrailer(void *insnPtr, unsigned &baseBytes,
				     unsigned &firstPossibBreakOffset,
				     unsigned &lastPossibBreakOffset) {
   // Sequence: trap, 2 special instructions to restore PCSQ, illegal
   // (the 2 special instructions should never return)

//   generateBreakPoint(insn[baseInstruc]);
//   firstPossibBreakOffset = lastPossibBreakOffset = baseInstruc * sizeof(instruction);
//   baseInstruc++;

   // Call DYNINSTbreakPoint, with no arguments
   vector<AstNode> args; // no arguments to DYNINSTbreakPoint
   AstNode ast("DYNINSTbreakPoint", args);

   initTramps();
   extern registerSpace *regSpace;
   regSpace->resetSpace();

   reg resultReg = ast.generateCode(this, regSpace, (char*)insnPtr, baseBytes, false);
   regSpace->freeRegister(resultReg);

   instruction *insn = (instruction *)insnPtr;
   unsigned baseInstruc = baseBytes / sizeof(instruction);

   // Put 2 special instructions here:
   // the special instructions are:
   // mtsp r21, sr0  (move value from r21 to space register 0)
   // ble,n 0(sr0, r22) (branch and link external)
   //                   (puts offset of return point [4 bytes beyond following instr]
   //                    into reg 31, and space of following str addr into SR 0)

   // NOT YET IMPLEMENTED!!!

   // And just to make sure that the special instructions don't fall through:
   genIllegalInsn(insn[baseInstruc++]);
   genIllegalInsn(insn[baseInstruc++]);

   baseBytes = baseInstruc * sizeof(instruction); // convert back

   return true;
}

unsigned emit(opCode op, reg src1, reg src2, reg dest, char *i, unsigned &base,
	      bool noCost)
{
    // TODO cast
    instruction *insn = (instruction *) ((void*)&i[base]);

    if (op == loadConstOp) {
      generateLoadConst(insn, src1, dest, base);
    } else if (op ==  loadOp) {
	generateLoadConst(insn, src1, dest, base);
        insn = (instruction *) ((void*)&i[base]);

	generateLoad(insn, dest, dest);
	base += sizeof(instruction);
    } else if (op ==  storeOp) {
	generateLoadConst(insn, dest, src2, base);
        insn = (instruction *) ((void*)&i[base]);
	generateStore(insn, src1, src2);
	base += sizeof(instruction);
    } else if (op ==  storeMemOp) {
	generateStore(insn, src1, src2, dest);
	base += sizeof(instruction);
    } else if (op ==  loadMemOp) {
	generateLoad(insn, src1, src2, dest);
	base += sizeof(instruction);
    } else if (op ==  ifOp) {
	generateCompareBranch(insn, src1, 0, dest); insn++;
	generateNOOP(insn);
	base += sizeof(instruction)*2;
	return(base - 2*sizeof(instruction)); 
    } else if (op ==  trampPreamble) {
        // store arguments (which were passed in registers) onto the stack.
/*
        // this is being done in the base tramp now - naim
	for (int ind=0; ind < 4; ind++) {
	    generateStore(insn, 26-ind,  30, -128-36-4*ind);
	    base += sizeof(instruction);
	    insn = (instruction *) ((void*)&i[base]);
	}
*/

	// Calculate observed cost:
	if (!noCost) {
	   generateLoadConst(insn, dest, 29, base);
	   insn = (instruction *) ((void*)&i[base]);
	   if (ABS(dest) <= MAX_IMM21) {
	      generateNOOP(insn); 
	      base += sizeof(instruction);	
	      insn = (instruction *) ((void*)&i[base]);
	   }
	       
	   generateLoad(insn, 29, 29);
	   base += sizeof(instruction);
	   insn = (instruction *) ((void*)&i[base]);
	
	   genArithImmn(insn, ADDIop, 29, 28, src1);
	   base += sizeof(instruction);	
	   insn = (instruction *) ((void*)&i[base]);

	   generateLoadConst(insn, dest, 29, base);
	   insn = (instruction *) ((void*)&i[base]);
	   if (ABS(dest) <= MAX_IMM21) {
	      generateNOOP(insn); 
	      base += sizeof(instruction);	
	      insn = (instruction *) ((void*)&i[base]);
	   }

	   generateStore(insn, 28, 29);
	   base += sizeof(instruction);
        }	
    } else if (op ==  trampTrailer) {
/*
        // this is being done in the base tramp now - naim
	// restore any saved registers, but we never save them
	// dest is in words of offset and generateBranchInsn is bytes offset
	for (int ind=0; ind < 4; ind++) {
	    generateLoad(insn, 30, 23+ind, -128-48+4*ind);
	    base += sizeof(instruction);
	    insn = (instruction *) ((void*)&i[base]);
	}
*/
	generateBranchInsn(insn, dest << 2);
	base += sizeof(instruction);
	insn++;

        generateNOOP(insn);
        insn++;
        base += sizeof(instruction);

	return(base -  2 * sizeof(instruction));

    } else if (op == noOp) {
	generateNOOP(insn);
	base += sizeof(instruction);

    } else if (op == getRetValOp) {
	generateLoad(insn, 30,  28, -112);
	insn++;
	base += sizeof(instruction);
        return 28;
    } else if (op == getParamOp) {
	// first 4 params are in reg r23, r24, r25, r26
	generateLoad(insn,  30,  26-src1, -128-36-src1*4);
	insn++;
	base += sizeof(instruction);
	if (src1 <= 4) {
 	    return (26-src1);
	}
	abort();
    } else if (op == saveRegOp) {
	// should never be called for this platform.
	abort();
    } else {
      unsigned aop = 0;
      unsigned ext7 = 0;
	switch (op) {
	    // integer ops
	    case plusOp:
		aop = ADDop;
		ext7 = ADDext7;
		break;

	    case minusOp:
		aop = SUBop;
		ext7 = SUBext7;
		break;

	    // Bool ops
	    case orOp:
		aop = ORop;
		ext7 = ORext7;
		break;

	    case andOp:
		aop = ANDop;
		ext7 = ANDext7;
		break;

	    // relops
	    case eqOp:
		genCmpOp(insn, COMCLR_EQ_C, COMCLR_EQ_F, src1, src2, dest);
		base += sizeof(instruction);
		return(0);
		break;

            case neOp:
                genCmpOp(insn, COMCLR_NE_C, COMCLR_NE_F, src1, src2, dest);
		base += sizeof(instruction);
                return(0);
                break;

	    case lessOp:
		genCmpOp(insn, COMCLR_LT_C, COMCLR_LT_F, src1, src2, dest);
		base += sizeof(instruction);
		return(0);
		break;

	    case greaterOp:
		genCmpOp(insn, COMCLR_LE_C, COMCLR_LE_T, src1, src2, dest);
		base += sizeof(instruction);
		return(0);
		break;

	    case leOp:
		genCmpOp(insn, COMCLR_LE_C, COMCLR_LE_F, src1, src2, dest);
		base += sizeof(instruction);
		return(0);
		break;

	    case geOp:
		genCmpOp(insn, COMCLR_LT_C, COMCLR_LT_T, src1, src2, dest);
		base += sizeof(instruction);
		return(0);
		break;

	    case timesOp:
		// emulated via "floating point!" multiply
	    case divOp:
		// emulated via millicode
		abort();
		break;

	    default:
		abort();
		break;
	}
	genArithLogInsn(insn, aop, ext7, src1, src2, dest);
	base += sizeof(instruction);
      }
    return(0);
}

int getInsnCost(opCode op)
{
    if (op == loadConstOp) {
	return(1);
    } else if (op ==  loadOp) {
	return(2+1);
    } else if (op ==  storeOp) {
	return(2+1);
    } else if (op ==  ifOp) {
	return(1+1);
    } else if (op ==  callOp) {
	return(2+2+3);
    } else if (op ==  trampPreamble) {
	return(0);
    } else if (op ==  trampTrailer) {
	return(1+1+4);
    } else if (op == noOp) {
	return(1);
    } else if (op == getParamOp) {
	return(0);
    } else if (op == saveRegOp) {
	return(0);
    } else {
	switch (op) {
	    case plusOp:
	    case minusOp:
	    case orOp:
	    case andOp:
	    case eqOp:
            case neOp:
	    case lessOp:
	    case greaterOp:
	    case leOp:
	    case geOp:
	    case timesOp:
		// emulated via "floating point!" multiply
	    case divOp:
		// emulated via millicode
	    default:
	        return(1);
		break;
	}
      }
    return(0);
}

bool isReturnInsn(const image *owner, Address adr, bool &lastOne)
{
    instruction instr;
    
    instr.raw = owner->get_instruction(adr);

    // all returns are of the form
    //   bv,n 0(%r2)
    //   bv   0(%r2)
    // inter-space jumps/returns are ignored
    if (isInsnType(instr, RETmask, RETmatch)) {
	lastOne = true;
	return true;
    }
    return false;
}



bool pdFunction::findInstPoints(const image *owner) {

  Address adr = addr();
  instruction instr;
  bool err;

  instr.raw = owner->get_instruction(adr);
  if (!IS_VALID_INSN(instr)) {
    return false;
  }

  bool done;

  /*szOfLr = 0;*/

  entryPoint.raw = exitPoint.raw = 0;

  Address entryAdr = adr; 
  Address retAdr = adr + size();

  funcEntry_ = new instPoint(this, instr, owner, adr, true, err, functionEntry);
  //if (err)
  //  return true;
  assert(funcEntry_);
  entryPoint = instr;

  if (size() <= 12) return false;

  int index = 0; 

  for (; adr < retAdr; adr += 4) {

     instr.raw = owner->get_instruction(adr); 

     bool done;
     
     if (isReturnInsn(owner, adr, done)) {
	 funcReturns += new instPoint(this, instr, owner, adr, false, 
				      err ,functionExit);
	 exitPoint = instr;
	 return;
     } else if (isLoadReturn(instr)) {
	 instPoint *ip = new instPoint(this, instr, owner, adr, true, err, noneType);
	 lr += ip;

     } else if (isCallInsnTest(instr, adr, entryAdr, retAdr)) {
	 adr = newCallPoint(adr, instr, owner, err);
	 //if (err)
	 //	 return false;
     }
 }
 
  return true;
 
}


// The exact semantics of the heap are processor specific.
//
// find all DYNINST symbols that are data symbols
//
bool image::heapIsOk(const vector<sym_data> &find_us) {
  Address curr, instHeapEnd;
  Symbol sym;
  string str;

  for (unsigned i=0; i<find_us.size(); i++) {
    str = find_us[i].name;
    if (!linkedFile.get_symbol(str, sym)) {
      string str1 = string("_") + str.string_of();
      if (!linkedFile.get_symbol(str1, sym) && find_us[i].must_find) {
	string msg;
        msg = string("Cannot find ") + str + string(". Exiting");
	statusLine(msg.string_of());
	showErrorCallback(50, msg);
	return false;
      }
    }
    addInternalSymbol(str, sym.addr());
  }

  string ghb = GLOBAL_HEAP_BASE;
  if (!linkedFile.get_symbol(ghb, sym)) {
    ghb = U_GLOBAL_HEAP_BASE;
    if (!linkedFile.get_symbol(ghb, sym)) {
      string msg;
      msg = string("Cannot find ") + str + string(". Exiting");
      statusLine(msg.string_of());
      showErrorCallback(50, msg);
      return false;
    }
  }
  instHeapEnd = sym.addr();
  addInternalSymbol(ghb, instHeapEnd);
  ghb = INFERIOR_HEAP_BASE;

  if (!linkedFile.get_symbol(ghb, sym)) {
    ghb = UINFERIOR_HEAP_BASE;
    if (!linkedFile.get_symbol(ghb, sym)) {
      string msg;
      msg = string("Cannot find ") + str + string(". Cannot use this application");
      statusLine(msg.string_of());
      showErrorCallback(50, msg);
      return false;
    }
  }
  curr = sym.addr();
  addInternalSymbol(ghb, curr);
  if (curr > instHeapEnd) instHeapEnd = curr;
  mprotect(instHeapEnd, SYN_INST_BUF_SIZE, PROT_URWX);

  // check that we can get to our heap.
  if (instHeapEnd > getMaxBranch()) {
    logLine("*** FATAL ERROR: Program text + data too big for dyninst\n");
    sprintf(errorLine, "    heap ends at %x\n", instHeapEnd);
    logLine(errorLine);
    return false;
  } else if (instHeapEnd + SYN_INST_BUF_SIZE > getMaxBranch()) {
    logLine("WARNING: Program text + data could be too big for dyninst\n");
    showErrorCallback(54, "");
    return false;
  }
  return true;
}

//
// This is specific to some processors that have info in registers that we
//   can read, but should not write.
//   HPPA has no such registers.
//
bool registerSpace::readOnlyRegister(reg reg_number) {
  return false;
}


bool returnInstance::checkReturnInstance(const Address adr) {
    if ((adr > addr_) && ( adr <= addr_+size_))
	return false;
    else 
	return true;
}
 
void returnInstance::installReturnInstance(process *proc) {
    proc->writeTextSpace((caddr_t)addr_, instSeqSize, (caddr_t) instructionSeq); 
}

void returnInstance::addToReturnWaitingList(Address pc, process * proc) {
    instruction insn;
    instruction insnTrap;
    generateBreakPoint(insnTrap);
    proc->readDataSpace((caddr_t)pc, sizeof(insn), (char *)&insn, true);
    proc->writeTextSpace((caddr_t)pc, sizeof(insnTrap), (caddr_t)&insnTrap);

    instWaitingList *instW = new instWaitingList; 
    
    instW->instructionSeq = instructionSeq;
    instW->instSeqSize = instSeqSize;
    instW->addr_ = addr_;

    instW->relocatedInstruction = insn;
    instW->relocatedInsnAddr = pc;

    instWList.add(instW, (void *)pc);
}


bool doNotOverflow(int value)
{
  //
  // this should be changed by the correct code. If there isn't any case to
  // be checked here, then the function should return TRUE. If there isn't
  // any immediate code to be generated, then it should return FALSE - naim
  //
  //  if ( (value <= 2047) && (value >= -2048) ) return (true); 
  return(false);
}

void instWaitingList::cleanUp(process *proc, Address pc) {
    proc->writeTextSpace((caddr_t)pc, sizeof(relocatedInstruction),
		    (caddr_t)&relocatedInstruction);
    proc->writeTextSpace((caddr_t)addr_, instSeqSize, (caddr_t)instructionSeq);
}
