/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#ifndef _BPatch_frame_h_
#define _BPatch_frame_h_

#include "BPatch_Vector.h"

/*
 * Frame information needed for stack walking
 */
typedef enum {
    BPatch_frameNormal,
    BPatch_frameSignal,
    BPatch_frameTrampoline
} BPatch_frameType;

class BPatch_function;
class BPatch_thread;

class BPATCH_DLL_EXPORT BPatch_frame {
    friend class BPatch_thread;
    BPatch_thread *thread;

    void *pc;
    void *fp;
    bool isSignalFrame;
    bool isTrampFrame;
    // BPatch defines that a trampoline is effectively a "function call" and
    // puts an extra tramp on the stack. Various people (frex, Paradyn) really
    // don't want to see this frame. To make life simpler for everyone, we
    // add a "only call if you know what you're doing" flag.
    bool isSynthFrame;

    // This is _so_ much easier than looking it up later. If we're
    // in instrumentation, stash the point
    BPatch_point *point_;

public:
    BPatch_frame();
    BPatch_frame(BPatch_thread *_thread, 
                 void *_pc, void *_fp, 
                 bool isf = false, 
                 bool istr = false, BPatch_point *point = NULL,
                 bool isSynth = false);


    //  BPatch_frame::getFrameType
    //  Returns type of frame: BPatch_frameNormal for a stack frame for a 
    //  function, BPatch_frameSignal for the stack frame created when a signal 
    //  is delivered, or BPatch_frameTrampoline for a stack frame created by 
    //  internal Dyninst instrumentation.
    BPatch_frameType getFrameType();

    //  Only call if you know what you are doing; per-frame method for determining
    //  how the frame was created.
    bool isSynthesized();

    //  BPatch_frame::getThread
    //  Returns:  value of program counter

    BPatch_thread * getThread(); 

    //  BPatch_frame::getThread
    //  Returns:  value of program counter

    BPatch_point * getPoint(); 

    //  BPatch_frame::getPC
    //  Returns:  value of program counter

    void * getPC(); 

    //  BPatch_frame::getFP

    void * getFP(); 

    //  BPatch_frame::findFunction
    //  Returns:  the function corresponding to this stack frame, NULL 
    //   if there is none

    BPatch_function * findFunction();
   
    // The following are planned but no yet implemented:
    // int getSignalNumber();

    BPatch_point * findPoint();
};

#endif
