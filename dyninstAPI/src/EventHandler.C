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


#if defined(__XLC__) || defined(__xlC__)
#include "common/h/EventHandler.h"
#else
#if !defined (os_windows)
#pragma implementation "EventHandler.h"
#endif
#endif

#include "mailbox.h"
#include "BPatch_eventLock.h"
#include "util.h"
#include "common/h/Vector.h"
#include "debuggerinterface.h"
#include "process.h"
#include "common/h/Dictionary.h"

using namespace Dyninst;

eventLock *threadMapLock = NULL;

dictionary_hash<Address, threadmap_t *> *threadmap;

void initializeThreadMap() {
    if (threadMapLock != NULL) return;

    threadMapLock  = new eventLock;
    threadMapLock->_Lock(FILE__, __LINE__);

    threadmap = new dictionary_hash<Address, threadmap_t *>(addrHash4);

    assert(threadmap->size() == 0);
    assert(getExecThreadID() == primary_thread_id);

    // Initialization
    threadmap_t *t = new threadmap_t;
    t->active = true;
    t->name = P_strdup("UI");
#if defined(os_windows)
    (*threadmap)[_threadid] = t;
#else
    (*threadmap)[getExecThreadID()] = t;
#endif

    threadMapLock->_Unlock(FILE__, __LINE__);
}

const char *defaultThreadName = "INVALID";
const char *anyThreadName = "any thread";

const char *getThreadStr(unsigned long tid)
{
    //fprintf(stderr, "... getThreadStr for 0x%lx\n", tid);
    initializeThreadMap();

    const char *retval = defaultThreadName;
  
    threadMapLock->_Lock(FILE__, __LINE__);

    if (threadmap->defines(tid)) {
        retval = (*threadmap)[tid]->name;
    }
    else if (tid == (unsigned long) -1L)  {
        retval = anyThreadName;
    }
    // Else... could be calling this before the thread is named.

    //fprintf(stderr, "... returning string %s\n", retval);
    threadMapLock->_Unlock(FILE__, __LINE__);
    
    return retval;
}

unsigned long getExecThreadID() 
{
#if defined (os_windows)
  return (unsigned long) _threadid;
#else
  return (unsigned long) pthread_self();
#endif
}

EventRecord::EventRecord() :
      proc(NULL), 
      lwp(NULL), 
      type(evtUndefined), 
      what(0),
      status(NULL_STATUS_INITIALIZER),
      address(0), 
      fd(0)
{
#if defined (os_windows)
  info.dwDebugEventCode = 0;
  info.dwProcessId = 0;
  info.dwThreadId = 0;
#else
  info = 0;
#endif
}

char *EventRecord::sprint_event(char *buf)
 {
    int pid = -1;
    if (proc && proc->sh && proc->status() != deleted) pid = proc->getPid();
    sprintf(buf, "[%s:proc=%d:lwp=%d:what=%d:status=%d:addr=%p:fd=%d]", eventType2str(type),
            pid, lwp ? lwp->get_lwp_id() : 0, what,
            (int) status, (void *) address, fd);
    return buf;
}

bool EventRecord::isTemplateOf(EventRecord &src) 
{
     //  returns true if key non-default member values match src
     if ((type != src.type) && (type != evtAnyEvent) && (type != evtUndefined)) {
       return false; 
     }
     //  maybe we should compare pid instead of proc ptr??
     if ((proc != src.proc) && (proc != NULL)) {
       return false;
     }
     //  maybe we should compare lwpid instead of lwp ptr??
     if ((lwp != src.lwp) && (lwp != NULL)) {
       return false;
     }
     if ((what != src.what) && (what != 0)){
       return false;
     }
     if ((status != src.status) && (status != NULL_STATUS_INITIALIZER)) {
       return false;
     }
#if 0
    // dont want to compare info, since this is either the raw status output
    // from waitpid (or waitForDebugEvent), or the argument from a runtime
    // library call -- eg the pid of a forked process.

     if ((info != src.info) && (info != NULL_INFO_INITIALIZER)) {
       return false;
     }
#endif
     return true;
}

//  A wrapper for pthread_create, or its equivalent.

inline THREAD_RETURN eventHandlerWrapper(void *h)
{
    thread_printf("%s[%d]:  about to call main() for %s\n", __FILE__, __LINE__, ((EventHandler<EventRecord> *)h)->idstr);
  ((EventHandler<EventRecord> * )h)->main();
  DO_THREAD_RETURN;
}

InternalThread::InternalThread(const char *id) :
  _isRunning(false),
  tid ((unsigned long ) -1),
  init_ok(true)
{
  idstr = P_strdup(id);
}

InternalThread::~InternalThread() 
{
  if (isRunning()) {
    //if (!killThread()) {
    //   fprintf(stderr, "%s[%d]:  failed to terminate internalThread\n", __FILE__, __LINE__);
   // }
  }
  tid = (unsigned long) -1L;
  free (idstr);
}

bool InternalThread::createThread()
{
    thread_printf("%s[%d]  welcome to createThread(%s)\n", __FILE__, __LINE__, idstr);
  if (isRunning()) {
     fprintf(stderr, "%s[%d]:  WARNING:  cannot create thread '%s'which is already running\n", 
             __FILE__, __LINE__, idstr);
     return true;
  }
  
  startupLock = new eventLock();
  startupLock->_Lock(__FILE__, __LINE__);

#if defined(os_windows)
  handler_thread = _beginthread(eventHandlerWrapper, 0, (void *) this);
  if (-1L == handler_thread) {
    bperr("%s[%d]:  _beginthread(...) failed\n", __FILE__, __LINE__);
    return false;
  }
#else  // Unixes

  int err = 0;
  pthread_attr_t handler_thread_attr;

  err = pthread_attr_init(&handler_thread_attr);
  if (err) {
    bperr("%s[%d]:  could not init async handler thread attributes: %s, %d\n",
          __FILE__, __LINE__, strerror(err), err);
    return false;
  }

  err = pthread_attr_setdetachstate(&handler_thread_attr, PTHREAD_CREATE_DETACHED);
  if (err) {
    bperr("%s[%d]:  could not set async handler thread attrixibcutes: %s, %d\n",
          __FILE__, __LINE__, strerror(err), err);
    return false;
  }

  try {
	  int n_try = 0;
try_again:
  err = pthread_create(&handler_thread, &handler_thread_attr,
                       &eventHandlerWrapper, (void *) this);
  if (err) {
    bperr("%s[%d]:  could not start async handler thread: %s, %d\n",
          __FILE__, __LINE__, strerror(err), err);
    fprintf(stderr,"%s[%d]:  could not start async handler thread: %s, %d\n",
          __FILE__, __LINE__, strerror(err), err);
	if (err == EAGAIN)
	{
		struct timeval slp;
		slp.tv_sec = 0;
		slp.tv_usec = 1000;
		select(0, NULL, NULL, NULL, &slp);
		n_try++;
		if (n_try < 10)
			goto try_again;
		else
		{
			fprintf(stderr,"%s[%d]:  FAIL:  giving up on async handler thread: %s, %d\n",
					__FILE__, __LINE__, strerror(err), err);
		}
	}
	return false;
  }
  } catch(...) {
    assert(0);
  }

  err = pthread_attr_destroy(&handler_thread_attr);
  if (err) {
    bperr("%s[%d]:  could not destroy async handler attr: %s, %d\n",
          __FILE__, __LINE__, strerror(err), err);
    return false;
  }
#endif

  while (!_isRunning && (init_ok)) {
      thread_printf("%s[%d]:  createThread (%s) waiting for thread main to start\n", __FILE__, __LINE__, idstr);
      startupLock->_WaitForSignal(__FILE__, __LINE__);
      thread_printf("%s[%d]:  createThread (%s) got signal\n", __FILE__, __LINE__, idstr);
  }
  startupLock->_Unlock(__FILE__, __LINE__);

  thread_printf("%s[%d]: createThread returning %d\n", FILE__, __LINE__, init_ok);

  if (!init_ok) {
    return false;
  }
  return true;

}

bool InternalThread::killThread()
{
  if (!isRunning()) {
    fprintf(stderr, "%s[%d]:  request to kill already-stopped thread\n", __FILE__, __LINE__);
    return true;
  }

#if defined(os_windows)
  fprintf(stderr, "%s[%d]:  cannot kill threads on windows\n", __FILE__, __LINE__);
  return false;
#else
  int killres;
  killres = pthread_kill(handler_thread, 9);
  if (killres) {
     fprintf(stderr, "%s[%d]:  pthread_kill: %s[%d]\n", __FILE__, __LINE__,
             strerror(killres), killres);
     return false;
  }
  fprintf(stderr, "%s[%d]:  \t\t..... killed.\n", __FILE__, __LINE__);
  _isRunning = false;
#endif
  return true; 
}

template <class T>
EventHandler<T>::EventHandler(eventLock *_lock, const char *id, bool create) :
  InternalThread(id),
  eventlock(_lock),
  stop_request(false),
  usage_count(0)
{
    //  presume that event handler is created on the ui thread, so make an entry
    initializeThreadMap();
    
    if (create) 
        if (!createThread()) {
            fprintf(stderr, "%s[%d]:  failed to create InternalThread\n", __FILE__, __LINE__);
        }
}

template <class T>
EventHandler<T>::~EventHandler()
{
}

template <class T>
bool EventHandler<T>::_Lock(const char *__file__, unsigned int __line__) 
{
  return eventlock->_Lock(__file__, __line__);
}
template <class T>
bool EventHandler<T>::_Unlock(const char *__file__, unsigned int __line__) 
{
  return eventlock->_Unlock(__file__, __line__);
}
template <class T>
bool EventHandler<T>::_WaitForSignal(const char *__file__, unsigned int __line__) 
{
  return eventlock->_WaitForSignal(__file__, __line__);
}
template <class T>
bool EventHandler<T>::_Broadcast(const char *__file__, unsigned int __line__) 
{
  return eventlock->_Broadcast(__file__, __line__);
}

template <class T>
void EventHandler<T>::main()
{
    MONITOR_ENTRY();

    addToThreadMap();

    thread_printf("%s[%d]:  welcome to main() for %s\n", __FILE__, __LINE__, idstr);
    thread_printf("%s[%d]:  new thread id %lu -- %s\n", __FILE__, __LINE__, tid, idstr);

    startupLock->_Lock(__FILE__, __LINE__);
    thread_printf("%s[%d]:  about to do init for %s\n", __FILE__, __LINE__, idstr);
    if (!initialize_event_handler()) {
        _isRunning = false;
        init_ok = false; 

        removeFromThreadMap();

        startupLock->_Broadcast(__FILE__, __LINE__);
        startupLock->_Unlock(__FILE__, __LINE__);
        return;
    }
    
    init_ok = true;
    thread_printf("%s[%d]:  init success for %s\n", __FILE__, __LINE__, idstr);
    
    _isRunning = true;
    startupLock->_Broadcast(__FILE__, __LINE__);
    startupLock->_Unlock(__FILE__, __LINE__);
    
    T ev;
    
    thread_printf("%s[%d]:  before main loop for %s\n", __FILE__, __LINE__, idstr);
    while (1) {
        if (!this->waitNextEvent(ev)) {
            fprintf(stderr, "%s[%d][%s]:  waitNextEvent failed \n", __FILE__, __LINE__,getThreadStr(getExecThreadID()));
            if (!stop_request)
                continue;
        }
        if (stop_request) {
            thread_printf("%s[%d]:  thread terminating at stop request\n", __FILE__, __LINE__);
            break;
        }
        if (!handleEvent(ev)) {
            
            fprintf(stderr, "%s[%d][%s]:  handleEvent() failed\n", __FILE__, __LINE__,  getThreadStr(getExecThreadID()));
        }
        if (stop_request) break;
    }
 
   global_mutex->_Lock(FILE__, __LINE__);
 
    removeFromThreadMap();
    
    _isRunning = false;
    if (global_mutex->depth() != 1) {
        fprintf(stderr, "%s[%d]:  WARNING:  global_mutex->depth() is %d, leaving thread %s\n",
                FILE__, __LINE__, global_mutex->depth(),idstr);
        global_mutex->printLockStack();
    }
    assert(global_mutex->depth() == 1);
    global_mutex->_Broadcast(FILE__, __LINE__);
    global_mutex->_Unlock(FILE__, __LINE__);
    
    thread_printf("%s[%d][%s]:  InternalThread::main exiting\n", FILE__, __LINE__, idstr);

    MONITOR_EXIT();
}

template <class T>
void EventHandler<T>::addToThreadMap() 
{
    assert(tid == (unsigned long) -1);
    assert(threadMapLock != NULL);
    tid = getExecThreadID();

    threadMapLock->_Lock(FILE__, __LINE__);

    if (threadmap->defines(tid)) {
        // Can happen if we reuse threads... nuke the old.
        if ((*threadmap)[tid]->active == true) {
            // Weird...
            fprintf(stderr, "Warning: replacing thread %s that's still marked as active\n",
                    (*threadmap)[tid]->name);
        }

        assert((*threadmap)[tid]->active == false);
        // We create a new name when we deactivate - delete it here.
        free((*threadmap)[tid]->name);
        threadmap_t *foo = (*threadmap)[tid];
        threadmap->undef(tid);
        delete foo;
    }

    threadmap_t *t = new threadmap_t;
    t->name = P_strdup(idstr);
    t->active = true;
    (*threadmap)[tid] = t;
    
    threadMapLock->_Unlock(FILE__, __LINE__);
}

template <class T>
void EventHandler<T>::removeFromThreadMap() 
{
    //  remove ourselves from the threadmap before exiting
    assert(threadMapLock != NULL);
    threadMapLock->_Lock(FILE__, __LINE__);

    if (threadmap->defines(getExecThreadID())) {
        (*threadmap)[getExecThreadID()]->active = false;

        // We mark the fact that the thread is deleted by changing out the name.
        char *oldname = (*threadmap)[getExecThreadID()]->name;
        assert(oldname);
        char *newname = (char *)malloc(strlen(oldname) + strlen("-DELETED") + 1);
        sprintf(newname, "%s-DELETED", oldname);
        (*threadmap)[getExecThreadID()]->name = newname;

        free(oldname);
    }
    
    threadMapLock->_Unlock(FILE__, __LINE__);
}

template <class T>
void EventHandler<T>::setName(char *newIdStr) 
{
    free(idstr);
    idstr = P_strdup(newIdStr);

    // Update the thread map
    if (threadmap->defines(getExecThreadID())) {
        free((*threadmap)[getExecThreadID()]->name);
        (*threadmap)[getExecThreadID()]->name = P_strdup(idstr);
    }
    else {
        fprintf(stderr, "ERROR: threadMap does not contain name for thread %lu (%s)\n",
                getExecThreadID(), idstr);
    }
}


#if !defined (CASE_RETURN_STR)
#define CASE_RETURN_STR(x) case x: return #x
#endif

const char *eventType2str(eventType x)
{
  switch(x) {
  CASE_RETURN_STR(evtUndefined);
  CASE_RETURN_STR(evtNullEvent);
  CASE_RETURN_STR(evtIgnore);
  CASE_RETURN_STR(evtAnyEvent);
  CASE_RETURN_STR(evtNewConnection);
  CASE_RETURN_STR(evtTimeout);
  CASE_RETURN_STR(evtSignalled);
  CASE_RETURN_STR(evtException);
  CASE_RETURN_STR(evtCritical);
  CASE_RETURN_STR(evtProcessCreate);
  CASE_RETURN_STR(evtProcessAttach);
  CASE_RETURN_STR(evtProcessExit); /* used to have exited normally, or via signal, now in info */
  CASE_RETURN_STR(evtProcessStop);
  CASE_RETURN_STR(evtProcessSelfTermination);
  CASE_RETURN_STR(evtThreadCreate);
  CASE_RETURN_STR(evtThreadExit);
  CASE_RETURN_STR(evtThreadContextStart);
  CASE_RETURN_STR(evtThreadContextStop);
  CASE_RETURN_STR(evtLoadLibrary);
  CASE_RETURN_STR(evtUnloadLibrary);
  CASE_RETURN_STR(evtSyscallEntry);
  CASE_RETURN_STR(evtSyscallExit);
  CASE_RETURN_STR(evtSuspended);
  CASE_RETURN_STR(evtRequestedStop);
  CASE_RETURN_STR(evtInstPointTrap);
  CASE_RETURN_STR(evtDebugStep);
  CASE_RETURN_STR(evtDynamicCall);
  CASE_RETURN_STR(evtRPCSignal);
  CASE_RETURN_STR(evtError);
  CASE_RETURN_STR(evtPreFork);
  CASE_RETURN_STR(evtPostFork);
  CASE_RETURN_STR(evtExec);
  CASE_RETURN_STR(evtOneTimeCode);
  CASE_RETURN_STR(evtUserEvent);
  CASE_RETURN_STR(evtShutDown);
  CASE_RETURN_STR(evtProcessInit);
  CASE_RETURN_STR(evtProcessLoadedRT);
  CASE_RETURN_STR(evtProcessInitDone);
  CASE_RETURN_STR(evtThreadDetect);
  CASE_RETURN_STR(evtLastEvent);
  CASE_RETURN_STR(evtLwpAttach);
  CASE_RETURN_STR(evtLibcLoaded);
  CASE_RETURN_STR(evtStopThread);
  CASE_RETURN_STR(evtSignalHandlerCB);
  CASE_RETURN_STR(evtCodeOverwrite);
  CASE_RETURN_STR(evtEmulatePOPAD);
  CASE_RETURN_STR(evtLibcTrap);
  default:
    fprintf(stderr, "%s[%d]:  unknown event type\n", FILE__, __LINE__);
  }
  return "unknown_event_type";
}

template <class T> 
void EventHandler<T>::MONITOR_ENTRY() {
    // These should do something, but I'm concerned about
    // changing a top-level object. So instead I'm
    // specializing SignalGeneratorCommon.
    usage_count++;
}

template <class T>
void EventHandler<T>::MONITOR_EXIT() {
    usage_count--;
}


//  OK -- these template instantiations probably belong more rightly
//  in templates2.C, however, including them here gets around
//  multiple definition problems introduced by having both template
//  and non-template functions in this file.
template class EventHandler<EventRecord>;
template class EventHandler<DBIEvent>;

