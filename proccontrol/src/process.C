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
#include "dyninstversion.h"
#include "int_process.h"
#include "irpc.h"
#include "procpool.h"
#include "int_handler.h"
#include "response.h"
#include "int_event.h"
#include "processplat.h"
#include "Mailbox.h"
#include "PCErrors.h"
#include "Generator.h"
#include "Event.h"
#include "Handler.h"
#include "ProcessSet.h"
#include "PlatFeatures.h"

#if defined(os_windows)
#include "windows_process.h"
#include "windows_thread.h"
#endif

#include "loadLibrary/injector.h"

#include <climits>
#include <cstring>
#include <cassert>
#include <map>
#include <sstream>
#include <iostream>
#include <iterator>
#include <errno.h>

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

const map<int,int> Process::emptyFDs;
const vector<string> Process::emptyEnvp;
Process::thread_mode_t threadingMode = Process::GeneratorThreading;
bool int_process::in_callback = false;
std::set<int_thread::continue_cb_t> int_thread::continue_cbs;
SymbolReaderFactory *int_process::user_set_symbol_reader = NULL;

static const int ProcControl_major_version = DYNINST_MAJOR_VERSION;
static const int ProcControl_minor_version = DYNINST_MINOR_VERSION;
static const int ProcControl_maintenance_version = DYNINST_PATCH_VERSION;

bool Dyninst::ProcControlAPI::is_restricted_ptrace = false;



void Process::version(int& major, int& minor, int& maintenance)
{
    major = ProcControl_major_version;
    minor = ProcControl_minor_version;
    maintenance = ProcControl_maintenance_version;
}

bool int_process::create(int_processSet *ps) {
   bool had_error = false;
   set<int_process *> procs;
   transform(ps->begin(), ps->end(), inserter(procs, procs.end()), ProcToIntProc());

   //Should be called with procpool lock held
   pthrd_printf("Calling plat_create for %d processes\n", (int) procs.size());
   for (set<int_process *>::iterator i = procs.begin(); i != procs.end(); ) {
      int_process *proc = *i;
      bool result = proc->plat_create();
      if (!result) {
         pthrd_printf("Could not create debuggee, %s\n", proc->executable.c_str());
         proc->setLastError(err_noproc, "Could not create process");
         i = procs.erase(i);
         had_error = true;
         continue;
      }
      i++;
   }

   pthrd_printf("Creating initial threads for %d processes\n", (int) procs.size());
   for (set<int_process *>::iterator i = procs.begin(); i != procs.end(); i++) {
      int_process *proc = *i;
	  // Because yo, windows processes have threads when they're created...
	  if (proc->threadPool()->empty()) {
        int_thread::createThread(proc, NULL_THR_ID, NULL_LWP, true, int_thread::as_created_attached);
	  }
	  ProcPool()->addProcess(proc);
      proc->setState(neonatal_intermediate);
      pthrd_printf("Created debugged %s on pid %d\n", proc->executable.c_str(), proc->pid);
   }

   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();


   pthrd_printf("Waiting for startup for %d processes\n", (int) procs.size());
   for (set<int_process *>::iterator i = procs.begin(); i != procs.end(); ) {
      int_process *proc = *i;
      string executable = proc->executable;
      Dyninst::PID pid = proc->pid;

      bool result = proc->waitfor_startup();
      if (!result) {
         // At this point, proc has been deleted
         pthrd_printf("Process %s/%d exited during create\n", executable.c_str(), pid);
         i = procs.erase(i);
         had_error = true;
         continue;
      }
      i++;
   }

   pthrd_printf("Triggering post-create for %d processes\n", (int) procs.size());
   while (!procs.empty()) {
      set<response::ptr> async_responses;
      bool ret_async = false;
      for (set<int_process *>::iterator i = procs.begin(); i != procs.end(); ) {
         int_process *proc = *i;

         async_ret_t result = proc->post_create(async_responses);
         if (result == aret_error) {
            pthrd_printf("Error during post create for %d\n", proc->pid);
            had_error = true;
            i = procs.erase(i);
         }
         else if (result == aret_success) {
            assert(proc->getState() == running);
            pthrd_printf("Finished post-create for %d.  Process is ready\n", proc->pid);
            i = procs.erase(i);
         }
         else {
            pthrd_printf("post-create for %d return async\n", proc->pid);
            ret_async = true;
            i++;
         }
      }
      if (ret_async) {
         waitForAsyncEvent(async_responses);
      }
   }

   return !had_error;
}

bool int_process::waitfor_startup()
{
   bool proc_exited = false;
   while (getState() != running) {
      if (proc_exited || getState() == exited) {
         pthrd_printf("Error.  Process exited during create/attach\n");
         globalSetLastError(err_exited, "Process exited during startup");
         return false;
      }
      pthrd_printf("Waiting for startup to complete for %d\n", pid);
      bool result = waitAndHandleForProc(true, this, proc_exited);
      if (!proc_exited && (!result || getState() == errorstate)) {
         pthrd_printf("Error.  Process %d errored during create/attach\n", pid);
         globalSetLastError(err_internal, "Process failed to startup");
         return false;
      }
      if (proc_exited || getState() == exited) {
         pthrd_printf("Error.  Proces exited during create/attach\n");
         globalSetLastError(err_exited, "Process exited during startup");
         return false;
      }
      if (getState() == errorstate) {
         pthrd_printf("Error.  Process in error state\n");
         globalSetLastError(err_internal, "Process errored during startup");
         return false;
      }
   }
   return true;
}

void int_process::plat_threadAttachDone()
{
}

bool int_process::attachThreads(bool &found_new_threads)
{
   found_new_threads = false;

   if (!needIndividualThreadAttach())
      return true;

   /**
    * This OS (linux) needs us to attach to each thread manually.
    * Get all the thread LWPs and create new thread objects for them.
    * After we have all the threads, check again to see if there are any
    * new ones.  We're dealing with the race condition where we could get
    * a list of LWPs, but then new threads are created before we attach to
    * all the existing threads.
    **/
   bool loop_new_threads;
   do {
      loop_new_threads = false;
      vector<Dyninst::LWP> lwps;
      bool result = getThreadLWPs(lwps);
      if (!result) {
         pthrd_printf("Failed to get thread LWPs for %d\n", pid);
         return false;
      }

      for (vector<Dyninst::LWP>::iterator i = lwps.begin(); i != lwps.end(); ++i) {
         int_thread *thr = threadpool->findThreadByLWP(*i);
         if (thr) {
            pthrd_printf("Already have thread %d in process %d\n", *i, pid);
            continue;
         }
         pthrd_printf("Creating new thread for %d/%d during attach\n", pid, *i);
         thr = int_thread::createThread(this, NULL_THR_ID, *i, false, int_thread::as_needs_attach);
         found_new_threads = loop_new_threads = true;
      }
   } while (loop_new_threads);

   return true;
}

bool int_process::attachThreads()
{
   bool found_new_threads = false;
   return attachThreads(found_new_threads);
}

bool int_process::plat_attachThreadsSync()
{
   // By default, platforms just call the idempotent attachThreads().
   // Some platforms may override, e.g. Linux should sync with all threads.
   if (!attachThreads()) {
      pthrd_printf("Failed to attach to threads in %d\n", pid);
      setLastError(err_internal, "Could not get threads during attach\n");
      return false;
   }
   return true;
}

bool int_process::attach(int_processSet *ps, bool reattach)
{
   bool had_error = false, should_sync = false;
   set<int_process *> procs;
   vector<Event::ptr> observedEvents;
   set<response::ptr> async_responses;
   transform(ps->begin(), ps->end(), inserter(procs, procs.end()), ProcToIntProc());

   //Should be called with procpool lock held

	pthrd_printf("Calling plat_attach for %d processes\n", (int) procs.size());
   map<pair<int_process *, Dyninst::LWP>, bool> runningStates;
   for (set<int_process *>::iterator i = procs.begin(); i != procs.end();) {
	   int_process *proc = *i;
      if (!proc) {
         i = procs.erase(i);
         continue;
      }
      if (reattach && proc->getState() != int_process::detached) {
         perr_printf("Attempted to reattach to attached process %d\n", proc->getPid());
         proc->setLastError(err_attached, "Cannot reAttach to attached process.\n");
         i = procs.erase(i);
         had_error = true;
         continue;
      }

      // Determine the running state of all threads before attaching
      map<Dyninst::LWP, bool> temp_runningStates;
      if (!proc->plat_getOSRunningStates(temp_runningStates)) {
         pthrd_printf("Could not get OS running states for %d\n", proc->getPid());
         i = procs.erase(i);
         had_error = true;
         continue;
      }

      //Keep track of the initial running states for each thread.  We'll fill them in latter
      // after we create the int_thread objects.
      bool allStopped = true;
      for(map<Dyninst::LWP, bool>::iterator j = temp_runningStates.begin(); j != temp_runningStates.end(); ++j) {
         if (j->second) {
            allStopped = false;
         }
         runningStates[pair<int_process *, LWP>(proc, j->first)] = j->second;
      }

      bool local_should_sync = false;
      pthrd_printf("Calling plat_attach for process %d\n", proc->getPid());
      bool result = proc->plat_attach(allStopped, local_should_sync);
      if (!result) {
         pthrd_printf("Failed to plat_attach to %d\n", proc->getPid());
         i = procs.erase(i);
         proc->setLastError(err_noproc, "Could not attach to process");
         had_error = true;
         continue;
      }
      if (local_should_sync)
         should_sync = true;
      i++;
   }

   //Create the int_thread objects via attach_threads
   if (!reattach) {
      for (set<int_process *>::iterator i = procs.begin(); i != procs.end(); i++) {
         int_process *proc = *i;
         ProcPool()->addProcess(proc);
         int_thread::createThread(proc, NULL_THR_ID, NULL_LWP, true,
                                  int_thread::as_created_attached); //initial thread
      }
   }

   if (should_sync) {
      ProcPool()->condvar()->broadcast();
      ProcPool()->condvar()->unlock();
      for (;;) {
         bool have_neonatal = false;
         for (set<int_process *>::iterator i = procs.begin(); i != procs.end(); i++) {
            int_process *proc = *i;
            if (proc->getState() == neonatal) {
               have_neonatal = true;
               break;
            }
         }
         if (!have_neonatal)
            break;
         bool result = waitAndHandleEvents(true);
         if (!result) {
            pthrd_printf("Error during waitAndHandleEvents during attach\n");
            return false;
         }
      }
      ProcPool()->condvar()->lock();
   }
   else {
      pthrd_printf("Attach done, moving processes to neonatal_intermediate\n");
      for (set<int_process *>::iterator i = procs.begin(); i != procs.end(); i++) {
         int_process *proc = *i;
         proc->setState(neonatal_intermediate);
      }
   }


   for (set<int_process *>::iterator i = procs.begin(); i != procs.end(); ) {
      int_process *proc = *i;
      if (proc->getState() == errorstate) {
         pthrd_printf("Removing process %d in error state\n", proc->getPid());
         i = procs.erase(i);
         had_error = true;
         continue;
      }
      pthrd_printf("Attaching to threads for %d\n", proc->getPid());
      bool result = proc->attachThreads();
      if (!result) {
         pthrd_printf("Failed to attach to threads in %d\n", proc->pid);
         i = procs.erase(i);
         had_error = true;
         continue;
      }

      if (reattach) {
         // Now, go back and set non-existing threads to detached to exclude them
         // from the bootstrap handling, creating thread destruction events for
         // these non-existing threads
         //
         // Also, at the same time issue attaches to existing threads
         int_threadPool *tp = proc->threadPool();
         for(int_threadPool::iterator j = tp->begin(); j != tp->end(); j++)
         {
            int_thread *thr = *j;

            if (thr->getDetachState().getState() != int_thread::detached) {
               //Small hack: This thread is new since we're reattaching.  Every old
               //thread has it's DetachState in 'detached', and we'd like to unset
               //those for the entire process.  Temporarily set this thread to detached
               //so that it gets properly restored with the rest.
               thr->getDetachState().desyncState(int_thread::detached);
               continue;
            }

            pair<int_process *, LWP> key(proc, thr->getLWP());
            map<pair<int_process *, LWP>, bool>::iterator findIter = runningStates.find(key);

            pthrd_printf("Re-attaching to thread %d/%d\n", proc->getPid(), thr->getLWP());
            if (findIter == runningStates.end() || !thr->attach()) {
               pthrd_printf("Creating thread destroy event for thread %d/%d\n", proc->getPid(), thr->getLWP());
               thr->getGeneratorState().setState(int_thread::detached);
               thr->getHandlerState().setState(int_thread::detached);
               thr->getUserState().setState(int_thread::detached);
               thr->getDetachState().setState(int_thread::detached);

               Event::ptr destroyEv;
               if (proc->plat_supportLWPPostDestroy()) {
                  destroyEv = Event::ptr(new EventLWPDestroy(EventType::Post));
               }
               else if (proc->plat_supportThreadEvents()) {
                  destroyEv = Event::ptr(new EventUserThreadDestroy(EventType::Post));
               }
               else {
                  perr_printf("Platform does not support any thread destroy events.  Now what?\n");
                  assert(0);
               }

               destroyEv->setProcess(proc->proc());
               destroyEv->setThread(thr->thread());
               destroyEv->setSyncType(Event::async);
               destroyEv->setUserEvent(true);
               observedEvents.push_back(destroyEv);
            }
         }
         proc->threadPool()->initialThread()->getDetachState().restoreStateProc();
      }
      i++;
   }

   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();

   //Wait for each process to make it to the 'running' state.
   for (set<int_process *>::iterator i = procs.begin(); i != procs.end(); ) {
      int_process *proc = *i;
      pthrd_printf("Wait for attach from process %d\n", proc->pid);

      bool result = proc->waitfor_startup();
      if (!result) {
         pthrd_printf("Error waiting for attach to %d\n", proc->pid);
         i = procs.erase(i);
         had_error = true;
         continue;
      }
      i++;
   }

   //Some OSs need to do their attachThreads here.  Since the operation is supposed to be
   //idempotent after success, then just do it again.
   for (set<int_process *>::iterator i = procs.begin(); i != procs.end(); ) {
      int_process *proc = *i;
      if (proc->getState() == errorstate)
         continue;
      bool result = proc->plat_attachThreadsSync();
      if (!result) {
         pthrd_printf("Failed to attach to threads in %d\n", proc->pid);
         i = procs.erase(i);
         had_error = true;
         continue;
      }

      // Now that all the threads are created, set their running states
      int_threadPool *tp = proc->threadPool();
      for(int_threadPool::iterator j = tp->begin(); j != tp->end(); j++) {
         int_thread *thr = *j;
         pair<int_process *, LWP> key(proc, thr->getLWP());
         map<pair<int_process *, LWP>, bool>::iterator findIter = runningStates.find(key);

         // There is a race that could be visible here where we are not
         // guaranteed to determine the running state of all threads in a process
         // before we attach -- if for some reason we don't know the running
         // state, assume it was running
         thr->setRunningWhenAttached(findIter == runningStates.end() ? true : findIter->second);
      }

      pthrd_printf("Thread attach is done for process %d\n", proc->getPid());
      proc->plat_threadAttachDone();
      i++;
   }

   pthrd_printf("Triggering post-attach for %d processes\n", (int) procs.size());
   std::set<int_process *> pa_procs = procs;
   while (!pa_procs.empty()) {
      async_responses.clear();
      bool ret_async = false;
      for (set<int_process *>::iterator i = pa_procs.begin(); i != pa_procs.end(); ) {
         int_process *proc = *i;

         async_ret_t result = proc->post_attach(false, async_responses);
         if (result == aret_error) {
            pthrd_printf("Error during post attach for %d\n", proc->pid);
            had_error = true;
            i = pa_procs.erase(i);
         }
         else if (result == aret_success) {
            assert(proc->getState() == running);
            pthrd_printf("Finished post-attach for %d.  Process is ready\n", proc->pid);
            i = pa_procs.erase(i);
         }
         else {
            pthrd_printf("post-attach for %d return async\n", proc->pid);
            ret_async = true;
            i++;
         }
      }
      if (ret_async) {
         waitForAsyncEvent(async_responses);
      }
   }

   //
   //Everything below this point is targeted at DOTF reattach--
   //reinstalling BPs and throwing dead thread events.
   if (!reattach)
      return !had_error;

   async_responses.clear();
   for (set<int_process *>::iterator i = procs.begin(); i != procs.end(); i++) {
      // Resume all breakpoints
      int_process *proc = *i;
      for(std::map<Dyninst::Address, sw_breakpoint *>::iterator j = proc->mem->breakpoints.begin();
          j != proc->mem->breakpoints.end(); j++)
      {
         pthrd_printf("Resuming breakpoint at 0x%lx in process %d\n", j->first, proc->getPid());
         bool result = j->second->resume(proc, async_responses);
         if(!result) {
            pthrd_printf("Error resuming breakpoint at %lx\n", j->first);
            //Drop this error.  The library containing this BP was likely unloaded.
         }
      }
   }
   waitForAsyncEvent(async_responses);

   // Report all events for observed process state changes
   for(vector<Event::ptr>::iterator i = observedEvents.begin(); i!= observedEvents.end(); i++)
   {
       int_thread *thrd = (*i)->getThread()->llthrd();
       pthrd_printf("Queuing event %s for thread %d/%d\n",
                    (*i)->getEventType().name().c_str(), (*i)->getProcess()->getPid(),
                    (*i)->getThread()->getLWP());

       // Make sure the thread is the correct state while in event handling
       thrd->getGeneratorState().setState(int_thread::detached);
       thrd->getHandlerState().setState(int_thread::detached);
       thrd->getUserState().setState(int_thread::detached);
       thrd->getDetachState().setState(int_thread::detached);

	   mbox()->enqueue(*i);
   }

   if (!observedEvents.empty()) {
      // As a sanity check, don't block
      bool result = waitAndHandleEvents(false);
      if (!result) {
         perr_printf("Internal error in waitAndHandleEvents under reattach\n");
      }
   }

   return !had_error;
}

bool int_process::reattach(int_processSet *pset)
{
   return attach(pset, true);
}

bool int_process::execed()
{
   ProcPool()->condvar()->lock();

   bool should_clean = false;
   mem->rmProc(this, should_clean);
   if (should_clean)
      delete mem;
   mem = new mem_state(this);

   arch = Dyninst::Arch_none;
   exec_mem_cache.clear();

   int_thread::State user_initial_thrd_state = threadpool->initialThread()->getUserState().getState();
   int_thread::State gen_initial_thrd_state = threadpool->initialThread()->getGeneratorState().getState();
   int_thread::State handler_initial_thrd_state = threadpool->initialThread()->getHandlerState().getState();

   int_threadPool::iterator i = threadpool->begin();
   for (; i != threadpool->end(); ++i) {
      int_thread *thrd = *i;
      thrd->getUserState().setState(int_thread::exited);
      thrd->getGeneratorState().setState(int_thread::exited);
      ProcPool()->rmThread(thrd);
      delete thrd;
   }
   threadpool->clear();

   int_thread *initial_thread = int_thread::createThread(this, NULL_THR_ID, NULL_LWP,
                                                         true, int_thread::as_created_attached);
   initial_thread->getUserState().setState(user_initial_thrd_state);
   initial_thread->getGeneratorState().setState(gen_initial_thrd_state);
   initial_thread->getHandlerState().setState(handler_initial_thrd_state);

   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();

   bool result = plat_execed();

   return result;
}

bool int_process::plat_execed()
{
   return true;
}

bool int_process::forked()
{
   ProcPool()->condvar()->lock();

   pthrd_printf("Setting up forked process %d\n", pid);
   creation_mode = ct_fork;
   bool result = plat_forked();
   if (!result) {
      pthrd_printf("Could not handle forked debuggee, %d\n", pid);
      return false;
   }

   int_thread *initial_thread;
   initial_thread = int_thread::createThread(this, NULL_THR_ID, NULL_LWP, true, int_thread::as_created_attached);
   (void)initial_thread; // suppress unused warning

   ProcPool()->addProcess(this);

   result = attachThreads();
   if (!result) {
      pthrd_printf("Failed to attach to threads in %d\n", pid);
      setLastError(err_internal, "Could not attach to process' threads");
      goto error;
   }

   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();

   result = post_forked();
   if (!result) {
      pthrd_printf("Post-fork failed on %d\n", pid);
      setLastError(err_internal, "Error handling forked process");
      goto error;
   }
   return true;

  error:
   if (getState() == exited) {
      setLastError(err_exited, "Process exited unexpectedly during attach\n");
      return false;
   }
   pthrd_printf("Error during process attach for %d\n", pid);
   return false;
}

bool int_process::post_forked()
{
   setState(running);
   return true;
}

async_ret_t int_process::initializeAddressSpace(std::set<response::ptr> &async_responses)
{
   std::set<int_library*> added, rmd;

   //This call will determine and cache the appropriate symbol reader, or NOP
   // if that's already been done.
   getSymReader();

   bool have_asyncs = false;
   bool result = refresh_libraries(added, rmd, have_asyncs, async_responses);
   if (!result && have_asyncs) {
      pthrd_printf("Postponing initializeAddressSpace of %d for async operations\n", getPid());
      return aret_async;

   }
   if (!result) {
      pthrd_printf("Failure refreshing libraries for %d\n", getPid());
      return aret_error;
   }
   pthrd_printf("Successfully initialized address space for %d\n", getPid());
   return aret_success;
}

async_ret_t int_process::post_attach(bool, std::set<response::ptr> &async_responses)
{
   pthrd_printf("Starting post_attach for process %d\n", getPid());
   return initializeAddressSpace(async_responses);
}

async_ret_t int_process::post_create(std::set<response::ptr> &async_responses)
{
   pthrd_printf("Starting post_create for process %d\n", getPid());
   return initializeAddressSpace(async_responses);
}

bool int_process::plat_processGroupContinues()
{
   return false;
}

bool int_process::getThreadLWPs(std::vector<Dyninst::LWP> &)
{
   return false;
}

const char *int_process::stateName(int_process::State s)
{
   switch (s) {
      case neonatal: return "neonatal";
      case neonatal_intermediate: return "neonatal_intermediate";
      case detached: return "detached";
      case running: return "running";
      case exited: return "exited";
      case errorstate: return "errorstate";
   }
   assert(0);
   return NULL;
}

void int_process::setState(int_process::State s)
{
   int old_state = (int) state;
   int new_state = (int) s;

   if (new_state < old_state && new_state != detached && old_state != detached) {
      perr_printf("Regression of state %s to %s on %d\n",
                  stateName(state), stateName(s), pid);
      return;
   }
   pthrd_printf("Changing state of process from %s to %s on %d\n",
                stateName(state), stateName(s), pid);
   state = s;


   int_thread::State new_thr_state = int_thread::errorstate;
   switch (s) {
      case neonatal: new_thr_state = int_thread::neonatal; break;
      case neonatal_intermediate: new_thr_state = int_thread::neonatal_intermediate; break;
      case running: new_thr_state = threadpool->initialThread()->getHandlerState().getState(); break;
      case exited: new_thr_state = int_thread::exited; break;
      case detached: new_thr_state = int_thread::detached; break;
      case errorstate: new_thr_state = int_thread::errorstate; break;
   }
   pthrd_printf("Setting state of all threads in %d to %s\n", pid,
                int_thread::stateStr(new_thr_state));
   for (int_threadPool::iterator i = threadpool->begin(); i != threadpool->end(); ++i)
   {
      (*i)->getUserState().setState(new_thr_state);
      (*i)->getHandlerState().setState(new_thr_state);
      (*i)->getGeneratorState().setState(new_thr_state);
	  if(new_thr_state == int_thread::exited)
	  {
        if((*i)->getPendingStopState().getState() != int_thread::dontcare) {
           (*i)->getPendingStopState().restoreState();
           (*i)->pendingStopsCount().dec();
        }
     }
   }
}

int_process::State int_process::getState() const
{
   return state;
}

int_process::creationMode_t int_process::getCreationMode() const
{
   return creation_mode;
}

void int_process::setContSignal(int sig) {
    continueSig = sig;
}

int int_process::getContSignal() const {
    return continueSig;
}

void int_process::setPid(Dyninst::PID p)
{
   pthrd_printf("Setting int_process %p to pid %d\n", (void*)this, p);
   pid = p;
}

Dyninst::PID int_process::getPid() const
{
   return pid;
}

int_threadPool *int_process::threadPool() const
{
   return threadpool;
}

Process::ptr int_process::proc() const
{
   return up_proc;
}

bool int_process::syncRunState()
{
   pthrd_printf("At top of syncRunState for %d\n", getPid());
   int_threadPool *tp = threadPool();

   /**
    * We can have some events (currently BreakpointClear and RPCLaunch) that
    * get thrown when a thread is continued.  We'll create and throw those
    * events here.
    * 1), we'll identify each thread that would be continued.  We calculate
    * this ahead of time as the act of throwing one of these events on a thread
    * can change the running state of another thread.
    * 2), for each thread that would run, we check if it has one of these
    * events and throw it.
    * 3), Check if any proc stopper events are ready, and if so then re-throw them
    * 4), we redo the target_state calculation in-case anything changed.
    **/
   //1)
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      int_thread *thr = *i;
      int_thread::State new_state = thr->getActiveState().getState();
      if (new_state == int_thread::ditto)
         new_state = thr->getHandlerState().getState();
      thr->setTargetState(new_state);
   }

   //2)
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      int_thread *thr = *i;
      if (!RUNNING_STATE(thr->getTargetState()))
         continue;
      if (thr->getActiveState().getID() == int_thread::PendingStopStateID) {
         //A pending-stop-state continue isn't a real continue.  Don't throw
         // new events.
         continue;
      }
      thr->throwEventsBeforeContinue();
   }
   //3)
   pthrd_printf("Checking if any ProcStop events on %d are ready\n", getPid());
   getProcStopManager().checkEvents();
   //4)
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      int_thread *thr = *i;
      int_thread::State new_state = thr->getActiveState().getState();
      if (new_state == int_thread::ditto)
         new_state = thr->getHandlerState().getState();
      thr->setTargetState(new_state);
   }
   if (dyninst_debug_proccontrol) {
      pc_print_lock();
      pthrd_printf("Current Threading State for %d:\n", getPid());
      for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
         int_thread *thr = *i;
         int_thread::StateTracker &target = thr->getActiveState();

         char state_code[int_thread::NumStateIDs+1];
         for (int j = 0; j < int_thread::NumStateIDs; j++) {
            state_code[j] = int_thread::stateLetter(thr->getStateByID(j).getState());
         }
         state_code[int_thread::NumStateIDs] = '\0';
         pthrd_printf("%d/%d hand: %s, gen: %s, %s: %s, code: %s\n",
                      getPid(), thr->getLWP(),
                      int_thread::stateStr(thr->getHandlerState().getState()),
                      int_thread::stateStr(thr->getGeneratorState().getState()),
                      int_thread::stateIDToName(target.getID()).c_str(),
                      int_thread::stateStr(target.getState()),
                      state_code);
      }
	  if(tp->empty())
	  {
		  pthrd_printf("Threadpool for %d is empty\n", getPid());
	  }
     pc_print_unlock();
   }

   pthrd_printf("Running plat_syncRunState on %d\n", getPid());
   bool result = plat_syncRunState();

   return result;
}

bool int_process::waitForAsyncEvent(response::ptr resp)
{
  if (resp) {
    int_process *proc = resp->getProcess();
    if (proc)
       proc->plat_preAsyncWait();
  }
  return getResponses().waitFor(resp);
}

bool int_process::waitForAsyncEvent(std::set<response::ptr> resp)
{
   for (set<response::ptr>::iterator i = resp.begin(); i != resp.end(); i++) {
     response::ptr r = *i;
     if (!r)
       continue;
     int_process *proc = r->getProcess();
     assert(proc);
     proc->plat_preAsyncWait();
   }

   bool has_error = false;
   for (set<response::ptr>::iterator i = resp.begin(); i != resp.end(); i++) {
      bool result = getResponses().waitFor(*i);
      if (!result)
         has_error = true;
   }

   return !has_error;
}

Counter &int_process::asyncEventCount()
{
   return async_event_count;
}

Counter &int_process::getForceGeneratorBlockCount()
{
   return force_generator_block_count;
}

Counter &int_process::getStartupTeardownProcs()
{
   return startupteardown_procs;
}

bool int_process::plat_waitAndHandleForProc()
{
  return true;
}

int_process *int_process::in_waitHandleProc = NULL;

bool int_process::waitAndHandleForProc(bool block, int_process *proc, bool &proc_exited)
{
   assert(in_waitHandleProc == NULL);
   in_waitHandleProc = proc;

   if (!proc->plat_waitAndHandleForProc()) {
     perr_printf("Failed platform specific waitAndHandle for %d\n", proc->getPid());
     return false;
   }

   bool result = waitAndHandleEvents(block);

   if (proc->getState() == int_process::exited) {
      pthrd_printf("Deleting proc %d from waitAndHandleForProc\n", proc->getPid());
      delete proc;
      proc_exited = true;
   }
   else {
      proc_exited = false;
   }

   in_waitHandleProc = NULL;
   return result;
}

#define checkHandlerThread      (hasHandlerThread      = (int) isHandlerThread())
#define checkBlock              (hasBlock              = (int) block)
#define checkGotEvent           (hasGotEvent           = (int) gotEvent)
#define checkAsyncPending       (hasAsyncPending       = (int) HandlerPool::hasProcAsyncPending())
#define checkRunningThread      (hasRunningThread      = (int) Counter::global(Counter::HandlerRunningThreads))
#define checkClearingBP         (hasClearingBP         = (int) Counter::global(Counter::ClearingBPs))
#define checkStopPending        (hasStopPending        = (int) Counter::global(Counter::PendingStops))
#define checkSyncRPCRunningThrd (hasSyncRPCRunningThrd = (int) Counter::global(Counter::SyncRPCRunningThreads))
#define checkProcStopRPC        (hasProcStopRPC        = (int) Counter::global(Counter::ProcStopRPCs))
#define checkStartupTeardownProcs (hasStartupTeardownProc = (int) Counter::global(Counter::StartupTeardownProcesses))
#define checkNeonatalThreads     (hasNeonatalThreads   = (int) Counter::global(Counter::NeonatalThreads))
#define checkAsyncEvents        (hasAsyncEvents        = (int) Counter::global(Counter::AsyncEvents))
#define UNSET_CHECK        -8
#define printCheck(VAL)    (((int) VAL) == UNSET_CHECK ? '?' : (VAL ? 'T' : 'F'))

bool int_process::waitAndHandleEvents(bool block)
{
   pthrd_printf("Top of waitAndHandleEvents.  Block = %s\n", block ? "true" : "false");
   bool gotEvent = false;
   assert(!int_process::in_callback);
   bool error = false;

   static bool recurse = false;
   assert(!recurse);
   recurse = true;

   for (;;)
   {
      /**
       * Check for possible blocking combinations using Counters
       *
       * The weirdness in the #defines and variables here is to record the result of
       * each check, which is used if debug printing is turned on, while also letting the
       * C++ partial evaluator not check parts this expression (as the checks involve an
       * expensive locking operation).
       *
       * For most people, just ignore the #defines.  The important bit is that we're really
       * calling the Counter::global(...) function to determine whether we should block.
       **/
      int hasHandlerThread = UNSET_CHECK, hasAsyncPending = UNSET_CHECK, hasRunningThread = UNSET_CHECK;
      int hasClearingBP = UNSET_CHECK, hasStopPending = UNSET_CHECK, hasSyncRPCRunningThrd = UNSET_CHECK;
      int hasProcStopRPC  = UNSET_CHECK, hasBlock = UNSET_CHECK, hasGotEvent = UNSET_CHECK;
      int hasStartupTeardownProc = UNSET_CHECK, hasNeonatalThreads = UNSET_CHECK, hasAsyncEvents = UNSET_CHECK;

      bool should_block = (!checkHandlerThread &&
                           ((checkBlock && !checkGotEvent && checkRunningThread) ||
                            (checkSyncRPCRunningThrd) ||
                            (checkStopPending) ||
                            (checkClearingBP) ||
                            (checkProcStopRPC) ||
                            (checkAsyncPending) ||
                            (checkStartupTeardownProcs) ||
                            (checkNeonatalThreads) ||
                            (checkAsyncEvents)
                           )
                          );
      //Entry for this print match the above tests in order and one-for-one.
      pthrd_printf("%s for events = !%c && ((%c && !%c && %c) || %c || %c || %c || %c || %c || %c || %c || %c)\n",
                   should_block ? "Blocking" : "Polling",
                   printCheck(hasHandlerThread),
                   printCheck(hasBlock), printCheck(hasGotEvent), printCheck(hasRunningThread),
                   printCheck(hasSyncRPCRunningThrd),
                   printCheck(hasStopPending),
                   printCheck(hasClearingBP),
                   printCheck(hasProcStopRPC),
                   printCheck(hasAsyncPending),
                   printCheck(hasStartupTeardownProc),
                   printCheck(hasNeonatalThreads),
                   printCheck(hasAsyncEvents));

      //TODO: If/When we move to per-process locks, then we'll need a smarter should_block check
      //      We don't want the should_block changing between the above measurement
      //      and the below dequeue.  Perhaps dequeue should alway poll, and the user thread loops
      //      over it while (should_block == true), with a condition variable signaling when the
      //      should_block would go to false (so we don't just spin).

      if (should_block && Counter::globalCount(Counter::ForceGeneratorBlock)) {
         // Entirely possible we didn't continue anything, but we want the generator to
         // wake up anyway
         ProcPool()->condvar()->broadcast();
      }

      Event::ptr ev = mbox()->dequeue(should_block);

      if (ev == Event::ptr())
      {
         if (gotEvent) {
            pthrd_printf("Returning after handling events\n");
            goto done;
         }
         if (should_block) {
            perr_printf("Blocking wait failed to get events\n");
            ProcControlAPI::globalSetLastError(err_internal, "Blocking wait returned without events");
            error = true;
            goto done;
         }
         if (hasHandlerThread) {
            pthrd_printf("Handler thread found nothing to do\n");
            goto done;
         }
         if (!hasRunningThread) {
            pthrd_printf("No threads are running to produce events\n");
            ProcControlAPI::globalSetLastError(err_notrunning, "No threads are running to produce events\n");
            error = true;
            goto done;
         }

         ProcControlAPI::globalSetLastError(err_noevents, "Poll failed to find events");
         pthrd_printf("Poll failed to find events\n");
         error = true;
         goto done;
      }
      if (mt()->getThreadMode() == Process::NoThreads ||
          mt()->getThreadMode() == Process::GeneratorThreading)
      {
         pthrd_printf("Clearing event from pipe after dequeue\n");
         notify()->clearEvent();
      }

      gotEvent = true;

      bool terminating = (ev->getProcess()->isTerminated());

      bool exitEvent = (ev->getEventType().time() == EventType::Post &&
                        ev->getEventType().code() == EventType::Exit);
      Process::const_ptr proc = ev->getProcess();
      int_process *llproc = proc->llproc();

      if (terminating) {
	if(!exitEvent || !llproc) {
	  // Since the user will never handle this one...
	  pthrd_printf("Received event %s on terminated process, ignoring\n",
		       ev->name().c_str());
	  if (!isHandlerThread() && ev->noted_event) notify()->clearEvent();
	  continue;
	}
      }

      HandlerPool *hpool = llproc->handlerpool;

      if (!ev->handling_started) {
         llproc->updateSyncState(ev, false);
         llproc->noteNewDequeuedEvent(ev);
         ev->handling_started = true;
      }

      llproc->plat_preHandleEvent();

      bool should_handle_ev = llproc->getProcStopManager().prepEvent(ev);
      if (should_handle_ev) {
         hpool->handleEvent(ev);
      }

      llproc = proc->llproc();

      if (llproc) {
         bool result = llproc->syncRunState();
         if (!result) {
            pthrd_printf("syncRunState failed.  Returning error from waitAndHandleEvents\n");
            error = true;
            goto done;
         }
         llproc->plat_postHandleEvent();
      }
      else
      {
         //Special case event handling, the process cleaned itself
         // under this event (likely post-exit or post-crash), but was
         // unable to clean its handlerpool (as we were using it).
         // Clean this for the process now.
			pthrd_printf("Process is gone, skipping syncRunState and deleting handler pool\n");
		  delete hpool;
      }
   }
  done:
   pthrd_printf("Leaving WaitAndHandleEvents with return %s, 'cause we're done\n", !error ? "true" : "false");
   recurse = false;
   return !error;
}

void int_process::throwDetachEvent(bool temporary, bool leaveStopped)
{
   pthrd_printf("%s detaching from process %d\n", temporary ? "Temporary" : "Permanent", getPid());
   EventDetach::ptr detach_ev = EventDetach::ptr(new EventDetach());
   detach_ev->getInternal()->temporary_detach = temporary;
   detach_ev->getInternal()->leave_stopped = leaveStopped;
   detach_ev->setProcess(proc());
   detach_ev->setThread(threadPool()->initialThread()->thread());
   detach_ev->setSyncType(Event::async);

   getStartupTeardownProcs().inc();
   threadPool()->initialThread()->getDetachState().desyncStateProc(int_thread::stopped);

   mbox()->enqueue(detach_ev);
}

bool int_process::plat_detachDone() {
   return true;
}

bool int_process::terminate(bool &needs_sync)
{

   //Should be called with the ProcPool lock held.
   pthrd_printf("Terminate requested on process %d\n", getPid());
   getStartupTeardownProcs().inc();

   bool result = plat_terminate(needs_sync);
   if (!result) {
      pthrd_printf("plat_terminate failed on %d\n", getPid());
      return false;
   }
   forcedTermination = true;

	// On windows this leads to doubling up events
#if defined(os_windows)
   // If we're on windows, we want to force the generator thread into waiting for a debug
   // event _if the process is stopped_. If the process is running then we're already
   // waiting (or will wait) for a debug event, and forcing the generator to block may
   // lead to doubling up.

   // The following code is similar to GeneratorWindows::hasLiveProc. This is not a coincidence.
	bool all_stopped = true;
   for (int_threadPool::iterator iter = threadpool->begin(); iter != threadpool->end(); ++iter) {
		if (RUNNING_STATE((*iter)->getActiveState().getState())) {
			all_stopped = false;
			break;
		}
   }
   if (all_stopped) {
	   setForceGeneratorBlock(true);
   }
#else
   // Do it all the time
   setForceGeneratorBlock(true);
#endif

   return true;
}

bool int_process::preTerminate() {
    return true;
}

int_libraryTracking *int_process::getLibraryTracking()
{
   if (LibraryTracking_set)
      return pLibraryTracking;
   LibraryTracking_set = true;
   pLibraryTracking = dynamic_cast<int_libraryTracking *>(this);
   if (!pLibraryTracking)
      return NULL;
   if (!pLibraryTracking->up_ptr)
      pLibraryTracking->up_ptr = new LibraryTracking(proc());
   return pLibraryTracking;
}

int_LWPTracking *int_process::getLWPTracking()
{
   if (LWPTracking_set){
      return pLWPTracking;
   }
   LWPTracking_set = true;
   pLWPTracking = dynamic_cast<int_LWPTracking *>(this);
   if (!pLWPTracking)
      return NULL;
   if (!pLWPTracking->up_ptr)
      pLWPTracking->up_ptr = new LWPTracking(proc());
   return pLWPTracking;
}

int_threadTracking *int_process::getThreadTracking()
{
   if (ThreadTracking_set)
      return pThreadTracking;
   ThreadTracking_set = true;
   pThreadTracking = dynamic_cast<int_threadTracking *>(this);
   if (!pThreadTracking)
      return NULL;
   if (!pThreadTracking->up_ptr)
      pThreadTracking->up_ptr = new ThreadTracking(proc());
   return pThreadTracking;
}

int_followFork *int_process::getFollowFork()
{
   if (FollowFork_set)
      return pFollowFork;
   FollowFork_set = true;
   pFollowFork = dynamic_cast<int_followFork *>(this);
   if (!pFollowFork)
      return NULL;
   if (!pFollowFork->up_ptr)
      pFollowFork->up_ptr = new FollowFork(proc());
   return pFollowFork;
}

int_callStackUnwinding *int_process::getCallStackUnwinding()
{
   if (CallStackUnwinding_set)
      return pCallStackUnwinding;
   CallStackUnwinding_set = true;
   pCallStackUnwinding = dynamic_cast<int_callStackUnwinding *>(this);
   return pCallStackUnwinding;
}

int_multiToolControl *int_process::getMultiToolControl()
{
   if (MultiToolControl_set)
      return pMultiToolControl;
   MultiToolControl_set = true;
   pMultiToolControl = dynamic_cast<int_multiToolControl *>(this);
   if (!pMultiToolControl)
      return NULL;
   if (!pMultiToolControl->up_ptr)
      pMultiToolControl->up_ptr = new MultiToolControl(proc());
   return pMultiToolControl;
}

int_memUsage *int_process::getMemUsage()
{
   if (MemUsage_set)
      return pMemUsage;
   MemUsage_set = true;
   pMemUsage = dynamic_cast<int_memUsage *>(this);
   if (!pMemUsage)
      return NULL;
   if (!pMemUsage->up_ptr)
      pMemUsage->up_ptr = new MemoryUsage(proc());
   return pMemUsage;
}

int_signalMask *int_process::getSignalMask()
{
   if (SignalMask_set)
      return pSignalMask;
   SignalMask_set = true;
   pSignalMask = dynamic_cast<int_signalMask *>(this);
   if (!pSignalMask)
      return NULL;
   if (!pSignalMask->up_ptr)
      pSignalMask->up_ptr = new SignalMask(proc());
   return pSignalMask;
}

int_remoteIO *int_process::getRemoteIO()
{
   if (remoteIO_set)
      return pRemoteIO;
   remoteIO_set = true;
   pRemoteIO = dynamic_cast<int_remoteIO *>(this);
   if (!pRemoteIO)
      return NULL;
   if (!pRemoteIO->up_ptr)
      pRemoteIO->up_ptr = new RemoteIO(proc());
   return pRemoteIO;
}

int_process::int_process(Dyninst::PID p, std::string e,
                         std::vector<std::string> a,
                         std::vector<std::string> envp,
                         std::map<int,int> f) :
   state(neonatal),
   pid(p),
   creation_mode(ct_launch),
   executable(e),
   argv(a),
   env(envp),
   fds(f),
   arch(Dyninst::Arch_none),
   threadpool(NULL),
   up_proc(Process::ptr()),
   handlerpool(NULL),
   hasCrashSignal(false),
   crashSignal(0),
   hasExitCode(false),
   forcedTermination(false),
   silent_mode(false),
   exitCode(0),
   mem(NULL),
   continueSig(0),
   mem_cache(this),
   async_event_count(Counter::AsyncEvents),
   force_generator_block_count(Counter::ForceGeneratorBlock),
   startupteardown_procs(Counter::StartupTeardownProcesses),
   proc_stop_manager(this),
   user_data(NULL),
   last_error_string(NULL),
   symbol_reader(NULL),
   pLibraryTracking(NULL),
   pLWPTracking(NULL),
   pThreadTracking(NULL),
   pFollowFork(NULL),
   pMultiToolControl(NULL),
   pSignalMask(NULL),
   pCallStackUnwinding(NULL),
   pMemUsage(NULL),
   pRemoteIO(NULL),
   LibraryTracking_set(false),
   LWPTracking_set(false),
   ThreadTracking_set(false),
   FollowFork_set(false),
   MultiToolControl_set(false),
   SignalMask_set(false),
   CallStackUnwinding_set(false),
   MemUsage_set(false),
   remoteIO_set(false)
{
    pthrd_printf("New int_process at %p\n", (void*)this);
    clearLastError();
	wasCreatedViaAttach(pid == 0);
   //Put any object initialization in 'initializeProcess', below.
}

int_process::int_process(Dyninst::PID pid_, int_process *p) :
   state(int_process::running),
   pid(pid_),
   creation_mode(ct_attach),
   executable(p->executable),
   argv(p->argv),
   env(p->env),
   arch(p->arch),
   hasCrashSignal(p->hasCrashSignal),
   crashSignal(p->crashSignal),
   hasExitCode(p->hasExitCode),
   forcedTermination(false),
   silent_mode(false),
   exitCode(p->exitCode),
   continueSig(p->continueSig),
   mem_cache(this),
   async_event_count(Counter::AsyncEvents),
   force_generator_block_count(Counter::ForceGeneratorBlock),
   startupteardown_procs(Counter::StartupTeardownProcesses),
   proc_stop_manager(this),
   user_data(NULL),
   last_error_string(NULL),
   symbol_reader(NULL),
   pLibraryTracking(NULL),
   pLWPTracking(NULL),
   pThreadTracking(NULL),
   pFollowFork(NULL),
   pMultiToolControl(NULL),
   pSignalMask(NULL),
   pCallStackUnwinding(NULL),
   pMemUsage(NULL),
   pRemoteIO(NULL),
   LibraryTracking_set(false),
   LWPTracking_set(false),
   ThreadTracking_set(false),
   FollowFork_set(false),
   MultiToolControl_set(false),
   SignalMask_set(false),
   CallStackUnwinding_set(false),
   MemUsage_set(false),
   remoteIO_set(false)
{
   pthrd_printf("New int_process at %p\n", (void*)this);
   Process::ptr hlproc = Process::ptr(new Process());
   clearLastError();
   mem = new mem_state(*p->mem, this);
   initializeProcess(hlproc);
}

void int_process::initializeProcess(Process::ptr p)
{
   assert(!p->llproc_);
   p->llproc_ = this;
   up_proc = p;
   threadpool = new int_threadPool(this);
   handlerpool = createDefaultHandlerPool(this);
   libpool.proc = this;
   if (!mem)
      mem = new mem_state(this);
   Generator::getDefaultGenerator(); //May create generator thread
}

int_thread *int_process::findStoppedThread()
{
   int_thread *result = NULL;
   for (int_threadPool::iterator i = threadpool->begin(); i != threadpool->end(); ++i)
   {
      int_thread *thr = *i;
      if (thr->getHandlerState().getState() == int_thread::stopped) {
         result = thr;
         break;
      }
   }
   return result;
}

bool int_process::readMem(Dyninst::Address remote, mem_response::ptr result, int_thread *thr)
{
   if (getAddressWidth() == 4) {
      Address old = remote;
      remote &= 0xffffffff;
      pthrd_printf("Address cropping for 32-bit: 0x%lx to 0x%lx\n",
                   old, remote);
   }

   if (!thr && plat_needsThreadForMemOps())
   {
      thr = findStoppedThread();
      if (!thr) {
         setLastError(err_notstopped, "A thread must be stopped to read from memory");
         perr_printf("Unable to find a stopped thread for read in process %d\n", getPid());
         return false;
      }
   }

   result->setProcess(this);
   bool bresult;


   if (!plat_needsAsyncIO()) {
      pthrd_printf("Reading from remote memory %lx to %p, size = %lu on %d/%d\n",
                   remote, (void*)result->getBuffer(), (unsigned long) result->getSize(),
				   getPid(), thr ? thr->getLWP() : (Dyninst::LWP)(-1));

      bresult = plat_readMem(thr, result->getBuffer(), remote, result->getSize());
      if (!bresult) {
          perr_printf("plat_readMem failed!\n");
         result->markError();
      }
      result->setResponse();

      int_eventAsyncIO *iev = result->getAsyncIOEvent();
      if (iev) {
         pthrd_printf("Enqueueing new EventAsyncRead into mailbox on synchronous platform\n");
         EventAsyncRead::ptr ev = EventAsyncRead::ptr(new EventAsyncRead(iev));
         ev->setProcess(proc());
         ev->setThread(threadPool()->initialThread()->thread());
         ev->setSyncType(Event::async);
         mbox()->enqueue(ev);
      }
   }
   else {
      pthrd_printf("Async read from remote memory %lx to %p, size = %lu on %d/%d\n",
                   remote, (void*)result->getBuffer(), (unsigned long) result->getSize(),
                   getPid(), thr ? thr->getLWP() : (Dyninst::LWP)(-1));

      getResponses().lock();
      bresult = plat_readMemAsync(thr, remote, result);
      if (bresult) {
         getResponses().addResponse(result, this);
      }
      getResponses().unlock();
      getResponses().noteResponse();
   }
   return bresult;
}

bool int_process::writeMem(const void *local, Dyninst::Address remote, size_t size, result_response::ptr result, int_thread *thr, bp_write_t bp_write)
{
   if (getAddressWidth() == 4) {
      Address old = remote;
      remote &= 0xffffffff;
      pthrd_printf("Address cropping for 32-bit: 0x%lx to 0x%lx\n",
                   old, remote);
   }

   if (!thr && plat_needsThreadForMemOps())
   {
      thr = findStoppedThread();
      if (!thr) {
         setLastError(err_notstopped, "A thread must be stopped to write to memory");
         perr_printf("Unable to find a stopped thread for write in process %d\n", getPid());
         return false;
      }
   }
   result->setProcess(this);
   bool bresult;
   if (!plat_needsAsyncIO()) {
      pthrd_printf("Writing to remote memory %lx from %p, size = %lu on %d/%d\n",
                   remote, local, (unsigned long) size,
                   getPid(), thr ? thr->getLWP() : (Dyninst::LWP)(-1));
      bresult = plat_writeMem(thr, local, remote, size, bp_write);
      if (!bresult) {
         result->markError();
      }
      result->setResponse(bresult);

      int_eventAsyncIO *iev = result->getAsyncIOEvent();
      if (iev) {
         pthrd_printf("Enqueueing new EventAsyncWrite into mailbox on synchronous platform\n");
         EventAsyncWrite::ptr ev = EventAsyncWrite::ptr(new EventAsyncWrite(iev));
         ev->setProcess(proc());
         ev->setThread(threadPool()->initialThread()->thread());
         ev->setSyncType(Event::async);
         mbox()->enqueue(ev);
      }
   }
   else {
      pthrd_printf("Async writing to remote memory %lx from %p, size = %lu on %d/%d\n",
                   remote, local, (unsigned long) size,
                   getPid(), thr ? thr->getLWP() : (Dyninst::LWP)(-1));

      getResponses().lock();
      bresult = plat_writeMemAsync(thr, local, remote, size, result, bp_write);
      if (bresult) {
         getResponses().addResponse(result, this);
      }
      getResponses().unlock();
      getResponses().noteResponse();
   }
   return bresult;
}

unsigned int_process::plat_getRecommendedReadSize()
{
   return getTargetPageSize();
}

Dyninst::Address int_process::mallocExecMemory(unsigned size)
{
   Dyninst::Address max = 0;
   std::map<Dyninst::Address, unsigned>::iterator i;
   for (i = exec_mem_cache.begin(); i != exec_mem_cache.end(); ++i) {
      if (i->first + i->second > max)
         max = i->first + i->second;
   }

   Dyninst::Address addr = plat_mallocExecMemory(max, size);
   exec_mem_cache[addr] = size;
   return addr;
}

void int_process::freeExecMemory(Dyninst::Address addr)
{
   std::map<Dyninst::Address, unsigned>::iterator i;
   i = exec_mem_cache.find(addr);
   assert(i != exec_mem_cache.end());
   exec_mem_cache.erase(i);
}

Dyninst::Address int_process::direct_infMalloc(unsigned long, bool, Dyninst::Address)
{
   assert(0);
   return 0;
}

bool int_process::direct_infFree(Dyninst::Address)
{
   assert(0);
   return false;
}

Address int_process::infMalloc(unsigned long size, bool use_addr, Address addr)
{
   int_addressSet as;
   as.insert(make_pair(addr, proc()));
   bool result = int_process::infMalloc(size, &as, use_addr);
   if (!result)
      return 0;
   return as.begin()->first;
}

bool int_process::infFree(Address addr) {
   int_addressSet as;
   as.insert(make_pair(addr, proc()));
   return int_process::infFree(&as);
}

bool int_process::infMalloc(unsigned long size, int_addressSet *aset, bool use_addr)
{
   bool had_error = false;
   set<pair<int_process *, int_iRPC::ptr> > active_mallocs;
   int_addressSet direct_results;

   for (int_addressSet::iterator i = aset->begin(); i != aset->end(); i++) {
      Process::ptr p = i->second;
      Address addr = i->first;
      if (!p)
         continue;
      int_process *proc = p->llproc();
      if (proc->getState() == int_process::detached)
         continue;
      pthrd_printf("Process %d is allocating memory of size %lu at 0x%lx\n", proc->getPid(), size, addr);

      if (proc->plat_supportDirectAllocation()) {
         //Platforms like Windows don't need IRPCs for allocation.  Just do it and put the results
         // in direct_results, which will be unioned with aset latter.
         Dyninst::Address addr_result = proc->direct_infMalloc(size, use_addr, addr);
         if (addr_result == 0) {
            had_error = true;
            continue;
         }
         direct_results.insert(make_pair(addr_result, p));
         continue;
      }

      int_iRPC::ptr rpc = rpcMgr()->createInfMallocRPC(proc, size, use_addr, addr);
      assert(rpc);
      bool result = rpcMgr()->postRPCToProc(proc, rpc);
      if (!result) {
         p->setLastError(err_internal, "Error posting infMalloc RPC to process\n");
         pthrd_printf("Error posting RPC to thread");
         had_error = true;
         continue;
      }

      int_thread *thr = rpc->thread();
      assert(thr);
      active_mallocs.insert(make_pair(proc, rpc));
      thr->getInternalState().desyncState(int_thread::running);
      rpc->setRestoreInternal(true);
      proc->throwNopEvent();
   }

   if (!active_mallocs.empty()) {
      bool result = int_process::waitAndHandleEvents(false);
      if (!result) {
         perr_printf("Internal error calling waitAndHandleEvents\n");
         for_each(aset->begin(), aset->end(),
                  setError(err_internal, "Error while calling waitAndHandleForProc from infMalloc\n"));
         return false;
      }
   }

   if (!use_addr)
      aset->clear();
   if (!direct_results.empty())
      aset->insert(direct_results.begin(), direct_results.end());


   for (set<pair<int_process *, int_iRPC::ptr> >::iterator i = active_mallocs.begin();
        i != active_mallocs.end(); i++)
   {
      int_process *proc = i->first;
      int_iRPC::ptr rpc = i->second;
      assert(rpc->getState() == int_iRPC::Finished);
      Dyninst::Address aresult = rpc->infMallocResult();
      pthrd_printf("Inferior malloc returning %lx on %d\n", aresult, proc->getPid());
      if (aresult == (unsigned long) -1) {
         perr_printf("infMalloc returned invalid address\n");
         proc->setLastError(err_procread, "Unable to allocate memory at given address");
         had_error = true;
         continue;
      }
      proc->memory()->inf_malloced_memory.insert(make_pair(aresult, size));
      if (use_addr)
         continue;
      aset->insert(make_pair(aresult, proc->proc()));
   }

   return !had_error;
}

bool int_process::infFree(int_addressSet *aset)
{
   bool had_error = false;
   set<pair<Process::ptr, int_iRPC::ptr> > active_frees;

   for (int_addressSet::iterator i = aset->begin(); i != aset->end(); i++) {
      Address addr = i->first;
      Process::ptr p = i->second;
      if (!p) {
         had_error = true;
         continue;
      }
      int_process *proc = p->llproc();
      if (!proc || proc->getState() == int_process::detached) {
         had_error = true;
         continue;
      }

      std::map<Dyninst::Address, unsigned long>::iterator j = proc->mem->inf_malloced_memory.find(addr);
      if (j == proc->mem->inf_malloced_memory.end()) {
         proc->setLastError(err_badparam, "Unknown address passed to freeMemory");
         perr_printf("Passed bad address, %lx, to infFree on %d\n", addr, proc->getPid());
         had_error = true;
         continue;
      }
      unsigned long size = j->second;

      if (proc->plat_supportDirectAllocation()) {
         bool result = proc->direct_infFree(addr);
         if (!result)
            had_error = true;
         continue;
      }

      int_iRPC::ptr rpc = rpcMgr()->createInfFreeRPC(proc, size, addr);
      assert(rpc);
      pthrd_printf("Process %d is freeing memory of size %lu at 0x%lx with rpc %lu\n", proc->getPid(),
                   size, addr, rpc->id());
      bool result = rpcMgr()->postRPCToProc(proc, rpc);
      if (!result) {
         pthrd_printf("Failed to post free rpc to process\n");
         had_error = true;
         continue;
      }

      int_thread *thr = rpc->thread();
      assert(thr);
      thr->getInternalState().desyncState(int_thread::running);
      rpc->setRestoreInternal(true);

      proc->throwNopEvent();
      active_frees.insert(make_pair(p, rpc));
   }

   if (!active_frees.empty()) {
      bool result = int_process::waitAndHandleEvents(false);
      if (!result) {
         perr_printf("Internal error calling waitAndHandleEvents\n");
         for_each(aset->begin(), aset->end(),
                  setError(err_internal, "Error while calling waitAndHandleForProc from infFree\n"));
         return false;
      }
   }

   for (set<pair<Process::ptr, int_iRPC::ptr> >::iterator i = active_frees.begin(); i != active_frees.end(); i++) {
      Process::ptr p = i->first;
      int_iRPC::ptr rpc = i->second;
      int_process *proc = p->llproc();

      if (!proc) {
         perr_printf("Process %d exited during infFree\n", p->getPid());
         p->setLastError(err_exited, "Process exited during infFree\n");
         had_error = true;
         continue;
      }
      assert(rpc->getState() == int_iRPC::Finished);
      Address addr = rpc->getInfFreeTarget();
      map<Dyninst::Address, unsigned long>::iterator j = proc->mem->inf_malloced_memory.find(addr);
      proc->mem->inf_malloced_memory.erase(j);
   }

   return !had_error;
}

bool int_process::plat_decodeMemoryRights(Process::mem_perm& rights_internal,
                                          unsigned long rights) {
    (void)rights_internal;
    (void)rights;
    perr_printf("Called decodeMemoryRights on unspported platform\n");
    setLastError(err_unsupported, "Decode Mem Permission not supported on this platform\n");
	return false;
}

bool int_process::plat_encodeMemoryRights(Process::mem_perm rights_internal,
                                          unsigned long& rights) {
    (void)rights_internal;
    (void)rights;
    perr_printf("Called encodeMemoryRights on unspported platform\n");
    setLastError(err_unsupported, "Encode Memory Permission not supported on this platform\n");
	return false;
}

bool int_process::getMemoryAccessRights(Dyninst::Address addr, Process::mem_perm& rights) {
    if (!plat_getMemoryAccessRights(addr, rights)) {
        pthrd_printf("Error get rights from memory %lx on target process %d\n",
                     addr, getPid());
        return false;
    }

    return true;
}

bool int_process::plat_getMemoryAccessRights(Dyninst::Address addr, Process::mem_perm& rights) {
    (void)addr;
    (void)rights;
    perr_printf("Called getMemoryAccessRights on unspported platform\n");
    setLastError(err_unsupported, "Get Memory Permission not supported on this platform\n");
	return false;
}

bool int_process::setMemoryAccessRights(Dyninst::Address addr, size_t size,
                                        Process::mem_perm rights,
                                        Process::mem_perm& oldRights) {
    if (!plat_setMemoryAccessRights(addr, size, rights, oldRights)) {
        pthrd_printf("ERROR: set rights to %s from memory %lx on target process %d\n",
                     rights.getPermName().c_str(), addr, getPid());
        return false;
    }

    return true;
}

bool int_process::plat_setMemoryAccessRights(Dyninst::Address addr, size_t size,
                                             Process::mem_perm rights,
                                             Process::mem_perm& oldRights) {
    (void)addr;
    (void)size;
    (void)rights;
    (void)oldRights;
    perr_printf("Called setMemoryAccessRights on unspported platform\n");
    setLastError(err_unsupported, "Set Memory Permission not supported on this platform\n");
	return false;
}

bool int_process::findAllocatedRegionAround(Dyninst::Address addr,
                                            Process::MemoryRegion& memRegion) {
    if (!plat_findAllocatedRegionAround(addr, memRegion)) {
        pthrd_printf("Error when find allocated memory region"
                     " for %lx on target process %d\n", addr, getPid());
        return false;
    }

    return true;
}

bool int_process::plat_findAllocatedRegionAround(Dyninst::Address addr,
                                                 Process::MemoryRegion& memRegion) {
    (void)addr;
    memRegion.first  = 0;
    memRegion.second = 0;
    perr_printf("Called findAllocatedRegionAround on unspported platform\n");
    setLastError(err_unsupported,
                 "Find Allocated Region Addr not supported on this platform\n");
	return false;
}

SymbolReaderFactory *int_process::getSymReader()
{
   if (symbol_reader) {
      return symbol_reader;
   }
   else if (user_set_symbol_reader) {
      symbol_reader = user_set_symbol_reader;
      return symbol_reader;
   }
   symbol_reader = plat_defaultSymReader();
   return symbol_reader;
}

void int_process::setSymReader(SymbolReaderFactory *fact)
{
   symbol_reader = fact;
}

SymbolReaderFactory *int_process::plat_defaultSymReader()
{
   //Default version of function for systems without symbol readers
   return NULL;
}

void int_process::setForceGeneratorBlock(bool b)
{
   if (b) {
      force_generator_block_count.inc();
   }
   else {
      if (force_generator_block_count.local())
         force_generator_block_count.dec();
   }

   pthrd_printf("forceGeneratorBlock - Count is now %d/%d\n",
                force_generator_block_count.localCount(),
                Counter::globalCount(Counter::ForceGeneratorBlock));
}

int int_process::getAddressWidth()
{
   switch (getTargetArch()) {
      case Arch_x86:
      case Arch_ppc32:
      case Arch_aarch32:
         return 4;
      case Arch_x86_64:
      case Arch_ppc64:
      case Arch_aarch64:
      case Arch_cuda:
      case Arch_intelGen9:
         return 8;
      case Arch_amdgpu_gfx908:
      case Arch_amdgpu_gfx90a:
      case Arch_amdgpu_gfx940:
         return 8;  
      case Arch_none:
         assert(0);
   }
   return 0;
}

HandlerPool *int_process::handlerPool() const
{
   return handlerpool;
}

bool int_process::addBreakpoint_phase1(bp_install_state *is)
{
   is->ibp = NULL;
   map<Address, sw_breakpoint *>::iterator i = mem->breakpoints.find(is->addr);
   is->do_install = (i == mem->breakpoints.end());
   if (!is->do_install) {
     is->ibp = i->second;
     assert(is->ibp && is->ibp->isInstalled());
     bool result = is->ibp->addToIntBreakpoint(is->bp, this);
     if (!result) {
       pthrd_printf("Failed to install new breakpoint\n");
       return false;
     }
     return true;
   }

   is->ibp = new sw_breakpoint(mem, is->addr);

   if (!is->ibp->checkBreakpoint(is->bp, this)) {
      pthrd_printf("Failed check breakpoint\n");
      delete is->ibp;
      is->ibp = NULL;
      return false;
   }

   is->mem_resp = mem_response::createMemResponse();
   is->mem_resp->markSyncHandled();
   bool result = is->ibp->prepBreakpoint(this, is->mem_resp);
   if (!result) {
      pthrd_printf("Failed to prep breakpoint\n");
      delete is->ibp;
      return false;
   }
   return true;
}

bool int_process::addBreakpoint_phase2(bp_install_state *is)
{
   if (!is->do_install)
      return true;

   if (is->mem_resp->hasError()) {
      pthrd_printf("Error prepping breakpoint\n");
      delete is->ibp;
      return false;
   }

   is->res_resp = result_response::createResultResponse();
   is->res_resp->markSyncHandled();
   bool result = is->ibp->insertBreakpoint(this, is->res_resp);
   if (!result) {
      pthrd_printf("Error writing new breakpoint\n");
      delete is->ibp;
      return false;
   }
   return true;
}

bool int_process::addBreakpoint_phase3(bp_install_state *is)
{
   if (!is->do_install)
      return true;

   bool result = is->ibp->addToIntBreakpoint(is->bp, this);
   if (!result) {
      pthrd_printf("Failed to install new breakpoint\n");
      return false;
   }

   if (is->res_resp->hasError()) {
      pthrd_printf("Error writing new breakpoint\n");
      delete is->ibp;
      return false;
   }

   return true;
}

bool int_process::addBreakpoint(Dyninst::Address addr, int_breakpoint *bp)
{
   if (getState() != running) {
      perr_printf("Attempted to add breakpoint at %lx to exited process %d\n", addr, getPid());
      setLastError(err_exited, "Attempted to insert breakpoint into exited process\n");
      return false;
   }

   bp_instance *instance = NULL;
   if (bp->isHW()) {
      instance = hw_breakpoint::create(this, bp, addr);
   }
   else {
      instance = sw_breakpoint::create(this, bp, addr);
   }
   return (instance != NULL);
}

bool int_process::removeAllBreakpoints() {
   if (!mem) return true;
   bool ret = true;
   std::map<Dyninst::Address, sw_breakpoint *>::iterator iter = mem->breakpoints.begin();
   while (iter != mem->breakpoints.end()) { 
      std::set<response::ptr> resps;
      // uninstall will call erase to remove the item,
      // so just keep calling begin() to iterate over all elements
      if (!iter->second->uninstall(this, resps)) {
         ret = false;
	 break;
      }
      assert(resps.empty());
      iter = mem->breakpoints.begin();
   }
   return ret;
}

bool int_process::removeBreakpoint(Dyninst::Address addr, int_breakpoint *bp, set<response::ptr> &resps)
{
   pthrd_printf("Removing breakpoint at %lx in %d\n", addr, getPid());
   set<bp_instance *> bps_to_remove;
   map<Address, sw_breakpoint *>::iterator i = mem->breakpoints.find(addr);
   if (i != mem->breakpoints.end()) {
      sw_breakpoint *swbp = i->second;
      assert(swbp && swbp->isInstalled());
      if (swbp->containsIntBreakpoint(bp))
          bps_to_remove.insert(static_cast<bp_instance *>(swbp));
   }

   for (auto thr : *threadPool()) {
      hw_breakpoint *hwbp = thr->getHWBreakpoint(addr);
      if (hwbp && hwbp->containsIntBreakpoint(bp)) {
         bps_to_remove.insert(static_cast<bp_instance *>(hwbp));
      }
   }

   if (bps_to_remove.empty()) {
      perr_printf("Attempted to removed breakpoint that isn't installed\n");
      setLastError(err_notfound, "Tried to uninstall breakpoint that isn't installed.\n");
      return false;
   }

   for (auto ibp : bps_to_remove) {
      bool empty;
      bool result = ibp->rmBreakpoint(this, bp, empty, resps);
      if (!result) {
         pthrd_printf("rmBreakpoint failed on breakpoint at %lx in %d\n", addr, getPid());
         return false;
      }
      if (empty) {
         delete ibp;
      }
   }

   return true;
}

sw_breakpoint *int_process::getBreakpoint(Dyninst::Address addr)
{
   std::map<Dyninst::Address, sw_breakpoint *>::iterator  i = mem->breakpoints.find(addr);
   if (i == mem->breakpoints.end())
      return NULL;
   return i->second;
}

int_library *int_process::getLibraryByName(std::string s) const
{
	// Exact matches first, but find substring matches and return if unique.
	// TODO: is this the behavior we actually want?
	bool substring_unique = true;
	std::set<int_library*>::iterator substring_match = mem->libs.end();
   for (set<int_library *>::iterator i = mem->libs.begin();
        i != mem->libs.end(); ++i)
   {
	   std::string n = (*i)->getName();
      if (s == n)
         return *i;
	  if((n.find(s) != std::string::npos)) {
		  if(substring_match == mem->libs.end()) {
			substring_match = i;
		  }
		  else {
			substring_unique = false;
		  }
	  }
   }
	if(substring_match != mem->libs.end() && substring_unique)
	{
		return *substring_match;
	}
   return NULL;
}

unsigned int int_process::plat_getCapabilities()
{
   return (Process::pc_read | Process::pc_write | Process::pc_irpc | Process::pc_control);
}

Event::ptr int_process::plat_throwEventsBeforeContinue(int_thread *)
{
   return Event::ptr();
}

bool int_process::plat_threadOpsNeedProcStop()
{
   return false;
}

size_t int_process::numLibs() const
{
   return mem->libs.size();
}

std::string int_process::getExecutable() const
{
   return executable; //The name of the exec passed to PC
}

bool int_process::isInCallback()
{
   return in_callback;
}

mem_state::ptr int_process::memory() const
{
   return mem;
}

err_t int_process::getLastError() {
   return last_error;
}

const char *int_process::getLastErrorMsg() {
   return last_error_string;
}

void int_process::clearLastError() {
   last_error = err_none;
   last_error_string = "ok";
}

void int_process::setLastError(err_t err, const char *str) {
   last_error = err;
   last_error_string = str;
   ProcControlAPI::globalSetLastError(err, str);
}

void int_process::setExitCode(int c)
{
   assert(!hasCrashSignal);
   hasExitCode = true;
   exitCode = c;
}

void int_process::setCrashSignal(int s)
{
   assert(!hasExitCode);
   hasCrashSignal = true;
   crashSignal = s;
}

bool int_process::getExitCode(int &c)
{
   c = exitCode;
   return hasExitCode;
}

bool int_process::getCrashSignal(int &s)
{
   s = crashSignal;
   return hasCrashSignal;
}

bool int_process::wasForcedTerminated() const
{
   return forcedTermination;
}

bool int_process::plat_individualRegRead(Dyninst::MachRegister, int_thread *)
{
   return plat_individualRegAccess();
}

bool int_process::plat_individualRegSet()
{
   return plat_individualRegAccess();
}

bool int_process::isInCB()
{
   return in_callback;
}

void int_process::setInCB(bool b)
{
   assert(in_callback == !b);
   in_callback = b;
}

void int_process::throwNopEvent()
{
   EventNop::ptr ev = EventNop::ptr(new EventNop());
   ev->setProcess(proc());
   ev->setThread(threadPool()->initialThread()->thread());
   ev->setSyncType(Event::async);

   mbox()->enqueue(ev);
}

async_ret_t int_process::plat_calcTLSAddress(int_thread *, int_library *, Offset,
                                             Address &, std::set<response::ptr> &)
{
   perr_printf("Unsupported access to plat_calcTLSAddress\n");
   setLastError(err_unsupported, "TLS Access not supported on this platform\n");
   return aret_error;
}

bool int_process::plat_needsAsyncIO() const
{
   return false;
}

bool int_process::plat_readMemAsync(int_thread *, Dyninst::Address,
                                    mem_response::ptr )
{
   assert(0);
   return false;
}

bool int_process::plat_writeMemAsync(int_thread *, const void *, Dyninst::Address,
                                     size_t, result_response::ptr, bp_write_t)
{
   assert(0);
   return false;
}

memCache *int_process::getMemCache()
{
   return &mem_cache;
}

void int_process::updateSyncState(Event::ptr ev, bool gen)
{
   // This works around a Linux bug where a continue races with a whole-process exit
   plat_adjustSyncType(ev, gen);

   EventType etype = ev->getEventType();
   switch (ev->getSyncType()) {
	  case Event::async: {
         break;
     }
      case Event::sync_thread: {
         int_thread *thrd = ev->getThread()->llthrd();
         if (!thrd) {
            pthrd_printf("No thread for sync thread event, assuming thread exited\n");
            return;
         }
         int_thread::StateTracker &st = gen ? thrd->getGeneratorState() : thrd->getHandlerState();
         int_thread::State old_state = st.getState();
         if (old_state == int_thread::exited) {
            //Silly, linux.  Giving us events on processes that have exited.
            pthrd_printf("Recieved events for exited thread, not changing thread state\n");
            break;
         }
         pthrd_printf("Event %s is thread synchronous, marking thread %d %s stopped\n",
                      etype.name().c_str(), thrd->getLWP(), gen ? "generator" : "handler");
/*
 * 	 Xiaozhu: this assert causes pc_irpc to fail non-deterministically.
 * 	 I think it is possible for the generator to receive one sync event one a thread,
 * 	 and before the handler handles it and unset its status, the generator
 * 	 receives another sync event on the same thread. 
 *
 * 	 This is because that even thought we attempt to stop the thread for a sync event,
 * 	 there is a gap between the sync event and the stop. There could be another sync 
 * 	 event happening. 
 *
         assert(RUNNING_STATE(old_state) ||
                thrd->llproc()->wasForcedTerminated() ||
                (old_state == int_thread::stopped && (thrd->isExiting() || thrd->isExitingInGenerator())));
*/
         if (old_state == int_thread::errorstate)
            break;
         st.setState(int_thread::stopped);
         break;
      }
      case Event::sync_process: {
         pthrd_printf("Event %s is process synchronous, marking process %d %s stopped\n",
                      etype.name().c_str(), getPid(), gen ? "generator" : "handler");
         int_threadPool *tp = threadPool();
         for (int_threadPool::iterator i = tp->begin(); i != tp->end(); ++i) {
            int_thread *thrd = *i;
            int_thread::StateTracker &st = gen ? thrd->getGeneratorState() : thrd->getHandlerState();
            int_thread::State old_state = st.getState();
            if (!RUNNING_STATE(old_state))
               continue;
            st.setState(int_thread::stopped);
         }
         break;
      }
      case Event::unset: {
         assert(0);
      }
   }
}

ProcStopEventManager &int_process::getProcStopManager()
{
   return proc_stop_manager;
}

bool int_process::plat_supportThreadEvents()
{
   return false;
}

bool int_process::plat_supportLWPCreate()
{
   return false;
}

bool int_process::plat_supportLWPPreDestroy()
{
   return false;
}

bool int_process::plat_supportLWPPostDestroy()
{
   return false;
}

bool int_process::plat_supportHWBreakpoint()
{
   return false;
}

bool int_process::plat_supportFork()
{
   return false;
}

bool int_process::plat_supportExec()
{
   return false;
}

async_ret_t int_process::plat_needsEmulatedSingleStep(int_thread *, std::vector<Address> &) {
   return aret_success;
}

void int_process::plat_getEmulatedSingleStepAsyncs(int_thread *, std::set<response::ptr>) {
   assert(0);
}

bool int_process::plat_needsPCSaveBeforeSingleStep()
{
   return false;
}

map<int, int> &int_process::getProcDesyncdStates()
{
   return proc_desyncd_states;
}

bool int_process::isRunningSilent()
{
   return silent_mode;
}

void int_process::setRunningSilent(bool b)
{
   silent_mode = b;
}

bool int_process::plat_supportDOTF()
{
   return true;
}

void int_process::noteNewDequeuedEvent(Event::ptr)
{
}

bool int_process::plat_preHandleEvent()
{
   return true;
}

bool int_process::plat_postHandleEvent()
{
   return true;
}

bool int_process::plat_preAsyncWait()
{
  return true;
}

int_process::~int_process()
{
   pthrd_printf("Deleting int_process at %p\n", (void*)this);
   if (up_proc != Process::ptr())
   {
      proc_exitstate *exitstate = new proc_exitstate();
      exitstate->pid = pid;
      exitstate->exited = hasExitCode && !forcedTermination;
      exitstate->exit_code = exitCode;
      exitstate->crashed = hasCrashSignal;
      exitstate->crash_signal = crashSignal;
      exitstate->user_data = user_data;
      assert(!up_proc->exitstate_);
      up_proc->exitstate_ = exitstate;
      up_proc->llproc_ = NULL;
   }

   if (threadpool) {
      delete threadpool;
      threadpool = NULL;
   }

   //Do not delete handlerpool yet, we're currently under
   // an event handler.  We do want to delete this if called
   // from detach.
   bool should_clean;
   mem->rmProc(this, should_clean);
   if (should_clean) {
      delete mem;
   }
   mem = NULL;

   if(ProcPool()->findProcByPid(getPid())) ProcPool()->rmProcess(this);
}

indep_lwp_control_process::indep_lwp_control_process(Dyninst::PID p, std::string e, std::vector<std::string> a,
                                                     std::vector<std::string> envp, std::map<int,int> f) :
   int_process(p, e, a, envp, f)
{
}

indep_lwp_control_process::indep_lwp_control_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p)
{
}

bool indep_lwp_control_process::plat_syncRunState()
{
   for (int_threadPool::iterator i = threadPool()->begin(); i != threadPool()->end(); i++) {
      int_thread *thr = *i;
      int_thread::State handler_state = thr->getHandlerState().getState();
      int_thread::State target_state = thr->getTargetState();
      bool result = true;

      pthrd_printf("plat_syncRunState for thread %d/%d\n", thr->proc()->getPid(), thr->getLWP());

      if (handler_state == target_state) {
    	 pthrd_printf("plat_syncRunState: thread is in desired state\n");
         continue;
      }
      else if (handler_state == int_thread::stopped && RUNNING_STATE(target_state)) {
         result = thr->intCont();
         pthrd_printf("plat_syncRunState: trying to continue; res=%d\n", result);
      }
      else if (RUNNING_STATE(handler_state) && target_state == int_thread::stopped) {
         result = thr->intStop();
         pthrd_printf("plat_syncRunState: trying to stop; res=%d\n", result);
      }

      if (!result && getLastError() == err_exited) {
    	  pthrd_printf("Suppressing error of continue/stop on exited process\n");
    	  if(thr->plat_handle_ghost_thread()) {
    		  thr->getHandlerState().setState(int_thread::running);
    	  }
      }
      else if (!result) {
         pthrd_printf("Error changing process state from plat_syncRunState\n");
         return false;
      }
   }
   return true;
}

indep_lwp_control_process::~indep_lwp_control_process()
{
}

unified_lwp_control_process::unified_lwp_control_process(Dyninst::PID p, std::string e,
                                                         std::vector<std::string> a,
                                                         std::vector<std::string> envp,
                                                         std::map<int,int> f) :
   int_process(p, e, a, envp, f)
{
}

unified_lwp_control_process::unified_lwp_control_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p)
{
}

unified_lwp_control_process::~unified_lwp_control_process()
{
}

bool unified_lwp_control_process::plat_syncRunState()
{
   bool want_ss_running = false;
   bool result = true;

   bool want_all_stopped = true;
   bool want_all_running = true;
   bool want_some_stopped = false;
   bool want_some_running = false;

   bool is_all_stopped = true;
   bool is_all_running = true;

   if (getState() == detached || getState() == exited || getState() == errorstate) {
      pthrd_printf("Process %d is in state %s, doing nothing in plat_syncRunState\n",
                   getPid(), int_process::stateName(getState()));
      return true;
   }

   for (int_threadPool::iterator i = threadPool()->begin(); i != threadPool()->end(); i++) {
      int_thread *thr = *i;
      int_thread::State handler_state = thr->getHandlerState().getState();
      int_thread::State target_state = thr->getTargetState();
      if (target_state == int_thread::ditto)
         target_state = thr->getHandlerState().getState();

      if (RUNNING_STATE(target_state)) {
         want_some_running = true;
         want_all_stopped = false;
      }
      else if (target_state == int_thread::stopped) {
         want_some_stopped = true;
         want_all_running = false;
      }
      else {
         want_all_stopped = false;
         want_all_running = false;
      }

      if (RUNNING_STATE(handler_state)) {
         is_all_stopped = false;
      }
      else if (handler_state == int_thread::stopped) {
         is_all_running = false;
      }
      else {
         is_all_stopped = false;
         is_all_running = false;
      }

      if (!RUNNING_STATE(handler_state) && RUNNING_STATE(target_state) && thr->singleStep()) {
         want_ss_running = true;
      }
   }

   pthrd_printf("In plat_syncRunState: want_all_stopped = %s, want_all_running = %s, want_some_stopped = %s, want_some_running = %s, is_all_stopped = %s, is_all_running = %s, want_ss_running = %s\n",
                want_all_stopped ? "t" : "f",
                want_all_running ? "t" : "f",
                want_some_stopped ? "t" : "f",
                want_some_running ? "t" : "f",
                is_all_stopped ? "t" : "f",
                is_all_running ? "t" : "f",
                want_ss_running ? "t" : "f");

   if (want_ss_running) {
      pthrd_printf("Process %d is single-stepping.  Continuing select threads\n", getPid());
      for (int_threadPool::iterator i = threadPool()->begin(); i != threadPool()->end(); i++) {
         int_thread *thr = *i;
         if (thr->singleStep() && thr->getHandlerState().getState() == int_thread::stopped &&
             RUNNING_STATE(thr->getTargetState()))
         {
            bool tresult = thr->intCont();
            if (!tresult) {
               pthrd_printf("plat_syncRunState single-step failed on %d/%d\n", getPid(), thr->getLWP());
               result = false;
            }
         }
      }
   }
   else if (want_all_running && is_all_running) {
      pthrd_printf("Process %d is running, needs to run.  Doing nothing\n", getPid());
   }
   else if (want_all_running && is_all_stopped) {
      pthrd_printf("Process %d is stopped, needs to run.  Continuing process\n", getPid());
      result = threadPool()->initialThread()->intCont();
   }
   else if (want_all_stopped && is_all_running) {
      pthrd_printf("Process %d is running, needs to stop.  Stopping process\n", getPid());
      result = threadPool()->initialThread()->intStop();
   }
   else if (want_all_stopped && is_all_stopped) {
      pthrd_printf("Process %d is stopped, needs to stop.  Doing nothing\n", getPid());
   }
   else if (want_some_stopped && !is_all_stopped) {
      pthrd_printf("Process %d is partially stopped, needs to stop.  Stopping process\n", getPid());
      result = threadPool()->initialThread()->intStop();
   }
   else if (want_some_running && !is_all_running) {
      pthrd_printf("Process %d is partially running, needs to run.  Continuing process\n", getPid());
      result = threadPool()->initialThread()->intCont();
   }
   else {
      pthrd_printf("Process %d is in startup or teardown state.  Doing nothing\n", getPid());
   }

   if (!result) {
      pthrd_printf("plat_syncRunState operation failed.  Returning false\n");
      return false;
   }

   return true;
}

bool unified_lwp_control_process::plat_processGroupContinues()
{
   return true;
}

hybrid_lwp_control_process::hybrid_lwp_control_process(Dyninst::PID p, std::string e,
                                                         std::vector<std::string> a,
                                                         std::vector<std::string> envp,
                                                         std::map<int,int> f) :
  int_process(p, e, a, envp, f),
  debugger_stopped(false)
{
}

hybrid_lwp_control_process::hybrid_lwp_control_process(Dyninst::PID pid_, int_process *p) :
  int_process(pid_, p),
  debugger_stopped(false)
{
  // Clone debugger_stopped from parent
  hybrid_lwp_control_process *par = dynamic_cast<hybrid_lwp_control_process *>(p);
  assert(par); // Otherwise we have a very, very strange system
  debugger_stopped = par->debugger_stopped;
  pthrd_printf("Set debugger stopped to %s on %d, matching parent %d\n",
	       (debugger_stopped ? "true" : "false"),
	       pid_,
	       p->getPid());

}

hybrid_lwp_control_process::~hybrid_lwp_control_process()
{
}

bool hybrid_lwp_control_process::suspendThread(int_thread *thr)
{
   if (thr->isSuspended())
      return true;

   bool result = plat_suspendThread(thr);
   if (!result)
      return false;
   thr->setSuspended(true);
   return true;
}

bool hybrid_lwp_control_process::resumeThread(int_thread *thr)
{
   if (!thr->isSuspended())
      return true;

   bool result = plat_resumeThread(thr);
   if (!result)
      return false;
   thr->setSuspended(false);
   return true;
}

bool hybrid_lwp_control_process::plat_processGroupContinues()
{
   return true;
}

void hybrid_lwp_control_process::noteNewDequeuedEvent(Event::ptr ev)
{
   if (ev->getSyncType() == Event::sync_process) {
	   pthrd_printf("Marking %d debugger suspended on event: %s\n", getPid(), ev->name().c_str());
      debugger_stopped = true;
      pthrd_printf("Setting, debugger stopped: %d (%p) (%d)\n", debugger_stopped, (void*)this, getPid());
   }
}

bool hybrid_lwp_control_process::plat_debuggerSuspended()
{
  pthrd_printf("Querying, debugger stopped: %d (%p) (%p) (%d)\n", debugger_stopped, (void*)&debugger_stopped, (void*)this, getPid());
   return debugger_stopped;
}

bool hybrid_lwp_control_process::plat_syncRunState()
{
   bool any_target_stopped = false, any_target_running = false;
   bool any_stopped = false, any_running = false;

   int_thread *a_running_thread = NULL;

   if (getState() == exited) {
      pthrd_printf("Returning from plat_syncRunState for exited process %d\n", getPid());
      return true;
   }

   int_threadPool *tp = threadPool();
   int_threadPool::iterator i;
   for (i = tp->begin(); i != tp->end(); i++) {
      int_thread *thr = *i;
      pthrd_printf("Checking %d/%d: state %s\n", getPid(), thr->getLWP(), int_thread::stateStr(thr->getUserState().getState()));
#if defined(os_freebsd)
	   if (thr->getUserState().getState() == int_thread::exited) {
	     // Let it go, man...
	     continue;
	   }
#endif
	   if (thr->getDetachState().getState() == int_thread::detached) {
	     pthrd_printf("%d/%d detached, skipping\n", getPid(), thr->getLWP());
	     continue;
	   }
	   if (!RUNNING_STATE(thr->getHandlerState().getState())) {
	     pthrd_printf("%d/%d not running, any_stopped = true\n", getPid(), thr->getLWP());
	     any_stopped = true;
	   } else {
	     pthrd_printf("%d/%d running, any_running = true\n", getPid(), thr->getLWP());
	     any_running = true;
	     if (!a_running_thread) a_running_thread = thr;
	   }
	   if (RUNNING_STATE(thr->getTargetState())) {
	     pthrd_printf("%d/%d target running, any_target_running = true\n", getPid(), thr->getLWP());
	     any_target_running = true;
	   }
	   if (!RUNNING_STATE(thr->getTargetState())) {
	     pthrd_printf("%d/%d target stopped, any_target_stopped = true\n", getPid(), thr->getLWP());
	     any_target_stopped = true;
	   }
   }
   if(!a_running_thread) {
     a_running_thread = tp->initialThread();
   }


   if (!any_target_running && !any_running) {
      pthrd_printf("Target process state %d is stopped and process is stopped, leaving\n", getPid());
      return true;
   }
   if (!any_target_stopped && !any_stopped) {
      pthrd_printf("Target process state %d is running and process is running, leaving\n", getPid());
      return true;
   }
   if (!plat_debuggerSuspended()) {
      pthrd_printf("Process %d is not debugger suspended, but have changes.  Stopping process.\n", getPid());
      return a_running_thread->intStop();
   }

   //If we're here, we must be debuggerSuspended, and thus no threads are running (!any_running)
   // since we didn't trigger the above if statement, we must have some thread we want to run
   // (any_target_running)
   assert(!any_running && any_target_running);
   pthrd_printf("Begin suspend/resume loop\n");
   for (i = tp->begin(); i != tp->end(); i++) {
      int_thread *thr = *i;
      bool result = true;
	  if (!thr->isUser()) {
		  // Pretend is member of Wu Tang clan -- Bill Williams, 13JUN2012
		  resumeThread(thr);
		  continue;
	  }
	  if (thr->getDetachState().getState() == int_thread::detached)
	    continue;

#if defined(os_freebsd)
	   if (thr->getUserState().getState() == int_thread::exited) {
	     // Let it go, man...
	     continue;
	   }
#endif

      if (thr->isSuspended() && RUNNING_STATE(thr->getTargetState())) {
         pthrd_printf("Resuming thread %d/%d\n", getPid(), thr->getLWP());
         result = resumeThread(thr);
      }
      if (thr->getDetachState().getState() == int_thread::detached)
         continue;
      if (RUNNING_STATE(thr->getTargetState())) {
         if (thr->isSuspended()) {
            pthrd_printf("Resuming thread %d/%d\n", getPid(), thr->getLWP());
            result = resumeThread(thr);
         }
         else {
            pthrd_printf("Thread %d/%d is already resumed, not resuming\n", getPid(), thr->getLWP());
         }
      }
      else if (!RUNNING_STATE(thr->getTargetState())) {
         if (!thr->isSuspended()) {
            pthrd_printf("Suspending thread %d/%d\n", getPid(), thr->getLWP());
            result = suspendThread(thr);
         }
         else {
            pthrd_printf("Thread %d/%d is already suspended, not suspending\n", getPid(), thr->getLWP());
         }
      }
      if (!result) {
         pthrd_printf("Error suspending/resuming threads\n");
         return false;
      }
   }

   pthrd_printf("Continuing process %d after suspend/resume of threads\n", getPid());
   debugger_stopped = false;
   return threadPool()->initialThread()->intCont();
}

int_thread::int_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   tid(t),
   lwp(l),
   proc_(p),
   continueSig_(0),
   attach_status(as_unknown),
   handler_running_thrd_count(Counter::HandlerRunningThreads),
   generator_running_thrd_count(Counter::GeneratorRunningThreads),
   sync_rpc_count(Counter::SyncRPCs),
   sync_rpc_running_thr_count(Counter::SyncRPCRunningThreads),
   pending_stop(Counter::PendingStops),
   clearing_bp_count(Counter::ClearingBPs),
   proc_stop_rpc_count(Counter::ProcStopRPCs),
   generator_nonexited_thrd_count(Counter::GeneratorNonExitedThreads),
   neonatal_threads(Counter::NeonatalThreads),
   pending_stackwalk_count(Counter::PendingStackwalks),
   postponed_syscall_state(this, PostponedSyscallStateID, dontcare),
   exiting_state(this, ExitingStateID, dontcare),
   startup_state(this, StartupStateID, dontcare),
   pending_stop_state(this, PendingStopStateID, dontcare),
   callback_state(this, CallbackStateID, dontcare),
   breakpoint_state(this, BreakpointStateID, dontcare),
   breakpoint_hold_state(this, BreakpointHoldStateID, dontcare),
   breakpoint_resume_state(this, BreakpointResumeStateID, dontcare),
   irpc_setup_state(this, IRPCSetupStateID, dontcare),
   irpc_wait_state(this, IRPCWaitStateID, dontcare),
   irpc_state(this, IRPCStateID, dontcare),
   async_state(this, AsyncStateID, dontcare),
   internal_state(this, InternalStateID, dontcare),
   detach_state(this, DetachStateID, dontcare),
   user_irpc_state(this, UserRPCStateID, dontcare),
   control_authority_state(this, ControlAuthorityStateID, dontcare),
   user_state(this, UserStateID, neonatal),
   handler_state(this, HandlerStateID, neonatal),
   generator_state(this, GeneratorStateID, neonatal),
   target_state(int_thread::none),
   saved_user_state(int_thread::none),
   user_single_step(false),
   single_step(false),
   handler_exiting_state(false),
   generator_exiting_state(false),
   running_when_attached(true),
   suspended(false),
   user_syscall(false),
   next_syscall_is_exit(false),
   stopped_on_breakpoint_addr(0x0),
   postponed_stopped_on_breakpoint_addr(0x0),
   clearing_breakpoint(NULL),
   em_singlestep(NULL),
   user_data(NULL),
   unwinder(NULL),
   addr_fakeSyscallExitBp(0),
   isSet_fakeSyscallExitBp(false)
{
   Thread::ptr new_thr(new Thread());

   new_thr->llthread_ = this;
   up_thread = new_thr;

   getGeneratorNonExitedThreadCount().inc();
}

int_thread::~int_thread()
{
   assert(!up_thread->exitstate_);

   thread_exitstate *tes = new thread_exitstate();
   tes->lwp = lwp;
   tes->thr_id = tid;
   tes->proc_ptr = proc();
   tes->user_data = user_data;
   up_thread->exitstate_ = tes;
   up_thread->llthread_ = NULL;
}

bool int_thread::intStop()
{
   pthrd_printf("intStop on thread %d/%d\n", llproc()->getPid(), getLWP());
   if (!llproc()->plat_processGroupContinues()) {
      assert(!RUNNING_STATE(target_state));
      assert(RUNNING_STATE(getHandlerState().getState()));

      if (hasPendingStop()) {
         pthrd_printf("Not throwing a second stop while another is in flight\n");
         return true;
      }
   }
   else {
      int_threadPool *tp = llproc()->threadPool();
      for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
         if ((*i)->hasPendingStop()) {
            pthrd_printf("Not throwing a second stop while another is in flight\n");
            return true;
         }
      }
   }

   setPendingStop(true);
   bool result = plat_stop();
   if (!result) {
      setPendingStop(false);
      if (getLastError() == err_noproc) {
         //Swallow this.
         result = true;
      }
   }
   return result;
}

bool int_thread::intCont()
{
   pthrd_printf("intCont on thread %d/%d\n", llproc()->getPid(), getLWP());
   if (!llproc()->plat_processGroupContinues()) {
      assert(RUNNING_STATE(target_state));
      assert(!RUNNING_STATE(getHandlerState().getState()));
   }

   async_ret_t aret = handleSingleStepContinue();
   if (aret == aret_async) {
      pthrd_printf("Postponing intCont on %d/%d due to single-step handling\n",
                   llproc()->getPid(), getLWP());
      return true;
   }
   else if (aret == aret_error) {
      pthrd_printf("Error in intCont %d/%d during single-step handling\n",
                   llproc()->getPid(), getLWP());
      return false;
   }

   ProcPool()->condvar()->lock();

   bool result = plat_cont();
   if (result) {
      if (llproc()->plat_processGroupContinues()) {
         int_threadPool *pool = llproc()->threadPool();
         for (int_threadPool::iterator i = pool->begin(); i != pool->end(); i++) {
            if ((*i)->isSuspended())
               continue;
            (*i)->getHandlerState().setState(int_thread::running);
            (*i)->getGeneratorState().setState(int_thread::running);
         }
      }
      else {
         getHandlerState().setState(int_thread::running);
         getGeneratorState().setState(int_thread::running);
      }
      triggerContinueCBs();
   }


   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();

   if (!result) {
      pthrd_printf("Failed to plat_cont %d/%d\n", llproc()->getPid(), getLWP());
      return false;
   }

   return true;
}

async_ret_t int_thread::handleSingleStepContinue()
{
   async_ret_t ret;
   set<int_thread *> thrds;

   if (llproc()->plat_processGroupContinues()) {
      int_threadPool *pool = llproc()->threadPool();
      for (int_threadPool::iterator i = pool->begin(); i != pool->end(); i++) {
         if (!(*i)->isSuspended() && (*i)->singleStepUserMode()) {
            thrds.insert(*i);
         }
      }

   }
   else if (singleStepUserMode()) {
      thrds.insert(this);
   }

   if (thrds.empty()) {
      //No threads are single-steping.
      return aret_success;
   }
   pthrd_printf("Found %d threads doing single step under continue.  Handling\n", (int) thrds.size());

   if (llproc()->plat_needsAsyncIO()) {
      /**
       * We want any async responses associated with an event, but this is under
       * a continue, which never has an associated event.  We'll make an EventNop
       * to accompany the async request.  However, since this isn't going to be
       * a common thing to have happen, we'll set a flag that just tells the
       * HandlerPool to create the nop event lazily only if anyone asks for it.
       **/
      llproc()->handlerPool()->setNopAsCurEvent();
   }

   for (set<int_thread *>::iterator i = thrds.begin(); i != thrds.end(); i++) {
      int_thread *thr = *i;
      vector<Address> addrs;
      async_ret_t aresult = llproc()->plat_needsEmulatedSingleStep(thr, addrs);
      if (aresult == aret_async) {
         pthrd_printf("Async return from plat_needsEmulatedSingleStep on %d/%d\n",
                      llproc()->getPid(), thr->getLWP());
         //We're not actually under a handler, so fake all the async handling
         // the handlerpool would have done if we were.
         set<response::ptr> resps;
         Event::ptr cur_nop_event = llproc()->handlerPool()->curEvent();
         llproc()->plat_getEmulatedSingleStepAsyncs(thr, resps);
         llproc()->handlerPool()->notifyOfPendingAsyncs(resps, cur_nop_event);
         llproc()->handlerPool()->markEventAsyncPending(cur_nop_event);
         ret = aret_async;
         goto done;
      }
      else if (aresult == aret_error) {
         pthrd_printf("Error in plat_needsEmultatedSingleStep on %d/%d\n",
                      llproc()->getPid(), thr->getLWP());
         ret = aret_error;
         goto done;
      }
      else if (addrs.empty()) {
         pthrd_printf("Thread %d/%d does not need emulated single-step\n",
                      llproc()->getPid(), thr->getLWP());
         continue;
      }

      pthrd_printf("Creating emulated single step for %d/%d\n",
                   llproc()->getPid(), thr->getLWP());
      emulated_singlestep *new_es = thr->getEmulatedSingleStep();
      if (!new_es)
         new_es = new emulated_singlestep(thr);
      for (vector<Address>::iterator j = addrs.begin(); j != addrs.end(); j++) {
         Address addr = *j;
         pthrd_printf("Installing emulated single-step breakpoint for %d/%d at %lx\n",
                      llproc()->getPid(), thr->getLWP(), addr);
         aresult = new_es->add(addr);
         if (aresult == aret_async) {
            pthrd_printf("Async return while installing breakpoint for emulated_singlestep\n");
            ret = aret_async;
            goto done;
         }
         if (aresult == aret_error) {
            pthrd_printf("Error return while installing breakpoint for emulated_singlestep\n");
            ret = aret_error;
            goto done;
         }
      }
   }

   ret = aret_success;
  done:

   if (llproc()->plat_needsAsyncIO())
      llproc()->handlerPool()->clearNopAsCurEvent();

   return ret;
}

bool int_thread::isStopped(int state_id)
{
   if (getHandlerState().getState() != stopped)
      return false;
   if (getStateByID(state_id).getState() != stopped &&
       getStateByID(state_id).getState() != ditto)
      return false;
   for (int i=0; i<state_id; i++) {
      if (getStateByID(i).getState() != dontcare)
         return false;
   }
   return true;
}

void int_thread::setPendingStop(bool b)
{
   pthrd_printf("Setting pending stop to %s, thread %d/%d\n", b ? "true" : "false", proc()->getPid(), getLWP());
   if (b) {
      pending_stop.inc();

      //You may ask why stopping moves the pending stop state to running.
      // We've sent the process a stop request, and we need that request to
      // be delivered.  The process won't take delivery of the stop request
      // unless it's running.
      getPendingStopState().desyncState(int_thread::running);
   }
   else {
      if (getPendingStopState().getState() != int_thread::dontcare) {
         getPendingStopState().restoreState();
         pending_stop.dec();
      }
      else {
         pthrd_printf("Pending stop state == dontcare, ignoring\n");
      }
   }
   pthrd_printf("\t Pending stop level is %d\n", pending_stop.localCount());
}

bool int_thread::hasPendingStop() const
{
   return pending_stop.local();
}

bool int_thread::wasRunningWhenAttached() const {
    return running_when_attached;
}

void int_thread::setRunningWhenAttached(bool b) {
    running_when_attached = b;
}

Process::ptr int_thread::proc() const
{
   return proc_->proc();
}

int_process *int_thread::llproc() const
{
   return proc_;
}

Dyninst::LWP int_thread::getLWP() const
{
   return lwp;
}

void int_thread::addContinueCB(continue_cb_t cb)
{
   continue_cbs.insert(cb);
}

void int_thread::triggerContinueCBs()
{
   bool sync_conts = llproc()->plat_processGroupContinues();
   for (set<continue_cb_t>::iterator i = continue_cbs.begin(); i != continue_cbs.end(); i++) {
      if (!sync_conts) {
         //Independent lwp control--only do this thread.
         (*i)(this);
      }
      else {
         //Threads are continued at once--do every thread
         int_threadPool *tp = llproc()->threadPool();
         for (int_threadPool::iterator j = tp->begin(); j != tp->end(); j++) {
            if ((*j)->isSuspended())
               continue;
            (*i)(*j);
         }
      }
   }
}

int_thread::StateTracker &int_thread::getPostponedSyscallState()
{
   return postponed_syscall_state;
}

int_thread::StateTracker &int_thread::getBreakpointState()
{
   return breakpoint_state;
}

int_thread::StateTracker &int_thread::getBreakpointHoldState()
{
   return breakpoint_hold_state;
}

int_thread::StateTracker &int_thread::getBreakpointResumeState()
{
   return breakpoint_resume_state;
}

int_thread::StateTracker &int_thread::getIRPCState()
{
   return irpc_state;
}

int_thread::StateTracker &int_thread::getIRPCSetupState()
{
   return irpc_setup_state;
}

int_thread::StateTracker &int_thread::getIRPCWaitState()
{
   return irpc_wait_state;
}

int_thread::StateTracker &int_thread::getAsyncState()
{
   return async_state;
}

int_thread::StateTracker &int_thread::getInternalState()
{
   return internal_state;
}

int_thread::StateTracker &int_thread::getDetachState()
{
   return detach_state;
}

int_thread::StateTracker &int_thread::getUserRPCState()
{
	return user_irpc_state;
}

int_thread::StateTracker &int_thread::getControlAuthorityState()
{
   return control_authority_state;
}

int_thread::StateTracker &int_thread::getUserState()
{
   return user_state;
}

int_thread::StateTracker &int_thread::getHandlerState()
{
   return handler_state;
}

int_thread::StateTracker &int_thread::getGeneratorState()
{
   return generator_state;
}

int_thread::StateTracker &int_thread::getExitingState()
{
   return exiting_state;
}

int_thread::StateTracker &int_thread::getStartupState()
{
   return startup_state;
}

int_thread::StateTracker &int_thread::getCallbackState()
{
   return callback_state;
}

int_thread::StateTracker &int_thread::getPendingStopState()
{
   return pending_stop_state;
}

int_thread::State int_thread::getTargetState() const
{
   return target_state;
}

void int_thread::setTargetState(State s)
{
	pthrd_printf("%d/%d: setting target state to %s\n", proc_->getPid(), getLWP(),
                RUNNING_STATE(s) ? "Running" : "Stopped");
   target_state = s;
}

int_thread::StateTracker &int_thread::getActiveState() {
   for (int i=0; i<int_thread::NumTargetStateIDs; i++) {
      if (all_states[i]->getState() != int_thread::dontcare) {
         return *all_states[i];
      }
   }
   assert(0); //At least user state should never be 'dontcare'
   return *all_states[0];
}

int_thread::StateTracker &int_thread::getStateByID(int id)
{
   switch (id) {
      case PostponedSyscallStateID: return postponed_syscall_state;
      case ExitingStateID: return exiting_state;
      case StartupStateID: return startup_state;
      case AsyncStateID: return async_state;
      case CallbackStateID: return callback_state;
      case PendingStopStateID: return pending_stop_state;
      case IRPCStateID: return irpc_state;
      case IRPCSetupStateID: return irpc_setup_state;
      case IRPCWaitStateID: return irpc_wait_state;
      case BreakpointStateID: return breakpoint_state;
      case BreakpointHoldStateID: return breakpoint_hold_state;
      case BreakpointResumeStateID: return breakpoint_resume_state;
      case InternalStateID: return internal_state;
      case DetachStateID: return detach_state;
      case UserRPCStateID: return user_irpc_state;
      case ControlAuthorityStateID: return control_authority_state;
      case UserStateID: return user_state;
      case HandlerStateID: return handler_state;
      case GeneratorStateID: return generator_state;
   }
   assert(0);
   return exiting_state;
}

std::string int_thread::stateIDToName(int id)
{
   switch (id) {
      case PostponedSyscallStateID: return "postponed syscall";
      case ExitingStateID: return "exiting";
      case StartupStateID: return "startup";
      case AsyncStateID: return "async";
      case CallbackStateID: return "callback";
      case PendingStopStateID: return "pending stop";
      case IRPCStateID: return "irpc";
      case IRPCSetupStateID: return "irpc setup";
      case IRPCWaitStateID: return "irpc wait";
      case BreakpointStateID: return "breakpoint";
      case BreakpointHoldStateID: return "bp hold";
      case BreakpointResumeStateID: return "breakpoint resume";
      case InternalStateID: return "internal";
      case UserRPCStateID: return "irpc user";
      case ControlAuthorityStateID: return "control authority";
      case UserStateID: return "user";
      case DetachStateID: return "detach";
      case HandlerStateID: return "handler";
      case GeneratorStateID: return "generator";
   }
   assert(0);
   return "";
}


const char *int_thread::stateStr(int_thread::State s)
{
   switch (s) {
      case none: return "none";
      case neonatal: return "neonatal";
      case neonatal_intermediate: return "neonatal_intermediate";
      case running: return "running";
      case stopped: return "stopped";
      case dontcare: return "dontcare";
      case ditto: return "ditto";
      case exited: return "exited";
      case detached: return "detached";
      case errorstate: return "errorstate";
   }
   assert(0);
   return NULL;
}

char int_thread::stateLetter(int_thread::State s)
{
   switch (s) {
      case none: return '0';
      case neonatal: return 'N';
      case neonatal_intermediate: return 'I';
      case running: return 'R';
      case stopped: return 'S';
      case dontcare: return '-';
      case ditto: return 'H';
      case exited: return 'X';
      case detached: return 'D';
      case errorstate: return 'E';
   }
   assert(0);
   return '\0';
}

Counter &int_thread::handlerRunningThreadsCount()
{
   return handler_running_thrd_count;
}

Counter &int_thread::generatorRunningThreadsCount()
{
   return generator_running_thrd_count;
}

Counter &int_thread::syncRPCCount()
{
   return sync_rpc_count;
}

Counter &int_thread::runningSyncRPCThreadCount()
{
   return sync_rpc_running_thr_count;
}

Counter &int_thread::pendingStopsCount()
{
   return pending_stop;
}

Counter &int_thread::clearingBPCount()
{
   return clearing_bp_count;
}

Counter &int_thread::procStopRPCCount()
{
   return proc_stop_rpc_count;
}

Counter &int_thread::getGeneratorNonExitedThreadCount()
{
   return generator_nonexited_thrd_count;
}

Counter &int_thread::neonatalThreadCount()
{
   return neonatal_threads;
}

Counter &int_thread::pendingStackwalkCount()
{
   return pending_stackwalk_count;
}

void int_thread::setContSignal(int sig)
{
   continueSig_ = sig;
}

int int_thread::getContSignal() {
    return continueSig_;
}

int_thread *int_thread::createThread(int_process *proc,
                                     Dyninst::THR_ID thr_id,
                                     Dyninst::LWP lwp_id,
                                     bool initial_thrd,
                                     attach_status_t astatus)
{
   // See if we already created a skeleton/dummy thread for this thread ID.
   int_thread *newthr = proc->threadPool()->findThreadByLWP(lwp_id);
   if (newthr) return newthr;

   newthr = createThreadPlat(proc, thr_id, lwp_id, initial_thrd);
   if(!newthr)
   {
	   pthrd_printf("createThreadPlat failed, returning NULL\n");
	   return NULL;
   }
   pthrd_printf("Creating %s thread %d/%d, thr_id = 0x%lx\n",
                initial_thrd ? "initial" : "new",
                proc->getPid(), newthr->getLWP(), (unsigned long)thr_id);

   proc->threadPool()->addThread(newthr);
   if (initial_thrd) {
      proc->threadPool()->setInitialThread(newthr);
   }
   ProcPool()->addThread(proc, newthr);
   newthr->attach_status = astatus;

   bool result = newthr->attach();
   if (!result) {
      pthrd_printf("Failed to attach to new thread %d/%d\n", proc->getPid(), lwp_id);
      newthr->getUserState().setState(errorstate);
      newthr->getHandlerState().setState(errorstate);
      newthr->getGeneratorState().setState(errorstate);
      ProcPool()->rmThread(newthr);
      proc->threadPool()->rmThread(newthr);
      return NULL;
   }

   if (newthr->isUser() && newthr->getUserState().getState() == neonatal) {
	   newthr->getUserState().setState(neonatal_intermediate);
	   newthr->getHandlerState().setState(neonatal_intermediate);
		newthr->getGeneratorState().setState(neonatal_intermediate);
   }

   return newthr;
}

void int_thread::changeLWP(Dyninst::LWP new_lwp)
{
  pthrd_printf("Changing LWP of %d/%d to %d\n", llproc()->getPid(), lwp, new_lwp);

  int_threadPool *tpool = llproc()->threadPool();
  map<Dyninst::LWP, int_thread *>::iterator i = tpool->thrds_by_lwp.find(lwp);
  assert(i != tpool->thrds_by_lwp.end());
  tpool->thrds_by_lwp.erase(i);
  tpool->thrds_by_lwp.insert(make_pair(new_lwp, this));

  ProcPool()->condvar()->lock();
  ProcPool()->rmThread(this);
  lwp = new_lwp;
  ProcPool()->addThread(llproc(), this);
  ProcPool()->condvar()->unlock();
}

void int_thread::throwEventsBeforeContinue()
{
  pthrd_printf("Checking thread %d/%d for events thrown before continue\n",
	       llproc()->getPid(), getLWP());
   if (llproc()->wasForcedTerminated()) return;

   Event::ptr new_ev;

   int_iRPC::ptr rpc = nextPostedIRPC();
   bp_instance *bpi = isStoppedOnBP();
   if (rpc && !runningRPC() && rpc->getState() == int_iRPC::Posted) {
      pthrd_printf("Found thread %d/%d ready to run IRPC, not continuing\n", llproc()->getPid(), getLWP());

      if (rpc->isProcStopRPC() || llproc()->plat_threadOpsNeedProcStop()) {
         getIRPCSetupState().desyncStateProc(int_thread::stopped);
      }
      else {
         getIRPCSetupState().desyncState(int_thread::stopped);
      }
      rpc->setState(int_iRPC::Prepping);
      new_ev = EventRPCLaunch::ptr(new EventRPCLaunch());
   }
   else if (bpi) {
      if (bpi->swBP()) {
         //Stop the process to clear a software breakpoint
         pthrd_printf("Found thread %d/%d to be stopped on a software BP, not continuing\n",
                      llproc()->getPid(), getLWP());
         getBreakpointState().desyncStateProc(int_thread::stopped);
         EventBreakpointClear::ptr evclear =  EventBreakpointClear::ptr(new EventBreakpointClear());
         evclear->getInternal()->stopped_proc = true;
         new_ev = evclear;
      }
      else {
         //No process stop needed for a hardware breakpoint
         pthrd_printf("Found thread %d/%d to be stopped on a hardware BP, not continuing\n",
                      llproc()->getPid(), getLWP());
         getBreakpointState().desyncState(int_thread::stopped);
         new_ev = EventBreakpointClear::ptr(new EventBreakpointClear());
      }
   }
   else {
      new_ev = llproc()->plat_throwEventsBeforeContinue(this);
   }

   if (new_ev) {
      new_ev->setProcess(proc());
      new_ev->setThread(thread());
      new_ev->setSyncType(Event::async);
      new_ev->setSuppressCB(true);
      mbox()->enqueue(new_ev);
   }
}

bool int_thread::suppressSanityChecks()
{
   return false;
}

bool int_thread::isExiting() const
{
    return handler_exiting_state;
}

void int_thread::setExiting(bool b)
{
    handler_exiting_state = b;
}

bool int_thread::isExitingInGenerator() const
{
    return generator_exiting_state;
}

void int_thread::setExitingInGenerator(bool b)
{
    generator_exiting_state = b;
}

void int_thread::cleanFromHandler(int_thread *thrd, bool should_delete)
{
   ProcPool()->condvar()->lock();

#if !defined(os_freebsd)
   thrd->getUserState().setState(int_thread::exited);
#else
   thrd->setExiting(true);
#endif

   if (should_delete) {
      thrd->getExitingState().setState(int_thread::exited);
	  if(ProcPool()->findThread(thrd->getLWP()) != NULL) {
	      ProcPool()->rmThread(thrd);
	  }
	  else {
			pthrd_printf("%d/%d already gone from top level ProcPool(), not removing\n",
				thrd->llproc()->getPid(), thrd->getLWP());
	  }
      thrd->llproc()->threadPool()->rmThread(thrd);
      delete thrd;
   }
   else {
      //If we're not yet deleting this thread, then we're dealing with
      // a pre-mature exit event.  The thread will be exiting soon, but
      // isn't there yet.  We'll run the thread instead to make sure it
      // reaches a proper exit.

      // If we're freeBSD, they don't give us a thread exit event and
      // so running only the exiting thread can result in a process
      // that is "executing" with no continued threads. That is bad.
#if !defined(os_freebsd)
      thrd->getExitingState().setState(int_thread::running);
#endif
   }
   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();
}

Thread::ptr int_thread::thread()
{
   return up_thread;
}

bool int_thread::getAllRegisters(allreg_response::ptr response)
{
   response->setThread(this);
   response->setProcess(llproc());

   pthrd_printf("Reading registers for thread %d\n", getLWP());

   regpool_lock.lock();
   if (cached_regpool.full && 0) {
      *response->getRegPool() = cached_regpool;
      response->getRegPool()->thread = this;
      response->markReady();
      pthrd_printf("Returning cached register set\n");
      regpool_lock.unlock();
      return true;
   }

   if (!llproc()->plat_needsAsyncIO())
   {
      pthrd_printf("plat_getAllRegisters on %d/%d\n", llproc()->getPid(), getLWP());
      bool result = plat_getAllRegisters(cached_regpool);
      if (!result) {
         pthrd_printf("plat_getAllRegisters returned error on %d\n", getLWP());
         response->markError();
         regpool_lock.unlock();
         return false;
      }
      cached_regpool.full = true;
      *(response->getRegPool()) = cached_regpool;
      response->getRegPool()->thread = this;
      response->markReady();
      regpool_lock.unlock();

      int_eventAsyncIO *iev = response->getAsyncIOEvent();
      if (iev) {
         pthrd_printf("Enqueueing new EventAsyncReadAllRegs into mailbox on synchronous platform\n");
         EventAsyncReadAllRegs::ptr ev = EventAsyncReadAllRegs::ptr(new EventAsyncReadAllRegs(iev));
         ev->setProcess(proc());
         ev->setThread(thread());
         ev->setSyncType(Event::async);
         mbox()->enqueue(ev);
      }

      pthrd_printf("Successfully retrieved all registers for %d\n", getLWP());
   }
   else
   {
      pthrd_printf("Async plat_getAllRegisters on %d/%d\n", llproc()->getPid(),
                   getLWP());
      regpool_lock.unlock();
      getResponses().lock();
      bool result = plat_getAllRegistersAsync(response);
      if (result) {
         getResponses().addResponse(response, llproc());
      }
      getResponses().unlock();
      getResponses().noteResponse();
      if (!result) {
         pthrd_printf("plat_getAllRegistersAsync returned error on %d\n", getLWP());
         return false;
      }
   }

   return true;
}

bool int_thread::setAllRegisters(int_registerPool &pool, result_response::ptr response)
{
   assert(getHandlerState().getState() == int_thread::stopped);
   assert(getGeneratorState().getState() == int_thread::stopped);
   response->setProcess(llproc());

   if (!llproc()->plat_needsAsyncIO()) {

      pthrd_printf("Setting registers for thread %d\n", getLWP());
      bool result = plat_setAllRegisters(pool);
      response->setResponse(result);
      if (!result) {
         pthrd_printf("plat_setAllRegisters returned error on %d\n", getLWP());
         return false;
      }

      int_eventAsyncIO *iev = response->getAsyncIOEvent();
      if (iev) {
         pthrd_printf("Enqueueing new EventAsyncSetAllRegs into mailbox on synchronous platform\n");
         EventAsyncSetAllRegs::ptr ev = EventAsyncSetAllRegs::ptr(new EventAsyncSetAllRegs(iev));
         ev->setProcess(proc());
         ev->setThread(thread());
         ev->setSyncType(Event::async);
         mbox()->enqueue(ev);
      }
      pthrd_printf("Successfully set all registers for %d\n", getLWP());
   }
   else {
      pthrd_printf("Async setting registers for thread %d\n", getLWP());
      getResponses().lock();
      bool result = plat_setAllRegistersAsync(pool, response);
      if (result) {
         getResponses().addResponse(response, llproc());
      }
      getResponses().unlock();
      getResponses().noteResponse();
      if (!result) {
         pthrd_printf("Error async setting registers on %d\n", getLWP());
         return false;
      }
   }

   regpool_lock.lock();
   cached_regpool = pool;
   cached_regpool.full = true;
   regpool_lock.unlock();

   return true;
}

bool int_thread::getRegister(Dyninst::MachRegister reg, reg_response::ptr response)
{
   bool ret_result = false;
   pthrd_printf("Get register value for thread %d, register %s\n", lwp, reg.name().c_str());
   response->setRegThread(reg, this);
   response->setProcess(llproc());

   if (!llproc()->plat_individualRegRead(reg, this))
   {
      //Convert the single get register access into a get all registers
      pthrd_printf("Platform does not support individual register access, "
                   "getting everything\n");
      if (!llproc()->plat_needsAsyncIO()) {
         int_registerPool pool;
         allreg_response::ptr allreg_resp = allreg_response::createAllRegResponse(&pool);
         bool result = getAllRegisters(allreg_resp);
         bool is_ready = allreg_resp->isReady();
         if (!result || allreg_resp->hasError()) {
            pthrd_printf("Unable to access full register set\n");
            return false;
         }
         assert(is_ready);
	 if(!is_ready) return false;

         response->setResponse(pool.regs[reg]);
      }
      else {
         allreg_response::ptr allreg_resp = allreg_response::createAllRegResponse(&cached_regpool);
         allreg_resp->setIndividualRegAccess(response, reg);
         getResponses().lock();
         getResponses().addResponse(response, llproc());
         getResponses().unlock();
         bool result = getAllRegisters(allreg_resp);
         if (!result) {
            pthrd_printf("Error accessing full register set\n");
            return false;
         }
         allreg_resp->isReady();
      }
      return true;
   }

   regpool_lock.lock();

   int_registerPool::reg_map_t::iterator i = cached_regpool.regs.find(reg);
   if (i != cached_regpool.regs.end()) {
      pthrd_printf("Had cached register value\n");
      response->setResponse(i->second);
   }
   else if (!llproc()->plat_needsAsyncIO()) {
      MachRegisterVal val = 0;
      bool result = plat_getRegister(reg, val);
      if (!result) {
         pthrd_printf("Error reading register value for %s on %d\n", reg.name().c_str(), lwp);
         response->markError(getLastError());
         goto done;
      }
      response->setResponse(val);
   }
   else {
      pthrd_printf("Async getting register for thread %d\n", getLWP());
      getResponses().lock();
      bool result = plat_getRegisterAsync(reg, response);
      if (result && !response->testReady()) {
         getResponses().addResponse(response, llproc());
      }
      getResponses().unlock();
      getResponses().noteResponse();
      if (!result) {
         pthrd_printf("Error getting async register for thread %d\n", getLWP());
         goto done;
      }
      ret_result = true;
      goto done;
   }

   pthrd_printf("Returning register value %lx for register %s on %d\n",
                response->getResult(), reg.name().c_str(), lwp);

   ret_result = true;
  done:
   regpool_lock.unlock();
   return ret_result;
}

bool int_thread::setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val,
                             result_response::ptr response)
{
   if (getGeneratorState().getState() == int_thread::exited) {
      // Happens when we force terminate someone at a breakpoint...
      return false;
   }
   if (!isGeneratorThread()) {
      assert(getHandlerState().getState() == int_thread::stopped);
      assert(getGeneratorState().getState() == int_thread::stopped);
   }
   response->setProcess(llproc());

   bool ret_result = false;
   pthrd_printf("%d/%d: Setting %s to 0x%lx...\n", proc()->getPid(), lwp, reg.name().c_str(), val);

   if (!llproc()->plat_individualRegSet())
   {
      pthrd_printf("Platform does not support individual register access, "
                   "setting everything\n");
      assert(!llproc()->plat_needsAsyncIO());
      int_registerPool pool;
      allreg_response::ptr allreg_resp = allreg_response::createAllRegResponse(&pool);
      bool result = getAllRegisters(allreg_resp);
      assert(allreg_resp->isReady());
      if (!result || allreg_resp->hasError()) {
         pthrd_printf("Unable to access full register set\n");
         return false;
      }
      pool.regs[reg] = val;
      result = setAllRegisters(pool, response);
      assert(response->isReady());
      if (!result || response->hasError()) {
         pthrd_printf("Unable to set full register set\n");
         return false;
      }
      return true;
   }

   pthrd_printf("Setting register %s for thread %d to %lx\n", reg.name().c_str(), getLWP(), val);
   regpool_lock.lock();

   MachRegister base_register = reg.getBaseRegister();
   if (!llproc()->plat_needsAsyncIO())
   {
      bool result = plat_setRegister(base_register, val);
      response->setResponse(result);
      if (!result) {
         response->markError(getLastError());
         pthrd_printf("Error setting register %s\n", base_register.name().c_str());
         goto done;
      }
   }
   else {
      pthrd_printf("Async setting register for thread %d\n", getLWP());
      getResponses().lock();
      bool result = plat_setRegisterAsync(reg, val, response);
      if (result) {
         getResponses().addResponse(response, llproc());
      }
      getResponses().unlock();
      getResponses().noteResponse();
      if (!result) {
         pthrd_printf("Error setting async register for thread %d\n", getLWP());
         goto done;
      }
   }

   cached_regpool.regs[base_register] = val;

   ret_result = true;
  done:
   regpool_lock.unlock();
   return ret_result;
}

void int_thread::updateRegCache(int_registerPool &pool)
{
   regpool_lock.lock();
   pool.full = true;
   cached_regpool = pool;
   regpool_lock.unlock();
}

void int_thread::updateRegCache(Dyninst::MachRegister reg,
                                Dyninst::MachRegisterVal val)
{
   regpool_lock.lock();
   cached_regpool.regs[reg] = val;
   regpool_lock.unlock();
}

bool int_thread::plat_getAllRegistersAsync(allreg_response::ptr)
{
   assert(0);
   return false;
}

bool int_thread::plat_getRegisterAsync(Dyninst::MachRegister,
                                       reg_response::ptr)
{
   assert(0);
   return false;
}

bool int_thread::plat_setAllRegistersAsync(int_registerPool &,
                                           result_response::ptr)
{
   assert(0);
   return false;
}

bool int_thread::plat_setRegisterAsync(Dyninst::MachRegister,
                                       Dyninst::MachRegisterVal,
                                       result_response::ptr)
{
   assert(0);
   return false;
}

bool int_thread::plat_handle_ghost_thread() { return true; }

void int_thread::addPostedRPC(int_iRPC::ptr rpc_)
{
   assert(rpc_);
   if (rpc_->isProcStopRPC() && posted_rpcs.empty()) {
      proc_stop_rpc_count.inc();
   }
   posted_rpcs.push_back(rpc_);
}

rpc_list_t *int_thread::getPostedRPCs()
{
   return &posted_rpcs;
}

bool int_thread::hasPostedRPCs()
{
   return (posted_rpcs.size() != 0);
}

void int_thread::setRunningRPC(int_iRPC::ptr rpc_)
{
   assert(!running_rpc);
   running_rpc = rpc_;

   if (stopped_on_breakpoint_addr) {
      pthrd_printf("Thread %d/%d is stopped on BP at 0x%lx and about to run RPC.  Postponing BP clear.\n",
                   llproc()->getPid(), getLWP(), stopped_on_breakpoint_addr);
      postponed_stopped_on_breakpoint_addr = stopped_on_breakpoint_addr;
      stopped_on_breakpoint_addr = 0x0;
   }

   if (rpc_->isProcStopRPC() && !proc_stop_rpc_count.local()) {
      proc_stop_rpc_count.inc();
   }
}

int_iRPC::ptr int_thread::runningRPC() const
{
   return running_rpc;
}

void int_thread::clearRunningRPC()
{
   if (postponed_stopped_on_breakpoint_addr) {
      pthrd_printf("Thread %d/%d is moving back to stopped breakpoint at %lx after IRPC completion\n",
                   llproc()->getPid(), getLWP(), postponed_stopped_on_breakpoint_addr);
      stopped_on_breakpoint_addr = postponed_stopped_on_breakpoint_addr;
      postponed_stopped_on_breakpoint_addr = 0x0;
   }

   if (running_rpc->isProcStopRPC()) {
      proc_stop_rpc_count.dec();
   }
   running_rpc = int_iRPC::ptr();
}

bool int_thread::saveRegsForRPC(allreg_response::ptr response)
{
   assert(!rpc_regs.full);
   response->setRegPool(&rpc_regs);
   bool ret = getAllRegisters(response);

   return ret;
}

bool int_thread::restoreRegsForRPC(bool clear, result_response::ptr response)
{
   assert(rpc_regs.full);

   bool result = setAllRegisters(rpc_regs, response);
   if (clear && result) {
      rpc_regs.regs.clear();
      rpc_regs.full = false;
   }
   return result;
}

bool int_thread::hasSavedRPCRegs()
{
   return rpc_regs.full;
}

void int_thread::incSyncRPCCount()
{
   if (!sync_rpc_count.local() && RUNNING_STATE(getHandlerState().getState())) {
      runningSyncRPCThreadCount().inc();
   }
   sync_rpc_count.inc();
}

void int_thread::decSyncRPCCount()
{
   sync_rpc_count.dec();
   if (!sync_rpc_count.local() && RUNNING_STATE(getHandlerState().getState())) {
      runningSyncRPCThreadCount().dec();
   }
}

bool int_thread::hasSyncRPC()
{
   return sync_rpc_count.local();
}

int_iRPC::ptr int_thread::nextPostedIRPC() const
{
   if (!posted_rpcs.size())
      return int_iRPC::ptr();
   return posted_rpcs.front();
}

bool int_thread::singleStepMode() const
{
   return single_step;
}

void int_thread::setSingleStepMode(bool s)
{
   single_step = s;
}

bool int_thread::singleStepUserMode() const
{
   return user_single_step;
}

void int_thread::setSingleStepUserMode(bool s)
{
   user_single_step = s;
}

bool int_thread::singleStep() const
{
   return single_step || user_single_step;
}

void int_thread::setSyscallUserMode(bool s)
{
    user_syscall = s;
}

bool int_thread::syscallUserMode() const
{
    return user_syscall;
}

bool int_thread::syscallMode() const
{
    return user_syscall;
}

bool int_thread::preSyscall()
{
    /* On Linux, we distinguish between system call entry and exit based on the
     * system call return value. This approach will always correctly label a
     * system call entry. Only in the cast where the invoked system call does
     * not exist (0 < syscall_number > __NR_syscall_max) will we incorrectly
     * label the system call exist as system call entry. */

    bool ret = !next_syscall_is_exit;
    MachRegisterVal syscallReturnValue;
    if (!plat_getRegister(MachRegister::getSyscallReturnValueReg(llproc()->getTargetArch()),
			  syscallReturnValue)) { ret = true; }

    if (syscallReturnValue == (MachRegisterVal)(-ENOSYS)) {
        ret = true;
    }
    next_syscall_is_exit = ret;

    return ret;
}

void int_thread::markClearingBreakpoint(bp_instance *bp)
{
   assert(!clearing_breakpoint || bp == NULL);
   pthrd_printf("%d/%d marking clearing bp %p (at 0x%lx)\n",
	   llproc()->getPid(), getLWP(), (void*)bp, bp ? bp->getAddr() : 0);
   clearing_breakpoint = bp;
   if (bp) {
      clearing_bp_count.inc();
   }
   else {
      clearing_bp_count.dec();
   }
}

void int_thread::markStoppedOnBP(bp_instance *bp)
{
   stopped_on_breakpoint_addr = bp ? bp->getAddr() : 0x0;
}

bp_instance *int_thread::isStoppedOnBP()
{
   if (!stopped_on_breakpoint_addr) {
      return NULL;
   }
   sw_breakpoint *swbp = llproc()->getBreakpoint(stopped_on_breakpoint_addr);
   if (swbp)
      return static_cast<bp_instance *>(swbp);
   hw_breakpoint *hwbp = getHWBreakpoint(stopped_on_breakpoint_addr);
   return static_cast<bp_instance *>(hwbp);
}

void int_thread::setTID(Dyninst::THR_ID tid_)
{
   tid = tid_;
}

bp_instance *int_thread::isClearingBreakpoint()
{
   return clearing_breakpoint;
}

bool int_thread::haveUserThreadInfo()
{
   return false;
}

bool int_thread::getTID(Dyninst::THR_ID &)
{
   perr_printf("Unsupported attempt to getTid on %d/%d\n", llproc()->getPid(), getLWP());
   setLastError(err_unsupported, "getTid not supported on this platform\n");
   return false;
}

bool int_thread::getStartFuncAddress(Dyninst::Address &)
{
   perr_printf("Unsupported attempt to get start func address on %d/%d\n", llproc()->getPid(), getLWP());
   setLastError(err_unsupported, "getStartFuncAddress not supported on this platform\n");
   return false;
}

bool int_thread::getStackBase(Dyninst::Address &)
{
   perr_printf("Unsupported attempt to get stack base on %d/%d\n", llproc()->getPid(), getLWP());
   setLastError(err_unsupported, "getStackBase not supported on this platform\n");
   return false;
}

bool int_thread::getStackSize(unsigned long &)
{
   perr_printf("Unsupported attempt to get stack size on %d/%d\n", llproc()->getPid(), getLWP());
   setLastError(err_unsupported, "getStackSize not supported on this platform\n");
   return false;
}

bool int_thread::getTLSPtr(Dyninst::Address &)
{
   perr_printf("Unsupported attempt to get TLS on %d/%d\n", llproc()->getPid(), getLWP());
   setLastError(err_unsupported, "getTLSPtr not supported on this platform\n");
   return false;
}

Dyninst::Address int_thread::getThreadInfoBlockAddr()
{
   perr_printf("Unsupported attempt to get ThreadInfoBlock Address on %d/%d\n",
	           llproc()->getPid(), getLWP());
   setLastError(err_unsupported, "getThreadInfoBlockAddr not supported on this platform\n");
   return 0;
}

unsigned int_thread::hwBPAvail(unsigned) {
   return 0;
}

EventBreakpoint::ptr int_thread::decodeHWBreakpoint(response::ptr &,
                                                    bool,
                                                    Dyninst::MachRegisterVal)
{
   return EventBreakpoint::ptr();
}

bool int_thread::addHWBreakpoint(hw_breakpoint *,
                                 bool,
                                 std::set<response::ptr> &,
                                 bool &)
{
   perr_printf("Tried to use hardware breakpoint on unsupported system\n");
   setLastError(err_unsupported, "Hardware breakpoints not supported on this platform\n");
   return false;
}

bool int_thread::rmHWBreakpoint(hw_breakpoint *,
                                bool,
                                std::set<response::ptr> &,
                                bool &)
{
   perr_printf("Tried to use hardware breakpoint on unsupported system\n");
   setLastError(err_unsupported, "Hardware breakpoints not supported on this platform\n");
   return false;
}

bool int_thread::bpNeedsClear(hw_breakpoint *)
{
   return false;
}

void int_thread::addEmulatedSingleStep(emulated_singlestep *es)
{
   assert(!em_singlestep);
   em_singlestep = es;
}

void int_thread::rmEmulatedSingleStep(emulated_singlestep *es)
{
  (void)es;

   assert(em_singlestep == es);
   delete em_singlestep;
   em_singlestep = NULL;
}

emulated_singlestep *int_thread::getEmulatedSingleStep()
{
   return em_singlestep;
}

void int_thread::clearRegCache()
{
   regpool_lock.lock();
   cached_regpool.regs.clear();
   cached_regpool.full = false;
   regpool_lock.unlock();
}

int_thread::StateTracker::StateTracker(int_thread *t, int id_, int_thread::State initial) :
   state(int_thread::none),
   id(id_),
   sync_level(0),
   up_thr(t)
{
   t->all_states[id] = this;
   setState(initial);
}

void int_thread::StateTracker::desyncState(State ns)
{
   sync_level++;
   pthrd_printf("Desyncing %d/%d %s state to level %d\n",
                up_thr->llproc()->getPid(), up_thr->getLWP(),
                getName().c_str(), sync_level);
   assert(id != int_thread::HandlerStateID && id != int_thread::GeneratorStateID && id != int_thread::UserStateID);
   if (ns != int_thread::none) {
      setState(ns);
   }
}


void int_thread::StateTracker::desyncStateProc(State ns)
{
   int_threadPool *pool = up_thr->llproc()->threadPool();
   for (int_threadPool::iterator i = pool->begin(); i != pool->end(); i++) {
      (*i)->getStateByID(id).desyncState(ns);
   }
   //Track the process level desyncs seperately.  This way if a
   // new thread appears we can initialized its states to be
   // consistent with the other threads.
   up_thr->llproc()->getProcDesyncdStates()[id]++;
}

void int_thread::StateTracker::restoreState()
{
   sync_level--;
   pthrd_printf("Restoring %d/%d %s state to level %d\n",
                up_thr->llproc()->getPid(), up_thr->getLWP(),
                getName().c_str(), sync_level);
   assert(id != int_thread::HandlerStateID && id != int_thread::GeneratorStateID && id != int_thread::UserStateID);
   assert(sync_level >= 0);
   if (sync_level == 0) {
      setState(int_thread::dontcare);
   }
}

void int_thread::StateTracker::restoreStateProc()
{
   int_threadPool *pool = up_thr->llproc()->threadPool();
   for (int_threadPool::iterator i = pool->begin(); i != pool->end(); i++) {
      (*i)->getStateByID(id).restoreState();
   }
   up_thr->llproc()->getProcDesyncdStates()[id]--;
}

int_thread::State int_thread::StateTracker::getState() const
{
   return state;
}

bool int_thread::StateTracker::isDesynced() const {
   return (sync_level != 0);
}

bool int_thread::StateTracker::setState(State to)
{
   std::string s = int_thread::stateIDToName(id);
   Dyninst::LWP thisLwp = up_thr->getLWP();
   Dyninst::PID pid = up_thr->llproc()->getPid();

   if (state == to) {
      pthrd_printf("Leaving %s state for %d/%d in state %s\n", s.c_str(), pid, thisLwp, stateStr(to));
      return true;
   }
   if (state == errorstate) {
      perr_printf("Attempted %s state reversion for %d/%d from errorstate to %s\n",
                  s.c_str(), pid, thisLwp, stateStr(to));
      return false;
   }
   if (state == exited && to != errorstate) {
      perr_printf("Attempted %s state reversion for %d/%d from exited to %s\n",
                  s.c_str(), pid, thisLwp, stateStr(to));
      return false;
   }
   if (to == neonatal && state != none) {
      perr_printf("Attempted %s state reversion for %d/%d from %s to neonatal\n",
                  s.c_str(), pid, thisLwp, stateStr(state));
      return false;
   }

   /**
    * We need to keep track of the running threads counts.  We'll do that here.
    **/
   if (RUNNING_STATE(to) && !RUNNING_STATE(state))
   {
      //We're moving a thread into a running state...
      if (id == int_thread::GeneratorStateID) {
         up_thr->generatorRunningThreadsCount().inc();
      }
      else if (id == int_thread::HandlerStateID) {
         up_thr->handlerRunningThreadsCount().inc();
         if (up_thr->syncRPCCount().local()) {
            up_thr->runningSyncRPCThreadCount().inc();
         }
      }
   }
   else if (RUNNING_STATE(state) && !RUNNING_STATE(to))
   {
      //We're moving a thread out of a running state...
      if (id == int_thread::GeneratorStateID) {
         up_thr->generatorRunningThreadsCount().dec();
	  }
      else if (id == int_thread::HandlerStateID) {
         up_thr->handlerRunningThreadsCount().dec();
         if (up_thr->syncRPCCount().local()) {
            up_thr->runningSyncRPCThreadCount().dec();
         }
      }
   }
   if (id == int_thread::GeneratorStateID && to == int_thread::exited) {
      up_thr->getGeneratorNonExitedThreadCount().dec();
   }
   if (id == int_thread::HandlerStateID) {
      if ((state == int_thread::neonatal || state == int_thread::neonatal_intermediate) &&
          (to != int_thread::neonatal && to != int_thread::neonatal_intermediate)) {
         //Moving away from neonatal/neonatal_intermediate
         up_thr->neonatalThreadCount().dec();
      }
      if ((state != int_thread::neonatal && state != neonatal_intermediate) &&
          (to == int_thread::neonatal || to == int_thread::neonatal_intermediate)) {
         //Moving into neonatal/neonatal_intermediate
         up_thr->neonatalThreadCount().inc();
      }
   }
   pthrd_printf("Changing %s state for %d/%d from %s to %s\n", s.c_str(), pid, thisLwp,
                stateStr(state), stateStr(to));
   state = to;

/*    Xiaozhu: the asserts can fail legitimately.
   if (up_thr->up_thread && !up_thr->suppressSanityChecks()) {
      int_thread::State handler_state = up_thr->getHandlerState().getState();
      int_thread::State generator_state = up_thr->getGeneratorState().getState();

 *
 *    The handler status and generator status of a thread are never in sync by design.
 *    We will need to revisit such design.
 *
      if (handler_state == stopped)
         assert(generator_state == stopped || generator_state == exited || generator_state == detached );
      if (generator_state == running)
         assert(handler_state == running);
   }
*/

   return true;
}

bool int_thread::StateTracker::setStateProc(State ns)
{
   bool had_error = false;
   int_threadPool *pool = up_thr->llproc()->threadPool();
   for (int_threadPool::iterator i = pool->begin(); i != pool->end(); i++) {
      int_thread *thr = *i;
      bool result = thr->getStateByID(id).setState(ns);
      if (!result)
         had_error = true;
   }
   return !had_error;
}

std::string int_thread::StateTracker::getName() const
{
   return int_thread::stateIDToName(id);
}

int int_thread::StateTracker::getID() const
{
   return id;
}

void int_thread::setSuspended(bool b)
{
   suspended = b;
}

bool int_thread::isSuspended() const
{
   return suspended;
}

void int_thread::setLastError(err_t ec, const char *es) {
   if (proc_)
      proc_->setLastError(ec, es);
}

hw_breakpoint *int_thread::getHWBreakpoint(Address a)
{
   std::set<hw_breakpoint *>::iterator i;
   for (i = hw_breakpoints.begin(); i != hw_breakpoints.end(); i++) {
      if ((*i)->getAddr() == a)
         return *i;
   }
   return NULL;
}

int_thread *int_threadPool::findThreadByLWP(Dyninst::LWP lwp)
{
   std::map<Dyninst::LWP, int_thread *>::iterator i = thrds_by_lwp.find(lwp);
   if (i == thrds_by_lwp.end())
      return NULL;
   return i->second;
}

int_thread *int_threadPool::initialThread() const
{
	//if(!initial_thread) {
//		initial_thread = *(threads.begin());
//	}
   return initial_thread;
}

bool int_threadPool::allHandlerStopped()
{
   for (iterator i = begin(); i != end(); i++) {
      if ((*i)->getHandlerState().getState() != int_thread::stopped)
         return false;
   }
   return true;
}

bool int_threadPool::allStopped(int state_id)
{
	for (iterator i = begin(); i != end(); i++) {
      if (!(*i)->isStopped(state_id))
         return false;
   }
   return true;
}

bool int_threadPool::hadMultipleThreads() const {
    return had_multiple_threads;
}

void int_threadPool::addThread(int_thread *thrd)
{
   Dyninst::LWP lwp = thrd->getLWP();
   std::map<Dyninst::LWP, int_thread *>::iterator i = thrds_by_lwp.find(lwp);
   assert (i == thrds_by_lwp.end());
   thrds_by_lwp[lwp] = thrd;
   threads.push_back(thrd);
   hl_threads.push_back(thrd->thread());

   if( threads.size() > 1 ) {
       had_multiple_threads = true;
   }
}

void int_threadPool::rmThread(int_thread *thrd)
{
	// On Windows we are not guaranteed that initial thread exit ==
	// process exit. So we have to handle the case where the initial thread goes away.
	// In particular this means we need to have a valid initial thread in the threadpool after
	// we finish removing this one, so that we have an arbitrary thread available
	// for continue calls.

	// FIXME: should probably kill most of the data duplication here. Log search for thread by LWP is
	// I guess legitimate, but there's AFAICT no reason for the initial thread to be anything other than
	// pinned to threads[0]. And the data duplication involved in threads by LWP is not a good thing
	// for consistency...

#if !defined(os_windows)
	assert(thrd != initial_thread);
#endif
	Dyninst::LWP lwp = thrd->getLWP();
   std::map<Dyninst::LWP, int_thread *>::iterator i = thrds_by_lwp.find(lwp);
   assert (i != thrds_by_lwp.end());
   thrds_by_lwp.erase(i);

   for (unsigned j=0; j<threads.size(); j++) {
      if (threads[j] != thrd)
         continue;
      threads[j] = threads[threads.size()-1];
      threads.pop_back();
      hl_threads[j] = hl_threads[hl_threads.size()-1];
      hl_threads.pop_back();
   }
   if(!threads.empty() && thrd == initial_thread)
   {
	   initial_thread = threads[0];
   }
}

void int_threadPool::noteUpdatedLWP(int_thread *thrd)
{
	std::map<Dyninst::LWP, int_thread *>::iterator i = thrds_by_lwp.begin();
	while(i != thrds_by_lwp.end())
	{
		if(i->second == thrd)
		{
			thrds_by_lwp.erase(i);
			thrds_by_lwp.insert(std::make_pair(thrd->getLWP(), thrd));
			return;
		}
		++i;
	}
}

void int_threadPool::clear()
{
   threads.clear();
   hl_threads.clear();
   thrds_by_lwp.clear();
   initial_thread = NULL;
}
void int_threadPool::setInitialThread(int_thread *thrd)
{
   initial_thread = thrd;
}

int_process *int_threadPool::proc() const
{
   return proc_;
}

unsigned int_threadPool::size() const
{
   return threads.size();
}

ThreadPool *int_threadPool::pool() const
{
   return up_pool;
}

void int_threadPool::saveUserState(Event::ptr ev)
{
   Event::SyncType et = ev->getSyncType();
   switch (et) {
      case Event::unset:
         assert(0);
         break;
      case Event::async:
         break;
      case Event::sync_thread: {
         int_thread *thr = ev->getThread()->llthrd();
         if (!thr)
            return;
         assert(thr->saved_user_state == int_thread::none);
         thr->saved_user_state = thr->getUserState().getState();
         thr->getUserState().setState(int_thread::stopped);
         break;
      }
      case Event::sync_process:
         for (iterator i = begin(); i != end(); i++) {
            int_thread *thr = *i;
            assert(thr->saved_user_state == int_thread::none);
            thr->saved_user_state = thr->getUserState().getState();
            thr->getUserState().setState(int_thread::stopped);
         }
         break;
   }
}

void int_threadPool::restoreUserState()
{
   for (iterator i = begin(); i != end(); i++) {
      int_thread *thr = *i;
      if (thr->saved_user_state == int_thread::none)
         continue;
      thr->getUserState().setState(thr->saved_user_state);
      thr->saved_user_state = int_thread::none;
   }
}

int_threadPool::int_threadPool(int_process *p) :
   initial_thread(NULL),
   proc_(p),
   had_multiple_threads(false)
{
   up_pool = new ThreadPool();
   up_pool->threadpool = this;
}

int_threadPool::~int_threadPool()
{
   assert(up_pool);
   delete up_pool;

   for (vector<int_thread*>::iterator i = threads.begin(); i != threads.end(); ++i)
   {
	   if(ProcPool()->findThread((*i)->getLWP()))
	   {
		   ProcPool()->rmThread(*i);
	   }
	   delete *i;
   }
}

int_breakpoint::int_breakpoint(Breakpoint::ptr up) :
   up_bp(up),
   to(0x0),
   isCtrlTransfer_(false),
   data(NULL),
   hw(false),
   hw_perms(0),
   hw_size(0),
   onetime_bp(false),
   onetime_bp_hit(false),
   procstopper(false),
   suppress_callbacks(false),
   offset_transfer(false)
{
}

int_breakpoint::int_breakpoint(Dyninst::Address to_, Breakpoint::ptr up, bool off) :
   up_bp(up),
   to(to_),
   isCtrlTransfer_(true),
   data(NULL),
   hw(false),
   hw_perms(0),
   hw_size(0),
   onetime_bp(false),
   onetime_bp_hit(false),
   procstopper(false),
   suppress_callbacks(false),
   offset_transfer(off)
{
}

int_breakpoint::int_breakpoint(unsigned int hw_prems_, unsigned int hw_size_, Breakpoint::ptr up) :
  up_bp(up),
  to(0x0),
  isCtrlTransfer_(false),
  data(NULL),
  hw(true),
  hw_perms(hw_prems_),
  hw_size(hw_size_),
  onetime_bp(false),
  onetime_bp_hit(false),
  procstopper(false),
  suppress_callbacks(false),
  offset_transfer(false)
{
}

int_breakpoint::~int_breakpoint()
{
}

bool int_breakpoint::isCtrlTransfer() const
{
   return isCtrlTransfer_;
}

Address int_breakpoint::toAddr() const
{
   return to;
}

void *int_breakpoint::getData() const
{
   return data;
}

void int_breakpoint::setData(void *v)
{
   data = v;
}

Breakpoint::weak_ptr int_breakpoint::upBreakpoint() const
{
   return up_bp;
}

void int_breakpoint::setThreadSpecific(Thread::const_ptr p)
{
   thread_specific.insert(p);
}

void int_breakpoint::setOneTimeBreakpoint(bool b)
{
   onetime_bp = b;
}

void int_breakpoint::markOneTimeHit()
{
   assert(onetime_bp);
   onetime_bp_hit = true;
}

bool int_breakpoint::isOneTimeBreakpoint() const
{
   return onetime_bp;
}

bool int_breakpoint::isOneTimeBreakpointHit() const
{
   return onetime_bp_hit;
}

bool int_breakpoint::isThreadSpecific() const
{
   return !thread_specific.empty();
}

bool int_breakpoint::isThreadSpecificTo(Thread::const_ptr p) const
{
   return thread_specific.find(p) != thread_specific.end();
}

void int_breakpoint::setProcessStopper(bool b)
{
   procstopper = b;
}

bool int_breakpoint::isProcessStopper() const
{
   return procstopper;
}

bool int_breakpoint::isHW() const
{
   return hw;
}

unsigned int_breakpoint::getHWSize() const
{
   return hw_size;
}

unsigned int_breakpoint::getHWPerms() const
{
   return hw_perms;
}

void int_breakpoint::setSuppressCallbacks(bool b)
{
   suppress_callbacks = b;
}

bool int_breakpoint::suppressCallbacks() const
{
   return suppress_callbacks;
}

bool int_breakpoint::isOffsetTransfer() const
{
   return offset_transfer;
}

bp_instance::bp_instance(Address addr_) :
   addr(addr_),
   installed(false),
   suspend_count(0),
   swbp(NULL),
   hwbp(NULL)
{
}

bp_instance::bp_instance(const bp_instance *ip) :
   bps(ip->bps),
   hl_bps(ip->hl_bps),
   addr(ip->addr),
   installed(ip->installed),
   suspend_count(ip->suspend_count),
   swbp(NULL),
   hwbp(NULL)
{
}

bp_instance::~bp_instance()
{
}

bool bp_instance::checkBreakpoint(int_breakpoint *bp, int_process *p)
{
   if (bp->isCtrlTransfer()) {
      for (set<int_breakpoint *>::iterator i = bps.begin(); i != bps.end(); i++) {
         if ((*i)->isCtrlTransfer()) {
            perr_printf("Error.  Attempted to add two control transfer breakpoints "
                        "at same place\n");
            for (auto j : p->memory()->procs) {
               j->setLastError(err_badparam, "Attempted two control transfer breakpoints at "
                                  "same location\n");
            }
            return false;
         }
      }
   }

   Breakpoint::ptr upbp = bp->upBreakpoint().lock();
   if (upbp != Breakpoint::ptr()) {
      //We keep the set of installed Breakpoints to keep the breakpoint shared
      // pointer reference counter from cleaning a Breakpoint while installed.
      hl_bps.insert(upbp);
   }
   bps.insert(bp);

   return true;
}

bool bp_instance::rmBreakpoint(int_process *proc, int_breakpoint *bp, bool &empty,
                               set<response::ptr> &resps)
{
   empty = false;
   set<int_breakpoint *>::iterator i = bps.find(bp);
   if (i == bps.end()) {
      perr_printf("Attempted to remove a non-installed breakpoint\n");
      proc->setLastError(err_badparam, "Breakpoint was not installed in process\n");
      return false;
   }
   bps.erase(i);

   set<Breakpoint::ptr>::iterator j = hl_bps.find(bp->upBreakpoint().lock());
   if (j != hl_bps.end()) {
      hl_bps.erase(j);
   }

   if (bps.empty()) {
      empty = true;
      bool result = uninstall(proc, resps);
      if (!result) {
         perr_printf("Failed to remove breakpoint at %lx\n", addr);
         proc->setLastError(err_internal, "Could not remove breakpoint\n");
         return false;
      }
   }

   return true;
}

Address bp_instance::getAddr() const
{
   return addr;
}

bp_instance::iterator bp_instance::begin()
{
   return bps.begin();
}

bp_instance::iterator bp_instance::end()
{
   return bps.end();
}

bool bp_instance::containsIntBreakpoint(int_breakpoint *bp) {
    return (bps.count(bp) > 0);
}

int_breakpoint *bp_instance::getCtrlTransferBP(int_thread *thrd)
{
   for (iterator i = begin(); i != end(); i++) {
      int_breakpoint *bp = *i;
      if (!bp->isCtrlTransfer())
         continue;
      if (thrd && bp->isThreadSpecific() && !bp->isThreadSpecificTo(thrd->thread()))
         continue;
      return bp;
   }
   return NULL;
}

bool bp_instance::isInstalled() const
{
   return installed;
}

sw_breakpoint *bp_instance::swBP()
{
   return swbp;
}

hw_breakpoint *bp_instance::hwBP()
{
   return hwbp;
}

bool bp_instance::suspend_common()
{
   suspend_count++;
   if (suspend_count > 1) {
      pthrd_printf("Breakpoint already suspended, suspend_count = %d\n",
                   suspend_count);
      return true;
   }
   return false;
}

bool bp_instance::resume_common()
{
   suspend_count--;
   assert(suspend_count >= 0);
   if (suspend_count > 0) {
      pthrd_printf("Breakpoint remaining suspended, suspend_count = %d\n", suspend_count);
      return true;
   }
   return false;
}

sw_breakpoint::sw_breakpoint(mem_state::ptr memory_, Address addr_) :
   bp_instance(addr_),
   memory(memory_),
   buffer_size(0),
   prepped(false),
   long_breakpoint(false)
{
   swbp = this;
   hwbp = NULL;
}

sw_breakpoint::sw_breakpoint(mem_state::ptr memory_,
                             const sw_breakpoint *ip) :
   bp_instance(ip),
   memory(memory_),
   buffer_size(ip->buffer_size),
   prepped(ip->prepped),
   long_breakpoint(ip->long_breakpoint)
{
   memcpy(buffer, ip->buffer, sizeof(buffer));
   swbp = this;
   hwbp = NULL;
}

sw_breakpoint::~sw_breakpoint()
{
}

sw_breakpoint *sw_breakpoint::create(int_process *proc, int_breakpoint *bp, Dyninst::Address addr)
{
   bool result;
   bp_install_state is;
   is.addr = addr;
   is.bp = bp;

   result = proc->addBreakpoint_phase1(&is);
   if (!result)
      return NULL;
   if (is.mem_resp && is.mem_resp->isPosted()) {
      result = int_process::waitForAsyncEvent(is.mem_resp);
      if (!result) {
         perr_printf("Error waiting for result of memory response\n");
         return NULL;
      }
   }

   result = proc->addBreakpoint_phase2(&is);
   if (!result)
      return NULL;
   if (is.res_resp && is.res_resp->isPosted()) {
      result = int_process::waitForAsyncEvent(is.res_resp);
      if (!result) {
         perr_printf("Error waiting for result of result response\n");
         return NULL;
      }
   }

   result = proc->addBreakpoint_phase3(&is);
   if (!result)
      return NULL;

   return is.ibp;
}

bool sw_breakpoint::writeBreakpoint(int_process *proc, result_response::ptr write_response_)
{
   assert(buffer_size != 0);
   unsigned char bp_insn[BP_BUFFER_SIZE];
   proc->plat_breakpointBytes(bp_insn);
   if (long_breakpoint) {
      unsigned bp_size = proc->plat_breakpointSize();
      for (unsigned i=bp_size; i<bp_size+BP_LONG_SIZE; i++) {
         bp_insn[i] = buffer[i];
      }
   }
   return proc->writeMem(bp_insn, addr, buffer_size, write_response_, NULL, int_process::bp_install);
}

bool sw_breakpoint::saveBreakpointData(int_process *proc, mem_response::ptr read_response_)
{
   if (buffer_size != 0) {
      return true;
   }

   buffer_size = proc->plat_breakpointSize();
   if (long_breakpoint) {
      buffer_size += BP_LONG_SIZE;
   }
   pthrd_printf("Saving original data for breakpoint insertion at %lx +%u\n", addr, (unsigned) buffer_size);
   assert(buffer_size <= BP_BUFFER_SIZE);

   read_response_->setBuffer(buffer, buffer_size);
   bool ret = proc->readMem(addr, read_response_);
   if (read_response_->isReady()) {
      pthrd_printf("Buffer contents from read breakpoint:\n");
      for (int i = 0; i < buffer_size; ++i) {
         pthrd_printf("\t 0x%x\n", (unsigned char)buffer[i]);
      }
   }
   return ret;
}

bool sw_breakpoint::restoreBreakpointData(int_process *proc, result_response::ptr res_resp)
{
   assert(buffer_size != 0);

   pthrd_printf("Restoring original code over breakpoint at %lx\n", addr);
   return proc->writeMem(buffer, addr, buffer_size, res_resp, NULL, int_process::bp_clear);
}

async_ret_t sw_breakpoint::uninstall(int_process *proc, set<response::ptr> &resps)
{
   assert(installed);
   bool had_success = true;
   result_response::ptr async_resp = result_response::createResultResponse();

   if (proc->getState() != int_process::exited)
   {
      bool result = proc->writeMem(&buffer, addr, buffer_size, async_resp);
      if (!result) {
         pthrd_printf("Failed to remove breakpoint at %lx from process %d\n",
                      addr, proc->getPid());
         had_success = false;
      }
   }
   installed = false;
   buffer_size = 0;

   std::map<Dyninst::Address, sw_breakpoint *>::iterator i;
   i = memory->breakpoints.find(addr);
   if (i == memory->breakpoints.end()) {
      perr_printf("Failed to remove breakpoint from list\n");
      proc->setLastError(err_notfound, "Tried to uninstall breakpoint that isn't installed.\n");
      return aret_error;
   }
   memory->breakpoints.erase(i);

   if (async_resp->isPosted() && !async_resp->isReady()) {
      resps.insert(async_resp);
      return aret_async;
   }

   return had_success ? aret_success : aret_error;
}

async_ret_t sw_breakpoint::suspend(int_process *proc, set<response::ptr> &resps)
{
   if (suspend_common())
      return aret_success;

   result_response::ptr result_resp = result_response::createResultResponse();
   bool result = restoreBreakpointData(proc, result_resp);
   if (!result) {
      pthrd_printf("Failed to suspend breakpoint at %lx from process %d\n",
                   addr, proc->getPid());
      return aret_error;
   }
   if (result_resp->isPosted() && !result_resp->isReady()) {
      resps.insert(result_resp);
      return aret_async;
   }
   return aret_success;
}

async_ret_t sw_breakpoint::resume(int_process *proc, set<response::ptr> &resps)
{
   if (resume_common())
      return aret_success;
   result_response::ptr result_resp = result_response::createResultResponse();
   bool result = writeBreakpoint(proc, result_resp);
   if (!result) {
      pthrd_printf("Failed to install breakpoint at %lx in process %d\n",
                   addr, proc->getPid());
      return aret_error;
   }
   if (result_resp->isPosted() && !result_resp->isReady()) {
      resps.insert(result_resp);
      return aret_async;
   }

   return aret_success;
}

unsigned sw_breakpoint::getNumIntBreakpoints() const {
    return bps.size();
}

bool sw_breakpoint::addToIntBreakpoint(int_breakpoint *bp, int_process *)
{
   memory->breakpoints[addr] = this;

   Breakpoint::ptr upbp = bp->upBreakpoint().lock();
   if (upbp != Breakpoint::ptr()) {
      //We keep the set of installed Breakpoints to keep the breakpoint shared
      // pointer reference counter from cleaning a Breakpoint while installed.
      hl_bps.insert(upbp);
   }
   bps.insert(bp);

   return true;
}

bool sw_breakpoint::prepBreakpoint(int_process *proc, mem_response::ptr mem_resp)
{
   assert(!prepped);
   assert(!installed);

   pthrd_printf("Prepping breakpoint at %lx\n", addr);
   bool result = saveBreakpointData(proc, mem_resp);
   if (!result) {
      pthrd_printf("Error, failed to save breakpoint data at %lx\n", addr);
      return false;
   }

   prepped = true;
   return true;
}

bool sw_breakpoint::insertBreakpoint(int_process *proc, result_response::ptr res_resp)
{
   assert(prepped);
   assert(!installed);

   bool result = writeBreakpoint(proc, res_resp);
   if (!result) {
      pthrd_printf("Error writing breakpoint\n");
      return false;
   }

   installed = true;
   return true;
}

bool sw_breakpoint::needsClear() {
   return true;
}

hw_breakpoint::hw_breakpoint(int_thread *thread, unsigned mode, unsigned size,
                             bool pwide, Dyninst::Address addr_) :
   bp_instance(addr_),
   hw_perms(mode),
   hw_size(size),
   proc_wide(pwide),
   thr(thread),
   error(false)
{
   swbp = NULL;
   hwbp = this;
}

hw_breakpoint::~hw_breakpoint()
{
}

bool hw_breakpoint::install(bool &done, set<response::ptr> &resps)
{
   bool result = thr->addHWBreakpoint(this, false, resps, done);
   if (!result)
      return false;
   if (!done)
      return true;

   thr->hw_breakpoints.insert(this);
   return true;
}

hw_breakpoint *hw_breakpoint::create(int_process *proc, int_breakpoint *bp, Dyninst::Address addr)
{
   unsigned mode = bp->getHWPerms();
   unsigned size = bp->getHWSize();

   set<int_thread *> thrds;
   bool proc_wide = bp->thread_specific.empty();
   if (!proc_wide) {
      for (set<Thread::const_ptr>::iterator i = bp->thread_specific.begin(); i != bp->thread_specific.end(); i++) {
         int_thread *llthrd = (*i)->llthrd();
         thrds.insert(llthrd);
         pthrd_printf("Adding thread-specific hardware breakpoint to %d/%d at %lx\n",
                      proc->getPid(), llthrd->getLWP(), addr);
      }
   }
   else {
      int_threadPool *tp = proc->threadPool();
      for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
         int_thread *llthrd = *i;
         thrds.insert(llthrd);
         pthrd_printf("Adding process-wide hardware breakpoint to %d/%d at %lx\n",
                      proc->getPid(), llthrd->getLWP(), addr);
      }
   }

   bool no_space = false;
   for (set<int_thread *>::iterator i = thrds.begin(); i != thrds.end(); i++) {
      int_thread *thr = *i;
      if (!thr->hwBPAvail(mode)) {
         perr_printf("Do not have space for new hardware breakpoint of mode 0%u on thread %d/%d\n",
                     mode, proc->getPid(), thr->getLWP());
         no_space = true;
      }
   }
   if (no_space) {
      proc->setLastError(err_bpfull, "Not enough space for a new hardware breakpoint\n");
      return NULL;
   }

   set<hw_breakpoint *> hw_bps;
   hw_breakpoint *first_bp = NULL;
   for (set<int_thread *>::iterator i = thrds.begin(); i != thrds.end(); i++) {
      int_thread *thr = *i;
      hw_breakpoint *hwbp = new hw_breakpoint(thr, mode, size, proc_wide, addr);
      assert(hwbp);
      if (!first_bp)
         first_bp = hwbp;
      hw_bps.insert(hwbp);
      hwbp->installed = true;
      if (!hwbp->checkBreakpoint(bp, proc))
         return NULL;
   }

   bool had_error = false;
   while (!hw_bps.empty()) {
      set<response::ptr> resps;
      for (set<hw_breakpoint *>::iterator i = hw_bps.begin(); i != hw_bps.end();) {
         hw_breakpoint *bps = *i;
         bool done = false;
         bool result = bps->install(done, resps);
         if (!result) {
            pthrd_printf("Error installing HW breakpoint on %d/%d at %lx\n",
                         proc->getPid(), bps->getThread()->getLWP(), addr);
            had_error = true;
            break;
         }
         if (done)
            i = hw_bps.erase(i);
         else
            i++;
      }

      bool result = int_process::waitForAsyncEvent(resps);
      if (!result || had_error) {
         pthrd_printf("Error during HW BP install\n");
         return NULL;
      }
   }

   return first_bp;
}

async_ret_t hw_breakpoint::suspend(int_process *, set<response::ptr> &resps)
{
   if (suspend_common())
      return aret_success;

   bool done = false;

   bool result = thr->rmHWBreakpoint(this, true, resps, done);
   if (!result) {
      pthrd_printf("Error suspending HWBreakpoint\n");
      return aret_error;
   }
   if (!done) {
      assert(!resps.empty());
      return aret_async;
   }
   return aret_success;
}

async_ret_t hw_breakpoint::resume(int_process *, set<response::ptr> &resps)
{
   if (resume_common())
      return aret_success;

   bool done = false;

   bool result = thr->addHWBreakpoint(this, true, resps, done);
   if (!result) {
      pthrd_printf("Error resuming HWBreakpoint\n");
      return aret_error;
   }
   if (!done) {
      assert(!resps.empty());
      return aret_error;
   }
   return aret_success;
}

async_ret_t hw_breakpoint::uninstall(int_process *proc, set<response::ptr> &resps)
{
   bool done = false;

   set<hw_breakpoint *>::iterator ibp = thr->hw_breakpoints.find(this);
   if (ibp == thr->hw_breakpoints.end()) {
      perr_printf("Tried to uninstall non-installed hardware breakpoint on %d/%d\n",
                  thr->llproc()->getPid(), thr->getLWP());
      proc->setLastError(err_badparam, "Tried to uninstall non-installed hardware breakpoint\n");
      return aret_error;
   }

   bool result = thr->rmHWBreakpoint(this, false, resps, done);
   if (!result) {
      pthrd_printf("Error suspending HWBreakpoint\n");
      return aret_error;
   }
   thr->hw_breakpoints.erase(ibp);
   installed = false;

   if (!done) {
      assert(!resps.empty());
      return aret_async;
   }
   return aret_success;
}

unsigned int hw_breakpoint::getPerms() const
{
   return hw_perms;
}

unsigned int hw_breakpoint::getSize() const
{
   return hw_size;
}

int_thread *hw_breakpoint::getThread() const {
   return thr;
}

bool hw_breakpoint::needsClear() {
   return thr->bpNeedsClear(this);
}

int_library::int_library(std::string n, bool shared_lib,
                         Dyninst::Address load_addr,
                         Dyninst::Address dynamic_load_addr, Dyninst::Address data_load_addr,
                         bool has_data_load_addr) :
   name(n),
   load_address(load_addr),
   data_load_address(data_load_addr),
   dynamic_address(dynamic_load_addr),
   sysv_map_address(0),
   has_data_load(has_data_load_addr),
   marked(false),
   user_data(NULL),
   is_shared_lib(shared_lib),
   memory(NULL)
{
//   assert(n != "");
   up_lib = Library::ptr(new Library());
   up_lib->lib = this;
}

int_library::int_library(int_library *l) :
   name(l->name),
   load_address(l->load_address),
   data_load_address(l->data_load_address),
   dynamic_address(l->dynamic_address),
   sysv_map_address(l->sysv_map_address),
   has_data_load(l->has_data_load),
   marked(l->marked),
   user_data(NULL),
   is_shared_lib(l->is_shared_lib),
   memory(NULL)
{
   up_lib = Library::ptr(new Library());
   up_lib->lib = this;
}

int_library::~int_library()
{
}

std::string int_library::getName()
{
   return name;
}

std::string int_library::getAbsName()
{
   if (!abs_name.empty())
      return abs_name;
   abs_name = int_process::plat_canonicalizeFileName(name);
   return abs_name;
}

Dyninst::Address int_library::getAddr()
{
   return load_address;
}

Dyninst::Address int_library::getDataAddr()
{
   return data_load_address;
}

bool int_library::hasDataAddr()
{
   return has_data_load;
}

Dyninst::Address int_library::getDynamicAddr()
{
   return dynamic_address;
}

void int_library::setLoadAddress(Address addr)
{
   load_address = addr;
}

void int_library::setDynamicAddress(Address addr)
{
   dynamic_address = addr;
}

Address int_library::mapAddress()
{
   return sysv_map_address;
}

void int_library::setMapAddress(Address a)
{
   sysv_map_address = a;
}

bool int_library::inProcess(int_process *p)
{
   return (p->memory() == memory);
}

bool int_library::isSharedLib() const {
   return is_shared_lib;
}

void int_library::setMark(bool b)
{
   marked = b;
}

bool int_library::isMarked() const
{
   return marked;
}

void int_library::setUserData(void *d)
{
   user_data = d;
}

void *int_library::getUserData()
{
   return user_data;
}

Library::ptr int_library::getUpPtr() const
{
   return up_lib;
}

void int_library::markAsCleanable()
{
   //The destruction of the Library may destroy 'this', which
   //can cause an invalid heap write in the 'up_lib = Library::ptr()'
   //assignment.  By keeping a second reference around (delay_any_clean)
   //we make sure that up_lib is cleaned after it's assigned.
   Library::ptr delay_any_clean = up_lib;
   up_lib = Library::ptr();
}

mem_state::mem_state(int_process *proc)
{
   procs.insert(proc);
}

mem_state::mem_state(mem_state &m, int_process *p)
{
   pthrd_printf("Copying mem_state to new process %d\n", p->getPid());
   procs.insert(p);

   // Do not copy over libraries -- need to use refresh_libraries to
   // maintain consistency with AddressTranslate layer

   /*
   set<int_library *>::iterator i;
   for (i = m.libs.begin(); i != m.libs.end(); i++)
   {
      int_library *orig_lib = *i;
      int_library *new_lib = new int_library(orig_lib);
      libs.insert(new_lib);
   }
   */

   map<Dyninst::Address, sw_breakpoint *>::iterator j;
   for (j = m.breakpoints.begin(); j != m.breakpoints.end(); j++)
   {
      Address orig_addr = j->first;
      sw_breakpoint *orig_bp = j->second;
      sw_breakpoint *new_bp = new sw_breakpoint(this, orig_bp);
      breakpoints[orig_addr] = new_bp;
   }
   inf_malloced_memory = m.inf_malloced_memory;
}

mem_state::~mem_state()
{
   pthrd_printf("Destroy memory image of old process\n");
   set<int_library *>::iterator i;
   for (i = libs.begin(); i != libs.end(); i++) {
      (*i)->markAsCleanable();
   }
   libs.clear();

   map<Dyninst::Address, sw_breakpoint *>::iterator j;
   for (j = breakpoints.begin(); j != breakpoints.end(); j++)
   {
      sw_breakpoint *ibp = j->second;
      delete ibp;
   }
   breakpoints.clear();
}

void mem_state::addProc(int_process *p)
{
   pthrd_printf("Adding process %d as sharing a memory state with existing proc\n",
                p->getPid());
   procs.insert(p);
}

void mem_state::rmProc(int_process *p, bool &should_clean)
{
   set<int_process *>::iterator i = procs.find(p);
   assert(i != procs.end());
   procs.erase(i);

   if (procs.empty()) {
      should_clean = true;
      pthrd_printf("Removed process %d from memory image, should clean image\n",
                   p->getPid());
   }
   else {
      should_clean = false;
      pthrd_printf("Removed process %d from memory image, others remain\n",
                   p->getPid());
   }
}

void mem_state::addLibrary(int_library *lib)
{
   libs.insert(lib);
   lib->memory = this;
}

void mem_state::rmLibrary(int_library *lib)
{
   set<int_library*>::iterator i = libs.find(lib);
   if (i == libs.end())
      return;
   libs.erase(i);
   lib->memory = NULL;
}

int_notify *int_notify::the_notify = NULL;
int_notify::int_notify() :
   events_noted(0)
{
   the_notify = this;
   up_notify.llnotify = this;
}

int_notify *notify()
{
   if (int_notify::the_notify)
      return int_notify::the_notify;

   static Mutex<> init_lock;
   init_lock.lock();
   if (!int_notify::the_notify) {
      int_notify::the_notify = new int_notify();
   }
   init_lock.unlock();
   return int_notify::the_notify;
}

void int_notify::noteEvent()
{
//MATT TODO lock around event pipe write/read when/if we move to process locks
   assert(isHandlerThread());
   if (events_noted == 0)
      my_internals.noteEvent();
   events_noted++;
   pthrd_printf("noteEvent - %d\n", events_noted);
   set<EventNotify::notify_cb_t>::iterator i;
   for (i = cbs.begin(); i != cbs.end(); ++i) {
      pthrd_printf("Calling notification CB\n");
      (*i)();
   }
}

static void notifyNewEvent()
{
   notify()->noteEvent();
}


void int_notify::clearEvent()
{
   assert(!isHandlerThread());
   events_noted--;
   pthrd_printf("clearEvent - %d\n", events_noted);
   if (events_noted == 0)
      my_internals.clearEvent();
}

bool int_notify::hasEvents()
{
   return (events_noted > 0);
}

void int_notify::registerCB(EventNotify::notify_cb_t cb)
{
   cbs.insert(cb);
}

void int_notify::removeCB(EventNotify::notify_cb_t cb)
{
   set<EventNotify::notify_cb_t>::iterator i = cbs.find(cb);
   if (i == cbs.end())
      return;
   cbs.erase(i);
}

int_notify::details_t::wait_object_t int_notify::getWaitable()
{
	if(!my_internals.internalsValid())
	{
		my_internals.createInternals();
	}
	return my_internals.getWaitObject();
}

Decoder::Decoder()
{
}

Decoder::~Decoder()
{
}

RegisterPool::RegisterPool()
{
   llregpool = new int_registerPool();
}

RegisterPool::RegisterPool(const RegisterPool &rp)
{
   llregpool = new int_registerPool();
   *llregpool = *rp.llregpool;
}

RegisterPool::~RegisterPool()
{
   delete llregpool;
}

RegisterPool::iterator RegisterPool::begin()
{
   return RegisterPool::iterator(llregpool->regs.begin());
}

RegisterPool::iterator RegisterPool::end()
{
   return RegisterPool::iterator(llregpool->regs.end());
}

RegisterPool::iterator RegisterPool::find(MachRegister r)
{
   return RegisterPool::iterator(llregpool->regs.find(r));
}

bool RegisterPool::iterator::operator==(const iterator &iter) const
{
    return i == iter.i;
}

bool RegisterPool::iterator::operator!=(const iterator &iter) const
{
    return i != iter.i;
}

RegisterPool::const_iterator RegisterPool::begin() const
{
   return RegisterPool::const_iterator(llregpool->regs.begin());
}

RegisterPool::const_iterator RegisterPool::end() const
{
   return RegisterPool::const_iterator(llregpool->regs.end());
}

RegisterPool::const_iterator RegisterPool::find(MachRegister r) const
{
	return RegisterPool::const_iterator(llregpool->regs.find(r));
}

bool RegisterPool::const_iterator::operator==(const const_iterator &iter) const
{
    return i == iter.i;
}

bool RegisterPool::const_iterator::operator!=(const const_iterator &iter) const
{
    return i != iter.i;
}

MachRegisterVal& RegisterPool::operator[](MachRegister r)
{
   return llregpool->regs[r];
}

const MachRegisterVal& RegisterPool::operator[](MachRegister r) const
{
   return llregpool->regs[r];
}

size_t RegisterPool::size() const
{
   return llregpool->regs.size();
}

Thread::ptr RegisterPool::getThread()
{
   return llregpool->thread->thread();
}

Thread::const_ptr RegisterPool::getThread() const
{
   return llregpool->thread->thread();
}

RegisterPool::iterator::iterator()
{
}

RegisterPool::iterator::iterator(int_iter i_) :
   i(i_)
{
}

std::pair<Dyninst::MachRegister, Dyninst::MachRegisterVal> RegisterPool::iterator::operator*()
{
   return *i;
}

RegisterPool::iterator RegisterPool::iterator::operator++() // prefix
{
   i++;
   return *this;
}

RegisterPool::iterator RegisterPool::iterator::operator++(int) // postfix
{
   RegisterPool::iterator orig = *this;
   i++;
   return orig;
}

RegisterPool::const_iterator::const_iterator()
{
}

RegisterPool::const_iterator::const_iterator(int_iter i_) :
   i(i_)
{
}

std::pair<Dyninst::MachRegister, Dyninst::MachRegisterVal> RegisterPool::const_iterator::operator*() const
{
   return *i;
}

RegisterPool::const_iterator RegisterPool::const_iterator::operator++() // prefix
{
   i++;
   return *this;
}

RegisterPool::const_iterator RegisterPool::const_iterator::operator++(int) // postfix
{
   RegisterPool::const_iterator orig = *this;
   i++;
   return orig;
}

void regpoolClearOnCont(int_thread *thr)
{
   pthrd_printf("Clearing register pool for %d/%d\n", thr->llproc()->getPid(), thr->getLWP());
   thr->clearRegCache();
}

int_registerPool::int_registerPool() :
   full(false),
   thread(NULL)
{
   static bool is_inited = false;
   if (!is_inited) {
      is_inited = true;
      int_thread::addContinueCB(regpoolClearOnCont);
   }
}

Library::Library() : lib(NULL)
{
}

Library::~Library()
{
   MTLock lock_this_func(MTLock::allow_generator);
   if (lib) {
      delete lib;
      lib = NULL;
   }
}

std::string Library::getName() const
{
   MTLock lock_this_func;
   return lib->getName();
}

std::string Library::getAbsoluteName() const
{
   MTLock lock_this_func;
   return lib->getAbsName();
}

Dyninst::Address Library::getLoadAddress() const
{
   MTLock lock_this_func;
   return lib->getAddr();
}

Dyninst::Address Library::getDataLoadAddress() const
{
   MTLock lock_this_func;
   return lib->getDataAddr();
}

Dyninst::Address Library::getDynamicAddress() const
{
   return lib->getDynamicAddr();
}

bool Library::isSharedLib() const {
   return lib->isSharedLib();
}

void *Library::getData() const
{
   return lib->getUserData();
}

void Library::setData(void *p) const
{
   lib->setUserData(p);
}

LibraryPool::LibraryPool() :
   proc(NULL)
{
}

LibraryPool::~LibraryPool()
{
}

size_t LibraryPool::size() const
{
   MTLock lock_this_func;
   if (!proc) {
      perr_printf("getExecutable on deleted process\n");
      globalSetLastError(err_exited, "Process is exited\n");
      return -1;
   }
   return proc->numLibs();
}

Library::ptr LibraryPool::getLibraryByName(std::string s)
{
   MTLock lock_this_func;
   if (!proc) {
      perr_printf("getLibraryByName on deleted process\n");
      globalSetLastError(err_exited, "Process is exited\n");
      return Library::ptr();
   }

   int_library *int_lib = proc->getLibraryByName(s);
   if (!int_lib)
      return Library::ptr();
   return int_lib->up_lib;
}

Library::const_ptr LibraryPool::getLibraryByName(std::string s) const
{
   MTLock lock_this_func;
   if (!proc) {
      perr_printf("getLibraryByName on deleted process\n");
      globalSetLastError(err_exited, "Process is exited\n");
      return Library::ptr();
   }

   int_library *int_lib = proc->getLibraryByName(s);
   if (!int_lib)
      return Library::const_ptr();
   return int_lib->up_lib;
}

Library::ptr LibraryPool::getExecutable()
{
   MTLock lock_this_func;
   if (!proc) {
      perr_printf("getExecutable on deleted process\n");
      globalSetLastError(err_exited, "Process is exited\n");
      return Library::ptr();
   }
   return proc->plat_getExecutable()->up_lib;
}

Library::const_ptr LibraryPool::getExecutable() const
{
   MTLock lock_this_func;
   if (!proc) {
      perr_printf("getExecutable on deleted process\n");
      globalSetLastError(err_exited, "Process is exited\n");
      return Library::ptr();
   }
   return proc->plat_getExecutable()->up_lib;
}

LibraryPool::iterator LibraryPool::find(Library::ptr lib) {
   LibraryPool::iterator i{};
   i.int_iter = proc->memory()->libs.find(lib->debug());
   return i;
}

LibraryPool::const_iterator LibraryPool::find(Library::ptr lib) const {
   LibraryPool::const_iterator i{};
   i.int_iter = proc->memory()->libs.find(lib->debug());
   return i;
}

LibraryPool::iterator::iterator()
{
}

Library::ptr LibraryPool::iterator::operator*() const
{
   return (*int_iter)->up_lib;
}

LibraryPool::iterator LibraryPool::iterator::operator++() // prefix
{
   ++int_iter;
   return *this;
}

LibraryPool::iterator LibraryPool::iterator::operator++(int) // postfix
{
   LibraryPool::iterator orig = *this;
   ++int_iter;
   return orig;
}

LibraryPool::iterator LibraryPool::begin()
{
   LibraryPool::iterator i{};
   i.int_iter = proc->memory()->libs.begin();
   return i;
}

LibraryPool::iterator LibraryPool::end()
{
   LibraryPool::iterator i{};
   i.int_iter = proc->memory()->libs.end();
   return i;
}

bool LibraryPool::iterator::operator==(const LibraryPool::iterator &i) const
{
   return int_iter == i.int_iter;
}

bool LibraryPool::iterator::operator!=(const LibraryPool::iterator &i) const
{
   return int_iter != i.int_iter;
}

LibraryPool::const_iterator LibraryPool::begin() const
{
   LibraryPool::const_iterator i{};
   i.int_iter = proc->memory()->libs.begin();
   return i;
}

LibraryPool::const_iterator LibraryPool::end() const
{
   LibraryPool::const_iterator i{};
   i.int_iter = proc->memory()->libs.end();
   return i;
}

LibraryPool::const_iterator::const_iterator()
{
}

Library::const_ptr LibraryPool::const_iterator::operator*() const
{
   return (*int_iter)->up_lib;
}

bool LibraryPool::const_iterator::operator==(const LibraryPool::const_iterator &i) const
{
   return int_iter == i.int_iter;
}

bool LibraryPool::const_iterator::operator!=(const LibraryPool::const_iterator &i) const
{
   return int_iter != i.int_iter;
}

LibraryPool::const_iterator LibraryPool::const_iterator::operator++() // prefix
{
   ++int_iter;
   return *this;
}

LibraryPool::const_iterator LibraryPool::const_iterator::operator++(int) // postfix
{
   LibraryPool::const_iterator orig = *this;
   ++int_iter;
   return orig;
}

bool Process::registerEventCallback(EventType evt, Process::cb_func_t cbfunc)
{
   MTLock lock_this_func(MTLock::allow_init);
   HandleCallbacks *cbhandler = HandleCallbacks::getCB();
   return cbhandler->registerCallback(evt, cbfunc);
}

bool Process::removeEventCallback(EventType evt, cb_func_t cbfunc)
{
   MTLock lock_this_func(MTLock::allow_init);
   HandleCallbacks *cbhandler = HandleCallbacks::getCB();
   return cbhandler->removeCallback(evt, cbfunc);
}

bool Process::removeEventCallback(EventType evt)
{
   MTLock lock_this_func(MTLock::allow_init);
   HandleCallbacks *cbhandler = HandleCallbacks::getCB();
   return cbhandler->removeCallback(evt);
}

bool Process::removeEventCallback(cb_func_t cbfunc)
{
   MTLock lock_this_func(MTLock::allow_init);
   HandleCallbacks *cbhandler = HandleCallbacks::getCB();
   return cbhandler->removeCallback(cbfunc);
}

bool Process::handleEvents(bool block)
{
   // One might think we should be delivering callbacks when taking this lock,
   //  but we'll do it under waitAndHandleEvent instead.
   MTLock lock_this_func(MTLock::allow_init);

   pthrd_printf("User triggered event handling\n");
   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      ProcControlAPI::globalSetLastError(err_incallback, "Cannot handleEvents from callback\n");
      return false;
   }

   bool result = int_process::waitAndHandleEvents(block);
   if (!result) {
      if (!block && ProcControlAPI::getLastError() == err_noevents)
         pthrd_printf("Polling Process::handleEvents returning false due to no events\n");
      else
         pthrd_printf("Error handling events for user\n");
      return false;
   }
   return true;
}

bool Process::setThreadingMode(thread_mode_t tm)
{
   MTLock lock_this_func(MTLock::allow_init);
   return mt()->setThreadMode(tm);
}

Process::ptr Process::createProcess(std::string executable,
                                    const std::vector<std::string> &argv,
                                    const std::vector<std::string> &envp,
                                    const std::map<int,int> &fds)
{
   MTLock lock_this_func(MTLock::allow_init, MTLock::deliver_callbacks);

   pthrd_printf("User asked to launch executable %s\n", executable.c_str());
   if (int_process::isInCB()) {
      perr_printf("User attempted call on process create while in CB, erroring.");
      ProcControlAPI::globalSetLastError(err_incallback, "Cannot createProcess from callback\n");
      return Process::ptr();
   }

   ProcPool()->condvar()->lock();

   Process::ptr newproc(new Process());
   int_process *llproc = int_process::createProcess(executable, argv, envp, fds);
   llproc->initializeProcess(newproc);

   int_processSet the_proc;
   the_proc.insert(newproc);
   bool result = int_process::create(&the_proc); //Releases procpool lock
   if (!result) {
      pthrd_printf("Unable to create process %s\n", executable.c_str());
      return Process::ptr();
   }

   return newproc;
}

Process::ptr Process::attachProcess(Dyninst::PID pid, std::string executable)
{
   MTLock lock_this_func(MTLock::allow_init, MTLock::deliver_callbacks);
   pthrd_printf("User asked to attach to process %d (%s)\n", pid, executable.c_str());
   if (int_process::isInCB()) {
      perr_printf("User attempted call on process attach while in CB, erroring.\n");
      ProcControlAPI::globalSetLastError(err_incallback, "Cannot attachProcess from callback\n");
      return Process::ptr();
   }

   ProcPool()->condvar()->lock();
   Process::ptr newproc(new Process());
   int_process *llproc = int_process::createProcess(pid, executable);
   llproc->initializeProcess(newproc);

   int_processSet the_proc;
   the_proc.insert(newproc);

   bool result = llproc->attach(&the_proc, false); //Releases procpool lock

   if (!result) {
      pthrd_printf("Unable to attach to process %d\n", pid);
      delete llproc;
      return Process::ptr();
   }

   return newproc;
}

Process::Process() :
   llproc_(NULL),
   exitstate_(NULL)
{
}

Process::~Process()
{
   if (exitstate_) {
      delete exitstate_;
      exitstate_ = NULL;
   }
}

void *Process::getData() const {
   MTLock lock_this_func;
   if (exitstate_) {
      return exitstate_->user_data;
   }
   return llproc_->user_data;
}

void Process::setData(void *p) const {
   MTLock lock_this_func;
   if (exitstate_) {
      exitstate_->user_data = p;
   }
   else {
      llproc_->user_data = p;
   }
}

Dyninst::PID Process::getPid() const
{
   MTLock lock_this_func(MTLock::allow_generator);
   if (!llproc_) {
      assert(exitstate_);
	  if(!exitstate_) return 0;
      return exitstate_->pid;
   }
   return llproc_->getPid();
}

const ThreadPool &Process::threads() const
{
   MTLock lock_this_func;
   static ThreadPool *err_pool;
   if (!llproc_) {
      perr_printf("threads on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      if (!err_pool) {
         err_pool = new ThreadPool();
      }
      return *err_pool;
   }

   return *(llproc_->threadPool()->pool());
}

ThreadPool &Process::threads()
{
   MTLock lock_this_func;
   static ThreadPool *err_pool;
   if (!llproc_) {
      perr_printf("threads on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      if (!err_pool) {
         err_pool = new ThreadPool();
      }
      return *err_pool;
   }

   return *(llproc_->threadPool()->pool());
}

const LibraryPool &Process::libraries() const
{
   MTLock lock_this_func;
   static LibraryPool *err_pool;
   if (!llproc_) {
      perr_printf("libraries on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      if (!err_pool) {
         err_pool = new LibraryPool();
         err_pool->proc = NULL;
      }
      return *err_pool;
   }

   return llproc_->libpool;
}

LibraryPool &Process::libraries()
{
   MTLock lock_this_func;
   static LibraryPool *err_pool;
   if (!llproc_) {
      perr_printf("libraries on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      if (!err_pool) {
         err_pool = new LibraryPool();
         err_pool->proc = NULL;
      }
      return *err_pool;
   }

   return llproc_->libpool;
}

bool Process::addLibrary(std::string library) {
   MTLock lock_this_func;
   if (!llproc_) {
      perr_printf("addLibrary on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }
   if (llproc_->getState() == int_process::detached) {
       perr_printf("addLibrary on detached process\n");
       setLastError(err_detached, "Process is detached\n");
       return false;
   }
   if (int_process::isInCB()) {
      perr_printf("User attempted addLibrary while in CB, erroring.");
      setLastError(err_incallback, "Cannot addLibrary from callback\n");
      return false;
   }
   if (hasRunningThread()) {
      perr_printf("User attempted to addLibrary on running process\n");
      setLastError(err_notstopped, "Attempted to addLibrary on running process\n");
      return false;
   }

   Injector inj(this);
   return inj.inject(library);
}

bool Process::continueProc()
{
   Process::ptr me = shared_from_this();
   ProcessSet::ptr ps = ProcessSet::newProcessSet(me);
   return ps->continueProcs();
}

bool Process::isCrashed() const
{
   MTLock lock_this_func;
   if (!llproc_) {
      assert(exitstate_);
      return exitstate_->crashed;
   }
   int crashSignal = 0;
   return (llproc_->getState() == int_process::exited && llproc_->getCrashSignal(crashSignal));
}

bool Process::isExited() const
{
   MTLock lock_this_func;
   int exitCode = 0;
   if (!llproc_) {
      assert(exitstate_);
      return exitstate_->exited;
   }
   return (llproc_->getState() == int_process::exited && llproc_->getExitCode(exitCode));
}

int Process::getCrashSignal() const
{
   MTLock lock_this_func;
   if (!llproc_) {
      assert(exitstate_);
      return exitstate_->crashed ? exitstate_->crash_signal : 0;
   }

   int crashSignal = 0;
   if (!(llproc_->getState() == int_process::exited && llproc_->getCrashSignal(crashSignal)))
      return 0;
   return crashSignal;
}

int Process::getExitCode() const
{
   MTLock lock_this_func;
   if (!llproc_) {
      assert(exitstate_);
      return !exitstate_->crashed ? exitstate_->exit_code : 0;
   }

   int exitCode = 0;
   if (!(llproc_->getState() == int_process::exited && llproc_->getExitCode(exitCode)))
      return 0;
   return exitCode;
}

bool Process::isDetached() const
{
    MTLock lock_this_func;
    PROC_EXIT_TEST("isDetached", false);

    return llproc_->getState() == int_process::detached;
}

bool Process::stopProc()
{
   ProcessSet::ptr ps = ProcessSet::newProcessSet(shared_from_this());
   return ps->stopProcs();
}

bool Process::detach(bool leaveStopped)
{
   ProcessSet::ptr ps = ProcessSet::newProcessSet(shared_from_this());
   return ps->detach(leaveStopped);
}

bool Process::temporaryDetach()
{
   ProcessSet::ptr ps = ProcessSet::newProcessSet(shared_from_this());
   return ps->temporaryDetach();
}

bool Process::reAttach()
{
   ProcessSet::ptr ps = ProcessSet::newProcessSet(shared_from_this());
   return ps->reAttach();
}

bool Process::terminate()
{
   if (!llproc_) return true; // Already terminated
   ProcessSet::ptr ps = ProcessSet::newProcessSet(shared_from_this());
   bool ret = ps->terminate();
   return ret;
}

bool Process::isTerminated() const
{
   MTLock lock_this_func;
   if (!llproc_) {
      return true;
   }
   return (llproc_->getState() == int_process::exited);
}

bool Process::hasStoppedThread() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("hasStoppedThread", false);

   int_threadPool::iterator i;
   for (i = llproc_->threadPool()->begin(); i != llproc_->threadPool()->end(); i++) {
      if ((*i)->getUserState().getState() == int_thread::stopped)
         return true;
   }
   return false;
}

bool Process::hasRunningThread() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("hasRunningThread", false);

   int_threadPool::iterator i;
   for (i = llproc_->threadPool()->begin(); i != llproc_->threadPool()->end(); ++i) {
		// Skip truly not-us-system-threads
	   if (!(*i)->isUser() && !(*i)->isRPCEphemeral()) continue;
       if ((*i)->getUserState().getState() == int_thread::running)
         return true;
   }
   return false;
}

bool Process::allThreadsStopped() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("allThreadsStopped", false);

   int_threadPool::iterator i;
   for (i = llproc_->threadPool()->begin(); i != llproc_->threadPool()->end(); i++) {
      if ((*i)->getUserState().getState() == int_thread::running || (*i)->getUserState().getState() == int_thread::detached)
         return false;
   }
   return true;
}

bool Process::allThreadsRunning() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("allThreadsRunning", false);

   int_threadPool::iterator i;
   for (i = llproc_->threadPool()->begin(); i != llproc_->threadPool()->end(); i++) {
      if ((*i)->getUserState().getState() == int_thread::stopped || (*i)->getUserState().getState() == int_thread::detached)
         return false;
   }
   return true;
}

bool Process::allThreadsRunningWhenAttached() const
{
    MTLock lock_this_func;
    PROC_EXIT_TEST("allThreadsRunningWhenAttached", false);

    for(int_threadPool::iterator i = llproc_->threadPool()->begin();
            i != llproc_->threadPool()->end(); ++i)
    {
        if( !(*i)->wasRunningWhenAttached() ) return false;
    }

    return true;
}

bool Process::runIRPCAsync(IRPC::ptr irpc)
{
   MTLock lock_this_func;
   PROC_EXIT_DETACH_TEST("runIRPCAsync", false);


   int_process *proc = llproc();
   int_iRPC::ptr rpc = irpc->llrpc()->rpc;
   rpc->setAsync(false);

   bool result = false;
   if (rpc->thread()) {
	   result = rpcMgr()->postRPCToThread(rpc->thread(), rpc);
   }
   else {
		result = rpcMgr()->postRPCToProc(proc, rpc);
   }
   if (!result) {
      pthrd_printf("postRPCToProc failed on %d\n", proc->getPid());
      return false;
   }

   int_thread::State user_state = rpc->thread()->getUserState().getState();
   rpc->setRestoreToState(user_state);

   result = rpc->thread()->getUserState().setState(int_thread::running);
   if (!result) {
      setLastError(err_internal, "Could not continue thread choosen for iRPC\n");
      perr_printf("Could not run user thread %d/%d\n", proc->getPid(), rpc->thread()->getLWP());
      return false;
   }

    //#sasha remove this afterwards
    //cerr << "Amount of threads in process: " << proc->threadPool()->size() <<endl;
    //auto thread = *(proc->threadPool()->begin());
    //thread->setSingleStepMode(true);
   llproc_->throwNopEvent();
   return true;
}


bool Process::runIRPCSync(IRPC::ptr irpc)
{
   MTLock lock_this_func;
   PROC_EXIT_DETACH_CB_TEST("runIRPCSync", false);
   pthrd_printf("Running SYNC RPC\n");
   bool result = runIRPCAsync(irpc);
   if (!result) return false;

   bool exited = false;
   int_process *proc = llproc();
   int_iRPC::ptr int_rpc = irpc->llrpc()->rpc;

   while (irpc->state() != IRPC::Done) {
      int_thread *thr = int_rpc->thread();
      if (thr && thr->isStopped(int_thread::UserStateID)) {
         pthrd_printf("RPC thread %d/%d was stopped during runIRPCSync, returning notrunning error\n",
                      proc->getPid(), thr->getLWP());
         setLastError(err_notrunning, "No threads are running to produce events\n");
         return false;
      }

      result = int_process::waitAndHandleForProc(true, proc, exited);
      if (exited) {
         perr_printf("Process %d exited while waiting for irpc completion\n", getPid());
         setLastError(err_exited, "Process exited during IRPC");
         return false;
      }
      if (!result) {
         if (getLastError() == err_notrunning)
            pthrd_printf("RPC thread was stopped during runIRPCSync\n");
         else
            perr_printf("Error waiting for process to finish iRPC\n");
         return false;
      }
   }
   return true;
}

// Apologies for the code duplication; if this works, refactor.
bool Thread::runIRPCAsync(IRPC::ptr irpc)
{
   int_iRPC::ptr rpc = irpc->llrpc()->rpc;
   rpc->setThread(llthrd());

   return getProcess()->runIRPCAsync(irpc);
}


bool Thread::runIRPCSync(IRPC::ptr irpc)
{
   int_iRPC::ptr rpc = irpc->llrpc()->rpc;
   rpc->setThread(llthrd());

	return getProcess()->runIRPCSync(irpc);
}

bool Process::postIRPC(IRPC::ptr irpc) const
{
   MTLock lock_this_func;
   PROC_EXIT_DETACH_TEST("postIRPC", false);

   int_process *proc = llproc();
   int_iRPC::ptr rpc = irpc->llrpc()->rpc;
   bool result = rpcMgr()->postRPCToProc(proc, rpc);
   if (!result) {
      pthrd_printf("postRPCToProc failed on %d\n", proc->getPid());
      return false;
   }
   llproc_->throwNopEvent();
	return true;
}

bool Process::getPostedIRPCs(std::vector<IRPC::ptr> &rpcs) const
{
   MTLock lock_this_func;
   PROC_EXIT_DETACH_TEST("getPostedIRPCs", false);
   int_threadPool *tp = llproc()->threadPool();
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); ++i)
   {
      int_thread *thr = *i;
      rpc_list_t *rpc_list = thr->getPostedRPCs();
      for (rpc_list_t::iterator j = rpc_list->begin(); j != rpc_list->end(); ++j) {
         IRPC::ptr up_rpc = (*j)->getIRPC().lock();
         if (up_rpc == IRPC::ptr())
            continue;
         rpcs.push_back(up_rpc);
      }
   }
   return true;
}

Dyninst::Architecture Process::getArchitecture() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getArchitecture", Dyninst::Arch_none);
   return llproc_->getTargetArch();
}

Dyninst::OSType Process::getOS() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getOS", Dyninst::OSNone);

   return llproc_->getOS();
}

bool Process::supportsLWPEvents() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("supportsLWPEvents", false);
   //Intentionally not testing plat_supportLWP*Destroy, which is complicated on BG
   return llproc_->plat_supportLWPCreate();
}

bool Process::supportsUserThreadEvents() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("supportsUserThreadEvents", false);
   return llproc_->plat_supportThreadEvents();
}

bool Process::supportsFork() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("supportsFork", false);
   return llproc_->plat_supportFork();
}

bool Process::supportsExec() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("supportsExec", false);
   return llproc_->plat_supportExec();
}

int Process::mem_perm::permVal() const {
   int tmp = 0;

   if (read)
      tmp += 4;

   if (write)
      tmp += 2;

   if (execute)
      tmp += 1;

   return tmp;
}

bool Process::mem_perm::operator<(const mem_perm& p) const {
   return permVal() < p.permVal();
}

bool Process::mem_perm::operator==(const mem_perm& p) const {
     return (read    == p.read) &&
            (write   == p.write) &&
            (execute == p.execute);
}

bool Process::mem_perm::operator!=(const mem_perm& p) const {
     return !(*this == p);
}

std::string Process::mem_perm::getPermName() const {
    if (isNone()) {
       return "NONE";
    } else if (isR()) {
       return "R";
    } else if (isX()) {
       return "X";
    } else if (isRW()) {
       return "RW";
    } else if (isRX()) {
       return "RX";
    } else if (isRWX()) {
       return "RWX";
    } else {
       return "Unsupported Permission";
    }
}

unsigned Process::getMemoryPageSize() const {
    if (!llproc_) {
        perr_printf("getMemoryPageSize on deleted process\n");
        setLastError(err_exited, "Process is exited\n");
        return false;
    }

    if (llproc_->getState() == int_process::detached) {
        perr_printf("getMemoryPageSize on detached process\n");
        setLastError(err_detached, "Process is detached\n");
        return false;
    }

    return llproc_->getTargetPageSize();
}

Dyninst::Address Process::mallocMemory(size_t size, Dyninst::Address addr)
{
   PROC_EXIT_DETACH_CB_TEST("mallocMemory", 0);
   ProcessSet::ptr pset = ProcessSet::newProcessSet(shared_from_this());
   AddressSet::ptr addrset = AddressSet::newAddressSet(pset, addr);
   bool result = pset->mallocMemory(size, addrset);
   if (!result) {
      return 0;
   }
   AddressSet::iterator i = addrset->begin();
   return i->first;
}

Dyninst::Address Process::mallocMemory(size_t size)
{
   PROC_EXIT_DETACH_CB_TEST("mallocMemory", 0);
   ProcessSet::ptr pset = ProcessSet::newProcessSet(shared_from_this());
   AddressSet::ptr addr_result = pset->mallocMemory(size);
   if (addr_result->empty()) {
      return 0;
   }
   return addr_result->begin()->first;
}

Dyninst::Address Process::findFreeMemory(size_t size)
{
   PROC_EXIT_DETACH_TEST("findFreeMemory", 0);
	return llproc()->plat_findFreeMemory(size);
}

bool Process::freeMemory(Dyninst::Address addr)
{
   PROC_EXIT_DETACH_CB_TEST("freeMemory", 0);
   Process::ptr this_ptr = shared_from_this();
   ProcessSet::ptr pset = ProcessSet::newProcessSet(this_ptr);
   AddressSet::ptr addrs = AddressSet::newAddressSet(pset, addr);
   return pset->freeMemory(addrs);
}

bool Process::writeMemory(Dyninst::Address addr, const void *buffer, size_t size) const
{
   MTLock lock_this_func;
   PROC_EXIT_DETACH_TEST("writeMemory", false);

   pthrd_printf("User wants to write memory to remote addr 0x%lx from buffer 0x%p of size %lu\n",
                addr, buffer, (unsigned long) size);
   result_response::ptr resp = result_response::createResultResponse();
   bool result = llproc_->writeMem(buffer, addr, size, resp);
   if (!result) {
      pthrd_printf("Error writing to memory\n");
      (void)resp->isReady();
      return false;
   }

   int_process::waitForAsyncEvent(resp);
   if (!resp->getResult() || resp->hasError()) {
      pthrd_printf("Error writing to memory async\n");
      return false;
   }
   return true;
}

bool Process::readMemory(void *buffer, Dyninst::Address addr, size_t size) const
{
   MTLock lock_this_func;
   PROC_EXIT_DETACH_TEST("readMemory", false);

   pthrd_printf("User wants to read memory from 0x%lx to 0x%p of size %lu\n",
                addr, buffer, (unsigned long) size);
   mem_response::ptr memresult = mem_response::createMemResponse((char *) buffer, size);
   bool result = llproc_->readMem(addr, memresult);
   if (!result) {
      pthrd_printf("Error reading from memory %lx on target process %d\n",
                   addr, llproc_->getPid());
      (void)memresult->isReady();
      return false;
   }

   int_process::waitForAsyncEvent(memresult);

   if (memresult->hasError()) {
      pthrd_printf("Error reading from memory %lx on target process %d\n",
                   addr, llproc_->getPid());
      return false;
   }
   return true;
}

bool Process::writeMemoryAsync(Dyninst::Address addr, const void *buffer, size_t size, void *opaque_val) const
{
   MTLock lock_this_func;
   PROC_EXIT_DETACH_TEST("writeMemoryAsync", false);
   pthrd_printf("User wants to async write memory to remote addr 0x%lx from buffer 0x%p of size %lu\n",
                addr, buffer, (unsigned long) size);
   result_response::ptr resp = result_response::createResultResponse();
   int_eventAsyncIO *iev = new int_eventAsyncIO(resp, int_eventAsyncIO::memwrite);
   iev->local_memory = const_cast<void *>(buffer);
   iev->remote_addr = addr;
   iev->size = size;
   iev->opaque_value = opaque_val;
   resp->setAsyncIOEvent(iev);

   bool result = llproc_->writeMem(buffer, addr, size, resp);
   if (!result) {
      pthrd_printf("Error writing to memory\n");
      (void)resp->isReady();
      return false;
   }
   llproc_->plat_preAsyncWait();

   return true;
}

bool Process::readMemoryAsync(void *buffer, Dyninst::Address addr, size_t size, void *opaque_val) const
{
   MTLock lock_this_func;
   PROC_EXIT_DETACH_TEST("readMemoryAsync", false);

   pthrd_printf("User wants to async read memory from 0x%lx to 0x%p of size %lu\n",
                addr, buffer, (unsigned long) size);

   mem_response::ptr memresult = mem_response::createMemResponse((char *) buffer, size);
   int_eventAsyncIO *iev = new int_eventAsyncIO(memresult, int_eventAsyncIO::memread);
   iev->local_memory = buffer;
   iev->remote_addr = addr;
   iev->size = size;
   iev->opaque_value = opaque_val;
   memresult->setAsyncIOEvent(iev);

   bool result = llproc_->readMem(addr, memresult);
   if (!result) {
      pthrd_printf("Error reading from memory %lx on target process %d\n",
                   addr, llproc_->getPid());
      (void)memresult->isReady();
      return false;
   }
   llproc_->plat_preAsyncWait();

   return true;
}

bool Process::getMemoryAccessRights(Dyninst::Address addr, mem_perm& rights) {
    if (!llproc_) {
        perr_printf("getMemoryAccessRights on deleted process\n");
        setLastError(err_exited, "Process is exited\n");
        return false;
    }

    if (llproc_->getState() == int_process::detached) {
        perr_printf("getMemoryAccessRights on detached process\n");
        setLastError(err_detached, "Process is detached\n");
        return false;
    }

    pthrd_printf("User wants to get Memory Rights at %lx\n", addr);

    if (!llproc_->getMemoryAccessRights(addr, rights)) {
        pthrd_printf("Error get rights from memory %lx on target process %d\n",
                     addr, llproc_->getPid());
       return false;
    }

    return true;
}

bool Process::setMemoryAccessRights(Dyninst::Address addr, size_t size,
                                    mem_perm rights, mem_perm& oldrights) {
    MTLock lock_this_func;
    if (!llproc_) {
        perr_printf("setMemoryAccessRights on deleted process\n");
        setLastError(err_exited, "Process is exited\n");
        return false;
    }

    if (llproc_->getState() == int_process::detached) {
        perr_printf("setMemoryAccessRights on detached process\n");
        setLastError(err_detached, "Process is detached\n");
        return false;
    }

    pthrd_printf("User wants to set Memory Rights to %s from [%lx %lx]\n",
                 rights.getPermName().c_str(), addr, addr+size);

    if (!llproc_->setMemoryAccessRights(addr, size, rights, oldrights)) {
        pthrd_printf("ERROR: set rights to %s from memory %lx on target process %d\n",
                     rights.getPermName().c_str(), addr, llproc_->getPid());
        return false;
    }

    return true;
}

bool Process::findAllocatedRegionAround(Dyninst::Address addr,
                                        MemoryRegion& memRegion) {
    if (!llproc_) {
        perr_printf("findAllocatedRegionAround on deleted process\n");
        setLastError(err_exited, "Process is exited\n");
        return false;
    }

    if (llproc_->getState() == int_process::detached) {
        perr_printf("findAllocatedRegionAround on detached process\n");
        setLastError(err_detached, "Process is detached\n");
        return false;
    }

    pthrd_printf("User wants to find Allocated Region contains %lx\n", addr);

    if (!llproc_->findAllocatedRegionAround(addr, memRegion)) {
        pthrd_printf("Error to find Allocated Region contains %lx on target process %d\n",
                     addr, llproc_->getPid());
        return false;
    }

    return true;
}

bool Process::addBreakpoint(Address addr, Breakpoint::ptr bp) const
{
   MTLock lock_this_func;
   PROC_EXIT_DETACH_TEST("addBreakpoint", false);

   if (hasRunningThread()) {
      perr_printf("User attempted to add breakpoint to running process\n");
      setLastError(err_notstopped, "Attempted to insert breakpoint into running process\n");
      return false;
   }

   if( llproc_->getState() == int_process::detached ) {
       perr_printf("User attempted to add breakpoint to detached process\n");
       setLastError(err_detached, "Attempted to insert breakpoint into detached process\n");
       return false;
   }

   return llproc_->addBreakpoint(addr, bp->llbp());
}

bool Process::rmBreakpoint(Dyninst::Address addr, Breakpoint::ptr bp) const
{
   MTLock lock_this_func;
   PROC_EXIT_DETACH_TEST("rmBreakpoint", false);

   if (hasRunningThread()) {
      perr_printf("User attempted to remove breakpoint on running process\n");
      setLastError(err_notstopped, "Attempted to remove breakpoint on running process\n");
      return false;
   }

   set<response::ptr> resps;
   bool result = llproc_->removeBreakpoint(addr, bp->llbp(), resps);
   if (!result) {
      pthrd_printf("Failed to removeBreakpoint\n");
      return false;
   }

   int_process::waitForAsyncEvent(resps);

   for (set<response::ptr>::iterator i = resps.begin(); i != resps.end(); i++) {
      response::ptr resp = *i;
      if (resp->hasError()) {
         pthrd_printf("Error removing breakpoint\n");
         return false;
      }
   }

   return true;

}

unsigned Process::numHardwareBreakpointsAvail(unsigned mode)
{
   MTLock lock_this_func;
   PROC_EXIT_DETACH_TEST("numHardwareBreakpointAvail", 0);

   unsigned min = INT_MAX;
   int_threadPool *tp = llproc_->threadPool();
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      int_thread *thr = *i;
      unsigned cur = thr->hwBPAvail(mode);
      if (cur < min)
         min = cur;
   }
   return min;
}

void Process::setDefaultSymbolReader(SymbolReaderFactory *f)
{
   int_process::user_set_symbol_reader = f;
}

SymbolReaderFactory *Process::getDefaultSymbolReader()
{
   return int_process::user_set_symbol_reader;
}

void Process::setSymbolReader(SymbolReaderFactory *f) const
{
   MTLock lock_this_func;
   if (!llproc_) {
      perr_printf("setSymbolReader on exited process\n");
      setLastError(err_exited, "Process is exited\n");
      return;
   }
   llproc_->setSymReader(f);
}

SymbolReaderFactory *Process::getSymbolReader() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getSymbolReader", NULL);
   return llproc_->getSymReader();
}

LibraryTracking *Process::getLibraryTracking()
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getLibraryTracking", NULL);
   int_libraryTracking *proc = llproc_->getLibraryTracking();
   if (!proc) return NULL;
   return proc->up_ptr;
}

ThreadTracking *Process::getThreadTracking()
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getThreadTracking", NULL);
   int_threadTracking *proc = llproc_->getThreadTracking();
   if (!proc) return NULL;
   return proc->up_ptr;
}

LWPTracking *Process::getLWPTracking()
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getLWPTracking", NULL);
   int_LWPTracking *proc = llproc_->getLWPTracking();
   if (!proc) return NULL;
   return proc->up_ptr;
}

CallStackUnwinding *Thread::getCallStackUnwinding()
{
   MTLock lock_this_func;
   THREAD_EXIT_TEST("getCallStackUnwinding", NULL);
   int_callStackUnwinding *uwproc = llthread_->llproc()->getCallStackUnwinding();
   if (!uwproc)
      return NULL;
   if (!llthread_->unwinder) {
      llthread_->unwinder = new CallStackUnwinding(shared_from_this());
   }
   return llthread_->unwinder;
}

FollowFork *Process::getFollowFork()
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getFollowFork", NULL);
   int_followFork *proc = llproc_->getFollowFork();
   if (!proc) return NULL;
   return proc->up_ptr;
}

SignalMask *Process::getSignalMask()
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getSignalMask", NULL);
   int_signalMask *proc = llproc_->getSignalMask();
   if (!proc) return NULL;
   return proc->up_ptr;
}

RemoteIO *Process::getRemoteIO()
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getRemoteIO", NULL);
   int_remoteIO *proc = llproc_->getRemoteIO();
   if (!proc) return NULL;
   return proc->up_ptr;
}

MemoryUsage *Process::getMemoryUsage()
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getMemoryUsage", NULL);
   int_memUsage *proc = llproc_->getMemUsage();
   if (!proc) return NULL;
   return proc->up_ptr;
}

const LibraryTracking *Process::getLibraryTracking() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getLibraryTracking", NULL);
   int_libraryTracking *proc = llproc_->getLibraryTracking();
   if (!proc) return NULL;
   return proc->up_ptr;
}

const ThreadTracking *Process::getThreadTracking() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getThreadTracking", NULL);
   int_threadTracking *proc = llproc_->getThreadTracking();
   if (!proc) return NULL;
   return proc->up_ptr;
}

const LWPTracking *Process::getLWPTracking() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getLWPTracking", NULL);
   int_LWPTracking *proc = llproc_->getLWPTracking();
   if (!proc) return NULL;
   return proc->up_ptr;
}

const SignalMask *Process::getSignalMask() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getSignalMask", NULL);
   int_signalMask *proc = llproc_->getSignalMask();
   if (!proc) return NULL;
   return proc->up_ptr;
}

const RemoteIO *Process::getRemoteIO() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getRemoteIO", NULL);
   int_remoteIO *proc = llproc_->getRemoteIO();
   if (!proc) return NULL;
   return proc->up_ptr;
}

const MemoryUsage *Process::getMemoryUsage() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getMemoryUsage", NULL);
   int_memUsage *proc = llproc_->getMemUsage();
   if (!proc) return NULL;
   return proc->up_ptr;
}

err_t Process::getLastError() const {
   MTLock lock_this_func;
   if (!llproc_) {
      return exitstate_->last_error;
   }
   return llproc_->getLastError();
}

const char *Process::getLastErrorMsg() const {
   MTLock lock_this_func;
   if (!llproc_) {
      return exitstate_->last_error_msg;
   }
   return llproc_->getLastErrorMsg();
}

void Process::setLastError(err_t ec, const char *es) const {
   MTLock lock_this_func;
   if (!llproc_) {
      exitstate_->last_error = ec;
      exitstate_->last_error_msg = es;
      ProcControlAPI::globalSetLastError(ec, es);
   }
   else {
      llproc_->setLastError(ec, es);
   }
}

void Process::clearLastError() const {
  MTLock lock_this_func;
  if (!llproc_) {
     exitstate_->last_error = err_none;
     exitstate_->last_error_msg = "ok";
  }
  else {
     llproc_->clearLastError();
  }
}

ExecFileInfo* Process::getExecutableInfo() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getExecutableInfo", NULL);

   return llproc()->plat_getExecutableInfo();
}

unsigned int Process::getCapabilities() const
{
   MTLock lock_this_func;
   PROC_EXIT_TEST("getCapabilities", 0);
   return llproc()->plat_getCapabilities();
}

Thread::Thread() :
   llthread_(NULL),
   exitstate_(NULL)
{
}

Thread::~Thread()
{
   if (exitstate_) {
      delete exitstate_;
      exitstate_ = NULL;
   }
}

void *Thread::getData() const {
   MTLock lock_this_func;
   if (exitstate_)
      return exitstate_->user_data;
   return llthread_->user_data;
}

void Thread::setData(void *p) const {
   MTLock lock_this_func;
   if (exitstate_) {
      exitstate_->user_data = p;
   }
   else {
      llthread_->user_data = p;
   }
}

Process::const_ptr Thread::getProcess() const
{
   MTLock lock_this_func;
   if (!llthread_) {
      assert(exitstate_);
      return exitstate_->proc_ptr;
   }
   return llthread_->proc();
}

Process::ptr Thread::getProcess()
{
   MTLock lock_this_func;
   if (!llthread_) {
      assert(exitstate_);
      return exitstate_->proc_ptr;
   }
   return llthread_->proc();
}

int_thread *Thread::llthrd() const
{
   return llthread_;
}

bool Thread::isStopped() const
{
   MTLock lock_this_func;
   THREAD_EXIT_TEST("isStopped", false);
   return llthread_->getUserState().getState() == int_thread::stopped;
}

bool Thread::isRunning() const
{
   MTLock lock_this_func;
   THREAD_EXIT_TEST("isRunning", false);
   return llthread_->getUserState().getState() == int_thread::running;
}

bool Thread::isLive() const
{
   MTLock lock_this_func;
   if (!llthread_) {
      return false;
   }
   return (llthread_->getUserState().getState() == int_thread::stopped ||
           llthread_->getUserState().getState() == int_thread::running);
}

bool Thread::isDetached() const
{
    MTLock lock_this_func;
    THREAD_EXIT_TEST("isDetached", false);
    return llthread_->getUserState().getState() == int_thread::detached;
}

bool Thread::stopThread()
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   THREAD_EXIT_DETACH_CB_TEST("stopThread", false);

   int_thread *thrd = llthrd();
   int_process *proc = thrd->llproc();

   pthrd_printf("User stopping thread %d/%d\n", proc->getPid(), thrd->getLWP());
   bool result = thrd->getUserState().setState(int_thread::running);
   if (!result) {
      perr_printf("Thread %d/%d was not in a stoppable state, error return from setState\n",
                  proc->getPid(), thrd->getLWP());
      setLastError(err_internal, "Could not set user state while stopping thread\n");
      return false;
   }
   proc->throwNopEvent();

   bool proc_exited = false;
   result = int_process::waitAndHandleForProc(false, proc, proc_exited);
   if (proc_exited) {
      perr_printf("Process exited while waiting for user thread stop, erroring\n");
      setLastError(err_exited, "Process exited while thread being stopped.\n");
      return false;
   }
   if (!result) {
      perr_printf("Internal error calling waitAndHandleForProc on %d\n", proc->getPid());
      setLastError(err_internal, "Error while calling waitAndHandleForProc from thread stop\n");
      return false;
   }
   return true;
}

bool Thread::continueThread()
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   THREAD_EXIT_DETACH_CB_TEST("continueThread", false);

   int_thread *thrd = llthrd();
   int_process *proc = thrd->llproc();

   pthrd_printf("User continuing thread %d/%d\n", proc->getPid(), thrd->getLWP());
   bool result = thrd->getUserState().setState(int_thread::running);
   if (!result) {
      perr_printf("Thread %d/%d was not in a continuable state, error return from setState\n",
                  proc->getPid(), thrd->getLWP());
      setLastError(err_internal, "Could not set user state while continuing thread\n");
      return false;
   }
   proc->throwNopEvent();
   return true;
}

bool Thread::getAllRegisters(RegisterPool &pool) const
{
   MTLock lock_this_func;
   THREAD_EXIT_DETACH_STOP_TEST("getAllRegisters", false);

   allreg_response::ptr response = allreg_response::createAllRegResponse(pool.llregpool);
   bool result = llthread_->getAllRegisters(response);
   if (!result) {
      pthrd_printf("Error getting all registers\n");
      return false;
   }

   result = llthread_->llproc()->waitForAsyncEvent(response);
   if (!result) {
      pthrd_printf("Error waiting for async events\n");
      return false;
   }
   assert(response->isReady());
   if (response->hasError()) {
      pthrd_printf("Async error reading registers\n");
      return false;
   }
   return true;
}

bool Thread::setAllRegisters(RegisterPool &pool) const
{
   MTLock lock_this_func;
   THREAD_EXIT_DETACH_STOP_TEST("setAllRegisters", false);

   result_response::ptr response = result_response::createResultResponse();
   bool result = llthread_->setAllRegisters(*pool.llregpool, response);
   if (!result) {
      pthrd_printf("Error setting all registers\n");
      return false;
   }
   result = llthread_->llproc()->waitForAsyncEvent(response);
   if (!result) {
      pthrd_printf("Error waiting for async events\n");
      return false;
   }
   assert(response->isReady());
   if (response->hasError()) {
      pthrd_printf("Async error setting registers\n");
      return false;
   }

   return true;
}

bool Thread::getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val) const
{
   MTLock lock_this_func;
   THREAD_EXIT_DETACH_STOP_TEST("getRegister", false);

   reg_response::ptr response = reg_response::createRegResponse();
   bool result = llthread_->getRegister(reg, response);
   if (!result) {
      pthrd_printf("Error getting register\n");
      return false;
   }
   result = llthread_->llproc()->waitForAsyncEvent(response);
   if (!result) {
      pthrd_printf("Error waiting for async events\n");
      return false;
   }
   assert(response->isReady());
   if (response->hasError()) {
      pthrd_printf("Async error getting register\n");
      return false;
   }
   val = response->getResult();
   return true;
}

bool Thread::setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val) const
{
   MTLock lock_this_func;
   THREAD_EXIT_DETACH_STOP_TEST("setRegister", false);

   result_response::ptr response = result_response::createResultResponse();
   bool result = llthread_->setRegister(reg, val, response);
   if (!result) {
      pthrd_printf("Error setting register value\n");
      return false;
   }
   result = llthread_->llproc()->waitForAsyncEvent(response);
   if (!result) {
      pthrd_printf("Error waiting for async events\n");
      return false;
   }
   assert(response->isReady());
   if (response->hasError()) {
      pthrd_printf("Async error reading registers\n");
      return false;
   }
   return true;
}

bool Thread::getAllRegistersAsync(RegisterPool &pool, void *opaque_val) const
{
   MTLock lock_this_func;
   THREAD_EXIT_DETACH_STOP_TEST("getAllRegistersAsync", false);

   pthrd_printf("User wants to async read registers on %d/%d\n",
                llthread_->proc()->getPid(), llthread_->getLWP());

   allreg_response::ptr response = allreg_response::createAllRegResponse(pool.llregpool);
   int_eventAsyncIO *iev = new int_eventAsyncIO(response, int_eventAsyncIO::regallread);
   iev->opaque_value = opaque_val;
   iev->rpool = &pool;
   response->setAsyncIOEvent(iev);
   bool result = llthread_->getAllRegisters(response);
   if (!result) {
      pthrd_printf("Error getting all registers async\n");
      return false;
   }
   llthread_->llproc()->plat_preAsyncWait();
   return true;
}

bool Thread::setAllRegistersAsync(RegisterPool &pool, void *opaque_val) const
{
   MTLock lock_this_func;
   THREAD_EXIT_DETACH_STOP_TEST("getAllRegistersAsync", false);
   pthrd_printf("User wants to async set registers on %d/%d\n",
                llthread_->proc()->getPid(), llthread_->getLWP());

   result_response::ptr response = result_response::createResultResponse();
   int_eventAsyncIO *iev = new int_eventAsyncIO(response, int_eventAsyncIO::regallwrite);
   iev->opaque_value = opaque_val;
   response->setAsyncIOEvent(iev);
   bool result = llthread_->setAllRegisters(*pool.llregpool, response);
   if (!result) {
      pthrd_printf("Error setting all registers async\n");
      return false;
   }
   llthread_->llproc()->plat_preAsyncWait();
   return true;
}

bool Thread::readThreadLocalMemory(void *buffer, Library::const_ptr lib, Dyninst::Offset tls_symbol_offset, size_t size) const
{
   MTLock lock_this_func;
   THREAD_EXIT_DETACH_STOP_TEST("readTLSMemory", false);
   TRUTH_TEST(buffer, "buffer", false);
   TRUTH_TEST(lib, "lib", false);

   int_process *llproc = llthread_->llproc();
   int_thread *llthrd = llthread_;
   int_library *intlib = lib->debug();

   if (!intlib || !intlib->inProcess(llproc)) {
      perr_printf("Library %s is not loaded in process %d\n", lib->getName().c_str(), llproc->getPid());
      setLastError(err_badparam, "Library object is not loaded in specified process\n");
      return false;
   }

   pthrd_printf("User wants to read TLS memory on thread %d/%d from library %s at offset %lu of size %lu\n",
                llproc->getPid(), llthrd->getLWP(), lib->getName().c_str(),
                (unsigned long) tls_symbol_offset, (unsigned long) size);

   Address var_address;
   async_ret_t ret;
   do {
      set<response::ptr> resps;
      ret = llproc->plat_calcTLSAddress(llthrd, intlib, tls_symbol_offset,
                                        var_address, resps);
      if (ret == aret_error) {
         pthrd_printf("Failed calculate memory address of TLS variable");
         return false;
      }
      if (ret == aret_async) {
         llproc->waitForAsyncEvent(resps);
      }
   } while (ret != aret_success);


   mem_response::ptr memresp = mem_response::createMemResponse((char *) buffer, size);
   bool result = llproc->readMem(var_address, memresp, llthrd);
   if (result)
      llproc->waitForAsyncEvent(memresp);
   if (!result || memresp->hasError()) {
      pthrd_printf("Failed to read TLS memory at address %lx of size %lu\n", var_address, size);
      (void)memresp->isReady();
      return false;
   }

   return true;
}

bool Thread::writeThreadLocalMemory(Library::const_ptr lib, Dyninst::Offset tls_symbol_offset, const void *buffer, size_t size) const
{
   MTLock lock_this_func;
   THREAD_EXIT_DETACH_STOP_TEST("writeTLSMemory", false);
   TRUTH_TEST(buffer, "buffer", false);
   TRUTH_TEST(lib, "lib", false);

   int_process *llproc = llthread_->llproc();
   int_thread *llthrd = llthread_;
   int_library *intlib = lib->debug();

   if (!intlib || !intlib->inProcess(llproc)) {
      perr_printf("Library %s is not loaded in process %d\n", lib->getName().c_str(), llproc->getPid());
      setLastError(err_badparam, "Library object is not loaded in specified process\n");
      return false;
   }

   pthrd_printf("User wants to write to TLS memory on thread %d/%d in library %s at offset %lu of size %lu\n",
                llproc->getPid(), llthrd->getLWP(), lib->getName().c_str(),
                (unsigned long) tls_symbol_offset, (unsigned long) size);

   Address var_address;
   async_ret_t ret;
   do {
      set<response::ptr> resps;
      ret = llproc->plat_calcTLSAddress(llthrd, intlib, tls_symbol_offset,
                                        var_address, resps);
      if (ret == aret_error) {
         pthrd_printf("Failed calculate memory address of TLS variable");
         return false;
      }
      if (ret == aret_async) {
         llproc->waitForAsyncEvent(resps);
      }
   } while (ret != aret_success);

   result_response::ptr resp = result_response::createResultResponse();
   bool result = llproc->writeMem(buffer, var_address, size, resp);
   if (result)
      llproc->waitForAsyncEvent(resp);
   if (!result || !resp->getResult() || resp->hasError()) {
      pthrd_printf("Failed to write to TLS memory at address %lx of size %lu\n", var_address, size);
      return false;
   }

   return true;
}

bool Thread::getThreadLocalAddress(Library::const_ptr lib, Dyninst::Offset tls_symbol_offset, Dyninst::Address &result_addr) const
{
   MTLock lock_this_func;
   THREAD_EXIT_DETACH_STOP_TEST("getThreadLocalAddress", false);
   TRUTH_TEST(lib, "lib", false);

   int_process *llproc = llthread_->llproc();
   int_thread *llthrd = llthread_;
   int_library *intlib = lib->debug();

   if (!intlib || !intlib->inProcess(llproc)) {
      perr_printf("Library %s is not loaded in process %d\n", lib->getName().c_str(), llproc->getPid());
      setLastError(err_badparam, "Library object is not loaded in specified process\n");
      return false;
   }

   pthrd_printf("User wants to get TLS address on thread %d/%d in library %s at offset %lu\n",
                llproc->getPid(), llthrd->getLWP(), lib->getName().c_str(),
                (unsigned long) tls_symbol_offset);

   Address var_address;
   async_ret_t ret;
   do {
      set<response::ptr> resps;
      ret = llproc->plat_calcTLSAddress(llthrd, intlib, tls_symbol_offset,
                                        var_address, resps);
      if (ret == aret_error) {
         pthrd_printf("Failed calculate memory address of TLS variable");
         return false;
      }
      if (ret == aret_async) {
         llproc->waitForAsyncEvent(resps);
      }
   } while (ret != aret_success);
   result_addr = var_address;
   return true;
}

bool Thread::isInitialThread() const
{
   MTLock lock_this_func;
   THREAD_EXIT_TEST("isInitialThread", false);
   return llthread_->llproc()->threadPool()->initialThread() == llthread_;
}

bool Thread::isUser() const
{
	MTLock lock_this_func;
   THREAD_EXIT_TEST("isUser", false);
	return llthread_->isUser();
}

bool Thread::setSingleStepMode(bool s) const
{
   MTLock lock_this_func;
   THREAD_EXIT_DETACH_STOP_TEST("setSingleStepMode", false);
   llthread_->setSingleStepUserMode(s);
   return true;
}

bool Thread::getSingleStepMode() const
{
   MTLock lock_this_func;
   THREAD_EXIT_TEST("getSingleStepMode", false);
   return llthread_->singleStepUserMode();
}

bool Thread::setSyscallMode(bool s) const
{
    MTLock lock_this_func;
    THREAD_EXIT_DETACH_STOP_TEST("getSyscallMode", false);
    llthread_->setSyscallUserMode(s);
    return true;
}

bool Thread::getSyscallMode() const
{
   MTLock lock_this_func;
   if (!llthread_) {
      perr_printf("getSyscallMode called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }
   return llthread_->syscallUserMode();
}

Dyninst::LWP Thread::getLWP() const
{
   MTLock lock_this_func;
   if (!llthread_) {
      assert(exitstate_);
      return exitstate_->lwp;
   }
   return llthread_->getLWP();
}

bool Thread::postIRPC(IRPC::ptr irpc) const
{
   MTLock lock_this_func;
   THREAD_EXIT_DETACH_TEST("postIRPC", false);

   int_thread *thr = llthread_;
   int_process *proc = thr->llproc();
   int_iRPC::ptr rpc = irpc->llrpc()->rpc;
   bool result = rpcMgr()->postRPCToThread(thr, rpc);
   if (!result) {
      pthrd_printf("postRPCToThread failed on %d\n", proc->getPid());
      return false;
   }
   proc->throwNopEvent();
	return true;
}

bool Thread::getPostedIRPCs(std::vector<IRPC::ptr> &rpcs) const
{
   MTLock lock_this_func;
   THREAD_EXIT_TEST("getPostedIRPCs", false);

   rpc_list_t *rpc_list = llthread_->getPostedRPCs();
   for (rpc_list_t::iterator j = rpc_list->begin(); j != rpc_list->end(); ++j) {
      IRPC::ptr up_rpc = (*j)->getIRPC().lock();
      if (up_rpc == IRPC::ptr())
         continue;
      rpcs.push_back(up_rpc);
   }
   return true;
}

bool Thread::haveUserThreadInfo() const
{
   MTLock lock_this_func;
   THREAD_EXIT_TEST("haveUserThreadInfo", false);
   return llthread_->haveUserThreadInfo();
}

Dyninst::THR_ID Thread::getTID() const
{
   MTLock lock_this_func;
   if (!llthread_) {
      if (exitstate_ && exitstate_->thr_id != NULL_THR_ID) {
         return exitstate_->thr_id;
      }
      perr_printf("getTID on deleted thread\n");
      setLastError(err_exited, "Thread is exited");
      return false;
   }

   Dyninst::THR_ID tid;
   bool result = llthread_->getTID(tid);
   if (!result) {
      return NULL_THR_ID;
   }
   llthread_->setTID(tid);
   return tid;
}

Dyninst::Address Thread::getStartFunction() const
{
   MTLock lock_this_func;
   THREAD_EXIT_TEST("getStartFunction", 0);

   Dyninst::Address addr;
   bool result = llthread_->getStartFuncAddress(addr);
   if (!result) {
      return 0;
   }
   return addr;
}

Dyninst::Address Thread::getStackBase() const
{
   MTLock lock_this_func;
   THREAD_EXIT_TEST("getStackBase", 0);

   Dyninst::Address addr;
   bool result = llthread_->getStackBase(addr);
   if (!result) {
      return 0;
   }
   return addr;
}

unsigned long Thread::getStackSize() const
{
   MTLock lock_this_func;
   THREAD_EXIT_TEST("getStackSize", 0);

   unsigned long size;
   bool result = llthread_->getStackSize(size);
   if (!result) {
      return 0;
   }
   return size;
}

Dyninst::Address Thread::getTLS() const
{
   MTLock lock_this_func;
   THREAD_EXIT_TEST("getTLS", 0);

   Dyninst::Address addr;
   bool result = llthread_->getTLSPtr(addr);
   if (!result) {
      return 0;
   }
   return addr;
}

Dyninst::Address Thread::getThreadInfoBlockAddr() const
{
   MTLock lock_this_func;
   if (!llthread_) {
      perr_printf("getThreadInfoBlockAddr on deleted thread\n");
      setLastError(err_exited, "Thread is exited");
      return 0;
   }

   return llthread_->getThreadInfoBlockAddr();
}

IRPC::const_ptr Thread::getRunningIRPC() const
{
   MTLock lock_this_func;
   THREAD_EXIT_DETACH_TEST("getRunningIRPC", IRPC::const_ptr());

   int_iRPC::ptr running = llthread_->runningRPC();
   if (running == int_iRPC::ptr())
      return IRPC::const_ptr();
   IRPC::ptr irpc = running->getIRPC().lock();
   return irpc;
}

void Thread::setLastError(err_t ec, const char *es) const {
   if (!llthread_) {
      assert(exitstate_);
      exitstate_->proc_ptr->setLastError(ec, es);
   }
   else {
      llthread_->setLastError(ec, es);
   }
}

ThreadPool::ThreadPool() : threadpool(NULL)
{
}

ThreadPool::~ThreadPool()
{
}

Thread::ptr ThreadPool::getInitialThread()
{
	return threadpool->initialThread()->thread();
}

Thread::const_ptr ThreadPool::getInitialThread() const
{
	return threadpool->initialThread()->thread();
}

ThreadPool::iterator::iterator()
{
   curp = NULL;
   curi = uninitialized_val;
   curh = Thread::ptr();
}

bool ThreadPool::iterator::operator==(const iterator &i) const
{
   return (i.curh == curh);
}

bool ThreadPool::iterator::operator!=(const iterator &i) const
{
   return (i.curh != curh);
}

Thread::ptr ThreadPool::iterator::operator*() const
{
   MTLock lock_this_func;
   assert(curp);
   //assert(curi >= 0 && ((curi < (signed) curp->hl_threads.size()) || !curh->llthrd()) ); //Likely dereferenced bad thread iterator
   return curh;
}

ThreadPool::iterator ThreadPool::iterator::operator++() // prefix
{
   MTLock lock_this_func;

   assert(curi >= 0); //If this fails, you incremented a bad iterator
   for (;;) {
      curi++;
      if (curi >= (signed int) curp->hl_threads.size()) {
         curh = Thread::ptr();
         curi = end_val;
         return *this;
      }
      curh = curp->hl_threads[curi];
      if (!curh->llthrd())
         continue;
      if (curh->llthrd()->getUserState().getState() == int_thread::exited)
         continue;
	  if (!curh->isUser())
		  continue;
      return *this;
   }
}

ThreadPool::iterator ThreadPool::iterator::operator++(int) // postfix
{
   MTLock lock_this_func;
   ThreadPool::iterator orig = *this;

   assert(curi >= 0); //If this fails, you incremented a bad iterator
   for (;;) {
      curi++;
      if (curi >= (signed int) curp->hl_threads.size()) {
         curh = Thread::ptr();
         curi = end_val;
         return orig;
      }
      curh = curp->hl_threads[curi];
      if (!curh->llthrd())
         continue;
      if (curh->llthrd()->getUserState().getState() == int_thread::exited)
         continue;
      if (!curh->isUser())
         continue;
      return orig;
   }
}

ThreadPool::iterator ThreadPool::begin()
{
   MTLock lock_this_func;
   ThreadPool::iterator i{};
   i.curp = threadpool;
   i.curi = 0;

   if (!threadpool->hl_threads.empty())
      i.curh = i.curp->hl_threads[i.curi];
   else
      i.curh = Thread::ptr();

   return i;
}

ThreadPool::iterator ThreadPool::end()
{
   MTLock lock_this_func;
   ThreadPool::iterator i{};
   i.curp = threadpool;
   i.curi = iterator::end_val;
   i.curh = Thread::ptr();
   return i;
}

ThreadPool::iterator ThreadPool::find(Dyninst::LWP lwp)
{
    MTLock lock_this_func;
    ThreadPool::iterator i{};
    int_thread *thread = threadpool->findThreadByLWP(lwp);
    if( !thread ) return end();

    i.curp = threadpool;
    i.curh = thread->thread();
    i.curi = threadpool->hl_threads.size()-1;

    return i;
}

ThreadPool::const_iterator::const_iterator()
{
   curp = NULL;
   curi = uninitialized_val;
   curh = Thread::ptr();
}

bool ThreadPool::const_iterator::operator==(const const_iterator &i) const
{
   return (i.curh == curh);
}

bool ThreadPool::const_iterator::operator!=(const const_iterator &i) const
{
   return (i.curh != curh);
}

Thread::const_ptr ThreadPool::const_iterator::operator*() const
{
   MTLock lock_this_func;
   assert(curp);
   assert(curi >= 0 && curi < (signed) curp->hl_threads.size());
   return curh;
}

ThreadPool::const_iterator ThreadPool::const_iterator::operator++() // prefix
{
   MTLock lock_this_func;

   assert(curi >= 0); //If this fails, you incremented a bad iterator
   for (;;) {
      curi++;
      if (curi >= (signed int) curp->hl_threads.size()) {
         curh = Thread::ptr();
         curi = end_val;
         return *this;
      }
      curh = curp->hl_threads[curi];
      if (!curh->llthrd())
         continue;
      if (curh->llthrd()->getUserState().getState() == int_thread::exited)
         continue;
	  if (!curh->isUser())
		  continue;
      return *this;
   }
}

ThreadPool::const_iterator ThreadPool::const_iterator::operator++(int) // postfix
{
   MTLock lock_this_func;
   ThreadPool::const_iterator orig = *this;

   assert(curi >= 0); //If this fails, you incremented a bad iterator
   for (;;) {
      curi++;
      if (curi >= (signed int) curp->hl_threads.size()) {
         curh = Thread::ptr();
         curi = end_val;
         return orig;
      }
      curh = curp->hl_threads[curi];
      if (!curh->llthrd())
         continue;
      if (curh->llthrd()->getUserState().getState() == int_thread::exited)
         continue;
	  if (!curh->isUser())
		  continue;
      return orig;
   }
}

ThreadPool::const_iterator ThreadPool::begin() const
{
   MTLock lock_this_func;
   ThreadPool::const_iterator i{};
   i.curp = threadpool;
   i.curi = 0;

   if (!threadpool->hl_threads.empty()) {
	   while(i.curi < (int)threadpool->hl_threads.size() &&
		   i.curp->hl_threads[i.curi] &&
		   !i.curp->hl_threads[i.curi]->isUser()) {
			   i.curi++;
	   }
      i.curh = i.curp->hl_threads[i.curi];
   }
   else
      i.curh = Thread::ptr();

   return i;
}

ThreadPool::const_iterator ThreadPool::end() const
{
   MTLock lock_this_func;
   ThreadPool::const_iterator i{};
   i.curp = threadpool;
   i.curi = const_iterator::end_val;
   i.curh = Thread::ptr();
   return i;
}

ThreadPool::const_iterator ThreadPool::find(Dyninst::LWP lwp) const
{
    MTLock lock_this_func;
    ThreadPool::const_iterator i{};
    int_thread *thread = threadpool->findThreadByLWP(lwp);
    if( !thread ) return end();

    i.curp = threadpool;
    i.curh = thread->thread();
    i.curi = threadpool->hl_threads.size()-1;

    return i;
}

Process::const_ptr ThreadPool::getProcess() const
{
   MTLock lock_this_func;
   return threadpool->proc()->proc();
}

Process::ptr ThreadPool::getProcess()
{
   MTLock lock_this_func;
   return threadpool->proc()->proc();
}

size_t ThreadPool::size() const
{
   MTLock lock_this_func;
   return threadpool->size();
}

EventNotify::EventNotify() : llnotify(NULL)
{
}

EventNotify::~EventNotify()
{
}

int EventNotify::getFD()
{
   return (int)(llnotify->getWaitable());
}

void EventNotify::registerCB(notify_cb_t cb)
{
   return llnotify->registerCB(cb);
}

void EventNotify::removeCB(notify_cb_t cb)
{
   return llnotify->removeCB(cb);
}

EventNotify *Dyninst::ProcControlAPI::evNotify()
{
   return &notify()->up_notify;
}

Breakpoint::Breakpoint() : llbreakpoint_(NULL)
{
}

Breakpoint::~Breakpoint()
{
   delete llbreakpoint_;
   llbreakpoint_ = NULL;
}

int_breakpoint *Breakpoint::llbp() const
{
   return llbreakpoint_;
}

Breakpoint::ptr Breakpoint::newBreakpoint()
{
   Breakpoint::ptr newbp = Breakpoint::ptr(new Breakpoint());
   newbp->llbreakpoint_ = new int_breakpoint(newbp);
   return newbp;
}

Breakpoint::ptr Breakpoint::newTransferBreakpoint(Dyninst::Address to)
{
   Breakpoint::ptr newbp = Breakpoint::ptr(new Breakpoint());
   newbp->llbreakpoint_ = new int_breakpoint(to, newbp, false);
   return newbp;
}

Breakpoint::ptr Breakpoint::newTransferOffsetBreakpoint(signed long shift)
{
   Breakpoint::ptr newbp = Breakpoint::ptr(new Breakpoint());
   newbp->llbreakpoint_ = new int_breakpoint((Address) shift, newbp, true);
   return newbp;
}

Breakpoint::ptr Breakpoint::newHardwareBreakpoint(unsigned int mode,
                                                  unsigned int size)
{
  Breakpoint::ptr newbp = Breakpoint::ptr(new Breakpoint());
  newbp->llbreakpoint_ = new int_breakpoint(mode, size, newbp);
  return newbp;
}

void *Breakpoint::getData() const {
   return llbreakpoint_->getData();
}

void Breakpoint::setData(void *d) const {
   llbreakpoint_->setData(d);
}

bool Breakpoint::isCtrlTransfer() const {
   return llbreakpoint_->isCtrlTransfer();
}

Dyninst::Address Breakpoint::getToAddress() const
{
   return llbreakpoint_->toAddr();
}

void Breakpoint::setSuppressCallbacks(bool b)
{
   return llbreakpoint_->setSuppressCallbacks(b);
}

bool Breakpoint::suppressCallbacks() const
{
   return llbreakpoint_->suppressCallbacks();
}

// Note: These locks are intentionally indirect and leaked!
// This is because we can't guarantee destructor order between compilation
// units, and a static array of locks here in process.C may be destroyed before
// int_cleanup in generator.C.  Thus the handler thread may still be running,
// manipulating Counters, which throws an exception when it tries to acquire
// the destroyed lock.
Mutex<false> * Counter::locks = new Mutex<false>[Counter::NumCounterTypes];
int Counter::global_counts[Counter::NumCounterTypes];

Counter::Counter(CounterType ct_) :
   local_count(0),
   ct(ct_)
{
   assert((int) ct < NumCounterTypes);
}

Counter::~Counter()
{
   if (!local_count)
      return;

   adjust(-1 * local_count);
}

void Counter::adjust(int val)
{
   int index = (int) ct;
   int orig, after;
   locks[index].lock();
   orig = global_counts[index];
   global_counts[index] += val;
   after = global_counts[index];
   assert(global_counts[index] >= 0);
   locks[index].unlock();
   pthrd_printf("Adjusting counter %s by %d, before %d, after %d\n", getNameForCounter(index), val, orig, after);
   local_count += val;
}

void Counter::inc()
{
   adjust(1);
}

void Counter::dec()
{
   adjust(-1);
}

bool Counter::local() const
{
   return localCount() != 0;
}

int Counter::localCount() const
{
   return local_count;
}

bool Counter::global(CounterType ct)
{
   return globalCount(ct) != 0;
}

int Counter::processCount(CounterType ct, int_process* p)
{
	int sum = 0;
	switch(ct)
	{
		case AsyncEvents:
			sum += p->asyncEventCount().localCount();
			return sum;
			break;
		case ForceGeneratorBlock:
			sum += p->getForceGeneratorBlockCount().localCount();
			return sum;
			break;
		case StartupTeardownProcesses:
			sum += p->getStartupTeardownProcs().localCount();
			return sum;
			break;
		default:
			break;
	}
	for(int_threadPool::iterator i = p->threadPool()->begin();
		i != p->threadPool()->end();
		++i)
	{
		switch(ct)
		{
		case HandlerRunningThreads:
			sum += (*i)->handlerRunningThreadsCount().localCount();
			break;
		case GeneratorRunningThreads:
			sum += (*i)->generatorRunningThreadsCount().localCount();
			break;
		case SyncRPCs:
			sum += (*i)->syncRPCCount().localCount();
			break;
		case SyncRPCRunningThreads:
			sum += (*i)->runningSyncRPCThreadCount().localCount();
			break;
		case PendingStops:
			sum += (*i)->pendingStopsCount().localCount();
			break;
		case ClearingBPs:
			sum += (*i)->clearingBPCount().localCount();
			break;
		case ProcStopRPCs:
			sum += (*i)->procStopRPCCount().localCount();
			break;
		case GeneratorNonExitedThreads:
			sum += (*i)->getGeneratorNonExitedThreadCount().localCount();
			break;
      case NeonatalThreads:
         sum += (*i)->neonatalThreadCount().localCount();
         break;
      case PendingStackwalks:
         sum += (*i)->pendingStackwalkCount().localCount();
		default:
			break;
		}
	}
	return sum;
}


int Counter::globalCount(CounterType ct)
{
   //Currently no lock here.  We're just reading, and
   // I'm assuming the updates from 'adjust' will be
   // just modifying a single int, and thus not ever
   // leave this in an invalid state.  If the count
   // ever moved to a long, then we may have to lock.
   return global_counts[(int) ct];
}

#if !defined(STR_CASE)
#define STR_CASE(X) case X: return #X
#endif
const char *Counter::getNameForCounter(int counter_type)
{
   switch (counter_type) {
      STR_CASE(HandlerRunningThreads);
      STR_CASE(GeneratorRunningThreads);
      STR_CASE(SyncRPCs);
      STR_CASE(SyncRPCRunningThreads);
      STR_CASE(PendingStops);
      STR_CASE(ClearingBPs);
      STR_CASE(ProcStopRPCs);
      STR_CASE(AsyncEvents);
      STR_CASE(ForceGeneratorBlock);
      STR_CASE(GeneratorNonExitedThreads);
      STR_CASE(StartupTeardownProcesses);
      STR_CASE(NeonatalThreads);
      STR_CASE(PendingStackwalks);
      default: assert(0);
   }
   return NULL;
}

MTManager::MTManager() :
   have_queued_events(false),
   is_running(false),
   should_exit(false),
   threadMode(Process::NoThreads)
{
}

MTManager::~MTManager()
{
   if (is_running)
      stop();
}

MTManager *MTManager::mt_ = NULL;

bool MTManager::handlerThreading()
{
   return (threadMode == Process::HandlerThreading ||
           threadMode == Process::CallbackThreading);
}

Process::thread_mode_t MTManager::getThreadMode()
{
   return threadMode;
}

bool MTManager::setThreadMode(Process::thread_mode_t tm, bool init)
{
   if (ProcPool()->numProcs()) {
      perr_printf("Attemted to setThreadMode with running processes\n");
      ProcControlAPI::globalSetLastError(err_noproc, "Can't setThreadMode while processes are running\n");
      return false;
   }
   if (!init && tm == threadMode) {
      pthrd_printf("Not change in thread mode, leaving as is\n");
      return true;
   }

   switch (tm) {
      case Process::NoThreads:
         pthrd_printf("Setting thread mode to NoThreads\n");
         break;
      case Process::GeneratorThreading:
         pthrd_printf("Setting thread mode to GeneratorThreading\n");
         break;
      case Process::HandlerThreading:
         pthrd_printf("Setting thread mode to HandlerThreading\n");
         break;
      case Process::CallbackThreading:
         pthrd_printf("Setting thread mode to CallbackThreading\n");
         break;
      default:
         perr_printf("Bad value %d passed to setThreadMode\n", (int) tm);
         ProcControlAPI::globalSetLastError(err_badparam, "Invalid parameter to setThreadMode\n");
         return false;
   }

   if ((tm == Process::NoThreads || tm == Process::GeneratorThreading) &&
       (!init) &&
       (threadMode == Process::HandlerThreading || threadMode == Process::CallbackThreading))
   {
      pthrd_printf("New state does not use handler threading, stopping handler thread\n");
      stop();
   }
   if ((tm == Process::HandlerThreading || tm == Process::CallbackThreading) &&
       (init || (threadMode == Process::NoThreads || threadMode == Process::GeneratorThreading)))
   {
      pthrd_printf("New state uses handler threading, running handler thread\n");
      run();
   }

   if (tm == Process::GeneratorThreading) {
      pthrd_printf("Moving to generator threading, registering notification cb\n");
      Generator::registerNewEventCB(notifyNewEvent);
   }
   if (!init && threadMode == Process::GeneratorThreading) {
      pthrd_printf("Moving away from generator threading, removing notification cb\n");
      Generator::removeNewEventCB(notifyNewEvent);
   }
   threadMode = tm;
   return true;
}

void MTManager::run()
{
   if (is_running)
      return;
   is_running = true;
   should_exit = false;
   evhandler_thread.spawn(MTManager::evhandler_main_wrapper, NULL);
}

void MTManager::stop()
{
   if( !is_running ) return;
   if (isHandlerThread()) {
     should_exit = true;
     return;
   }

   pending_event_lock.lock();
   should_exit = true;
   pending_event_lock.signal();
   pending_event_lock.unlock();
   evhandler_thread.join();
   is_running = false;
   setHandlerThread(-1);
}

void MTManager::evhandler_main()
{
   have_queued_events = false;
   for (;;) {
      if (should_exit)
         return;

      startWork();
      pthrd_printf("Handler starting to check for events\n");
      if (mbox()->size() && !notify()->hasEvents()) {
         bool result = int_process::waitAndHandleEvents(false);
         if (!result) {
            pthrd_printf("Error handling events in handler thread\n");
         }
      }
      else
         pthrd_printf("Events already handled, going back to sleep\n");
      endWork();


      pending_event_lock.lock();
      if (should_exit) {
         pending_event_lock.unlock();
         return;
      }
      if (!have_queued_events) {
         pending_event_lock.wait();
      }
      have_queued_events = false;
      pending_event_lock.unlock();
   }
}

#if defined(os_windows)
unsigned long MTManager::evhandler_main_wrapper(void *)
#else
void MTManager::evhandler_main_wrapper(void *)
#endif
{
   setHandlerThread(DThread::self());
   mt()->evhandler_main();
#if defined(os_windows)
   return 0;
#else
   return;
#endif

}

void MTManager::eventqueue_cb_wrapper()
{
   mt()->eventqueue_cb();
}

void MTManager::eventqueue_cb()
{
   pending_event_lock.lock();
   have_queued_events = true;
   pending_event_lock.signal();
   pending_event_lock.unlock();
}

void MTManager::startWork()
{
   work_lock.lock();
}

void MTManager::endWork()
{
   assert(!isGeneratorThread());
   work_lock.unlock();
}

ProcStopEventManager::ProcStopEventManager(int_process *p) :
   proc(p)
{
}

ProcStopEventManager::~ProcStopEventManager()
{
}

bool ProcStopEventManager::prepEvent(Event::ptr ev)
{
   if (!ev->procStopper()) {
      //Most events are not procStoppers and should hit here
      return true;
   }

   pthrd_printf("Adding event %s on %d/%d to pending proc stopper list\n",
                ev->name().c_str(), ev->getProcess()->llproc()->getPid(),
                ev->getThread()->llthrd()->getLWP());
   pair<set<Event::ptr>::iterator, bool> result = held_pstop_events.insert(ev);
   assert(result.second);
   return false;
}

void ProcStopEventManager::checkEvents()
{
   for (set<Event::ptr>::iterator i = held_pstop_events.begin(); i != held_pstop_events.end();) {
      Event::ptr ev = *i;
      if (ev->procStopper()) {
         i++;
         continue;
      }

      pthrd_printf("ProcStop event %s on %d/%d is ready, adding to queue\n",
                ev->name().c_str(), ev->getProcess()->llproc()->getPid(),
                ev->getThread()->llthrd()->getLWP());

      i = held_pstop_events.erase(i);
      mbox()->enqueue(ev);
   }
}

bool ProcStopEventManager::processStoppedTo(int state_id)
{
   return proc->threadPool()->allStopped(state_id);
}

bool ProcStopEventManager::threadStoppedTo(int_thread *thr, int state_id)
{
   return thr->isStopped(state_id);
}

emulated_singlestep::emulated_singlestep(int_thread *thr_) :
   thr(thr_)
{
   bp = new int_breakpoint(Breakpoint::ptr());
   bp->setOneTimeBreakpoint(true);
   bp->setThreadSpecific(thr->thread());

   saved_user_single_step = thr->singleStepUserMode();
   saved_single_step = thr->singleStepMode();

   thr->setSingleStepMode(false);
   thr->setSingleStepUserMode(false);

   thr->addEmulatedSingleStep(this);
}

emulated_singlestep::~emulated_singlestep()
{
   delete bp;
   bp = NULL;
}

bool emulated_singlestep::containsBreakpoint(Address addr) const
{
   return addrs.find(addr) != addrs.end();
}

async_ret_t emulated_singlestep::add(Address addr) {
   if (addrs.find(addr) != addrs.end())
      return aret_success;

   int_process *proc = thr->llproc();
   proc->addBreakpoint(addr, bp);
   addrs.insert(addr);

   return aret_success;
}

async_ret_t emulated_singlestep::clear()
{
   int_process *proc = thr->llproc();
   if (clear_resps.empty())
   {
      for (set<Address>::iterator i = addrs.begin(); i != addrs.end(); i++) {
         result_response::ptr resp = result_response::createResultResponse();
         proc->removeBreakpoint(*i, bp, clear_resps);
      }
   }

   for (set<response::ptr>::iterator i = clear_resps.begin(); i != clear_resps.end();) {
      response::ptr resp = *i;
      if (resp->isReady()) {
         i = clear_resps.erase(i);
         continue;
      }
      i++;
   }

   return clear_resps.empty() ? aret_success : aret_async;
}

void emulated_singlestep::restoreSSMode()
{
   thr->setSingleStepMode(saved_single_step);
   thr->setSingleStepUserMode(saved_user_single_step);
}

void int_thread::terminate() {
	plat_terminate();
}

void int_thread::plat_terminate() {
	assert(0 && "Unimplemented!");
}

/*
 * non-thread specific solution to arm kernel bug.
 * not used.
syscall_exit_breakpoints::syscall_exit_breakpoints(){
    enabled = false;
    bp = NULL;
}

bool syscall_exit_breakpoints::contains_breakpoints(Address addr) const{
    return addrs.find(addr) != addrs.end();
}

bool syscall_exit_breakpoints::is_enabled() const{
    return enabled;
}
*/
