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

#ifndef _BPatch_frame_h_
#define _BPatch_frame_h_

#include "BPatch_Vector.h"

/*
 * Frame information needed for stack walking
 */
typedef enum {
    BPatch_frameNormal,
    BPatch_frameSignal,
    BPatch_frameTrampoline,
} BPatch_frameType;

class BPatch_function;
class BPatch_thread;

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_frame

class BPATCH_DLL_EXPORT BPatch_frame : public BPatch_eventLock{
    friend class BPatch_thread;
    friend class BPatch_Vector<BPatch_frame>;
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
    BPatch_frame();
    BPatch_frame(BPatch_thread *_thread, 
                 void *_pc, void *_fp, 
                 bool isf = false, 
                 bool istr = false, BPatch_point *point = NULL,
                 bool isSynth = false);

    // This is _so_ much easier than looking it up later. If we're
    // in instrumentation, stash the point
    BPatch_point *point_;

public:
    //  BPatch_frame::getFrameType
    //  Returns type of frame: BPatch_frameNormal for a stack frame for a 
    //  function, BPatch_frameSignal for the stack frame created when a signal 
    //  is delivered, or BPatch_frameTrampoline for a stack frame created by 
    //  internal Dyninst instrumentation.
    API_EXPORT(Int, (),

    BPatch_frameType,getFrameType,());

    //  Only call if you know what you are doing; per-frame method for determining
    //  how the frame was created.
    API_EXPORT(Int, (),
    bool, isSynthesized, ());

    //  BPatch_frame::getPC
    //  Returns:  value of program counter
    API_EXPORT(Int, (),

    void *,getPC,()); 

    //  BPatch_frame::getFP
    API_EXPORT(Int, (),

    void *,getFP,()); 

    //  BPatch_frame::findFunction
    //  Returns:  the function corresponding to this stack frame, NULL 
    //   if there is none
    API_EXPORT(Int, (),

    BPatch_function *,findFunction,());
   
    // The following are planned but no yet implemented:
    // int getSignalNumber();

    API_EXPORT(Int, (), 
    BPatch_point *,findPoint,());

};

#endif
