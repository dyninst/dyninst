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
 *
 * inst-power.C,v
 * Revision 1.16  1996/05/12  05:16:45  tamches
 * (really Jeff)
 * Now works with aix 4.1
 *
 * Revision 1.15  1996/05/10 05:12:34  tamches
 * changed vrble addr to dest; can now compile ok
 *
 * Revision 1.14  1996/04/29 22:18:44  mjrg
 * Added size to functions (get size from symbol table)
 * Use size to define function boundary
 * Find multiple return points for sparc
 * Instrument branches and jumps out of a function as return points (sparc)
 * Recognize tail-call optimizations and instrument them as return points (sparc)
 * Move instPoint to machine dependent files
 *
 * Revision 1.13  1996/04/26 20:53:36  lzheng
 * Changes to the procedure emitFuncCall. (move all the code dealing with
 * the function Calls in the miniTrampoline to here)
 *
 * Revision 1.12  1996/03/25 22:58:02  hollings
 * Support functions that have multiple exit points.
 *
 * Revision 1.11  1996/03/20  20:40:31  hollings
 * Fixed bug in register save/restore for function calls and conditionals
 *
 */

#include "util/h/headers.h"

#include "rtinst/h/rtinst.h"
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
#include "paradynd/src/showerror.h"
#include "util/h/debugOstream.h"

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

unsigned getMaxBranch() {
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

inline void genImmInsn(instruction *insn, int op, reg rt, reg ra, int immd)
{
  // something should be here to make sure immd is within bounds
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

inline void genSimpleInsn(instruction *insn, int op, reg src1, reg src2, 
                          reg dest, unsigned &base)
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

inline void genRelOp(instruction *insn, int cond, int mode, reg rs1,
		     reg rs2, reg rd, unsigned &base)
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
void relocateInstruction(instruction *insn, int origAddr, int targetAddr)
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
		                                 + 4 * sizeof(temp->raw);
		break;
	    case GLOBAL_PRE_BRANCH:
		thisTemp->globalPreOffset = ((void*)temp - (void*)tramp);
		break;
	    case LOCAL_POST_BRANCH:
		thisTemp->localPostOffset = ((void*)temp - (void*)tramp);
		thisTemp->localPostReturnOffset = thisTemp->localPostOffset
		                                  + 4 * sizeof(temp->raw);
		break;
	    case GLOBAL_POST_BRANCH:
		thisTemp->globalPostOffset = ((void*)temp - (void*)tramp);
		break;
	    case SKIP_PRE_INSN:
                thisTemp->skipPreInsOffset = ((void*)temp - (void*)tramp);
                break;
	    case UPDATE_COST_INSN:
		thisTemp->updateCostOffset = ((void*)temp - (void*)tramp);
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
  	}	
    }
    thisTemp->cost = 8;
    thisTemp->prevBaseCost = 20;
    thisTemp->postBaseCost = 30;
    thisTemp->prevInstru = thisTemp->postInstru = false;
    thisTemp->size = (int) temp - (int) tramp;
}

registerSpace *regSpace;

// 11-12 are defined not to have live values at procedure call points.
// reg 3-10 are used to pass arguments to functions.
//   We must save them before we can use them.
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
int deadRegList[] = { 11 };
#else
int deadRegList[] = { 11, 12 };
#endif

// allocate in reverse order since we use them to build arguments.
int liveRegList[] = { 10, 9, 8, 7, 6, 5, 4, 3 };

void initTramps()
{
    static bool inited=false;

    if (inited) return;
    inited = true;

    initATramp(&baseTemplate, (instruction *) baseTramp);

    regSpace = new registerSpace(sizeof(deadRegList)/sizeof(int), deadRegList, 
				 sizeof(liveRegList)/sizeof(int), liveRegList);
}


           ////////////////////////////////////////////////////////////////////
	   //Generates instructions to save link register onto stack.
	   //  Returns the number of bytes needed to store the generated
	   //    instructions.
	   //  The instruction storage pointer is advanced the number of 
	   //    instructions generated.
	   //
static int saveLR(instruction *&insn,       //Instruction storage pointer
		  reg           scratchReg, //Scratch register
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
		     reg           scratchReg, //Scratch register
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
		  reg           scratchReg,  //Scratch register
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
	      unsigned  loc, //Address in process to write into
	      unsigned  val) //Value to set link register
{
  instruction  i[8];                          //8 just to be safe
  instruction *t        = i;                  //To avoid side-effects on i
  int          numBytes = val ? setBRL(t, 10, val, BRLraw)
                              : setBRL(t, 10, val, NOOPraw);

  p->writeTextSpace((void *)loc, numBytes, i);
}

     //////////////////////////////////////////////////////////////////////////
     //Writes out a `br' instruction
     //
void resetBR(process  *p,    //Process to write instruction into
	     unsigned  loc)  //Address in process to write into
{
  instruction i;

  i.raw = BRraw;

  p->writeDataSpace((void *)loc, sizeof(instruction), &i);
}

static void saveRegister(instruction *&insn, unsigned &base, int reg,
		  int offset)
{
  assert(reg >= 0);
  genImmInsn(insn, STop, reg, 1, -1*((reg+1)*4 + offset));
  insn++;
  base += sizeof(instruction);
}

static void restoreRegister(instruction *&insn, unsigned &base, int reg, int dest,
		     int offset)
{
  assert(reg >= 0);
  genImmInsn(insn, Lop, dest, 1, -1*((reg+1)*4 + offset));
  insn++;
  base += sizeof(instruction);
}

static void restoreRegister(instruction *&insn, unsigned &base, int reg,
		     int offset)
{
  restoreRegister(insn, base, reg, reg, offset);
}	

static void saveFPRegister(instruction *&insn, unsigned &base, int reg,
		    int offset)
{
  assert(reg >= 0);
  genImmInsn(insn, STFDop, reg, 1, -1*((reg+1)*8 + offset));
  insn++;
  base += sizeof(instruction);
}

static void restoreFPRegister(instruction *&insn, unsigned &base, int reg, int dest,
		       int offset)
{
  assert(reg >= 0);
  genImmInsn(insn, LFDop, dest, 1, -1*((reg+1)*8 + offset));
  insn++;
  base += sizeof(instruction);
}

static void restoreFPRegister(instruction *&insn, unsigned &base, int reg,
		       int offset)
{
  restoreFPRegister(insn, base, reg, reg, offset);
}	

void saveAllRegistersThatNeedsSaving(instruction *insn, unsigned &base)
{
   unsigned numInsn=0;
   for (int i = 0; i < regSpace->getRegisterCount(); i++) {
     registerSlot *reg = regSpace->getRegSlot(i);
     if (reg->startsLive) {
       saveRegister(insn,numInsn,reg->number,8);
     }
   }
   base += numInsn/sizeof(instruction);
}

void restoreAllRegistersThatNeededSaving(instruction *insn, unsigned &base)
{
   unsigned numInsn=0;
   for (int i = 0; i < regSpace->getRegisterCount(); i++) {
     registerSlot *reg = regSpace->getRegSlot(i);
     if (reg->startsLive) {
       restoreRegister(insn,numInsn,reg->number,8);
     }
   }
   base += numInsn/sizeof(instruction);
}

void generateMTpreamble(char *insn, unsigned &base, process *proc)
{
  AstNode *t1,*t2,*t3,*t4,*t5;
  vector<AstNode *> dummy;
  unsigned tableAddr;
  int value; 
  bool err;
  reg src = -1;

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
  src = t3->generateCode(proc, regSpace, insn, base, false);
  removeAst(t3);
  instruction *tmp_insn = (instruction *) ((void*)&insn[base]);
  genImmInsn(tmp_insn, ORILop, src, REG_MT, 0);  
  base += sizeof(instruction);
  regSpace->freeRegister(src);
}

/*
 * Install a base tramp -- fill calls with nop's for now.
 *
 */
trampTemplate *installBaseTramp(instPoint *location, process *proc,
                                unsigned exitTrampAddr = 0) 
{
    unsigned currAddr;
    instruction *code;
    instruction *temp;

    unsigned baseAddr = inferiorMalloc(proc, baseTemplate.size, textHeap);
    code = new instruction[baseTemplate.size / sizeof(instruction)];
    memcpy((char *) code, (char*) baseTemplate.trampTemp, baseTemplate.size);
    // bcopy(baseTemplate.trampTemp, code, baseTemplate.size);

    for (temp = code, currAddr = baseAddr; 
	(int) (currAddr - baseAddr) < baseTemplate.size;
	temp++, currAddr += sizeof(instruction)) {
        if(temp->raw == UPDATE_LR) {
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
	} else if (temp->raw == EMULATE_INSN) {
	  if(location->ipLoc == ipFuncReturn) {
	    generateNOOP(temp);
	  } else {
	    *temp = location->originalInstruction;
	    relocateInstruction(temp, location->addr, currAddr);
	  }
	} else if (temp->raw == RETURN_INSN) {
	  if(location->ipLoc == ipFuncReturn) {
	    temp->raw = BRraw;
	  } else {
	    generateBranchInsn(temp, (location->addr + 
				      sizeof(instruction) - currAddr));
	  }
        } else if (temp->raw == SKIP_PRE_INSN) {
          unsigned offset;
          //offset = baseAddr+baseTemplate.updateCostOffset-currAddr;
          offset = baseAddr+baseTemplate.emulateInsOffset-currAddr;
          generateBranchInsn(temp,offset);

        } else if (temp->raw == SKIP_POST_INSN) {
	    unsigned offset;
	    offset = baseAddr+baseTemplate.returnInsOffset-currAddr;
	    generateBranchInsn(temp,offset);

	} else if (temp->raw == UPDATE_COST_INSN) {
	    baseTemplate.costAddr = currAddr;
	    generateNOOP(temp);

	} else if ((temp->raw == SAVE_PRE_INSN) || 
                   (temp->raw == SAVE_POST_INSN)) {
            unsigned numInsn=0;
            for(int i = 0; i < regSpace->getRegisterCount(); i++) {
	      registerSlot *reg = regSpace->getRegSlot(i);
              if (reg->startsLive) {
                numInsn = 0;
                saveRegister(temp,numInsn,reg->number,8);
		assert(numInsn > 0);
                currAddr += numInsn;
	      }
	    }

	    currAddr += saveLR(temp, 10, -56);  //Save link register on stack

	    // Also save the floating point registers which could
	    // be modified, f0-r13
	    for(int i=0; i <= 13; i++) {
	      numInsn = 0;
	      saveFPRegister(temp,numInsn,i,8+(32*4));
	      currAddr += numInsn;
	    }
	    
            // if there is more than one instruction, we need this
            if (numInsn>0) {
              temp--;
              currAddr -= sizeof(instruction);
	    }
	} else if ((temp->raw == RESTORE_PRE_INSN) || 
                   (temp->raw == RESTORE_POST_INSN)) {

            currAddr += restoreLR(temp, 10, -56); //Restore link register from
						  //  stack
            unsigned numInsn=0;
            for (int i = 0; i < regSpace->getRegisterCount(); i++) {
	      registerSlot *reg = regSpace->getRegSlot(i);
              if (reg->startsLive) {
                numInsn = 0;
                restoreRegister(temp,numInsn,reg->number,8);
		assert(numInsn > 0);
                currAddr += numInsn;
	      }
	    }
	    // Also load the floating point registers which were save
	    // since they could have been modified, f0-r13
	    for(int i=0; i <= 13; i++) {
	      numInsn = 0;
	      restoreFPRegister(temp,numInsn,i,8+(32*4));
	      currAddr += numInsn;
	    }

            // if there is more than one instruction, we need this 
            if (numInsn>0) {
              temp--;
              currAddr -= sizeof(instruction);
	    }
	} else if ((temp->raw == LOCAL_PRE_BRANCH) ||
		   (temp->raw == GLOBAL_PRE_BRANCH) ||
		   (temp->raw == LOCAL_POST_BRANCH) ||
		   (temp->raw == GLOBAL_POST_BRANCH)) {
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

    /* fill with no-op */
    /* ori 0,0,0 */
    insn.raw = 0;
    insn.dform.op = ORILop;

    proc->writeTextWord((caddr_t)addr, insn.raw);
}


trampTemplate *findAndInstallBaseTramp(process *proc, 
				       instPoint *&location,
				       returnInstance *&retInstance,
				       bool) // last bool is noCost
{
    trampTemplate *ret;
    process *globalProc;
    retInstance = NULL;

    globalProc = proc;
    if (!globalProc->baseMap.defines(location)) {      
        if((location->ipLoc == ipFuncEntry) ||
	   (location->ipLoc == ipFuncReturn)) {
	  instPoint*     newLoc;
	  trampTemplate* exTramp;
	  instruction    code[5];
	  
	  newLoc = location->func->funcExits(globalProc)[0];
	  exTramp = installBaseTramp(newLoc, globalProc);
	  globalProc->baseMap[newLoc] = exTramp;

	  newLoc = location->func->funcEntry(globalProc);
	  assert(newLoc->ipLoc == ipFuncEntry);
          assert(exTramp->baseAddr != 0);
	  ret = installBaseTramp(newLoc, globalProc, exTramp->baseAddr);

	  instruction *insn = new instruction;
	  generateBranchInsn(insn, ret->baseAddr - newLoc->addr);
	  globalProc->baseMap[newLoc] = ret;
	  retInstance = new returnInstance((instruction *)insn, 
					   sizeof(instruction), newLoc->addr,
					   sizeof(instruction));
	  
	  if(location->ipLoc == ipFuncReturn) {
	    ret = exTramp;
	  }
        } else {
	  ret = installBaseTramp(location, globalProc);
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
	if (point->callee && !(point->callee->isLibTag())) {
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


unsigned emitImm(opCode op, reg src1, reg src2, reg dest, char *i, 
                 unsigned &base, bool noCost)
{
        instruction *insn = (instruction *) ((void*)&i[base]);
        int iop=-1;
        int result;
	switch (op) {
	    // integer ops
	    case plusOp:
		iop = CALop;
		genImmInsn(insn, iop, dest, src1, src2);
		base += sizeof(instruction);
		return(0);
		break;

	    case minusOp:
		iop = SIop;
		genImmInsn(insn, iop, dest, src1, src2);
		base += sizeof(instruction);
		return(0);
		break;

	    case timesOp:
               if (isPowerOf2(src2,result) && (result<32)) {
                  generateLShift(insn, src1, result, dest);           
                  base += sizeof(instruction);
                  return(0);
	        }
                else {
                  reg dest2 = regSpace->allocateRegister(i, base, noCost);
                  (void) emit(loadConstOp, src2, dest2, dest2, i, base, noCost);
                  (void) emit(op, src1, dest2, dest, i, base, noCost);
                  regSpace->freeRegister(dest2);
                  return(0);
		}
		break;

	    case divOp:
                if (isPowerOf2(src2,result) && (result<32)) {
                  generateRShift(insn, src1, result, dest);           
                  base += sizeof(instruction);
                  return(0);
	        }
		else {
                  reg dest2 = regSpace->allocateRegister(i, base, noCost);
                  (void) emit(loadConstOp, src2, dest2, dest2, i, base, noCost);
                  (void) emit(op, src1, dest2, dest, i, base, noCost);
                  regSpace->freeRegister(dest2);
                  return(0);
		}
		break;

	    // Bool ops
	    case orOp:
		iop = ORILop;
		// For some reason, the destField is 2nd for ORILop and ANDILop
		genImmInsn(insn, iop, src1, dest, src2);
		base += sizeof(instruction);
		return(0);
		break;

	    case andOp:
		iop = ANDILop;
		// For some reason, the destField is 2nd for ORILop and ANDILop
		genImmInsn(insn, iop, src1, dest, src2);
		base += sizeof(instruction);
		return(0);
		break;

	    default:
                reg dest2 = regSpace->allocateRegister(i, base, noCost);
                (void) emit(loadConstOp, src2, dest2, dest2, i, base, noCost);
                (void) emit(op, src1, dest2, dest, i, base, noCost);
                regSpace->freeRegister(dest2);
                return(0);
		break;
	}
}


static int dummy[3];

void
initTocOffset(int toc_offset) {
    //  st r2,20(r1)  ; 0x90410014 save toc register  
    dummy[0] = 0x90410014; 

    //  liu r2, 0x0000     ;0x3c40abcd reset the toc value to 0xabcdefgh
    dummy[1] = (0x3c400000 | (toc_offset >> 16));

    //  oril    r2, r2,0x0000   ;0x6042efgh
    dummy[2] = (0x60420000 | (toc_offset & 0x0000ffff));
}


void cleanUpAndExit(int status);

//
// Author: Jeff Hollingsworth (3/26/96)
//
// Emit a function call.
//   It saves registers as needed.
//   copy the passed arguments into the canonical argument registers (r3-r10)
//   generate a branch and link the the destiation
//   restore the saved registers.
//
// Parameters:
//   op - unused parameter (to be compatible with sparc)
//   srcs - vector of ints indicating the registers that contain the parameters
//   dest - the destination address (should be unsigned not reg). 
//   insn - pointer to the code we are generating
//   based - offset into the code generated.
//

unsigned emitFuncCall(opCode /* ocode */, 
		      registerSpace *rs,
		      char *iPtr, unsigned &base, 
		      const vector<AstNode *> &operands, 
		      const string &callee, process *proc, bool noCost)
{
  unsigned dest;
  bool err;
  vector <reg> srcs;
  int i;
  unsigned ui;
  
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
	
  // If this code tries to save registers, it might save them in the
  // wrong area; there was a conflict with where the base trampoline saved
  // registers and the function, so now the function is saving the registers
  // above the location where the base trampoline did
  for (unsigned u = 0; u < operands.size(); u++) {
    srcs += operands[u]->generateCode(proc, rs, iPtr, base, false);
  }
  
  // TODO cast
  instruction *insn = (instruction *) ((void*)&iPtr[base]);
  vector<int> savedRegs;
  
  //     Save the link register.
  // mflr r0
  insn->raw = MFLR0raw;
  insn++;
  base += sizeof(instruction);
  
  // st r0, (r1)
  saveRegister(insn,base,0,8+(46*4));
  savedRegs += 0;
  
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
  // save REG_MT
  saveRegister(insn,base,REG_MT,8+(46*4));
  savedRegs += REG_MT;
#endif
  
  // see what others we need to save.
  for (i = 0; i < regSpace->getRegisterCount(); i++) {
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
	saveRegister(insn,base,reg->number,8+(46*4));
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
    string msg = "Too many arguments to function call in instrumentation code: only 8 arguments can be passed on the power architecture.\n";
    fprintf(stderr, msg.string_of());
    showErrorCallback(94,msg);
    cleanUpAndExit(-1);
  }
  
  // Now load the parameters into registers.
  for (unsigned u=0; u<srcs.size(); u++){
    // check that is is not already in the register
    if (srcs[u] == (int) u+3) {
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

  //  Decrement stack ptr
  genImmInsn(insn, STUop, 1, 1, -376);
  insn++;
  base += sizeof(instruction);
    
  // save and reset the value of TOC 
  insn->raw = dummy[0]; insn++; base += sizeof(instruction);
  insn->raw = dummy[1]; insn++; base += sizeof(instruction);
  insn->raw = dummy[2]; insn++; base += sizeof(instruction);
  // 0x90410014 0x3c402000 0x6042fd7c 
  // cout << "Dummy" << dummy[0] << "\t" << dummy[1]<< "\t" << dummy[2]<<endl;

  // generate a to the subroutine to be called.
  // load r0 with address, then move to link reg and branch and link.
  
  // really addis 0,dest,HIGH(dest) aka lis dest, HIGH(dest)
  genImmInsn(insn, CAUop, 0, 0, HIGH(dest));
  insn++;
  base += sizeof(instruction);
  
  // ori dest,dest,LOW(src1)
  genImmInsn(insn, ORILop, 0, 0, LOW(dest));
  insn++;
  base += sizeof(instruction);
  
  // mtlr	0 (aka mtspr 8, rs) = 0x7c0803a6
  insn->raw = MTLR0raw;
  insn++;
  base += sizeof(instruction);
  
  // brl - branch and link through the link reg.
  insn->raw = BRLraw;
  insn++;
  base += sizeof(instruction);
  
  // should their be a nop of some sort here?, sec
  // insn->raw = 0x4ffffb82;  // nop
  // insn++;
  // base += sizeof(instruction);
  
  // now cleanup.
  genImmInsn(insn, CALop, 1, 1, 376);
  insn++;
  base += sizeof(instruction);
  
  // sec
  //  // restore TOC
  //  genImmInsn(insn, Lop, 2, 1, 20);
  //  insn++;
  //  base += sizeof(instruction);

  // get a register to keep the return value in.
  reg retReg = regSpace->allocateRegister(iPtr, base, noCost);
  // This next line is a hack! - jkh 6/27/96
  // It is required since allocateRegister can generate code.
  insn = (instruction *) ((void*)&iPtr[base]);

  // put the return value from register 3 to the newly allocated register.
  genImmInsn(insn, ORILop, 3, retReg, 0);
  insn++;
  base += sizeof(instruction);
  
  // restore saved registers.
  for (ui = 0; ui < savedRegs.size(); ui++) {
    restoreRegister(insn,base,savedRegs[ui],8+(46*4));
  }
  
  // mtlr	0 (aka mtspr 8, rs) = 0x7c0803a6
  insn->raw = MTLR0raw;
  insn++;
  base += sizeof(instruction);
  
  // return value is the register with the return value from the
  //   called function.
  return(retReg);
}

 
unsigned emit(opCode op, reg src1, reg src2, reg dest, char *baseInsn, 
	      unsigned &base, bool noCost)
{
    // TODO cast
    instruction *insn = (instruction *) ((void*)&baseInsn[base]);

    if (op == loadConstOp) {
	if (ABS(src1) > MAX_IMM) {
	    // really addis dest,0,HIGH(src1) aka lis dest, HIGH(src1)
	    genImmInsn(insn, CAUop, dest, 0, HIGH(src1));
	    insn++;

	    // ori dest,dest,LOW(src1)
	    genImmInsn(insn, ORILop, dest, dest, LOW(src1));
	    base += 2 * sizeof(instruction);
	} else {
	    // really add regd,0,imm
	    genImmInsn(insn, CALop, dest, 0, src1);
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

    } else if (op == loadIndirOp) {
	// really load dest, (dest)imm
	genImmInsn(insn, Lop, dest, src1, 0);

        base += sizeof(instruction);

    } else if (op ==  storeOp) {
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
	reg temp = regSpace->allocateRegister(baseInsn, base, noCost);

	// This next line is a hack! - jkh 6/27/96
	//   It is required since allocateRegister can generate code.
	insn = (instruction *) ((void*)&baseInsn[base]);

	// set upper 16 bits of  temp to be the top high.
	genImmInsn(insn, CAUop, temp, 0, high);
	base += sizeof(instruction);
        insn++;

	// low == LOW(dest)
	// generate -- st src1, low(temp)
	insn->dform.op = STop;
	insn->dform.rt = src1;
	insn->dform.ra = temp;
	insn->dform.d_or_si = LOW(dest);
	base += sizeof(instruction);
        insn++;

	regSpace->freeRegister(temp);

    } else if (op ==  storeIndirOp) {

	// generate -- st src1, dest
	insn->dform.op = STop;
	insn->dform.rt = src1;
	insn->dform.ra = dest;
	insn->dform.d_or_si = 0;
	base += sizeof(instruction);

    } else if (op ==  ifOp) {
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

     } else if (op == branchOp) {
	generateBranchInsn(insn, dest);
	insn++;

	generateNOOP(insn);
	base += sizeof(instruction)*2;
	return(base - 2*sizeof(instruction));

     } else if (op == updateCostOp) {
        if (!noCost) {

           regSpace->resetSpace();

	   // add in the cost to the passed pointer variable.

	   // high order bits of the address of the cummlative cost.
	   reg obsCostAddr = regSpace->allocateRegister(baseInsn, base, noCost);

	   // actual cost.
	   reg obsCostValue = regSpace->allocateRegister(baseInsn, base, noCost);

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
             reg reg = regSpace->allocateRegister(baseInsn, base, noCost);
             genImmInsn(insn, CAUop, reg, 0, HIGH(src1));
             insn++;
             genImmInsn(insn, ORILop, reg, reg, LOW(src1));
             insn++;
             base += 2 * sizeof(instruction);
             (void) emit(plusOp, reg, obsCostValue, obsCostValue, baseInsn, 
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
    } else if (op ==  trampPreamble) {
        // nothing to do in this platform
        
    } else if (op ==  trampTrailer) {
/*
	// restore the registers we have saved
	int i;
	for (i = 0; i < regSpace->getRegisterCount(); i++) {
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
    } else if (op == noOp) {
	generateNOOP(insn);
	base += sizeof(instruction);
    } else if (op == getRetValOp) {
	// return value is in register 3
	int i;
	int reg = 3;
	registerSlot *regSlot;

	// find the registerSlot for this register.
	for (i = 0; i < regSpace->getRegisterCount(); i++) {
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
    } else if (op == getParamOp) {
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
                reg temp;
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
                return(0);
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
                return(0);
		break;

	    // rel ops
	    case eqOp:
		genRelOp(insn, EQcond, BTRUEcond, src1, src2, dest, base);
		return(0);
		break;

            case neOp:
                genRelOp(insn, EQcond, BFALSEcond, src1, src2, dest, base);
                return(0);
                break;

            case lessOp:
		genRelOp(insn, LTcond, BTRUEcond, src1, src2, dest, base);
		return(0);
		break;

            case greaterOp:
                genRelOp(insn, GTcond, BTRUEcond, src1, src2, dest, base);
                return(0);
                break;

            case leOp:
		genRelOp(insn, GTcond, BFALSEcond, src1, src2, dest, base);
		return(0);
		break;

            case geOp:
                genRelOp(insn, LTcond, BFALSEcond, src1, src2, dest, base);
                return(0);
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
      return(0);
    }

//
// I don't know how to compute cycles for POWER instructions due to 
//   multiple functional units.  However, we can computer the number of
//   instructions and hope that is fairly close. - jkh 1/30/96
//
int getInsnCost(opCode op)
  {
    int cost = 0;

    /* XXX Need to add branchOp */
    if (op == loadConstOp) {
	// worse case is addi followed by ori
	cost = 2;
    } else if (op ==  loadOp) {
	// addis
	// l 
	cost = 2;
    } else if (op ==  loadIndirOp) {
	cost = 1;
    } else if (op ==  storeOp) {
	cost = 2;
    } else if (op ==  storeIndirOp) {
	cost = 2;
    } else if (op ==  ifOp) {
	// cmpi 0,0,src1,0
	// be 0, dest
	// nop
	cost = 3;
    } else if (op ==  callOp) {
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
    } else if (op == updateCostOp) {
        // In most cases this cost is 4, but if we can't use CALop because
        // the value is to large, then we'll need 2 additional operations to
        // load that value - naim
        cost += 4;

    } else if (op ==  trampPreamble) {
	// Generate code to update the observed cost.
	// generated code moved to the base trampoline.
	cost += 0;

    } else if (op ==  trampTrailer) {
	// Should compute the cost to restore registers here.  However, we lack 
	//   sufficient information to compute this value. We need to be 
	//   inside the code generator to know this amount.
	//

	// branch
	// nop
	cost += 2;
    } else if (op == noOp) {
	cost = 1;
    } else if (op == getParamOp) {
	// worse case is it is on the stack and takes one instruction.
	cost = 1;
    } else if (op == saveRegOp) {
	cost = 1;
    } else {
	switch (op) {
	    // integer ops
	    case plusOp:
	    case minusOp:
	    case timesOp:
	    case divOp:
		cost = 1;
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

	    default:
		cost = 0;
		break;
	}
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
  instruction retInsn;
  retInsn.raw = 0;
  funcReturns += new instPoint(this, retInsn, owner, 0, false, ipFuncReturn);

  instr.raw = owner->get_instruction(adr);
  while(instr.raw != 0x0) {
    if (isCallInsn(instr)) {
      // define a call point
      // this may update address - sparc - aggregate return value
      // want to skip instructions
      adr = newCallPoint(adr, instr, owner, err);
      if (err) return false;
    }

    // now do the next instruction
    adr += 4;
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
    sprintf(errorLine, "    heap starts at %x\n", instHeapStart);
    logLine(errorLine);
    sprintf(errorLine, "    max reachable at %x\n", 
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

  return true;
}



//
// This is specific to some processors that have info in registers that we
//   can read, but should not write.
//   On power, registers r3-r11 are parameters that must be read only.
//     However, sometimes we have spilled to paramter registers back to the
//     stack to use them as scratch registers.  In this case they are writeable.
//
bool registerSpace::readOnlyRegister(reg reg_number) 
{
    registerSlot *regSlot;

    // it's not a parameter registers so it is read/write
    if ((reg_number > 11) || (reg_number < 3)) return false;

    // find the registerSlot for this register.
    for (int i = 0; i < regSpace->getRegisterCount(); i++) {
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


bool returnInstance::checkReturnInstance(const vector<Address> &adr,
					 u_int &index)
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

   vector<heapItem*> srcAllocatedBlocks = parentProc->heaps[textHeap].heapActive.values();

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

      if (theLocation->addr==NULL) {
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

    if((target = (function_base *)instr.iPgetCallee())) {
       return true;
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
	generateBranchInsn(&newInsn,
			   newFunc->addr()+sizeof(instruction)-point->addr);
	newInsn.iform.lk = 1;
    }

    writeTextSpace((caddr_t)point->addr, sizeof(instruction), &newInsn);

    return true;
}
