/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/metricDefs-cm5.C,v 1.3 1994/02/03 23:29:44 hollings Exp $";
#endif

/*
 * metric.C - define and create metrics.
 *
 * $Log: metricDefs-cm5.C,v $
 * Revision 1.3  1994/02/03 23:29:44  hollings
 * Corrected metric type for active_processes.
 *
 * Revision 1.2  1994/02/01  18:46:54  hollings
 * Changes for adding perfConsult thread.
 *
 * Revision 1.1  1994/01/27  20:31:30  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.10  1993/12/13  19:55:26  hollings
 * support duplicate functions with the same name in a single binary.
 *
 * Revision 1.9  1993/11/01  22:51:15  hollings
 * fixed per procedure waiting time.
 *
 * Revision 1.8  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.7  1993/10/04  21:43:09  hollings
 * corrected per procedure cpu time metric.
 *
 * Revision 1.6  1993/10/01  21:29:41  hollings
 * Added resource discovery and filters.
 *
 * Revision 1.5  1993/08/20  22:02:16  hollings
 * added perProcedureCalls
 *
 * Revision 1.4  1993/08/11  01:49:07  hollings
 * chages for predicated cost model (build the inst before using it).
 *
 * Revision 1.3  1993/07/13  18:28:49  hollings
 * new include file syntax.
 * fixed procedure call defintion for number of points.
 *
 * Revision 1.2  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.1  1993/06/08  20:14:34  hollings
 * Initial revision
 *
 * Revision 1.1  1993/03/19  22:45:45  hollings
 * Initial revision
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"

AstNode *defaultProcedurePredicate(metricDefinitionNode *mn, char *funcName,
    AstNode *pred)
{
    int i;
    function *func;
    dataReqNode *dataPtr;
    AstNode *enterNode, *leaveNode;

    func = findFunction(mn->proc->symbols, funcName);
    if (!func) {
	/* no such function in this process */
	// 0 predicate if always false.
	return(new AstNode(Constant, 0));
    }

    dataPtr = mn->addIntCounter(0, False);

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
    return(new AstNode(DataValue, dataPtr));
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
	    mn->addInst(func->funcEntry, newCall,
		callPreInsn, orderLastAtPoint);
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

extern libraryList msgFilterFunctions;
extern libraryList msgByteFunctions;
extern libraryList msgByteSentFunctions;
extern libraryList msgByteRecvFunctions;

//
// ***** Warning this metric is CM-5 specific. *****
//
void createMsgBytesMetric(metricDefinitionNode *mn,
			  libraryList *funcs,
			  AstNode *trigger)
{
    function *func;
    AstNode *msgBytesAst;
    dataReqNode *dataPtr;

    dataPtr = mn->addIntCounter(0, True);

    // addCounter(counter, param4 * param5)
    msgBytesAst = new AstNode("addCounter", 
	    new AstNode(DataValue, dataPtr),
	    new AstNode(timesOp, new AstNode(Param, (void *) 4), 
				 new AstNode(Param, (void *) 5)));
    if (trigger) msgBytesAst = createIf(trigger, msgBytesAst);

    for (func = mn->proc->symbols->funcs; func; func = func->next) {
	if (funcs->find(func->prettyName)) {
	    mn->addInst(func->funcEntry, msgBytesAst,
		callPreInsn, orderLastAtPoint);
	}
    }
}

void createMsgBytesTotal(metricDefinitionNode *mn, AstNode *tr)
{
    createMsgBytesMetric(mn, &msgByteFunctions, tr);
}

void createMsgBytesSent(metricDefinitionNode *mn, AstNode *tr)
{
    createMsgBytesMetric(mn, &msgByteSentFunctions, tr);
}

void createMsgBytesRecv(metricDefinitionNode *mn, AstNode *tr)
{
    createMsgBytesMetric(mn, &msgByteSentFunctions, tr);
}

AstNode *defaultMSGTagPredicate(metricDefinitionNode *mn, 
			        char *tag, AstNode *trigger)
{
    int iTag;
    function *func;
    dataReqNode *data;
    AstNode *tagTest;
    AstNode *filterNode, *clearNode;

    iTag = atoi(tag);

    data = mn->addIntCounter(0, False);

    // (== param2, iTag)
    tagTest = new AstNode(eqOp, new AstNode(Param, (void *) 2),
				new AstNode(Constant, (void *) iTag));

    filterNode = createIf(tagTest, createPrimitiveCall("addCounter", data, 1));
    if (trigger) filterNode = createIf(trigger, filterNode);

    clearNode = createPrimitiveCall("setCounter", data, 0);
    if (trigger) clearNode = createIf(trigger, clearNode);

    for (func = mn->proc->symbols->funcs; func; func = func->next) {
        if (msgFilterFunctions.find(func->prettyName)) {
            mn->addInst(func->funcEntry, filterNode,
                callPreInsn, orderFirstAtPoint);
            mn->addInst(func->funcReturn, clearNode,
                callPreInsn, orderLastAtPoint);
        }
    }
    return(new AstNode(DataValue, data));
}

void createSyncWait(metricDefinitionNode *mn, AstNode *trigger)
{
    dataReqNode *dataPtr;
    AstNode *stopNode, *startNode;

    dataPtr = mn->addTimer(processTime);

    startNode = new AstNode("DYNINSTstartProcessTimer", 
	new AstNode(DataValue, dataPtr), NULL);
    if (trigger) startNode = createIf(trigger, startNode);

    stopNode = new AstNode("DYNINSTstopProcessTimer", 
	new AstNode(DataValue, dataPtr), NULL);
    if (trigger) stopNode = createIf(trigger, stopNode);

    instAllFunctions(mn, TAG_MSG_FUNC, startNode, stopNode);
}


void perProcedureWallTime(metricDefinitionNode *mn, 
			  char *funcName, 
			  AstNode *pred)
{
    int i;
    function *func;
    dataReqNode *dataPtr;
    AstNode *startNode, *stopNode;

    dataPtr = mn->addTimer(wallTime);

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

AstNode *perProcedureCPUTime(metricDefinitionNode *mn, 
			     char *funcName, 
			     AstNode *trigger)
{

    int i;
    function *func;
    AstNode *newTrigger;
    dataReqNode *dataPtr;
    AstNode *startNode, *stopNode;

    func = findFunction(mn->proc->symbols, funcName);

    /* function does not exhist in this process */
    if (!func) return(NULL);

#ifdef notdef
    // Why did I put this here ????
    newTrigger = defaultProcedurePredicate(mn, funcName, trigger);
    dataPtr = createCPUTime(mn, newTrigger);
#endif
    dataPtr = mn->addTimer(processTime);

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

AstNode *perProcedureCalls(metricDefinitionNode *mn, 
			     char *funcName, 
			     AstNode *trigger)
{
    function *func;
    AstNode *newCall;
    dataReqNode *counter;

    func = findFunction(mn->proc->symbols, funcName);

    /* function does not exhist in this process */
    if (!func) return(new AstNode(Constant, 0));

    counter = mn->addIntCounter(0, True);
    newCall = createPrimitiveCall("addCounter", counter, 1);
    if (trigger) newCall = createIf(trigger, newCall);

    for (; func; func=func->sibling) {
	mn->addInst(func->funcEntry, newCall, callPreInsn, orderLastAtPoint);
    }

    return(new AstNode(DataValue, counter));
}

resourcePredicate cpuTimePredicates[] = {
  { "/SyncObject/MsgTag",	
    invalidPredicate,		
    (createPredicateFunc) NULL },
  { "/SyncObject",	
    invalidPredicate,		
    (createPredicateFunc) NULL },
  { "/Machine",	
    nullPredicate,		
    (createPredicateFunc) NULL },
  { "/Process",	
    nullPredicate,		
    (createPredicateFunc) NULL },
  { "/Procedure",	
    replaceBase,		
    (createPredicateFunc) perProcedureCPUTime },
  { NULL, nullPredicate, (createPredicateFunc) NULL },
};

resourcePredicate wallTimePredicates[] = {
  { "/SyncObject/MsgTag",	
    simplePredicate,		
    (createPredicateFunc) defaultMSGTagPredicate },
  { "/SyncObject",	
    invalidPredicate,		
    (createPredicateFunc) NULL },
  { "/Procedure",	
    replaceBase,		
    (createPredicateFunc) perProcedureWallTime },
  { "/Machine",	
    nullPredicate,		
    (createPredicateFunc) NULL },
  { "/Process",	
    nullPredicate,		
    (createPredicateFunc) NULL },
  { NULL, nullPredicate, (createPredicateFunc) NULL },
};

resourcePredicate procCallsPredicates[] = {
  { "/SyncObject",	
    invalidPredicate,		
    (createPredicateFunc) NULL },
  { "/Machine",	
    nullPredicate,		
    (createPredicateFunc) NULL },
  { "/Process",	
    nullPredicate,		
    (createPredicateFunc) NULL },
  { "/Procedure",	
    replaceBase,		
    (createPredicateFunc) perProcedureCalls },
  { NULL, nullPredicate, (createPredicateFunc) NULL },
};

resourcePredicate msgPredicates[] = {
  { "/SyncObject/MsgTag",	
    simplePredicate,		
    (createPredicateFunc) defaultMSGTagPredicate },
  { "/SyncObject",	
    invalidPredicate,		
    (createPredicateFunc) NULL },
  { "/Machine",	
    nullPredicate,		
    (createPredicateFunc) NULL },
  { "/Process",	
    nullPredicate,		
    (createPredicateFunc) NULL },
 { "/Procedure",
   simplePredicate,	
   (createPredicateFunc) defaultProcedurePredicate },
 { NULL, nullPredicate, (createPredicateFunc) NULL },
};

resourcePredicate defaultPredicates[] = {
  { "/SyncObject/MsgTag",	
    simplePredicate,		
    (createPredicateFunc) defaultMSGTagPredicate },
  { "/SyncObject",	
    invalidPredicate,		
    (createPredicateFunc) NULL },
  { "/Machine",	
    nullPredicate,		
    (createPredicateFunc) NULL },
  { "/Process",	
    nullPredicate,		
    (createPredicateFunc) NULL },
 { "/Procedure",
   simplePredicate,	
   (createPredicateFunc) defaultProcedurePredicate },
 { NULL, nullPredicate, (createPredicateFunc) NULL },
};

struct _metricRec DYNINSTallMetrics[] = {
    { { "active_processes", EventCounter, "Processes" },
      { (createMetricFunc) createActiveProcesses, defaultPredicates },
    },
    { { "cpu", EventCounter, "# CPUs" },
      { (createMetricFunc) createCPUTime, cpuTimePredicates },
    },
    { { "exec_time", EventCounter, "%Time" },
      { (createMetricFunc) createExecTime, wallTimePredicates },
    },
    { { "procedure_calls", EventCounter, "Calls/sec" },
      { (createMetricFunc) createProcCalls, procCallsPredicates },
    },
    { { "msgs", EventCounter, "Ops/sec" },
      { (createMetricFunc) createMsgs, defaultPredicates },
    },
    { { "msg_bytes", EventCounter, "Bytes/Sec" },
      { (createMetricFunc) createMsgBytesTotal, defaultPredicates },
    },
    { { "msg_bytes_sent", EventCounter, "Bytes/Sec" },
      { (createMetricFunc) createMsgBytesSent, defaultPredicates },
    },
    { { "msg_bytes_recv", EventCounter, "Bytes/Sec" },
      { (createMetricFunc) createMsgBytesRecv, defaultPredicates },
    },
    { { "sync_ops", EventCounter, "Ops/sec" },
      { (createMetricFunc) createSyncOps, defaultPredicates },
    },
    { { "sync_wait", EventCounter, "# Waiting" },
      { (createMetricFunc) createSyncWait, defaultPredicates },
    },
};

int metricCount = sizeof(DYNINSTallMetrics)/sizeof(DYNINSTallMetrics[0]);
