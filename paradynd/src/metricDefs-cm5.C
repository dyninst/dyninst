/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/metricDefs-cm5.C,v 1.10 1994/07/02 01:46:43 markc Exp $";
#endif

/*
 * metric.C - define and create metrics.
 *
 * $Log: metricDefs-cm5.C,v $
 * Revision 1.10  1994/07/02 01:46:43  markc
 * Use aggregation operator defines from util/h/aggregation.h
 * Changed average aggregations to summations.
 *
 * Revision 1.9  1994/07/01  22:14:15  markc
 * Moved createSyncWait from metricDefs-common to machine dependent files
 * since pvm uses a wall timer and cm5 uses a process timer.  On the cm5 the
 * process timer continues to run during blocking system calls.
 *
 * Revision 1.8  1994/06/29  02:52:39  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.7  1994/06/27  18:57:00  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.6  1994/04/13  16:48:11  hollings
 * fixed pause_time to work with multiple processes/node.
 *
 * Revision 1.5  1994/04/11  23:25:24  hollings
 * Added pause_time metric.
 *
 * Revision 1.4  1994/03/25  23:00:44  hollings
 * Made active_processes a sampledFunction.
 *
 * Revision 1.3  1994/02/03  23:29:44  hollings
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
#include "util.h"
#include "metricDefs-common.h"

extern libraryList msgFilterFunctions;
extern libraryList msgByteFunctions;
extern libraryList msgByteSentFunctions;
extern libraryList msgByteRecvFunctions;


// A process timer is used because it does not stop on blocking
// system calls.
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
    (createPredicateFunc) perModuleCPUTime },
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
    (createPredicateFunc) perModuleWallTime },
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
    (createPredicateFunc) perModuleCalls },
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
   (createPredicateFunc) defaultModulePredicate },
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
   (createPredicateFunc) defaultModulePredicate },
 { NULL, nullPredicate, (createPredicateFunc) NULL },
};

resourcePredicate globalOnlyPredicates[] = {
  { "/SyncObject/MsgTag",	
    simplePredicate,		
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
   simplePredicate,	
   (createPredicateFunc) NULL },
 { NULL, nullPredicate, (createPredicateFunc) NULL },
};

struct _metricRec DYNINSTallMetrics[] = {
    { { "active_processes", SampledFunction, aggSum, "Processes" },
      { (createMetricFunc) createActiveProcesses, defaultPredicates },
    },
    { { "cpu", EventCounter, aggSum, "# CPUs" },
      { (createMetricFunc) createCPUTime, cpuTimePredicates },
    },
    { { "exec_time", EventCounter, aggSum, "%Time" },
      { (createMetricFunc) createExecTime, wallTimePredicates },
    },
    { { "procedure_calls", EventCounter, aggSum, "Calls/sec" },
      { (createMetricFunc) createProcCalls, procCallsPredicates },
    },
    { { "msgs", EventCounter, aggSum, "Ops/sec" },
      { (createMetricFunc) createMsgs, defaultPredicates },
    },
    { { "msg_bytes", EventCounter, aggSum, "Bytes/Sec" },
      { (createMetricFunc) createMsgBytesTotal, defaultPredicates },
    },
    { { "msg_bytes_sent", EventCounter, aggSum, "Bytes/Sec" },
      { (createMetricFunc) createMsgBytesSent, defaultPredicates },
    },
    { { "msg_bytes_recv", EventCounter, aggSum, "Bytes/Sec" },
      { (createMetricFunc) createMsgBytesRecv, defaultPredicates },
    },
    { { "sync_ops", EventCounter, aggSum, "Ops/sec" },
      { (createMetricFunc) createSyncOps, defaultPredicates },
    },
    { { "sync_wait", EventCounter, aggSum, "# Waiting" },
      { (createMetricFunc) createSyncWait, defaultPredicates },
    },
};

int metricCount = sizeof(DYNINSTallMetrics)/sizeof(DYNINSTallMetrics[0]);
