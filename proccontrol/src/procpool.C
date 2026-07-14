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
#include "procpool.h"
#include "int_process.h"
#include "PCErrors.h"

#include <vector>

using Dyninst::ProcControlAPI::Process;
using Dyninst::ProcControlAPI::Thread;

#include <assert.h>

// ---------------------------------------------------------------------------
// Locking discipline (see the design note in procpool.h)
//
//  map_lock is the SOLE guardian of the registry containers -- procs, lwps,
//  and deadThreads.  It is a LEAF lock: acquired only for the duration of a
//  container operation, and never held across a blocking wait, a user
//  callback, an int_process/int_thread delete, or an acquisition of the
//  condvar (var) or the MTManager work_lock.  Ordering is therefore
//    work_lock  >  var (condvar)  >  map_lock
//  and because map_lock is a leaf no cycle is possible.
//
//  Every public method that touches a container takes map_lock exactly once,
//  then delegates to a `_nolock` helper that assumes it is held.  The helpers
//  are what the lock-holding methods call each other through, so map_lock can
//  be a plain (non-recursive) mutex -- the reentrancy is explicit in the call
//  graph rather than hidden in the lock.
//
//  var (a CondVar) keeps its distinct role: cross-thread signaling of
//  process-state progress (the generator waits on it, handlers broadcast) and
//  the coarse serialization callers bracket around multi-step lifecycle
//  sequences.  It is NOT the container guardian.
// ---------------------------------------------------------------------------

ProcessPool *ProcPool()
{
   static ProcessPool *ppool = NULL;
   if (!ppool) {
      ppool = new ProcessPool();
   }
   return ppool;
}

ProcessPool::ProcessPool()
{
}

ProcessPool::~ProcessPool()
{
}

// ---- container primitives (assume map_lock held) --------------------------

Process::ptr ProcessPool::findProcByPid_nolock(Dyninst::PID pid)
{
   auto i = procs.find(pid);
   if (i == procs.end())
      return Process::ptr();
   return i->second;
}

Thread::ptr ProcessPool::findThread_nolock(Dyninst::LWP lwp)
{
   if (!LWPIDsAreUnique())
      return Thread::ptr();
   auto i = lwps.find(lwp);
   if (i == lwps.end())
      return Thread::ptr();
   return i->second;
}

void ProcessPool::addDeadThread_nolock(Dyninst::LWP lwp)
{
   deadThreads.insert(lwp);
}

void ProcessPool::rmThread_nolock(Thread::ptr thr)
{
   if (!LWPIDsAreUnique())
      return;
   int_thread *llthrd = thr->llthrd();
   assert(llthrd);
   auto i = lwps.find(llthrd->getLWP());
   addDeadThread_nolock(llthrd->getLWP());
   assert(i != lwps.end());
   assert(i->second == thr);
   // Unregister only: the wrapper<->impl link stays intact until destroy.
   lwps.erase(i);
}

void ProcessPool::rmProcess_nolock(Process::ptr proc)
{
   int_process *llproc = proc->llproc();
   assert(llproc);
   pthrd_printf("Removing process %d from pool\n", llproc->getPid());
   auto i = procs.find(llproc->getPid());
   assert(i != procs.end());
   assert(i->second == proc);

   // Sweep this process's still-registered threads out of lwps first (their
   // wrappers come from the threadpool, so no impl->wrapper resolution).
   int_threadPool *tpool = llproc->threadPool();
   if (tpool && LWPIDsAreUnique()) {
      for (auto t : *tpool) {
         auto li = lwps.find(t->getLWP());
         if (li != lwps.end() && li->second->llthrd() == t)
            rmThread_nolock(li->second);
      }
   }
   procs.erase(i);
}

// ---- public container API (take map_lock once, delegate) ------------------

Process::ptr ProcessPool::findProcByPid(Dyninst::PID pid)
{
   ScopeLock<Mutex<> > guard(map_lock);
   return findProcByPid_nolock(pid);
}

Thread::ptr ProcessPool::findThread(Dyninst::LWP lwp)
{
   ScopeLock<Mutex<> > guard(map_lock);
   return findThread_nolock(lwp);
}

void ProcessPool::addProcess(Process::ptr proc)
{
   ScopeLock<Mutex<> > guard(map_lock);
   int_process *llproc = proc->llproc();
   assert(llproc);
   pthrd_printf("Adding process %d to pool\n", llproc->getPid());

   auto i = procs.find(llproc->getPid());
   assert(i == procs.end());
   procs[llproc->getPid()] = proc;
}

void ProcessPool::rmProcess(Process::ptr proc)
{
   ScopeLock<Mutex<> > guard(map_lock);
   rmProcess_nolock(proc);
}

void ProcessPool::addThread(Thread::ptr wrapper)
{
   ScopeLock<Mutex<> > guard(map_lock);
   assert(wrapper);
   int_thread *thr = wrapper->llthrd();
   assert(thr);
   if (!LWPIDsAreUnique())
      return;   // PROTOTYPE: no wrapper storage on non-unique-LWP platforms!
   auto i = lwps.find(thr->getLWP());
   assert(i == lwps.end());
   lwps[thr->getLWP()] = wrapper;
   // Interior refactor: seed the wrapper->wrapper cache (Thread knows its
   // Process) so downstream code never resolves impl->wrapper.
   auto pi = procs.find(thr->llproc()->getPid());
   if (pi != procs.end())
      wrapper->proc_wrapper_ = pi->second;
   // Un-kill if a LWP has been recycled (because we've run long enough?)
   auto found = deadThreads.find(thr->getLWP());
   if(found != deadThreads.end()) deadThreads.erase(found);
}

void ProcessPool::rmThread(Thread::ptr thr)
{
   ScopeLock<Mutex<> > guard(map_lock);
   rmThread_nolock(thr);
}

// ---- destruction: check+unregister under the lock, publish+delete outside -

// ProcScopeLock (the per-process migration lock, design 1) is defined in
// int_process.h so the generator (linux.C) can take the same lock.

void ProcessPool::destroyProcess(Process::ptr proc)
{
   assert(proc);
   int_process *llproc = proc->llproc();
   if (!llproc)
      return;   // already destroyed
   // Hold this process's migration lock across the whole teardown.  Today
   // this is always under work_lock (uncontended); it becomes load-bearing
   // once the generator refactor (step 2) has the generator take the same
   // lock around its decode-time use of the impl.
   ProcScopeLock plock(proc);
   // Atomic check-and-unregister: the whole find-then-remove is one locked
   // step (no TOCTOU with a concurrent mutation).  The wrapper<->impl links
   // stay intact through this -- rmProcess's sweep needs them.
   {
      ScopeLock<Mutex<> > guard(map_lock);
      if (findProcByPid_nolock(llproc->getPid()) == proc)
         rmProcess_nolock(proc);
   }
   // Outside map_lock: publish exit state, sever, delete.  delete re-enters
   // the pool via ~int_threadPool -> rmThread (which takes map_lock fresh),
   // so this must NOT run under the leaf lock.  Thread wrappers come from the
   // threadpool and the proc wrapper is passed down, so nothing resolves
   // impl->wrapper after unregistration.
   if (llproc->threadPool())
      llproc->threadPool()->destroyAllThreads(proc);
   llproc->publishExitState(proc);
   proc->clearLLProc();
   delete llproc;
}

void ProcessPool::destroyThread(Thread::ptr thr)
{
   assert(thr);
   int_thread *llthrd = thr->llthrd();
   if (!llthrd)
      return;   // already destroyed
   // Per-process granularity: a thread teardown takes its process's lock
   // (the wrapper is cached on the thread, resolved lock-free).
   ProcScopeLock plock(thr->procWrapperInternal());
   {
      ScopeLock<Mutex<> > guard(map_lock);
      if (findThread_nolock(llthrd->getLWP()) == thr)
         rmThread_nolock(thr);
   }
   // Outside map_lock (see destroyProcess).
   Process::ptr pw = thr->proc_wrapper_ ? thr->proc_wrapper_ : llthrd->proc();
   llthrd->publishExitState(thr, pw);
   thr->clearLLThread();
   delete llthrd;
}

// ---- impl -> wrapper resolution (starved to zero on the hot paths) --------

// PROTOTYPE-MEASURE: runtime starvation counters for the interior refactor.
// Goal: zero calls -- every wrapper should travel with the control flow.
static long wrapperfor_proc_calls = 0;
static long wrapperfor_thread_calls = 0;
static void report_wrapperfor_calls()
{
   // Silent when fully starved; any output is a starvation regression.
   if (wrapperfor_proc_calls || wrapperfor_thread_calls)
      fprintf(stderr, "PROTOTYPE-COUNT: wrapperFor calls proc=%ld thread=%ld\n",
              wrapperfor_proc_calls, wrapperfor_thread_calls);
}
static struct wrapperfor_reporter_t {
   wrapperfor_reporter_t() { atexit(report_wrapperfor_calls); }
} wrapperfor_reporter;

Process::ptr ProcessPool::wrapperFor(int_process *proc)
{
   ScopeLock<Mutex<> > guard(map_lock);
   wrapperfor_proc_calls++;
   auto i = procs.find(proc->getPid());
   if (i != procs.end() && i->second->llproc() == proc)
      return i->second;
   // PROTOTYPE tripwire: impl->wrapper resolution after unregistration.
   fprintf(stderr, "PROTOTYPE: post-unregister proc() for pid %d\n",
           proc->getPid());
   return Process::ptr();
}

Thread::ptr ProcessPool::wrapperFor(int_thread *thr)
{
   ScopeLock<Mutex<> > guard(map_lock);
   wrapperfor_thread_calls++;
   if (LWPIDsAreUnique()) {
      auto i = lwps.find(thr->getLWP());
      if (i != lwps.end() && i->second->llthrd() == thr)
         return i->second;
   }
   // PROTOTYPE tripwire (see the proc overload).
   fprintf(stderr, "PROTOTYPE: post-unregister thread() for lwp %d\n",
           (int)thr->getLWP());
   abort();
   return Thread::ptr();
}

// ---- iteration: snapshot under the lock, run callbacks outside it ----------

bool ProcessPool::for_each(ifunc f, void *data)
{
   // Snapshot under map_lock, then release before invoking the callback: f is
   // arbitrary code (it may re-enter the pool or block), which must never run
   // under the leaf lock.
   std::vector<Process::ptr> snapshot;
   {
      ScopeLock<Mutex<> > guard(map_lock);
      snapshot.reserve(procs.size());
      for (auto &kv : procs)
         snapshot.push_back(kv.second);
   }
   for (auto &p : snapshot) {
      int_process *llproc = p->llproc();
      if (!llproc)
         continue;   // unregistered/destroyed since the snapshot
      if (!f(llproc, data))
         return false;
   }
   return true;
}

CondVar<> *ProcessPool::condvar()
{
   return &var;
}

// ---- deadThreads accessors (guarded; recycled-LWP tombstones) -------------

bool ProcessPool::deadThread(Dyninst::LWP lwp) {
   ScopeLock<Mutex<> > guard(map_lock);
   return (deadThreads.find(lwp) != deadThreads.end());
}

void ProcessPool::addDeadThread(Dyninst::LWP lwp) {
   ScopeLock<Mutex<> > guard(map_lock);
   addDeadThread_nolock(lwp);
}

void ProcessPool::removeDeadThread(Dyninst::LWP lwp) {
    // Called when we get a LWP create, as that had *better*
    // not be for an alread-dead thread.
    ScopeLock<Mutex<> > guard(map_lock);
    deadThreads.erase(lwp);
}

unsigned ProcessPool::numProcs()
{
   ScopeLock<Mutex<> > guard(map_lock);
   return (unsigned) procs.size();
}
