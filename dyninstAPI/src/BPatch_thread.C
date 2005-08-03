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

// $Id: BPatch_thread.C,v 1.130 2005/08/03 23:00:50 bernat Exp $

#define BPATCH_FILE

#include "BPatch_thread.h"
#include "BPatch.h"
#include "process.h"
#include "BPatch_libInfo.h"


/*
 * BPatch_thread::getCallStack
 *
 * Returns information about the frames currently on the thread's stack.
 *
 * stack	The vector to fill with the stack trace information.
 */
bool BPatch_thread::getCallStackInt(BPatch_Vector<BPatch_frame>& stack)
{
    pdvector<pdvector<Frame> > stackWalks;

    proc->llproc->walkStacks(stackWalks);

    // We can only handle one thread right now; change when we begin to handle
    // multiple threads.
    assert(stackWalks.size() == 1);

    // The internal representation of a stack walk treats instrumentation
    // as part of the original instrumented function. That is to say, if A() 
    // calls B(), and B() is instrumented, the stack will appear as so:
    // A()
    // instrumentation

    // We want it to look like so:
    // A()
    // B()
    // instrumentation

    // We handle this by adding a synthetic frame to the stack walk whenever
    // we discover an instrumentation frame.

    for (unsigned int i = 0; i < stackWalks[0].size(); i++) {
        bool isSignalFrame = false;
        bool isInstrumentation = false;
        BPatch_point *point = NULL;

        Frame frame = stackWalks[0][i];
        frame.calcFrameType();
        if (frame.frameType_ != FRAME_unset) {
            isSignalFrame = frame.isSignalFrame();
            isInstrumentation = frame.isInstrumentation();
        }

        if (isInstrumentation) {
            // This is a bit of a slog, actually. We want to only show
            // bpatch points that exist, not describe internals to the
            // user. So instead of calling findOrCreateBPPoint, we manually
            // poke through the mapping table. If there isn't a point, we
            // skip this instrumentation frame instead
            instPoint *iP = frame.getPoint();
            fprintf(stderr, "got iP %p\n", iP);
            if (iP && proc->instp_map->defines(iP))
                point = proc->instp_map->get(iP);
            fprintf(stderr, "and point %p\n", point);
            if (point) {
                stack.push_back(BPatch_frame(this,
                                             (void*)stackWalks[0][i].getPC(),
                                             (void*)stackWalks[0][i].getFP(),
                                             false,
                                             true,
                                             point));
                // And the "top-level function" one.
                stack.push_back(BPatch_frame(this,
                                             (void *)stackWalks[0][i].getUninstAddr(),
                                             (void *)stackWalks[0][i].getFP(),
                                             false, // not signal handler,
                                             false, // not inst.
                                             NULL, // No point
                                             true)); // Synthesized frame
            }
            else {
                // No point = internal instrumentation, make it go away.
                stack.push_back(BPatch_frame(this,
                                             (void *)stackWalks[0][i].getUninstAddr(),
                                             (void *)stackWalks[0][i].getFP(),
                                             false, // not signal handler,
                                             false, // not inst.
                                             NULL, // No point
                                             false)); // Synthesized frame
                
            }
        }
        else {
            // Not instrumentation, normal case
            stack.push_back(BPatch_frame(this,
                                         (void *)stackWalks[0][i].getPC(),
                                         (void *)stackWalks[0][i].getFP(),
                                         isSignalFrame));
        }
    }
    return true;
}

BPatch_thread::BPatch_thread(BPatch_process *parent) 
{
   proc = parent;
}


BPatch_process *BPatch_thread::getProcessInt() 
{
   return proc;
}

unsigned BPatch_thread::getTidInt()
{
   assert(0);
   return 0;
}

