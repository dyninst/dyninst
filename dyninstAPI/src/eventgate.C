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

// $Id: signalhandler.C,v 

#include "eventgate.h"
#include "debug.h"
#include "util.h"

EventGate::EventGate(eventLock *l, eventType t, process *p, dyn_lwp *lwp,
                     eventStatusCode_t status) :
     lock(l), waiting(false)
{
  //cond = new eventCond(lock);
  EventRecord target_event;
  target_event.type = t;
  target_event.proc = p;
  target_event.lwp = lwp;
  target_event.status = status;
  evts.push_back(target_event);

  if (t != evtProcessExit) {
    EventRecord process_exit_event;
    process_exit_event.type = evtProcessExit;
    process_exit_event.proc = p;
    evts.push_back(process_exit_event);
  }
}

bool EventGate::addEvent(eventType type, process *p)
{
  EventRecord target_event;
  target_event.type = type;
  target_event.proc = p;
  if (type == evtProcessExit) {
    for (unsigned int i = 0; i < evts.size(); ++i) {
      if (evts[i].isTemplateOf(target_event)) {
        //fprintf(stderr, "%s[%d]:  dropping duplicate request to wait for proces exit\n",
         //       FILE__, __LINE__);
        return true;
      }
    }
  }
  evts.push_back(target_event);
  return true;
}

EventGate::~EventGate()
{
  //delete cond;
}

EventRecord &EventGate::wait(bool executeCallbacks /* = true */)
{
  trigger.type = evtUndefined;
  assert(lock->depth());
 
  still_waiting:
  waiting = true;
  if (executeCallbacks) 
    getMailbox()->executeCallbacks(FILE__, __LINE__);

  if (trigger.type != evtUndefined) {
    return trigger;
  }
  extern int dyn_debug_signal;
  if (dyn_debug_signal) {
    signal_printf("%s[%d][%s]: waiting for event matching:\n", FILE__, __LINE__,
            getThreadStr(getExecThreadID()));
    for (unsigned int i = 0; i < evts.size(); ++i) {
      char buf[1024];
      signal_printf("\t%s\n", evts[i].sprint_event(buf));
    }
  }

  lock->_WaitForSignal(FILE__, __LINE__);
  waiting = false;
  

  bool triggered = false;
  for (unsigned int i = 0; i < evts.size(); ++i) {
    if (evts[i].isTemplateOf(trigger)) {
      triggered = true;
      break;
    }
  }
  if (!triggered) goto still_waiting;

  return trigger;
}

bool EventGate::signalIfMatch(EventRecord &ev)
{
  lock->_Lock(FILE__, __LINE__);
  if (!waiting) {
    lock->_Unlock(FILE__, __LINE__);
    return false;
  }
  bool ret = false;
  for (unsigned int i = 0; i < evts.size(); ++i) {
    if (evts[i].isTemplateOf(ev)) {
      ret = true;
      trigger = ev;
      lock->_Broadcast(FILE__, __LINE__);
      break;
    }
  }
  lock->_Unlock(FILE__, __LINE__);
  return ret;
}

