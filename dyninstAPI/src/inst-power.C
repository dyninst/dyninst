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
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "inst-power.h"
#include "arch-power.h"
#include "aix.h"
#include "ast.h"
#include "util.h"
#include "internalMetrics.h"
#include "stats.h"
#include "os.h"
#include "showerror.h"

#define perror(a) P_abort();

class instPoint {
public:
  instPoint(pdFunction *f, const instruction &instr, const image *owner,
	    const Address adr, const bool delayOK);

  ~instPoint() {  /* TODO */ }

  // can't set this in the constructor because call points can't be classified until
  // all functions have been seen -- this might be cleaned up
  void set_callee(pdFunction *to) { callee = to; }


  Address addr;                   /* address of inst point */
  instruction originalInstruction;    /* original instruction */

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
    insn->branch.op = Bop;
    insn->branch.aa = 0;
    insn->branch.lk = 0;
    insn->branch.li = offset >> 2;

    // logLine("ba,a %x\n", offset);
}

inline void genImmInsn(instruction *insn, int op, reg rt, reg ra, int immd)
{
    insn->raw = 0;
    insn->dform.op = op;
    insn->dform.rt = rt;
    insn->dform.ra = ra;
    insn->dform.d_or_si = immd;
}

//
// generate an instruction that does nothing and has to side affect except to
//   advance the program counter.
//
inline void generateNOOP(instruction *insn)
{
    insn->raw = 0;
    insn->dform.op = ORIop;
    insn->dform.rt = 0;
    insn->dform.ra = 0;
    insn->dform.d_or_si = 0;
}

inline void genRelOp(instruction *insn, int cond, int mode, reg rs1,
		     reg rs2, reg rd, unsigned &base)
{
    // cmp rs1, rs2
    insn->raw = 0;
    insn->xform.op = XFPop;
    // really bf & l sub fields of rt we care about
    insn->xform.rt = 0;
    insn->xform.ra = rs1;
    insn->xform.rb = rs2;
    insn->xform.xo = CMPxop;
    insn++;

    // li rd, 1
    genImmInsn(insn, ADDIop, rd, 0, 1);
    insn++;

    // b??,a +2
    insn->cbranch.op = BCop;
    insn->cbranch.bi = cond;
    insn->cbranch.bo = mode;
    insn->cbranch.bd = 2;		// + two instructions */
    insn->cbranch.aa = 0;
    insn->cbranch.lk = 0;
    insn++;

    // clr rd
    genImmInsn(insn, ADDIop, rd, 0, 0);
    base += 4 * sizeof(instruction);
}


instPoint::instPoint(pdFunction *f, const instruction &instr, 
		     const image *owner, Address adr,
		     bool delayOK)
: addr(adr), originalInstruction(instr), inDelaySlot(false), isDelayed(false),
  callIndirect(false), callAggregate(false), callee(NULL), func(f)
{
}

// Determine if the called function is a "library" function or a "user" function
// This cannot be done until all of the functions have been seen, verified, and
// classified
//
void pdFunction::checkCallPoints() {
  unsigned int i;
  instPoint *p;
  Address loc_addr;

  vector<instPoint*> non_lib;

  for (i=0; i<calls.size(); ++i) {
    /* check to see where we are calling */
    p = calls[i];
    assert(p);

    if (isInsnType(p->originalInstruction, CALLmask, CALLmatch)) {
      loc_addr = p->addr + (p->originalInstruction.branch.li << 2);
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
    err = true;

    point = new instPoint(this, instr, owner, adr, false);

    if (!isInsnType(instr, CALLmask, CALLmatch)) {
      point->callIndirect = true;
      point->callee = NULL;
    } else
      point->callIndirect = false;

    point->callAggregate = false;

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
	// 35 cycles for base tramp
	return(35);
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
      newOffset = origAddr  - targetAddr + (insn->branch.li << 2);
      if (ABS(newOffset) > MAX_BRANCH) {
	logLine("a branch too far\n");
	abort();
      } else {
	insn->branch.li = newOffset >> 2;
      }
    } else if (isInsnType(*insn, Bmask, BCmatch)) {
      // conditional pc relative branch.
      newOffset = origAddr - targetAddr + (insn->cbranch.bd << 2);
      if (ABS(newOffset) > MAX_CBRANCH) {
	logLine("a branch too far\n");
	abort();
      } else {
	insn->cbranch.bd = newOffset >> 2;
      }
    } else if (insn->branch.op == SCop) {
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
  	}	
    }
    thisTemp->size = (int) temp - (int) tramp;
}

registerSpace *regSpace;

// 11-12 are defined not to have live values at procedure call points.
// reg 3-10 are used to pass arguments to functions.
//   We must save them before we can use them.
int deadRegList[] = { 11, 12 };
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
	    relocateInstruction(temp, location->addr, currAddr);
	} else if (temp->raw == RETURN_INSN) {
	    generateBranchInsn(temp, 
		(location->addr+sizeof(instruction) - currAddr));
        } else if (temp->raw == SKIP_PRE_INSN) {
          unsigned offset;
          offset = baseAddr+baseTemplate.emulateInsOffset-currAddr;
          generateBranchInsn(temp,offset);
        } else if (temp->raw == SKIP_POST_INSN) {
          unsigned offset;
          offset = baseAddr+baseTemplate.returnInsOffset-currAddr;
          generateBranchInsn(temp,offset);
	} else if ((temp->raw == LOCAL_PRE_BRANCH) ||
		   (temp->raw == GLOBAL_PRE_BRANCH) ||
		   (temp->raw == LOCAL_POST_BRANCH) ||
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

    /* fill with no-op */
    /* ori 0,0,0 */
    insn.raw = 0;
    insn.dform.op = ORIop;

    proc->writeTextWord((caddr_t)addr, insn.raw);
}


trampTemplate *findAndInstallBaseTramp(process *proc, 
				 instPoint *location,
				 returnInstance *&retInstance)
{
    trampTemplate *ret;
    process *globalProc;
    retInstance = NULL;

    globalProc = proc;
    if (!globalProc->baseMap.defines(location)) {
	ret = installBaseTramp(location, globalProc);
	instruction *insn = new instruction;
	generateBranchInsn(insn, ret->baseAddr - location->addr);
	globalProc->baseMap[location] = ret;
	retInstance = new returnInstance((instruction *)insn, 
					 sizeof(instruction), location->addr, sizeof(instruction));
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
      atAddr = inst->baseInstance->baseAddr+baseTemplate.skipPreInsOffset;
    }
    else {
      atAddr = inst->baseInstance->baseAddr+baseTemplate.skipPostInsOffset; 
    }
    generateNoOp(inst->proc, atAddr);
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
pdFunction *getFunction(instPoint *point)
{
    return(point->callee ? point->callee : point->func);
}


#define saveRegister(reg)			\
{						\
    genImmInsn(insn, STWop, reg, 1, -(reg+2)*4);	\
    insn++;					\
    base += sizeof(instruction);		\
}						\


#define restoreRegister(reg)			\
{						\
    genImmInsn(insn, Lop, reg, 1, -(reg+2)*4);	\
    insn++;					\
    base += sizeof(instruction);		\
}	\

unsigned emitImm(opCode op, reg src1, reg src2, reg dest, char *i, 
                 unsigned &base)
{
}

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

unsigned emitFuncCall(opCode op, 
		      registerSpace *rs,
		      char *iPtr, unsigned &base, 
		      vector<AstNode> operands, 
		      string callee, process *proc)
{
    unsigned dest;
    bool err;
    vector <reg> srcs;

    dest = (proc->symbols)->findInternalAddress(callee, false, err);
    if (err) {
	pdFunction *func = (proc->symbols)->findOneFunction(callee);
        if (!func) {
	    ostrstream os(errorLine, 1024, ios::out);
            os << "Internal error: unable to find addr of " << callee << endl;
            logLine(errorLine);
            showErrorCallback(80, (const char *) errorLine);
            P_abort();
	}
	dest = func->addr();
    }
	
    for (unsigned u = 0; u < operands.size(); u++)
	srcs += operands[u].generateCode(proc, rs, iPtr, base);

    // TODO cast
    instruction *insn = (instruction *) ((void*)&iPtr[base]);

    vector<int> savedRegs;

    //     Save the link register.
    // mflr r0
    insn->raw = MFLR0;
    insn++;
    base += sizeof(instruction);

    // st r0, (r1)
    saveRegister(0);
    savedRegs += 0;

    // see what others we need to save.
    int i;
    for (i = 0; i < regSpace->getRegisterCount(); i++) {
	registerSlot *reg = regSpace->getRegSlot(i);
	if (reg->needsSaving) {
	    // needsSaving -> caller saves register
	    // we MUST save restore this and the end of the function call
	    //     rather than delay it to the end of the tramp due to:
	    //        (1) we could be in a conditional & the restores would
	    //            be unconditional (i.e. restore bad data)
	    //        (2) $arg[n] code depeneds on paramters being in registers
	    //
	    saveRegister(reg->number);
	    savedRegs += reg->number;
	} else if (reg->inUse && !reg->mustRestore) {
	    // inUse && !mustRestore -> in use scratch register 
	    //		(i.e. part of an expression being evaluated).
	    saveRegister(reg->number);
	    savedRegs += reg->number;
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


    // Now load the parameters into registers.
    for (unsigned u=0; u<srcs.size(); u++){
	if (u >= 8) {
	     string msg = "Too many arguments to function call in instrumentation code: only 8 arguments can be passed on the power architecture.\n";
	     fprintf(stderr, msg.string_of());
	     showErrorCallback(94,msg);
	     cleanUpAndExit(-1);
	}

	// check that is is not already in the register
	if (srcs[u] == u+3) {
	    continue;
	}

	if (!regSpace->isFreeRegister(u+3)) {
	     // internal error we expect this register to be free here
	     abort();
	}

	genImmInsn(insn, ORIop, srcs[u], u+3, 0); insn++;
	base += sizeof(instruction);
	rs->freeRegister(srcs[u]);
    }

    // 
    //     Update the stack pointer
    //       argarea,     32 (8 registers)
    //       linkarea	    24 (6 registers)
    //       ngprs         32 (register we (may) clobber)
    //       szdsa =       4*ngprs+linkarea+argarea  = 184
    //       Decrement stack ptr and save back chain.

    //  stu r1, -184(r1)		(AKA STWU)
    genImmInsn(insn, STWUop, 1, 1, -184); insn++;
    base += sizeof(instruction);

    // generate a to the subroutine to be called.
    // load r0 with address, then move to link reg and branch and link.

    // really addis 0,dest,HIGH(dest) aka lis dest, HIGH(dest)
    genImmInsn(insn, ADDISop, 0, 0, HIGH(dest));
    insn++;
    base += sizeof(instruction);

    // ori dest,dest,LOW(src1)
    genImmInsn(insn, ORIop, 0, 0, LOW(dest));
    insn++;
    base += sizeof(instruction);

    // mtlr	0 (aka mtspr 8, rs) = 0x7c0803a6
    insn->raw = MTLR0;
    insn++;
    base += sizeof(instruction);

    // brl - branch and link through the link reg.
    insn->raw = BRL;
    
    insn++;
    base += sizeof(instruction);

    // now cleanup.

    //  ai r1, r1, 184
    genImmInsn(insn, ADDIop, 1, 1, 184);	
    insn++;
    base += sizeof(instruction);

    // get a register to keep the return value in.
    reg retReg = regSpace->allocateRegister(iPtr, base);

    // This next line is a hack! - jkh 6/27/96
    //   It is required since allocateRegister can generate code.
    insn = (instruction *) ((void*)&iPtr[base]);

    // put the return value from register 3 to the newly allocated register.
    genImmInsn(insn, ORIop, 3, retReg, 0); insn++;
    base += sizeof(instruction);

    // restore saved registers.
    for (i = 0; i < savedRegs.size(); i++) {
	restoreRegister(savedRegs[i]);
    }

    // mtlr	0 (aka mtspr 8, rs) = 0x7c0803a6
    insn->raw = MTLR0;
    insn++;
    base += sizeof(instruction);

    // return value is the register with the return value from the
    //   called function.
    return(retReg);
}
 
unsigned emit(opCode op, reg src1, reg src2, reg dest, char *baseInsn, 
	      unsigned &base)
{
    // TODO cast
    instruction *insn = (instruction *) ((void*)&baseInsn[base]);

    if (op == loadConstOp) {
	if (ABS(src1) > MAX_IMM) {
	    // really addis dest,0,HIGH(src1) aka lis dest, HIGH(src1)
	    genImmInsn(insn, ADDISop, dest, 0, HIGH(src1));
	    insn++;

	    // ori dest,dest,LOW(src1)
	    genImmInsn(insn, ORIop, dest, dest, LOW(src1));
	    base += 2 * sizeof(instruction);
	} else {
	    // really add regd,0,imm
	    genImmInsn(insn, ADDIop, dest, 0, src1);
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
	genImmInsn(insn, ADDISop, dest, 0, high);
        insn++;

	// really load dest, (dest)imm
	genImmInsn(insn, Lop, dest, dest, LOW(src1));

        base += sizeof(instruction)*2;
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
	reg temp = regSpace->allocateRegister(baseInsn, base);

	// This next line is a hack! - jkh 6/27/96
	//   It is required since allocateRegister can generate code.
	insn = (instruction *) ((void*)&baseInsn[base]);

	// set upper 16 bits of  temp to be the top high.
	genImmInsn(insn, ADDISop, temp, 0, high);
	base += sizeof(instruction);
        insn++;

	// low == LOW(dest)
	// generate -- st src1, low(temp)
	insn->dform.op = STWop;
	insn->dform.rt = src1;
	insn->dform.ra = temp;
	insn->dform.d_or_si = LOW(dest);
	base += sizeof(instruction);
        insn++;

	regSpace->freeRegister(temp);
    } else if (op ==  ifOp) {
	// cmpi 0,0,src1,0
        insn->raw = 0;
	insn->dform.op = CMPIop;
	insn->dform.ra = src1;
	insn->dform.d_or_si = 0;
	insn++;

	// bne 0, dest
	insn->cbranch.op = BCop;
	insn->cbranch.bo = BFALSEcond;
	insn->cbranch.bi = EQcond;		// not equal
	insn->cbranch.bd = dest/4;
	insn->cbranch.aa = 0;
	insn->cbranch.lk = 0;
	insn++;

	generateNOOP(insn);
	base += sizeof(instruction)*3;
	return(base - 2*sizeof(instruction));
    } else if (op ==  trampPreamble) {
	// add in the cost to the passed pointer variable.

	// high order bits of the address of the cummlative cost.
	reg obsCostAddr = regSpace->allocateRegister(baseInsn, base);

	// actual cost.
	reg obsCostValue = regSpace->allocateRegister(baseInsn, base);

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
        genImmInsn(insn, ADDISop, obsCostAddr, 0, high);
        insn++;

        // really load obsCostValue, (obsCostAddr)imm
        genImmInsn(insn, Lop, obsCostValue, obsCostAddr, LOW(dest));
	insn++;

	assert(src1 <= MAX_IMM);
        genImmInsn(insn, ADDIop, obsCostValue, obsCostValue, LOW(src1));
        insn++;

	// now store it back.
	// low == LOW(dest)
	// generate -- st obsCostValue, obsCostAddr+low(dest)
	insn->dform.op = STWop;
	insn->dform.rt = obsCostValue;
	insn->dform.ra = obsCostAddr;
	insn->dform.d_or_si = LOW(dest);
        insn++;
	base += 4 * sizeof(instruction);

	regSpace->freeRegister(obsCostValue);
	regSpace->freeRegister(obsCostAddr);
    } else if (op ==  trampTrailer) {
	// restore the registers we have saved
	int i;
	for (i = 0; i < regSpace->getRegisterCount(); i++) {
	    registerSlot *reg = regSpace->getRegSlot(i);
	    if (reg->mustRestore) {
		// cleanup register space for next trampoline.
		reg->needsSaving = true;
		reg->mustRestore = false;
		// actually restore the regitser.
		restoreRegister(reg->number);
	    }
	}

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
	    // ld i, -((reg+2)*4)(r1)
	    genImmInsn(insn, Lop, dest, 1, -(reg+2)*4);
	    insn++;	
	    base += sizeof(instruction);
	    return(dest);
	} else {
	    // its still in a register so return the register it is in.
	    return(reg);
	}
    } else if (op == getParamOp) {
	// parameters are stored in regster param number plus three
	int i;
	int reg = src1 + 3;
	registerSlot *regSlot;

	// arguments start in register 3 and run to 11.
	//   after that they are on the stack somewhere and we don't know
	//   how to find them (for now at least).
	if (reg > 11) abort();

	// find the registerSlot for this register.
	for (i = 0; i < regSpace->getRegisterCount(); i++) {
	    regSlot = regSpace->getRegSlot(i);
	    if (regSlot->number == reg) {
		break;
	    }
	}

	if (regSlot->mustRestore) {
	    // its on the stack so load it.
	    // ld i, -((reg+2)*4)(r1)
	    genImmInsn(insn, Lop, dest, 1, -(reg+2)*4);
	    insn++;	
	    base += sizeof(instruction);
	    return(dest);
	} else {
	    // its still in a register so return the register it is in.
	    return(reg);
	}
    } else if (op == saveRegOp) {
	saveRegister(src1);
    } else {
        int xop=-1;
	switch (op) {
	    // integer ops
	    case plusOp:
		xop = ADDxop;
		break;

	    case minusOp:
		xop = SUBFxop;
		break;

	    case timesOp:
		xop = MULLWxop;
		break;

	    case divOp:
		xop = DIVWxop;
		break;

	    // Bool ops
	    case orOp:
		abort();
		break;

	    case andOp:
		abort();		
		break;

	    // rel ops
	    case eqOp:
		genRelOp(insn, EQcond, BFALSEcond, src1, src2, dest, base);
		return(0);
		break;

            case neOp:
                genRelOp(insn, EQcond, BTRUEcond, src1, src2, dest, base);
                return(0);
                break;

            case lessOp:
		genRelOp(insn, LTcond, BFALSEcond, src1, src2, dest, base);
		return(0);
		break;

            case greaterOp:
                genRelOp(insn, GTcond, BFALSEcond, src1, src2, dest, base);
                return(0);
                break;

            case leOp:
		genRelOp(insn, GTcond, BTRUEcond, src1, src2, dest, base);
		return(0);
		break;

            case geOp:
                genRelOp(insn, LTcond, BTRUEcond, src1, src2, dest, base);
                return(0);
                break;

	    default:
		// internal error, invalid op.
		assert(0 && "Invalid op passed to emit");
		abort();
		break;
	}
	insn->raw = 0;
	insn->xoform.op = XFPop;
	insn->xoform.rt = dest;
	insn->xoform.ra = src1;
	insn->xoform.rb = src2;
	insn->xoform.xo = xop;

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

    if (op == loadConstOp) {
	// worse case is addi followed by ori
	cost = 2;
    } else if (op ==  loadOp) {
	// addis
	// l 
	cost = 2;
    } else if (op ==  storeOp) {
	cost = 2;
    } else if (op ==  ifOp) {
	// cmpi 0,0,src1,0
	// bne 0, dest
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
	//  stu r1, -184(r1)		(AKA STWU)
	// load r0 with address, then move to link reg and branch and link.
	// ori dest,dest,LOW(src1)
	// mtlr	0 (aka mtspr 8, rs) = 0x7c0803a6
	// brl - branch and link through the link reg.
	cost += 7;
	
	// now cleanup.

	//  ai r1, r1, 184
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
    } else if (op ==  trampPreamble) {
	// Generate code to update the observed cost.
	cost += 4;

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

//
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
//        2.) bctr or b insn followed by zero insn is a return from  a function.
//
// WARNING:  We use the heuristic that any return insns followed by a 0
//   is the last return in the current function.
//
bool isReturnInsn(const image *owner, Address adr, bool &lastOne)
{
    bool ret;
    instruction instr;
    instruction nextInstr;

    instr.raw = owner->get_instruction(adr);
    nextInstr.raw = owner->get_instruction(adr+4);
    if (isInsnType(instr, RETmask, RETmatch)) {
	// br instruction
	ret =  true;
    } else if (isInsnType(instr, RETmask, BCTRmatch) ||
	       isInsnType(instr, Bmask, Bmatch)) {
	// bctr or b 
	
	if (nextInstr.raw == 0) {
	    ret =  true;
	} else {
	    ret =  false;
	}
    } else {
	ret =  false;
    }

    if (ret == true) {
	// check for this to be the last instruction in a function
	//   we know it is the last one if the next words are 0. 
	//   this is the start of the traceback linkage for AIX.
	if (nextInstr.raw == 0) {
	    lastOne = true;
	} else {
	    lastOne = false;
	}
    }
    return ret;
}


bool pdFunction::findInstPoints(const image *owner) 
{  
  Address adr = addr();
  instruction instr;
  bool err;

  instr.raw = owner->get_instruction(adr);
  if (!IS_VALID_INSN(instr)) {
    return false;
  }

  funcEntry_ = new instPoint(this, instr, owner, adr, true);
  assert(funcEntry_);

  while (true) {
    instr.raw = owner->get_instruction(adr);

    bool done;

    // check for return insn and as a side affect decide if we are at the
    //   end of the function.
    if (isReturnInsn(owner, adr, done)) {
      // define the return point
      funcReturns += new instPoint(this, instr, owner, adr, false);

      // see if this return is the last one 
      if (done) return;
    } else if (isCallInsn(instr)) {
      // define a call point
      // this may update address - sparc - aggregate return value
      // want to skip instructions

      adr = newCallPoint(adr, instr, owner, err);
      if (err)
	return false;
    }

    // now do the next instruction
    adr += 4;

   }
}


//
// Each processor may have a different heap layout.
//   So we make this processor specific.
//
// find all DYNINST symbols that are data symbols
bool image::heapIsOk(const vector<sym_data> &find_us) {
  Address instHeapStart;
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

  string ghb = "_DYNINSTtext";
  if (!linkedFile.get_symbol(ghb, sym)) {
      string msg;
      msg = string("Cannot find ") + ghb + string(". Exiting");
      statusLine(msg.string_of());
      showErrorCallback(50, msg);
      return false;
  }
  instHeapStart = sym.addr();
  addInternalSymbol(ghb, instHeapStart);

  // check that we can get to our heap.
  if (instHeapStart > getMaxBranch() + linkedFile.code_off()) {
    logLine("*** FATAL ERROR: Program text + data too big for dyninst\n");
    sprintf(errorLine, "    heap starts at %x\n", instHeapStart);
    logLine(errorLine);
    sprintf(errorLine, "    max reachable at %x\n", 
	getMaxBranch() + linkedFile.code_off());
    logLine(errorLine);
    showErrorCallback(53,(const char *) errorLine);
    return false;
  } else if (instHeapStart + SYN_INST_BUF_SIZE > 
	     getMaxBranch() + linkedFile.code_off()) {
    logLine("WARNING: Program text + data could be too big for dyninst\n");
    showErrorCallback(54,(const char *) errorLine);
    return false;
  }

  string hd = "DYNINSTdata";
  if (!linkedFile.get_symbol(hd, sym)) {
      string msg;
      msg = string("Cannot find ") + hd + string(". Exiting");
      statusLine(msg.string_of());
      showErrorCallback(50, msg);
      return false;
  }
  instHeapStart = sym.addr();
  addInternalSymbol(hd, instHeapStart);

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


bool returnInstance::checkReturnInstance(const Address adr) {
    return true;
}
 
void returnInstance::installReturnInstance(process *proc) {
    proc->writeTextSpace((caddr_t)addr_, instSeqSize, (caddr_t) instructionSeq); 
}

void returnInstance::addToReturnWaitingList(Address , process * ) {
    P_abort();
}

void generateBreakPoint(instruction &insn) {
    insn.raw = BREAK_POINT_INSN;
}

bool doNotOverflow(int value)
{
  //
  // this should be changed by the correct code. If there isn't any case to
  // be checked here, then the function should return TRUE. If there isn't
  // any immediate code to be generated, then it should return FALSE - naim
  //
  return(false);
}

void instWaitingList::cleanUp(process * , Address ) {
    P_abort();
}
