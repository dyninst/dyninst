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

// $Id: BPatch_frame.C,v 1.1 2005/08/04 13:11:55 bernat Exp $

#define BPATCH_FILE

#include "BPatch_thread.h"
#include "BPatch.h"
#include "instPoint.h"
#include "codeRange.h"
#include "frame.h"

/***************************************************************************
 * BPatch_frame
 ***************************************************************************/

/*
 * BPatch_frame::getFrameType()
 *
 * Returns the type of frame: BPatch_frameNormal for the stack frame for a
 * function, BPatch_frameSignal for the stack frame created when a signal is
 * delivered, or BPatch_frameTrampoline for a stack frame for a trampoline.
 */
BPatch_frameType BPatch_frame::getFrameTypeInt()
{
	if( isSignalFrame ) { return BPatch_frameSignal; }
	else if( isTrampFrame ) { return BPatch_frameTrampoline; }
	else { return BPatch_frameNormal; } 
}

void *BPatch_frame::getPCInt() 
{
  return pc;
}
void *BPatch_frame::getFPInt()
{
  return fp;
}

/*
 * BPatch_frame::findFunction()
 *
 * Returns the function associated with the stack frame, or NULL if there is
 * none.
 */
BPatch_function *BPatch_frame::findFunctionInt()
{
    return thread->findFunctionByAddr(getPC());
}

BPatch_frame::BPatch_frame() : 
   thread(NULL), pc(NULL), fp(NULL), isSignalFrame(false), 
   isTrampFrame(false), isSynthFrame(false), point_(NULL)
{
};

BPatch_frame::BPatch_frame(BPatch_thread *_thread, void *_pc, void *_fp, 
                           bool isf, bool istr, BPatch_point *point,
                           bool isSynth) :
    thread(_thread), pc(_pc), fp(_fp), 
    isSignalFrame(isf), isTrampFrame(istr),
    isSynthFrame(isSynth),
    point_(point)
{
    if (isTrampFrame) assert(point_);
};

bool BPatch_frame::isSynthesizedInt() {
    return isSynthFrame;
}

BPatch_point *BPatch_frame::findPointInt() {
    // If we're not in instrumentation, then return false
    if (!isTrampFrame)
        return NULL;

    assert(point_);
    return point_;
}


