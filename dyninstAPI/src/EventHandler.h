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

#ifndef __EVENT_HANDLER__H__
#define __EVENT_HANDLER__H__

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME EVENT_HANDLER

#include "common/h/Types.h" /*for Address */
#include "common/h/Pair.h" /*for pdpair */
#include "common/h/Vector.h" /*for pdvector */
#include "common/h/headers.h" /*for DEBUG_EVENT */
#include "common/h/Dictionary.h" /* threadmap */
#include "os.h"
#include "BPatch_eventLock.h"
#include "mailbox.h"

#define NULL_STATUS_INITIALIZER statusUnknown
typedef int eventFileDesc_t;

class process;
class dyn_lwp;

typedef enum {
  evtUndefined,   /* undefined event -- usually indicates error */
  evtNullEvent,   /* null event, nothing (important) happened, but no error */
  evtIgnore, /* Throw this away when decode is done */
  evtAnyEvent,
  evtNewConnection,
  evtTimeout,
  evtSignalled,
  evtException,
  evtCritical,
  evtProcessCreate,
  evtProcessAttach,
  evtLwpAttach,
  evtProcessExit, /* used to have exited normally, or via signal, now in status */
  evtProcessStop,
  evtProcessSelfTermination,
  evtThreadCreate,
  evtThreadExit,
  evtThreadContextStart,
  evtThreadContextStop,
  evtLoadLibrary,
  evtUnloadLibrary,
  evtSyscallEntry,
  evtSyscallExit,
  evtSuspended, 
  evtRequestedStop, // For platforms where "stop" can generate a signal
  evtInstPointTrap,
  evtDebugStep,
  evtDynamicCall,
  evtRPCSignal,
  evtError,
  evtPreFork,
  evtPostFork,
  evtExec,
  evtOneTimeCode,
  evtUserEvent,
  evtShutDown,
  evtLibcLoaded, /* libc has been loaded */
  evtLibcTrap, /* reached trap in __libc_start_main, handle the trap */
  evtStopThread, /* executed stop thread code snippet */
  evtSignalHandlerCB, /* invoke SignalHandlerCallback in response to signal */
  evtCodeOverwrite, /* Analysis invalidated by code being over-written */
  evtProcessInit, /*aka "initialized" */
  evtProcessLoadedRT, /* dyninst RTlib has been loaded */
  evtProcessInitDone, /* aka bootstrapped */
  evtThreadDetect, /* only for linux waitpid loop */
  //  If you are adding to this list, please augment the function eventType2str(),
  //  to reflect your changes.
  evtLastEvent /* placeholder for the end of the list*/
} eventType;

const char *eventType2str(eventType x); 

typedef Address eventAddress_t;

typedef enum {
  statusUnknown,
  statusNormal,
  statusSignalled,
  statusRPCDone,
  statusRPCAtReturn,
  statusError
} eventStatusCode_t;


class EventRecord {
  public:
   EventRecord(); 
   process *proc;
   dyn_lwp *lwp;  // the lwps causing the event (not the representative lwp)
   eventType type;
   eventWhat_t what;
   eventStatusCode_t status;
   eventInfo_t info;
   eventMoreInfo_t info2;
   eventAddress_t address;
   eventFileDesc_t fd; /* only used in async events -- wasteful for clarity*/
   char *sprint_event(char *buf); 
   bool isTemplateOf(EventRecord &src);

   void clear();
}; 

class InternalThread {
  public:
  //  InternalThread::getThreadID(), return thread id
  //  returns tid (unsigned) -1 if thread not created yet
  unsigned long getThreadID() { return tid;}
  bool isRunning() {return _isRunning;}
  const char *getName() {return idstr;}

  internal_thread_t handler_thread;

  protected:
  InternalThread(const char *id);
  virtual ~InternalThread();
  bool createThread();
  bool killThread();

  virtual void main() = 0;

  bool _isRunning;
  unsigned long tid;
  char *idstr;
  eventLock *startupLock;
  
  bool init_ok;

  private:
  
};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME EventHandler

template <class T>
class EventHandler : public InternalThread {
  friend THREAD_RETURN eventHandlerWrapper(void *);
  friend THREAD_RETURN asyncHandlerWrapper(void *);

  public:
  void stopThreadNextIter() {stop_request = true;}
  bool stopRequested() {return stop_request;}
  protected:

  //  initialize_event_handler is called before the main event handling
  //  loop begins on the event handling thread.  When overloaded, allows 
  //  derived classes to do some initialization on the created thread.
  //  Returns false on failure to initialize.  Upon failure, the event loop
  //  is not entered at all and the created thread exits.
  virtual bool initialize_event_handler() {return true;}

  EventHandler(eventLock *_lock, const char *id, bool create_thread);
  virtual ~EventHandler();

  virtual bool waitNextEvent(T &ev) = 0; 
  virtual bool handleEvent(T &ev) = 0;
  
  bool _Lock(const char *__file__, unsigned int __line__); 
  bool _Unlock(const char *__file__, unsigned int __line__); 
  bool _WaitForSignal(const char *__file__, unsigned int __line__); 
  bool _Broadcast(const char *__file__, unsigned int __line__); 

  void addToThreadMap();
  void removeFromThreadMap();
  void setName(char *idstr);

  eventLock *eventlock;
  bool stop_request;
  private:
  void main();

  // Monitor-equivalent functions; call before entering or leaving
  // a blocking function. This allows us to delete the object after
  // the last thread has left. 
  virtual void MONITOR_ENTRY();
  virtual void MONITOR_EXIT();

  unsigned usage_count;
};

typedef struct {
    char *name;
    bool active;
} threadmap_t;

const char *getThreadStr(unsigned long tid);
unsigned long getExecThreadID();

#endif
