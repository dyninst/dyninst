/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) /p/paradyn/CVSROOT/core/paradynd/src/metricDefs-cm5.C,v 1.19 1995/05/18 10:39:29 markc Exp";
#endif

/*
 * metric.C - define and create metrics.
 *
 * $Log: metricDefs-cm5.C,v $
 * Revision 1.21  1995/10/24 03:37:56  tamches
 * Commented out createSyncWait, as part of removing metricDefs-common.C
 *
 * Revision 1.20  1995/08/24 15:04:22  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.19  1995/05/18  10:39:29  markc
 * These are no longer needed
 *
 * Revision 1.18  1995/02/16  08:53:49  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.17  1995/02/16  08:33:59  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.16  1994/11/11  05:12:31  markc
 * Turned off writing to cout when message metrics are considered.  This is
 * not a good thing to do if the underlying file descriptor is not there.
 *
 * Revision 1.15  1994/11/09  18:40:20  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.14  1994/11/02  11:11:47  markc
 * Removed compiler warnings.
 *
 * Revision 1.13  1994/09/22  02:17:26  markc
 * Added static class initializers for DYNINSTallMetrics
 *
 * Revision 1.12  1994/07/12  19:29:48  jcargill
 * Changed argument offsets for msgBytes, msgTags; search /Procedure first
 *
 * Revision 1.11  1994/07/05  03:26:12  hollings
 * observed cost model
 *
 * Revision 1.10  1994/07/02  01:46:43  markc
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

#include "symtab.h"
#include "process.h"
#include "rtinst/h/rtinst.h"
#include "inst.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "rtinst/h/trace.h"
#include "metricDef.h"

//// A process timer is used because it does not stop on blocking
//// system calls.
//void createSyncWait(metricDefinitionNode *mn, AstNode *trigger)
//{
//    dataReqNode *dataPtr;
//    AstNode *stopNode, *startNode;
//
//    dataPtr = mn->addTimer(processTime);
//
//    startNode = new AstNode("DYNINSTstartProcessTimer", 
//	new AstNode(DataValue, dataPtr), NULL);
//    if (trigger) startNode = createIf(trigger, startNode);
//
//    stopNode = new AstNode("DYNINSTstopProcessTimer", 
//	new AstNode(DataValue, dataPtr), NULL);
//    if (trigger) stopNode = createIf(trigger, stopNode);
//
//    instAllFunctions(mn, TAG_MSG_FILT, startNode, stopNode);
//}

//
// ***** Warning this metric is CM-5 specific. *****
//
void createMsgBytesMetric(metricDefinitionNode *mn,
			  unsigned matchTag,
			  AstNode *trigger)
{
    pdFunction *func;
    AstNode *msgBytesAst;
    dataReqNode *dataPtr;

    dataPtr = mn->addIntCounter(0, true);

    // addCounter(counter, param4 * param5)
    msgBytesAst = new AstNode("addCounter", 
	    new AstNode(DataValue, dataPtr),
	    new AstNode(Param, (void *) 3));

// WAS:	    new AstNode(timesOp, new AstNode(Param, (void *) 3), 
//				 new AstNode(Param, (void *) 4)));

    if (trigger) msgBytesAst = createIf(trigger, msgBytesAst);

    dictionary_hash_iter<unsigned, pdFunction*> fi((mn->proc())->symbols->funcsByAddr);
    unsigned u;

    while (fi.next(u, func)) {
      if (func->isTagSimilar(matchTag))
	mn->addInst(func->funcEntry(), msgBytesAst, callPreInsn, orderLastAtPoint);
    }
}

void createMsgBytesTotal(metricDefinitionNode *mn, AstNode *tr)
{
    createMsgBytesMetric(mn, TAG_MSG_SEND | TAG_MSG_RECV, tr);
}

void createMsgBytesSent(metricDefinitionNode *mn, AstNode *tr)
{
    createMsgBytesMetric(mn, TAG_MSG_SEND, tr);
}

void createMsgBytesRecv(metricDefinitionNode *mn, AstNode *tr)
{
    createMsgBytesMetric(mn, TAG_MSG_RECV, tr);
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

    // (== param2, iTag)
    tagTest = new AstNode(eqOp, new AstNode(Param, (void *) 1),
				new AstNode(Constant, (void *) iTag));

    filterNode = createIf(tagTest, createPrimitiveCall("addCounter", data, 1));
    if (trigger) filterNode = createIf(trigger, filterNode);

    clearNode = createPrimitiveCall("setCounter", data, 0);
    if (trigger) clearNode = createIf(trigger, clearNode);

    dictionary_hash_iter<unsigned, pdFunction*> fi((mn->proc())->symbols->funcsByAddr);
    unsigned u;

    while (fi.next(u, func)) {
      if (func->isTagSimilar(TAG_MSG_FILT)) {
	mn->addInst(func->funcEntry(), filterNode,
		    callPreInsn, orderFirstAtPoint);
	mn->addInst(func->funcReturn(), clearNode,
		    callPreInsn, orderLastAtPoint);
      }
    }
    return(new AstNode(DataValue, data));
}

#ifdef notdef
void createCPUTime(metricDefinitionNode *mn, AstNode *pred)
{
    dataReqNode *dataPtr = mn->addTimer(processTime);
    assert(dataPtr);

    AstNode *startNode = new AstNode("DYNINSTstartProcessTimer", 
				     new AstNode(DataValue, dataPtr), NULL);
    assert(startNode);

    if (pred) startNode = createIf(pred, startNode);
    assert(startNode);
    AstNode *stopNode = new AstNode("DYNINSTstopProcessTimer", 
				    new AstNode(DataValue, dataPtr), NULL);

    assert(stopNode);
    if (pred) stopNode = createIf(pred, stopNode);
    assert(stopNode);
    instAllFunctions(mn, TAG_CPU_STATE, stopNode, startNode);

    pdFunction *func = ((mn->proc())->symbols)->findOneFunction("main");
    assert(func);

    mn->addInst(func->funcEntry(), startNode,callPreInsn,orderLastAtPoint);

    mn->addInst(func->funcReturn(), stopNode,callPreInsn,orderLastAtPoint);

    func = ((mn->proc())->symbols)->findOneFunction(EXIT_NAME);
    assert(func);

    mn->addInst(func->funcEntry(), stopNode, callPreInsn,orderLastAtPoint);

    // TODO - why is this in a common file -> CM is a CM-5 function
    func = ((mn->proc())->symbols)->findOneFunction("CMNA_dispatch_idle"); 
    if (func) {
      mn->addInst(func->funcReturn(), startNode, callPreInsn,orderLastAtPoint); 
      mn->addInst(func->funcEntry(), stopNode, callPreInsn,orderLastAtPoint); 
    } 
}
#endif
