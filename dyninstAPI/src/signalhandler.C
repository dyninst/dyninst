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

EventGate::EventGate(eventLock *l, eventType t, process *p, dyn_lwp *lwp,
                     eventStatusCode_t status) :
     lock(l), waiting(false)
{
  cond = new eventCond(lock);
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
  delete cond;
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

  //getMailbox()->executeCallbacks(FILE__, __LINE__);
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

SignalGenerator::SignalGenerator() :
                 EventHandler<EventRecord>(BPatch_eventLock::getLock(),
                 "SYNC",/*start thread?*/ false)
{
  signal_printf("%s[%d]:  new SignalGenerator\n", FILE__, __LINE__);
}

SignalGenerator::~SignalGenerator() 
{
  fprintf(stderr, "%s[%d]:  welcome to ~SignalGenerator\n", FILE__, __LINE__);
  killThread();

  for (unsigned int i = 0; i < handlers.size(); ++i) {
    signal_printf("%s[%d]:  destroying handler %s\n", FILE__, __LINE__, 
                  handlers[i]->getName());
    delete handlers[i];
  }
}

bool SignalGenerator::handleEventLocked(EventRecord &ev)
{
  char buf[128];
  signal_printf("%s[%d]:  dispatching event %s\n", FILE__, __LINE__,
                ev.sprint_event(buf));

  if (ev.type == evtUndefined) {
    fprintf(stderr, "%s[%d]:  CHECK THIS, undefined event\n", FILE__, __LINE__); 
    return true;
  }

  SignalHandler *sh = NULL;
  for (unsigned int i = 0; i < handlers.size(); ++i) {
    if (handlers[i]->assignEvent(ev)) {
      sh = handlers[i];
      break;
    }
  }

  if ((sh) && (handlers.size() > HANDLER_TRIM_THRESH)) {
    for (int i = handlers.size() - 1; i >= 0; i--) {
      if (!handlers[i]->idle()) break;
      if (handlers[i] != sh) {
        signal_printf("%s[%d]:  trimming idle signal handler %s\n", FILE__, __LINE__,
                      handlers[i]->getName());
        delete handlers[i];
        handlers.erase(i,i);
      }
    }
  }

  if (handlers.size() > MAX_HANDLERS) {
     fprintf(stderr, "%s[%d]:  FATAL:  Something is horribly wrong.\n", FILE__, __LINE__);
     fprintf(stderr, "\thave %d signal handlers, max is %d\n", 
             handlers.size(), MAX_HANDLERS);
     abort();
  }

  bool ret = true;
  if (!sh) {
    int shid = handlers.size();
    char shname[16];
    sprintf(shname, "SH-%d", shid);
    signal_printf("%s[%d]:  about to create event handler %s\n", 
                   FILE__, __LINE__, shname);
    sh = newSignalHandler(shname, shid);
    sh->createThread();
    handlers.push_back(sh);
    ret = sh->assignEvent(ev);
    if (!ret)  {
       char buf[128];
       fprintf(stderr, "%s[%d]:  failed to assign event %s to handler\n", 
               FILE__, __LINE__, ev.sprint_event(buf));
    }
  }


  return ret;
}

bool SignalGenerator::signalActiveProcess()
{         
  bool ret = true;
  if (waiting_for_active_process)
    ret = __BROADCAST;
  return ret;
}   

bool SignalGenerator::signalEvent(EventRecord &ev)
{
  if (ev.type != evtError) {
    char buf[128];
    signal_printf("%s[%d][%s]:  signalEvent(%s)\n", FILE__, __LINE__, 
                  getThreadStr(getExecThreadID()), ev.sprint_event(buf));
  }
  assert(global_mutex->depth());

  getMailbox()->executeCallbacks(FILE__, __LINE__);

  if (ev.type == evtProcessStop || ev.type == evtProcessExit) {
     //fprintf(stderr, "%s[%d]:  flagging BPatch status change\n", FILE__, __LINE__);
     SignalHandler::flagBPatchStatusChange();
  }

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

bool SignalGenerator::signalEvent(eventType t)
{
  EventRecord ev;
  ev.type = t;
  return signalEvent(ev);
}

eventType SignalGenerator::waitForOneOf(pdvector<eventType> &evts)
{
  assert(global_mutex->depth());

  if (getExecThreadID() == getThreadID()) {
    fprintf(stderr, "%s[%d][%s]:   ILLEGAL:  SYNC THREAD waiting on for a signal\n", 
            FILE__, __LINE__, getThreadStr(getExecThreadID()));
    abort();
  }

  //  When to set wait_flag ??
  //    (1)  If we are running on an event handler thread
  //    (2)  If we are currently running inside a callback AND an 
  //         event handler is waiting for the completion of this callback
  SignalHandler *sh = findSHWithThreadID(getExecThreadID());
  if (sh)
    sh->wait_flag = true;
  else {
    CallbackBase *cb = NULL;
    if (NULL != (cb = getMailbox()->runningInsideCallback())) {
      sh = findSHWaitingForCallback(cb);
      if (sh)
        sh->wait_flag = true;
    }
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
     fprintf(stderr, "%s[%d]:  BAD NEWS, somehow lost a pointer to eg\n", 
             FILE__, __LINE__);
  }

  if (sh)
    sh->wait_flag = false;
  return result.type;
}

eventType SignalGenerator::waitForEvent(eventType evt, process *p, dyn_lwp *lwp,
                                        eventStatusCode_t status)
{
  if (getExecThreadID() == getThreadID()) {
    fprintf(stderr, "%s[%d][%s]:   ILLEGAL:  SYNC THREAD waiting on for a signal: %s\n", 
            FILE__, __LINE__, getThreadStr(getExecThreadID()), eventType2str(evt));
    abort();
  }

  //  When to set wait_flag ??
  //    (1)  If we are running on an event handler thread
  //    (2)  If we are currently running inside a callback AND a 
  //         signal handler is waiting for the completion of this callback
  SignalHandler *sh = findSHWithThreadID(getExecThreadID());
  if (sh)
    sh->wait_flag = true;
  else {
    CallbackBase *cb = NULL;
    if (NULL != (cb = getMailbox()->runningInsideCallback())) {
      sh = findSHWaitingForCallback(cb);
      if (sh)
        sh->wait_flag = true;
    }
  }

  EventGate *eg = new EventGate(global_mutex,evt,p, lwp, status);
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
     fprintf(stderr, "%s[%d]:  BAD NEWS, somehow lost a pointer to eg\n", 
             FILE__, __LINE__);
  }

  if (sh)
    sh->wait_flag = false;
  return result.type;
}

SignalHandler *SignalGenerator::findSHWithThreadID(unsigned long tid)
{
  for (unsigned int i = 0; i < handlers.size(); ++i) {
    if (handlers[i]->getThreadID() == tid)
      return handlers[i];
  }
  return NULL;
}

SignalHandler *SignalGenerator::findSHWaitingForCallback(CallbackBase *cb)
{
  for (unsigned int i = 0; i < handlers.size(); ++i) {
    if (handlers[i]->wait_cb == cb)
      return handlers[i];
  }
  return NULL;
}

bool SignalGenerator::activeHandlerForProcess(process *p)
{
  for (unsigned int i = 0; i < handlers.size(); ++i) {
    if (handlers[i]->activeProcess() == p)
      return true;
  }
  return false;
}

bool SignalGenerator::anyActiveHandlers()
{
  for (unsigned int i = 0; i < handlers.size(); ++i) {
    if (!handlers[i]->idle())
      return true;
  }
  return false;
}

SignalHandler::~SignalHandler()
{
   fprintf(stderr, "%s[%d]:  welcome to ~SignalHandler\n", FILE__, __LINE__);
   if (idle_flag || wait_flag) {
     // maybe a bit heavy handed here....
     killThread();
   }else {
     fprintf(stderr, "%s[%d]:  waiting for idle before killing thread %s\n", 
             FILE__, __LINE__);
     int timeout = 2000 /*ms*/, time_elapsed = 0;
     while (!idle_flag && !wait_flag) {
       struct timeval slp;
       slp.tv_sec = 0;
       slp.tv_usec = 10 /*ms*/ *1000;
       select(0, NULL, NULL, NULL, &slp);
       time_elapsed +=10;
       if (time_elapsed >= timeout) {
         fprintf(stderr, "%s[%d]:  cannot kill thread %s, did not become idle\n", FILE__, __LINE__);
         break;
       }
     }
     if (idle_flag || wait_flag) {
       killThread();
     }
   }
}

bool SignalHandler::idle()
{
  bool ret;
  _Lock(FILE__, __LINE__);
  ret = idle_flag;
  _Unlock(FILE__, __LINE__);
  return ret;
}

bool SignalHandler::waiting()
{
  bool ret;
  _Lock(FILE__, __LINE__);
  ret = wait_flag;
  _Unlock(FILE__, __LINE__);
  return ret;
}

bool SignalHandler::assignEvent(EventRecord &ev) 
{
  char buf[128];
  bool ret = false;
  assert(global_mutex->depth());

  //  after we get the lock, the handler thread should be either idle, or waiting
  //  for some event.  

  while (!idle_flag) {
    if (wait_flag) {
      signal_printf("%s[%d]:  cannot assign event %s to %s, while it is waiting\n", 
                    FILE__, __LINE__, ev.sprint_event(buf), getName());
      return false;
    }
    signal_printf("%s[%d]:  shoving event %s into the queue for %s\n",
         FILE__, __LINE__, ev.sprint_event(buf), getName());
    events_to_handle.push_back(ev);
    if (ev.type == evtProcessExit) 
      getSH()->signalEvent(ev);
    else 
      _Broadcast(FILE__, __LINE__);
    return true;
  }

  if (idle_flag) {
    // handler thread is not doing anything, assign away...
    signal_printf("%s[%d]:  assigning event %s to %s\n", FILE__, __LINE__,
                  ev.sprint_event(buf), getName());
    events_to_handle.push_back(ev);
    ret = true;
    idle_flag = false;
    _Broadcast(FILE__, __LINE__);
  } 
 // else {
 //   char buf[128];
 //   fprintf(stderr, "%s[%d]:  WEIRD, tried to assign %s to busy handler\n",
 //          FILE__, __LINE__, ev.sprint_event(buf));
 // } 

  return ret;
}

bool SignalHandler::waitNextEvent(EventRecord &ev)
{
  bool ret;
  _Lock(FILE__, __LINE__);
  while (idle_flag) {
    signal_printf("%s[%d]:  handler %s waiting for something to do\n", 
                  FILE__, __LINE__, getName());
    if (!getSH()->anyActiveHandlers() && getSH()->waitingForActiveProcess()) {
      //flagBPatchStatusChange();
      getSH()->signalEvent(evtProcessStop);
      //_Broadcast(FILE__, __LINE__);
    }
    _WaitForSignal(FILE__, __LINE__);
    signal_printf("%s[%d]:  handler %s has been signalled: got event = %s\n", 
                  FILE__, __LINE__, getName(), idle_flag ? "false" : "true");
  }

  ret = true;
  ev = events_to_handle[0];
  events_to_handle.erase(0,0);
  active_proc = ev.proc;

  if (ev.type == evtUndefined) {
    fprintf(stderr, "%s[%d]:  got evtUndefined for next event!\n", FILE__, __LINE__);
    ret = false;
  }

  _Unlock(FILE__, __LINE__);
  if (!ret) abort();
  return ret;
}

signal_handler_location::signal_handler_location(Address addr, unsigned size) :
    addr_(addr),
    size_(size) {}
