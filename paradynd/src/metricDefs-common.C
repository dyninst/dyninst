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

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/metricDefs-common.C,v 1.13 1995/02/16 08:53:50 markc Exp $";
#endif

/*
 * $Log: metricDefs-common.C,v $
 * Revision 1.13  1995/02/16 08:53:50  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.12  1995/02/16  08:34:05  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.11  1994/11/10  18:58:11  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.10  1994/11/09  18:40:22  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.9  1994/11/02  11:12:52  markc
 * Removed static lists and replaced them with lists initialized
 * int init-<>.C
 *
 * Rewrote module constraint handling.
 *
 * Revision 1.8  1994/09/30  19:47:10  rbi
 * Basic instrumentation for CMFortran
 *
 * Revision 1.7  1994/09/22  02:18:08  markc
 * Changed name of class function pdFunction
 *
 * Revision 1.6  1994/08/08  20:13:45  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.5  1994/08/02  18:23:43  hollings
 * changed module lists to use new lists.
 *
 * Revision 1.4  1994/07/26  20:00:28  hollings
 * removed un userd if
 *
 * Revision 1.3  1994/07/05  03:26:13  hollings
 * observed cost model
 *
 * Revision 1.2  1994/07/01  22:14:17  markc
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

#include "symtab.h"
#include "process.h"
#include "rtinst/h/rtinst.h"
#include "inst.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "util.h"
#include "rtinst/h/trace.h"
#include "metricDef.h"
#include "os.h"

// this flag is checked on writes to determine how to use the cost
static dictionary_hash<int, dataReqNode*> msgFlags(intHash);

//
// input: 
//    constraint = "<module name>[/<procedure name>]"
//
// output:
//    returns true when successful
//    mod = the module pointer
//    func = the pdFunction pointer if the constraint is for functions
//    func = NULL if the constraint is for a module only
// 
// given a constraint, finds the module and function from the image
//
// It is not an error if the module or function are not found
//
bool getModuleAndFunction(char *constraint, image *im, module *&mod, pdFunction *&func) {
  mod = NULL; func = NULL;
  bool byFunc = false;
  string funcCons, modCons;
  char *buffer, *funcName;

  assert(constraint);
  buffer = new char[strlen(constraint)];
  strcpy(buffer, constraint);

  // is this <module>/<procedure> or just <module>

  if (funcName = strchr(buffer, '/')) {
    // split the string
    *funcName = 0;
    funcName++;
    funcCons = funcName;
    byFunc = true;
  }
  modCons = buffer;

  // find the module
  mod = im->findModule(modCons);
  if (!mod) {
    // no such module 
    // ostrstream os(errorLine, 1024, ios::out);
    // os << "Module " << modCons << " not found \n";
    // logLine(errorLine);
    delete buffer;
    return false;
  }

  // find the function, if it had been specified
  if (byFunc) {
    // do one function in the module
    func = mod->findFunction(funcCons);
    if (!func) {
      // ostrstream os(errorLine, 1024, ios::out);
      // os << "Module " << modCons << " not found \n";
      // logLine(errorLine);
      mod = NULL;
      delete buffer;
      return false;
    }
  }
  delete buffer;
  return true;
}

// The functions may have "siblings" in which case this should be
// called more than once
void createDefaultFuncPred(metricDefinitionNode *mn,
			   pdFunction *func, 
			   dataReqNode *dataPtr, 
			   AstNode *pred)
{
    int i;
    AstNode *enterNode, *leaveNode;

    enterNode = createPrimitiveCall("setCounter", dataPtr, 1);
    leaveNode = createPrimitiveCall("setCounter", dataPtr, 0);
    assert(enterNode);
    assert(leaveNode);
    if (pred) {
	enterNode = createIf(pred, enterNode);
	leaveNode = createIf(pred, leaveNode);
    }
    assert(enterNode);
    assert(leaveNode);
    for (i = 0; i < func->calls.size(); i++) {
      if (callsTrackedFuncP(func->calls[i])) {
	mn->addInst(func->calls[i], leaveNode,
		    callPreInsn, orderLastAtPoint);
	mn->addInst(func->calls[i], enterNode,
		    callPostInsn, orderFirstAtPoint);
      }
    }
    mn->addInst(func->funcEntry(), enterNode, callPreInsn, orderLastAtPoint);
    mn->addInst(func->funcReturn(), leaveNode, callPreInsn,orderFirstAtPoint);
}

// TODO - is this needed?
// defaultModulePredicate seems to have removed the need for this - mdc
#ifdef notdef
void defaultProcedurePredicate(metricDefinitionNode *mn,
			       pdFunction *func,
			       AstNode *pred)
{
    dataReqNode *dataPtr;

    vector<pdFunction*> a;

    if (!((mn->proc->symbols)->findFunction(funcName, a))) {
      /* no such function in this process */
      // 0 predicate if always false.
      return(new AstNode(Constant, 0));
    }

    dataPtr = mn->addIntCounter(0, false);

    int i;
    for (i=0; i<a.size(); i++)
      createDefaultFuncPred(mn, a[i], dataPtr, pred);

    // return(new AstNode(DataValue, dataPtr));
}
#endif

AstNode *defaultModulePredicate(metricDefinitionNode *mn,
				char *constraint,
				AstNode *pred)
{
    module *mod;
    pdFunction *func;
    dataReqNode *dataPtr;


    if (!getModuleAndFunction(constraint, mn->proc->symbols, mod, func)) {
      // logLine("In default module predicate, module/function lookup failed\n");
      // assert(0);
      return (new AstNode(Constant, 0));
    }

    dataPtr = mn->addIntCounter(0, false);
    assert(dataPtr);

    if (func) {
      // function in a module
      createDefaultFuncPred(mn, func, dataPtr, pred);
    } else {
      // module only
      // TODO make this private
      dictionary_hash_iter<string, pdFunction*> mi(mod->funcMap);
      string pds; pdFunction *pdf;
      while (mi.next(pds, pdf))
	createDefaultFuncPred(mn, pdf, dataPtr, pred);

    }
    return (new AstNode(DataValue, dataPtr));
}

AstNode *defaultProcessPredicate(metricDefinitionNode *mn, char *process,
				 AstNode *pred) {
  abort();
  return NULL;    // get rid of compiler warning
}


void createProcCalls(metricDefinitionNode *mn, AstNode *pred)
{

    AstNode *newCall;
    dataReqNode *counter;

    counter = mn->addIntCounter(0, true);
    assert(counter);
    newCall = createPrimitiveCall("addCounter", counter, 1);
    assert(newCall);
    dictionary_hash_iter<unsigned, pdFunction*> fi(mn->proc->symbols->funcsByAddr);
    unsigned u; pdFunction *pdf;
    while (fi.next(u, pdf)) {
      if (!pdf->isLibTag()) {
	mn->addInst(pdf->funcEntry(), newCall, callPreInsn, orderLastAtPoint);
      }
    }
}

void instAllFunctions(metricDefinitionNode *nm,
		      unsigned tag,		/* bit mask to use */
		      AstNode *enterAst,
		      AstNode *leaveAst)
{
  pdFunction *func; unsigned u;
  dictionary_hash_iter<unsigned, pdFunction*> fi(nm->proc->symbols->funcsByAddr);
    
  while (fi.next(u, func)) {
    if (func->isTagSimilar(tag)) {
      if (enterAst)
	nm->addInst(func->funcEntry(), enterAst, callPreInsn, orderLastAtPoint);
      if (leaveAst)
	nm->addInst(func->funcReturn(), leaveAst, callPreInsn, orderFirstAtPoint);
    }
  }
}

void createObservedCost(metricDefinitionNode *mn, AstNode *pred)
{
    pdFunction *sampler;
    AstNode *reportNode;
    dataReqNode *dataPtr;

    dataPtr = mn->addIntCounter(0, false);
    assert(dataPtr);

    sampler = (mn->proc->symbols)->findOneFunction("DYNINSTsampleValues");
    assert(sampler);
    reportNode = new AstNode("DYNINSTreportCost", 
		 new AstNode(DataPtr, dataPtr), new AstNode(Constant, 0));
    assert(reportNode);
    mn->addInst(sampler->funcEntry(), reportNode, callPreInsn, orderLastAtPoint);
}


void createExecTime(metricDefinitionNode *mn, AstNode *pred)
{
    pdFunction *func;
    dataReqNode *dataPtr;
    AstNode *startNode, *stopNode;

    dataPtr = mn->addTimer(wallTime);
    assert(dataPtr);
    startNode = createPrimitiveCall("DYNINSTstartWallTimer", dataPtr, 0);
    assert(startNode);
    if (pred) startNode = createIf(pred, startNode);
    assert(startNode);

    stopNode = createPrimitiveCall("DYNINSTstopWallTimer", dataPtr, 0);
    assert(stopNode);
    if (pred) stopNode = createIf(pred, stopNode);
    assert(stopNode);

    func = (mn->proc->symbols)->findOneFunction("main");
    assert(func);
    mn->addInst(func->funcEntry(), startNode, callPreInsn, orderLastAtPoint);

    mn->addInst(func->funcReturn(), stopNode, callPreInsn, orderLastAtPoint);

    func = (mn->proc->symbols)->findOneFunction(EXIT_NAME);
    assert(func);

    mn->addInst(func->funcEntry(), stopNode, callPreInsn, orderLastAtPoint);
}

void createSyncOps(metricDefinitionNode *mn, AstNode *trigger)
{
    AstNode *newSyncOp;
    dataReqNode *counter;
    
    counter = mn->addIntCounter(0, true);
    assert(counter);
    newSyncOp = createPrimitiveCall("addCounter", counter, 1);
    assert(newSyncOp);
    if (trigger) newSyncOp = createIf(trigger, newSyncOp);
    assert(newSyncOp);

    instAllFunctions(mn, TAG_CPU_STATE, newSyncOp, NULL);
}

void createMsgs(metricDefinitionNode *mn, AstNode *trigger)
{
    AstNode *newMsgOp;
    dataReqNode *counter;
    
    counter = mn->addIntCounter(0, true);
    assert(counter);
    newMsgOp = createPrimitiveCall("addCounter", counter, 1);
    assert(newMsgOp);
    if (trigger) newMsgOp = createIf(trigger, newMsgOp);
    assert(newMsgOp);
    instAllFunctions(mn, TAG_MSG_SEND | TAG_MSG_RECV, newMsgOp, NULL);
}

//
// place holder for pause time metric.
//
void dummyCreate(metricDefinitionNode *mn, AstNode *trigger) {  }


void perProcedureWallTime(metricDefinitionNode *mn, 
			  pdFunction *func,
			  AstNode *pred,
			  dataReqNode *dataPtr)
{
    int i;

    AstNode *startNode, *stopNode;

    if (!dataPtr) dataPtr = mn->addTimer(wallTime);
    assert(dataPtr);

    startNode = createPrimitiveCall("DYNINSTstartWallTimer", dataPtr, 0);
    assert(startNode);
    if (pred) startNode = createIf(pred, startNode);
    assert(startNode);
    stopNode = createPrimitiveCall("DYNINSTstopWallTimer", dataPtr, 0);
    assert(stopNode);
    if (pred) stopNode = createIf(pred, stopNode);
    assert(stopNode);
    for (i = 0; i < func->calls.size(); i++) {
      if (callsTrackedFuncP(func->calls[i])) {
	mn->addInst(func->calls[i], stopNode,
		    callPreInsn, orderLastAtPoint);
	
	mn->addInst(func->calls[i], startNode,
		    callPostInsn, orderFirstAtPoint);
      }
    }
    mn->addInst(func->funcEntry(), startNode, callPreInsn, orderLastAtPoint);
    mn->addInst(func->funcReturn(), stopNode, callPreInsn, orderFirstAtPoint);
}

AstNode *perModuleWallTime(metricDefinitionNode *mn, 
			   char *constraint, 
			   AstNode *trigger)
{
  module *mod;
  pdFunction *func;
  dataReqNode *result;

  if (!getModuleAndFunction(constraint, mn->proc->symbols, mod, func)) {
    // logLine("In default module predicate, module/function lookup failed\n");
    // assert(0);
    return NULL;
  }

  result = mn->addTimer(wallTime);
  assert(result);

  if (func) {
    // do one function in one module
    perProcedureWallTime(mn, func, trigger, result);
    return NULL;
  } else {
    // do all the functions in this module
    dictionary_hash_iter<string, pdFunction*> fi(mod->funcMap);
    string pds; pdFunction *pdf;
    while (fi.next(pds, pdf)) 
      perProcedureWallTime(mn, pdf, trigger, result);
    return NULL;
  }
}

// this function should be used for replaceBase
// it always returns NULL
AstNode *perProcedureCPUTime(metricDefinitionNode *mn, 
			     pdFunction *func,
			     AstNode *trigger,
			     dataReqNode *dataPtr)
{

    int i;
    AstNode *startNode, *stopNode;

    if (!dataPtr) dataPtr = mn->addTimer(processTime);
    assert(dataPtr);
    startNode = new AstNode("DYNINSTstartProcessTimer", 
	new AstNode(DataValue, dataPtr), NULL);
    assert(startNode);
    if (trigger) startNode = createIf(trigger, startNode);
    assert(startNode);
    stopNode = new AstNode("DYNINSTstopProcessTimer", 
	new AstNode(DataValue, dataPtr), NULL);
    assert(stopNode);
    if (trigger) stopNode = createIf(trigger, stopNode);
    assert(stopNode);
    for (i = 0; i < func->calls.size(); i++) {
      if (callsTrackedFuncP(func->calls[i])) {
	mn->addInst(func->calls[i], stopNode,
		    callPreInsn, orderFirstAtPoint);
	
	mn->addInst(func->calls[i], startNode,
		    callPostInsn, orderFirstAtPoint);
      }
    }
    mn->addInst(func->funcEntry(), startNode, callPreInsn, orderLastAtPoint);

    mn->addInst(func->funcReturn(), stopNode, callPreInsn, orderFirstAtPoint);
    return NULL;
}

// this function should be used for replaceBase
// it always returns NULL
AstNode *perModuleCPUTime(metricDefinitionNode *mn, 
			  char *constraint, 
			  AstNode *trigger)
{
  module *mod;
  pdFunction *func;
  dataReqNode *result;

  if (!getModuleAndFunction(constraint, mn->proc->symbols, mod, func)) {
    // logLine("In default module predicate, module/function lookup failed\n");
    // assert(0);
    return NULL;
  }

  result = mn->addTimer(processTime);
  assert(result);

  if (func) {
    // do one function in one module
    perProcedureCPUTime(mn, func, trigger, result);
    return NULL;
  } else {
    // do all the functions in this module
    dictionary_hash_iter<string, pdFunction*> fi(mod->funcMap);
    string pds; pdFunction *pdf;
    while (fi.next(pds, pdf)) 
      perProcedureCPUTime(mn, pdf, trigger, result);
    return NULL;
  }
}

// this function should be used for replaceBase
// it always returns NULL
AstNode *perProcedureCalls(metricDefinitionNode *mn, 
			   pdFunction *func,
			   AstNode *trigger,
			   dataReqNode *counter)
{
    AstNode *newCall;

    assert(counter);

    newCall = createPrimitiveCall("addCounter", counter, 1);
    assert(newCall);
    if (trigger) newCall = createIf(trigger, newCall);
    assert(newCall);
    mn->addInst(func->funcEntry(), newCall, callPreInsn, orderLastAtPoint);
    return NULL;
}

// this function should be used for replaceBase
// it always returns NULL
AstNode *perModuleCalls(metricDefinitionNode *mn, 
			char *constraint, 
			AstNode *trigger)
{
  module *mod;
  pdFunction *func;
  dataReqNode *counter;

  if (!getModuleAndFunction(constraint, mn->proc->symbols, mod, func)) {
    // logLine("In default module predicate, module/function lookup failed\n");
    // assert(0);
    return NULL;
  }

  counter = mn->addIntCounter(0, true);
  assert(counter);

  if (func) {
    // do one function in one module
    perProcedureCalls(mn, func, trigger, counter);
    return NULL;
  } else {
    // do all the functions in this module
    dictionary_hash_iter<string, pdFunction*> fi(mod->funcMap);
    string pds; pdFunction *pdf;
    while (fi.next(pds, pdf)) 
      perProcedureCalls(mn, pdf, trigger, counter);
    return NULL;
  }
}


