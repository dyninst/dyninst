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

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/inst-sparc.C,v 1.8 1994/07/06 00:35:44 hollings Exp $";
#endif

/*
 * inst-sparc.C - Identify instrumentation points for a SPARC processors.
 *
 * $Log: inst-sparc.C,v $
 * Revision 1.8  1994/07/06 00:35:44  hollings
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

#include <stdio.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <memory.h>

#include "rtinst/h/rtinst.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "inst-sparc.h"
#include "ast.h"
#include "util.h"
#include "internalMetrics.h"

#define perror(a) abort();

/*
 * Define sparc instruction information.
 *
 */
struct fmt1 {
    unsigned op:2;
    signed disp30:30;
};

struct fmt2 {
    unsigned op:2;
    unsigned anneal:1;
    unsigned cond:4;
    unsigned op2:3;
    signed disp22:22;
};

struct fmt2a {
    unsigned op:2;
    unsigned rd:5;
    unsigned op2:3;
    signed imm22:22;
};

struct fmt3 {
    unsigned op:2;
    unsigned rd:5;
    unsigned op3:6;
    unsigned rs1:5;
    unsigned i:1;
    unsigned unused:8;
    unsigned rs2:5;
};

struct fmt3i {
    unsigned op:2;
    unsigned rd:5;
    unsigned op3:6;
    unsigned rs1:5;
    unsigned i:1;
    signed simm13:13;
};

union instructUnion {
    struct fmt1	call;
    struct fmt2	branch;
    struct fmt2a sethi;
    struct fmt3 rest;
    struct fmt3i resti;
    unsigned int raw;
};

typedef union instructUnion instruction;


/*
 * Define the operation codes
 *
 */
#define SetCC		16
#define ADDop3		0
#define ANDop3		1
#define ORop3		2
#define SUBop3		4
#define SUBop3cc	SetCC|SUBop3
#define SMULop3		11
#define SDIVop3		15
#define XNORop3		SetCC|7
#define SAVEop3		60
#define RESTOREop3	61
#define JMPLop3		56

/* op = 11 */
#define STop	3
#define LDop3	0
#define STop3	4

#define FALSE	0
#define TRUE	1

#define ABS(x)		((x) > 0 ? x : -x)
#define MAX_BRANCH	0x1<<23
#define MAX_IMM		0x1<<12		/* 11 plus shign == 12 bits */


struct instPointRec {
    int addr;                   /* address of inst point */
    instruction originalInstruction;    /* original instruction */
    instruction delaySlotInsn;  /* original instruction */
    instruction aggregateInsn;  /* aggregate insn */
    int inDelaySlot;            /* Is the instruction in a dealy slot */
    int isDelayed;		/* is the instruction a delayed instruction */
    int callsUserFunc;		/* is it a call to a user function */
    int callIndirect;		/* is it a call whose target is rt computed ? */
    int callAggregate;		/* calling a func that returns an aggregate
				   we need to reolcate three insns in this case
				 */
    function *callee;		/* what function is called */
    function *func;		/* what function we are inst */
};

/* mask bits for various parts of the instruction format */
#define OPmask		0xc0000000
#define OP2mask		0x00e00000
#define OP3mask		0x01f80000
#define RDmask		0x3e000000

#define DISP30mask	0x3fffffff

/* op = 01 -- mask for and match for call instruction */
#define	CALLop		1
#define CALLmask	OPmask
#define CALLmatch	0x40000000

/* (op==10) && (op3 == 111000) 
 */
#define RESTop		2
#define JMPLmask	(OPmask|OP3mask)
#define JMPLmatch	0x81c00000

#define FMT2op		0
#define LOADop		3

/*
 * added this on 8/18 (jkh) to tell a jmpl from a call indirect.
 *
 */
#define CALLImask	(OPmask|RDmask|OP3mask)
#define CALLImatch	0x9fc00000

/* (op=10) && (op3==111001) trap instructions */
#define TRAPmask	(OPmask|OP3mask)
#define TRAPmatch	0x81c10000

/* (op == 00) && (op2 ^ 2) mask for conditional branching instructions */
#define BICCop2		2

#define BEcond		1
#define BLEcond		2
#define BAcond		8
#define BNEcond		9

#define BRNCHmask	(OPmask|OP2mask)
#define BRNCHmatch	0x1<<23

/* really jmpl %i7+8,%g0 */
/* changed this to jmpl %i7+??,%g0 since g++ sometimes returns +0xc not +0x8 
 * jkh - 7/12/93
 */
#define RETmask         0xfffff000
#define RETmatch	0x81c7e000

/* retl - leaf return really jmpl %o7+8,%g0 */
/* changed this to jmpl %i7+??,%g0 since g++ sometimes returns +0xc not +0x8
 * jkh - 7/12/93
 */
#define RETLmask        0xfffff000
#define RETLmatch	0x81c3e000

/* noop insn */
#define NOOPop2		4
#define SETHIop2	4

/* If these bits are non-zero an op2 instruction is a non-annuled branch */
#define ANNUL_BIT	0x40000000

#define LOW(x)	((x)%1024)
#define HIGH(x)	((x)/1024)

#define isInsn(insn, mask, match)	(((insn).raw & mask) == match)

#define IS_DELAYED_INST(insn)	\
	(insn.call.op == CALLop || \
	 isInsn(insn, JMPLmask, JMPLmatch) || \
	 isInsn(insn, BRNCHmask, BRNCHmatch) || \
	 isInsn(insn, TRAPmask, TRAPmatch))

/* catch small ints that are invalid instructions */
/*
 * insn.call.op checks for CALL or Format 3 insns
 * op2 == {2,4,6,7} checks for valid format 2 instructions.
 *    See SPARC Arch manual v8 p. 44.
 *
 */
#define IS_VALID_INSN(insn)     \
        ((insn.call.op) || ((insn.branch.op2 == 2) ||   \
                           (insn.branch.op2 == 4) ||    \
                           (insn.branch.op2 == 6) ||    \
                           (insn.branch.op2 == 7)))

extern int errno;
extern int totalMiniTramps;

#define	REG_G5		5
#define	REG_G6		6
#define	REG_G7		7

char *registerNames[] = { "g0", "g1", "g2", "g3", "g4", "g5", "g6", "g7",
			  "o0", "o1", "o2", "o3", "o4", "o5", "sp", "o7",
			  "l0", "l1", "l2", "l3", "l4", "l5", "l6", "l7",
			  "i0", "i1", "i2", "i3", "i4", "i5", "i6", "i7" };

char *getStrOp(int op)
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

void defineInstPoint(function *func, instPoint *point, instruction *code, 
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
	 sprintf(errorLine, "**** inst point %s %s at addr %x in a dely slot\n", 
	     func->file->fullName, func->prettyName, point->addr);
	 logLine(errorLine);
	 point->inDelaySlot = 1;
    }
}

void newCallPoint(function *func, instruction *code, int codeIndex, int offset)
{
    caddr_t addr;
    instPoint *point;

    if (!func->calls) {
	func->callLimit = 10;
	func->calls = (instPoint**) xmalloc(sizeof(instPoint*)*func->callLimit);
    } else if (func->callCount == func->callLimit) {
	func->callLimit += 10;
	func->calls = (instPoint**) xrealloc(func->calls, 
	    sizeof(instPoint*)*func->callLimit);
    }

    point = (instPoint*) xcalloc(sizeof(instPoint), 1);
    defineInstPoint(func, point, code, codeIndex, offset, FALSE);

    point->callsUserFunc = 0;
    /* check to see where we are calling */
    if (isInsn(code[codeIndex], CALLmask, CALLmatch)) {
	point->callIndirect = 0;
	addr = (caddr_t) point->addr + (code[codeIndex].call.disp30 << 2);
	point->callee = findFunctionByAddr(func->file->exec, addr);
	if (point->callee && !(point->callee->tag & TAG_LIB_FUNC)) {
	    point->callsUserFunc = 1;
	}
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
    funcFrequencyTable.add(1, "main");
    funcFrequencyTable.add(1, "exit");
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
    function *func;

    if (point->callee)
        func = point->callee;
    else
        func = point->func;

    val = funcFrequencyTable.find(func->prettyName);
    if (!val) {
	if (func->tag & TAG_LIB_FUNC) {
	    return(1000);
	} else {
	    return(1000);
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
void locateInstPoints(function *func, void *codeV, int offset, int calls)
{
    int done;
    int codeIndex;
    instruction *code = (instruction *) codeV;

    codeIndex = ((unsigned int) (func->addr-offset))/sizeof(instruction);
    offset /= sizeof(instruction);
    if (!IS_VALID_INSN(code[codeIndex])) {
        func->funcEntry = NULL;
        func->funcReturn = NULL;
        return;
    }
    func->funcEntry = (instPoint *) xcalloc(sizeof(instPoint), 1);
    defineInstPoint(func, func->funcEntry, code,codeIndex,offset,TRUE);
    done = 0;
    while (!done) {
	if (isInsn(code[codeIndex],RETmask, RETmatch) || 
	    isInsn(code[codeIndex],RETLmask, RETLmatch)) {
	    done = 1;
	    func->funcReturn = (instPoint *) xcalloc(sizeof(instPoint), 1);
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
    function *func;
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

    for (func=i->funcs; func; func=func->next) {
	if (!(func->tag & TAG_LIB_FUNC)) {
//	    if (func->line) {
//		printf("inst %s line %d:%s\n", func->file->fileName, 
//		       func->line, func->prettyName);
//		logLine (output);
//	    }
	    locateInstPoints(func, (instruction *) i->code, i->textOffset,TRUE);
	}
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
		thisTemp->globalPreOffset = ((void*)temp - (void *)tramp);
		break;
	    case LOCAL_POST_BRANCH:
		thisTemp->localPostOffset = ((void*)temp - (void *)tramp);
		break;
	    case GLOBAL_POST_BRANCH:
		thisTemp->globalPostOffset = ((void*)temp- (void *)tramp);
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
    (void) PCptrace(PTRACE_WRITEDATA, proc, (char *)baseAddr, 
	baseTemplate.size, (char *) code);
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

    (void) PCptrace(PTRACE_POKETEXT, proc, (char *) addr, insn.raw, NULL);
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
    errno = 0;
    (void) PCptrace(PTRACE_WRITEDATA, inst->proc, (char *)inst->trampBase, 
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
    (void) PCptrace(PTRACE_POKETEXT, proc, (char *) fromAddr, 
	insn.raw, NULL);
    if (errno) {
	perror("PCptrace");
    }
}

int callsUserFuncP(instPoint *point)
{
    if (point->callIndirect) {
        printf("*** Warning call indirect\n from %s %s (addr %d)\n",
            point->func->file->fullName, point->func->prettyName, point->addr);
        return(1);
    } else {
        return(point->callsUserFunc);
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
function *getFunction(instPoint *point)
{
    return(point->callee ? point->callee : point->func);
}

caddr_t emit(opCode op, reg src1, reg src2, reg dest, char *i, caddr_t *base)
{
    int op3;
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
	// SPARC ABI reserved register %g7 (%g6 is used as the high order word)
	// add %g7, <cost>, %d7
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
	// add %g6, <cost>, %d6
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
