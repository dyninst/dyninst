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

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/src/libstate.h"
#include "common/h/headers.h"
#include <assert.h>
#include <string>
#include <vector>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;
using namespace std;

class DefaultLibState : public LibraryState
{
public:
   DefaultLibState(ProcessState *parent) : 
      LibraryState(parent) 
   {
   }

   virtual bool getLibraryAtAddr(Address, LibAddrPair &) {
      return false;
   }

   virtual bool getLibraries(std::vector<LibAddrPair> &) {
      return false;
   }

   virtual void notifyOfUpdate() {
   }

   virtual Address getLibTrapAddress() {
      return 0x0;
   }

   ~DefaultLibState() { 
   }
};

std::map<Dyninst::PID, ProcessState *> ProcessState::proc_map;

ProcessState::ProcessState(Dyninst::PID pid_, std::string executable_path_) :
   library_tracker(NULL),
   executable_path(executable_path_)
{
   std::map<PID, ProcessState *>::iterator i = proc_map.find(pid_);
   if (i != proc_map.end())
   {
      sw_printf("[%s:%u] - Already attached to debuggee %d\n",
                __FILE__, __LINE__, pid_);
      setLastError(err_badparam, "Attach requested to already " \
                   "attached process");
      return;
   }
   setPid(pid_);
}

void ProcessState::setPid(Dyninst::PID pid_)
{
   pid = pid_;
   proc_map[pid] = this;
}

Dyninst::PID ProcessState::getProcessId() 
{
   return pid;
}

bool ProcessState::preStackwalk(Dyninst::THR_ID)
{
   return true;
}

bool ProcessState::postStackwalk(Dyninst::THR_ID)
{
   return true;
}

void ProcessState::setDefaultLibraryTracker()
{
  if (library_tracker) return;

  std::string execp = getExecutablePath();
  library_tracker = new TrackLibState(this, execp);
}

Walker *ProcessState::getWalker() const
{
   return walker;
}

ProcessState::~ProcessState()
{
   if (library_tracker)
      delete library_tracker;
   proc_map.erase(pid);
}

int ProcDebug::pipe_in = -1;
int ProcDebug::pipe_out = -1;

ProcDebug::ProcDebug(PID p, string exe) : 
  ProcessState(p, exe),
  initial_thread(NULL),
  active_thread(NULL),
  sigfunc(NULL)
{
  initial_thread = ThreadState::createThreadState(this);
  threads[initial_thread->getTid()] = initial_thread;   
}

ProcDebug::ProcDebug(std::string executable, 
                     const std::vector<std::string> & /*argv*/) : 
   ProcessState(0, executable),
   initial_thread(NULL),
   active_thread(NULL),
   sigfunc(NULL)
{
}

bool ProcDebug::create(string executable, 
                       const vector<string> &argv)
{
  bool result = debug_create(executable, argv);
  if (!result) {
    sw_printf("[%s:%u] - Could not create debuggee, %s\n",
              __FILE__, __LINE__, executable.c_str());
    return false;
  }

  setPid(pid);
  initial_thread = ThreadState::createThreadState(this, NULL_THR_ID, true);

  threads[initial_thread->getTid()] = initial_thread;   

  sw_printf("[%s:%u] - Created debugged %s on pid %d\n",
            __FILE__, __LINE__, executable.c_str(), pid);
  result = debug_waitfor_create();
  if (state() == ps_exited) {
     sw_printf("[%s:%u] - Process %s exited during create\n", 
               __FILE__, __LINE__, executable.c_str());
    return false; 
  }
  if (!result) {
    sw_printf("[%s:%u] - Error during process create for %d\n",
	      __FILE__, __LINE__, pid);
    return false;
  }
  result = debug_post_create();
    if (!result) {
    sw_printf("[%s:%u] - Error during post create for %d\n",
                __FILE__, __LINE__, pid);
      return false;
    }
     
  assert(state() == ps_running);
  
  return true;
}

bool ProcDebug::debug_waitfor_create()
{
  for (;;) {
    bool handled, result;
    
    result = debug_wait_and_handle(true, false, handled);
    if (!result || state() == ps_errorstate) {
      sw_printf("[%s:%u] - Error.  Process %d errored during create\n",
                __FILE__, __LINE__, pid);
      return false;
    }
    if (state() == ps_exited) {
      sw_printf("[%s:%u] - Error.  Process %d exited during create\n",
                __FILE__, __LINE__, pid);
      return false;
    }
    if (state() == ps_running) {
      sw_printf("[%s:%u] - Successfully completed create on %d\n",
                __FILE__, __LINE__, pid);
      return true;
    }
  }
}

bool ProcDebug::multi_attach(vector<ProcDebug *> &pids)
{
   bool result;
   bool had_error = false;
   vector<ProcDebug *>::iterator i;

#define for_each_procdebug(func, cond, err_msg) \
   for (i = pids.begin(); i != pids.end(); i++) { \
      ProcDebug *pd = (*i); \
      if (!pd) \
        continue; \
      if (!cond) \
        continue; \
      result = pd->func(pd->initial_thread); \
      if (!result) { \
         sw_printf("[%s:%u] - Could not %s to %d", __FILE__, __LINE__, err_msg, pd->pid); \
         delete pd; \
         *i = NULL; \
         had_error = true; \
      } \
   }

   for_each_procdebug(debug_attach, true, "attach");

   for_each_procdebug(debug_waitfor_attach, true, "wait for attach");

   for_each_procdebug(debug_post_attach, true, "post attach");

   for_each_procdebug(debug_continue, (pd->state() != ps_running), "send continue");

   return had_error;
}

bool ProcDebug::attach()
{
  bool result = debug_attach(initial_thread);
  if (!result) {
    sw_printf("[%s:%u] - Could not attach to debuggee, %d\n",
	      __FILE__, __LINE__, pid);
    return false;
  }

  result = debug_waitfor_attach(initial_thread);
  if (!result) {
     sw_printf("[%s:%u] - Error waiting for attach\n", __FILE__, __LINE__);
     goto error;
  }

  result = debug_post_attach(initial_thread);
  if (!result) {
     sw_printf("[%s:%u] - Error on post attach for %d\n",
               __FILE__, __LINE__, pid);
     goto error;
  }

  if (state() != ps_running) {
     result = debug_continue(initial_thread);
     if (!result) {
        sw_printf("[%s:%u] - Could not continue debuggee %d after attach\n",
                  __FILE__, __LINE__, pid);
        goto error;
     }
  }

  return true;

 error:
  if (state() == ps_exited) {
    setLastError(err_procexit, "Process exited unexpectedly during attach");
  }
  sw_printf("[%s:%u] - Error during process attach for %d\n",
            __FILE__, __LINE__, pid);
  return false;
}


bool ProcDebug::debug_waitfor_attach(ThreadState *ts)
{
  Dyninst::THR_ID tid = ts->getTid();
  for (;;) {
    bool handled, result;

    if (ts->state() == ps_exited) {
      sw_printf("[%s:%u] - Error.  Thread %d/%d exited during attach\n",
                __FILE__, __LINE__, pid, tid);
      return false;
    }
    if (ts->state() == ps_attached || ts->state() == ps_running)  {
      sw_printf("[%s:%u] - Successfully completed attach on %d/%d\n",
                __FILE__, __LINE__, pid, tid);
      return true;
    }
    
    result = debug_wait_and_handle(true, false, handled);
    if (!result || ts->state() == ps_errorstate) {
      sw_printf("[%s:%u] - Error.  Thread %d/%d errored during attach\n",
                __FILE__, __LINE__, pid, tid);
      return false;
    }
  }
}

bool ProcDebug::debug_post_attach(ThreadState *)
{
   return true;
}

bool ProcDebug::debug_post_create()
{
   return true;
}

bool ProcDebug::resume(Dyninst::THR_ID tid)
{
   sw_printf("[%s:%u] - User is continuing process %d, thread %d\n",
             __FILE__, __LINE__, pid, tid);
   
   ThreadState *thr = NULL;
   if (tid != NULL_THR_ID) {
      //Handle the case where we're being asked to continue one thread.
      thread_map_t::iterator i = threads.find(tid);
      if (i == threads.end()) {
         sw_printf("[%s:%u] - Thread %d in process %d was not valid\n",
                   __FILE__, __LINE__, tid, pid);
         setLastError(err_badparam, "No such thread");
         return false;
      }
      thr = (*i).second;
      assert(thr);
      if (thr->state() == ps_exited) {
         sw_printf("[%s:%u] - Attempt to resume thread %d in proc %d that "
                   "already exited\n", __FILE__, __LINE__, tid, pid);
         setLastError(err_badparam, "Thread already exited");
         return false;
      }
      if (!thr->isStopped()) {
         sw_printf("[%s:%u] - thread %d is already running on process %d\n",
                   __FILE__, __LINE__, tid, pid);
         thr->setUserStopped(false);
      }
      else {
         bool result = resume_thread(thr);
         if (result) {
            thr->setUserStopped(false);
         }
      }
      return true;
   }
   
   //Handle the case where we're continuing all threads
   thread_map_t::iterator i;
   bool had_error = false;
   for (i = threads.begin(); i != threads.end(); i++) {
      thr = (*i).second;
      int thr_tid = thr->getTid();
      assert(thr);
      if (thr->state() == ps_exited) {
         sw_printf("[%s:%u] - thread %d on process %d already exited\n",
                __FILE__, __LINE__, thr_tid, pid);
         continue;
      }
      if (!thr->isStopped()) {
         sw_printf("[%s:%u] - thread %d is already running on process %d\n",
                   __FILE__, __LINE__, thr_tid, pid);
         thr->setUserStopped(false);
         continue;
      }

      sw_printf("[%s:%u] - Continuing thread %d on process %d\n",
                __FILE__, __LINE__, thr_tid, pid);
      bool result = resume_thread(thr);
      if (!result) {
         sw_printf("[%s:%u] - Error resuming thread %d on process %d\n",
                __FILE__, __LINE__, thr_tid, pid);
         had_error = true;
      }
      else {
         thr->setUserStopped(false);
      }
   }
   return !had_error;
}

bool ProcDebug::resume_thread(ThreadState *thr)
{
   Dyninst::THR_ID tid = thr->getTid();
   sw_printf("[%s:%u] - Top level resume for %d/%d\n",
             __FILE__, __LINE__, pid, tid);
   bool result = debug_continue(thr);
   if (!result) {
      sw_printf("[%s:%u] - Could not resume debugee %d, thread %d\n",
                __FILE__, __LINE__, pid, tid);
      return false;
   }

   result = debug_waitfor_continue(thr);
   if (state() == ps_exited) {
      setLastError(err_procexit, "Process exited unexpectedly during continue");
      return false; 
   }
   if (!result) {
      sw_printf("[%s:%u] - Error during process resume for %d\n",
                __FILE__, __LINE__, pid);
      return false;
   }
   
   return true;
}

bool ProcDebug::debug_waitfor_continue(ThreadState *thr)
{
  sw_printf("[%s:%u] - At debug_waitfor_continue, isStopped = %d\n",
            __FILE__, __LINE__, (int) thr->isStopped());
  while (thr->isStopped()) {
    bool handled, result;
    
    result = debug_wait_and_handle(true, false, handled);
    if (!result || state() == ps_errorstate) {
      sw_printf("[%s:%u] - Error.  Process %d errored during continue\n",
                __FILE__, __LINE__, pid);
      return false;
    }
    if (state() == ps_exited || thr->state() == ps_exited) {
      sw_printf("[%s:%u] - Error.  Process %d exited during continue\n",
                __FILE__, __LINE__, pid);
      return false;
    }
  }
  sw_printf("[%s:%u] - Successfully continued %d/%d\n",
            __FILE__, __LINE__, pid, thr->getTid());
  return true;
}

bool ProcDebug::pause(Dyninst::THR_ID tid)
{
   sw_printf("[%s:%u] - User is stopping process %d, thread %d\n",
             __FILE__, __LINE__, pid, tid);
   
   ThreadState *thr = NULL;
   if (tid != NULL_THR_ID) {
      //Handle the case where we're being asked to stop one thread.
      thread_map_t::iterator i = threads.find(tid);
      if (i == threads.end()) {
         sw_printf("[%s:%u] - Thread %d in process %d was not valid\n",
                   __FILE__, __LINE__, tid, pid);
         setLastError(err_badparam, "No such thread");
         return false;
      }
      thr = (*i).second;
      assert(thr);
      if (thr->state() == ps_exited) {
         sw_printf("[%s:%u] - Attempt to resume thread %d in proc %d that "
                   "already exited\n", __FILE__, __LINE__, tid, pid);
         setLastError(err_procexit, "Thread already exited");
         return false;
      }
      bool result = true;
      if (!thr->isStopped()) {
         result = pause_thread(thr);
         if (result) {
            thr->setUserStopped(true);         
         }
      }
      return result;
   }
   
   //Handle the case where we're stopping all threads
   thread_map_t::iterator i;
   bool had_error = false;
   for (i = threads.begin(); i != threads.end(); i++) {
      thr = (*i).second;
      assert(thr);
      if (thr->state() == ps_exited) {
         sw_printf("[%s:%u] - thread %d on process %d already exited\n",
                __FILE__, __LINE__, tid, pid);
         continue;
      }
      sw_printf("[%s:%u] - Pausing thread %d on process %d\n",
                __FILE__, __LINE__, tid, pid);
      if (!thr->isStopped()) {
         bool result = pause_thread(thr);
         if (!result) {
            sw_printf("[%s:%u] - Error pausing thread %d on process %d\n",
                      __FILE__, __LINE__, tid, pid);
            had_error = true;
            continue;
         }
      }
      thr->setUserStopped(true);         
   }
   return !had_error;   
}

bool ProcDebug::pause_thread(ThreadState *thr)
{
   Dyninst::THR_ID tid = thr->getTid();
   sw_printf("[%s:%u] - Top level thread pause for %d/%d\n",
             __FILE__, __LINE__, pid, tid);
   if (thr->isStopped())
   {
      sw_printf("Thread already paused, returning\n");
      return true;
   }

   bool result = debug_pause(thr);
   if (!result) {
      sw_printf("[%s:%u] - Could not pause debuggee %d, thr %d\n",
                __FILE__, __LINE__, pid, tid);
    return false;
  }
  
   result = debug_waitfor_pause(thr);
   if (thr->state() == ps_exited) {
    setLastError(err_procexit, "Process exited unexpectedly during pause");
    return false; 
  }
  if (!result) {
      sw_printf("[%s:%u] - Error during process pause for %d, thr %d\n",
                __FILE__, __LINE__, pid, tid);
    return false;
  }

   assert(thr->isStopped());
  return true;
}

bool ProcDebug::debug_waitfor_pause(ThreadState *thr)
{
   Dyninst::THR_ID tid = thr->getTid();
   sw_printf("[%s:%u] - Waiting for %d, %d to stop\n", __FILE__, __LINE__, pid, tid);
   while (!thr->isStopped()) {
      bool handled, result;
      
      result = debug_wait_and_handle(true, false, handled);
      if (!result || thr->state() == ps_errorstate) {
         sw_printf("[%s:%u] - Error.  Process %d, %d errored during pause\n",
                   __FILE__, __LINE__, pid, tid);
         return false;
      }
      if (thr->state() == ps_exited) {
         sw_printf("[%s:%u] - Error.  Process %d, %d exited during pause\n",
                   __FILE__, __LINE__, pid, tid);
         return false;
      }
   }
   sw_printf("[%s:%u] - Successfully stopped %d, %d\n", 
             __FILE__, __LINE__, pid, tid);
  return true;
}


bool ProcDebug::debug_waitfor(dbg_t event_type) {
  bool handled;
  dbg_t handled_type;
  bool flush = false;
#if defined(os_linux)
  flush = true;
#endif
  while (debug_wait_and_handle(true, flush, handled, &handled_type)) {
    if (handled_type == event_type) {
      return true;
    }
  }
  return false;
}


bool ProcDebug::debug_wait_and_handle(bool block, bool flush, bool &handled, 
                                      dbg_t *event_type)
{
  bool result;
  handled = false;
  
  for (;;) {
     DebugEvent ev = debug_get_event(block);

     if (ev.dbg == dbg_noevent)
     {
        sw_printf("[%s:%u] - Returning from debug_wait_and_handle, %s.\n",
                  __FILE__, __LINE__, handled ? "handled event" : "no event");
        if (!handled && event_type) {
           *event_type = dbg_noevent;
        }
        return true;
     }
     if (event_type) {
        *event_type = ev.dbg;
     }
     if (ev.dbg == dbg_err)
     {
        sw_printf("[%s:%u] - Returning from debug_wait_and_handle with error\n",
                  __FILE__, __LINE__);
        return false;
     }

     sw_printf("[%s:%u] - Handling event for pid %d: dbg %d, data %d\n", 
               __FILE__, __LINE__, ev.proc->pid, ev.dbg, ev.data.idata);
     result = ev.proc->debug_handle_event(ev);
     if (!result) {
        sw_printf("[%s:%u] - debug_handle_event returned error for ev.dbg = %d, " \
                  "ev.proc = %d\n", __FILE__, __LINE__, ev.dbg, ev.proc->pid);
        handled = false;
        return false;
     }
     
     sw_printf("[%s:%u] - Event %d on pid %d successfully handled\n", 
               __FILE__, __LINE__, ev.dbg, ev.proc->pid);
     handled = true;

     if (!flush)
        break;
     block = false;
  }
  return true;
} 


bool ProcDebug::add_new_thread(THR_ID tid) {
  if (threads.count(tid)) return true;

  sw_printf("[%s:%u] - Adding new thread %d, in proc %d\n",
            __FILE__, __LINE__, tid, pid);

  ThreadState *new_thread = ThreadState::createThreadState(this, tid);
  threads[tid] = new_thread;
  
  if (!new_thread && getLastError() == err_noproc) {
    //Race condition, get thread ID or running thread, which then 
    // exits.  Should be rare...
    sw_printf("[%s:%u] - Error creating thread %d, does not exist\n",
              __FILE__, __LINE__, tid);
    clearLastError();
    return false;

  } else if (!new_thread) {
    sw_printf("[%s:%u] - Unexpected error creating thread %d\n",
              __FILE__, __LINE__, tid);
    return false;
  }

  return true;
}


ProcDebug::~ProcDebug()
{
}

unsigned ProcSelf::getAddressWidth()
{
   return sizeof(void *);
}

bool ProcSelf::isFirstParty()
{
  return true;
}

ProcSelf::~ProcSelf()
{
}

bool ProcDebug::handleDebugEvent(bool block)
{
  bool result;
  bool handled;
  
  result = debug_wait_and_handle(block, true, handled);
  if (!result) {
    sw_printf("[%s:%u] - Error waiting for event in handleDebugEvent\n",
	      __FILE__, __LINE__);
    return false;
  }

  return true;
}

bool ProcDebug::preStackwalk(Dyninst::THR_ID tid)
{
   if (tid == NULL_THR_ID)
      tid = initial_thread->getTid();

   sw_printf("[%s:%u] - Calling preStackwalk for thread %d\n", __FILE__, __LINE__, tid);

   thread_map_t::iterator i = threads.find(tid);
   if (i == threads.end()) {
      sw_printf("[%s:%u] - Couldn't find thread %d!\n", __FILE__, __LINE__, tid);
      return false;
   }
   
   bool result = true;
   active_thread = (*i).second;
   if (active_thread->state() != ps_running) {
      sw_printf("[%s:%u] - Attempt to stackwalk on non-running thread %d/%d\n",
                __FILE__, __LINE__, active_thread->proc()->getProcessId(), 
                active_thread->getTid());
      setLastError(err_procexit, "Stackwalk attempted on exited thread");
      return false;
   }

   if (!active_thread->userIsStopped()) {
      active_thread->setShouldResume(true);
   }
   if (!active_thread->isStopped())
      result = pause_thread(active_thread);
   return result;
}

bool ProcDebug::postStackwalk(Dyninst::THR_ID tid)
{
   if (tid == NULL_THR_ID)
      tid = initial_thread->getTid();

   sw_printf("[%s:%u] - Calling preStackwalk for thread %d\n", __FILE__, __LINE__, tid);

   thread_map_t::iterator i = threads.find(tid);
   if (i == threads.end()) {
      sw_printf("[%s:%u] - Couldn't find thread %d!\n", __FILE__, __LINE__, tid);
      return false;
   }

   assert(active_thread == (*i).second);
   if (active_thread->shouldResume()) {
      resume_thread(active_thread);
      active_thread->setShouldResume(false);      
   }
   return true;
}

bool ProcDebug::isTerminated()
{
   return (state() == ps_exited || state() == ps_errorstate);
}

bool ProcDebug::isFirstParty()
{
  return false;
}

LibraryState::LibraryState(ProcessState *parent) :
   procstate(parent)
{
}

LibraryState::~LibraryState()
{
}

#ifndef os_bg_ion
bool ProcDebug::newProcDebugSet(const vector<Dyninst::PID> &pids,
                                vector<ProcDebug *> &out_set)
{
   vector<Dyninst::PID>::const_iterator i;
   for (i=pids.begin(); i!=pids.end(); i++)
   {
      Dyninst::PID pid = *i;
      ProcDebug *new_pd = ProcDebug::newProcDebug(pid);
      if (!new_pd) {
         fprintf(stderr, "[%s:%u] - Unable to allocate new ProcDebugBG\n",
                 __FILE__, __LINE__);
         return false;
   }
      out_set.push_back(new_pd);
   }
   return true;
}
#endif // !os_bg_ion

LibraryState *ProcessState::getLibraryTracker()
{
   return library_tracker;
}

proc_state ProcDebug::state()
{
   assert(initial_thread);
   return initial_thread->state();
}

void ProcDebug::setState(proc_state p)
{
   assert(initial_thread);
   sw_printf("[%s:%u] - Setting initial thread for %d to state %d\n",
             __FILE__, __LINE__, pid, p);
   initial_thread->setState(p);
}

void ProcDebug::registerSignalCallback(sig_callback_func f)
{
   sigfunc = f;
}

string ProcessState::getExecutablePath() {
  return executable_path;
}

ThreadState::ThreadState(ProcDebug *p, Dyninst::THR_ID id) :
   is_stopped(true),
   user_stopped(true),
   should_resume(false),
   tid(id),
   thr_state(ps_neonatal),
   parent(p),
   pending_sigstops(0)
{
}

bool ThreadState::isStopped()
{
   return is_stopped;
}

void ThreadState::setStopped(bool s)
{
   is_stopped = s;
}

bool ThreadState::userIsStopped()
{
   return user_stopped;
}

void ThreadState::setUserStopped(bool u)
{
   user_stopped = u;
}

bool ThreadState::shouldResume()
{
   return should_resume;
}

void ThreadState::setShouldResume(bool r)
{
   should_resume = r;
}

Dyninst::THR_ID ThreadState::getTid()
{
   return tid;
}

proc_state ThreadState::state()
{
   return thr_state;
}

void ThreadState::setState(proc_state s)
{
   sw_printf("[%s:%u] - Setting thread %d to state %d\n",
             __FILE__, __LINE__, tid, s);
   thr_state = s;
}

ProcDebug* ThreadState::proc() 
{
   return parent;
}

ThreadState::~ThreadState()
{
}

void ThreadState::markPendingStop() {
  pending_sigstops++;
}

void ThreadState::clearPendingStop() {
  pending_sigstops--;
}

bool ThreadState::hasPendingStop() {
  return pending_sigstops != 0;
}

