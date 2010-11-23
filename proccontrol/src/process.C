/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
#include "proccontrol/h/Mailbox.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Handler.h"

#include <cstring>
#include <cassert>
#include <map>

using namespace Dyninst;
using namespace ProcControlAPI;
using namespace std;

const map<int,int> Process::emptyFDs;
Process::thread_mode_t threadingMode = Process::GeneratorThreading;
bool int_process::in_callback = false;

bool int_process::create()
{
   ProcPool()->condvar()->lock();
   
   bool result = plat_create();
   if (!result) {
      pthrd_printf("Could not create debuggee, %s\n", executable.c_str());
      ProcPool()->condvar()->unlock();
      return false;
   }

   int_thread *initial_thread;
   initial_thread = int_thread::createThread(this, NULL_THR_ID, NULL_LWP, true);

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

bool int_process::multi_attach(std::vector<int_process *> &pids)
{
   bool result;
   bool had_error = false;
   std::vector<int_process *>::iterator i;

#define for_each_procdebug(func, err_msg)                      \
   for (i = pids.begin(); i != pids.end(); i++) {              \
      int_process *pd = (*i);                                  \
      if (!pd)                                                 \
         continue;                                             \
      result = pd->func();                                     \
      if (!result) {                                           \
         pthrd_printf("Could not %s to %d", err_msg, pd->pid); \
         delete pd;                                            \
         *i = NULL;                                            \
         had_error = true;                                     \
      }                                                        \
   }

   ProcPool()->condvar()->lock();

   for_each_procdebug(plat_attach, "attach");
   //MATT TODO: Add to ProcPool

   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();

   for_each_procdebug(waitfor_startup, "wait for attach");

   for_each_procdebug(post_attach, "post attach");

   return had_error;
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
         perr_printf("Failed to get thread LWPs for %d\n", pid);
         return false;
      }
      
      for (vector<Dyninst::LWP>::iterator i = lwps.begin(); i != lwps.end(); i++) {
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
   ProcPool()->condvar()->lock();

   pthrd_printf("Attaching to process %d\n", pid);
   bool result = plat_attach();
   if (!result) {
      ProcPool()->condvar()->broadcast();
      ProcPool()->condvar()->unlock();
      pthrd_printf("Could not attach to debuggee, %d\n", pid);
      return false;
   }

   int_thread *initial_thread;
   initial_thread = int_thread::createThread(this, NULL_THR_ID, NULL_LWP, true);

   ProcPool()->addProcess(this);

   setState(neonatal_intermediate);

   result = attachThreads();
   if (!result) {
      pthrd_printf("Failed to attach to threads in %d\n", pid);
      setLastError(err_internal, "Could not attach to process' threads");
      goto error;
   }

   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();


   pthrd_printf("Wait for attach from process %d\n", pid);
   result = waitfor_startup();
   if (!result) {
      pthrd_printf("Error waiting for attach to %d\n", pid);
      setLastError(err_internal, "Process failed to startup");
      goto error;
   }

   result = post_attach();
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
   
   while (!proc_stoppers.empty()) proc_stoppers.pop();

   int_threadPool::iterator i = threadpool->begin(); 
   for (; i != threadpool->end(); i++) {
      int_thread *thrd = *i;
      thrd->setHandlerState(int_thread::exited);
      thrd->setInternalState(int_thread::exited);
      thrd->setUserState(int_thread::exited);
      thrd->setGeneratorState(int_thread::exited);
      ProcPool()->rmThread(thrd);
      delete thrd;
   }
   threadpool->clear();


   int_thread *initial_thread;
   initial_thread = int_thread::createThread(this, NULL_THR_ID, NULL_LWP, true);
   initial_thread->setGeneratorState(int_thread::stopped);
   initial_thread->setHandlerState(int_thread::stopped);
   initial_thread->setInternalState(int_thread::running);
   initial_thread->setUserState(int_thread::running);


   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();
   
   bool result = plat_execed();

   return result;
}

bool int_process::forked()
{
   ProcPool()->condvar()->lock();

   pthrd_printf("Setting up forked process %d\n", pid);
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

bool int_process::post_attach()
{
   bool result = initLibraryMechanism();
   if (!result) {
      pthrd_printf("Error initializing library mechanism\n");
      return false;
   }

   std::set<int_library*> added, rmd;
   for (;;) {
      std::set<response::ptr> async_responses;
      result = refresh_libraries(added, rmd, async_responses);
      if (!result && !async_responses.empty()) {
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
      return true;
   }
}

bool int_process::post_create()
{
   return initLibraryMechanism();
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
   
   if (new_state < old_state) {
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
      case running: new_thr_state = int_thread::stopped; break;
      case exited: new_thr_state = int_thread::exited; break;
      case errorstate: new_thr_state = int_thread::errorstate; break;
   }
   pthrd_printf("Setting state of all threads in %d to %s\n", pid, 
                int_thread::stateStr(new_thr_state));
   for (int_threadPool::iterator i = threadpool->begin(); i != threadpool->end(); i++)
   {
      (*i)->setUserState(new_thr_state);
      (*i)->setInternalState(new_thr_state);
      (*i)->setHandlerState(new_thr_state);
      (*i)->setGeneratorState(new_thr_state);
   }
}

int_process::State int_process::getState() const
{
   return state;
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

struct syncRunStateRet_t {
   bool hasRunningThread;
   bool hasSyncRPC;
   bool hasStopPending;
   bool hasClearingBP;
   bool hasProcStopRPC;
   bool hasAsyncEvent;
   std::vector<int_process *> readyProcStoppers;
   syncRunStateRet_t() :
      hasRunningThread(false),
      hasSyncRPC(false),
      hasStopPending(false),
      hasClearingBP(false),
      hasProcStopRPC(false),
      hasAsyncEvent(false)
   {
   }
};

// Used by HybridLWPControl
bool int_process::continueProcess() {
    bool foundResumedThread = false;
    bool foundHandlerRunning = false;

    int_threadPool::iterator i;
    for(i = threadPool()->begin(); i != threadPool()->end(); ++i) {
        if( (*i)->isResumed() ) {
            pthrd_printf("Found resumed thread %d/%d\n",
                    getPid(), (*i)->getLWP());
            foundResumedThread = true;
        }

        if( (*i)->getHandlerState() == int_thread::running ) {
            foundHandlerRunning = true;
            break;
        }
    }

    if( foundResumedThread && !foundHandlerRunning ) { 
        if( !plat_contProcess() ) {
            perr_printf("Failed to continue whole process\n");
            return false;
        }
    }else{
        pthrd_printf("Did not find sufficient conditions to continue process %d\n",
                getPid());
    }

    return true;
}

bool syncRunState(int_process *p, void *r)
{
   int_threadPool *tp = p->threadPool();
   syncRunStateRet_t *ret = (syncRunStateRet_t *) r;
   assert(ret);
   
   if (p->hasQueuedProcStoppers() && p->threadPool()->allStopped()) {
      ret->readyProcStoppers.push_back(p);
   }
   if (p->forceGeneratorBlock()) {
      pthrd_printf("Process %d is forcing blocking via generator block\n", p->getPid());
      ret->hasRunningThread = true;
   }

   if (p->handlerPool()->hasAsyncEvent()) {
      ret->hasAsyncEvent = true;
   }

   if (dyninst_debug_proccontrol) {
      for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++)
      {
         int_thread *thr = *i;
         pthrd_printf("Pre-Thread %d/%d is in handler state %s with internal state %s (user is %s)\n",
                      p->getPid(), thr->getLWP(), 
                      int_thread::stateStr(thr->getHandlerState()),
                      int_thread::stateStr(thr->getInternalState()),
                      int_thread::stateStr(thr->getUserState()));
      }
   }

   /**
    * RPC Handling. 
    **/
   bool force_leave_stopped = false;
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++)
   {
      int_thread *thr = *i;

      thr->handleNextPostedIRPC(int_thread::hnp_no_stop, false);

      int_iRPC::ptr rpc = thr->hasRunningProcStopperRPC();
      if (!rpc) continue;

      ret->hasProcStopRPC = true;
      if (rpc->getState() >= int_iRPC::Prepping) {
         pthrd_printf("Thread %d/%d has pending proc stopper RPC(%lu), leaving other threads stopped\n",
                      p->getPid(), thr->getLWP(), rpc->id());
         force_leave_stopped = true;            
      }
   }

   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++)
   {
      int_thread *thr = *i;

      if (thr->hasPendingStop()) {
         ret->hasStopPending = true;
      }

      if (thr->isClearingBreakpoint()) {
         ret->hasClearingBP = true;
      }

      int_iRPC::ptr pstop_rpc = thr->hasRunningProcStopperRPC();
      if (thr->hasPendingStop() && thr->getHandlerState() == int_thread::stopped) {
         pthrd_printf("Continuing thread %d/%d to clear out pending stop\n", 
                      thr->llproc()->getPid(), thr->getLWP());

         thr->intCont();
      }
      else if (thr->getInternalState() == int_thread::stopped && pstop_rpc && 
               (pstop_rpc->getState() == int_iRPC::Running ||
                pstop_rpc->getState() == int_iRPC::Ready) )
      {
         pthrd_printf("Continuing thread %d/%d due to pending procstop iRPC\n",
                      p->getPid(), thr->getLWP());
         thr->intCont();
      }
      else if (pstop_rpc && 
               thr->getInternalState() == int_thread::running && 
               thr->getHandlerState() == int_thread::stopped &&
               pstop_rpc == thr->runningRPC())
      {
         pthrd_printf("Thread %d/%d was stopped during proccstopper (maybe due to signal).\n",
                      p->getPid(), thr->getLWP());
         thr->intCont();
      }
      else if (thr->getInternalState() == int_thread::running && 
               thr->getHandlerState() == int_thread::stopped &&
               !force_leave_stopped)
      {
         //The thread is stopped, but the user wants it running (we probably just finished 
         // handling a sync event). Go ahead and continue the thread.
         pthrd_printf("Continuing thread %d/%d to match internal state after events\n",
                      p->getPid(), thr->getLWP());
         thr->intCont();
      }

      if (thr->getInternalState() == int_thread::running || 
          thr->getInternalState() == int_thread::neonatal_intermediate ||
          thr->isResumed() ) 
      {
         //Keep track if any threads are running and running synchronous RPCs
         ret->hasRunningThread = true;
         if (thr->hasSyncRPC() && thr->runningRPC()) {
            ret->hasSyncRPC = true;
            pthrd_printf("Thread %d/%d has sync RPC\n",
                    p->getPid(), thr->getLWP());
         }
      }
   }
   pthrd_printf("Finished syncing runState for %d\n", p->getPid());
   if (dyninst_debug_proccontrol) {
      for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++)
      {
         int_thread *thr = *i;
         pthrd_printf("Post-Thread %d/%d is in handler state %s with internal state %s (user is %s)\n",
                      p->getPid(), thr->getLWP(), 
                      int_thread::stateStr(thr->getHandlerState()),
                      int_thread::stateStr(thr->getInternalState()),
                      int_thread::stateStr(thr->getUserState()));
      }
   }

   if( useHybridLWPControl() ) {
       return p->continueProcess();
   }

   return true;
}

bool int_process::waitForAsyncEvent(response::ptr resp)
{
   return getResponses().waitFor(resp);
}

bool int_process::waitForAsyncEvent(std::set<response::ptr> resp)
{
   bool has_error = false;
   for (set<response::ptr>::iterator i = resp.begin(); i != resp.end(); i++) {
      bool result = waitForAsyncEvent(*i);
      if (!result)
         has_error = true;
   }

   return !has_error;
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
       * Check status of threads
       **/
      pthrd_printf("Updating state of each process\n");
      syncRunStateRet_t ret;
      ProcPool()->for_each(syncRunState, &ret);

      if (ret.readyProcStoppers.size()) {
         int_process *proc = ret.readyProcStoppers[0];
         Event::ptr ev = proc->removeProcStopper();
         if (ev->triggersCB() &&
             isHandlerThread() && 
             mt()->getThreadMode() == Process::HandlerThreading) 
         {
            pthrd_printf("Handler thread sees postponed callback requiring " 
                         "event '%s', not taking\n",
                         ev->name().c_str());
            notify()->noteEvent();
            goto done;
         }

         pthrd_printf("Handling postponed proc stopper event on %d\n", proc->getPid());
         proc->handlerpool->handleEvent(ev);
         continue;
      }

      /**
       * Check for possible error combinations from syncRunState
       **/
      bool hasAsyncPending = HandlerPool::hasProcAsyncPending();
      if (!ret.hasRunningThread && !hasAsyncPending) {
         if (gotEvent) {
            //We've successfully handled an event, but no longer have any running threads
            pthrd_printf("Returning after handling events, no threads running\n");
            goto done;
         }
         if (isHandlerThread()) {
            //Not an error for the handler thread to get no events.
            pthrd_printf("Returning to handler due to no running threads\n");
            goto done;
         }
         //The user called us with no running threads
         setLastError(err_notrunning, "No running processes or threads to receive events on");
         pthrd_printf("No running threads, returning from waitAndHandleEvents\n");
         error = true;
         goto done;
      }

      /**
       * The handler thread doesn't want to pick up anything that
       * requires a callback, leaving that for the user.  Peek ahead
       * in the mailbox and abort out if we're the handler thread and
       * the next event will require a callback.
       **/
      if (isHandlerThread() && mt()->getThreadMode() == Process::HandlerThreading) {
         Event::ptr ev = mbox()->peek();
         if (ev == Event::ptr()) 
         {
            pthrd_printf("Handler thread returning due to lack of events\n");
            goto done;
         }
         if (ev->triggersCB())
         {
            pthrd_printf("Handler thread sees callback requiring event '%s', "
                         "not taking\n", ev->name().c_str());
            notify()->noteEvent();
            goto done;
         }
      }
      /**
       * Check for new events
       **/
      bool should_block = ((block && !gotEvent) || ret.hasStopPending || 
                           ret.hasSyncRPC || ret.hasClearingBP || ret.hasProcStopRPC || hasAsyncPending);
      pthrd_printf("%s for events (%d %d %d %d %d %d %d)\n", 
                   should_block ? "Blocking" : "Polling",
                   (int) block, (int) gotEvent, (int) ret.hasStopPending, 
                   (int) ret.hasSyncRPC, (int) ret.hasClearingBP, 
                   (int) ret.hasProcStopRPC, (int) hasAsyncPending);
      Event::ptr ev = mbox()->dequeue(should_block);

      if (ev == Event::ptr())
      {
         if (block && gotEvent) {
            pthrd_printf("Returning after handling events\n");
            goto done;
         }
         if (should_block) {
            perr_printf("Blocking wait failed to get events\n");
            setLastError(err_internal, "Blocking wait returned without events");
            error = true;
            goto done;
         }
         if (isHandlerThread()) {
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

      HandlerPool *hpool = ev->getProcess()->llproc()->handlerpool;
      
      ev->getProcess()->llproc()->updateSyncState(ev, false);
      if (ev->procStopper()) {
         /**
          * This event wants the process stopped before it gets handled.
          * We'll start that here, and then postpone the event until it's 
          * stopped.  It's up to the event to continue the process again.
          **/
         int_process *proc = ev->getProcess()->llproc();
         int_threadPool *tp = proc->threadPool();
         tp->desyncInternalState();

         bool result = proc->threadPool()->intStop(false);
         if (!result) {
            pthrd_printf("Failed to stop process for event.\n");
         }
         else {
            proc->addProcStopper(ev);
         }
         continue;
      }
    
      hpool->handleEvent(ev);
      
      if (!ev->getProcess()->llproc())
      {
         //Special case event handling, the process cleaned itself
         // under this event (likely post-exit or post-crash), but was 
         // unable to clean its handlerpool (as we were using it).  
         // Clean this for the process now.
         delete hpool;
      }
   }
  done:
   recurse = false;
   return !error;
}

bool int_process::detach(bool &should_delete)
{
   should_delete = false;
   bool had_error = false;
   bool result;
   int_threadPool *tp = threadPool();
   pthrd_printf("Detach requested on %d\n", getPid());
   while (!tp->allStopped()) {
      pthrd_printf("Stopping process for detach\n");
      tp->intStop(true);
   }
   
   std::set<response::ptr> async_responses;
   while (!mem->breakpoints.empty())
   {
      std::map<Dyninst::Address, installed_breakpoint *>::iterator i = mem->breakpoints.begin();
      result_response::ptr resp = result_response::createResultResponse();
      bool result = i->second->uninstall(this, resp);
      if (!result) {
         perr_printf("Error removing breakpoint at %lx\n", i->first);
         setLastError(err_internal, "Error removing breakpoint before detach\n");
         had_error = true;
      }
      async_responses.insert(resp);
   }

   waitForAsyncEvent(async_responses);
   for (set<response::ptr>::iterator i = async_responses.begin(); i != async_responses.end(); i++) {
      if ((*i)->hasError()) {
         perr_printf("Failed to remove breakpoints\n");
         setLastError(err_internal, "Error removing breakpoint before detach\n");
         had_error = true;
      }
   }
   async_responses.clear();

   ProcPool()->condvar()->lock();

   result = plat_detach();
   if (!result) {
      pthrd_printf("Error performing lowlevel detach\n");
      goto done;
   }

   setState(int_process::exited);
   ProcPool()->rmProcess(this);

   had_error = false;
  done:
   ProcPool()->condvar()->signal();
   ProcPool()->condvar()->unlock();

   if (had_error) 
      return false;
   should_delete = true;
   return true;
}

bool int_process::terminate(bool &needs_sync)
{
   pthrd_printf("Terminate requested on process %d\n", getPid());
   bool had_error = true;
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
   ProcPool()->condvar()->signal();
   ProcPool()->condvar()->unlock();
   return !had_error;
}

int_process::int_process(Dyninst::PID p, std::string e,
                         std::vector<std::string> a,
                         std::map<int,int> f) :
   state(neonatal),
   pid(p),
   executable(e),
   argv(a),
   fds(f),
   arch(Dyninst::Arch_none),
   threadpool(NULL),
   up_proc(Process::ptr()),
   handlerpool(NULL),
   hasCrashSignal(false),
   crashSignal(0),
   hasExitCode(false),
   forceGenerator(false),
   forcedTermination(false),
   exitCode(0),
   mem(NULL),
   continueSig(0)
{
   //Put any object initialization in 'initializeProcess', below.
}

int_process::int_process(Dyninst::PID pid_, int_process *p) :
   state(int_process::running),
   pid(pid_),
   executable(p->executable),
   argv(p->argv),
   arch(p->arch),
   hasCrashSignal(p->hasCrashSignal),
   crashSignal(p->crashSignal),
   hasExitCode(p->hasExitCode),
   forceGenerator(false),
   forcedTermination(false),
   exitCode(p->exitCode),
   exec_mem_cache(exec_mem_cache),
   continueSig(p->continueSig)
{
   Process::ptr hlproc = Process::ptr(new Process());
   mem = new mem_state(*p->mem, this);
   initializeProcess(hlproc);
}

void int_process::initializeProcess(Process::ptr p)
{
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
   ProcPool()->condvar()->lock();
   int_thread *result = NULL;
   for (int_threadPool::iterator i = threadpool->begin(); i != threadpool->end(); i++)
   {
      int_thread *thr = *i;
      if (thr->getHandlerState() == int_thread::stopped) {
         result = thr;
         break;
      }
   }   
   ProcPool()->condvar()->unlock();
   if (result) {
      assert(result->getGeneratorState() == int_thread::stopped);
   }
   return result;
}

bool int_process::readMem(Dyninst::Address remote, mem_response::ptr result)
{
   int_thread *thr = findStoppedThread();
   if (!thr) {
      setLastError(err_notstopped, "A thread must be stopped to read from memory");
      perr_printf("Unable to find a stopped thread for read in process %d\n", getPid());
      return false;
   }

   bool bresult;
   if (!plat_needsAsyncIO()) {
      pthrd_printf("Reading from remote memory %lx to %p, size = %lu on %d/%d\n",
                   remote, result->getBuffer(), (unsigned long) result->getSize(),
                   getPid(), thr->getLWP());

      bresult = plat_readMem(thr, result->getBuffer(), remote, result->getSize());
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
   }
   return bresult;      
}

bool int_process::writeMem(const void *local, Dyninst::Address remote, size_t size, result_response::ptr result)
{
   int_thread *thr = findStoppedThread();
   if (!thr) {
      setLastError(err_notstopped, "A thread must be stopped to write to memory");
      perr_printf("Unable to find a stopped thread for write in process %d\n", getPid());
      return false;
   }

   bool bresult;
   if (!plat_needsAsyncIO()) {
      pthrd_printf("Writing to remote memory %lx from %p, size = %lu on %d/%d\n",
                   remote, local, (unsigned long) size,
                   getPid(), thr->getLWP());

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
   }
   return bresult;
}

Dyninst::Address int_process::mallocExecMemory(unsigned size)
{
   Dyninst::Address max = 0;
   std::map<Dyninst::Address, unsigned>::iterator i;
   for (i = exec_mem_cache.begin(); i != exec_mem_cache.end(); i++) {
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
   pthrd_printf("Process %d is allocating memory of size %lu at 0x%lx\n", getPid(), size, addr);
   int_iRPC::ptr rpc = rpcMgr()->createInfMallocRPC(this, size, use_addr, addr);
   assert(rpc);
   rpcMgr()->postRPCToProc(this, rpc);

   int_thread *thr = rpc->thread();
   bool block = true;
   while (rpc->getState() != int_iRPC::Finished) {
      pthrd_printf("RPC State is %s\n", rpc->getStrState());
      bool result = thr->handleNextPostedIRPC(int_thread::hnp_allow_stop, true);
      if (!result) {
         pthrd_printf("Failed to handleNextPostedIRPC\n");
         return 0;
      }
      if (rpc->getState() == int_iRPC::Finished)
         block = false;

      bool proc_exited;
      result = waitAndHandleForProc(block, this, proc_exited);
      if (proc_exited) {
         perr_printf("Process exited during infMalloc\n");
         setLastError(err_exited, "Process exited during infMalloc\n");
         return 0;
      }
      if (!result && block) {
         pthrd_printf("Error in waitAndHandleEvents\n");
         return 0;
      }
   }
   assert(rpc->getState() == int_iRPC::Finished);

   Dyninst::Address aresult = rpc->infMallocResult();
   pthrd_printf("Inferior malloc returning %lx\n", aresult);
   mem->inf_malloced_memory[aresult] = size;
   return aresult;
}

bool int_process::infFree(Dyninst::Address addr)
{
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
   bool block = true;
   while (rpc->getState() != int_iRPC::Finished) {
      bool result = thr->handleNextPostedIRPC(int_thread::hnp_allow_stop, true);
      if (!result) {
         pthrd_printf("Failed to handleNextPostedIRPC\n");
         return 0;
      }
      if (rpc->getState() == int_iRPC::Finished)
         block = false;

      bool proc_exited;
      result = waitAndHandleForProc(block, this, proc_exited);
      if (proc_exited) {
         perr_printf("Process exited during infFree\n");
         setLastError(err_exited, "Process exited during infFree\n");
         return false;
      }
      if (!result && block) {
         pthrd_printf("Error in waitAndHandleEvents\n");
         return false;
      }
   }
   assert(rpc->getState() == int_iRPC::Finished);

   pthrd_printf("Inferior free returning successfully\n");
   mem->inf_malloced_memory.erase(i);
   return true;
}

void int_process::addProcStopper(Event::ptr ev)
{
   proc_stoppers.push(ev);
}

bool int_process::forceGeneratorBlock() const
{
   return forceGenerator;
}

void int_process::setForceGeneratorBlock(bool b)
{
   forceGenerator = b;
}

Event::ptr int_process::removeProcStopper()
{
   assert(proc_stoppers.size());
   Event::ptr ret = proc_stoppers.front();
   proc_stoppers.pop();
   return ret;
}

bool int_process::hasQueuedProcStoppers() const
{
   return !proc_stoppers.empty();
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
   for (set<int_library *>::iterator i = mem->libs.begin(); 
        i != mem->libs.end(); i++) 
   {
      if (s == (*i)->getName())
         return *i;
   }
   return NULL;
}

size_t int_process::numLibs() const
{
   return mem->libs.size();
}

std::string int_process::getExecutable() const
{
   return executable;
}

bool int_process::isInCallback()
{
   return in_callback;
}

mem_state::ptr int_process::memory() const
{
   return mem;
}

/**
 * The below code involving InternalRPCEvents is to work around an
 * annoyance with Async systems and iRPCs.  When posting an iRPC from
 * handleNextPostedIRPC we may want to create EventRPCInternal events
 * to associate with async responses.  However, this needs to be done
 * at the constructor of the response, which is far removed from the
 * handleNextPostedIRPC function.
 *
 * Rather than passing event pointers around through all the low-level
 * functions, we'll just set AllowInternalRPCEvents for the process while
 * in handleNextPostedIRPC, and the response constructor will check this
 * when building a response.
 *
 * This should be okay since we shouldn't ever call handleNextPostedIRPC
 * recursively or in parallel.
 **/
void int_process::setAllowInternalRPCEvents(int_thread *thr)
{
   if (thr) {
      allowInternalRPCEvents.push(thr);
   }
   else {
      allowInternalRPCEvents.pop();
   }
}

EventRPCInternal::ptr int_process::getInternalRPCEvent()
{
   if (allowInternalRPCEvents.empty())
      return EventRPCInternal::ptr();

   EventRPCInternal::ptr new_ev = EventRPCInternal::ptr(new EventRPCInternal());
#if defined(os_linux)
   //Linux has a mode where it fakes async for testing purposes.  Since the
   // events are fake, they don't have a proper generator filling in this info.
   // Thus we fill it in here.
   new_ev->setProcess(this->proc());
   new_ev->setThread(allowInternalRPCEvents.top()->thread());
   new_ev->setSyncType(Event::async);
#endif               
   handlerPool()->markEventAsyncPending(new_ev);

   return new_ev;
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

void int_process::updateSyncState(Event::ptr ev, bool gen)
{
   EventType etype = ev->getEventType();
   switch (ev->getSyncType()) {
      case Event::async:
         pthrd_printf("Event %s is asynchronous\n", etype.name().c_str());
         break;
      case Event::sync_thread: {
         int_thread *thrd = ev->getThread()->llthrd();
         if (!thrd) {
            pthrd_printf("No thread for sync thread event, assuming thread exited\n");
            return;
         }
         int_thread::State old_state = gen ? thrd->getGeneratorState() : thrd->getHandlerState();
         if (old_state == int_thread::exited) {
            //Silly, linux.  Giving us events on processes that have exited.
            pthrd_printf("Recieved events for exited thread, not chaning thread state\n");
            break;
         }
         pthrd_printf("Event %s is thread synchronous, marking thread %d stopped\n", 
                      etype.name().c_str(), thrd->getLWP());
         assert(old_state == int_thread::running ||
                old_state == int_thread::neonatal_intermediate ||
                thrd->llproc()->plat_needsAsyncIO() || 
                thrd->llproc()->wasForcedTerminated());
         if (old_state == int_thread::errorstate)
            break;
         if (gen)
            thrd->setGeneratorState(int_thread::stopped);
         else
            thrd->setHandlerState(int_thread::stopped);
         break;
      }
      case Event::sync_process: {
         pthrd_printf("Event %s is process synchronous, marking process %d stopped\n", 
                      etype.name().c_str(), getPid());
         int_threadPool *tp = threadPool();
         for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
            int_thread *thrd = *i;
            int_thread::State old_state = gen ? thrd->getGeneratorState() : thrd->getHandlerState();
            if (old_state != int_thread::running &&
                old_state != int_thread::neonatal_intermediate)
               continue;
            if (gen)
               thrd->setGeneratorState(int_thread::stopped);
            else
               thrd->setHandlerState(int_thread::stopped);
         }
         break;
      }
      case Event::unset: {
         assert(0);
      }
   }
}

int_process::~int_process()
{
   pthrd_printf("Deleting int_process at %p\n", this);
   if (up_proc != Process::ptr())
   {
      proc_exitstate *exitstate = new proc_exitstate();
      exitstate->pid = pid;
      exitstate->exited = hasExitCode;
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
}

static
bool stopAllThenContinue(int_threadPool *tp) {
    tp->desyncInternalState();

    // XXX
    // This loop is necessary because there exists a case where the following
    // intStop will leave some threads running due to a proc stop RPC being
    // prepped and run while performing the stop.  
    //
    // While prepping the proc stop RPC, the state is desync'd to run a single
    // thread. When the proc stop RPC completes, the state is restored;
    // however, the restore doesn't move the thread to a stopped state because
    // two levels of desync have occurred. After the RPC is handled,
    // syncRunState continues the process (to match the internal state of
    // affairs). The following restore then fails to resume any threads that
    // were stopped because the process is running and therefore, ptrace
    // commands cannot be run on the process.

    do {
        pthrd_printf("Stopping %d for thread continue\n", tp->proc()->getPid());
        if( !tp->intStop(true) ) {
            perr_printf("Failed to stop all running threads\n");
            setLastError(err_internal, "Failed to stop all running threads\n");
            return false;
        }
    }while( !tp->allStopped() );

    tp->restoreInternalState(true);

    bool anyThreadResumed = false;
    for(int_threadPool::iterator i = tp->begin(); i != tp->end(); ++i) {
        if( (*i)->isResumed() ) {
            anyThreadResumed = true;
            break;
        }
    }

    if( anyThreadResumed ) {
        if( !tp->proc()->plat_contProcess() ) {
            perr_printf("Failed to continue whole process\n");
            setLastError(err_internal, "Failed to continue whole process");
            return false;
        }
    }

    return true;
}

bool int_threadPool::userCont()
{
   return cont(true);
}

bool int_threadPool::intCont()
{
   return cont(false);
}

bool int_threadPool::cont(bool user_cont)
{
   pthrd_printf("%s continuing process %d\n", user_cont ? "User" : "Int", proc()->getPid());

   Dyninst::PID pid = proc()->getPid();
   bool had_error = false;
   bool cont_something = false;

   if( useHybridLWPControl(this) && user_cont && !allStopped() ) {
       // This thread control mode requires that all threads are stopped before
       // continuing a single thread. To peform these stops while still 
       // maintaining the internal state, each thread's internal state is
       // desync'd during the stop and restored after.
       pthrd_printf("Stopping all threads to perform continue\n");

       for(iterator i = begin(); i != end(); ++i) {
           if( !(*i)->setUserState(int_thread::running) ) {
               perr_printf("Failed to change user state\n");
               continue;
           }
       }

       return stopAllThenContinue(this);
   }

   ProcPool()->condvar()->lock();

   for (iterator i = begin(); i != end(); i++) {
      int_thread *thr = *i;
      assert(thr);

      pthrd_printf("Continuing thread %d on process %d\n", thr->getLWP(), pid);
      int_thread::stopcont_ret_t ret = thr->cont(user_cont, true);
      switch (ret) {
         case int_thread::sc_skip:
            break;
         case int_thread::sc_error:
            had_error = true;
            break;
         case int_thread::sc_success:
         case int_thread::sc_success_pending:
            cont_something = true;
            break;
      }
   }

   ProcPool()->condvar()->signal();
   ProcPool()->condvar()->unlock();

   if (!cont_something) {
      perr_printf("Failed to continue exited process %d\n", pid);
      setLastError(err_exited, "Continue attempted on exite/d process\n");
      return false;
   }

   if( useHybridLWPControl() ) {
       if( user_cont ) {
           if( !proc()->plat_contProcess() ) {
               perr_printf("Failed to continue whole process\n");
               setLastError(err_internal, "Failed to continue whole process");
               return false;
           }
       }
   }

   return !had_error;
}

bool int_thread::userCont()
{
   return cont(true);
}

bool int_thread::intCont()
{
   return cont(false);
}

bool int_thread::cont(bool user_cont)
{
   pthrd_printf("%s continuing single thread %d/%d\n", user_cont ? "User" : "Int",
                llproc()->getPid(), getLWP());

   bool completed_rpc = true;
   bool result = rpcMgr()->handleThreadContinue(this, user_cont, completed_rpc);
   if (!result) {
      pthrd_printf("Error handling IRPC during continue\n");
      return false;
   }
   if (!completed_rpc && !hasPendingStop()) {
      /**
       * A thread has an RPC being prepped and has been asked to continue.
       * We'll postpone this continue until the RPC is prepped.  This should
       * only happen on an async system (BlueGene), in which case we'll
       * generate RPCInternal events to move the system along until everything is complete.
       *
       * We'll still allow a continue on a thread with a pending stop, since the thread
       * will move to a proper stop state before actually running.
       **/
      pthrd_printf("Unable to complete post of RPC, postponing continue\n");
      if (user_cont) {
         bool result = setUserState(int_thread::running);
         if (!result) {
            setLastError(err_exited, "Attempted thread continue on exited thread\n");
            perr_printf("Failed to continue thread %d/%d--bad state\n", llproc()->getPid(), getLWP());
            return false;
         }
      }
      if (!postponed_continue) {
         desyncInternalState();
         postponed_continue = true;
      }
      return true;
   }

   if ( int_process::getThreadControlMode() == int_process::NoLWPControl ) {
      pthrd_printf("%s continuing entire process %d on thread operation on %d\n",
                   user_cont ? "User" : "Int", llproc()->getPid(), getLWP());
      if (user_cont) {
         return llproc()->threadPool()->userCont();
      }
      else {
         return llproc()->threadPool()->intCont();
      }
   }

   if( useHybridLWPControl(llproc()) && user_cont && 
       !llproc()->threadPool()->allStopped() )
   {
       // This thread control mode requires that all threads are stopped before
       // continuing a single thread. To peform these stops while still 
       // maintaining the internal state, each thread's internal state is
       // desync'd during the stop and restored after.
       pthrd_printf("Stopping all threads to perform continue\n");

       if( !setUserState(int_thread::running) ) {
           perr_printf("Failed to change user state\n");
           setLastError(err_internal, "Failed to change user state");
       }

       // In the process of readying a new RPC in handleThreadContinue above, 
       // the thread could already be set running, in which case the work
       // has already been done
       if( getInternalState() == running ) {
           pthrd_printf("Thread %d/%d already running, not stopping threads\n",
                   llproc()->getPid(), lwp);
           return true;
       }

       return stopAllThenContinue(llproc()->threadPool());
   }

   stopcont_ret_t ret = cont(user_cont, false);

   if (ret == sc_skip) {
      perr_printf("Attempted to continue exited thread\n");
      setLastError(err_exited, "Attempted thread continue on exited thread\n");
      return false;
   }
   if (ret == sc_error) {
      if (user_cont) {
         //The internal state is running, so there was an internal error during continue, but
         // the user state was stopped.  We won't treat this as a user error and instead
         // just change the user state.
         pthrd_printf("Ignoring previous error on %d/%d\n", llproc()->getPid(), getLWP());
      }
      else {
         pthrd_printf("Error continuing thread %d/%d\n", llproc()->getPid(), getLWP());
         return false;
      }
   }

   if (user_cont) 
   {
      bool result = setUserState(int_thread::running);
      if (!result) {
         setLastError(err_exited, "Attempted thread continue on exited thread\n");
         perr_printf("Failed to continue thread %d/%d--bad state\n", llproc()->getPid(), getLWP());
         return false;
      }
   }

   if( useHybridLWPControl() ) {
        if( user_cont && ret != sc_error ) {
            if( !llproc()->plat_contProcess() ) {
               perr_printf("Failed to continue whole process\n");
               setLastError(err_internal, "Failed to continue whole process");
               return false;
            }
        }
   }

   return true;
}

int_thread::stopcont_ret_t int_thread::cont(bool user_cont, bool have_proc_lock)
{
   Dyninst::PID pid = proc()->getPid();

   pthrd_printf("Top level %s continue for %d/%d\n",
                user_cont ? "user" : "int", pid, lwp);

   if (getHandlerState() == errorstate) {
      pthrd_printf("thread %d on process %d in error state\n", getLWP(), pid);
      return sc_skip;
   }
   if (getHandlerState() == exited) {
      pthrd_printf("thread %d on process %d already exited\n", getLWP(), pid);
      return sc_skip;
   }

   if (user_cont) {
      setUserState(running);
      /*if (num_locked_stops) {
         pthrd_printf("Thread is desync'd.  Not doing low level continue\n");
         return sc_success;
         }*/
   }

   if (getHandlerState() != stopped) {
      perr_printf("Error. continue attempted on running thread %d/%d\n", pid, lwp);
      setLastError(err_notstopped, "Continue attempted on running thread\n");
      return sc_error;
   }

   if (!have_proc_lock) {
      ProcPool()->condvar()->lock();
   }

   regpool_lock.lock();
   cached_regpool.regs.clear();
   cached_regpool.full = false;
   regpool_lock.unlock();

   bool result = plat_cont();
   if (result) {
      if( !useHybridLWPControl() ) {
          setInternalState(running);
          setHandlerState(running);
          setGeneratorState(running);
      }else{
          setResumed(true);
      }
   }

   if (!have_proc_lock) {
      ProcPool()->condvar()->signal();
      ProcPool()->condvar()->unlock();
   }

   if (!result) {
      pthrd_printf("Could not resume debugee %d, thread %d\n", pid, lwp);
      return sc_error;
   }

   return sc_success;
}

bool int_threadPool::userStop()
{
   return stop(true, true);
}

bool int_threadPool::intStop(bool sync)
{
   return stop(false, sync);
}

bool int_threadPool::stop(bool user_stop, bool sync)
{
   bool stopped_something = false;
   bool had_error = false;
   bool needs_sync = false;

   unsigned numThreadsBefore = size();
   bool finished = true;

   do{
       for (iterator i = begin(); i != end(); i++) {
          int_thread *thr = *i;

          pthrd_printf("Process %d performing %s stop on thread %d\n", proc()->getPid(),
                       user_stop ? "user" : "int",  thr->getLWP());
          int_thread::stopcont_ret_t ret = thr->stop(user_stop);
          switch (ret) {
             case int_thread::sc_skip:
                pthrd_printf("int_thread::stop on %d/%d returned sc_skip\n", 
                             proc()->getPid(), thr->getLWP());
                break;
             case int_thread::sc_error:            
                pthrd_printf("int_thread::stop on %d/%d returned sc_error\n", 
                             proc()->getPid(), thr->getLWP());
                if (getLastError() == err_noproc) 
                   pthrd_printf("int_thread::stop thread exit on %d/%d, skipping stop\n",
                                proc()->getPid(), thr->getLWP());
                else
                   had_error = true;
                break;
             case int_thread::sc_success_pending:
                pthrd_printf("int_thread::stop on %d/%d return sc_success_pending\n",
                             proc()->getPid(), thr->getLWP());
                stopped_something = true;
                needs_sync = true;
                break;
             case int_thread::sc_success:
                pthrd_printf("int_thread::stop on %d/%d returned sc_success\n", 
                             proc()->getPid(), thr->getLWP());
                stopped_something = true;
                break;
          }

          // Need to handle the case where new threads are created during a stop
          // If new threads are created, this iteration is invalid
          if( numThreadsBefore != size() ) {
              numThreadsBefore = size();
              finished = false;
              break;
          }else{
              finished = true;
          }
       }
   }while( !finished );

   if (had_error) {
      pthrd_printf("Error while stopping threads on %d\n", proc()->getPid());
      setLastError(err_internal, "Could not stop process\n");
      return false;
   }
   if (!stopped_something) {
      perr_printf("No threads can be stopped on %d\n", proc()->getPid());
      setLastError(err_notrunning, "Attempt to stop a process that isn't running\n");
      return false;
   }

   if (needs_sync && sync)
   {
      bool proc_exited;
      bool result = int_process::waitAndHandleForProc(true, proc(), proc_exited);
      if (proc_exited) {
         pthrd_printf("Process exited during stop\n");
         setLastError(err_exited, "Process exited during stop\n");
         return false;
      }
      if (!result) {
         perr_printf("Error waiting for events after stop on %d\n", proc()->getPid());
         return false;
      }
   }

   return true;
}

int_thread::stopcont_ret_t int_thread::stop(bool user_stop)
{
   Dyninst::PID pid = proc()->getPid();
   pthrd_printf("Top level %s thread pause for %d/%d\n", 
                user_stop ? "user" : "int", pid, lwp);

   if (getHandlerState() == errorstate) {
      pthrd_printf("thread %d on process %d in error state\n", getLWP(), pid);
      return sc_skip;
   }
   if (getHandlerState() == exited) {
      pthrd_printf("thread %d on process %d already exited\n", getLWP(), pid);
      return sc_skip;
   }

   if (pending_stop) {
      pthrd_printf("thread %d has in-progress stop on process %d\n", getLWP(), pid);
      return sc_success_pending;
   }
   if (getHandlerState() == stopped) {         
      pthrd_printf("thread %d on process %d is already handler stopped, leaving\n", 
                   getLWP(), pid);
      if (user_stop)
         setUserState(stopped);
      setInternalState(stopped);

      return sc_success;
   }
   if (getInternalState() == stopped) {
      pthrd_printf("thread %d is already stopped on process %d\n", getLWP(), pid);
      if (user_stop)
         setUserState(stopped);
      return sc_success;
   }
   if (getHandlerState() != running)
   {
      perr_printf("Attempt to stop thread %d/%d in bad state %d\n", 
                  pid, lwp, getHandlerState());
      setLastError(err_internal, "Bad state during thread stop attempt\n");
      return sc_error;
   }

   assert(!pending_stop);
   pending_stop = true;
   if (user_stop) {
      assert(!pending_user_stop);
      pending_user_stop = true;
   }

   bool result = plat_stop();
   if (!result) {
      pthrd_printf("Could not pause debuggee %d, thr %d\n", pid, lwp);
      pending_stop = false;
      return sc_error;
   }

   if (pending_stop)
      return sc_success_pending;
   else
      return sc_success;
}

bool int_thread::stop(bool user_stop, bool sync)
{
   if ( int_process::getThreadControlMode() == int_process::NoLWPControl ) {
      if (user_stop) {
         pthrd_printf("User stopping entire process %d on thread operation on %d\n",
                      llproc()->getPid(), getLWP());
         return llproc()->threadPool()->userStop();
      }
      else {
         pthrd_printf("Int stopping entire process %d on thread operation on %d\n",
                      llproc()->getPid(), getLWP());
         return llproc()->threadPool()->intStop();
      }
   }

   pthrd_printf("%s stopping single thread %d/%d\n", user_stop ? "User" : "Int",
                llproc()->getPid(), getLWP());

   stopcont_ret_t ret = stop(user_stop);
   if (ret == sc_skip) {
      perr_printf("Thread %d/%d was not in a stoppable state\n", 
                  llproc()->getPid(), getLWP());
      setLastError(err_notrunning, "Attempt to stop a thread that isn't running\n");
      return false;
   }
   if (ret == sc_error) {
      pthrd_printf("Thread %d/%d returned error during stop\n",
                   llproc()->getPid(), getLWP());
      return false;
   }
   if (ret == sc_success) {
      pthrd_printf("Thread %d/%d successfully stopped\n",
                   llproc()->getPid(), getLWP());
      return true;
   }
   assert(ret == sc_success_pending);
   if (!sync) {
      pthrd_printf("Thread %d/%d successfully stopped, but not sync'd\n",
                   llproc()->getPid(), getLWP());
      return true;
   }

   bool proc_exited;
   bool result = int_process::waitAndHandleForProc(true, llproc(), proc_exited);
   if (proc_exited) {
      pthrd_printf("Process exited during thread stop\n");
      setLastError(err_exited, "Process exited during stop\n");
      return false;
   }
   if (!result) {
      perr_printf("Error waiting for events after stop on %d\n", getLWP());
      return false;
   }
   return true;
}

bool int_thread::userStop()
{
   return stop(true, true);
}

bool int_thread::intStop(bool sync)
{
   return stop(false, sync);
}

void int_thread::setPendingUserStop(bool b)
{
   pending_user_stop = b;
}

bool int_thread::hasPendingUserStop() const
{
   return pending_user_stop;
}

void int_thread::setPendingStop(bool b)
{
   pending_stop = b;
}

bool int_thread::hasPendingStop() const
{
   return pending_stop;
}

void int_thread::setResumed(bool b)
{
    resumed = b;
}

bool int_thread::isResumed() const
{
    return resumed;
}

Process::ptr int_thread::proc() const
{
   return proc_->proc();
}

int_process *int_thread::llproc() const
{
   return proc_;
}

Dyninst::THR_ID int_thread::getTid() const
{
   return tid;
}

Dyninst::LWP int_thread::getLWP() const
{
   return lwp;
}

int_thread::State int_thread::getHandlerState() const
{
   return handler_state;
}

int_thread::State int_thread::getUserState() const
{
   return user_state;
}

int_thread::State int_thread::getGeneratorState() const
{
   return generator_state;
}

int_thread::State int_thread::getInternalState() const
{
   return internal_state;
}

const char *int_thread::stateStr(int_thread::State s)
{
   switch (s) {
      case neonatal: return "neonatal";
      case neonatal_intermediate: return "neonatal_intermediate";
      case running: return "running";
      case stopped: return "stopped";
      case exited: return "exited";
      case errorstate: return "errorstate";
   }
   assert(0);
   return NULL;
}

bool int_thread::setAnyState(int_thread::State *from, int_thread::State to)
{
   const char *s = NULL;
   if (from == &handler_state) {
      s = "handler state";
   }
   else if (from == &user_state) {
      s = "user state";
   }
   else if (from == &generator_state) {
      s = "generator state";
   }
   else if (from == &internal_state) {
      s = "internal state";
   }
   assert(s);

   if (*from == to) {
      pthrd_printf("Leaving %s for %d in state %s\n", s, lwp, stateStr(to));
      return true;
   }
   if (to == errorstate) {
      perr_printf("Setting %s for %d from %s to errorstate\n", 
                  s, lwp, stateStr(*from));
      *from = to;
      return true;
   }
   if (*from == errorstate) {
      perr_printf("Attempted %s reversion for %d from errorstate to %s\n", 
                  s, lwp, stateStr(to));
      return false;
   }
   if (*from == exited) {
      perr_printf("Attempted %s reversion for %d from exited to %s\n", 
                  s, lwp, stateStr(to));
      return false;
   }
   if (to == neonatal && *from != neonatal) {
      perr_printf("Attempted %s reversion for %d from %s to neonatal\n", 
                  s, lwp, stateStr(*from));
      return false;
   }

   pthrd_printf("Changing %s for %d/%d from %s to %s\n", s, llproc()->getPid(), lwp, 
                stateStr(*from), stateStr(to));
   *from = to;

   if (internal_state == stopped)  assert(handler_state == stopped || handler_state == exited );
   if (handler_state == stopped)   assert(generator_state == stopped || generator_state == exited);
   if (generator_state == running) assert(handler_state == running);
   if (handler_state == running)   assert(internal_state == running);
   return true;
}

bool int_thread::setHandlerState(int_thread::State s)
{
   return setAnyState(&handler_state, s);
}

bool int_thread::setUserState(int_thread::State s)
{
   return setAnyState(&user_state, s);
}

bool int_thread::setGeneratorState(int_thread::State s)
{
   return setAnyState(&generator_state, s);
}

bool int_thread::setInternalState(int_thread::State s)
{
   return setAnyState(&internal_state, s);
}

void int_thread::desyncInternalState()
{
   pthrd_printf("Thread %d/%d is desyncing int from user state %d\n",
                llproc()->getPid(), getLWP(), num_locked_stops+1);
   num_locked_stops++;
}

void int_thread::restoreInternalState(bool sync)
{
   pthrd_printf("Thread %d/%d is restoring int to user state, %d\n",
                llproc()->getPid(), getLWP(), num_locked_stops-1);
   assert(num_locked_stops > 0);
   num_locked_stops--;
   if (num_locked_stops > 0) 
      return;
   
   pthrd_printf("Changing internal state, %s, to user state, %s.\n",
                int_thread::stateStr(internal_state), int_thread::stateStr(user_state));

   if (internal_state == user_state)
   {
      return;
   }
   else if (internal_state == int_thread::exited ||
            user_state == int_thread::exited) 
   {
      setInternalState(int_thread::exited);
   }
   else if (internal_state == int_thread::stopped &&
            user_state == int_thread::running)
   {
      bool result = intCont();
      if (!result) {
         perr_printf("Error continuing internal process %d/%d when resyncing\n",
                     llproc()->getPid(), getLWP());
         return;
      }
   }
   else if (internal_state == int_thread::running &&
            user_state == int_thread::stopped) 
   {
      bool result = intStop(sync);
      if (!result) {
         perr_printf("Error stopping internal process %d/%d when resyncing\n",
                     llproc()->getPid(), getLWP());
         return;
      }
   }
   else {
      setInternalState(user_state);
   }
}

void int_thread::setContSignal(int sig)
{
   continueSig_ = sig;
}

int int_thread::getContSignal() {
    return continueSig_;
}

int_thread::int_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   tid(t),
   lwp(l),
   proc_(p),
   continueSig_(0),
   handler_state(neonatal),
   user_state(neonatal),
   generator_state(neonatal),
   internal_state(neonatal),
   regpool_lock(true),
   sync_rpc_count(0),
   pending_user_stop(false),
   pending_stop(false),
   resumed(false),
   num_locked_stops(0),
   user_single_step(false),
   single_step(false),
   postponed_continue(false),
   clearing_breakpoint(false)
{
   Thread::ptr new_thr(new Thread());

   new_thr->llthread_ = this;
   up_thread = new_thr;
}

int_thread::~int_thread()
{
   assert(!up_thread->exitstate_);

   up_thread->exitstate_ = new thread_exitstate();
   up_thread->exitstate_->lwp = lwp;
   up_thread->exitstate_->thr_id = tid;
   up_thread->llthread_ = NULL;
}

int_thread *int_thread::createThread(int_process *proc, 
                                     Dyninst::THR_ID thr_id, 
                                     Dyninst::LWP lwp_id,
                                     bool initial_thrd)
{
   int_thread *newthr = createThreadPlat(proc, thr_id, lwp_id, initial_thrd);
   pthrd_printf("Creating %s thread %d/%d, thr_id = %d\n", 
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
   newthr->setUserState(neonatal_intermediate);
   newthr->setInternalState(neonatal_intermediate);
   newthr->setHandlerState(neonatal_intermediate);
   newthr->setGeneratorState(neonatal_intermediate);

   return newthr;
}

bool int_thread::hasPostponedContinue() const
{
   return postponed_continue;
}

void int_thread::setPostponedContinue(bool b)
{
   postponed_continue = b;
}

Thread::ptr int_thread::thread()
{
   return up_thread;
}

bool int_thread::getAllRegisters(allreg_response::ptr response)
{
   bool result = false;
   response->setThread(this);

   pthrd_printf("Reading registers for thread %d\n", getLWP());
   regpool_lock.lock();
   if (cached_regpool.full) {
      *response->getRegPool() = cached_regpool;
      response->getRegPool()->thread = this;
      response->markReady();
      pthrd_printf("Returning cached register set\n");
      result = true;
      goto done;
   }

   if (!llproc()->plat_needsAsyncIO())
   {
      pthrd_printf("plat_getAllRegisters on %d/%d\n", llproc()->getPid(), getLWP());
      result = plat_getAllRegisters(cached_regpool);
      if (!result) {
         pthrd_printf("plat_getAllRegisters returned error on %d\n", getLWP());
         response->markError();
         goto done;
      }
      cached_regpool.full = true;
      *(response->getRegPool()) = cached_regpool;
      response->getRegPool()->thread = this;
      response->markReady();
      pthrd_printf("Successfully retrieved all registers for %d\n", getLWP());
   }
   else
   {
      pthrd_printf("Async plat_getAllRegisters on %d/%d\n", llproc()->getPid(), 
                   getLWP());
      getResponses().lock();
      result = plat_getAllRegistersAsync(response);
      if (result) {
         getResponses().addResponse(response, llproc());
      }
      getResponses().unlock();
      if (!result) {
         pthrd_printf("plat_getAllRegistersAsync returned error on %d\n", getLWP());
         goto done;
      }
   }

   result = true;
  done:
   regpool_lock.unlock();
   return result;
}

bool int_thread::setAllRegisters(int_registerPool &pool, result_response::ptr response)
{
   assert(getHandlerState() == int_thread::stopped);
   assert(getGeneratorState() == int_thread::stopped);
   regpool_lock.lock();

   bool ret_result = false;
   if (!llproc()->plat_needsAsyncIO()) {
      pthrd_printf("Setting registers for thread %d\n", getLWP());
      bool result = plat_setAllRegisters(pool);
      response->setResponse(result);
      if (!result) {
         pthrd_printf("plat_setAllRegisters returned error on %d\n", getLWP());
         goto done;
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
      if (!result) {
         pthrd_printf("Error async setting registers on %d\n", getLWP());
         goto done;
      }
   }

   cached_regpool = pool;
   cached_regpool.full = true;

   ret_result = true;
  done:
   regpool_lock.unlock();
   return ret_result;
}

bool int_thread::getRegister(Dyninst::MachRegister reg, reg_response::ptr response)
{
   bool ret_result = false;
   pthrd_printf("Get register value for thread %d, register %s\n", lwp, reg.name());
   response->setRegThread(reg, this);

   if (!llproc()->plat_individualRegAccess())
   {
      pthrd_printf("Platform does not support individual register access, " 
                   "getting everything\n");
      assert(!llproc()->plat_needsAsyncIO());

      int_registerPool pool;
      allreg_response::ptr allreg_resp = allreg_response::createAllRegResponse(&pool);
      bool result = getAllRegisters(allreg_resp);
      assert(allreg_resp->isReady());
      if (!result || allreg_resp->hasError()) {
         pthrd_printf("Unable to access full register set\n");
         return false;
      }
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
         pthrd_printf("Error reading register value for %s on %d\n", reg.name(), lwp);
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
      if (!result) {
         pthrd_printf("Error getting async register for thread %d\n", getLWP());
         goto done;
      }
      ret_result = true;
      goto done;
   }

   pthrd_printf("Returning register value %lx for register %s on %d\n", 
                response->getResult(), reg.name(), lwp);

   ret_result = true;
  done:
   regpool_lock.unlock();
   return ret_result;
}

bool int_thread::setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val,
                             result_response::ptr response)
{
   assert(getHandlerState() == int_thread::stopped);
   assert(getGeneratorState() == int_thread::stopped);
   bool ret_result = false;
   
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

   pthrd_printf("Setting register %s for thread %d to %lx\n", reg.name(), getLWP(), val);
   regpool_lock.lock();

   MachRegister base_register = reg.getBaseRegister();
   if (!llproc()->plat_needsAsyncIO())
   {   
      bool result = plat_setRegister(base_register, val);
      response->setResponse(result);
      if (!result) {
         pthrd_printf("Error setting register %s\n", base_register.name());
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
}

int_iRPC::ptr int_thread::runningRPC() const
{
   return running_rpc;
}

void int_thread::clearRunningRPC()
{
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

bool int_thread::runningInternalRPC() const
{
   if (runningRPC() && runningRPC()->isInternalRPC()) {
      return true;
   }
   if (posted_rpcs.size() && posted_rpcs.front()->isInternalRPC()) {
      return true;
   }
   return false;
}

void int_thread::incSyncRPCCount()
{
   sync_rpc_count++;
}

void int_thread::decSyncRPCCount()
{
   assert(sync_rpc_count > 0);
   sync_rpc_count--;
}

bool int_thread::hasSyncRPC()
{
   return (sync_rpc_count != 0);
}

int_iRPC::ptr int_thread::nextPostedIRPC() const
{
   if (!posted_rpcs.size())
      return int_iRPC::ptr();
   return posted_rpcs.front();
}

bool int_thread::handleNextPostedIRPC(hnp_stop_t allow_stop, bool is_sync)
{
   int_iRPC::ptr posted_rpc = nextPostedIRPC();
   if (!posted_rpc || runningRPC() )
      return true;

   if (!is_sync) {
      llproc()->setAllowInternalRPCEvents(this);
   }

   bool ret_result = false;
   pthrd_printf("Handling next postd irpc %lu on %d/%d of type %s in state %s\n",
                posted_rpc->id(), llproc()->getPid(), getLWP(), 
                posted_rpc->getStrType(), posted_rpc->getStrState());
   
   if (posted_rpc->getState() == int_iRPC::Posted) {
      bool error = false;
      pthrd_printf("Prepping next rpc to run on %d/%d\n", llproc()->getPid(), getLWP());
      bool result = rpcMgr()->prepNextRPC(this, allow_stop == hnp_allow_stop, error);
      if (!result && error) {
         perr_printf("Failed to prep RPC\n");
         goto done;
      }
   }
   if (posted_rpc->getState() == int_iRPC::Prepping) {
      pthrd_printf("Checking if rpc is prepped on %d/%d\n", llproc()->getPid(), getLWP());
      if (!posted_rpc->isRPCPrepped())
         pthrd_printf("RPC not yet prepped\n");
   }

   if (posted_rpc->getState() == int_iRPC::Prepped) {
      pthrd_printf("Saving RPC state on %d/%d\n", llproc()->getPid(), getLWP());
      posted_rpc->saveRPCState();
   }

   posted_rpc->syncAsyncResponses(is_sync);
   
   if (posted_rpc->getState() == int_iRPC::Saving) {
      pthrd_printf("Checking if RPC on %d/%d has finished save\n", 
                   llproc()->getPid(), getLWP());
      if (!posted_rpc->checkRPCFinishedSave()) {
         pthrd_printf("RPC has not yet finished save\n");
      }
   }

   posted_rpc->syncAsyncResponses(is_sync);

   if (posted_rpc->getState() == int_iRPC::Saved) {
      pthrd_printf("Writing RPC on %d/%d\n", llproc()->getPid(), getLWP());
      posted_rpc->writeToProc();
   }

   posted_rpc->syncAsyncResponses(is_sync);

   if (posted_rpc->getState() == int_iRPC::Writing) {
      pthrd_printf("Checking if RPC on %d/%d has finished write\n",
                   llproc()->getPid(), getLWP());
      if (!posted_rpc->checkRPCFinishedWrite()) {
         pthrd_printf("RPC has not yet finished write\n");
      }
   }

   posted_rpc->syncAsyncResponses(is_sync);

   if (posted_rpc->getState() == int_iRPC::Ready)
   {
      pthrd_printf("Readying next RPC on %d/%d\n", llproc()->getPid(), getLWP());
      posted_rpc->runIRPC(allow_stop == hnp_allow_stop);
   }

   ret_result = true;
  done:
   if (!is_sync) {
      llproc()->setAllowInternalRPCEvents(NULL);
   }

   return ret_result;
}

int_iRPC::ptr int_thread::hasRunningProcStopperRPC() const
{
   int_iRPC::ptr running = runningRPC();
   if (running && running->isProcStopRPC()) {
      return running;
   }
   int_iRPC::ptr nextposted = nextPostedIRPC();
   if (!running && nextposted && nextposted->isProcStopRPC() && 
       nextposted->getState() != int_iRPC::Posted) 
   {
      return nextposted;
   }
   return int_iRPC::ptr();
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
   clearing_breakpoint = bp;
}

installed_breakpoint *int_thread::isClearingBreakpoint()
{
   return clearing_breakpoint;
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
   return initial_thread;
}

bool int_threadPool::allStopped()
{
   for (iterator i = begin(); i != end(); i++) {
      if ((*i)->getInternalState() == int_thread::running)
         return false;
   }
   return true;
}

void int_threadPool::addThread(int_thread *thrd)
{
   Dyninst::LWP lwp = thrd->getLWP();
   std::map<Dyninst::LWP, int_thread *>::iterator i = thrds_by_lwp.find(lwp);
   assert (i == thrds_by_lwp.end());
   thrds_by_lwp[lwp] = thrd;
   threads.push_back(thrd);
   hl_threads.push_back(thrd->thread());
}

void int_threadPool::rmThread(int_thread *thrd)
{
   assert(thrd != initial_thread);
   Dyninst::LWP lwp = thrd->getLWP();
   std::map<Dyninst::LWP, int_thread *>::iterator i = thrds_by_lwp.find(lwp);
   assert (i != thrds_by_lwp.end());
   thrds_by_lwp.erase(i);

   for (unsigned i=0; i<threads.size(); i++) {
      if (threads[i] != thrd)
         continue;
      threads[i] = threads[threads.size()-1];
      threads.pop_back();
      hl_threads[i] = hl_threads[hl_threads.size()-1];
      hl_threads.pop_back();
   }
}

void int_threadPool::clear()
{
   threads.clear();
   hl_threads.clear();
   thrds_by_lwp.clear();
   initial_thread = NULL;
}

void int_threadPool::desyncInternalState()
{
   for (iterator i = begin(); i != end(); i++) {
      (*i)->desyncInternalState();
   }
}

void int_threadPool::restoreInternalState(bool sync)
{
   for (iterator i = begin(); i != end(); i++) {
      (*i)->restoreInternalState(false);
   }
   if (sync)
      int_process::waitAndHandleEvents(false);
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

int_threadPool::int_threadPool(int_process *p) :
   proc_(p)
{
   up_pool = new ThreadPool();
   up_pool->threadpool = this;
}

int_threadPool::~int_threadPool()
{
   assert(up_pool);
   delete up_pool;

   for (vector<int_thread*>::iterator i = threads.begin(); i != threads.end(); i++)
   {
      delete *i;
   }
}

int_breakpoint::int_breakpoint(Breakpoint::ptr up) :
   up_bp(up),
   to(0x0),
   isCtrlTransfer_(false),
   data(false)
{
}

int_breakpoint::int_breakpoint(Dyninst::Address to_, Breakpoint::ptr up) :
   up_bp(up),
   to(to_),
   isCtrlTransfer_(true),
   data(false)
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

installed_breakpoint::installed_breakpoint(mem_state::ptr memory_, Address addr_) :
   memory(memory_),
   buffer_size(0),
   prepped(false),
   installed(false),
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
   char bp_insn[BP_BUFFER_SIZE];
   proc->plat_breakpointBytes(bp_insn);
   return proc->writeMem(bp_insn, addr, buffer_size, write_response);
}

bool installed_breakpoint::saveBreakpointData(int_process *proc, mem_response::ptr read_response)
{
   if (buffer_size != 0) {
      return true;
   }

   buffer_size = proc->plat_breakpointSize();
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
   bool had_failure = false;
   if (proc->getState() != int_process::exited)
   {
      bool result = proc->writeMem(&buffer, addr, buffer_size, async_resp);
      if (!result) {
         pthrd_printf("Failed to remove breakpoint at %lx from process %d\n", 
                      addr, proc->getPid());
         had_failure = true;
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

   return !had_failure;
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

bool installed_breakpoint::addBreakpoint(int_breakpoint *bp)
{
   if (bp->isCtrlTransfer()) {
      for (set<int_breakpoint *>::iterator i = bps.begin(); i != bps.end(); i++)
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
   
   return true;
}

Dyninst::Address installed_breakpoint::getAddr() const
{
   return addr;
}

int_library::int_library(std::string n, Dyninst::Address load_addr) :
   name(n),
   load_address(load_addr),
   data_load_address(0),
   has_data_load(false),
   marked(false)
{
   up_lib = new Library();
   up_lib->lib = this;
}

int_library::int_library(std::string n, Dyninst::Address load_addr, Dyninst::Address data_load_addr) :
   name(n),
   load_address(load_addr),
   data_load_address(data_load_addr),
   has_data_load(true),
   marked(false)
{
   up_lib = new Library();
   up_lib->lib = this;
}

int_library::int_library(int_library *l) :
   name(l->name),
   load_address(l->load_address),
   data_load_address(l->data_load_address),
   has_data_load(l->has_data_load),
   marked(l->marked)
{
   up_lib = new Library();
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

void int_library::setMark(bool b)
{
   marked = b;
}

bool int_library::isMarked() const
{
   return marked;
}

Library::ptr int_library::getUpPtr() const
{
   return up_lib;
}

mem_state::mem_state(int_process *proc)
{
   procs.insert(proc);
}

mem_state::mem_state(mem_state &m, int_process *p)
{
   pthrd_printf("Copying mem_state to new process %d\n", p->getPid());
   procs.insert(p);
   set<int_library *>::iterator i;
   for (i = m.libs.begin(); i != m.libs.end(); i++)
   {
      int_library *orig_lib = *i;
      int_library *new_lib = new int_library(orig_lib);
      libs.insert(new_lib);
   }
   map<Dyninst::Address, installed_breakpoint *>::iterator j;
   for (j = m.breakpoints.begin(); j != m.breakpoints.end(); j++)
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
   for (i = libs.begin(); i != libs.end(); i++)
   {
      int_library *lib = *i;
      delete lib;
   }
   libs.clear();

   map<Dyninst::Address, installed_breakpoint *>::iterator j;
   for (j = breakpoints.begin(); j != breakpoints.end(); j++)
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
   pipe_in(-1),
   pipe_out(-1),
   pipe_count(0),
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
   assert(isHandlerThread());
   assert(events_noted == 0);
   writeToPipe();
   events_noted++;
   pthrd_printf("noteEvent - %d\n", events_noted);
   set<EventNotify::notify_cb_t>::iterator i;
   for (i = cbs.begin(); i != cbs.end(); i++) {
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
   assert(events_noted == 0);
   readFromPipe();
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

int int_notify::getPipeIn()
{
   if (pipe_in == -1)
      createPipe();
   return pipe_in;
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

Thread::ptr RegisterPool::getThread() const
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


int_registerPool::int_registerPool() :
   full(false),
   thread(NULL)
{
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
   if (lib) {
      delete lib;
      lib = NULL;
   }
}

std::string Library::getName() const
{
   return lib->getName();
}

Dyninst::Address Library::getLoadAddress() const
{
   return lib->getAddr();
}

Dyninst::Address Library::getDataLoadAddress() const
{
   return lib->getDataAddr();
}

LibraryPool::LibraryPool()
{
}

LibraryPool::~LibraryPool()
{
}

size_t LibraryPool::size() const
{
   return proc->numLibs();
}

Library::ptr LibraryPool::getLibraryByName(std::string s)
{
   int_library *int_lib = proc->getLibraryByName(s);
   if (!int_lib)
      return NULL;
   return int_lib->up_lib;
}

Library::ptr LibraryPool::getLibraryByName(std::string s) const
{
   int_library *int_lib = proc->getLibraryByName(s);
   if (!int_lib)
      return NULL;
   return int_lib->up_lib;
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
                                    const std::map<int,int> &fds)
{
   MTLock lock_this_func(MTLock::allow_init, MTLock::deliver_callbacks);

   pthrd_printf("User asked to launch executable %s\n", executable.c_str());
   if (int_process::isInCB()) {
      perr_printf("User attempted call on process create while in CB, erroring.");
      setLastError(err_incallback, "Cannot createProcess from callback\n");
      return Process::ptr();
   }

   Process::ptr newproc(new Process());
   int_process *llproc = int_process::createProcess(executable, argv, fds);
   llproc->initializeProcess(newproc);
   
   bool result = llproc->create();
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
   Process::ptr newproc(new Process());
   int_process *llproc = int_process::createProcess(pid, executable);
   llproc->initializeProcess(newproc);

   bool result = llproc->attach();
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

   pthrd_printf("User continuing entire process %d\n", getPid());
   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      setLastError(err_incallback, "Cannot continueProc from callback\n");
      return false;
   }

   return llproc_->threadPool()->userCont();
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

bool Process::stopProc()
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   if (!llproc_) {
      perr_printf("stopProc on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

   pthrd_printf("User stopping entire process %d\n", getPid());
   if (int_process::isInCB()) {
      perr_printf("User attempted call on process while in CB, erroring.");
      setLastError(err_incallback, "Cannot continueProc from callback\n");
      return false;
   }

   return llproc_->threadPool()->userStop();
}

bool Process::detach()
{
   MTLock lock_this_func(MTLock::deliver_callbacks);

   if (!llproc_) {
      perr_printf("detach on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
      return false;
   }

   bool should_delete;
   bool result = llproc_->detach(should_delete);
   if (!result) {
      pthrd_printf("Failed to detach from process\n");
      return false;
   }
   else if (should_delete) {
      HandlerPool *hp = llproc_->handlerPool();
      delete llproc_;
      delete hp;
      assert(!llproc_);
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
      if ((*i)->getUserState() == int_thread::stopped)
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
   for (i = llproc_->threadPool()->begin(); i != llproc_->threadPool()->end(); i++) {
      if ((*i)->getUserState() == int_thread::running)
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
      if ((*i)->getUserState() == int_thread::running)
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
      if ((*i)->getUserState() == int_thread::stopped)
         return false;
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

   int_process *proc = llproc();
   int_iRPC::ptr rpc = irpc->llrpc()->rpc;
   bool result = rpcMgr()->postRPCToProc(proc, rpc);
   if (!result) {
      pthrd_printf("postRPCToProc failed on %d\n", proc->getPid());
      return Thread::ptr();
   }

   if (int_process::in_callback) {
      pthrd_printf("Returning from postIRPC in callback\n");
      return rpc->thread()->thread();
   }
   int_thread *thr = rpc->thread();
   if (thr->getInternalState() == int_thread::running) {
      //The thread is running, let's go ahead and start the RPC going.
      bool result = thr->handleNextPostedIRPC(int_thread::hnp_allow_stop, true);
      if (!result) {
         pthrd_printf("handleNextPostedIRPC failed\n");
         return Thread::ptr();
      }
   }
   return thr->thread();
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
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++)
   {
      int_thread *thr = *i;
      rpc_list_t *rpc_list = thr->getPostedRPCs();
      for (rpc_list_t::iterator j = rpc_list->begin(); j != rpc_list->end(); j++) {
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

Dyninst::Address Process::mallocMemory(size_t size, Dyninst::Address addr)
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   if (!llproc_) {
      perr_printf("mallocMemory on deleted process\n");
      setLastError(err_exited, "Process is exited\n");
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

   pthrd_printf("User wants to write memory from 0x%lx to 0x%p of size %lu\n", 
                addr, buffer, (unsigned long) size);
   result_response::ptr resp = result_response::createResultResponse();
   bool result = llproc_->writeMem(buffer, addr, size, resp);
   if (!result) {
      pthrd_printf("Error writing to memory\n");
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

   pthrd_printf("User wants to read memory from 0x%lx to 0x%p of size %lu\n", 
                addr, buffer, (unsigned long) size);
   mem_response::ptr memresult = mem_response::createMemResponse((char *) buffer, size);
   bool result = llproc_->readMem(addr, memresult);
   if (!result) {
      pthrd_printf("Error reading from memory %lx on target process %d\n",
                   addr, llproc_->getPid());
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

Process::ptr Thread::getProcess() const
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
   return llthread_->getUserState() == int_thread::stopped;
}

bool Thread::isRunning() const
{
   MTLock lock_this_func;
   if (!llthread_) {
      perr_printf("isRunning called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }
   return llthread_->getUserState() == int_thread::running;
}

bool Thread::isLive() const
{
   MTLock lock_this_func;
   if (!llthread_) {
      return false;
   }
   return (llthread_->getUserState() == int_thread::stopped ||
           llthread_->getUserState() == int_thread::running);
}

bool Thread::stopThread()
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   if (!llthread_) {
      perr_printf("stopThread called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }

   if (int_process::isInCB()) {
      perr_printf("User attempted continue call on thread while in CB, erroring.");
      setLastError(err_incallback, "Cannot continueThread from callback\n");
      return false;
   }

   return llthread_->userStop();   
}

bool Thread::continueThread()
{
   MTLock lock_this_func(MTLock::deliver_callbacks);
   if (!llthread_) {
      perr_printf("continueThread called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }

   if (int_process::isInCB()) {
      perr_printf("User attempted continue call on thread while in CB, erroring.");
      setLastError(err_incallback, "Cannot continueThread from callback\n");
      return false;
   }

   return llthread_->userCont();
}

bool Thread::getAllRegisters(RegisterPool &pool) const
{
   MTLock lock_this_func;
   if (!llthread_) {
      perr_printf("getAllRegisters called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }

   if (llthread_->getUserState() != int_thread::stopped) {
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
   if (llthread_->getUserState() != int_thread::stopped) {
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
   if (llthread_->getUserState() != int_thread::stopped) {
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
   if (llthread_->getUserState() != int_thread::stopped) {
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
      perr_printf("isInitialThrad called on exited thread\n");
      setLastError(err_exited, "Thread is exited\n");
      return false;
   }
   return llthread_->llproc()->threadPool()->initialThread() == llthread_;
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

Dyninst::THR_ID Thread::getTid() const
{
   MTLock lock_this_func;
   if( !llthread_ ) {
       assert(exitstate_);
       return exitstate_->thr_id;
   }

   return llthread_->getTid();
}

bool Thread::postIRPC(IRPC::ptr irpc) const
{
   MTLock lock_this_func;
   if (!llthread_) {
      perr_printf("postIRPC on deleted thread\n");
      setLastError(err_exited, "Thread is exited\n");
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

   if (int_process::isInCallback()) {
      pthrd_printf("Returning from postIRPC in callback\n");
      return true;
   }
   if (thr->getInternalState() == int_thread::running) {
      //The thread is running, let's go ahead and start the RPC going.
      bool result = thr->handleNextPostedIRPC(int_thread::hnp_allow_stop, true);
      if (!result) {
         pthrd_printf("handleNextPostedIRPC failed\n");
         return false;
      }
   }
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
   for (rpc_list_t::iterator j = rpc_list->begin(); j != rpc_list->end(); j++) {
      IRPC::ptr up_rpc = (*j)->getIRPC().lock();
      if (up_rpc == IRPC::ptr()) 
         continue;
      rpcs.push_back(up_rpc);
   }
   return true;
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

const Thread::ptr ThreadPool::getInitialThread() const
{
   return threadpool->initialThread()->thread();
}

ThreadPool::iterator::iterator()
{
   curp = NULL;
   curi = -1;
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
   assert(curi >= 0 && curi < (signed) curp->hl_threads.size());
   return curh;
}

ThreadPool::iterator ThreadPool::iterator::operator++()
{
   MTLock lock_this_func;
   ThreadPool::iterator orig = *this;
   if (curp->hl_threads[curi] == curh)
      curi++;

   if (curi < (signed int) curp->hl_threads.size())
      curh = curp->hl_threads[curi];
   else
      curh = Thread::ptr();

   return orig;
}

ThreadPool::iterator ThreadPool::iterator::operator++(int)
{
   MTLock lock_this_func;
   if (curp->hl_threads[curi] == curh)
      curi++;

   if (curi < (signed int) curp->hl_threads.size())
      curh = curp->hl_threads[curi];
   else
      curh = Thread::ptr();

   return *this;
}

ThreadPool::iterator ThreadPool::begin()
{
   MTLock lock_this_func;
   ThreadPool::iterator i;
   i.curp = threadpool;
   i.curi = 0;

   if (i.curi < (signed int) i.curp->hl_threads.size())
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
   i.curi = threadpool->hl_threads.size();
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
   curi = -1;
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

const Thread::ptr ThreadPool::const_iterator::operator*() const
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
   if (curp->hl_threads[curi] == curh)
      curi++;

   if (curi < (signed int) curp->hl_threads.size())
      curh = curp->hl_threads[curi];
   else
      curh = Thread::ptr();

   return orig;
}

ThreadPool::const_iterator ThreadPool::const_iterator::operator++(int)
{
   MTLock lock_this_func;
   if (curp->hl_threads[curi] == curh)
      curi++;

   if (curi < (signed int) curp->hl_threads.size())
      curh = curp->hl_threads[curi];
   else
      curh = Thread::ptr();

   return *this;
}

ThreadPool::const_iterator ThreadPool::begin() const
{
   MTLock lock_this_func;
   ThreadPool::const_iterator i;
   i.curp = threadpool;
   i.curi = 0;

   if (i.curi < (signed int) i.curp->hl_threads.size())
      i.curh = i.curp->hl_threads[i.curi];
   else
      i.curh = Thread::ptr();

   return i;
}

ThreadPool::const_iterator ThreadPool::end() const
{
   MTLock lock_this_func;
   ThreadPool::const_iterator i;
   i.curp = threadpool;
   i.curi = (int) threadpool->hl_threads.size();
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

const Process::ptr ThreadPool::getProcess() const
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
   return llnotify->getPipeIn();
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
   Generator::registerNewEventCB(eventqueue_cb_wrapper);
   is_running = true;
   should_exit = false;
   evhandler_thread.spawn(MTManager::evhandler_main_wrapper, NULL);
}

void MTManager::stop()
{
   Generator::removeNewEventCB(eventqueue_cb_wrapper);
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

void MTManager::evhandler_main_wrapper(void *)
{
   setHandlerThread(DThread::self());
   mt()->evhandler_main();
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

bool useHybridLWPControl(int_threadPool *tp) {
    return (   int_process::getThreadControlMode() == int_process::HybridLWPControl 
            && tp->size() > 1 );
}

bool useHybridLWPControl(int_process *p) {
    return (   int_process::getThreadControlMode() == int_process::HybridLWPControl 
            && p->threadPool()->size() > 1 );
}

bool useHybridLWPControl(int_thread *thrd) {
    return (   int_process::getThreadControlMode() == int_process::HybridLWPControl 
            && thrd->llproc()->threadPool()->size() > 1 );
}

bool useHybridLWPControl() {
    return ( int_process::getThreadControlMode() == int_process::HybridLWPControl );
}
