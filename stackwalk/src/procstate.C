/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/procstate.h"
#include "common/h/headers.h"
#include <assert.h>
#include <string>
#include <vector>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;
using namespace std;

void ProcessState::preStackwalk()
{
}

void ProcessState::postStackwalk()
{
}

ProcessState::~ProcessState()
{
}

bool procdebug_ltint::operator()(int a, int b) const
{
   return a < b;
}

std::map<PID, ProcDebug *, procdebug_ltint> ProcDebug::proc_map;
int ProcDebug::pipe_in = -1;
int ProcDebug::pipe_out = -1;

ProcDebug::ProcDebug(PID p) : 
  state(ps_neonatal),
  isRunning(false),
  user_isRunning(false),
  should_resume(false),
  pid(p)
{
   std::map<PID, ProcDebug *, procdebug_ltint>::iterator i = proc_map.find(p);
   if (i != proc_map.end())
   {
      sw_printf("[%s:%u] - Already attached to debuggee %d\n",
                __FILE__, __LINE__, pid);
      setLastError(err_badparam, "Attach requested to already " \
                   "attached to process");
      return;
   }
   
   proc_map[pid] = this;
}

ProcDebug::ProcDebug(const std::string & /*executable*/, 
                     const std::vector<std::string> & /*argv*/) : 
  state(ps_neonatal),
  isRunning(false),
  user_isRunning(false),
  should_resume(false)
{
}

bool ProcDebug::create(const string &executable, 
                       const vector<string> &argv)
{
  bool result = debug_create(executable, argv);
  if (!result) {
    sw_printf("[%s:%u] - Could not create debuggee, %s\n",
              __FILE__, __LINE__, executable.c_str());
    return false;
  }

  proc_map[pid] = this;

  sw_printf("[%s:%u] - Created debugged %s on pid %d\n",
            __FILE__, __LINE__, executable.c_str(), pid);
  result = debug_waitfor_create();
  if (state == ps_exited) {
     sw_printf("[%s:%u] - Process %s exited during create\n", 
               __FILE__, __LINE__, executable.c_str());
    return false; 
  }
  if (!result) {
    sw_printf("[%s:%u] - Error during process create for %s\n",
	      __FILE__, __LINE__, pid);
    return false;
  }
  assert(state == ps_running);
  
  /*
  if (!isRunning) {
    result = resume();
    if (!result) {
      sw_printf("[%s:%u] - Could not continue process %d after create\n",
                __FILE__, __LINE__, pid);
      return false;
    }
  }
  */
  
  return true;
}

bool ProcDebug::debug_waitfor_create()
{
  for (;;) {
    bool handled, result;
    
    result = debug_wait_and_handle(true, handled);
    if (!result || state == ps_errorstate) {
      sw_printf("[%s:%u] - Error.  Process %d errored during create\n",
                __FILE__, __LINE__, pid);
      return false;
    }
    if (state == ps_exited) {
      sw_printf("[%s:%u] - Error.  Process %d exited during create\n",
                __FILE__, __LINE__, pid);
      return false;
    }
    if (state == ps_running) {
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
      result = pd->func(); \
      if (!result) { \
         sw_printf("[%s:%u] - Could not %s to %d", __FILE__, __LINE__, err_msg, pd->pid); \
         delete pd; \
         *i = NULL; \
         had_error = true; \
      } \
   }

   for_each_procdebug(debug_attach, true, "attach");

   for_each_procdebug(debug_waitfor_attach, true, "wait for attach");

   for_each_procdebug(debug_continue, (pd->state != ps_running), "send continue");

   return had_error;
}

bool ProcDebug::attach()
{
  bool result = debug_attach();
  if (!result) {
    sw_printf("[%s:%u] - Could not attach to debuggee, %d\n",
	      __FILE__, __LINE__, pid);
    return false;
  }

  result = debug_waitfor_attach();
  if (!result) {
     sw_printf("[%s:%u] - Error waiting for attach\n", __FILE__, __LINE__);
     goto error;
  }

  if (state != ps_running) {
     result = debug_continue();
     if (!result) {
        sw_printf("[%s:%u] - Could not continue debuggee %d after attach\n",
                  __FILE__, __LINE__, pid);
        goto error;
     }
  }

  assert(state == ps_running);
  return true;

 error:
  if (state == ps_exited) {
    setLastError(err_procexit, "Process exited unexpectedly during attach");
  }
  sw_printf("[%s:%u] - Error during process attach for %d\n",
            __FILE__, __LINE__, pid);
  return false;
}


bool ProcDebug::debug_waitfor_attach()
{
  for (;;) {
    bool handled, result;

    if (state == ps_exited) {
      sw_printf("[%s:%u] - Error.  Process %d exited during attach\n",
                __FILE__, __LINE__, pid);
      return false;
    }
    if (state == ps_attached || state == ps_running)  {
      sw_printf("[%s:%u] - Successfully completed attach on %d\n",
                __FILE__, __LINE__, pid);
      return true;
    }
    
    result = debug_wait_and_handle(true, handled);
    if (!result || state == ps_errorstate) {
      sw_printf("[%s:%u] - Error.  Process %d errored during attach\n",
                __FILE__, __LINE__, pid);
      return false;
    }
  }
}

bool ProcDebug::resume()
{
   bool result = debug_continue();
   if (!result) {
      sw_printf("[%s:%u] - Could not resume debugee %d\n",
                __FILE__, __LINE__, pid);
      return false;
   }

   result = debug_waitfor_continue();
   if (state == ps_exited) {
      setLastError(err_procexit, "Process exited unexpectedly during continue");
      return false; 
   }
   if (!result) {
      sw_printf("[%s:%u] - Error during process resume for %d\n",
                __FILE__, __LINE__, pid);
      return false;
   }
   
   user_isRunning = true;

   assert(isRunning);
   return true;
}

bool ProcDebug::debug_waitfor_continue()
{
  while (!isRunning) {
    bool handled, result;
    
    result = debug_wait_and_handle(true, handled);
    if (!result || state == ps_errorstate) {
      sw_printf("[%s:%u] - Error.  Process %d errored during attach\n",
                __FILE__, __LINE__, pid);
      return false;
    }
    if (state == ps_exited) {
      sw_printf("[%s:%u] - Error.  Process %d exited during attach\n",
                __FILE__, __LINE__, pid);
      return false;
    }
  }
  sw_printf("[%s:%u] - Successfully continued %d\n",
            __FILE__, __LINE__, pid);
  return true;
}

bool ProcDebug::pause()
{
  bool result = debug_pause();
  if (!result) {
    sw_printf("[%s:%u] - Could not pause debuggee %d\n",
	      __FILE__, __LINE__, pid);
    return false;
  }
  
  result = debug_waitfor_pause();
  if (state == ps_exited) {
    setLastError(err_procexit, "Process exited unexpectedly during pause");
    return false; 
  }
  if (!result) {
    sw_printf("[%s:%u] - Error during process pause for %d\n",
	      __FILE__, __LINE__, pid);
    return false;
  }

  user_isRunning = false;

  assert(!isRunning);
  return true;
}

bool ProcDebug::debug_waitfor_pause()
{
  sw_printf("[%s:%u] - Waiting for %d to stop\n", __FILE__, __LINE__, pid);
  while (isRunning) {
    bool handled, result;
    
    result = debug_wait_and_handle(true, handled);
    if (!result || state == ps_errorstate) {
      sw_printf("[%s:%u] - Error.  Process %d errored during pause\n",
		__FILE__, __LINE__, pid);
      return false;
    }
    if (state == ps_exited) {
      sw_printf("[%s:%u] - Error.  Process %d exited during pause\n",
		__FILE__, __LINE__, pid);
      return false;
    }
  }
  sw_printf("[%s:%u] - Successfully stopped %d\n", __FILE__, __LINE__, pid);
  return true;
}

bool ProcDebug::debug_wait_and_handle(bool block, bool &handled)
{
  bool result;
  DebugEvent ev = debug_get_event(block);

  if (ev.dbg == dbg_noevent)
  {
    sw_printf("[%s:%u] - Returning from debug_wait_and_handle with nothing to do\n",
	      __FILE__, __LINE__);
    handled = false;
    return true;
  }
  if (ev.dbg == dbg_err)
  {
    sw_printf("[%s:%u] - Returning from debug_wait_and_handle with error\n",
	      __FILE__, __LINE__);
    handled = false;
    return false;
  }

  sw_printf("[%s:%u] - Handling event on for pid %d: dbg %d, data %d\n", 
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
  return true;
} 

ProcDebug::~ProcDebug()
{
}

unsigned ProcSelf::getAddressWidth()
{
   return sizeof(void *);
}

PID ProcSelf::getProcessId()
{
  return mypid;
}

ProcSelf::~ProcSelf()
{
}

bool ProcDebug::handleDebugEvent(bool block)
{
  bool result;
  bool handled;
  
  result = debug_wait_and_handle(block, handled);
  if (!result) {
    sw_printf("[%s:%u] - Error waiting for event in handleDebugEvent\n",
	      __FILE__, __LINE__);
    return false;
  }

  return true;
}

PID ProcDebug::getProcessId()
{
   return pid;
}

void ProcDebug::preStackwalk()
{
   if (isRunning) {
      should_resume = true;
      pause();
   }
}

void ProcDebug::postStackwalk()
{
   if (should_resume) {
      resume();
      should_resume = false;
   }
}

bool ProcDebug::isTerminated()
{
  return (state == ps_exited || state == ps_errorstate);
}

