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

using Dyninst::ProcControlAPI::Process;
using Dyninst::ProcControlAPI::Thread;

#include <assert.h>

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

Process::ptr ProcessPool::findProcByPid(Dyninst::PID pid)
{
   ScopeLock<Mutex<true> > guard(map_lock);
   auto i = procs.find(pid);
   if (i == procs.end())
      return Process::ptr();
   return i->second;
}

void ProcessPool::addProcess(Process::ptr proc)
{
   ScopeLock<Mutex<true> > guard(map_lock);
   int_process *llproc = proc->llproc();
   assert(llproc);
   pthrd_printf("Adding process %d to pool\n", llproc->getPid());

   auto i = procs.find(llproc->getPid());
   assert(i == procs.end());
   procs[llproc->getPid()] = proc;
}

void ProcessPool::rmProcess(Process::ptr proc)
{
   ScopeLock<Mutex<true> > guard(map_lock);
   int_process *llproc = proc->llproc();
   assert(llproc);
   pthrd_printf("Removing process %d from pool\n", llproc->getPid());
   auto i = procs.find(llproc->getPid());
   assert(i != procs.end());
   assert(i->second == proc);

   // Unregister only: the pool drops its strong refs, but the wrapper<->impl
   // links stay intact -- post-exit handlers and callbacks still traverse
   // them.  Publish + sever + delete happen in destroyProcess.
   int_threadPool *tpool = llproc->threadPool();
   if (tpool) {
      for (auto t : *tpool) {
         if (LWPIDsAreUnique()) {
            auto li = lwps.find(t->getLWP());
            if (li != lwps.end() && li->second->llthrd() == t)
               rmThread(li->second);
         }
      }
   }
   procs.erase(i);
}

void ProcessPool::destroyProcess(Process::ptr proc)
{
   assert(proc);
   int_process *llproc = proc->llproc();
   if (!llproc)
      return;   // already destroyed
   // Order matters: unregister FIRST, while the wrapper<->impl links are
   // intact -- rmProcess's thread sweep and rmThread's identity asserts
   // check them.  Then publish + sever, then delete.  (Thread wrappers come
   // from the threadpool's hl_threads and the proc wrapper is passed down,
   // so nothing resolves impl->wrapper after unregistration.)
   if (findProcByPid(llproc->getPid()) == proc)
      rmProcess(proc);
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
   // Unregister first, while the link is intact (see destroyProcess).
   if (findThread(llthrd->getLWP()) == thr)
      rmThread(thr);
   Process::ptr pw = thr->proc_wrapper_ ? thr->proc_wrapper_ : llthrd->proc();
   llthrd->publishExitState(thr, pw);
   thr->clearLLThread();
   delete llthrd;
}

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
   ScopeLock<Mutex<true> > guard(map_lock);
   wrapperfor_proc_calls++;
   auto i = procs.find(proc->getPid());
   if (i != procs.end() && i->second->llproc() == proc)
      return i->second;
   // PROTOTYPE tripwire: impl->wrapper resolution after unregistration.
   // Measured across the full testsuite, the only such caller was the thread
   // dtors' exitstate publish, now satisfied by int_process::destroy passing
   // the wrapper down.  Anything printing here is an unplumbed site.
   fprintf(stderr, "PROTOTYPE: post-unregister proc() for pid %d\n",
           proc->getPid());
   return Process::ptr();
}

Thread::ptr ProcessPool::wrapperFor(int_thread *thr)
{
   ScopeLock<Mutex<true> > guard(map_lock);
   wrapperfor_thread_calls++;
   if (LWPIDsAreUnique()) {
      auto i = lwps.find(thr->getLWP());
      if (i != lwps.end() && i->second->llthrd() == thr)
         return i->second;
   }
   // PROTOTYPE tripwire (see the proc overload): thread() never resolved
   // post-unregister anywhere in the full testsuite.
   fprintf(stderr, "PROTOTYPE: post-unregister thread() for lwp %d\n",
           (int)thr->getLWP());
   abort();
   return Thread::ptr();
}

bool ProcessPool::for_each(ifunc f, void *data)
{
	condvar()->lock();
   for (auto i = procs.begin(); i != procs.end(); ++i) {
      bool result = f(i->second->llproc(), data);
	  if (!result) {
			condvar()->broadcast();
			condvar()->unlock();
		  return false;
	  }
   }
	condvar()->broadcast();
	condvar()->unlock();
   return true;
}

CondVar<> *ProcessPool::condvar()
{
   return &var;
}

void ProcessPool::addThread(Thread::ptr wrapper)
{
   ScopeLock<Mutex<true> > guard(map_lock);
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
   // Un-kill if a LWP has been recycled 
   // (because we've run long enough?)
   auto found = deadThreads.find(thr->getLWP());
   if(found != deadThreads.end()) deadThreads.erase(found);
   
}

void ProcessPool::rmThread(Thread::ptr thr)
{
   ScopeLock<Mutex<true> > guard(map_lock);
   if (!LWPIDsAreUnique())
      return;
   int_thread *llthrd = thr->llthrd();
   assert(llthrd);
   auto i = lwps.find(llthrd->getLWP());
   addDeadThread(llthrd->getLWP());
   assert(i != lwps.end());
   assert(i->second == thr);
   // Unregister only (see rmProcess).
   lwps.erase(i);
}

Thread::ptr ProcessPool::findThread(Dyninst::LWP lwp)
{
   ScopeLock<Mutex<true> > guard(map_lock);
   if (!LWPIDsAreUnique()) {
      return Thread::ptr();
   }
   auto i = lwps.find(lwp);
   if (i == lwps.end())
      return Thread::ptr();
   return i->second;
}

bool ProcessPool::deadThread(Dyninst::LWP lwp) {
   return (deadThreads.find(lwp) != deadThreads.end());
}

void ProcessPool::addDeadThread(Dyninst::LWP lwp) {
   deadThreads.insert(lwp);
}
void ProcessPool::removeDeadThread(Dyninst::LWP lwp) {
    // Called when we get a LWP create, as that had *better*
    // not be for an alread-dead thread.
    deadThreads.erase(lwp);
}

unsigned ProcessPool::numProcs()
{
   return (unsigned) procs.size();
}
