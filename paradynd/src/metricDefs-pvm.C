/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/metricDefs-pvm.C,v 1.14 1994/09/22 02:18:57 markc Exp $";
#endif

/*
 * metric.C - define and create metrics.
 *
 * $Log: metricDefs-pvm.C,v $
 * Revision 1.14  1994/09/22 02:18:57  markc
 * Added static class initializers for DYNINSTallMetrics
 *
 * Revision 1.13  1994/08/17  18:15:34  markc
 * Removed finishMsgBytesMetric.
 * Moved code to check number of bytes sent to the function entry since
 * the send buffer will be freed by the function exit.
 *
 * Msg bytes received continues to check the message buffer at function
 * entry, since the buffer does not exist until the receive is done.
 *
 * Revision 1.12  1994/07/05  03:26:15  hollings
 * observed cost model
 *
 * Revision 1.11  1994/07/02  01:46:44  markc
 * Use aggregation operator defines from util/h/aggregation.h
 * Changed average aggregations to summations.
 *
 * Revision 1.10  1994/07/01  22:14:18  markc
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

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
}

#include "symtab.h"
#include "process.h"
#include "rtinst/h/rtinst.h"
#include "inst.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "rtinst/h/trace.h"
#include "metricDefs-common.h"

extern List<libraryFunc*> msgFilterFunctions;
extern List<libraryFunc*> msgByteFunctions;
extern List<libraryFunc*> msgByteSentFunctions;
extern List<libraryFunc*> msgByteRecvFunctions;

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




//
// ***** Warning this metric is pvm specific. *****
//
void createMsgBytesRecvMetric(metricDefinitionNode *mn,
			      List<libraryFunc*> *funcs,
			      AstNode *trigger,
			      dataReqNode *dataPtr,
			      dataReqNode *tempCounter)
{
  AstNode *msgBytesAst;
  pdFunction *func;

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

  if (trigger)
    msgBytesAst = createIf(trigger, msgBytesAst);

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
void createMsgBytesSentMetric(metricDefinitionNode *mn,
			      List<libraryFunc*> *funcs,
			      AstNode *trigger,
			      dataReqNode *dataPtr,
			      dataReqNode *tempCounter)
{
  AstNode *msgBytesAst;
  pdFunction *func;

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

  if (trigger)
    msgBytesAst = createIf(trigger, msgBytesAst);

  for (func = mn->proc->symbols->funcs; func; func = func->next) {
    if (funcs->find(func->prettyName)) {
      mn->addInst(func->funcEntry, msgBytesAst,
		  callPreInsn, orderFirstAtPoint);
    }
  }
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
    pdFunction *func;
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

metric DYNINSTallMetrics[] = {
  metric (dynMetricInfo("active_processes", SampledFunction, aggSum, "Processes"),
	  metricDefinition((createMetricFunc) createActiveProcesses, defaultPredicates)),
  metric (dynMetricInfo("observed_cost", EventCounter, aggMax, "# CPUs"),
	  metricDefinition((createMetricFunc) createObservedCost, observedCostPredicates)),
  metric (dynMetricInfo("cpu", EventCounter, aggSum, "# CPUs"),
	  metricDefinition((createMetricFunc) createCPUTime, cpuTimePredicates)),
  metric (dynMetricInfo("exec_time", EventCounter, aggSum, "%Time"),
	  metricDefinition((createMetricFunc) createExecTime, wallTimePredicates)),
  metric (dynMetricInfo("procedure_calls", EventCounter, aggSum, "Calls/sec"),
	  metricDefinition((createMetricFunc) createProcCalls, procCallsPredicates)),
  metric (dynMetricInfo( "msgs", EventCounter, aggSum, "Ops/sec"),
	  metricDefinition((createMetricFunc) createMsgs, defaultPredicates)),
  metric (dynMetricInfo( "msg_bytes", EventCounter, aggSum, "Bytes/Sec"),
	  metricDefinition((createMetricFunc) createMsgBytesTotal, defaultPredicates)),
  metric (dynMetricInfo("msg_bytes_sent", EventCounter, aggSum, "Bytes/Sec"),
	  metricDefinition((createMetricFunc) createMsgBytesSent, defaultPredicates)),
  metric (dynMetricInfo("msg_bytes_recv", EventCounter, aggSum, "Bytes/Sec"),
	  metricDefinition((createMetricFunc) createMsgBytesRecv, defaultPredicates)),
  metric (dynMetricInfo( "sync_ops", EventCounter, aggSum, "Ops/sec"),
	  metricDefinition((createMetricFunc) createSyncOps, msgPredicates)),
   metric (dynMetricInfo( "sync_wait", EventCounter, aggSum, "# Waiting"),
	   metricDefinition((createMetricFunc) createSyncWait, msgPredicates))
};

int metricCount = sizeof(DYNINSTallMetrics)/sizeof(DYNINSTallMetrics[0]);








