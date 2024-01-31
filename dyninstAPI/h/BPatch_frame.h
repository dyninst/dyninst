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
    bool isSynthFrame;

    BPatch_point *point_;

public:
    BPatch_frame();
    BPatch_frame(BPatch_thread *_thread, 
                 void *_pc, void *_fp, 
                 bool isf = false, 
                 bool istr = false, BPatch_point *point = NULL,
                 bool isSynth = false);


    BPatch_frameType getFrameType();

    bool isSynthesized();

    BPatch_thread * getThread(); 

    BPatch_point * getPoint(); 

    void * getPC(); 

    void * getFP(); 

    BPatch_function * findFunction();

    // int getSignalNumber();

    BPatch_point * findPoint();
};

#endif
