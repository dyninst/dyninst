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

#ifndef _BPatch_thread_h_
#define _BPatch_thread_h_

#include <stdio.h>

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_image.h"
#include "BPatch_snippet.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"
#include "BPatch_frame.h"

class PCProcess;
class BPatch;
class BPatch_thread;
class BPatch_process;
class BPatch_statement;
class PCThread;

typedef Dyninst::THR_ID dynthread_t;

class BPATCH_DLL_EXPORT BPatch_thread {
    friend class BPatch_frame;
    friend class BPatch_process;
    friend class BPatch_addressSpace;
    friend class BPatch;

    BPatch_process *proc;
    PCThread *llthread;

	bool madeExitCallback_;

 protected:
    BPatch_thread(BPatch_process *parent, PCThread *thr);

    static BPatch_thread *createNewThread(BPatch_process *proc, PCThread *thr);

    void updateThread(PCThread *newThr);
    
	bool madeExitCallback() { return madeExitCallback_; }
	void setMadeExitCallback() { madeExitCallback_ = true; }

 public:

    bool getCallStack(BPatch_Vector<BPatch_frame>& stack);

    BPatch_process *  getProcess();

    dynthread_t getTid();

    Dyninst::LWP getLWP();

    unsigned getBPatchID();

    BPatch_function *  getInitialFunc();
    
    unsigned long getStackTopAddr();

    bool isDeadOnArrival();

    ~BPatch_thread();

    unsigned long os_handle();

    void * oneTimeCode(const BPatch_snippet &expr, bool *err = NULL);

    bool oneTimeCodeAsync(const BPatch_snippet &expr, void *userData = NULL, BPatchOneTimeCodeCallback cb = NULL);
};

#endif /* BPatch_thread_h_ */
