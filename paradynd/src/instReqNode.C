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
#include "paradynd/src/debug.h"
#include "common/h/timing.h"
#include "paradynd/src/pd_process.h"
#include "pdutil/h/pdDebugOstream.h"

#include "dyninstAPI/h/BPatch_point.h"
#include "dyninstAPI/h/BPatch_function.h"
#include "dyninstAPI/h/BPatch_thread.h"

const int MAX_INSERTION_ATTEMPTS_USING_RELOCATION = 1000;

/*
 * functions to operate on inst request graph.
 *
 */

// special copy constructor used for fork handling

//   -- Question -- should we be doing a hard (not just pointer) copy of snippet?
instReqNode::instReqNode(const instReqNode &par, pd_process *childProc) : 
   point(par.point), snip(par.snip), 
   when(par.when), order(par.order),
   instrLoaded_(par.instrLoaded_),
   rpcCount(par.rpcCount), 
   loadInstAttempts(par.loadInstAttempts)
{
    if (!par.instrLoaded()) {
        // can't setup the child if the parent isn't setup
        return;
    }
    
   fprintf(stderr, "%s[%d]:  ERROR:  need to figure out copying of instrumentation handles in fork of %d\n", FILE__, __LINE__, childProc->getPid());
}

bool instReqNode::loadInstrIntoApp(pd_process *theProc)
{
    ++loadInstAttempts;
#if defined(cap_relocation)
#endif
    
    // NEW: We may manually trigger the instrumentation, via a call to
    // postRPCtoDo()
    if(theProc->hasExited()) {
        return false;
    }
    
    // OVERRIDE: We say BPatch_callBefore/BPatch_exit; we need
    // BPatch_callAfter/BPatch_exit. ARGH.
    // FIXME SOMEWHERE ELSE
    if ((point->getPointType() == BPatch_exit) 
       && (when == BPatch_callBefore))
        when = BPatch_callAfter;

   snipHandle = theProc->get_dyn_process()->insertSnippet(*snip, *point, when, order);

   if (!snipHandle) {
     fprintf(stderr, "%s[%d]:  insertSnippet failed\n", FILE__, __LINE__);
     return false;
   }

   return true;
}

void instReqNode::disable()
{
  //cerr << "in instReqNode (" << this << ")::disable  loadedIntoApp = "
  //     << loadedIntoApp << ", hookedUp: " << trampsHookedUp_
  //     << ", deleting inst: " << mtHandle.inst << "\n";
  //     << " points to check\n";

    if (instrLoaded()) {
        assert(snipHandle);
        BPatch_process *p = snipHandle->getProcess();
        assert(p);
        if (!p->deleteSnippet(snipHandle)) {
           fprintf(stderr, "%s[%d]:  failed to deleteSnippet\n", FILE__, __LINE__);
           //  hrmmmm...
        }
    }
}

instReqNode::~instReqNode()
{
}

timeLength instReqNode::cost(pd_process *) const
{
    assert(snip);
    assert(point);
    float cost_in_seconds = snip->getCostAtPoint(point);
    timeLength value(cost_in_seconds, timeUnit::sec());
    return(value);
}

void instReqNode::catchupRPCCallback(void * /*returnValue*/ ) 
{
   ++rpcCount;
}

