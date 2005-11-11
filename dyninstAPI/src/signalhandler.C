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

#include "process.h"
#include "signalhandler.h"

SignalHandler *global_sh = NULL;

SignalHandler *getSH() {
   if(global_sh == NULL) {
      signal_printf("%s[%d]:  about to create new SignalHandler\n", FILE__, __LINE__);
      global_sh = new SignalHandler();
      global_sh->createThread();
   }

   return global_sh;
}

EventGate::EventGate(eventLock *l, eventType t, process *p, dyn_lwp *lwp) :
     lock(l), waiting(false)
{
  cond = new eventCond(lock);
  EventRecord target_event;
  target_event.type = t;
  target_event.proc = p;
  target_event.lwp = lwp;
  evts.push_back(target_event);

  if (t != evtProcessExit) {
    EventRecord process_exit_event;
    process_exit_event.type = evtProcessExit;
    process_exit_event.proc = p;
    process_exit_event.lwp = lwp;
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
  delete cond;
}

EventRecord &EventGate::wait()
{
  trigger.type = evtUndefined;
  assert(lock->depth());
 
  still_waiting:
  getMailbox()->executeCallbacks(FILE__, __LINE__);
  waiting = true;
  extern int dyn_debug_signal;
  if (dyn_debug_signal) {
    signal_printf("%s[%d][%s]: waiting for event matching:\n", FILE__, __LINE__,
            getThreadStr(getExecThreadID()));
    for (unsigned int i = 0; i < evts.size(); ++i) {
      char buf[1024];
      signal_printf("\t\%s\n", evts[i].sprint_event(buf));
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

bool SignalHandler::signalActiveProcess()
{         
  bool ret = true;
  if (waiting_for_active_process)
    ret = __BROADCAST;
  return ret;
}   

bool SignalHandler::signalEvent(EventRecord &ev)
{
  signal_printf("%s[%d][%s]:  signalEvent(%s)\n", FILE__, __LINE__, 
                getThreadStr(getExecThreadID()), eventType2str(ev.type));
  assert(global_mutex->depth());

  getMailbox()->executeCallbacks(FILE__, __LINE__);

  if (ev.type == evtProcessStop || ev.type == evtProcessExit)
      BPatch::bpatch->mutateeStatusChange = true;

  bool ret = false;
  for (unsigned int i = 0; i <wait_list.size(); ++i) {
    if (wait_list[i]->signalIfMatch(ev)) {
      ret = true;
    }
  }

  
#if 0
  if (!ret) 
    signal_printf("%s[%d][%s]:  signalEvent(%s): nobody waiting\n", FILE__, __LINE__, 
                  getThreadStr(getExecThreadID()), eventType2str(ev.type));
#endif
  return ret;
}

bool SignalHandler::signalEvent(eventType t)
{
  EventRecord ev;
  ev.type = t;
  return signalEvent(ev);
}

eventType SignalHandler::waitForOneOf(pdvector<eventType> &evts)
{
  assert(global_mutex->depth());

  if (getExecThreadID() == getThreadID()) {
    fprintf(stderr, "%s[%d][%s]:   ILLEGAL:  SYNC THREAD waiting on for a signal\n", FILE__, __LINE__, getThreadStr(getExecThreadID()));
    abort();
  }

  EventGate *eg = new EventGate(global_mutex,evts[0]);
  for (unsigned int i = 1; i < evts.size(); ++i) {
    eg->addEvent(evts[i]);
  }
  wait_list.push_back(eg);
  EventRecord result = eg->wait();
  
  bool found = false;
  for (int i = wait_list.size() -1; i >= 0; i--) {
    if (wait_list[i] == eg) {
       found = true;
       wait_list.erase(i,i);
       delete eg;
       break;
    } 
  }

  if (!found) {
     fprintf(stderr, "%s[%d]:  BAD NEWS, somehow lost a pointer to eg\n", FILE__, __LINE__);
  }

  return result.type;
}

eventType SignalHandler::waitForEvent(eventType evt, process *p, dyn_lwp *lwp)
{
  if (getExecThreadID() == getThreadID()) {
    fprintf(stderr, "%s[%d][%s]:   ILLEGAL:  SYNC THREAD waiting on for a signal\n", FILE__, __LINE__, getThreadStr(getExecThreadID()));
    abort();
  }

  EventGate *eg = new EventGate(global_mutex,evt,p, lwp);
  wait_list.push_back(eg);
  EventRecord result = eg->wait();
  
  bool found = false;
  for (int i = wait_list.size() -1; i >= 0; i--) {
    if (wait_list[i] == eg) {
       found = true;
       wait_list.erase(i,i);
       delete eg;
       break;
    } 
  }

  if (!found) {
     fprintf(stderr, "%s[%d]:  BAD NEWS, somehow lost a pointer to eg\n", FILE__, __LINE__);
  }

  return result.type;
}

signal_handler_location::signal_handler_location(Address addr, unsigned size) :
    addr_(addr),
    size_(size) {}
