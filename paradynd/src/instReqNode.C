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
#include "dyninstAPI/src/process.h"
#include "pdutil/h/pdDebugOstream.h"

extern pdDebug_ostream metric_cerr;


/*
 * functions to operate on inst request graph.
 *
 */
instReqNode::instReqNode(instPoint *iPoint, AstNode *iAst, callWhen iWhen,
                         callOrder o) : loadedIntoApp(false), 
  trampsHookedUp(false), rinstance(NULL) {
  point = iPoint;
  when = iWhen;
  order = o;
  ast = assignAst(iAst);
  rpcCount = 0;
  mtHandle.location = iPoint;
  mtHandle.when = iWhen;
  assert(point);
}

instReqNode instReqNode::forkProcess(const instReqNode &parentNode,
		      const dictionary_hash<instInstance*,instInstance*> &map)
{
  /*
  instReqNode ret = instReqNode(parentNode.point, parentNode.ast, 
				parentNode.when, parentNode.order);
  
  if (!map.find(parentNode.instance, ret.instance)) // writes to ret.instance
    assert(false);
  */
  instReqNode ret;
  return ret;
}

bool instReqNode::unFork(process *proc, 
			 dictionary_hash<instInstance*,instInstance*> &map) 
  const {
  /*
  // The fork syscall duplicates all trampolines from the parent into the
  // child. For those mi's which we don't want to propagate to the child,
  // this creates a problem.  We need to remove instrumentation code from the
  // child.  This routine does that.
  //
  // "this" represents an instReqNode in the PARENT process.  "map" maps all
  // instInstance*'s of the parent process to instInstance*'s in the child
  // process.  We modify "map" by setting a value to NULL.

  //instInstance *parentInstance = getInstance();
   
  instInstance *childInstance;
  if (!map.find(parentInstance, childInstance)) // writes to childInstance
    assert(false);
  
  deleteInst(proc, childInstance);
  
  map[parentInstance] = NULL; // since we've deleted...
  */
  
  return true; // success
}

// returns false if instr insert was deferred
loadMiniTramp_result instReqNode::loadInstrIntoApp(process *theProc,
						returnInstance *&retInstance,
						   instInstance **mtInst) {
  (*mtInst) = NULL;
  if(loadedIntoApp) return success_res;

  // NEW: We may manually trigger the instrumentation, via a call to
  // postRPCtoDo()
  
  // addInstFunc() is one of the key routines in all paradynd.  It installs a
  // base tramp at the point (if needed), generates code for the tramp, calls
  // inferiorMalloc() in the text heap to get space for it, and actually
  // inserts the instrumentation.
  instInstance *instI = new instInstance;
  loadMiniTramp_result res = 
    loadMiniTramp(instI, theProc, point, ast, when, order,
		  false, // false --> don't exclude cost
		  retInstance,
		  false // false --> do not allow recursion
		  );
  rinstance = retInstance;
  
  if(res == success_res) {
    loadedIntoApp = true;
    mtHandle.inst = instI;
    mtHandle.when = when;
    mtHandle.location = point;
    (*mtInst) = instI;
  } else {
    delete instI;
    (*mtInst) = NULL;
  }

  return res;
}

void instReqNode::hookupJumps(process *proc) {
  assert(trampsHookedUp == false);
  hookupMiniTramp(proc, mtHandle, order);
  // since we've used it for it's only stated purpose, get rid of it
  trampsHookedUp = true;
}

void instReqNode::disable(process *proc)
{
  //cerr << "in instReqNode::disable - " << pointsToCheck.size()
  //     << " points to check\n";
  if(loadedIntoApp == true)
    deleteInst(proc, mtHandle);
}

instReqNode::~instReqNode()
{
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

bool instReqNode::postCatchupRPC(process *theProc,
				 Frame &triggeredFrame,
				 int mid) 
{
  // So we have an instrumentation node (this), and a frame
  // to trigger it in.
  theProc->postRPCtoDo(ast, false, // don't skip cost
		       NULL,
		       (void *)this,
		       mid,
		       triggeredFrame.getThread(),
		       triggeredFrame.getLWP(),
		       false); // normal RPC, not thread-independent
  if (pd_debug_catchup)
    cerr << "catchup inferior RPC has been launched for instReqNode"
	 << (int) this 
	 << " with frame"
	 << triggeredFrame << endl;

  // Don't launch the RPC immediately. We'll launch them all
  // at one time.
  return true;
}

void instReqNode::catchupRPCCallback(void * /*returnValue*/ ) {
   ++rpcCount;
}


bool instReqNode::triggeredInStackFrame(Frame &frame, process *p)
{
   return p->triggeredInStackFrame(point, frame, when, order);
}



