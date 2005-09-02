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

#include "dyninstAPI/src/function.h"
#include "dyninstAPI/h/BPatch_function.h"

// Catchup
#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/frame.h"
#include "dyninstAPI/src/instPoint.h"

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
#if 0
    if(loadInstAttempts == MAX_INSERTION_ATTEMPTS_USING_RELOCATION) {
        BPatch_function *bpf = const_cast<BPatch_function *>(point->getFunction());
        int_function *function_not_inserted = bpf->PDSEP_pdf();
        
        if(function_not_inserted != NULL)
            function_not_inserted->markAsNeedingRelocation(false);
    }
#endif
#endif
    
    // NEW: We may manually trigger the instrumentation, via a call to
    // postRPCtoDo()
    
    bool trampRecursiveDesired = false;
    
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


    if (!BPatchToInternalArgs(*point, when, order, cw, co)) {
        fprintf(stderr, "ERROR: unable to convert Dyninst inst args to internals\n");
        return false;
    }    

    if(theProc->hasExited()) {
        return false;
    }
    
    instPoint *pt = point->PDSEP_instPoint();
    
    mtHandle = pt->addInst(lr_ast,
                           cw,
                           co,
                           trampRecursiveDesired,
                           false); // noCost
    
    if (!mtHandle) {
        return false;
    }
    
    instrAdded_ = true;
    
    return true;
}

bool instReqNode::generateInstr() {
    if(instrGenerated()) 
        return true;
    
    instPoint *pt = point->PDSEP_instPoint();
    
    if (!pt->generateInst()) {
        fprintf(stderr, "********** generate failed\n");
        return false;
    }
    if (!pt->installInst()) {
        fprintf(stderr, "********** install failed\n");
        return false;
    }

    instrGenerated_ = true;
    
    return true;
}

bool instReqNode::checkInstr(pdvector<pdvector<Frame> > &stackWalks) {
    if (instrLinked())
        return true;
    
    instPoint *pt = point->PDSEP_instPoint();
    
    pdvector<Address> pcs;
    
    for (unsigned sI = 0; sI < stackWalks.size(); sI++) {
        for (unsigned fI = 0; fI < stackWalks[sI].size(); fI++) {
            /*
              // For bpatch_frame...
            if (stackWalks[sI][fI].isSynthesized()) 
                continue;
            */
            pcs.push_back((Address)stackWalks[sI][fI].getPC());
        }
    }

    return pt->checkInst(pcs);
}

bool instReqNode::linkInstr() {
    if (instrLinked()) return true;
    instPoint *pt = point->PDSEP_instPoint();
    
    if (!pt->linkInst()) {
        fprintf(stderr, "*********** linkInst failed\n");
        return false;
    }
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

timeLength instReqNode::cost(pd_process *) const
{
  // Currently the predicted cost represents the maximum possible cost of the
  // snippet.  For instance, for the statement "if(cond) <stmtA> ..."  the
  // cost of <stmtA> is currently included, even if it's actually not called.
  // Feel free to change the maxCost call below to ast->minCost or
  // ast->avgCost if the semantics need to be changed.
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

typedef enum {nowhere_l, 
              beforePoint_l, 
              notMissed_l, 
              missed_l, 
              afterPoint_l} logicalPCLocation_t;
extern bool pd_debug_catchup;

bool instReqNode::triggeredInStackFrame(Frame &frame, 
                                        pd_process *p)
{
    // Things we have:
    // mtHandle: a miniTramp
    // point: a BPatch_point

    // We first see if we're before, during, or after the BPatch_point. Before and
    // after have odd meanings, since functions aren't really linear widgets. So
    // we try and get this to match.

    instPoint *iP = Point()->PDSEP_instPoint();

    if (frame.getPC() == 0) return false;
    
    if(pd_debug_catchup) {
        fprintf(stderr, "--------\n");
        fprintf(stderr, "Catchup for PC 0x%lx (%d), instpoint at 0x%lx (%s), ",
                frame.getPC(), 
                frame.getThread() == NULL ? -1 : frame.getThread()->get_tid(),
                iP->addr(),
                iP->func()->prettyName().c_str());
        fprintf(stderr, "point type is ");
        if (Point()->getPointType() == BPatch_locEntry)
            fprintf(stderr, "FuncEntry, ");
        else if (Point()->getPointType() == BPatch_locExit)
            fprintf(stderr, "FuncExit, ");
        else if (Point()->getPointType() == BPatch_locSubroutine)
            fprintf(stderr, "CallSite, ");
        else if (Point()->getPointType() == BPatch_locLoopEntry)
            fprintf(stderr, "LoopEntry, ");
        else if (Point()->getPointType() == BPatch_locLoopExit)
            fprintf(stderr, "LoopExit, ");
        else if (Point()->getPointType() == BPatch_locLoopStartIter)
            fprintf(stderr, "LoopStartIter, ");
        else if (Point()->getPointType() == BPatch_locLoopEndIter)
            fprintf(stderr, "LoopEndIter, ");
        else fprintf(stderr, "other, ");
        
        fprintf(stderr, "callWhen is ");
        if (When() == BPatch_callBefore)
            fprintf(stderr, "callBefore");
        else
            fprintf(stderr, "callAfter");
        fprintf(stderr, ", order is ");
        if (Order() == BPatch_firstSnippet)
            fprintf(stderr, "insertFirst");
        else
            fprintf(stderr, "insertLast");
        fprintf(stderr,"\n");
    }

    // First, if we're not even in the right _function_, then break out.
    // Note: a lot of this logic should be moved to the BPatch layer. For now,
    // it's here.

    if (frame.getFunc() != Point()->getFunction()->PDSEP_pdf())
        return false;

    // If we're inside the function, find whether we're before, inside, or after the point.
    // This is done by address comparison and used to demultiplex the logic below.
    
    logicalPCLocation_t location;

    instPoint::catchup_result_t iPresult = iP->catchupRequired(frame.getPC(), mtHandle);

    if (iPresult == instPoint::notMissed_c)
        location = notMissed_l;
    else if (iPresult == instPoint::missed_c)
        location = missed_l;
    else
        location = nowhere_l;

    // We check for the instPoint before this because we use instrumentation
    // that may cover multiple instructions.
    // USE THE UNINSTRUMENTED ADDR :)
    if (location == nowhere_l) {
        // Back off to address comparison
        if ((Address)Point()->getAddress() < frame.getUninstAddr())
            location = afterPoint_l;
        else
            location = beforePoint_l;
    }

    if (pd_debug_catchup) {
        switch(location) {
        case beforePoint_l:
            fprintf(stderr, "strictly before point...");
            break;
        case notMissed_l:
            fprintf(stderr, "before point...");
            break;
        case missed_l:
            fprintf(stderr, "after point...");
            break;
        case afterPoint_l:
            fprintf(stderr, "strictly after point...");
            break;
        case nowhere_l:
            fprintf(stderr, "serious problem with the compiler...");
            break;
        }    
    }


    bool catchupNeeded = false;
    
    // We split cases out by the point type
    // All of these must fit the following criteria:
    // An object with a well-defined entry and exit;
    // An object where we can tell if a PC is "within".
    // Examples: functions, basic blocks, loops
    switch(Point()->getPointType()) {
    case BPatch_locEntry:
        // Entry is special, since it's one of the rare
        // cases where "after" is good enough. TODO:
        // check whether we're "in" a function in a manner
        // similar to loops.
        // We know we can get away with >= because we're in the
        // function; if not we'd have already returned.
        if (location >= missed_l)
            catchupNeeded = true;
        break;
    case BPatch_locExit:
        // Only do this if we triggered "missed". If we're
        // after, we might well be later in the function.
        // If this is true, we're cancelling an earlier entry
        // catchup.
        if (location == missed_l)
            catchupNeeded = true;
        break;
    case BPatch_subroutine:
        // Call sites. Again, only if missed; otherwise we may
        // just be elsewhere
        if (location == missed_l)
            catchupNeeded = true;
        break;
    case BPatch_locLoopEntry:
    case BPatch_locLoopStartIter:
        if (location == missed_l)
            catchupNeeded = true;
        if (location == afterPoint_l) {
            BPatch_basicBlockLoop *loop = Point()->getLoop();
            if (loop->containsAddressInclusive(frame.getUninstAddr()))
                catchupNeeded = true;
        }
        break;
    case BPatch_locLoopExit:
    case BPatch_locLoopEndIter:
        // See earlier treatment of, well, everything else
        if (location == missed_l)
            catchupNeeded = true;
        break;

    case BPatch_locBasicBlockEntry:
    case BPatch_locBasicBlockExit:       
    default:
        // Nothing here
        break;
    }
    
    if (pd_debug_catchup) {
        if (catchupNeeded)
            fprintf(stderr, "catchup needed, ret true\n========\n");
        else
            fprintf(stderr, "catchup not needed, ret false\n=======\n");
    }
    
    return catchupNeeded;
}
