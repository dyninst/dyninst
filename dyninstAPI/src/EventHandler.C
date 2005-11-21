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


#if defined(__XLC__) || defined(__xlC__)
#include "common/h/EventHandler.h"
#else
#pragma implementation "EventHandler.h"
#endif

#include "mailbox.h"
#include "BPatch_eventLock.h"
#include "util.h"
#include "common/h/Vector.h"
#include "debuggerinterface.h"
#include "process.h"


pdvector< pdpair <unsigned long, const char *> > threadmap;
const char *getThreadStr(unsigned long tid)
{
  if (!threadmap.size() && (getExecThreadID() == primary_thread_id)) {
    pdpair<unsigned long, const char *> trec;
#if defined(os_windows)
    trec.first = (unsigned long) _threadid;
#else
    trec.first = (unsigned long) pthread_self();
#endif
    trec.second = "UI";
    threadmap.push_back(trec);
  }

  for (unsigned int i = 0; i < threadmap.size(); ++i) {
    if (threadmap[i].first == tid)
      return threadmap[i].second;
  }
  if (tid == -1UL) 
    return "any_thread";
  fprintf(stderr, "%s[%d]:  FIXME, no entry found for thread id %lu\n", __FILE__, __LINE__, tid);
  return "invalid";
}

unsigned long getExecThreadID() 
{
#if defined (os_windows)
  return (unsigned long) _threadid;
#else
  return (unsigned long) pthread_self();
#endif
}

char *EventRecord::sprint_event(char *buf)
 {
    sprintf(buf, "[%s:proc=%d:lwp=%d:%d:%d:%d:%p:%d]", eventType2str(type),
            proc ? proc->getPid() : 0, lwp ? lwp->get_lwp_id() : 0, what,
            (int) status, (int)info, (void *) address, fd);
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
     if ((info != src.info) && (info != NULL_INFO_INITIALIZER)) {
       return false;
     }

     return true;
}

//  A wrapper for pthread_create, or its equivalent.

inline THREAD_RETURN eventHandlerWrapper(void *h)
{
  startup_printf("%s[%d]:  about to call main() for %s\n", __FILE__, __LINE__, ((EventHandler<EventRecord> *)h)->idstr);
  ((EventHandler<EventRecord> * )h)->main();
  DO_THREAD_RETURN;
}

template <class S>
InternalThread<S>::InternalThread(const char *id) :
  _isRunning(false),
  tid ((unsigned long ) -1)
{
  idstr = strdup(id);
}

template <class S>
InternalThread<S>::~InternalThread() 
{
  if (isRunning()) {
    if (!killThread()) {
       fprintf(stderr, "%s[%d]:  failed to terminate internalThread\n", __FILE__, __LINE__);
    }
  }
  tid = (unsigned long) -1L;
  free (idstr);
}

template <class S>
bool InternalThread<S>::createThread()
{
  mailbox_printf("%s[%d]  welcome to createThread(%s)\n", __FILE__, __LINE__, idstr);
  if (isRunning()) {
     fprintf(stderr, "%s[%d]:  WARNING:  cannot create thread '%s'which is already running\n", 
             __FILE__, __LINE__, idstr);
     return true;
  }
  
#if defined(os_windows)
  fprintf(stderr, "%s[%d]:  about to start thread\n", __FILE__, __LINE__);
  handler_thread = _beginthread(eventHandlerWrapper<T>, 0, (void *) this);
  if (-1L == handler_thread) {
    bperr("%s[%d]:  _beginthread(...) failed\n", __FILE__, __LINE__);
    fprintf(stderr,"%s[%d]:  _beginthread(...) failed\n", __FILE__, __LINE__);
    return false;
  }
  fprintf(stderr, "%s[%d]:  started thread\n", __FILE__, __LINE__);
  return true;
#else  // Unixes

  int err = 0;
  pthread_attr_t handler_thread_attr;

  err = pthread_attr_init(&handler_thread_attr);
  if (err) {
    bperr("%s[%d]:  could not init async handler thread attributes: %s, %d\n",
          __FILE__, __LINE__, strerror(err), err);
    return false;
  }

#if defined (os_solaris) 
  err = pthread_attr_setdetachstate(&handler_thread_attr, PTHREAD_CREATE_DETACHED);
  if (err) {
    bperr("%s[%d]:  could not set async handler thread attrixibcutes: %s, %d\n",
          __FILE__, __LINE__, strerror(err), err);
    return false;
  }
#endif
  startupLock = new eventLock();
  startupLock->_Lock(__FILE__, __LINE__);
  try {
  err = pthread_create(&handler_thread, &handler_thread_attr,
                       &eventHandlerWrapper, (void *) this);
  if (err) {
    bperr("%s[%d]:  could not start async handler thread: %s, %d\n",
          __FILE__, __LINE__, strerror(err), err);
    fprintf(stderr,"%s[%d]:  could not start async handler thread: %s, %d\n",
          __FILE__, __LINE__, strerror(err), err);
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

  while (!_isRunning) {
    startup_printf("%s[%d]:  createThread (%s) waiting for thread main to start\n", __FILE__, __LINE__, idstr);
   startupLock->_WaitForSignal(__FILE__, __LINE__);
   startup_printf("%s[%d]:  createThread (%s) got signal\n", __FILE__, __LINE__, idstr);
  }
  startupLock->_Unlock(__FILE__, __LINE__);
  return true;
#endif

}

template <class S>
bool InternalThread<S>::killThread()
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
  InternalThread<T>(id),
  eventlock(_lock),
  stop_request(false)
{
  //  presume that event handler is created on the ui thread, so make an entry
  if (!threadmap.size()) {
    pdpair<unsigned long, const char *> trec;
    trec.first = getExecThreadID();
    trec.second = "UI";
    threadmap.push_back(trec);
  }
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

//bool EventHandler::waitNextEvent(EventRecord &ev)
//bool EventHandler::handleEvent(EventRecord &ev)
template <class T>
void EventHandler<T>::main()
{
  
  startup_printf("%s[%d]:  welcome to main() for %s\n", __FILE__, __LINE__, idstr);
  pdpair<unsigned long, const char *> trec;
  trec.first = getExecThreadID();
  trec.second = (const char *) idstr;
  threadmap.push_back(trec);
  startup_printf("%s[%d]:  new thread id %lu -- %s\n", __FILE__, __LINE__, trec.first, trec.second);
  tid = trec.first;

  startupLock->_Lock(__FILE__, __LINE__);
  _isRunning = true;
  startupLock->_Broadcast(__FILE__, __LINE__);
  startupLock->_Unlock(__FILE__, __LINE__);

  T ev;

  while (1) {
    //fprintf(stderr, "%s[%d]:  %s waiting for an event\n", __FILE__, __LINE__, idstr);
    if (!this->waitNextEvent(ev)) {
       fprintf(stderr, "%s[%d][%s]:  waitNextEvent failed \n", __FILE__, __LINE__,getThreadStr(getExecThreadID()));
       if (!stop_request)
         continue;
    }
    if (stop_request) {
      fprintf(stderr, "%s[%d]:  thread terminating at stop request\n", __FILE__, __LINE__);
      break;
    }
    if (!handleEvent(ev)) {
       
      fprintf(stderr, "%s[%d][%s]:  handleEvent() failed\n", __FILE__, __LINE__,  getThreadStr(getExecThreadID()));
    }
  }

  fprintf(stderr, "%s[%d]:  InternalThread::main exiting\n", __FILE__, __LINE__);
  _isRunning = false;
}

//EventHandler::
//EventHandler::
//EventHandler::
#if !defined (CASE_RETURN_STR)
#define CASE_RETURN_STR(x) case x: return #x
#endif
char *eventType2str(eventType x)
{
  switch(x) {
  CASE_RETURN_STR(evtUndefined);
  CASE_RETURN_STR(evtNullEvent);
  CASE_RETURN_STR(evtAnyEvent);
  CASE_RETURN_STR(evtNewConnection);
  CASE_RETURN_STR(evtTimeout);
  CASE_RETURN_STR(evtSignalled);
  CASE_RETURN_STR(evtException);
  CASE_RETURN_STR(evtProcessCreate);
  CASE_RETURN_STR(evtProcessExit); /* used to have exited normally, or via signal, now in info */
  CASE_RETURN_STR(evtProcessStop);
  CASE_RETURN_STR(evtProcessSelfTermination);
  CASE_RETURN_STR(evtThreadCreate);
  CASE_RETURN_STR(evtThreadExit);
  CASE_RETURN_STR(evtThreadContextStart);
  CASE_RETURN_STR(evtThreadContextStop);
  CASE_RETURN_STR(evtLoadLibrary);
  CASE_RETURN_STR(evtSyscallEntry);
  CASE_RETURN_STR(evtSyscallExit);
  CASE_RETURN_STR(evtSuspended);
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
  default:
    fprintf(stderr, "%s[%d]:  unknown event type\n", __FILE__, __LINE__);
  }
  return "unknown_event_type";
}


//  OK -- these template instantiations probably belong more rightly
//  in templates2.C, however, including them here gets around
//  multiple definition problems introduced by having both template
//  and non-template functions in this file.
template class InternalThread<EventRecord>;
template class EventHandler<EventRecord>;
template class InternalThread<DBIEvent>;
template class EventHandler<DBIEvent>;
