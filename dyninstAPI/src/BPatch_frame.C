/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: BPatch_frame.C,v 1.3 2006/07/07 00:01:00 jaw Exp $

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

BPatch_thread *BPatch_frame::getThreadInt() 
{
  return thread;
}

BPatch_point *BPatch_frame::getPointInt() 
{
  return point_;
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
  if (!getPC()) {
    return NULL;
  }
  return thread->getProcess()->findFunctionByAddr(getPC());
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


