/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/inst.C,v 1.3 1994/06/27 18:56:51 hollings Exp $";
#endif

/*
 * inst.C - Code to install and remove inst funcs from a running process.
 *
 * $Log: inst.C,v $
 * Revision 1.3  1994/06/27 18:56:51  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.2  1994/03/20  01:53:08  markc
 * Added a buffer to each process structure to allow for multiple writers on the
 * traceStream.  Replaced old inst-pvm.C.  Changed addProcess to return type
 * int.
 *
 * Revision 1.1  1994/01/27  20:31:24  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.13  1993/12/15  21:02:42  hollings
 * added PVM support.
 *
 * Revision 1.12  1993/12/13  19:54:59  hollings
 * count operations.
 *
 * Revision 1.11  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.10  1993/10/04  21:37:10  hollings
 * re-enabled inst ordering directives.
 *
 * Revision 1.10  1993/10/04  21:37:10  hollings
 * re-enabled inst ordering directives.
 *
 * Revision 1.9  1993/10/01  21:29:41  hollings
 * Added resource discovery and filters.
 *
 * Revision 1.8  1993/08/23  23:11:19  hollings
 * fixed removing tramps to work correctly.
 *
 * Revision 1.7  1993/08/11  01:31:57  hollings
 * fixed call noargs & with args to be more general.
 *
 * Revision 1.6  1993/07/13  18:28:19  hollings
 * new include file syntax.
 *
 * Revision 1.5  1993/06/28  23:13:18  hollings
 * fixed process stopping.
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
#include <stdlib.h>
#include <assert.h>
#include <sys/ptrace.h>
#include <sys/signal.h>
#include <sys/param.h>
#include <errno.h>

#include "rtinst/h/rtinst.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "ast.h"
#include "util.h"
#include "internalMetrics.h"

static instInstance *instList;

extern int trampBytes;
extern trampTemplate baseTemplate;
extern trampTemplate noArgsTemplate;
extern trampTemplate withArgsTemplate;

int getBaseBranchAddr(process *proc, instInstance *inst)
{
    int fromAddr;

    fromAddr = (int) inst->baseAddr;
    if (inst->when == callPreInsn) {
	fromAddr += proc->aggregate ? baseTemplate.globalPreOffset :
	    baseTemplate.localPreOffset;
    } else {
	fromAddr += proc->aggregate ? baseTemplate.globalPostOffset :
	    baseTemplate.localPostOffset;
    }
    return(fromAddr);
}

void clearBaseBranch(process *proc, instInstance *inst)
{
    int addr;

    addr = (int) inst->baseAddr;
    if (inst->when == callPreInsn) {
	addr += proc->aggregate ? baseTemplate.globalPreOffset :
	    baseTemplate.localPreOffset;
    } else {
	addr += proc->aggregate ? baseTemplate.globalPostOffset :
	    baseTemplate.localPostOffset;
    }
    generateNoOp(proc, addr);
}

// implicit assumption that tramps generate to less than 64K bytes!!!
static char insn[65536];

instInstance *addInstFunc(process *proc, instPoint *location, AstNode *ast,
    callWhen when, callOrder order)
{
    int count;
    int fromAddr;
    instInstance *ret;
    instInstance *lastAtPoint;
    instInstance *firstAtPoint;

    assert(proc && location);

    initTramps();

    /* check if there are other inst points at this location. for this process
       at the same pre/post mode */
    firstAtPoint = NULL;
    lastAtPoint = NULL;
    for (ret=instList; ret; ret = ret->next) {
	if ((ret->location == location) && 
	    (ret->proc == proc) &&
	    (ret->when == when)) {
	    if (!ret->nextAtPoint) lastAtPoint = ret;
	    if (!ret->prevAtPoint) firstAtPoint = ret;
	}
    }

    ret = (instInstance*) xcalloc(sizeof(instInstance), 1);
    ret->proc = proc;

    /* make sure the base tramp has been installed for this point */
    ret->baseAddr = findAndInstallBaseTramp(proc, location);

    // 
    // Generate the code for this tramp.
    //
    // return value is offset of return stmnt.
    //
    count = 0;
    ret->returnAddr = ast->generateTramp(proc, insn, &count); 

    ret->trampBase = inferriorMalloc(proc, count);
    trampBytes += count;
    ret->returnAddr += ret->trampBase;

    ret->when = when;
    ret->location = location;

    ret->next = instList;
    ret->prev = NULL;
    if (instList) instList->prev = ret;
    instList = ret;

    /* first inst. at this point so install the tramp */
    fromAddr = (int) ret->baseAddr;
    if (ret->when == callPreInsn) {
	fromAddr += proc->aggregate ? baseTemplate.globalPreOffset :
	    baseTemplate.localPreOffset;
    } else {
	fromAddr += proc->aggregate ? baseTemplate.globalPostOffset :
	    baseTemplate.localPostOffset;
    }

    /*
     * Now make the call to actually put the code in place.
     *
     */
    installTramp(ret, insn, count);

    if (!lastAtPoint) {
	int fromAddr;

	fromAddr = getBaseBranchAddr(proc, ret);
	generateBranch(proc, fromAddr, ret->trampBase);

	generateBranch(proc, ret->returnAddr, fromAddr+4);
    } else if (order == orderLastAtPoint) {
	/* patch previous tramp to call us rather than return */
	generateBranch(proc, lastAtPoint->returnAddr, ret->trampBase);
	lastAtPoint->nextAtPoint = ret;
	ret->prevAtPoint = lastAtPoint;

	generateBranch(proc, ret->returnAddr, fromAddr+4);
    } else {
	/* first at point */
	firstAtPoint->prevAtPoint = ret;
	ret->nextAtPoint = firstAtPoint;

	/* branch to the old first one */
	generateBranch(proc, ret->returnAddr, firstAtPoint->trampBase);

	/* base tramp branches to us */
	fromAddr = getBaseBranchAddr(proc, ret);
	generateBranch(proc, fromAddr, ret->trampBase);
    }

    return(ret);
}

/*
 * The tramps are chained together left to right, so we need to find the
 *    tramps to the left anf right of the one to delete, and then patch the
 *    call from the left one to the old to call the right one.
 *    Also we need to patch the return from the right one to go to the left
 *    one.
 *
 */
void deleteInst(instInstance *old)
{
    instInstance *lag;
    instInstance *left;
    instInstance *right;
    instInstance *othersAtPoint;

    /* check if there are other inst points at this location. */
    othersAtPoint = NULL;
    left = right = NULL;
    for (lag=instList; lag; lag = lag->next) {
	if ((lag->location == old->location) && 
	    (lag->proc == old->proc) &&
	    (lag->when == old->when)) {
	    if (lag != old) {
		othersAtPoint = lag;
		left = old->prevAtPoint;
		right = old->nextAtPoint;
		assert(right || left);
	    }
	}
    }

    if (!othersAtPoint) {
	extern internalMetric activePoints;

	activePoints.value--;
	clearBaseBranch(old->proc, old);
    } else {
	if (left) {
	    if (right) {
		/* set left's return insn to branch to tramp to the right */
		generateBranch(old->proc, left->returnAddr, right->trampBase);
	    } else {
		/* branch back to the correct point in the base tramp */
		int fromAddr;

		fromAddr = getBaseBranchAddr(old->proc, old);

		// this assumes sizeof(int) == sizeof(instruction)
		generateBranch(old->proc,left->returnAddr,fromAddr+sizeof(int));
	    }
	} else {
	    /* old is first one make code call right tramp */
	    int fromAddr;

	    fromAddr = getBaseBranchAddr(old->proc, right);
	    generateBranch(old->proc, fromAddr, right->trampBase);
	}
    }

    inferriorFree(old->proc, old->trampBase);

    /* remove old from atPoint linked list */
    if (right) right->prevAtPoint = left;
    if (left) left->nextAtPoint = right;

    /* remove from doubly linked list for all insts */
    if (old->prev) {
	lag = old->prev;
	lag->next = old->next;
	if (old->next) old->next->prev = lag;
    } else {
	instList = old->next;
	if (old->next) old->next->prev = NULL;
    }
    free(old);
}


void installDefaultInst(process *proc, instMaping *initialRequests)
{
    int i;
    AstNode *ast;
    function *func;
    instMaping *item;

    for (item = initialRequests; item->func; item++) {
	func = findFunction(proc->symbols, item->func);
	if (!func) {
	    continue;
	}

	if (item->where & FUNC_ARG) {
	    ast = new AstNode(item->inst, item->arg, NULL);
	} else {
	    ast = new AstNode(item->inst, new AstNode(Constant, 0), NULL);
	}
	if (item->where & FUNC_EXIT) {
	    (void) addInstFunc(proc, func->funcReturn, ast,
		callPreInsn, orderLastAtPoint);
	}
	if (item->where & FUNC_ENTRY) {
	    (void) addInstFunc(proc, func->funcEntry, ast,
		callPreInsn, orderLastAtPoint);
	}
	if (item->where & FUNC_CALL) {
	    if (!func->callCount) {
		sprintf(errorLine, "no function calls in procedure %s\n", func->prettyName);
		logLine(errorLine);
	    } else {
		for (i = 0; i < func->callCount; i++) {
		    (void) addInstFunc(proc, func->calls[i], ast,
			callPreInsn, orderLastAtPoint);
		}
	    }
	}
	delete(ast);
    }
}

void pauseProcess(process *proc)
{
    if (proc->status == running) {
	(void) PCptrace(PTRACE_INTERRUPT, proc, (int*)1, 0, 0);
        proc->status = stopped;
    }
}

void continueProcess(process *proc)
{
    if (proc->status == stopped) {
	(void) PCptrace(PTRACE_CONT, proc, (int*)1, 0, 0);
        proc->status = running;
    }
}

void dumpCore(process *proc)
{
    (void) PCptrace(PTRACE_DUMPCORE, proc, "core.out", 0, 0);
}

/*
 * Copy data from controller process to the named process.
 *
 */
void copyToProcess(process *proc, void *from, void *to, int size)
{
    (void) PCptrace(PTRACE_WRITEDATA, proc, to, size, from);
}

void copyFromProcess(process *proc, void *from, void *to, int size)
{
    (void) PCptrace(PTRACE_READDATA, proc, from, size, to);
}
