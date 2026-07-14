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
#if !defined(PROCPOOL_H_)
#define PROCPOOL_H_

#include <set>
#include <map>
#include "dyntypes.h"
#include "common/src/dthread.h"
#include "PCProcess.h"

using namespace Dyninst;

class int_process;
class int_thread;

// PROTOTYPE (pool-owns-wrapper): the pool holds the reference-counted
// Process/Thread wrappers instead of raw impl pointers.  The pool's strong
// ref is what keeps a wrapper alive for the debugging session; the impl
// (int_process/int_thread) holds no up-pointer at all.
//
// Lifecycle:
//  - Registration happens at birth (initializeProcess/createThread) once the
//    OS id is known.
//  - rmProcess/rmThread only unregister (drop the pool's strong ref).  The
//    wrapper<->impl link is NOT severed there: post-exit handlers and the
//    callback layer still traverse wrapper->llproc() after rmProcess.
//  - Publish-exit-state + sever + delete happen together in
//    int_process::destroy(Process::ptr) / int_thread::destroy(Thread::ptr),
//    which every impl deletion site calls with the wrapper in hand.
//    Measured across the full testsuite: the only post-unregister
//    impl->wrapper resolution was the thread dtors' exitstate publish, which
//    destroy() satisfies by passing the wrapper down -- so no zombie
//    bookkeeping is needed, and the impl dtors never talk to the pool.
class ProcessPool
{
   friend ProcessPool *ProcPool();
 protected:
   std::set<Dyninst::LWP> deadThreads;
   std::map<Dyninst::PID, Dyninst::ProcControlAPI::Process::ptr> procs;
   std::map<Dyninst::LWP, Dyninst::ProcControlAPI::Thread::ptr> lwps;
   // PROTOTYPE: the old code resolved impl->wrapper via a lock-free field
   // read (up_proc/up_thread); the pool's maps are read from threads that do
   // NOT all hold the ProcPool condvar (e.g. the generator's decoder), so
   // the maps get their own leaf-level recursive mutex.  Recursive because
   // destroy-side helpers nest rmProcess/rmThread.
   Mutex<true> map_lock;
   ProcessPool();
   CondVar<> var;
 public:
   ~ProcessPool();
   typedef bool(*ifunc)(int_process *, void *data);

   // The pool API traffics exclusively in wrappers: OS ids in, wrappers out.
   // Raw impl pointers never cross this interface.
   Dyninst::ProcControlAPI::Process::ptr findProcByPid(Dyninst::PID pid);
   void addProcess(Dyninst::ProcControlAPI::Process::ptr proc);
   // Registers a newborn wrapper (and thereby establishes the impl->wrapper
   // mapping -- before this call, wrapperFor() cannot resolve it, so the
   // creator must hand the wrapper in).  The impl is derived via llthrd().
   void addThread(Dyninst::ProcControlAPI::Thread::ptr wrapper);
   void rmProcess(Dyninst::ProcControlAPI::Process::ptr proc);
   void rmThread(Dyninst::ProcControlAPI::Thread::ptr thr);
   Dyninst::ProcControlAPI::Thread::ptr findThread(Dyninst::LWP lwp);

   // The only deletion points for the impls: unregister (links intact),
   // publish exit state into the wrappers (passed down, never resolved
   // post-unregistration), sever the ll links, delete the impl.
   void destroyProcess(Dyninst::ProcControlAPI::Process::ptr proc);
   void destroyThread(Dyninst::ProcControlAPI::Thread::ptr thr);

   // impl -> wrapper resolution (live maps; identity-checked).
   Dyninst::ProcControlAPI::Process::ptr wrapperFor(int_process *proc);
   Dyninst::ProcControlAPI::Thread::ptr wrapperFor(int_thread *thr);
   // On Linux, we can get notifications for dead threads. Fun. 
   bool deadThread(Dyninst::LWP lwp);
   void addDeadThread(Dyninst::LWP lwp);
   void removeDeadThread(Dyninst::LWP lwp);
   unsigned numProcs();
   bool LWPIDsAreUnique();
   bool for_each(ifunc f, void *data = NULL);
   CondVar<> *condvar();
};

ProcessPool *ProcPool();

#endif
