/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/metricDefs-pvm.C,v 1.10 1994/07/01 22:14:18 markc Exp $";
#endif

/*
 * metric.C - define and create metrics.
 *
 * $Log: metricDefs-pvm.C,v $
 * Revision 1.10  1994/07/01 22:14:18  markc
 * Moved createSyncWait from metricDefs-common to machine dependent files
 * since pvm uses a wall timer and cm5 uses a process timer.  On the cm5 the
 * process timer continues to run during blocking system calls.
 *
 * Revision 1.9  1994/06/29  02:52:43  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.8  1994/06/27  18:57:02  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.7  1994/05/31  18:06:25  markc
 * Reordered predicate definitions.  Procedure preds must appear before sync 
 * object preds to enable conditions to be tested in the correct order.  Now, 
 * the condition variable that is set in a focus procedure is used as the 
 * trigger for a sync object procedure.  In the past, the order was reversed.  
 * Since all sync object procedures are called from focus procedures, the "if" 
 * was always false when a focus consisted of a specific procedure and 
 * specific msg tag.
 *
 * Revision 1.6  1994/05/12  22:24:08  markc
 * Fixed instrumentation for createMsgBytesSentMetric.
 *
 * Revision 1.5  1994/04/18  15:54:41  markc
 * Changed defaultMSGTagPredicate to look at the second parameter which is the
 * message tag in pvm.
 *
 * Revision 1.4  1994/04/18  15:11:00  markc
 * Changed tag test in defaultMSGTagPredicate to original value.
 *
 * Revision 1.3  1994/04/13  16:48:13  hollings
 * fixed pause_time to work with multiple processes/node.
 *
 * Revision 1.2  1994/04/13  03:09:01  markc
 * Turned off pause_metric reporting for paradyndPVM because the metricDefNode is
 * not setup properly.  Updated inst-pvm.C and metricDefs-pvm.C to reflect changes
 * in cm5 versions.
 *
 * Revision 1.1  1994/01/27  20:31:32  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.1  1993/12/15  21:03:07  hollings
 * Initial revision
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

#include "symtab.h"
#include "process.h"
#include "rtinst/h/rtinst.h"
#include "inst.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "rtinst/h/trace.h"
#include "metricDefs-common.h"

extern libraryList msgFilterFunctions;
extern libraryList msgByteFunctions;
extern libraryList msgByteSentFunctions;
extern libraryList msgByteRecvFunctions;

// A wall timer is used because the process timer will be stopped
// on blocking system calls.

void createSyncWait(metricDefinitionNode *mn, AstNode *trigger)
{
    dataReqNode *dataPtr;
    AstNode *stopNode, *startNode;

    dataPtr = mn->addTimer(wallTime);

    startNode = new AstNode("DYNINSTstartWallTimer", 
	new AstNode(DataValue, dataPtr), NULL);
    if (trigger) startNode = createIf(trigger, startNode);

    stopNode = new AstNode("DYNINSTstopWallTimer", 
	new AstNode(DataValue, dataPtr), NULL);
    if (trigger) stopNode = createIf(trigger, stopNode);

    instAllFunctions(mn, TAG_MSG_FUNC, startNode, stopNode);
}


void finishMsgBytesMetric(metricDefinitionNode *mn,
			  libraryList *funcs,
			  AstNode *trigger,
			  AstNode *msgBytesAst)
{
  function *func;

  if (trigger) msgBytesAst = createIf(trigger, msgBytesAst);

  for (func = mn->proc->symbols->funcs; func; func = func->next) {
    if (funcs->find(func->prettyName)) {
      mn->addInst(func->funcReturn, msgBytesAst,
		  callPreInsn, orderFirstAtPoint);
    }
  }
}


//
// ***** Warning this metric is pvm specific. *****
//
void createMsgBytesRecvMetric(metricDefinitionNode *mn,
			      libraryList *funcs,
			      AstNode *trigger,
			      dataReqNode *dataPtr,
			      dataReqNode *tempCounter)
{
  AstNode *msgBytesAst;

  msgBytesAst =
    new AstNode (
		 new AstNode (
			      "pvm_bufinfo",
			      new AstNode("pvm_getrbuf", NULL, NULL),
			      new AstNode(DataPtr, tempCounter)
			      ),
		 new AstNode (
			      "addCounter",
			      new AstNode (DataValue, dataPtr),
			      new AstNode(DataValue, tempCounter)
			      )
		 );
  finishMsgBytesMetric (mn, funcs, trigger, msgBytesAst);
}

//
// ***** Warning this metric is pvm specific. *****
//
void createMsgBytesSentMetric(metricDefinitionNode *mn,
			      libraryList *funcs,
			      AstNode *trigger,
			      dataReqNode *dataPtr,
			      dataReqNode *tempCounter)
{
  AstNode *msgBytesAst;

  msgBytesAst =
    new AstNode (
		 new AstNode (
			      "pvm_bufinfo",
			      new AstNode("pvm_getsbuf", NULL, NULL),
			      new AstNode(DataPtr, tempCounter)
			      ),
		 new AstNode (
			      "addCounter",
			      new AstNode (DataValue, dataPtr),
			      new AstNode(DataValue, tempCounter)
			      )
		 );
  finishMsgBytesMetric (mn, funcs, trigger, msgBytesAst);
}

void createMsgBytesSent(metricDefinitionNode *mn, AstNode *tr)
{
  dataReqNode *dataPtr, *tempCounter;

  dataPtr = mn->addIntCounter(0, True);
  tempCounter = mn->addIntCounter(0, False);

  createMsgBytesSentMetric (mn, &msgByteSentFunctions, tr, dataPtr, tempCounter);
}

void createMsgBytesRecv(metricDefinitionNode *mn, AstNode *tr)
{
  dataReqNode *dataPtr, *tempCounter;

  dataPtr = mn->addIntCounter(0, True);
  tempCounter = mn->addIntCounter(0, False);

  createMsgBytesRecvMetric (mn, &msgByteRecvFunctions, tr, dataPtr, tempCounter);
}

// provide different send and receive metric funcs since the call to determine
// the message buffer is different for send and receive
void createMsgBytesTotal(metricDefinitionNode *mn, AstNode *tr)
{
  // these will be shared by both the send and receive 
  dataReqNode *dataPtr, *tempCounter;

  dataPtr = mn->addIntCounter(0, True);
  tempCounter = mn->addIntCounter(0, False);

  createMsgBytesRecvMetric (mn, &msgByteRecvFunctions, tr, dataPtr, tempCounter);
  createMsgBytesSentMetric (mn, &msgByteSentFunctions, tr, dataPtr, tempCounter);
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

    // NOTE - this is 1 since paramters are numbered starting with
    // 0.
    tagTest = new AstNode(eqOp, new AstNode(Param, (void *) 1),
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
  { "/Procedure",	
    replaceBase,		
    (createPredicateFunc) perModuleCPUTime },
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
  { NULL, nullPredicate, (createPredicateFunc) NULL },
};

resourcePredicate wallTimePredicates[] = {
  { "/Process",	
    nullPredicate,		
    (createPredicateFunc) NULL },
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
  { NULL, nullPredicate, (createPredicateFunc) NULL },
};

resourcePredicate procCallsPredicates[] = {
  { "/Procedure",	
    replaceBase,		
    (createPredicateFunc) perModuleCalls },
  { "/SyncObject",	
    invalidPredicate,		
    (createPredicateFunc) NULL },
  { "/Machine",	
    nullPredicate,		
    (createPredicateFunc) NULL },
  { "/Process",	
    nullPredicate,		
    (createPredicateFunc) NULL },
  { NULL, nullPredicate, (createPredicateFunc) NULL },
};

resourcePredicate msgPredicates[] = {
 { "/Procedure",
   simplePredicate,	
   (createPredicateFunc) defaultModulePredicate },
  { "/SyncObject/MsgTag",	
    simplePredicate,		
    (createPredicateFunc) defaultMSGTagPredicate },
  { "/SyncObject",	
    nullPredicate,
    (createPredicateFunc) NULL },
  { "/Machine",	
    nullPredicate,		
    (createPredicateFunc) NULL },
  { "/Process",	
    nullPredicate,		
    (createPredicateFunc) NULL },
 { NULL, nullPredicate, (createPredicateFunc) NULL },
};

resourcePredicate defaultPredicates[] = {
 { "/Procedure",
   simplePredicate,	
   (createPredicateFunc) defaultModulePredicate },
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
 { NULL, nullPredicate, (createPredicateFunc) NULL },
};

resourcePredicate globalOnlyPredicates[] = {
 { "/Procedure",
   simplePredicate,	
   (createPredicateFunc) NULL },
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
 { NULL, nullPredicate, (createPredicateFunc) NULL },
};

struct _metricRec DYNINSTallMetrics[] = {
    { { "active_processes", SampledFunction, opAvg, "Processes" },
      { (createMetricFunc) createActiveProcesses, defaultPredicates },
    },
    { { "cpu", EventCounter, opAvg, "# CPUs" },
      { (createMetricFunc) createCPUTime, cpuTimePredicates },
    },
    { { "exec_time", EventCounter, opAvg, "%Time" },
      { (createMetricFunc) createExecTime, wallTimePredicates },
    },
    { { "procedure_calls", EventCounter, opAvg, "Calls/sec" },
      { (createMetricFunc) createProcCalls, procCallsPredicates },
    },
    { { "msgs", EventCounter, opAvg, "Ops/sec" },
      { (createMetricFunc) createMsgs, defaultPredicates },
    },
    { { "msg_bytes", EventCounter, opAvg, "Bytes/Sec" },
      { (createMetricFunc) createMsgBytesTotal, defaultPredicates },
    },
    { { "msg_bytes_sent", EventCounter, opAvg, "Bytes/Sec" },
      { (createMetricFunc) createMsgBytesSent, defaultPredicates },
    },
    { { "msg_bytes_recv", EventCounter, opAvg, "Bytes/Sec" },
      { (createMetricFunc) createMsgBytesRecv, defaultPredicates },
    },
    { { "sync_ops", EventCounter, opAvg, "Ops/sec" },
      { (createMetricFunc) createSyncOps, msgPredicates },
    },
    { { "sync_wait", EventCounter, opAvg, "# Waiting" },
      { (createMetricFunc) createSyncWait, msgPredicates },
    },
};

int metricCount = sizeof(DYNINSTallMetrics)/sizeof(DYNINSTallMetrics[0]);








