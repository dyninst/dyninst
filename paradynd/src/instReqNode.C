/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
#include "dyninstAPI/h/BPatch_snippet.h"
#include "common/h/timing.h"
#include "paradynd/src/pd_process.h"
#include "pdutil/h/pdDebugOstream.h"

const int MAX_INSERTION_ATTEMPTS_USING_RELOCATION = 1000;

/*
 * functions to operate on inst request graph.
 *
 */

// special copy constructor used for fork handling

//  PDSEP -- Question -- should we be doing a hard (not just pointer) copy of snippet?
instReqNode::instReqNode(const instReqNode &par, pd_process *childProc) : 
   point(par.point), snip(par.snip), 
   when(par.when), order(par.order),
   instrAdded_(par.instrAdded_), 
   instrGenerated_(par.instrGenerated_),
   instrLinked_(par.instrLinked_),
   rpcCount(par.rpcCount), 
   loadInstAttempts(par.loadInstAttempts)
{
    if(!par.instrAdded() || 
       !par.instrGenerated() || 
       !par.instrLinked()) {
        // can't setup the child if the parent isn't setup
        return;
    }
    
    process *llproc = childProc->get_dyn_process()->lowlevel_process();
    bool res = getInheritedMiniTramp(par.mtHandle, mtHandle, llproc);
	assert(res == true);
}

// This handles conversion without requiring inst.h in a header file...
extern bool BPatchToInternalArgs(BPatch_point point,
                                 BPatch_callWhen when,
                                 BPatch_snippetOrder order,
                                 callWhen &ipWhen,
                                 callOrder &ipOrder);

// returns false if instr insert was deferred
// Runs generate and install... but not link
bool instReqNode::addInstr(pd_process *theProc) {
    if(instrGenerated()) return true;
    
    ++loadInstAttempts;
#if defined(cap_relocation)
    if(loadInstAttempts == MAX_INSERTION_ATTEMPTS_USING_RELOCATION) {
        BPatch_function *bpf = const_cast<BPatch_function *>(point->getFunction());
        int_function *function_not_inserted = bpf->PDSEP_pdf();
        
        if(function_not_inserted != NULL)
            function_not_inserted->markAsNeedingRelocation(false);
    }
#endif
    
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
    
    //  PDSEP -- eventually need to use insertSnippet here
    AstNode * l_ast = snip->PDSEP_ast();
    AstNode * &lr_ast = l_ast;
    
    
    //  PDSEP, these switches will go away....
    callWhen cw = callPreInsn;
    callOrder co = orderFirstAtPoint;
    
    // OVERRIDE: We say BPatch_callBefore/BPatch_exit; we need
    // BPatch_callAfter/BPatch_exit. ARGH.
    // FIXME SOMEWHERE ELSE
    if ((point->getPointType() == BPatch_exit) &&
        (when == BPatch_callBefore))
        when = BPatch_callAfter;


    if (!BPatchToInternalArgs(*point, when, order, cw, co))
        return false;
    
    if(theProc->hasExited()) {
        return false;
    }
    
    instPoint *pt = point->PDSEP_instPoint();
    
    miniTramp *mt = pt->addInst(lr_ast,
                                cw,
                                co,
                                trampRecursiveDesired,
                                false); // noCost
    
    if (!mt) {
        return false;
    }
    
    instrAdded_ = true;
    
    return true;
}

bool instReqNode::generateInstr() {
    if(instrGenerated()) 
        return true;
    
    instPoint *pt = point->PDSEP_instPoint();
    
    if (!pt->generateInst(false))
        return false;
    
    instrGenerated_ = true;

    return true;
}

bool instReqNode::checkInstr(pdvector<pdvector<Frame> > &stackWalks) {
    if (instrLinked()) return true;
    
    instPoint *pt = point->PDSEP_instPoint();

    if (!pt->checkInst(stackWalks))
        return false;

    return true;
}

bool instReqNode::linkInstr() {
    if (instrLinked()) return true;
    instPoint *pt = point->PDSEP_instPoint();
    
    if (!pt->linkInst())
        return false;
    instrLinked_ = true;
    return true;
}

void instReqNode::setAffectedDataNodes(miniTrampFreeCallback cb, 
                                       void *v) {
    assert(instrAdded() == true);
    mtHandle->registerCallback(cb, v);
}

void instReqNode::disable()
{
  //cerr << "in instReqNode (" << this << ")::disable  loadedIntoApp = "
  //     << loadedIntoApp << ", hookedUp: " << trampsHookedUp_
  //     << ", deleting inst: " << mtHandle.inst << "\n";
  //     << " points to check\n";
    if(instrAdded() == true && instrGenerated() == true) {
        assert(mtHandle);
        mtHandle->uninstrument();
    }
}

instReqNode::~instReqNode()
{
}

timeLength instReqNode::cost(pd_process *theProc) const
{
  // Currently the predicted cost represents the maximum possible cost of the
  // snippet.  For instance, for the statement "if(cond) <stmtA> ..."  the
  // cost of <stmtA> is currently included, even if it's actually not called.
  // Feel free to change the maxCost call below to ast->minCost or
  // ast->avgCost if the semantics need to be changed.
    process *llproc = theProc->get_dyn_process()->lowlevel_process();
    int unitCostInCycles = snip->PDSEP_ast()->maxCost() 
        + point->PDSEP_instPoint()->getPointCost() +
        getInsnCost(trampPreamble) + getInsnCost(trampTrailer);
    timeLength unitCost(unitCostInCycles, getCyclesPerSecond());
    float frequency = getPointFrequency(point->PDSEP_instPoint());
    timeLength value = unitCost * frequency;
    return(value);
}

void instReqNode::catchupRPCCallback(void * /*returnValue*/ ) {
   ++rpcCount;
}


bool instReqNode::triggeredInStackFrame(Frame &frame, 
                                        pd_process *p)
{
   return p->triggeredInStackFrame(frame, point,
                                   when, order);
}



