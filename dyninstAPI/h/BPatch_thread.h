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

#ifndef _BPatch_thread_h_
#define _BPatch_thread_h_

#include <stdio.h>

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_image.h"
#include "BPatch_snippet.h"
#include "BPatch_eventLock.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"
#include "BPatch_frame.h"

class process;
class BPatch;
class BPatch_thread;
class BPatch_process;
class BPatch_statement;
class dyn_thread;

typedef long dynthread_t;

/*
 * Represents a thread of execution.
 */
#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_thread

class BPATCH_DLL_EXPORT BPatch_thread : public BPatch_eventLock {
    friend class BPatch_frame;
    friend class BPatch_process;
    friend class BPatch_addressSpace;
    friend class BPatch;
    friend bool pollForStatusChange();
    friend class BPatch_asyncEventHandler;
    friend class AsyncThreadEventCallback;
    friend class process;

    BPatch_process *proc;
    dyn_thread *llthread;
    unsigned index;
    bool updated;
    bool doa;
    bool reported_to_user; //Have we notified this user of thread creation yet?
    //  If thread is doa, keep a record of the tid around so that user
    //  callbacks can get the right tid, even if they can't do anything else
    //  with it.
    dynthread_t doa_tid;
    bool deleted_callback_made;
    bool is_deleted;
    bool legacy_destructor;

 protected:
    BPatch_thread(BPatch_process *parent, dyn_thread *dthr);
    BPatch_thread(BPatch_process *parent, int ind, int lwp_id, dynthread_t async_tid);

    void setDynThread(dyn_thread *thr);
    //Generator for above constructor
    static BPatch_thread *createNewThread(BPatch_process *proc, int ind, 
                                          int lwp_id, dynthread_t async_tid);
    void deleteThread(bool cleanup = true);
    void removeThreadFromProc();
    void updateValues(dynthread_t tid, unsigned long stack_start, 
                      BPatch_function *initial_func, int lwp_id);

 public:

    void markVisiblyStopped(bool new_state);
    bool isVisiblyStopped();

    //  BPatch_thread::getCallStack
    //  Returns a vector of BPatch_frame, representing the current call stack
    API_EXPORT(Int, (stack),
    bool,getCallStack,(BPatch_Vector<BPatch_frame>& stack));

    //  BPatch_thread::getProcess
    //  Returns a pointer to the process that owns this thread
    API_EXPORT(Int, (),
    BPatch_process *, getProcess, ());

    API_EXPORT(Int, (),
    dynthread_t, getTid, ());

    API_EXPORT(Int, (),
    int, getLWP, ());

    API_EXPORT(Int, (),
    unsigned, getBPatchID, ());

    API_EXPORT(Int, (),
    BPatch_function *, getInitialFunc, ());
    
    API_EXPORT(Int, (),
    unsigned long, getStackTopAddr, ());

    API_EXPORT(Int, (),
    bool, isDeadOnArrival, ());

    API_EXPORT_DTOR(_dtor, (),
    ~,BPatch_thread,());

    API_EXPORT(Int, (),
    unsigned long, os_handle, ());

    //  BPatch_thread::oneTimeCode
    //  Have mutatee execute specified code expr once.  Wait until done.
    API_EXPORT(Int, (expr, err),
    void *,oneTimeCode,(const BPatch_snippet &expr, bool *err = NULL));

    //  BPatch_thread::oneTimeCodeAsync
    //  Have mutatee execute specified code expr once.  Dont wait until done.
    API_EXPORT(Int, (expr, userData, cb),
    bool,oneTimeCodeAsync,(const BPatch_snippet &expr, void *userData = NULL, BPatchOneTimeCodeCallback cb = NULL));


    /*
    // DO NOT USE
    // this function should go away as soon as Paradyn links against Dyninst
    process *lowlevel_process() { return proc->llproc; }
    */

};

#endif /* BPatch_thread_h_ */
