/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/metricDefs-pvm.C,v 1.22 1995/10/24 03:39:22 tamches Exp $";
#endif

/*
 * metric.C - define and create metrics.
 *
 * $Log: metricDefs-pvm.C,v $
 * Revision 1.22  1995/10/24 03:39:22  tamches
 * Commented out createCPUTime and createSyncWait, as part of removing
 * the obsolete metricDefs-common.C
 *
 * Revision 1.21  1995/05/18 10:39:33  markc
 * These are no longer needed
 *
 * Revision 1.20  1995/02/26  22:47:39  markc
 * Upgraded to compile using new interfaces.  Many public data members became private.
 *
 * Revision 1.19  1995/02/16  08:34:10  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.18  1995/01/30  17:32:15  jcargill
 * changes for gcc-2.6.3; intCounter was both a typedef and an enum constant
 *
 * Revision 1.17  1994/11/10  18:58:13  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.16  1994/11/09  18:40:27  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.15  1994/11/02  11:12:50  markc
 * Removed static lists and replaced them with lists initialized
 * int init-<>.C
 *
 * Rewrote module constraint handling.
 *
 * Revision 1.14  1994/09/22  02:18:57  markc
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "symtab.h"
#include "process.h"
#include "rtinst/h/rtinst.h"
#include "inst.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "rtinst/h/trace.h"
#include "metricDef.h"
#include "os.h"

// Allows write to be multiplexed.  If write is reached when this counter is 0,
// then the write will be counted as a write.  The flag must be global, since
// the io functions need to know its location.
void osDependentInst(process *proc) {
  return;
#ifdef notdef
  // this flag is checked on writes to determine how to use the cost
  static dictionary_hash<int, dataReqNode*> msgFlags(intHash);
  // This is code that needs more testing
  assert(!msgFlags.defines(proc->getPid()));

  dataReqNode *msgFlag = new dataReqNode(INTCOUNTER, proc, 0, false, processTime);
  msgFlag->insertGlobal();
  msgFlags[proc->getPid()] = msgFlag;

  AstNode *setNode = createPrimitiveCall("setCounter", msgFlag, 1);
  AstNode *unsetNode = createPrimitiveCall("setCounter", msgFlag, 0);
  dictionary_hash_iter<unsigned, pdFunction*> fi(proc->symbols->funcsByAddr);
  unsigned u; pdFunction *func;

  while (fi.next(u, func)) {
    if (func->isTagSimilar(TAG_MSG_SEND | TAG_MSG_RECV)) {
      addInstFunc(proc, func->funcEntry(), setNode, callPreInsn, orderLastAtPoint);
      addInstFunc(proc, func->funcReturn(), unsetNode, callPreInsn, orderFirstAtPoint);
    }
  }
#endif
}
 
// A wall timer is used because the process timer will be stopped
// on blocking system calls.

//void createSyncWait(metricDefinitionNode *mn, AstNode *trigger)
//{
//    dataReqNode *dataPtr;
//    AstNode *stopNode, *startNode;
//
//    dataPtr = mn->addTimer(wallTime);
//
//    startNode = new AstNode("DYNINSTstartWallTimer", 
//	new AstNode(DataValue, dataPtr), NULL);
//    if (trigger) startNode = createIf(trigger, startNode);
//
//    stopNode = new AstNode("DYNINSTstopWallTimer", 
//	new AstNode(DataValue, dataPtr), NULL);
//    if (trigger) stopNode = createIf(trigger, stopNode);
//
//    instAllFunctions(mn, TAG_MSG_SEND | TAG_MSG_RECV, startNode, stopNode);
//}

//
// ***** Warning this metric is pvm specific. *****
//
void createMsgBytesRecvMetric(metricDefinitionNode *mn,
			      unsigned matchTag,
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

  dictionary_hash_iter<unsigned, pdFunction*> fi((mn->proc())->symbols->funcsByAddr);
  unsigned u;

  while (fi.next(u, func)) {
    if (func->isTagSimilar(matchTag)) 
      mn->addInst(func->funcReturn(), msgBytesAst,
		  callPreInsn, orderFirstAtPoint);
  }
}

//
// ***** Warning this metric is pvm specific. *****
//
void createMsgBytesSentMetric(metricDefinitionNode *mn,
			      unsigned matchTag,
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

  dictionary_hash_iter<unsigned, pdFunction*> fi((mn->proc())->symbols->funcsByAddr);
  unsigned u;

  while (fi.next(u, func)) {
    if (func->isTagSimilar(matchTag))
      mn->addInst(func->funcEntry(), msgBytesAst,
		  callPreInsn, orderFirstAtPoint);
  }
}

void createMsgBytesSent(metricDefinitionNode *mn, AstNode *tr)
{
  dataReqNode *dataPtr, *tempCounter;

  dataPtr = mn->addIntCounter(0, true);
  tempCounter = mn->addIntCounter(0, false);

  createMsgBytesSentMetric (mn, TAG_MSG_SEND, tr, dataPtr, tempCounter);
}

void createMsgBytesRecv(metricDefinitionNode *mn, AstNode *tr)
{
  dataReqNode *dataPtr, *tempCounter;

  dataPtr = mn->addIntCounter(0, true);
  tempCounter = mn->addIntCounter(0, false);

  createMsgBytesRecvMetric (mn, TAG_MSG_RECV, tr, dataPtr, tempCounter);
}

// provide different send and receive metric funcs since the call to determine
// the message buffer is different for send and receive
void createMsgBytesTotal(metricDefinitionNode *mn, AstNode *tr)
{
  // these will be shared by both the send and receive 
  dataReqNode *dataPtr, *tempCounter;

  dataPtr = mn->addIntCounter(0, true);
  tempCounter = mn->addIntCounter(0, false);

  createMsgBytesRecvMetric (mn, TAG_MSG_RECV, tr, dataPtr, tempCounter);
  createMsgBytesSentMetric (mn, TAG_MSG_SEND, tr, dataPtr, tempCounter);
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

    data = mn->addIntCounter(0, false);

    // NOTE - this is 1 since paramters are numbered starting with
    // 0.
    tagTest = new AstNode(eqOp, new AstNode(Param, (void *) 1),
				new AstNode(Constant, (void *) iTag));

    filterNode = createIf(tagTest, createPrimitiveCall("addCounter", data, 1));
    if (trigger) filterNode = createIf(trigger, filterNode);

    clearNode = createPrimitiveCall("setCounter", data, 0);
    if (trigger) clearNode = createIf(trigger, clearNode);

    dictionary_hash_iter<unsigned, pdFunction*> fi((mn->proc())->symbols->funcsByAddr);
    unsigned u;

    while (fi.next(u, func)) {
      if (func->isTagSimilar(TAG_MSG_SEND | TAG_MSG_RECV)) {
	mn->addInst(func->funcEntry(), filterNode,
		    callPreInsn, orderFirstAtPoint);
	mn->addInst(func->funcReturn(), clearNode,
		    callPreInsn, orderLastAtPoint);
      }
    }
    return(new AstNode(DataValue, data));
}

//void createCPUTime(metricDefinitionNode *mn, AstNode *pred)
//{
//    pdFunction *func;
//    dataReqNode *dataPtr;
//    AstNode *stopNode, *startNode;
//
//    dataPtr = mn->addTimer(processTime);
//    assert(dataPtr);
//
//    startNode = new AstNode("DYNINSTstartProcessTimer", 
//	new AstNode(DataValue, dataPtr), NULL);
//    assert(startNode);
//    if (pred) startNode = createIf(pred, startNode);
//    assert(startNode);
//    stopNode = new AstNode("DYNINSTstopProcessTimer", 
//	new AstNode(DataValue, dataPtr), NULL);
//    assert(stopNode);
//    if (pred) stopNode = createIf(pred, stopNode);
//    assert(stopNode);
//
//    instAllFunctions(mn, TAG_CPU_STATE, stopNode, startNode);
//
//    func = ((mn->proc())->symbols)->findOneFunction("main");
//    assert(func);
//    mn->addInst(func->funcEntry(), startNode,callPreInsn,orderLastAtPoint);
//
//    mn->addInst(func->funcReturn(), stopNode,callPreInsn,orderLastAtPoint);
//
//    func = ((mn->proc())->symbols)->findOneFunction(EXIT_NAME);
//    assert(func);
//
//    mn->addInst(func->funcEntry(), stopNode, callPreInsn,orderLastAtPoint);
//}






