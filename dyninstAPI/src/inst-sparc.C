/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993, 1994 Barton P. Miller, \
  Jeff Hollingsworth, Jon Cargille, Krishna Kunchithapadam, Karen Karavanic,\
  Tia Newhall, Mark Callaghan.  All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/dyninstAPI/src/inst-sparc.C,v 1.29 1995/10/19 22:30:54 mjrg Exp $";
#endif

/*
 * inst-sparc.C - Identify instrumentation points for a SPARC processors.
 *
 * $Log: inst-sparc.C,v $
 * Revision 1.29  1995/10/19 22:30:54  mjrg
 * Fixed code generation for constants in the range 1024 to 4096.
 *
 * Revision 1.28  1995/09/26  20:34:42  naim
 * Minor fix: change all msg char[100] by string msg everywhere, since this can
 * cause serious troubles. Adding some error messages too.
 *
 * Revision 1.27  1995/08/24  15:04:01  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.26  1995/08/05  17:14:46  krisna
 * read the code to find out why
 *
 * Revision 1.25  1995/05/25  20:39:02  markc
 * Classify indirect calls as "unknown" user functions
 *
 * Revision 1.24  1995/05/18  10:36:08  markc
 * Prevent reference null call point
 *
 * Revision 1.23  1995/03/10  19:33:47  hollings
 * Fixed several aspects realted to the cost model:
 *     track the cost of the base tramp not just mini-tramps
 *     correctly handle inst cost greater than an imm format on sparc
 *     print starts at end of pvm apps.
 *     added option to read a file with more accurate data for predicted cost.
 *
 * Revision 1.22  1995/02/24  04:42:04  markc
 * Check if an address could be for an instruction before checking to see if it
 * is delayed, since we should not be checking instructions that are out of range.
 *
 * Revision 1.21  1995/02/16  08:53:22  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.20  1995/02/16  08:33:26  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.19  1994/11/02  19:01:24  hollings
 * Made the observed cost model use a normal variable rather than a reserved
 * register.
 *
 * Revision 1.18  1994/11/02  11:07:09  markc
 * Attempted to reduce the number of types used to represent addresses
 * to 1.  Move sparc-independent routines to symtab.C.
 *
 * Revision 1.16  1994/10/13  07:24:42  krisna
 * solaris porting and updates
 *
 * Revision 1.15  1994/09/22  01:58:17  markc
 * made getStrOp() return const char*
 * changed *allocs to news
 * enter funcFrequencyTable handles into stringPool
 * cast args to ptrace, PCptrace
 *
 * Revision 1.14  1994/08/08  20:13:37  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.13  1994/07/28  22:40:38  krisna
 * changed definitions/declarations of xalloc functions to conform to alloc.
 *
 * Revision 1.12  1994/07/26  19:57:31  hollings
 * moved instruction definitions to seperate header file.
 *
 * Revision 1.11  1994/07/20  23:23:35  hollings
 * added insn generated metric.
 *
 * Revision 1.10  1994/07/14  23:30:24  hollings
 * Hybrid cost model added.
 *
 * Revision 1.9  1994/07/12  20:09:06  jcargill
 * Added warning if a function's code appears to be a valid insn.
 *
 * Revision 1.8  1994/07/06  00:35:44  hollings
 * Added code to handle SPARC ABI aggregate return type calling convention
 * of using the instruction after the call's delay slot to indicate aggregate
 * size.  We treat this as an extra delay slot and relocate it to the
 * base tramp as needed.
 *
 * Revision 1.7  1994/07/05  03:26:03  hollings
 * observed cost model
 *
 * Revision 1.6  1994/06/30  18:01:35  jcargill
 * Fixed MAX_BRANCH definition (offset is in words, not bytes).
 *
 * Revision 1.5  1994/06/29  22:37:19  hollings
 * Changed heap bound brack error to warning if we can reach some of the inst
 * heap.  The firewall will catch if this is a real error.
 *
 * Revision 1.4  1994/06/29  02:52:28  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.3  1994/06/27  18:56:47  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.2  1994/06/22  01:43:15  markc
 * Removed warnings.  Changed bcopy in inst-sparc.C to memcpy.  Changed process.C
 * reference to proc->status to use proc->heap->status.
 *
 * Revision 1.1  1994/01/27  20:31:22  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.11  1993/12/15  21:02:42  hollings
 * added PVM support.
 *
 * Revision 1.10  1993/12/13  19:54:03  hollings
 * count internal operations and recognize invalid instructions.
 *
 * Revision 1.9  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.9  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.8  1993/08/20  21:59:32  hollings
 * added generateNoOp.
 *
 * Revision 1.7  1993/08/17  22:29:59  hollings
 * corrected definition of call indirect to not include jmp %x (used in case).
 *
 * Revision 1.6  1993/08/11  01:54:12  hollings
 * added predicated cost model
 *
 * Revision 1.5  1993/07/13  18:27:37  hollings
 * changed return insn pattern to check consider any jmp through %i7 as a
 * return insn.
 *
 * Revision 1.4  1993/06/24  16:18:06  hollings
 * global fixes.
 *
 * Revision 1.3  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:45:45  hollings
 * Initial revision
 *
 *
 */

#include "util/h/headers.h"

#include "rtinst/h/rtinst.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "inst-sparc.h"
#include "arch-sparc.h"
#include "ast.h"
#include "util.h"
#include "internalMetrics.h"
#include "stats.h"
#include "os.h"
#include "showerror.h"

#define perror(a) P_abort();

#define ABS(x)		((x) > 0 ? x : -x)
#define MAX_BRANCH	0x1<<23
//#define MAX_IMM		0x1<<12		/* 11 plus shign == 12 bits */
#define MAX_IMM13       (4095)
#define MIN_IMM13       (-4096)

unsigned getMaxBranch() {
  return MAX_BRANCH;
}

#define	REG_G5		5
#define	REG_G6		6
#define	REG_G7		7

#define REG_L0          16
#define REG_L1          17
#define REG_L2          18

const char *registerNames[] = { "g0", "g1", "g2", "g3", "g4", "g5", "g6", "g7",
				"o0", "o1", "o2", "o3", "o4", "o5", "sp", "o7",
				"l0", "l1", "l2", "l3", "l4", "l5", "l6", "l7",
				"i0", "i1", "i2", "i3", "i4", "i5", "i6", "i7" };

dictionary_hash<string, unsigned> funcFrequencyTable(string::hash);

string getStrOp(int op)
{
    switch (op) {
	case SetCC: 	return("set");
	case ADDop3:	return("add");
	case ANDop3:	return("and");
	case ORop3:	return("or");
	case SUBop3:	return("sub");
	case SUBop3cc:	return("subcc");
	case SMULop3:	return("smul");
	case SDIVop3:	return("sdiv");
	case XNORop3:	return("xnor");
	case SAVEop3:	return("save");
	case RESTOREop3:	return("restore");
	case JMPLop3:	return("jmpl");
	default: 	return("???");
    }
}

inline void generateNOOP(instruction *insn)
{
    insn->raw = 0;
    insn->branch.op = 0;
    insn->branch.op2 = NOOPop2;

    // logLine("nop\n");
}

inline void generateBranchInsn(instruction *insn, int offset)
{
    if (ABS(offset) > MAX_BRANCH) {
	logLine("a branch too far\n");
	showErrorCallback(52, "");
	abort();
    }

    insn->raw = 0;
    insn->branch.op = 0;
    insn->branch.cond = BAcond;
    insn->branch.op2 = BICCop2;
    insn->branch.anneal = true;
    insn->branch.disp22 = offset >> 2;

    // logLine("ba,a %x\n", offset);
}

inline void genSimpleInsn(instruction *insn, int op, reg rs1, reg rs2, reg rd)
{
    insn->raw = 0;
    insn->rest.op = RESTop;
    insn->rest.rd = rd;
    insn->rest.op3 = op;
    insn->rest.rs1 = rs1;
    insn->rest.rs2 = rs2;

    // logLine("%s %%%s,%%%s,%%%s\n", getStrOp(op), registerNames[rs1], 
    // 	registerNames[rs2], registerNames[rd]);
}

inline void genImmInsn(instruction *insn, int op, reg rs1, int immd, reg rd)
{
    insn->raw = 0;
    insn->resti.op = RESTop;
    insn->resti.rd = rd;
    insn->resti.op3 = op;
    insn->resti.rs1 = rs1;
    insn->resti.i = 1;
    insn->resti.simm13 = immd;

    // logLine("%s %%%s,%d,%%%s\n", getStrOp(op), registerNames[rs1], immd,
    // 	registerNames[rd]);
}

inline void genRelOp(instruction *insn, int cond, reg rs1,
		     reg rs2, reg rd, unsigned &base)
{
    // cmp rs1, rs2
    genSimpleInsn(insn, SUBop3cc, rs1, rs2, 0); insn++;
    // mov 1, rd
    genImmInsn(insn, ORop3, 0, 1, rd); insn++;

    // b??,a +2
    insn->branch.op = 0;
    insn->branch.cond = cond;
    insn->branch.op2 = BICCop2;
    insn->branch.anneal = true;
    insn->branch.disp22 = 2;
    insn++;

    // clr rd
    genSimpleInsn(insn, ORop3, 0, 0, rd); insn++;
    base += 4 * sizeof(instruction);
}

inline void generateSetHi(instruction *insn, int src1, int dest)
{
    insn->raw = 0;
    insn->sethi.op = FMT2op;
    insn->sethi.rd = dest;
    insn->sethi.op2 = SETHIop2;
    insn->sethi.imm22 = HIGH(src1);

    // logLine("sethi  %%hi(0x%x), %%%s\n", HIGH(src1)*1024, 
    // 	registerNames[dest]);
}

// st rd, [rs1 + offset]
inline void generateStore(instruction *insn, int rd, int rs1, int offset)
{
    insn->resti.op = STop;
    insn->resti.rd = rd;
    insn->resti.op3 = STop3;
    insn->resti.rs1 = rs1;
    insn->resti.i = 1;
    insn->resti.simm13 = LOW(offset);
}

// load [rs1 + offset], rd
inline void generateLoad(instruction *insn, int rs1, int offset, int rd)
{
    insn->resti.op = LOADop;
    insn->resti.op3 = LDop3;
    insn->resti.rd = rd;
    insn->resti.rs1 = rs1;
    insn->resti.i = 1;
    insn->resti.simm13 = LOW(offset);
}

instPoint::instPoint(pdFunction *f, const instruction &instr, 
		     const image *owner, Address adr,
		     bool delayOK)
: addr(adr), originalInstruction(instr), inDelaySlot(false), isDelayed(false),
  callIndirect(false), callAggregate(false), callee(NULL), func(f)
{
  delaySlotInsn.raw = owner->get_instruction(adr+4);
  aggregateInsn.raw = owner->get_instruction(adr+8);

  if (IS_DELAYED_INST(instr))
    isDelayed = true;

  if (owner->isValidAddress(adr-4)) {
    instruction iplus1;
    iplus1.raw = owner->get_instruction(adr-4);
    if (IS_DELAYED_INST(iplus1) && !delayOK) {
      // ostrstream os(errorLine, 1024, ios::out);
      // os << "** inst point " << func->file->fullName << "/"
      //  << func->prettyName() << " at addr " << addr <<
      //	" in a delay slot\n";
      // logLine(errorLine);
      inDelaySlot = true;
    }
  }
}

// Determine if the called function is a "library" function or a "user" function
// This cannot be done until all of the functions have been seen, verified, and
// classified
//
void pdFunction::checkCallPoints() {
  int i;
  instPoint *p;
  Address loc_addr;

  vector<instPoint*> non_lib;

  for (i=0; i<calls.size(); ++i) {
    /* check to see where we are calling */
    p = calls[i];
    assert(p);

    if (isInsnType(p->originalInstruction, CALLmask, CALLmatch)) {
      // Direct call
      loc_addr = p->addr + (p->originalInstruction.call.disp30 << 2);
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

//    bool err = true;
//    AAAAAARRRRRRGGGGGGGHHHHHHH
//    why do a function parameter and a function scope local variable
//    need to have the same names
//
//    obviously this CODE HAS NOT WORKED CORRECTLY BEFORE
//    and all RESULTS OBTAINED FROM IT ARE BOGUS
//    or the caller was not checking errors (SURPRISE !)
//
//    modify the incoming parameter
//
    err = true;
    point = new instPoint(this, instr, owner, adr, false);

    if (!isInsnType(instr, CALLmask, CALLmatch)) {
      point->callIndirect = true;
      point->callee = NULL;
    } else
      point->callIndirect = false;

    // check for aggregate type being returned 
    // this is marked by insn after call's delay solt being an
    //   invalid insn.  We treat this as an extra delay slot and
    //   relocate it to base tramps as needed.
    instruction iplus2;
    iplus2.raw = owner->get_instruction(adr+8);
    if (!IS_VALID_INSN(iplus2)) {
      point->callAggregate = true;
      // increment to skip the invalid instruction in code verification
      ret += 8;
    }

    calls += point;
    err = false;
    return ret;
}

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
        printf("found freq.input file\n");
    }
    while (!feof(fp)) {
        fscanf(fp, "%s %f\n", name, &value);
        funcFrequencyTable[name] = (int) value;
        printf("adding %s %f\n", name, value);
    }
    fclose(fp);
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
	return(250);
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
        // 8 cycles for base tramp
        return(8);
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

    if (isInsnType(*insn, CALLmask, CALLmatch)) {
      newOffset = origAddr  - targetAddr + (insn->call.disp30 << 2);
      insn->call.disp30 = newOffset >> 2;
    } else if (isInsnType(*insn, BRNCHmask, BRNCHmatch)) {
      newOffset = origAddr - targetAddr + (insn->branch.disp22 << 2);
      if (ABS(newOffset) > MAX_BRANCH) {
	logLine("a branch too far\n");
	abort();
      } else {
	insn->branch.disp22 = newOffset >> 2;
      }
    } else if (isInsnType(*insn, TRAPmask, TRAPmatch)) {
      logLine("attempt to relocate trap\n");
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
		break;
	    case GLOBAL_PRE_BRANCH:
		thisTemp->globalPreOffset = ((void*)temp - (void*)tramp);
		break;
	    case LOCAL_POST_BRANCH:
		thisTemp->localPostOffset = ((void*)temp - (void*)tramp);
		break;
	    case GLOBAL_POST_BRANCH:
		thisTemp->globalPostOffset = ((void*)temp - (void*)tramp);
		break;
  	}	
    }
    thisTemp->size = (int) temp - (int) tramp;
}

registerSpace *regSpace;
int deadList[] = {16, 17, 18, 19, 20, 21, 22, 23 };

void initTramps()
{
    static bool inited=false;

    if (inited) return;
    inited = true;

    initATramp(&baseTemplate, (instruction *) baseTramp);

    regSpace = new registerSpace(sizeof(deadList)/sizeof(int), deadList,					 0, NULL);
}

/*
 * Install a base tramp -- fill calls with nop's for now.
 *
 */
void installBaseTramp(unsigned baseAddr, 
		      instPoint *location,
		      process *proc) 
{
    unsigned currAddr;
    instruction *code;
    instruction *temp;

    code = new instruction[baseTemplate.size];
    memcpy((char *) code, (char*) baseTemplate.trampTemp, baseTemplate.size);
    // bcopy(baseTemplate.trampTemp, code, baseTemplate.size);

    for (temp = code, currAddr = baseAddr; 
	(currAddr - baseAddr) < baseTemplate.size;
	temp++, currAddr += sizeof(instruction)) {
	if (temp->raw == EMULATE_INSN) {
	    *temp = location->originalInstruction;
	    relocateInstruction(temp, location->addr, currAddr);
	    if (location->isDelayed) {
		/* copy delay slot instruction into tramp instance */
		*(temp+1) = location->delaySlotInsn;
	    }
	    if (location->callAggregate) {
		/* copy invalid insn with aggregate size in it */
		*(temp+2) = location->aggregateInsn;
	    }
	} else if (temp->raw == RETURN_INSN) {
	    generateBranchInsn(temp, 
		(location->addr+sizeof(instruction) - currAddr));
	    if (location->isDelayed) {
		/* skip the delay slot instruction */
		temp->branch.disp22 += 1;
	    }
	    if (location->callAggregate) {
		/* skip the aggregate size slot */
		temp->branch.disp22 += 1;
	    }
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
    // PCptrace(PTRACE_WRITEDATA, proc, (char*)baseAddr, baseTemplate.size,
    // (char*)code);

    free(code);
}

void generateNoOp(process *proc, int addr)
{
    instruction insn;

    /* fill with no-op */
    insn.raw = 0;
    insn.branch.op = 0;
    insn.branch.op2 = NOOPop2;

    // TODO cast
    proc->writeTextWord((caddr_t)addr, insn.raw);
    // (void) PCptrace(PTRACE_POKETEXT, proc, (char*)addr, insn.raw, NULL);
}


unsigned findAndInstallBaseTramp(process *proc, 
				 instPoint *location)
{
    unsigned ret;
    process *globalProc;

    if (nodePseudoProcess && (proc->symbols == nodePseudoProcess->symbols)){
	globalProc = nodePseudoProcess;
	// logLine("findAndInstallBaseTramp global\n");
    } else {
	globalProc = proc;
    }
    if (!globalProc->baseMap.defines(location)) {
	ret = inferiorMalloc(globalProc, baseTemplate.size, textHeap);
	installBaseTramp(ret, location, globalProc);
	generateBranch(globalProc, location->addr, (int) ret);
	globalProc->baseMap[location] = ret;
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
    // PCptrace(PTRACE_WRITEDATA, inst->proc, (char*) inst->trampBase, codeSize, code);
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
    // (void) PCptrace(PTRACE_POKETEXT, proc, (char*)fromAddr, insn.raw, NULL);
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

unsigned emit(opCode op, reg src1, reg src2, reg dest, char *i, unsigned &base)
{
    // TODO cast
    instruction *insn = (instruction *) ((void*)&i[base]);

    if (op == loadConstOp) {
      // dest = src1:imm    TODO
      if (src1 > MAX_IMM13 || src1 < MIN_IMM13) {
	    generateSetHi(insn, src1, dest);
	    base += sizeof(instruction);
	    insn++;

	    // or regd,imm,regd
	    genImmInsn(insn, ORop3, dest, LOW(src1), dest);
	    base += sizeof(instruction);
	} else {
	    // really or %g0,imm,regd
	    genImmInsn(insn, ORop3, 0, src1, dest);

	    base += sizeof(instruction);
	}
    } else if (op ==  loadOp) {
	// dest = [src1]   TODO
	generateSetHi(insn, src1, dest);
	insn++;

	generateLoad(insn, dest, src1, dest);

	base += sizeof(instruction)*2;
    } else if (op ==  storeOp) {
	insn->sethi.op = FMT2op;
	insn->sethi.rd = src2;
	insn->sethi.op2 = SETHIop2;
	insn->sethi.imm22 = HIGH(dest);
	insn++;

	generateStore(insn, src1, src2, dest);

	base += sizeof(instruction)*2;
    } else if (op ==  ifOp) {
	// cmp src1,0
	genSimpleInsn(insn, SUBop3cc, src1, 0, 0); insn++;

	insn->branch.op = 0;
	insn->branch.cond = BEcond;
	insn->branch.op2 = BICCop2;
	insn->branch.anneal = false;
	insn->branch.disp22 = dest/4;
	insn++;

	generateNOOP(insn);
	base += sizeof(instruction)*3;
	return(base - 2*sizeof(instruction));
    } else if (op ==  callOp) {
	if (src1 > 0) {
	    genSimpleInsn(insn, ORop3, 0, src1, 8); insn++;
	    base += sizeof(instruction);
	}
	if (src2 > 0) {
	    genSimpleInsn(insn, ORop3, 0, src2, 9); insn++;
	    base += sizeof(instruction);
	}
	/* ??? - should really set up correct # of args */
	// clr i2
	genSimpleInsn(insn, ORop3, 0, 0, 10); insn++;
	base += sizeof(instruction);

	// clr i3
	genSimpleInsn(insn, ORop3, 0, 0, 11); insn++;
	base += sizeof(instruction);


	generateSetHi(insn, dest, 13); insn++;
	genImmInsn(insn, JMPLop3, 13, LOW(dest), 15); insn++;
	generateNOOP(insn);

	base += 3 * sizeof(instruction);

	// return value is the register with the return value from the
	//   function.
	// This needs to be %o0 since it is back in the callers scope.
	return(8);
    } else if (op ==  trampPreamble) {
        genImmInsn(insn, SAVEop3, 14, -112, 14);
	base += sizeof(instruction);
        insn++;
      
        // generate code to update the observed cost.
	// sethi %hi(dest), %l0
        generateSetHi(insn, dest, REG_L0);
	base += sizeof(instruction);
        insn++;
  
	// ld [%l0+ lo(dest)], %l1
        generateLoad(insn, REG_L0, dest, REG_L1);
	base += sizeof(instruction);
        insn++;
  
        // update value
	if (src1 <= MAX_IMM13) {
	    genImmInsn(insn, ADDop3, REG_L1, src1, REG_L1);
	    base += sizeof(instruction);
	    insn++;
	} else {
	    // load in two parts
            generateSetHi(insn, src1, REG_L2);
            base += sizeof(instruction);
            insn++;

            // or regd,imm,regd
            genImmInsn(insn, ORop3, REG_L2, LOW(src1), REG_L2);
            base += sizeof(instruction);
            insn++;

            // now add it
            genSimpleInsn(insn, ADDop3, REG_L1, REG_L2, REG_L1);
            base += sizeof(instruction);
            insn++;
	}
  
        // store result st %l1, [%l0+ lo(dest)];
        generateStore(insn, REG_L1, REG_L0, dest);
	base += sizeof(instruction);
	insn++;
    } else if (op ==  trampTrailer) {
        genSimpleInsn(insn, RESTOREop3, 0, 0, 0); 
	base += sizeof(instruction);
	insn++;

	generateNOOP(insn);
	base += sizeof(instruction);
	insn++;

	// dest is in words of offset and generateBranchInsn is bytes offset
	generateBranchInsn(insn, dest << 2);
	base += sizeof(instruction);
	insn++;

        // add no-op, SS-5 sometimes seems to try to decode this insn - jkh 2/14
        generateNOOP(insn);
        insn++;
        base += sizeof(instruction);

	return(base -  2 * sizeof(instruction));
    } else if (op == noOp) {
	generateNOOP(insn);
	base += sizeof(instruction);
    } else if (op == getParamOp) {
	// first 8 parameters are in register 24 ....
	if (src1 <= 8) {
	    return(24+src1);
	}
	abort();
    } else if (op == saveRegOp) {
	// should never be called for this platform.
	abort();
    } else {
      int op3=-1;
	switch (op) {
	    // integer ops
	    case plusOp:
		op3 = ADDop3;
		break;

	    case minusOp:
		op3 = SUBop3;
		break;

	    case timesOp:
		op3 = SMULop3;
		break;

	    case divOp:
		op3 = SDIVop3;
		break;

	    // Bool ops
	    case orOp:
		op3 = ORop3;
		break;

	    case andOp:
		op3 = ANDop3;
		break;

	    // rel ops
	    case eqOp:
		genRelOp(insn, BNEcond, src1, src2, dest, base);
		return(0);
		break;

            case neOp:
                genRelOp(insn, BEcond, src1, src2, dest, base);
                return(0);
                break;

	    case lessOp:
	    case greaterOp:
	    case leOp:
	    case geOp:
		abort();
		break;
	    
	    default:
		abort();
		break;
	}
	genSimpleInsn(insn, op3, src1, src2, dest);

	base += sizeof(instruction);
      }
    return(0);
}

//
// All values based on Cypress0 && Cypress1 implementations as documented in
//   SPARC v.8 manual p. 291
//
int getInsnCost(opCode op)
{
    if (op == loadConstOp) {
	return(1);
    } else if (op ==  loadOp) {
	// sethi + load single
	return(1+1);
    } else if (op ==  storeOp) {
	// sethi + store single
	// return(1+3); 
	// for SS-5 ?
	return(1+2); 
    } else if (op ==  ifOp) {
	// subcc
	// be
	// nop
	return(1+1+1);
    } else if (op ==  callOp) {
	int count = 0;

	// mov src1, %o0
	count += 1;

	// mov src2, %o1
	count += 1;

	// clr i2
	count += 1;

	// clr i3
	count += 1;

	// sethi
	count += 1;

	// jmpl
	count += 1;

	// noop
	count += 1;

	return(count);
    } else if (op ==  trampPreamble) {
	// save
        // sethi %hi(obsCost), %l0
        // ld [%lo + %lo(obsCost)], %l1
        // add %l1, <cost>, %l1
        // st %l1, [%lo + %lo(obsCost)]
        // return(1+1+2+1+3);
	return(1+1+1+1+2);
    } else if (op ==  trampTrailer) {
	// restore
	// noop
	// retl
	return(1+1+1);
    } else if (op == noOp) {
	// noop
	return(1);
    } else {
	switch (op) {
	    // rel ops
	    case eqOp:
            case neOp:
	        // bne -- assume taken
	        return(2);
	        break;

	    case lessOp:
	    case greaterOp:
	    case leOp:
	    case geOp:
		abort();
		return(-1);
		break;
	    
	    default:
		return(1);
		break;
	}
    }
}



/************************************************************************
 * void restore_original_instructions(process* p, instPoint* ip)
************************************************************************/

void
restore_original_instructions(process* p, instPoint* ip) {
    unsigned addr = ip->addr;

    // TODO cast
    p->writeTextWord((caddr_t)addr, ip->originalInstruction.raw);
    // PCptrace(PTRACE_POKETEXT, p, (char*)addr, ip->originalInstruction.raw, 0);

    addr += sizeof(instruction);

    if (ip->isDelayed) {
      // TODO cast
      p->writeTextWord((caddr_t)addr, ip->delaySlotInsn.raw);
      // PCptrace(PTRACE_POKETEXT, p, (char*)addr, ip->delaySlotInsn.raw, 0);
      addr += sizeof(instruction);
    }

    if (ip->callAggregate) {
      // TODO cast
      p->writeTextWord((caddr_t)addr, ip->aggregateInsn.raw);
      // PCptrace(PTRACE_POKETEXT, p, (char*)addr, ip->aggregateInsn.raw, 0);
      addr += sizeof(instruction);
    }

    return;
}

bool isReturnInsn(const instruction instr)
{
    if (isInsnType(instr, RETmask, RETmatch) ||
        isInsnType(instr, RETLmask, RETLmatch)) {
        if ((instr.resti.simm13 != 8) && (instr.resti.simm13 != 12)) {
	    logLine("*** FATAL Error:");
	    sprintf(errorLine, " unsupported return\n");
	    logLine(errorLine);
	    showErrorCallback(55, "");
	    P_abort();
        }
	return true;
    }
    return false;
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
