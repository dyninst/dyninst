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

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/inst-sparc.C,v 1.17 1994/10/25 22:20:38 hollings Exp $";
#endif

/*
 * inst-sparc.C - Identify instrumentation points for a SPARC processors.
 *
 * $Log: inst-sparc.C,v $
 * Revision 1.17  1994/10/25 22:20:38  hollings
 * Added code to suppress "functions" that have aninvalid instruction
 * as their first instruction.  These are really read-only data that has
 * been placed in the text segment to protect it from writing.
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

extern "C" {
#include <stdio.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <memory.h>
}

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

#define perror(a) abort();

#define FALSE	0
#define TRUE	1

#define ABS(x)		((x) > 0 ? x : -x)
#define MAX_BRANCH	0x1<<23
#define MAX_IMM		0x1<<12		/* 11 plus shign == 12 bits */

extern int errno;
extern int insnGenerated;
extern int totalMiniTramps;

#define	REG_G5		5
#define	REG_G6		6
#define	REG_G7		7

const char *registerNames[] = { "g0", "g1", "g2", "g3", "g4", "g5", "g6", "g7",
				"o0", "o1", "o2", "o3", "o4", "o5", "sp", "o7",
				"l0", "l1", "l2", "l3", "l4", "l5", "l6", "l7",
				"i0", "i1", "i2", "i3", "i4", "i5", "i6", "i7" };

const char *getStrOp(int op)
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
	abort();
    }

    insn->raw = 0;
    insn->branch.op = 0;
    insn->branch.cond = BAcond;
    insn->branch.op2 = BICCop2;
    insn->branch.anneal = TRUE;
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
		     reg rs2, reg rd, caddr_t *base)
{
    // cmp rs1, rs2
    genSimpleInsn(insn, SUBop3cc, rs1, rs2, 0); insn++;
    // mov 1, rd
    genImmInsn(insn, ORop3, 0, 1, rd); insn++;

    // b??,a +2
    insn->branch.op = 0;
    insn->branch.cond = cond;
    insn->branch.op2 = BICCop2;
    insn->branch.anneal = TRUE;
    insn->branch.disp22 = 2;
    insn++;

    // clr rd
    genSimpleInsn(insn, ORop3, 0, 0, rd); insn++;
    *base += 4 * sizeof(instruction);
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

void defineInstPoint(pdFunction *func, instPoint *point, instruction *code, 
    int codeIndex, int offset, int delayOK)
{
    point->func = func;
    point->addr = (codeIndex+offset)<<2;
    point->originalInstruction = code[codeIndex];
    point->delaySlotInsn = code[codeIndex+1];
    point->aggregateInsn = code[codeIndex+2];
    point->isDelayed = 0;
    if (IS_DELAYED_INST(code[codeIndex])) {
	 point->isDelayed = 1;
    }
    point->inDelaySlot = 0;
    if (IS_DELAYED_INST(code[codeIndex-1]) && !delayOK) {
	 sprintf(errorLine,"**** inst point %s %s at addr %x in a dely slot\n", 
	     (char*) func->file->fullName, (char*)func->prettyName, point->addr);
	 logLine(errorLine);
	 point->inDelaySlot = 1;
    }
}

void newCallPoint(pdFunction *func, instruction *code, int codeIndex, int offset)
{
    caddr_t addr;
    instPoint *point;
    
    if (!func->calls) {
      func->callLimit = 10;
      func->calls = new instPoint*[func->callLimit];
    } else if (func->callCount == func->callLimit) {
      instPoint **newCalls;
      int i;
      newCalls = new instPoint*[func->callLimit + 10];
      for (i=0; i<func->callLimit; i++) {
	newCalls[i] = func->calls[i];
      }
      func->callLimit += 10;
      func->calls = newCalls;
    }

    point = new instPoint;
    defineInstPoint(func, point, code, codeIndex, offset, FALSE);

    /* check to see where we are calling */
    if (isInsn(code[codeIndex], CALLmask, CALLmatch)) {
	point->callIndirect = 0;
	addr = (caddr_t) point->addr + (code[codeIndex].call.disp30 << 2);
	point->callee = findFunctionByAddr(func->file->exec, addr);
    } else {
	point->callIndirect = 1;
	point->callee = NULL;
    }

    // check for aggregate type being returned 
    // this is marked by insn after call's delay solt being an
    //   invalid insn.  We treat this as an extra delay slot and
    //   relocate it to base tramps as needed.
    if (!IS_VALID_INSN(code[codeIndex+2])) {
	point->callAggregate = 1;
    }

    func->calls[func->callCount++] = point;
}

StringList<int> funcFrequencyTable;

void initDefaultPointFrequencyTable()
{
    funcFrequencyTable.add(1, pool.findAndAdd("main"));
    funcFrequencyTable.add(1, pool.findAndAdd("exit"));
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
    int val;
    pdFunction *func;

    if (point->callee)
        func = point->callee;
    else
        func = point->func;

    val = funcFrequencyTable.find(func->prettyName);
    if (!val) {
	if (func->tag & TAG_LIB_FUNC) {
	    return(100);
	} else {
	    return(250);
	}
    }
    return(val);
}

/*
 * Given the definition of a function and a pointer to the code for that
 *   function, find the standard entry/exit points from the function.
 *
 * WARNING: This code is highly machine dependent it looks as SPARC executables
 *   and finds selected instructions!
 *
 */
void locateInstPoints(pdFunction *func, void *codeV, int offset, int calls)
{
    int done;
    int codeIndex;
    instruction *code = (instruction *) codeV;

    codeIndex = ((unsigned int) (func->addr-offset))/sizeof(instruction);
    offset /= sizeof(instruction);
    if (!IS_VALID_INSN(code[codeIndex])) {
	func->tag |= TAG_NON_FUNC;
	// comment out warning since all gobal const have this property.
	// jkh 10/17/94
	// sprintf (errorLine, "Func '%s', code %x is not a valid insn\n", 
		 // (char*)func->prettyName, (code[codeIndex]).raw);
	// logLine (errorLine);
        func->funcEntry = NULL;
        func->funcReturn = NULL;
        return;
    }
    func->funcEntry = new instPoint;
    defineInstPoint(func, func->funcEntry, code,codeIndex,offset,TRUE);
    done = 0;
    while (!done) {
	if (isInsn(code[codeIndex],RETmask, RETmatch) || 
	    isInsn(code[codeIndex],RETLmask, RETLmatch)) {
	    done = 1;
	    func->funcReturn = new instPoint;
	    defineInstPoint(func, func->funcReturn, code, codeIndex, 
		offset,FALSE);
	    if ((code[codeIndex].resti.simm13 != 8) &&
	        (code[codeIndex].resti.simm13 != 12)) {
		logLine("*** FATAL Error:");
		sprintf(errorLine, " unsupported return at %x\n", 
		    func->funcReturn->addr);
		logLine(errorLine);
		abort();
	    }
	} else if (isInsn(code[codeIndex], CALLmask, CALLmatch) ||
		   isInsn(code[codeIndex], CALLImask, CALLImatch)) {
	    if (calls) {
		newCallPoint(func, code, codeIndex, offset);

	    }
	}
	codeIndex++;
    }
}

Boolean locateAllInstPoints(image *i)
{
    int curr;
    pdFunction *func;
    int instHeapEnd;


    instHeapEnd = (int) findInternalAddress(i, GLOBAL_HEAP_BASE, True);
    
    curr = (int) findInternalAddress(i, INFERRIOR_HEAP_BASE, True);
    if (curr > instHeapEnd) instHeapEnd = curr;

    // check that we can get to our heap.
    if (instHeapEnd > MAX_BRANCH) {
	logLine("*** FATAL ERROR: Program text + data too big for dyninst\n");
	sprintf(errorLine, "    heap ends at %x\n", instHeapEnd);
	logLine(errorLine);
	return(FALSE);
    } else if (instHeapEnd + SYN_INST_BUF_SIZE > MAX_BRANCH) {
	logLine("WARNING: Program text + data could be too big for dyninst\n");
    }

//
// always scan ALL functions, library or not to identify
// valid code points and stuff
//
// these functions may not be reported to paradyn nor instrumented
// otherwise
//

    for (func=i->funcs; func; func=func->next) {
//	if (!(func->tag & TAG_LIB_FUNC)) {
//	    if (func->line) {
//		sprintf(errorLine, "inst %s line %d:%s\n", func->file->fileName, 
//		       func->line, func->prettyName);
//		logLine (errorLine);
//	    }
	    locateInstPoints(func, (instruction *) i->code, i->textOffset,TRUE);
#ifdef notdef
	    if (!func->funcEntry && !func->funcReturn) {
		sprintf(errorLine, "could not find entry and exit for this function\n");
		logLine(errorLine);
	    }
#endif
//	}
//	else {
//	    sprintf(errorLine, "ignoring library function: %s %d %s\n",
//		func->file->fileName, func->line, func->prettyName);
//	    logLine(errorLine);
//	}
    }
    return(TRUE);
}



/*
 * Given and instruction, relocate it to a new address, patching up
 *   any relitive addressing that is present.
 *
 */
void relocateInstruction(instruction *insn, int origAddr, int targetAddr)
{
    int newOffset;

    if (isInsn(*insn, CALLmask, CALLmatch)) {
	newOffset = origAddr  - targetAddr + (insn->call.disp30 << 2);
	insn->call.disp30 = newOffset >> 2;
    } else if isInsn(*insn, BRNCHmask, BRNCHmatch) {
	newOffset = origAddr - targetAddr + (insn->branch.disp22 << 2);
	if (ABS(newOffset) > MAX_BRANCH) {
	    logLine("a branch too far\n");
	    abort();
	} else {
	    insn->branch.disp22 = newOffset >> 2;
	}
    } else if isInsn(*insn, TRAPmask, TRAPmatch) {
	logLine("attempt to relocate trap\n");
	abort();
    } 
    /* The rest of the instructions should be fine as is */
}

trampTemplate baseTemplate;
trampTemplate noArgsTemplate;
trampTemplate withArgsTemplate;

extern "C" void baseTramp();
extern "C" void noArgsTramp();
extern "C" void withArgsTramp();

void initATramp(trampTemplate *thisTemp, instruction *tramp)
{
    instruction *temp;

    thisTemp->trampTemp = (void *) tramp;
    for (temp = tramp; temp->raw != END_TRAMP; temp++) {
	switch (temp->raw) {
	    case LOCAL_PRE_BRANCH:
		thisTemp->localPreOffset = ((void *)temp - (void *)tramp);
		break;
	    case GLOBAL_PRE_BRANCH:
		thisTemp->globalPreOffset = ((void *)temp - (void *)tramp);
		break;
	    case LOCAL_POST_BRANCH:
		thisTemp->localPostOffset = ((void *)temp - (void *)tramp);
		break;
	    case GLOBAL_POST_BRANCH:
		thisTemp->globalPostOffset = ((void *)temp - (void *)tramp);
		break;
  	}	
    }
    thisTemp->size = (int) temp - (int) tramp;
}

registerSpace *regSpace;
int regList[] = {16, 17, 18, 19, 20, 21, 22, 23 };

void initTramps()
{
    static Boolean inited;

    if (inited) return;
    inited = True;

    initATramp(&baseTemplate, (instruction *) baseTramp);

    regSpace = new registerSpace(sizeof(regList)/sizeof(int), regList);
}

/*
 * Install a base tramp -- fill calls with nop's for now.
 *
 */
void installBaseTramp(int baseAddr, 
		      instPoint *location,
		      process *proc) 
{
    int currAddr;
    extern int errno;
    instruction *code;
    instruction *temp;

    code = (instruction *) xmalloc(baseTemplate.size);
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

    errno = 0;
    (void) PCptrace(PTRACE_WRITEDATA, proc, (void*)baseAddr, 
	baseTemplate.size, (void*) code);
    if (errno) {
	perror("data PCptrace");
    }

    free(code);
}

void generateNoOp(process *proc, int addr)
{
    instruction insn;

    /* fill with no-op */
    insn.raw = 0;
    insn.branch.op = 0;
    insn.branch.op2 = NOOPop2;

    (void) PCptrace(PTRACE_POKETEXT, proc, (void*) addr, insn.raw, NULL);
}


void *findAndInstallBaseTramp(process *proc, instPoint *location)
{
    void *ret;
    process *globalProc;
    extern process *nodePseudoProcess;

    if (nodePseudoProcess && (proc->symbols == nodePseudoProcess->symbols)){
	globalProc = nodePseudoProcess;
	// logLine("findAndInstallBaseTramp global\n");
    } else {
	globalProc = proc;
    }
    ret = globalProc->baseMap.find((void *) location);
    if (!ret) {
	ret = (void *) inferriorMalloc(globalProc, baseTemplate.size);
	installBaseTramp((int) ret, location, globalProc);
	generateBranch(globalProc, location->addr, (int) ret);
	globalProc->baseMap.add(ret, location);
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

    errno = 0;
    (void) PCptrace(PTRACE_WRITEDATA, inst->proc, (void*)inst->trampBase, 
	codeSize, (char *) code);
    if (errno) {
	perror("data PCptrace");
    }
}

/*
 * change the insn at addr to be a branch to newAddr.
 *   Used to add multiple tramps to a point.
 */
void generateBranch(process *proc, int fromAddr, int newAddr)
{
    int disp;
    instruction insn;

    disp = newAddr-fromAddr;
    generateBranchInsn(&insn, disp);

    errno = 0;
    (void) PCptrace(PTRACE_POKETEXT, proc, (void*) fromAddr, 
	insn.raw, NULL);
    if (errno) {
	perror("PCptrace");
    }
}

int callsTrackedFuncP(instPoint *point)
{
    if (point->callIndirect) {
#ifdef notdef
	// it's rare to call a library function as a parameter.
        sprintf(errorLine, "*** Warning call indirect\n from %s %s (addr %d)\n",
            point->func->file->fullName, point->func->prettyName, point->addr);
	logLine(errorLine);
#endif
        return(TRUE);
    } else {
	if (point->callee && !(point->callee->tag & TAG_LIB_FUNC)) {
	    return(TRUE);
	} else {
	    return(FALSE);
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

caddr_t emit(opCode op, reg src1, reg src2, reg dest, char *i, caddr_t *base)
{
    instruction *insn = (instruction *) &i[(unsigned) *base];

    if (op == loadConstOp) {
	if (ABS(src1) > MAX_IMM) {
	    generateSetHi(insn, src1, dest);
	    *base += sizeof(instruction);
	    insn++;

	    // or regd,imm,regd
	    genImmInsn(insn, ORop3, dest, LOW(src1), dest);
	    *base += sizeof(instruction);
	} else {
	    // really or %g0,imm,regd
	    genImmInsn(insn, ORop3, 0, LOW(src1), dest);
	    *base += sizeof(instruction);
	}
    } else if (op ==  loadOp) {
	generateSetHi(insn, src1, dest);
	insn++;

	insn->resti.op = LOADop;
	insn->resti.op3 = LDop3;
	insn->resti.rd = dest;
	insn->resti.rs1 = dest;
	insn->resti.i = 1;
	insn->resti.simm13 = LOW(src1);

	*base += sizeof(instruction)*2;
    } else if (op ==  storeOp) {
	insn->sethi.op = FMT2op;
	insn->sethi.rd = src2;
	insn->sethi.op2 = SETHIop2;
	insn->sethi.imm22 = HIGH(dest);
	insn++;

	insn->resti.op = STop;
	insn->resti.rd = src1;
	insn->resti.op3 = STop3;
	insn->resti.rs1 = src2;
	insn->resti.i = 1;
	insn->resti.simm13 = LOW(dest);

	*base += sizeof(instruction)*2;
    } else if (op ==  ifOp) {
	// cmp src1,0
	genSimpleInsn(insn, SUBop3cc, src1, 0, 0); insn++;

	insn->branch.op = 0;
	insn->branch.cond = BEcond;
	insn->branch.op2 = BICCop2;
	insn->branch.anneal = FALSE;
	insn->branch.disp22 = dest/4;
	insn++;

	generateNOOP(insn);
	*base += sizeof(instruction)*3;
	return(*base - 2*sizeof(instruction));
    } else if (op ==  callOp) {
	if (src1 > 0) {
	    genSimpleInsn(insn, ORop3, 0, src1, 8); insn++;
	    *base += sizeof(instruction);
	}
	if (src2 > 0) {
	    genSimpleInsn(insn, ORop3, 0, src2, 9); insn++;
	    *base += sizeof(instruction);
	}
	/* ??? - should really set up correct # of args */
	// clr i2
	genSimpleInsn(insn, ORop3, 0, 0, 10); insn++;
	*base += sizeof(instruction);

	// clr i3
	genSimpleInsn(insn, ORop3, 0, 0, 11); insn++;
	*base += sizeof(instruction);


	generateSetHi(insn, dest, 13); insn++;
	genImmInsn(insn, JMPLop3, 13, LOW(dest), 15); insn++;
	generateNOOP(insn);

	*base += 3 * sizeof(instruction);
    } else if (op ==  trampPreamble) {
	genImmInsn(insn, SAVEop3, 14, -112, 14);
	insn++;

	// generate code to update the observed cost register.
	// SPARC ABI reserved register %g7
	// add %g7, <cost>, %g7
	genImmInsn(insn, ADDop3, REG_G7, src1, REG_G7);

	*base += 2 * sizeof(instruction);
    } else if (op ==  trampTrailer) {
	genSimpleInsn(insn, RESTOREop3, 0, 0, 0); insn++;

	generateNOOP(insn);
	insn++;

	// dest is in words of offset and generateBranchInsn is bytes offset
	generateBranchInsn(insn, dest << 2);

	*base += sizeof(instruction) * 3;
	return(*base -  sizeof(instruction));
    } else if (op == noOp) {
	generateNOOP(insn);
	*base += sizeof(instruction);
    } else {
      int op3;
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

	*base += sizeof(instruction);
      }
    return(0);
}

//
// move the passed parameter into the passed register, or if it is already
//    in a register return the register number.
//
reg getParameter(reg dest, int param)
{
    if (param <= 8) {
	return(24+param);
    }
    abort();
    return(-1);
}

//
// All values based on Cypress0 && Cypress1 implementations as documented in
//   SPARC v.8 manual p. 291
//
int getInsnCost(opCode op)
{
    if (op == loadConstOp) {
	return(2);
    } else if (op ==  loadOp) {
	// sethi + load single
	return(1+2);
    } else if (op ==  storeOp) {
	// sethi + store single
	return(1+3);
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
	count += 2;

	// noop
	count += 1;

	return(count);
    } else if (op ==  trampPreamble) {
	// save %o6, -112, %o6
	// add %g7, <cost>, %g7
	return(2);
    } else if (op ==  trampTrailer) {
	// restore
	// noop
	// retl
	return(1+1+2);
    } else if (op == noOp) {
	// noop
	return(1);
    } else {
	switch (op) {
	    // rel ops
	    case eqOp:
            case neOp:
	        // bne -- assume taken
	        return(3);
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
    int addr = ip->addr;

    PCptrace(PTRACE_POKETEXT, p, (void*) addr,
        ip->originalInstruction.raw, 0);
    addr += sizeof(instruction);

    if (ip->isDelayed) {
        PCptrace(PTRACE_POKETEXT, p, (void*) addr,
            ip->delaySlotInsn.raw, 0);
        addr += sizeof(instruction);
    }

    if (ip->callAggregate) {
        PCptrace(PTRACE_POKETEXT, p, (void*) addr,
            ip->aggregateInsn.raw, 0);
        addr += sizeof(instruction);
    }

    return;
}
