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
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/ast.h"
#include "common/h/timing.h"
#include "paradynd/src/pd_process.h"
#include "pdutil/h/pdDebugOstream.h"
#include "dyninstAPI/src/instPoint.h"

extern unsigned enable_pd_metric_debug;

#if ENABLE_DEBUG_CERR == 1
#define metric_cerr if (enable_pd_metric_debug) cerr
#else
#define metric_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

const int MAX_INSERTION_ATTEMPTS_USING_RELOCATION = 1000;

/*
 * functions to operate on inst request graph.
 *
 */

// special copy constructor used for fork handling
instReqNode::instReqNode(const instReqNode &par, pd_process *childProc) : 
   point(par.point), ast(assignAst(par.ast)), when(par.when), 
   order(par.order), loadedIntoApp_(par.loadedIntoApp_), 
   trampsHookedUp_(par.trampsHookedUp_), 
   rinstance(par.rinstance), rpcCount(par.rpcCount), 
   loadInstAttempts(par.loadInstAttempts)
{
    if(!par.instrLoaded() || !par.trampsHookedUp()) {
        // can't setup the child if the parent isn't setup
        return;
    }
    
    process *llproc = childProc->get_dyn_process()->lowlevel_process();
    bool res = getInheritedMiniTramp(par.mtHandle, mtHandle, llproc);
	assert(res == true);
}


// returns false if instr insert was deferred
loadMiniTramp_result instReqNode::loadInstrIntoApp(pd_process *theProc,
					      returnInstance *&retInstance) {
   if(loadedIntoApp_) return success_res;
      
   ++loadInstAttempts;
   if(loadInstAttempts == MAX_INSERTION_ATTEMPTS_USING_RELOCATION) {
      pd_Function *function_not_inserted = point->pointFunc();

      if(function_not_inserted != NULL)
         function_not_inserted->markAsNeedingRelocation(false);
   }
   
   // NEW: We may manually trigger the instrumentation, via a call to
   // postRPCtoDo()
   
   bool trampRecursiveDesired = false;

   // --------------------------------------------------------------
   // can remove this once implement MT tramp guards on linux
#if defined(os_linux)
   // ignore tramp guards on MT Linux until it is implemented
   if(theProc->multithread_capable())
      trampRecursiveDesired = true;
#endif
   // --------------------------------------------------------------

   // addInstFunc() is one of the key routines in all paradynd.  It installs a
   // base tramp at the point (if needed), generates code for the tramp, calls
   // inferiorMalloc() in the text heap to get space for it, and actually
   // inserts the instrumentation.
   loadMiniTramp_result res = 
      loadMiniTramp(mtHandle,
                    theProc->get_dyn_process()->lowlevel_process(),
                    point, ast, when, order,
                    false, // false --> don't exclude cost
                    retInstance,
                    trampRecursiveDesired
                    );
   if(theProc->hasExited()) {
      res = failure_res;
   }
   rinstance = retInstance;
   
   if(res == success_res) {
       loadedIntoApp_ = true;
   }
   

   return res;
}

void instReqNode::hookupJumps(pd_process *proc) {
   if(trampsHookedUp()) 
      return;
   hookupMiniTramp(proc->get_dyn_process()->lowlevel_process(), mtHandle,
                   order);
   // since we've used it for it's only stated purpose, get rid of it
   trampsHookedUp_ = true;
}

void instReqNode::setAffectedDataNodes(miniTrampHandleFreeCallback cb, 
                                       pdvector<instrDataNode *> *affectedNodes) {
    assert(loadedIntoApp_ == true);
    mtHandle->registerCallback(cb, (void *)affectedNodes);
}

void instReqNode::disable(pd_process *proc)
{
  //cerr << "in instReqNode (" << this << ")::disable  loadedIntoApp = "
  //     << loadedIntoApp << ", hookedUp: " << trampsHookedUp_
  //     << ", deleting inst: " << mtHandle.inst << "\n";
  //     << " points to check\n";
   if(loadedIntoApp_ == true && trampsHookedUp_ == true)
      deleteInst(proc->get_dyn_process()->lowlevel_process(), mtHandle);
}

instReqNode::~instReqNode()
{
  removeAst(ast);
}

timeLength instReqNode::cost(pd_process *theProc) const
{
  // Currently the predicted cost represents the maximum possible cost of the
  // snippet.  For instance, for the statement "if(cond) <stmtA> ..."  the
  // cost of <stmtA> is currently included, even if it's actually not called.
  // Feel free to change the maxCost call below to ast->minCost or
  // ast->avgCost if the semantics need to be changed.
   process *llproc = theProc->get_dyn_process()->lowlevel_process();
  int unitCostInCycles = ast->maxCost() + getPointCost(llproc, point) +
                        getInsnCost(trampPreamble) + getInsnCost(trampTrailer);
  timeLength unitCost(unitCostInCycles, getCyclesPerSecond());
  float frequency = getPointFrequency(point);
  timeLength value = unitCost * frequency;
  return(value);
}

void instReqNode::catchupRPCCallback(void * /*returnValue*/ ) {
   ++rpcCount;
}


bool instReqNode::triggeredInStackFrame(Frame &frame, 
                                        pd_process *p)
{
   return p->triggeredInStackFrame(frame, point, when, order);
}



