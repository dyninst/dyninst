/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#include "paradynd/src/instReqNode.h"
#include "dyninstAPI/src/ast.h"
#include "common/h/timing.h"
#include "dyninstAPI/src/process.h"
#include "pdutil/h/pdDebugOstream.h"

extern pdDebug_ostream metric_cerr;


/*
 * functions to operate on inst request graph.
 *
 */
instReqNode::instReqNode(instPoint *iPoint, AstNode *iAst, callWhen iWhen,
                         callOrder o) {
  point = iPoint;
  when = iWhen;
  order = o;
  instance = NULL; // set when loadInstrIntoApp() calls addInstFunc()
  ast = assignAst(iAst);
  assert(point);
}

instReqNode instReqNode::forkProcess(const instReqNode &parentNode,
		      const dictionary_hash<instInstance*,instInstance*> &map)
{
  instReqNode ret = instReqNode(parentNode.point, parentNode.ast, 
				parentNode.when, parentNode.order);
  
  if (!map.find(parentNode.instance, ret.instance)) // writes to ret.instance
    assert(false);
  
  return ret;
}

bool instReqNode::unFork(dictionary_hash<instInstance*,instInstance*> &map) 
  const {
  // The fork syscall duplicates all trampolines from the parent into the
  // child. For those mi's which we don't want to propagate to the child,
  // this creates a problem.  We need to remove instrumentation code from the
  // child.  This routine does that.
  //
  // "this" represents an instReqNode in the PARENT process.  "map" maps all
  // instInstance*'s of the parent process to instInstance*'s in the child
  // process.  We modify "map" by setting a value to NULL.

  instInstance *parentInstance = getInstance();
   
  instInstance *childInstance;
  if (!map.find(parentInstance, childInstance)) // writes to childInstance
    assert(false);
  
  vector<Address> pointsToCheck; // is it right leaving this empty on a fork()???
  deleteInst(childInstance, pointsToCheck);
  
  map[parentInstance] = NULL; // since we've deleted...
  
  return true; // success
}

instInstance * instReqNode::loadInstrIntoApp(process *theProc,
					     returnInstance *&retInstance,
					     bool *deferred)
{
  // NEW: We may manually trigger the instrumentation, via a call to
  // postRPCtoDo()
  
  // addInstFunc() is one of the key routines in all paradynd.  It installs a
  // base tramp at the point (if needed), generates code for the tramp, calls
  // inferiorMalloc() in the text heap to get space for it, and actually
  // inserts the instrumentation.
  instance = addInstFunc(theProc, point, ast, when, order,
			 false, // false --> don't exclude cost
			 retInstance,
			 *deferred,
			 false // false --> do not allow recursion
			 );
  
  //if( !retInstance )
  //cerr << "addInstFunc returned a NULL retInstance" << endl;
  rinstance = retInstance;
  
  return instance;
}

void instReqNode::disable(const vector<Address> &pointsToCheck)
{
  //cerr << "in instReqNode::disable - " << pointsToCheck.size()
  //     << " points to check\n";
#if defined(MT_THREAD)
  if (instance) 
    deleteInst(instance, pointsToCheck);
#else
  deleteInst(instance, pointsToCheck);
#endif
  instance = NULL;
}

instReqNode::~instReqNode()
{
  instance = NULL;
  removeAst(ast);
}

timeLength instReqNode::cost(process *theProc) const
{
  // Currently the predicted cost represents the maximum possible cost of the
  // snippet.  For instance, for the statement "if(cond) <stmtA> ..."  the
  // cost of <stmtA> is currently included, even if it's actually not called.
  // Feel free to change the maxCost call below to ast->minCost or
  // ast->avgCost if the semantics need to be changed.
  int unitCostInCycles = ast->maxCost() + getPointCost(theProc, point) +
                       getInsnCost(trampPreamble) + getInsnCost(trampTrailer);
  timeLength unitCost(unitCostInCycles, getCyclesPerSecond());
  float frequency = getPointFrequency(point);
  timeLength value = unitCost * frequency;
  return(value);
}

extern void checkProcStatus();

bool instReqNode::triggerNow(process *theProc, int mid) {
   bool needToCont = theProc->status() == running;
#ifdef DETACH_ON_THE_FLY
   if ( !theProc->reattachAndPause() )
#else
   if ( !theProc->pause() )
#endif
   {
      cerr << "instReqNode::triggerNow -- pause failed" << endl;
      return false;
   }
   
   // If multithreaded, we need to trigger for each thread necessary
   for (unsigned i = 0; i < fries.size(); i++) {
#if defined(MT_THREAD)
      int thrID = fries[i];
      /*
	fprintf(stderr, "Starting instrumentation for thread %d\n", thrID);
      */
      // trigger the instrumentation
      
      for (unsigned i=0;i<manuallyTriggerTIDs.size();i++) {
	 if (manuallyTriggerTIDs[i]==thrID) {
	    continue;
	 }
      }
      // Avoid multiply triggering, since the thread IDs stored in "fries"
      // may not be unique
      manuallyTriggerTIDs += thrID;
      
#if defined(TEST_DEL_DEBUG)
      sprintf(errorLine,"***** posting inferiorRPC for mid=%d and tid=%d\n",mid,thrId);
      logLine(errorLine);
#endif
#endif
      
      theProc->postRPCtoDo(ast, false, // don't skip cost
			   instReqNode::triggerNowCallbackDispatch, this,
#if defined(MT_THREAD)
			   mid,
			   thrID,
			   false
#else
			   mid
#endif
			   ); //false --> regular RPC, true-->SAFE RPC
      metric_cerr << "   inferiorRPC has been launched for this thread. " << endl;
      rpcCount = 0;
      
      if (pd_debug_catchup) {
	 cerr << "launched catchup instrumentation, waiting rpc to finish ..." << endl;
      }
      // Launch RPC now since we're playing around with timers and such.
      // We should really examine the stack to be sure that it's safe, but
      // for now assume it is.
      do { 
	 // Make sure that we are not currently in an RPC to avoid race
	 // conditions between catchup instrumentation and waitProcs()
	 // loops
	 if ( !theProc->isRPCwaitingForSysCallToComplete() ) 
	    theProc->launchRPCifAppropriate(false, false); 
	 checkProcStatus(); 
      } while ( !rpcCount && theProc->status() != exited );
      if ( pd_debug_catchup ) {
	 metric_cerr << "catchup instrumentation finished ..." << endl;
      }
   }     
   if( needToCont && (theProc->status() != running)) {
#ifdef DETACH_ON_THE_FLY
      theProc->detachAndContinue();
#else
      theProc->continueProc();
#endif
   }
   else if ( !needToCont && theProc->status()==running ) {
#ifdef DETACH_ON_THE_FLY
      theProc->reattachAndPause();
#else
      theProc->pause();
#endif
   }
   return true;
}

void instReqNode::triggerNowCallback(void * /*returnValue*/ ) {
   ++rpcCount;
}


bool instReqNode::triggeredInStackFrame(pd_Function *stack_fn, Address pc,
					process *p)
{
   return p->triggeredInStackFrame(point, stack_fn, pc, when, order);
}



