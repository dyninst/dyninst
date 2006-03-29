/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: signalhandler.C,v 

#include "eventgate.h"
#include "showerror.h"
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

EventRecord &EventGate::wait()
{
  trigger.type = evtUndefined;
  assert(lock->depth());
 
  still_waiting:
  waiting = true;
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

