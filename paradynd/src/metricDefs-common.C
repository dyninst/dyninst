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

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/metricDefs-common.C,v 1.2 1994/07/01 22:14:17 markc Exp $";
#endif

/*
 * $Log: metricDefs-common.C,v $
 * Revision 1.2  1994/07/01 22:14:17  markc
 * Moved createSyncWait from metricDefs-common to machine dependent files
 * since pvm uses a wall timer and cm5 uses a process timer.  On the cm5 the
 * process timer continues to run during blocking system calls.
 *
 * Revision 1.1  1994/06/29  02:52:41  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 *
 */

/* Note - createSyncWait is machine dependent.  The cm5 process timer
 * continues to run during blocking system calls, but the pvm process
 * timer does not.  Timing system calls is done with a wall timer in
 * pvm and a process timer in cm5 because of this.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "symtab.h"
#include "process.h"
#include "rtinst/h/rtinst.h"
#include "inst.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "rtinst/h/trace.h"

void createDefaultFuncPred(metricDefinitionNode *mn,
			   function *func, 
			   dataReqNode *dataPtr, 
			   AstNode *pred)
{
    int i;
    AstNode *enterNode, *leaveNode;

    enterNode = createPrimitiveCall("setCounter", dataPtr, 1);
    leaveNode = createPrimitiveCall("setCounter", dataPtr, 0);

    if (pred) {
	enterNode = createIf(pred, enterNode);
	leaveNode = createIf(pred, leaveNode);
    }

    for (; func; func=func->sibling) {
	for (i = 0; i < func->callCount; i++) {
	    if (callsUserFuncP(func->calls[i])) {
		mn->addInst(func->calls[i], leaveNode,
		    callPreInsn, orderLastAtPoint);
		mn->addInst(func->calls[i], enterNode,
		    callPostInsn, orderFirstAtPoint);
	    }
	}
	mn->addInst(func->funcEntry, enterNode, callPreInsn, orderLastAtPoint);

	mn->addInst(func->funcReturn, leaveNode, callPreInsn,orderFirstAtPoint);
    }
}

AstNode *defaultProcedurePredicate(metricDefinitionNode *mn, char *funcName,
    AstNode *pred)
{
    function *func;
    dataReqNode *dataPtr;

    func = findFunction(mn->proc->symbols, funcName);
    if (!func) {
	/* no such function in this process */
	// 0 predicate if always false.
	return(new AstNode(Constant, 0));
    }

    dataPtr = mn->addIntCounter(0, False);

    createDefaultFuncPred(mn, func, dataPtr, pred);

    return(new AstNode(DataValue, dataPtr));
}

AstNode *defaultModulePredicate(metricDefinitionNode *mn, char *constraint,
    AstNode *pred)
{
    int i;
    module *mod;
    function *func;
    char *funcName;
    dataReqNode *dataPtr;

    if (funcName = strchr(constraint, '/')) {
	funcName++;
	return(defaultProcedurePredicate(mn, funcName, pred));
    } else {
	// its for a module.
	mod = findModule(mn->proc->symbols, constraint);
	if (!mod) {
	    /* no such module in this process */
	    // 0 predicate if always false.
	    return(new AstNode(Constant, 0));
	}

	dataPtr = mn->addIntCounter(0, False);
	for (func = mod->funcs, i=0; i < mod->funcCount; i++) {
	    createDefaultFuncPred(mn, func, dataPtr, pred);
	}
	return(new AstNode(DataValue, dataPtr));
    }
}

AstNode *defaultProcessPredicate(metricDefinitionNode *mn, char *process,
    AstNode *pred)
{
    abort();
}


void createProcCalls(metricDefinitionNode *mn, AstNode *pred)
{
    function *func;
    AstNode *newCall;
    dataReqNode *counter;

    counter = mn->addIntCounter(0, True);
    newCall = createPrimitiveCall("addCounter", counter, 1);

    for (func = mn->proc->symbols->funcs; func; func = func->next) {
	if (!func->tag & TAG_LIB_FUNC) {
	    mn->addInst(func->funcEntry, newCall, callPreInsn,orderLastAtPoint);
	}
    }
    return;
}

void instAllFunctions(metricDefinitionNode *nm,
		      int tag,		/* bit mask to use */
		      AstNode *enterAst,
		      AstNode *leaveAst)
{
    function *func;

    for (func = nm->proc->symbols->funcs; func; func=func->next) {
	if (func->tag & tag) {
	    if (enterAst) {
		nm->addInst(func->funcEntry,
		    enterAst, callPreInsn, orderLastAtPoint);
	    }
	    if (leaveAst) {
		nm->addInst(func->funcReturn,
		    leaveAst, callPreInsn, orderFirstAtPoint);
	    }
	}
    }
}

dataReqNode *createCPUTime(metricDefinitionNode *mn, AstNode *pred)
{
    function *func;
    dataReqNode *dataPtr;
    AstNode *stopNode, *startNode;


    dataPtr = mn->addTimer(processTime);

    startNode = new AstNode("DYNINSTstartProcessTimer", 
	new AstNode(DataValue, dataPtr), NULL);
    if (pred) startNode = createIf(pred, startNode);

    stopNode = new AstNode("DYNINSTstopProcessTimer", 
	new AstNode(DataValue, dataPtr), NULL);
    if (pred) stopNode = createIf(pred, stopNode);

    instAllFunctions(mn, TAG_CPU_STATE, stopNode, startNode);

    func = findFunction(mn->proc->symbols, "main");
    mn->addInst(func->funcEntry, startNode,callPreInsn,orderLastAtPoint);

    mn->addInst(func->funcReturn, stopNode,callPreInsn,orderLastAtPoint);

    func = findFunction(mn->proc->symbols, "exit");
    assert(func);

    mn->addInst(func->funcEntry, stopNode, callPreInsn,orderLastAtPoint);

    return(dataPtr);
}

void createExecTime(metricDefinitionNode *mn, AstNode *pred)
{
    function *func;
    dataReqNode *dataPtr;
    AstNode *startNode, *stopNode;

    dataPtr = mn->addTimer(wallTime);

    startNode = createPrimitiveCall("DYNINSTstartWallTimer", dataPtr, 0);
    if (pred) startNode = createIf(pred, startNode);

    stopNode = createPrimitiveCall("DYNINSTstopWallTimer", dataPtr, 0);
    if (pred) stopNode = createIf(pred, stopNode);

    func = findFunction(mn->proc->symbols, "main");
    mn->addInst(func->funcEntry, startNode, callPreInsn, orderLastAtPoint);

    mn->addInst(func->funcReturn, stopNode, callPreInsn, orderLastAtPoint);

    func = findFunction(mn->proc->symbols, "exit");
    assert(func);

    mn->addInst(func->funcEntry, stopNode, callPreInsn, orderLastAtPoint);
}

void createSyncOps(metricDefinitionNode *mn, AstNode *trigger)
{
    AstNode *newSyncOp;
    dataReqNode *counter;
    
    counter = mn->addIntCounter(0, True);

    newSyncOp = createPrimitiveCall("addCounter", counter, 1);
    if (trigger) newSyncOp = createIf(trigger, newSyncOp);

    instAllFunctions(mn, TAG_CPU_STATE, newSyncOp, NULL);
}

void createActiveProcesses(metricDefinitionNode *mn, AstNode *trigger)
{
    mn->addIntCounter(1, True);

    return;
}

void createMsgs(metricDefinitionNode *mn, AstNode *trigger)
{
    AstNode *newMsgOp;
    dataReqNode *counter;
    
    counter = mn->addIntCounter(0, True);

    newMsgOp = createPrimitiveCall("addCounter", counter, 1);
    if (trigger) newMsgOp = createIf(trigger, newMsgOp);

    instAllFunctions(mn, TAG_MSG_FUNC, newMsgOp, NULL);

}

//
// place holder for pause time metric.
//
void dummyCreate(metricDefinitionNode *mn, AstNode *trigger)
{
}


void perProcedureWallTime(metricDefinitionNode *mn, 
			  char *funcName, 
			  AstNode *pred,
			  dataReqNode *dataPtr)
{
    int i;
    function *func;
    AstNode *startNode, *stopNode;

    if (!dataPtr) dataPtr = mn->addTimer(wallTime);

    /* function does not exhist in this process */
    func = findFunction(mn->proc->symbols, funcName);
    if (!func) return;

    startNode = createPrimitiveCall("DYNINSTstartWallTimer", dataPtr, 0);
    if (pred) startNode = createIf(pred, startNode);

    stopNode = createPrimitiveCall("DYNINSTstopWallTimer", dataPtr, 0);
    if (pred) stopNode = createIf(pred, stopNode);

    for (; func; func=func->sibling) {
	for (i = 0; i < func->callCount; i++) {
	    if (callsUserFuncP(func->calls[i])) {
		mn->addInst(func->calls[i], stopNode,
		    callPreInsn, orderLastAtPoint);

		mn->addInst(func->calls[i], startNode,
		    callPostInsn, orderFirstAtPoint);
	    }
	}
	mn->addInst(func->funcEntry, startNode, callPreInsn, orderLastAtPoint);
	mn->addInst(func->funcReturn, stopNode, callPreInsn, orderFirstAtPoint);
    }
}

void perModuleWallTime(metricDefinitionNode *mn, 
			  char *constraint, 
			  AstNode *pred)
{
    int i;
    module *mod;
    function *func;
    char *funcName;
    dataReqNode *result;

    if (funcName = strchr(constraint, '/')) {
	funcName++;
	perProcedureWallTime(mn, funcName, pred, NULL);
    } else {
	mod = findModule(mn->proc->symbols, constraint);
	if (!mod) {
	    /* no such module in this process */
	    return;
	}

	result = mn->addTimer(wallTime);
	for (func = mod->funcs, i=0; i < mod->funcCount; i++, func=func->next) {
	    perProcedureWallTime(mn, func->prettyName, pred, result);
	}

	return;
    }
}

AstNode *perProcedureCPUTime(metricDefinitionNode *mn, 
			     char *funcName, 
			     AstNode *trigger,
			     dataReqNode *dataPtr)
{

    int i;
    function *func;
    AstNode *startNode, *stopNode;

    func = findFunction(mn->proc->symbols, funcName);

    /* function does not exhist in this process */
    if (!func) return(NULL);

    if (!dataPtr) dataPtr = mn->addTimer(processTime);

    startNode = new AstNode("DYNINSTstartProcessTimer", 
	new AstNode(DataValue, dataPtr), NULL);
    if (trigger) startNode = createIf(trigger, startNode);

    stopNode = new AstNode("DYNINSTstopProcessTimer", 
	new AstNode(DataValue, dataPtr), NULL);
    if (trigger) stopNode = createIf(trigger, stopNode);

    for (; func; func=func->sibling) {
	for (i = 0; i < func->callCount; i++) {
	    if (callsUserFuncP(func->calls[i])) {
		mn->addInst(func->calls[i], stopNode,
		    callPreInsn, orderFirstAtPoint);
		
		mn->addInst(func->calls[i], startNode,
		    callPostInsn, orderFirstAtPoint);
	    }
	}
	mn->addInst(func->funcEntry, startNode, callPreInsn, orderLastAtPoint);

	mn->addInst(func->funcReturn, stopNode, callPreInsn, orderFirstAtPoint);
    }

    return(NULL);
}

void perModuleCPUTime(metricDefinitionNode *mn, 
		      char *constraint, 
		      AstNode *trigger)
{
    int i;
    module *mod;
    function *func;
    char *funcName;
    dataReqNode *result;

    if (funcName = strchr(constraint, '/')) {
	funcName++;
	perProcedureCPUTime(mn, funcName, trigger, NULL);
    } else {
	mod = findModule(mn->proc->symbols, constraint);
	if (!mod) {
	    /* no such module in this process */
	    return;
	}

	result = mn->addTimer(processTime);
	for (func = mod->funcs, i=0; i < mod->funcCount; i++, func=func->next) {
	    perProcedureCPUTime(mn, func->prettyName, trigger, result);
	}
    }
}

void perProcedureCalls(metricDefinitionNode *mn, 
		       char *funcName, 
		       AstNode *trigger)
{
    function *func;
    AstNode *newCall;
    dataReqNode *counter;

    counter = mn->addIntCounter(0, True);

    func = findFunction(mn->proc->symbols, funcName);

    /* function does not exhist in this process */
    if (!func) return;

    newCall = createPrimitiveCall("addCounter", counter, 1);
    if (trigger) newCall = createIf(trigger, newCall);

    for (; func; func=func->sibling) {
	mn->addInst(func->funcEntry, newCall, callPreInsn, orderLastAtPoint);
    }

    return;
}

void perModuleCalls(metricDefinitionNode *mn, 
		    char *constraint, 
		    AstNode *trigger)
{
    int i;
    module *mod;
    char *funcName;
    function *func;
    function *curr;
    AstNode *newCall;
    dataReqNode *counter;

    if (funcName = strchr(constraint, '/')) {
	funcName++;
	perProcedureCalls(mn, funcName, trigger);
	return;
    } else {
	counter = mn->addIntCounter(0, True);
	mod = findModule(mn->proc->symbols, constraint);
	if (!mod) {
	    /* no such module in this process */
	    return;
	}

	newCall = createPrimitiveCall("addCounter", counter, 1);
	if (trigger) newCall = createIf(trigger, newCall);

	for (func = mod->funcs, i=0; i < mod->funcCount; i++, func=func->next) {
	    for (curr=func; curr; curr=curr->sibling) {
		mn->addInst(curr->funcEntry, newCall, 
		    callPreInsn,orderLastAtPoint);
	    }
	}
	return;
    }
}

