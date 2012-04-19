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

#include "proccontrol/src/int_process.h"
#include "proccontrol/src/irpc.h"
#include "proccontrol/src/procpool.h"
#include "proccontrol/src/int_handler.h"
#include "proccontrol/src/response.h"
#include "proccontrol/src/int_event.h"
#include "proccontrol/h/Mailbox.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Handler.h"
#include "proccontrol/src/GeneratorWindows.h"

#if defined(os_windows)
#include "proccontrol/src/windows_process.h"
#include "proccontrol/src/windows_thread.h"
#endif

#include <cstring>
#include <cassert>
#include <map>
#include <sstream>
#include <iostream>

#if defined(os_windows)
#pragma warning(disable:4355)
#endif

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

const map<int,int> Process::emptyFDs;
const vector<string> Process::emptyEnvp;
Process::thread_mode_t threadingMode = Process::GeneratorThreading;
bool int_process::in_callback = false;
std::set<int_thread::continue_cb_t> int_thread::continue_cbs;

static const int ProcControl_major_version = 0;
static const int ProcControl_minor_version = 1;
static const int ProcControl_maintenance_version = 0;

void Process::version(int& major, int& minor, int& maintenance)
{
    major = ProcControl_major_version;
    minor = ProcControl_minor_version;
    maintenance = ProcControl_maintenance_version;
}


bool int_process::create()
{
   //Should be called with procpool lock held
	// plat_create is allowed to, but not required to, create an initial thread
	// if the OS will cooperate
   bool result = plat_create();
   if (!result) {
      pthrd_printf("Could not create debuggee, %s\n", executable.c_str());
	  setLastError(err_noproc, "Could not create process");
      ProcPool()->condvar()->unlock();
      return false;
   }

   // Windows gives us (some) initial thread info when we create the process.
   // So don't double-create the initial thread, and don't create it with a junk
   // TID/LWP if we had a legitimate one available.
   if(threadPool()->empty())
   {
	   int_thread *initial_thread;
	   initial_thread = int_thread::createThread(this, NULL_THR_ID, NULL_LWP, true);
   }

   ProcPool()->addProcess(this);
   setState(neonatal_intermediate);

   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();

   pthrd_printf("Created debugged %s on pid %d\n", executable.c_str(), pid);
   result = waitfor_startup();
   if (getState() == int_process::exited) {
      pthrd_printf("Process %s exited during create\n", executable.c_str());
      return false;
   }
   if (!result) {
      pthrd_printf("Error during process create for %d\n", pid);
      return false;
   }
   result = post_create();
   if (!result) {
      pthrd_printf("Error during post create for %d\n", pid);
      return false;
   }
  
   assert(getState() == running);
   return true;
}

bool int_process::waitfor_startup()
{
   bool proc_exited;
   for (;;) {
      bool result;
      
      pthrd_printf("Waiting for startup to complete for %d\n", pid);
      result = waitAndHandleForProc(true, this, proc_exited);
      if (proc_exited || getState() == exited) {
         pthrd_printf("Error.  Proces exited during create/attach\n");
         return false;
      }
      if (!result || getState() == errorstate) {
         pthrd_printf("Error.  Process %d errored during create/attach\n", pid);
         return false;
      }
      if (getState() == running) {
         pthrd_printf("Successfully completed create/attach on %d\n", pid);
         return true;
      }
   }
}

bool int_process::attachThreads()
{
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
   bool found_new_threads;
   do {
      found_new_threads = false;
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
         thr = int_thread::createThread(this, NULL_THR_ID, *i, false);
         found_new_threads = true;         
      }
   } while (found_new_threads);

   return true;
}

bool int_process::attach()
{
   //Should be called with procpool lock held

   // Determine the running state of all threads before attaching
   map<Dyninst::LWP, bool> runningStates;
   if( !plat_getOSRunningStates(runningStates) ) {
       ProcPool()->condvar()->broadcast();
       ProcPool()->condvar()->unlock();
       return false;
   }

   bool allStopped = true;
   for(map<Dyninst::LWP, bool>::iterator i = runningStates.begin();
           i != runningStates.end(); ++i)
   {
       if( i->second ) {
           allStopped = false;
           break;
       }
   }

   pthrd_printf("Attaching to process %d\n", pid);
   bool result = plat_attach(allStopped);
   if (!result) {
      ProcPool()->condvar()->broadcast();
      ProcPool()->condvar()->unlock();
      pthrd_printf("Could not attach to debuggee, %d\n", pid);
	  setLastError(err_noproc, "Could not attach to process");
      return false;
   }

   int_thread *initial_thread;
   initial_thread = int_thread::createThread(this, NULL_THR_ID, NULL_LWP, true);

   ProcPool()->addProcess(this);

   setState(neonatal_intermediate);

   result = attachThreads();
   if (!result) {
      pthrd_printf("Failed to attach to threads in %d--will try again\n", pid);
   }

   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();

   // Now that all the threads are created, set their running states
   for(int_threadPool::iterator i = threadPool()->begin();
           i != threadPool()->end(); ++i)
   {
       map<Dyninst::LWP, bool>::iterator findIter = runningStates.find((*i)->getLWP());

       // There is a race that could be visible here where we are not
       // guaranteed to determine the running state of all threads in a process
       // before we attach -- if for some reason we don't know the running
       // state, assume it was running
       if( findIter == runningStates.end() ) {
           (*i)->setRunningWhenAttached(true);
       }else{
           (*i)->setRunningWhenAttached(findIter->second);
       }
   }
      
   pthrd_printf("Wait for attach from process %d\n", pid);
   result = waitfor_startup();
   if (!result) {
      pthrd_printf("Error waiting for attach to %d\n", pid);
      setLastError(err_internal, "Process failed to startup");
      goto error;
   }

   result = attachThreads();
   if (!result) {
      pthrd_printf("Failed to attach to threads in %d--now an error\n", pid);
      setLastError(err_internal, "Could not get threads during attach\n");
      goto error;
   }


   result = post_attach(false);
   if (!result) {
      pthrd_printf("Error on post attach for %d\n", pid);
      setLastError(err_internal, "Process failed post-startup");
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

bool int_process::reattach()
{
   vector<Event::ptr> observedEvents;
   std::set<response::ptr> async_responses;

   ProcPool()->condvar()->lock();

   // Determine the running state of all threads before attaching
   map<Dyninst::LWP, bool> runningStates;
   if( !plat_getOSRunningStates(runningStates) ) {
       ProcPool()->condvar()->broadcast();
       ProcPool()->condvar()->unlock();
       return false;
   }

   bool allStopped = true;
   for(map<Dyninst::LWP, bool>::iterator i = runningStates.begin();
           i != runningStates.end(); ++i)
   {
       if( i->second ) {
           allStopped = false;
           break;
       }
   }

   pthrd_printf("Re-attaching to process %d\n", pid);
   bool result = plat_attach(allStopped);
   if (!result) {
      ProcPool()->condvar()->broadcast();
      ProcPool()->condvar()->unlock();
      pthrd_printf("Could not attach to debuggee, %d\n", pid);
      return false;
   }

   result = attachThreads();
   if (!result) {
      pthrd_printf("Failed to re-attach to threads in %d\n", pid);
      setLastError(err_internal, "Could not re-attach to process' threads");
      goto error;
   }

   // To reuse existing bootstrap code this needs to be set
   setState(neonatal_intermediate);

   // Now, go back and set non-existing threads to detached to exclude them
   // from the bootstrap handling, creating thread destruction events for
   // these non-existing threads
   //
   // Also, at the same time issue attaches to existing threads

   threadPool()->initialThread()->getDetachState().restoreStateProc();
   for(int_threadPool::iterator i = threadPool()->begin();
           i != threadPool()->end(); ++i)
   {
       map<Dyninst::LWP, bool>::iterator findIter = runningStates.find((*i)->getLWP());
       if( findIter == runningStates.end() ) {
           pthrd_printf("Creating thread destroy event for thread %d/%d\n", pid,
                   (*i)->getLWP());
           (*i)->getGeneratorState().setState(int_thread::detached);
           (*i)->getHandlerState().setState(int_thread::detached);
           (*i)->getUserState().setState(int_thread::detached);
           (*i)->getDetachState().setState(int_thread::detached);

           Event::ptr destroyEv;
           if (plat_supportLWPPostDestroy()) {
              destroyEv = Event::ptr(new EventLWPDestroy(EventType::Post));
           }
           else if (plat_supportThreadEvents()) {
              destroyEv = Event::ptr(new EventUserThreadDestroy(EventType::Post));
           }
           else {
              perr_printf("Platform does not support any thread destroy events.  Now what?\n");
              assert(0);
           }

           destroyEv->setProcess(proc());
           destroyEv->setThread((*i)->thread());
           destroyEv->setSyncType(Event::async);
           destroyEv->setUserEvent(true);
           observedEvents.push_back(destroyEv);
       }else{
           pthrd_printf("Re-attaching to thread %d/%d\n", pid, (*i)->getLWP());
           if( !(*i)->attach() ) {
               perr_printf("Failed to re-attach to thread %d/%d\n", pid, (*i)->getLWP());
               setLastError(err_internal, "Could not re-attach to thread\n");
               goto error;
           }
       }
   }

   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();

   pthrd_printf("Wait for re-attach from process %d\n", pid);
   result = waitfor_startup();
   if (!result) {
      pthrd_printf("Error waiting for re-attach to %d\n", pid);
      setLastError(err_internal, "Process failed to re-attach");
      goto error;
   }

   result = post_attach(true);
   if (!result) {
      pthrd_printf("Error on post re-attach for %d\n", pid);
      setLastError(err_internal, "Process failed post re-attach");
      goto error;
   }

   // Resume all breakpoints
   for(std::map<Dyninst::Address, installed_breakpoint *>::iterator i = mem->breakpoints.begin();
           i != mem->breakpoints.end(); ++i)
   {
       pthrd_printf("Resuming breakpoint at 0x%lx in process %d\n",
               i->first, pid);
       result_response::ptr resp = result_response::createResultResponse();
       bool result = i->second->resume(this, resp);
       if(!result) {
          perr_printf("Error resuming breakpoint at %lx\n", i->first);
          setLastError(err_internal, "Error resuming breakpoint before detach\n");
          goto error;
       }
       async_responses.insert(resp);
   }

   waitForAsyncEvent(async_responses);
   for (set<response::ptr>::iterator i = async_responses.begin(); i != async_responses.end(); ++i) {
      if ((*i)->hasError()) {
         perr_printf("Failed to resuming breakpoints\n");
         setLastError(err_internal, "Error resuming breakpoint before detach\n");
         goto error;
      }
   }
   async_responses.clear();

   // Report all events for observed process state changes
   for(vector<Event::ptr>::iterator i = observedEvents.begin();
           i != observedEvents.end(); ++i)
   {
       int_thread *thrd = (*i)->getThread()->llthrd();

       pthrd_printf("Queuing event %s for thread %d/%d\n",
               (*i)->getEventType().name().c_str(), pid, 
               (*i)->getThread()->getLWP());

       // Make sure the thread is the correct state while in event handling
       thrd->getGeneratorState().setState(int_thread::detached);
       thrd->getHandlerState().setState(int_thread::detached);
       thrd->getUserState().setState(int_thread::detached);
       thrd->getDetachState().setState(int_thread::detached);

	   mbox()->enqueue(*i);
   }

   if( !observedEvents.empty() ) {
      // As a sanity check, don't block
      bool proc_exited;
      bool result = waitAndHandleForProc(false, this, proc_exited);
      if (proc_exited || getState() == exited) {
         pthrd_printf("Error.  Proces exited during re-attach\n");
         goto error;
      }
      if (!result || getState() == errorstate) {
         pthrd_printf("Error.  Process %d errored re-attach\n", pid);
         goto error;
      }
   }

   return true;

  error:
   if (getState() == exited) {
      setLastError(err_exited, "Process exited unexpectedly during re-attach\n");
      return false;
   }

   pthrd_printf("Error during process re-attach for %d\n", pid);
   return false;
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

   int_thread *initial_thread = int_thread::createThread(this, NULL_THR_ID, NULL_LWP, true);
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
   initial_thread = int_thread::createThread(this, NULL_THR_ID, NULL_LWP, true);

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

bool int_process::initializeAddressSpace()
{
   bool result = initLibraryMechanism();
   if (!result) {
      pthrd_printf("Error initializing library mechanism\n");
      return false;
   }

   std::set<int_library*> added, rmd;
   for (;;) {
      std::set<response::ptr> async_responses;
      bool have_asyncs = false;
      result = refresh_libraries(added, rmd, have_asyncs, async_responses);
      if (!result && have_asyncs) {
         result = waitForAsyncEvent(async_responses);
         if (!result) {
            pthrd_printf("Failure waiting for async completion\n");
            return false;
         }
         continue;
      }
      if (!result) {
         pthrd_printf("Failure refreshing libraries for %d\n", getPid());
         return false;
      }
      pthrd_printf("Successfully initialized address space for %d\n", getPid());
      return true;
   }
}

bool int_process::post_attach(bool)
{
   return initializeAddressSpace();
}

bool int_process::post_create()
{
   return initializeAddressSpace();
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
		  if((*i)->getPendingStopState().getState() != int_thread::dontcare)
			  (*i)->getPendingStopState().restoreState();
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
   pthrd_printf("Setting int_process %p to pid %d\n", this, p);
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
      pthrd_printf("Current Threading State for %d:\n", getPid());
      for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
         int_thread *thr = *i;
         int_thread::StateTracker &target = thr->getActiveState();
         
         char state_code[int_thread::NumStateIDs+1];
         for (int i = 0; i < int_thread::NumStateIDs; i++) {
            state_code[i] = int_thread::stateLetter(thr->getStateByID(i).getState());
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
   }

   pthrd_printf("Running plat_syncRunState on %d\n", getPid());
   bool result = plat_syncRunState();

   return result;
}

bool int_process::waitForAsyncEvent(response::ptr resp)
{
   return getResponses().waitFor(resp);
}

bool int_process::waitForAsyncEvent(std::set<response::ptr> resp)
{
   bool has_error = false;
   for (set<response::ptr>::iterator i = resp.begin(); i != resp.end(); ++i) {
      bool result = waitForAsyncEvent(*i);
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

int_process *int_process::in_waitHandleProc = NULL;
bool int_process::waitAndHandleForProc(bool block, int_process *proc, bool &proc_exited)
{
   assert(in_waitHandleProc == NULL);
   in_waitHandleProc = proc;

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
#define UNSET_CHECK        -8
#define printCheck(VAL)    (((int) VAL) == UNSET_CHECK ? '?' : (VAL ? 'T' : 'F'))

bool int_process::waitAndHandleEvents(bool block)
{
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
      int hasStartupTeardownProc = UNSET_CHECK;

      bool should_block = (!checkHandlerThread && 
                           ((checkBlock && !checkGotEvent && checkRunningThread) ||
                            (checkSyncRPCRunningThrd) ||
                            (checkStopPending) ||
                            (checkClearingBP) ||
                            (checkProcStopRPC) ||
                            (checkAsyncPending) ||
                            (checkStartupTeardownProcs)
                           )
                          );
      //Entry for this print match the above tests in order and one-for-one.
      pthrd_printf("%s for events = !%c && ((%c && !%c && %c) || %c || %c || %c || %c || %c || %c)\n",
                   should_block ? "Blocking" : "Polling",
                   printCheck(hasHandlerThread),
                   printCheck(hasBlock), printCheck(hasGotEvent), printCheck(hasRunningThread), 
                   printCheck(hasSyncRPCRunningThrd), 
                   printCheck(hasStopPending),
                   printCheck(hasClearingBP), 
                   printCheck(hasProcStopRPC),
                   printCheck(hasAsyncPending),
                   printCheck(hasStartupTeardownProc));

      //TODO: If/When we move to per-process locks, then we'll need a smarter should_block check
      //      We don't want the should_block changing between the above measurement
      //      and the below dequeue.  Perhaps dequeue should alway poll, and the user thread loops
      //      over it while (should_block == true), with a condition variable signaling when the 
      //      should_block would go to false (so we don't just spin).
         
      /**
       * Check for new events
       **/
      Event::ptr ev = mbox()->dequeue(should_block);

      if (ev == Event::ptr())
      {
         if (gotEvent) {
            pthrd_printf("Returning after handling events\n");
            goto done;
         }
         if (should_block) {
            perr_printf("Blocking wait failed to get events\n");
            setLastError(err_internal, "Blocking wait returned without events");
            error = true;
            goto done;
         }
         if (hasHandlerThread) {
            pthrd_printf("Handler thread found nothing to do\n");
            goto done;
         }
         setLastError(err_noevents, "Poll failed to find events");
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
	  int_process* llp = ev->getProcess()->llproc();
	  if(!llp) {
		  error = true;
		  goto done;
	  }

      Process::const_ptr proc = ev->getProcess();
      int_process *llproc = proc->llproc();
      HandlerPool *hpool = llproc->handlerpool;
      
      if (!ev->handling_started) {
         llproc->updateSyncState(ev, false);
         llproc->noteNewDequeuedEvent(ev);
         ev->handling_started = true;
      }

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
   pthrd_printf("Leaving WaitAndHandleEvents 'cause we're done\n");
   recurse = false;
   return !error;
}

bool int_process::detach(int_process *proc, bool temporary)
{
   pthrd_printf("%s detaching from process %d\n", temporary ? "Temporary" : "Permanent", proc->getPid());
   EventDetach::ptr detach_ev = EventDetach::ptr(new EventDetach());
   detach_ev->getInternal()->temporary_detach = temporary;
   detach_ev->setProcess(proc->proc());
   detach_ev->setThread(proc->threadPool()->initialThread()->thread());
   detach_ev->setSyncType(Event::async);

   proc->getStartupTeardownProcs().inc();
   proc->threadPool()->initialThread()->getDetachState().desyncStateProc(int_thread::stopped);

   mbox()->enqueue(detach_ev);
   
   bool exited = false;
   do {
      bool result = waitAndHandleForProc(true, proc, exited);
      if (!result) {
         perr_printf("Error during waitAndHandleForProc while detaching from %d\n", proc->getPid());
         return false;
      }
   } while (!exited && !detach_ev->getInternal()->done);

   if (exited && temporary) {
      perr_printf("Process exited during temporary detach\n");
      setLastError(err_exited, "Process exited during temporary detach");
      return false;
   }

   return true;
}

bool int_process::terminate(bool &needs_sync)
{
   pthrd_printf("Terminate requested on process %d\n", getPid());

   if (!preTerminate()) {
       perr_printf("pre-terminate hook failed\n");
       setLastError(err_internal, "Pre-terminate hook failed\n");
       return false;
   }

   bool had_error = true;
   getStartupTeardownProcs().inc();
   ProcPool()->condvar()->lock();
   bool result = plat_terminate(needs_sync);
   if (!result) {
      pthrd_printf("plat_terminate failed on %d\n", getPid());
      goto done;
   }
   forcedTermination = true;
   setForceGeneratorBlock(true);
   had_error = false;
  done:
   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();
   return !had_error;
}

bool int_process::preTerminate() {
    return true;
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
   proc_stop_manager(this)
{
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
   exec_mem_cache(exec_mem_cache),
   continueSig(p->continueSig),
   mem_cache(this),
   async_event_count(Counter::AsyncEvents),
   force_generator_block_count(Counter::ForceGeneratorBlock),
   startupteardown_procs(Counter::StartupTeardownProcesses),
   proc_stop_manager(this)
{
   Process::ptr hlproc = Process::ptr(new Process());
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
   if (!thr && plat_needsThreadForMemOps())
   {
      thr = findStoppedThread();
      if (!thr) {
         setLastError(err_notstopped, "A thread must be stopped to read from memory");
         perr_printf("Unable to find a stopped thread for read in process %d\n", getPid());
         return false;
      }
   }

   bool bresult;
   if (!plat_needsAsyncIO()) {
      pthrd_printf("Reading from remote memory %lx to %p, size = %lu on %d/%d\n",
                   remote, result->getBuffer(), (unsigned long) result->getSize(),
				   getPid(), thr ? thr->getLWP() : (Dyninst::LWP)(-1));

      bresult = plat_readMem(thr, result->getBuffer(), remote, result->getSize());
	  std::stringstream s;
	  s << "\t";
	  for(unsigned long byte = 0; byte < result->getSize(); byte++)
	  {
		  s << std::hex << "0x" << (int)(result->getBuffer()[byte]);
	  }
	  s << std::endl;
	  pthrd_printf("%s\n", s.str().c_str());
      if (!bresult) {
         result->markError();
      }
      result->setResponse();
   }
   else {
      pthrd_printf("Async read from remote memory %lx to %p, size = %lu on %d/%d\n",
                   remote, result->getBuffer(), (unsigned long) result->getSize(), 
                   getPid(), thr->getLWP());

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

bool int_process::writeMem(const void *local, Dyninst::Address remote, size_t size, result_response::ptr result, int_thread *thr)
{
   if (!thr && plat_needsThreadForMemOps()) 
   {
      thr = findStoppedThread();
      if (!thr) {
         setLastError(err_notstopped, "A thread must be stopped to write to memory");
         perr_printf("Unable to find a stopped thread for write in process %d\n", getPid());
         return false;
      }
   }

   bool bresult;
   if (!plat_needsAsyncIO()) {
      pthrd_printf("Writing to remote memory %lx from %p, size = %lu on %d/%d\n",
                   remote, local, (unsigned long) size,
				   getPid(), thr ? thr->getLWP() : (Dyninst::LWP)(-1));
	  std::stringstream s;
	  s << "\t";
	  for(unsigned long byte = 0; byte < size; byte++)
	  {
		  s << std::hex << "0x" << (int)(((char*)(local))[byte]);
	  }
	  s << std::endl;
	  pthrd_printf("%s", s.str().c_str());

      bresult = plat_writeMem(thr, local, remote, size);
      if (!bresult) {
         result->markError();
      }
      result->setResponse(bresult);
   }
   else {
      pthrd_printf("Async writing to remote memory %lx from %p, size = %lu on %d/%d\n",
                   remote, local, (unsigned long) size,
                   getPid(), thr->getLWP());

      getResponses().lock();
      bresult = plat_writeMemAsync(thr, local, remote, size, result);
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

SymbolReaderFactory *int_process::plat_defaultSymReader()
{
  return NULL;
}

Dyninst::Address int_process::infMalloc(unsigned long size, bool use_addr, Dyninst::Address addr)
{
   bool proc_exited = false;

   pthrd_printf("Process %d is allocating memory of size %lu at 0x%lx\n", getPid(), size, addr);
   int_iRPC::ptr rpc = rpcMgr()->createInfMallocRPC(this, size, use_addr, addr);
   assert(rpc);
   bool result = rpcMgr()->postRPCToProc(this, rpc);
   if (!result) {
      pthrd_printf("Error posting RPC to thread");
      return 0;
   }

   int_thread *thr = rpc->thread();
   assert(thr);
   thr->getInternalState().desyncState(int_thread::running);
   rpc->setRestoreInternal(true);

   throwNopEvent();
   result = waitAndHandleForProc(false, this, proc_exited);
   if (proc_exited) {
      perr_printf("Process exited during infMalloc\n");
      setLastError(err_exited, "Process exited during infMalloc\n");
      return 0;
   }
   if (!result) {
      pthrd_printf("Error in waitAndHandleEvents\n");
      return 0;
   }
   assert(rpc->getState() == int_iRPC::Finished);

   Dyninst::Address aresult = rpc->infMallocResult();
   pthrd_printf("Inferior malloc returning %lx\n", aresult);
   mem->inf_malloced_memory[aresult] = size;
   return aresult;
}

bool int_process::infFree(Dyninst::Address addr)
{
   bool proc_exited = false;

   std::map<Dyninst::Address, unsigned long>::iterator i = mem->inf_malloced_memory.find(addr);
   if (i == mem->inf_malloced_memory.end()) {
      setLastError(err_badparam, "Unknown address passed to freeMemory");
      perr_printf("Passed bad address, %lx, to infFree\n", addr);
      return false;
   }
   unsigned long size = i->second;

   int_iRPC::ptr rpc = rpcMgr()->createInfFreeRPC(this, size, addr);
   assert(rpc);
   pthrd_printf("Process %d is freeing memory of size %lu at 0x%lx with rpc %lu\n", getPid(), size, addr,
                rpc->id());
   rpcMgr()->postRPCToProc(this, rpc);

   int_thread *thr = rpc->thread();
   assert(thr);
   thr->getInternalState().desyncState(int_thread::running);
   rpc->setRestoreInternal(true);

   throwNopEvent();
   bool result = waitAndHandleForProc(false, this, proc_exited);
   if (proc_exited) {
      perr_printf("Process exited during infFree\n");
      setLastError(err_exited, "Process exited during infFree\n");
      return 0;
   }
   if (!result) {
      pthrd_printf("Error in waitAndHandleEvents\n");
      return 0;
   }
   assert(rpc->getState() == int_iRPC::Finished);

   pthrd_printf("Inferior free returning successfully\n");
   mem->inf_malloced_memory.erase(i);
   return true;
}

void int_process::setForceGeneratorBlock(bool b)
{
   if (b)
      force_generator_block_count.inc();
   else
      force_generator_block_count.dec();
}

int int_process::getAddressWidth()
{
   switch (getTargetArch()) {
      case Arch_x86:
      case Arch_ppc32:
         return 4;
      case Arch_x86_64:
      case Arch_ppc64:
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

bool int_process::addBreakpoint(Dyninst::Address addr, int_breakpoint *bp)
{
   if (getState() != running) {
      perr_printf("Attempted to add breakpoint at %lx to stopped process %d\n", addr, getPid());
      setLastError(err_exited, "Attempted to insert breakpoint into exited process\n");
      return false;
   }

   pthrd_printf("Installing new breakpoint at %lx into %d\n", addr, getPid());
   installed_breakpoint *ibp = NULL;
   map<Address, installed_breakpoint *>::iterator i = mem->breakpoints.find(addr);
   if (i == mem->breakpoints.end()) {
      pthrd_printf("Adding new breakpoint to %d\n", getPid());
      ibp = new installed_breakpoint(mem, addr);

      mem_response::ptr mem_resp = mem_response::createMemResponse();
      mem_resp->markSyncHandled();
      bool result = ibp->prepBreakpoint(this, mem_resp);
      if (!result) {
         pthrd_printf("Failed to prep breakpoint\n");
         delete ibp;
         return false;
      }

      result = waitForAsyncEvent(mem_resp);
      if (!result || mem_resp->hasError()) {
         pthrd_printf("Error prepping breakpoint\n");
         delete ibp;
         return false;
      }

      result_response::ptr res_resp = result_response::createResultResponse();
      res_resp->markSyncHandled();
      result = ibp->insertBreakpoint(this, res_resp);
      if (!result) {
         pthrd_printf("Error writing new breakpoint\n");
         delete ibp;
         return false;
      }

      result = waitForAsyncEvent(res_resp);
      if (!result || res_resp->hasError()) {
         pthrd_printf("Error writing new breakpoint\n");
         delete ibp;
         return false;
      }

      ibp->addBreakpoint(bp);
      if (!result) {
         pthrd_printf("Failed to install new breakpoint\n");
         delete ibp;
         return false;
      }
      return true;
   }
   ibp = i->second;
   assert(ibp && ibp->isInstalled());
   bool result = ibp->addBreakpoint(bp);
   if (!result) {
      pthrd_printf("Failed to install new breakpoint\n");
      return false;
   }

   return true;
}

bool int_process::rmBreakpoint(Dyninst::Address addr, int_breakpoint *bp, result_response::ptr async_resp)
{
   map<Address, installed_breakpoint *>::iterator i = mem->breakpoints.find(addr);
   if (i == mem->breakpoints.end()) {
      perr_printf("Attempted to removed breakpoint that isn't installed\n");
      return false;
   }
   installed_breakpoint *ibp = i->second;
   assert(ibp && ibp->isInstalled());

   bool empty;
   bool result = ibp->rmBreakpoint(this, bp, empty, async_resp);
   if (!result) {
      pthrd_printf("rmBreakpoint failed on breakpoint at %lx in %d\n", addr, getPid());
      return false;
   }
   if (empty) {
      delete ibp;
   }

   return true;
}

installed_breakpoint *int_process::getBreakpoint(Dyninst::Address addr)
{
   std::map<Dyninst::Address, installed_breakpoint *>::iterator  i = mem->breakpoints.find(addr);
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
                                     size_t, result_response::ptr )
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
   EventType etype = ev->getEventType();
   switch (ev->getSyncType()) {
	  case Event::async: {
         break;
	 }
      case Event::sync_thread: {
         int_thread *thrd = ev->getThread()->llthrd();
         int_thread::StateTracker &st = gen ? thrd->getGeneratorState() : thrd->getHandlerState();
         if (!thrd) {
            pthrd_printf("No thread for sync thread event, assuming thread exited\n");
            return;
         }
         int_thread::State old_state = st.getState();
         if (old_state == int_thread::exited) {
            //Silly, linux.  Giving us events on processes that have exited.
            pthrd_printf("Recieved events for exited thread, not changing thread state\n");
            break;
         }
         pthrd_printf("Event %s is thread synchronous, marking thread %d %s stopped\n", 
                      etype.name().c_str(), thrd->getLWP(), gen ? "generator" : "handler");
         assert(RUNNING_STATE(old_state) || 
                thrd->llproc()->wasForcedTerminated() ||
                (old_state == int_thread::stopped && (thrd->isExiting() || thrd->isExitingInGenerator())));
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

int_process::~int_process()
{
   pthrd_printf("Deleting int_process at %p\n", this);
   if (up_proc != Process::ptr())
   {
      proc_exitstate *exitstate = new proc_exitstate();
      exitstate->pid = pid;
      exitstate->exited = hasExitCode && !forcedTermination;
      exitstate->exit_code = exitCode;
      exitstate->crashed = hasCrashSignal;
      exitstate->crash_signal = crashSignal;
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
            
      if (handler_state == target_state) {
         continue;
      }
      else if (handler_state == int_thread::stopped && RUNNING_STATE(target_state)) {
         result = thr->intCont();
      }
      else if (RUNNING_STATE(handler_state) && target_state == int_thread::stopped) {
         result = thr->intStop();
      }
      if (!result && getLastError() == err_exited) {
         pthrd_printf("Suppressing error of continue on exited process");
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
   int_process(p, e, a, envp, f)
{
}

hybrid_lwp_control_process::hybrid_lwp_control_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p)
{
}

hybrid_lwp_control_process::~hybrid_lwp_control_process()
{
}

bool hybrid_lwp_control_process::suspendThread(int_thread *thr)
{
   bool result = plat_suspendThread(thr);
   if (!result) 
      return false;
   thr->setSuspended(true);
   return true;
}

bool hybrid_lwp_control_process::resumeThread(int_thread *thr)
{
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
   }
}

bool hybrid_lwp_control_process::plat_debuggerSuspended()
{
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
	   pthrd_printf("Checking %d/%d\n", getPid(), thr->getLWP());
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
	   pthrd_printf("WARNING: did not find a running thread, using initial thread\n");
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
	  // DEBUG HACK
	  windows_thread* wt = dynamic_cast<windows_thread*>(thr);
	  if(wt) {
		  wt->plat_suspend();
		  wt->plat_resume();
	  }
	  if (thr->getDetachState().getState() == int_thread::detached)
         continue;
      if (thr->isSuspended() && RUNNING_STATE(thr->getTargetState())) {
         pthrd_printf("Resuming thread %d/%d\n", getPid(), thr->getLWP());
         result = resumeThread(thr);
      }
      else if (!thr->isSuspended() && !RUNNING_STATE(thr->getTargetState())) {
         pthrd_printf("Suspending thread %d/%d\n", getPid(), thr->getLWP());
         result = suspendThread(thr);
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
   handler_running_thrd_count(Counter::HandlerRunningThreads),
   generator_running_thrd_count(Counter::GeneratorRunningThreads),
   sync_rpc_count(Counter::SyncRPCs),
   sync_rpc_running_thr_count(Counter::SyncRPCRunningThreads),
   pending_stop(Counter::PendingStops),
   clearing_bp_count(Counter::ClearingBPs),
   proc_stop_rpc_count(Counter::ProcStopRPCs),
   generator_nonexited_thrd_count(Counter::GeneratorNonExitedThreads),
   exiting_state(this, ExitingStateID, dontcare),
   startup_state(this, StartupStateID, dontcare),
   pending_stop_state(this, PendingStopStateID, dontcare),
   callback_state(this, CallbackStateID, dontcare),
   breakpoint_state(this, BreakpointStateID, dontcare),
   breakpoint_resume_state(this, BreakpointResumeStateID, dontcare),
   irpc_setup_state(this, IRPCSetupStateID, dontcare),
   irpc_wait_state(this, IRPCWaitStateID, dontcare),
   irpc_state(this, IRPCStateID, dontcare),
   async_state(this, AsyncStateID, dontcare),
   internal_state(this, InternalStateID, dontcare),
   detach_state(this, DetachStateID, dontcare),
   user_irpc_state(this, UserRPCStateID, dontcare),
   user_state(this, UserStateID, neonatal),
   handler_state(this, HandlerStateID, neonatal),
   generator_state(this, GeneratorStateID, neonatal),
   target_state(int_thread::none),
   saved_user_state(int_thread::none),
   regpool_lock(true),
   user_single_step(false),
   single_step(false),
   handler_exiting_state(false),
   generator_exiting_state(false),
   running_when_attached(true),
   suspended(false),
   stopped_on_breakpoint_addr(0x0),
   clearing_breakpoint(NULL),
   em_singlestep(NULL)
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
   if (b) {
      pending_stop.inc();

      //You may ask why stopping moves the pending stop state to running.
      // We've sent the process a stop request, and we need that request to 
      // be delivered.  The process won't take delivery of the stop request
      // unless it's running.
      getPendingStopState().desyncState(int_thread::running);
   }
   else {
      getPendingStopState().restoreState();
      pending_stop.dec();
   }
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

int_thread::StateTracker &int_thread::getBreakpointState()
{
   return breakpoint_state;
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
	pthrd_printf("%d/%d: setting target state %s\n", proc_->getPid(), getLWP(), RUNNING_STATE(s) ? "R" : "S");
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
      case ExitingStateID: return exiting_state;
      case StartupStateID: return startup_state;
      case AsyncStateID: return async_state;
      case CallbackStateID: return callback_state;
      case PendingStopStateID: return pending_stop_state;
      case IRPCStateID: return irpc_state;
      case IRPCSetupStateID: return irpc_setup_state;
      case IRPCWaitStateID: return irpc_wait_state;
      case BreakpointStateID: return breakpoint_state;
      case BreakpointResumeStateID: return breakpoint_resume_state;
      case InternalStateID: return internal_state;
      case DetachStateID: return detach_state;
      case UserRPCStateID: return user_irpc_state;
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
      case ExitingStateID: return "exiting";      
      case StartupStateID: return "startup";
      case AsyncStateID: return "async";
      case CallbackStateID: return "callback";
      case PendingStopStateID: return "pending stop";
      case IRPCStateID: return "irpc";
      case IRPCSetupStateID: return "irpc setup";
      case IRPCWaitStateID: return "irpc wait";
      case BreakpointStateID: return "breakpoint";
      case BreakpointResumeStateID: return "breakpoint resume";
      case InternalStateID: return "internal";
      case UserRPCStateID: return "irpc user";
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
                                     bool initial_thrd)
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
                proc->getPid(), newthr->getLWP(), thr_id);
   proc->threadPool()->addThread(newthr);
   if (initial_thrd) {
      proc->threadPool()->setInitialThread(newthr);
   }
   ProcPool()->addThread(proc, newthr);
   bool result = newthr->attach();
   if (!result) {
      pthrd_printf("Failed to attach to new thread %d/%d\n", proc->getPid(), lwp_id);
      return NULL;
   }
   if (newthr->isUser() && newthr->getUserState().getState() == neonatal) {
	   newthr->getUserState().setState(neonatal_intermediate);
	   newthr->getHandlerState().setState(neonatal_intermediate);
		newthr->getGeneratorState().setState(neonatal_intermediate);
   }
   return newthr;
}

void int_thread::throwEventsBeforeContinue()
{
   Event::ptr new_ev;

   int_iRPC::ptr rpc = nextPostedIRPC();
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
   else if (isStoppedOnBP()) {
      pthrd_printf("Found thread %d/%d to be stopped on a BP, not continuing\n", llproc()->getPid(), getLWP());
      getBreakpointState().desyncStateProc(int_thread::stopped);
      new_ev = EventBreakpointClear::ptr(new EventBreakpointClear());
   }

   if (new_ev) {
      new_ev->setProcess(proc());
      new_ev->setThread(thread());
      new_ev->setSyncType(Event::async);
      new_ev->setSuppressCB(true);
      mbox()->enqueue(new_ev);
   }
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
   
   thrd->getUserState().setState(int_thread::exited);

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
      thrd->getExitingState().setState(int_thread::running);
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

   pthrd_printf("Reading registers for thread %d\n", getLWP());
   regpool_lock.lock();
   if (cached_regpool.full) {
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

   if (!llproc()->plat_needsAsyncIO()) {
      pthrd_printf("Setting registers for thread %d\n", getLWP());
      bool result = plat_setAllRegisters(pool);
      response->setResponse(result);
      if (!result) {
         pthrd_printf("plat_setAllRegisters returned error on %d\n", getLWP());
         return false;
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

   if (!llproc()->plat_individualRegAccess())
   {
      pthrd_printf("Platform does not support individual register access, " 
                   "getting everything\n");
      assert(!llproc()->plat_needsAsyncIO());

      int_registerPool pool;
      allreg_response::ptr allreg_resp = allreg_response::createAllRegResponse(&pool);
      bool result = getAllRegisters(allreg_resp);
      bool is_ready = allreg_resp->isReady();
      if (!result || allreg_resp->hasError()) {
         pthrd_printf("Unable to access full register set\n");
         return false;
      }
      assert(is_ready);
      response->setResponse(pool.regs[reg]);
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
      if (result) {
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
   assert(getHandlerState().getState() == int_thread::stopped);
   assert(getGeneratorState().getState() == int_thread::stopped);
   bool ret_result = false;
   pthrd_printf("%d/%d: Setting %s to 0x%lx...\n", proc()->getPid(), tid, reg.name().c_str(), val);
   
   if (!llproc()->plat_individualRegAccess())
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
   if (running_rpc->isProcStopRPC()) {
      proc_stop_rpc_count.dec();
   }
   running_rpc = int_iRPC::ptr();
}

bool int_thread::saveRegsForRPC(allreg_response::ptr response)
{
   assert(!rpc_regs.full);
   response->setRegPool(&rpc_regs);
   return getAllRegisters(response);
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

void int_thread::markClearingBreakpoint(installed_breakpoint *bp)
{
   assert(!clearing_breakpoint || bp == NULL);
   pthrd_printf("%d/%d marking clearing bp %p (at 0x%lx)\n",
	   llproc()->getPid(), getLWP(), bp, bp ? bp->getAddr() : 0);
   clearing_breakpoint = bp;
   if (bp) {
      clearing_bp_count.inc();
   }
   else {
      clearing_bp_count.dec();
   }
}

void int_thread::markStoppedOnBP(installed_breakpoint *bp)
{
   stopped_on_breakpoint_addr = bp ? bp->getAddr() : 0x0;
}

installed_breakpoint *int_thread::isStoppedOnBP()
{
   return stopped_on_breakpoint_addr ? llproc()->getBreakpoint(stopped_on_breakpoint_addr) : NULL;
}

void int_thread::setTID(Dyninst::THR_ID tid_)
{
   tid = tid_;
}

installed_breakpoint *int_thread::isClearingBreakpoint()
{
   return clearing_breakpoint;
}

bool int_thread::haveUserThreadInfo()
{
   return false;
}

bool int_thread::getTID(Dyninst::THR_ID &)
{
   return false;
}

bool int_thread::getStartFuncAddress(Dyninst::Address &)
{
   return false;
}

bool int_thread::getStackBase(Dyninst::Address &)
{
   return false;
}

bool int_thread::getStackSize(unsigned long &)
{
   return false;
}

bool int_thread::getTLSPtr(Dyninst::Address &)
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

bool int_thread::StateTracker::setState(State to)
{
   std::string s = int_thread::stateIDToName(id);
   Dyninst::LWP lwp = up_thr->getLWP();
   Dyninst::PID pid = up_thr->llproc()->getPid();

   if (state == to) {
      pthrd_printf("Leaving %s state for %d/%d in state %s\n", s.c_str(), pid, lwp, stateStr(to));
      return true;
   }
   if (to == errorstate) {
      perr_printf("Setting %s state for %d/%d from %s to errorstate\n", 
                  s.c_str(), pid, lwp, stateStr(state));
      state = to;
      return true;
   }
   if (state == errorstate) {
      perr_printf("Attempted %s state reversion for %d/%d from errorstate to %s\n", 
                  s.c_str(), pid, lwp, stateStr(to));
      return false;
   }
   if (state == exited) {
      perr_printf("Attempted %s state reversion for %d/%d from exited to %s\n", 
                  s.c_str(), pid, lwp, stateStr(to));
      return false;
   }
   if (to == neonatal && state != none) {
      perr_printf("Attempted %s state reversion for %d/%d from %s to neonatal\n", 
                  s.c_str(), pid, lwp, stateStr(state));
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

   pthrd_printf("Changing %s state for %d/%d from %s to %s\n", s.c_str(), pid, lwp, 
                stateStr(state), stateStr(to));
   state = to;

   int_thread::State handler_state = up_thr->getHandlerState().getState();
   int_thread::State generator_state = up_thr->getGeneratorState().getState();
   if (up_thr->up_thread && handler_state == stopped) assert(generator_state == stopped || generator_state == exited || generator_state == detached );
   if (up_thr->up_thread && generator_state == running) assert(handler_state == running);
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
   proc_(p), had_multiple_threads(false),
	   initial_thread(NULL)
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
   data(false),
   onetime_bp(false),
   onetime_bp_hit(false),
   procstopper(false)
{
}

int_breakpoint::int_breakpoint(Dyninst::Address to_, Breakpoint::ptr up) :
   up_bp(up),
   to(to_),
   isCtrlTransfer_(true),
   data(false),
   onetime_bp(false),
   onetime_bp_hit(false),
   procstopper(false)
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

installed_breakpoint::installed_breakpoint(mem_state::ptr memory_, Address addr_) :
   memory(memory_),
   buffer_size(0),
   prepped(false),
   installed(false),
   long_breakpoint(false),
   suspend_count(0),
   addr(addr_)
{
}

installed_breakpoint::installed_breakpoint(mem_state::ptr memory_, 
                                           const installed_breakpoint *ip) :
   memory(memory_),
   bps(ip->bps),
   hl_bps(ip->hl_bps),
   buffer_size(ip->buffer_size),
   prepped(ip->prepped),
   installed(ip->installed),
   long_breakpoint(ip->long_breakpoint),
   suspend_count(ip->suspend_count),
   addr(ip->addr)
{
   memcpy(buffer, ip->buffer, sizeof(buffer));
}

installed_breakpoint::~installed_breakpoint()
{
}

bool installed_breakpoint::isInstalled() const
{
   return installed;
}

bool installed_breakpoint::writeBreakpoint(int_process *proc, result_response::ptr write_response)
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
   return proc->writeMem(bp_insn, addr, buffer_size, write_response);
}

bool installed_breakpoint::saveBreakpointData(int_process *proc, mem_response::ptr read_response)
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

   read_response->setBuffer(buffer, buffer_size);
   return proc->readMem(addr, read_response);
}

bool installed_breakpoint::restoreBreakpointData(int_process *proc, result_response::ptr res_resp)
{
   assert(buffer_size != 0);

   pthrd_printf("Restoring original code over breakpoint at %lx\n", addr);
   return proc->writeMem(buffer, addr, buffer_size, res_resp);
}

bool installed_breakpoint::uninstall(int_process *proc, result_response::ptr async_resp)
{
   assert(installed);
   bool had_success = true;
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

   std::map<Dyninst::Address, installed_breakpoint *>::iterator i;
   i = memory->breakpoints.find(addr);
   if (i == memory->breakpoints.end()) {
      perr_printf("Failed to remove breakpoint from list\n");
      return false;
   }
   memory->breakpoints.erase(i);

   return had_success;
}

bool installed_breakpoint::suspend(int_process *proc, result_response::ptr result_resp)
{
   suspend_count++;
   if (suspend_count > 1) {
      pthrd_printf("Breakpoint already suspended, suspend_count = %d\n", 
                   suspend_count);
      return true;
   }

   bool result = restoreBreakpointData(proc, result_resp);
   if (!result) {
      pthrd_printf("Failed to suspend breakpoint at %lx from process %d\n", 
                   addr, proc->getPid());
      return false;
   }
   return true;
}

bool installed_breakpoint::resume(int_process *proc, result_response::ptr async_resp)
{
   suspend_count--;
   assert(suspend_count >= 0);
   if (suspend_count > 0) {
      pthrd_printf("Breakpoint remaining suspended, suspend_count = %d\n", suspend_count);
      return true;
   }
   
   bool result = writeBreakpoint(proc, async_resp);
   if (!result) {
      pthrd_printf("Failed to install breakpoint at %lx in process %d\n",
                   addr, proc->getPid());
      return false;
   }

   return true;
}

bool installed_breakpoint::containsIntBreakpoint(int_breakpoint *bp) {
    return (bps.count(bp) > 0);
}

int_breakpoint *installed_breakpoint::getCtrlTransferBP(int_thread *thrd)
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

unsigned installed_breakpoint::getNumIntBreakpoints() const {
    return bps.size();
}

bool installed_breakpoint::addBreakpoint(int_breakpoint *bp)
{
   if (bp->isCtrlTransfer()) {
      for (set<int_breakpoint *>::iterator i = bps.begin(); i != bps.end(); ++i)
      {
         if ((*i)->isCtrlTransfer()) {
            perr_printf("Error.  Attempted to add two control transfer breakpoints " 
                        "at same place");
            setLastError(err_badparam, "Attempted two control transfer breakpoints at " 
                         "same location\n");
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

   memory->breakpoints[addr] = this;

   return true;
}

bool installed_breakpoint::prepBreakpoint(int_process *proc, mem_response::ptr mem_resp)
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

bool installed_breakpoint::insertBreakpoint(int_process *proc, result_response::ptr res_resp)
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

bool installed_breakpoint::rmBreakpoint(int_process *proc, int_breakpoint *bp, bool &empty, 
                                        result_response::ptr async_resp)
{
   pthrd_printf("Removing breakpoint at %lx\n", addr);
   empty = false;
   set<int_breakpoint *>::iterator i = bps.find(bp);
   if (i == bps.end()) {
      perr_printf("Attempted to remove a non-installed breakpoint\n");
      setLastError(err_badparam, "Breakpoint was not installed in process\n");
      empty = false;
      return false;
   }
   bps.erase(i);

   set<Breakpoint::ptr>::iterator j = hl_bps.find(bp->upBreakpoint().lock());
   if (j != hl_bps.end()) {
      hl_bps.erase(j);
   }

   if (bps.empty()) {
      empty = true;
      bool result = uninstall(proc, async_resp);
      if (!result) {
         perr_printf("Failed to remove breakpoint at %lx\n", addr);
         setLastError(err_internal, "Could not remove breakpoint\n");
         return false;
      }
   }
   else {
      pthrd_printf("installed_breakpoint %lx not empty after int_breakpoint remove.  Leaving.\n",
                   addr);
       async_resp->setResponse(true);
   }
   
   return true;
}

Dyninst::Address installed_breakpoint::getAddr() const
{
   return addr;
}

installed_breakpoint::iterator installed_breakpoint::begin()
{
   return bps.begin();
}

installed_breakpoint::iterator installed_breakpoint::end()
{
   return bps.end();
}


int_library::int_library(std::string n, Dyninst::Address load_addr, Dyninst::Address dynamic_load_addr, Dyninst::Address data_load_addr, bool has_data_load_addr) :
   name(n),
   load_address(load_addr),
   data_load_address(data_load_addr),
   dynamic_address(dynamic_load_addr),
   has_data_load(has_data_load_addr),
   marked(false),
   user_data(NULL)
{
   up_lib = Library::ptr(new Library());
   up_lib->lib = this;
}

int_library::int_library(int_library *l) :
   name(l->name),
   load_address(l->load_address),
   data_load_address(l->data_load_address),
   dynamic_address(l->dynamic_address),
   has_data_load(l->has_data_load),
   marked(l->marked),
   user_data(NULL)
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

   map<Dyninst::Address, installed_breakpoint *>::iterator j;
   for (j = m.breakpoints.begin(); j != m.breakpoints.end(); ++j)
   {
      Address orig_addr = j->first;
      installed_breakpoint *orig_bp = j->second;
      installed_breakpoint *new_bp = new installed_breakpoint(this, orig_bp);
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

   map<Dyninst::Address, installed_breakpoint *>::iterator j;
   for (j = breakpoints.begin(); j != breakpoints.end(); ++j)
   {
      installed_breakpoint *ibp = j->second;
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


int_notify *int_notify::the_notify = NULL;
int_notify::int_notify() :
   events_noted(0)
{
   the_notify = this;
   up_notify = new EventNotify();
   up_notify->llnotify = this;
}

int_notify *notify()
{
   if (int_notify::the_notify)
      return int_notify::the_notify;

   static Mutex init_lock;
   init_lock.lock();
   if (!int_notify::the_notify) {
      int_notify::the_notify = new int_notify();
   }
   init_lock.unlock();
   return int_notify::the_notify;
}

void int_notify::noteEvent()
{
//MATT TODO lock around event pipe write/read when using process locks
   assert(isHandlerThread());
   //assert(events_noted == 0);
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
   //assert(events_noted == 0);
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

bool RegisterPool::iterator::operator==(const iterator &iter)
{
    return i == iter.i;
}

bool RegisterPool::iterator::operator!=(const iterator &iter)
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

bool RegisterPool::const_iterator::operator==(const const_iterator &iter)
{
    return i != iter.i;
}

bool RegisterPool::const_iterator::operator!=(const const_iterator &iter)
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

RegisterPool::iterator::~iterator()
{
}

std::pair<Dyninst::MachRegister, Dyninst::MachRegisterVal> RegisterPool::iterator::operator*()
{
   return *i;
}

RegisterPool::iterator RegisterPool::iterator::operator++()
{
   int_iter orig = i;
   i++;
   return RegisterPool::iterator(i);
}

RegisterPool::iterator RegisterPool::iterator::operator++(int)
{
   i++;
   return *this;
}

RegisterPool::const_iterator::const_iterator()
{
}

RegisterPool::const_iterator::const_iterator(int_iter i_) :
   i(i_)
{
}

RegisterPool::const_iterator::~const_iterator()
{
}

std::pair<Dyninst::MachRegister, Dyninst::MachRegisterVal> RegisterPool::const_iterator::operator*() const
{
   return *i;
}

RegisterPool::const_iterator RegisterPool::const_iterator::operator++()
{
   int_iter orig = i;
   i++;
   return RegisterPool::const_iterator(i);
}

RegisterPool::const_iterator RegisterPool::const_iterator::operator++(int)
{
   i++;
   return *this;
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

int_registerPool::int_registerPool(const int_registerPool &c) :
   regs(c.regs),
   full(c.full),
   thread(c.thread)
{
}

int_registerPool::~int_registerPool()
{
}

Library::Library()
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
      setLastError(err_exited, "Process is exited\n");
      return -1;
   }
   return proc->numLibs();
}

Library::ptr LibraryPool::getLibraryByName(std::string s)
{
   MTLock lock_this_func;
   if (!proc) {
      perr_printf("getLibraryByName on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
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
      setLastError(err_exited, "Process is exited\n");
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
      setLastError(err_exited, "Process is exited\n");
      return Library::ptr();
   }
   return proc->plat_getExecutable()->up_lib;
}

Library::const_ptr LibraryPool::getExecutable() const
{
   MTLock lock_this_func;
   if (!proc) {
      perr_printf("getExecutable on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return Library::ptr();
   }
   return proc->plat_getExecutable()->up_lib;
}

LibraryPool::iterator::iterator()
{
}

LibraryPool::iterator::~iterator()
{
}

Library::ptr LibraryPool::iterator::operator*() const
{
   return (*int_iter)->up_lib;
}

LibraryPool::iterator LibraryPool::iterator::operator++()
{
   LibraryPool::iterator orig = *this;
   ++int_iter;
   return orig;
}

LibraryPool::iterator LibraryPool::iterator::operator++(int)
{
   ++int_iter;
   return *this;
}

LibraryPool::iterator LibraryPool::begin()
{
   LibraryPool::iterator i;
   i.int_iter = proc->memory()->libs.begin();
   return i;
}

LibraryPool::iterator LibraryPool::end()
{
   LibraryPool::iterator i;
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
   LibraryPool::const_iterator i;
   i.int_iter = proc->memory()->libs.begin();
   return i;
}

LibraryPool::const_iterator LibraryPool::end() const
{
   LibraryPool::const_iterator i;
   i.int_iter = proc->memory()->libs.end();
   return i;
}

LibraryPool::const_iterator::const_iterator()
{
}

LibraryPool::const_iterator::~const_iterator()
{
}

Library::const_ptr LibraryPool::const_iterator::operator*() const
{
   return (*int_iter)->up_lib;
}

bool LibraryPool::const_iterator::operator==(const LibraryPool::const_iterator &i)
{
   return int_iter == i.int_iter;
}

bool LibraryPool::const_iterator::operator!=(const LibraryPool::const_iterator &i)
{
   return int_iter != i.int_iter;
}

LibraryPool::const_iterator LibraryPool::const_iterator::operator++()
{
   LibraryPool::const_iterator orig = *this;
   ++int_iter;
   return orig;
}

LibraryPool::const_iterator LibraryPool::const_iterator::operator++(int)
{
   ++int_iter;
   return *this;
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
      setLastError(err_incallback, "Cannot handleEvents from callback\n");
      return false;
   }

   bool result = int_process::waitAndHandleEvents(block);
   if (!result) {
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
      setLastError(err_incallback, "Cannot createProcess from callback\n");
      return Process::ptr();
   }

   ProcPool()->condvar()->lock();
   
   Process::ptr newproc(new Process());
   int_process *llproc = int_process::createProcess(executable, argv, envp, fds);
   llproc->initializeProcess(newproc);
   
   bool result = llproc->create(); //Releases procpool lock
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
      setLastError(err_incallback, "Cannot attachProcess from callback\n");
      return Process::ptr();
   }

   ProcPool()->condvar()->lock();

   Process::ptr newproc(new Process());
   int_process *llproc = int_process::createProcess(pid, executable);
   llproc->initializeProcess(newproc);

   bool result = llproc->attach(); //Releases procpool lock
   if (!result) {
      pthrd_printf("Unable to attach to process %d\n", pid);
      delete llproc;
      return Process::ptr();
   }

   return newproc;
}

Process::Process() :
   llproc_(NULL),
   exitstate_(NULL),
   userData_(NULL)
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
    return userData_;
}

void Process::setData(void *p) {
    userData_ = p;
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

int_process *Process::llproc() const
{
   return llproc_;
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

bool Process::continueProc()
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   if (!llproc_) {
      perr_printf("coninueProc on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

   if( llproc_->getState() == int_process::detached ) {
       perr_printf("continueProc on detached process\n");
       setLastError(err_detached, "Process is detached\n");
       return false;
   }

   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      setLastError(err_incallback, "Cannot continueProc from callback\n");
      return false;
   }

   pthrd_printf("User continuing entire process %d\n", getPid());
   llproc_->threadPool()->initialThread()->getUserState().setStateProc(int_thread::running);
   llproc_->throwNopEvent();

   return true;
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
    if (!llproc_) {
        perr_printf("isDetached called on deleted process\n");
        setLastError(err_exited, "Process is exited\n");
        return false;
    }

    return llproc_->getState() == int_process::detached;
}

bool Process::stopProc()
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   if (!llproc_) {
      perr_printf("stopProc on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

   if( llproc_->getState() == int_process::detached ) {
      perr_printf("stopProc on detached process\n");
      setLastError(err_detached, "Process is detached\n");
      return false;
   }

   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      setLastError(err_incallback, "Cannot stopProc from callback\n");
      return false;
   }

   pthrd_printf("User stopping entire process %d\n", getPid());
   llproc_->threadPool()->initialThread()->getUserState().setStateProc(int_thread::stopped);
   llproc_->throwNopEvent();

   bool proc_exited = false;
   bool result = int_process::waitAndHandleForProc(false, llproc_, proc_exited);
   if (proc_exited) {
      perr_printf("Process exited while waiting for user stop, erroring\n");
      setLastError(err_exited, "Process exited while being stopped.\n");
      return false;
   }
   if (!result) {
      perr_printf("Internal error calling waitAndHandleForProc on %d\n", llproc_->getPid());
      setLastError(err_internal, "Error while calling waitAndHandleForProc from process stop\n");
      return false;
   }
   return true;
}

bool Process::detach()
{
   MTLock lock_this_func(MTLock::deliver_callbacks);

   if (!llproc_) {
      perr_printf("detach on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

   if( llproc_->getState() == int_process::detached ) {
       perr_printf("detach on detached process\n");
       setLastError(err_detached, "Process is detached\n");
       return false;
   }

   bool pendingRPCs = false;
   int_threadPool *tp = llproc()->threadPool();
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); ++i) {
       int_thread *thr = *i;
       if( thr->getPostedRPCs()->size() > 0 ) {
           pendingRPCs = true;
           break;
       }
   }

   if( pendingRPCs ) {
       perr_printf("detach on a process with pending RPCs\n");
       setLastError(err_pendingirpcs, "Process has pending iRPCs, cannot detach\n");
       return false;
   }

   bool result = int_process::detach(llproc_, false);
   if (!result) {
      pthrd_printf("Failed to detach from process\n");
      return false;
   }
  
   return true;
}

bool Process::temporaryDetach()
{
    MTLock lock_this_func(MTLock::deliver_callbacks);
    if (!llproc_) {
        perr_printf("temporary detach on deleted process\n");
        setLastError(err_exited, "Process is exited\n");
        return false;
    }
    if (!llproc_->plat_supportDOTF()) {
       perr_printf("Temporary detach not supported on this platform\n");
       setLastError(err_unsupported, "Temporary detach not supported on this platform\n");
       return false;
    }

    if( llproc_->getState() == int_process::detached ) {
        perr_printf("temporary detach on already detached process\n");
        setLastError(err_detached, "Process is already detached\n");
        return false;
    }

    bool pendingRPCs = false;
    int_threadPool *tp = llproc()->threadPool();
    for (int_threadPool::iterator i = tp->begin(); i != tp->end(); ++i) {
        int_thread *thr = *i;
        if( thr->getPostedRPCs()->size() > 0 ) {
            pendingRPCs = true;
            break;
        }
    }

    if( pendingRPCs ) {
        perr_printf("temporary detach on a process with pending RPCs\n");
        setLastError(err_pendingirpcs, "Process has pending iRPCs, cannot detach\n");
        return false;
    }

    bool result = int_process::detach(llproc_, true);
    if( !result ) {
        pthrd_printf("Failed to detach from process\n");
        return false;
    }

    return true;
}

bool Process::reAttach()
{
    MTLock lock_this_func(MTLock::deliver_callbacks);
    if (!llproc_) {
        perr_printf("reAttach on deleted process\n");
        setLastError(err_exited, "Process is exited\n");
        return false;
    }
    if (!llproc_->plat_supportDOTF()) {
       perr_printf("reAttach not supported on this platform\n");
       setLastError(err_unsupported, "reAttach not supported on this platform\n");
       return false;
    }

    bool result = llproc_->reattach();
    if( !result ) {
        pthrd_printf("Failed to reattach to process\n");
        return false;
    }

    return true;
}

bool Process::terminate()
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   if (!llproc_) {
      perr_printf("terminate on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

   if( llproc_->getState() == int_process::detached ) {
       perr_printf("terminate on detached process\n");
       setLastError(err_detached, "Process is detached\n");
       return false;
   }

   pthrd_printf("User terminating process %d\n", llproc_->getPid());

   bool needsSync = false;
   bool result = llproc_->terminate(needsSync);
   if (!result) {
      pthrd_printf("Terminating process %d failed\n", llproc_->getPid());
      return false;
   }

   if (needsSync) {
      bool proc_exited = false;
      while (!proc_exited) {
         bool result = int_process::waitAndHandleForProc(true, llproc_, proc_exited);
         if (!result) {
            perr_printf("Error waiting for process to terminate\n");
            return false;
         }
      }
   }
   else {
      HandlerPool *hp = llproc_->handlerPool();
      delete llproc_;
      delete hp;
      assert(!llproc_);
   }

   return true;
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
   if (!llproc_) {
      perr_printf("hasStoppedThread on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

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
   if (!llproc_) {
      perr_printf("hasRunningThread on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

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
   if (!llproc_) {
      perr_printf("allThreadsStopped on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

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
   if (!llproc_) {
      perr_printf("allThreadsRunning on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

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
    if(!llproc_) {
        perr_printf("allThreadsRunningWhenAttached on deleted process\n");
        setLastError(err_exited, "Process is exited\n");
        return false;
    }

    for(int_threadPool::iterator i = llproc_->threadPool()->begin(); 
            i != llproc_->threadPool()->end(); ++i)
    {
        if( !(*i)->wasRunningWhenAttached() ) return false;
    }

    return true;
}

bool Process::postSyncIRPC(IRPC::ptr irpc)
{
   MTLock lock_this_func;
   if (!llproc_) {
      perr_printf("postIRPC on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

   if (llproc_->getState() == int_process::detached) {
       perr_printf("postIRPC on detached process\n");
       setLastError(err_detached, "Process is detached\n");
       return false;
   }

   int_process *proc = llproc();
   int_iRPC::ptr rpc = irpc->llrpc()->rpc;
   bool result = rpcMgr()->postRPCToProc(proc, rpc);
   if (!result) {
      pthrd_printf("postRPCToProc failed on %d\n", proc->getPid());
      return false;
   }
   rpc->thread()->throwEventsBeforeContinue();
   assert(!rpc->isAsync());
	while ((rpc->getState() != int_iRPC::Finished)) {
		bool result = int_process::waitAndHandleEvents(true);
	 if (!result) {
		perr_printf("Error waiting for RPC to complete\n");
		return false;
	 }
	}
	return true;

}

Thread::ptr Process::postIRPC(IRPC::ptr irpc) const
{
   MTLock lock_this_func;
   if (!llproc_) {
      perr_printf("postIRPC on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return Thread::ptr();
   }

   if (llproc_->getState() == int_process::detached) {
       perr_printf("postIRPC on detached process\n");
       setLastError(err_detached, "Process is detached\n");
       return Thread::ptr();
   }

   int_process *proc = llproc();
   int_iRPC::ptr rpc = irpc->llrpc()->rpc;
   bool result = rpcMgr()->postRPCToProc(proc, rpc);
   if (!result) {
      pthrd_printf("postRPCToProc failed on %d\n", proc->getPid());
      return Thread::ptr();
   }
   rpc->thread()->throwEventsBeforeContinue();
   if(rpc->isAsync()) 
	   return rpc->thread()->thread();
	while ((rpc->getState() != int_iRPC::Finished)) {
		bool result = int_process::waitAndHandleEvents(true);
	 if (!result) {
		perr_printf("Error waiting for process to terminate\n");
		return Thread::ptr();
	 }
	}
	return rpc->thread()->thread();
   //llproc_->throwNopEvent();
}

bool Process::getPostedIRPCs(std::vector<IRPC::ptr> &rpcs) const
{
   MTLock lock_this_func;
   if (!llproc_) {
      perr_printf("postIRPC on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }
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
   if (!llproc_) {
      perr_printf("getArchitecture on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return Dyninst::Arch_none;
   }
   return llproc_->getTargetArch();
}

Dyninst::OSType Process::getOS() const
{
   MTLock lock_this_func;
   if (!llproc_) {
      perr_printf("getOS on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return Dyninst::OSNone;
   }

   return llproc_->getOS();
}

bool Process::supportsLWPEvents() const
{
   MTLock lock_this_func;
   if (!llproc_) {
      perr_printf("Support query on deleted process\n");
      return false;
   }
   //Intentionally not testing plat_supportLWP*Destroy, which is complicated on BG
   return llproc_->plat_supportLWPCreate(); 
}

bool Process::supportsUserThreadEvents() const
{
   MTLock lock_this_func;
   if (!llproc_) {
      perr_printf("Support query on deleted process\n");
      return false;
   }
   return llproc_->plat_supportThreadEvents();
}

bool Process::supportsFork() const
{
   MTLock lock_this_func;
   if (!llproc_) {
      perr_printf("Support query on deleted process\n");
      return false;
   }
   return llproc_->plat_supportFork();
}

bool Process::supportsExec() const
{
   MTLock lock_this_func;
   if (!llproc_) {
      perr_printf("Support query on deleted process\n");
      return false;
   }
   return llproc_->plat_supportExec();
}


Dyninst::Address Process::mallocMemory(size_t size, Dyninst::Address addr)
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   if (!llproc_) {
      perr_printf("mallocMemory on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

   if( llproc_->getState() == int_process::detached ) {
       perr_printf("mallocMemory on detached process\n");
       setLastError(err_detached, "Process is detached\n");
       return false;
   }

   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      setLastError(err_incallback, "Cannot mallocMemory from callback\n");
      return false;
   }
   return llproc_->infMalloc(size, true, addr);
}

Dyninst::Address Process::mallocMemory(size_t size)
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   if (!llproc_) {
      perr_printf("mallocMemory on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

   if( llproc_->getState() == int_process::detached ) {
       perr_printf("mallocMemory on detached process\n");
       setLastError(err_detached, "Process is detached\n");
       return false;
   }

   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      setLastError(err_incallback, "Cannot mallocMemory from callback\n");
      return false;
   }
   return llproc_->infMalloc(size, false, 0x0);
}

bool Process::freeMemory(Dyninst::Address addr)
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   if (!llproc_) {
      perr_printf("freeMemory on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

   if( llproc_->getState() == int_process::detached ) {
       perr_printf("freeMemory on detached process\n");
       setLastError(err_detached, "Process is detached\n");
       return false;
   }

   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      setLastError(err_incallback, "Cannot freeMemory from callback\n");
      return false;
   }
   return llproc_->infFree(addr);
}

bool Process::writeMemory(Dyninst::Address addr, const void *buffer, size_t size) const
{
   MTLock lock_this_func;
   if (!llproc_) {
      perr_printf("writeMemory on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

   if( llproc_->getState() == int_process::detached ) {
       perr_printf("writeMemory on detached process\n");
       setLastError(err_detached, "Process is detached\n");
       return false;
   }

   pthrd_printf("User wants to write memory to remote addr 0x%lx from buffer 0x%p of size %lu\n", 
                addr, buffer, (unsigned long) size);
   result_response::ptr resp = result_response::createResultResponse();
   bool result = llproc_->writeMem(buffer, addr, size, resp);
   if (!result) {
      pthrd_printf("Error writing to memory\n");
      resp->isReady();
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
   if (!llproc_) {
      perr_printf("readMemory on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

   if( llproc_->getState() == int_process::detached ) {
       perr_printf("readMemory on detached process\n");
       setLastError(err_detached, "Process is detached\n");
       return false;
   }

   pthrd_printf("User wants to read memory from 0x%lx to 0x%p of size %lu\n", 
                addr, buffer, (unsigned long) size);
   mem_response::ptr memresult = mem_response::createMemResponse((char *) buffer, size);
   bool result = llproc_->readMem(addr, memresult);
   if (!result) {
      pthrd_printf("Error reading from memory %lx on target process %d\n",
                   addr, llproc_->getPid());
      memresult->isReady();
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

bool Process::addBreakpoint(Address addr, Breakpoint::ptr bp) const
{
   MTLock lock_this_func;
   if (!llproc_) {
      perr_printf("addBreakpoint on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

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
   if (!llproc_) {
      perr_printf("rmBreakpoint on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

   if (hasRunningThread()) {
      perr_printf("User attempted to remove breakpoint on running process\n");
      setLastError(err_notstopped, "Attempted to remove breakpoint on running process\n");
      return false;
   }

   if( llproc_->getState() == int_process::detached ) {
       perr_printf("User attempted to remove breakpoint from detached process\n");
       setLastError(err_detached, "Attempted to remove breakpoint from detached process\n");
       return false;
   }

   result_response::ptr resp = result_response::createResultResponse();   
   bool result = llproc_->rmBreakpoint(addr, bp->llbp(), resp);
   if (!result) {
      pthrd_printf("Failed to rmBreakpoint\n");
      return false;
   }

   int_process::waitForAsyncEvent(resp);

   if (resp->hasError() || !resp->getResult()) {
      pthrd_printf("Error removing breakpoint\n");
      return false;
   }
   
   return true;
   
}

SymbolReaderFactory *Process::getDefaultSymbolReader()
{
   MTLock lock_this_func;
   if (!llproc_) {
      perr_printf("getDefaultSymbolReader on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return NULL;
   }

   return llproc()->plat_defaultSymReader();
}

ExecFileInfo* Process::getExecutableInfo() const
{
   MTLock lock_this_func;
   if (!llproc_) {
      perr_printf("getExecutableInfo on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return NULL;
   }

   return llproc()->plat_getExecutableInfo();
}


Thread::Thread() :
   llthread_(NULL),
   exitstate_(NULL),
   userData_(NULL)
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
    return userData_;
}

void Thread::setData(void *p) {
    userData_ = p;
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
   if (!llthread_) {
      perr_printf("isStopped called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }
   return llthread_->getUserState().getState() == int_thread::stopped;
}

bool Thread::isRunning() const
{
   MTLock lock_this_func;
   if (!llthread_) {
      perr_printf("isRunning called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }
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
    if (!llthread_) {
        perr_printf("isDetached called on exited thread\n");
        setLastError(err_exited, "Thread is exited\n");
        return false;
    }
    return llthread_->getUserState().getState() == int_thread::detached;
}

bool Thread::stopThread()
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   if (!llthread_) {
      perr_printf("stopThread called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }

   if( llthread_->getUserState().getState() == int_thread::detached ) {
       perr_printf("stopThread on detached thread\n");
       setLastError(err_detached, "Thread is detached\n");
       return false;
   }

   if (int_process::isInCB()) {
      perr_printf("User attempted continue call on thread while in CB, erroring.");
      setLastError(err_incallback, "Cannot continueThread from callback\n");
      return false;
   }

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
   if (!llthread_) {
      perr_printf("continueThread called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }

   if( llthread_->getUserState().getState() == int_thread::detached ) {
       perr_printf("continueThread on detached thread\n");
       setLastError(err_detached, "Thread is detached\n");
       return false;
   }

   if (int_process::isInCB()) {
      perr_printf("User attempted continue call on thread while in CB, erroring.");
      setLastError(err_incallback, "Cannot continueThread from callback\n");
      return false;
   }

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
   if (!llthread_) {
      perr_printf("getAllRegisters called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }

   if (llthread_->getUserState().getState() != int_thread::stopped) {
      setLastError(err_notstopped, "Thread must be stopped before getting registers");
      perr_printf("User called getAllRegisters on running thread %d\n", llthread_->getLWP());
      return false;
   }
   
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
   if (!llthread_) {
      perr_printf("setAllRegisters called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }
   if (llthread_->getUserState().getState() != int_thread::stopped) {
      setLastError(err_notstopped, "Thread must be stopped before setting registers");
      perr_printf("User called setAllRegisters on running thread %d\n", llthread_->getLWP());
      return false;
   }
   
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
   if (!llthread_) {
      perr_printf("getRegister called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }
   if (llthread_->getUserState().getState() != int_thread::stopped) {
      setLastError(err_notstopped, "Thread must be stopped before getting registers");
      perr_printf("User called getRegister on running thread %d\n", llthread_->getLWP());
      return false;
   }

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
   if (!llthread_) {
      perr_printf("setRegister called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }
   if (llthread_->getUserState().getState() != int_thread::stopped) {
      setLastError(err_notstopped, "Thread must be stopped before setting registers");
      perr_printf("User called setRegister on running thread %d\n", llthread_->getLWP());
      return false;
   }
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

bool Thread::isInitialThread() const
{
   MTLock lock_this_func;
   if (!llthread_) {
      perr_printf("isInitialThread called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }
   return llthread_->llproc()->threadPool()->initialThread() == llthread_;
}

bool Thread::isUser() const 
{
	MTLock lock_this_func;
	if (!llthread_) {
		perr_printf("isUser called on exited thread\n");
		setLastError(err_exited, "Thread is exited\n");
		return false;
	}
	return llthread_->isUser();
}

void Thread::setSingleStepMode(bool s) const
{
   MTLock lock_this_func;
   if (!llthread_) {
      perr_printf("setSingleStepMode called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return;
   }
   llthread_->setSingleStepUserMode(s);
}

bool Thread::getSingleStepMode() const
{
   MTLock lock_this_func;
   if (!llthread_) {
      perr_printf("getSingleStepMode called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }
   return llthread_->singleStepUserMode();
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
   if (!llthread_) {
	   perr_printf("postIRPC on deleted thread %d\n", getLWP());
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }

   if( llthread_->getUserState().getState() == int_thread::detached ) {
       perr_printf("postIRPC on detached thread\n");
       setLastError(err_detached, "Thread is detached\n");
       return false;
   }

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
   if (!llthread_) {
      perr_printf("postIRPC on deleted thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }
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
   if (!llthread_) {
      perr_printf("getStartFunction on deleted thread\n");
      setLastError(err_exited, "Thread is exited");
      return false;
   }

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
   if (!llthread_) {
      perr_printf("getStartFunction on deleted thread\n");
      setLastError(err_exited, "Thread is exited");
      return false;
   }

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
   if (!llthread_) {
      perr_printf("getStartFunction on deleted thread\n");
      setLastError(err_exited, "Thread is exited");
      return false;
   }

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
   if (!llthread_) {
      perr_printf("getStartFunction on deleted thread\n");
      setLastError(err_exited, "Thread is exited");
      return false;
   }

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
   if (!llthread_) {
      perr_printf("getStartFunction on deleted thread\n");
      setLastError(err_exited, "Thread is exited");
      return false;
   }

   Dyninst::Address addr;
   bool result = llthread_->getTLSPtr(addr);
   if (!result) {
      return 0;
   }
   return addr;
}

IRPC::const_ptr Thread::getRunningIRPC() const
{
   MTLock lock_this_func;
   if (!llthread_) {
      perr_printf("getRunningIRPC on deleted thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return IRPC::const_ptr();
   }
   int_iRPC::ptr running = llthread_->runningRPC();
   if (running == int_iRPC::ptr())
      return IRPC::const_ptr();
   IRPC::ptr irpc = running->getIRPC().lock();
   return irpc;
}

ThreadPool::ThreadPool()
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

ThreadPool::iterator::~iterator()
{
}

bool ThreadPool::iterator::operator==(const iterator &i)
{
   return (i.curh == curh);
}

bool ThreadPool::iterator::operator!=(const iterator &i)
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

ThreadPool::iterator ThreadPool::iterator::operator++()
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

ThreadPool::iterator ThreadPool::iterator::operator++(int)
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
	  if (curh->isUser())
		 continue;
      return *this;
   }
}

ThreadPool::iterator ThreadPool::begin()
{
   MTLock lock_this_func;
   ThreadPool::iterator i;
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
   ThreadPool::iterator i;
   i.curp = threadpool;
   i.curi = iterator::end_val;
   i.curh = Thread::ptr();
   return i;
}

ThreadPool::iterator ThreadPool::find(Dyninst::LWP lwp) 
{
    MTLock lock_this_func;
    ThreadPool::iterator i;
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

ThreadPool::const_iterator::~const_iterator()
{
}

bool ThreadPool::const_iterator::operator==(const const_iterator &i)
{
   return (i.curh == curh);
}

bool ThreadPool::const_iterator::operator!=(const const_iterator &i)
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

ThreadPool::const_iterator ThreadPool::const_iterator::operator++()
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

ThreadPool::const_iterator ThreadPool::const_iterator::operator++(int)
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

ThreadPool::const_iterator ThreadPool::begin() const
{
   MTLock lock_this_func;
   ThreadPool::const_iterator i;
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
   ThreadPool::const_iterator i;
   i.curp = threadpool;
   i.curi = const_iterator::end_val;
   i.curh = Thread::ptr();
   return i;
}

ThreadPool::const_iterator ThreadPool::find(Dyninst::LWP lwp) const 
{
    MTLock lock_this_func;
    ThreadPool::const_iterator i;
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

EventNotify::EventNotify()
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
   return notify()->up_notify;
}

Breakpoint::Breakpoint()
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
   newbp->llbreakpoint_ = new int_breakpoint(to, newbp);
   return newbp;
}

void *Breakpoint::getData() const {
   return llbreakpoint_->getData();
}

void Breakpoint::setData(void *d) {
   llbreakpoint_->setData(d);
}

bool Breakpoint::isCtrlTransfer() const {
   return llbreakpoint_->isCtrlTransfer();
}

Dyninst::Address Breakpoint::getToAddress() const
{
   return llbreakpoint_->toAddr();
}

Mutex Counter::locks[Counter::NumCounterTypes];
int Counter::global_counts[Counter::NumCounterTypes];

Counter::Counter(CounterType ct_) :
   local_count(0),
   ct(ct_)
{
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
   locks[index].lock();
   global_counts[index] += val;
   assert(global_counts[index] >= 0);
   pthrd_printf("Adjusting counter %d by %d\n", index, val);
   locks[index].unlock();
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

MTManager::MTManager() :
   work_lock(true),
   have_queued_events(false),
   is_running(false),
   should_exit(false)
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
      setLastError(err_noproc, "Can't setThreadMode while processes are running\n");
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
         setLastError(err_badparam, "Invalid parameter to setThreadMode\n");
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
      if (should_exit)
         return;
      if (!have_queued_events) {
         pending_event_lock.wait();
      }
      have_queued_events = false;
      pending_event_lock.unlock();
   }
}

unsigned long MTManager::evhandler_main_wrapper(void *)
{
   setHandlerThread(DThread::self());
   mt()->evhandler_main();
   return 0;
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

      held_pstop_events.erase(i++);
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
         proc->rmBreakpoint(*i, bp, resp);
         clear_resps.insert(resp);
      }
   }

   for (set<response::ptr>::iterator i = clear_resps.begin(); i != clear_resps.end();) {
      response::ptr resp = *i;
      if (resp->isReady()) {
         clear_resps.erase(i++);
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
