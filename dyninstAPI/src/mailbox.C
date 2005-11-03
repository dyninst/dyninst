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

#include "dyninstAPI/src/mailbox.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/showerror.h"
ThreadMailbox *eventmb = NULL;
ThreadMailbox *getMailbox()
{
  if (eventmb == NULL) {
    eventmb = new ThreadMailbox();
  }
  return eventmb;
}

extern eventLock *global_mutex;
eventLock::eventLock()
{
#if defined(os_windows)
  InitializeCriticalSection(&mutex);
//HANDLE CreateEvent(
//  LPSECURITY_ATTRIBUTES lpEventAttributes,
//  BOOL bManualReset,
//  BOOL bInitialState,
//  LPCTSTR lpName
  cond = CreateEvent(NULL, true /*true is manual reset, false for auto*/,
                     false /*initially not in signalled state */,
                     NULL /*name*/);
  assert (cond);
#else
  int err = 0;
  pthread_mutexattr_t mutex_type;
  if (0 != pthread_mutexattr_init(&mutex_type)) {
   abort();
  }
  if (0 != pthread_mutexattr_settype(&mutex_type, PTHREAD_MUTEX_TYPE)) {
    abort();
  }
  if (0 != pthread_mutex_init(&mutex, &mutex_type)) {
    ERROR_BUFFER;
    fprintf(stderr, "%s[%d]:  failed to init mutex: %s[%d]\n",
            FILE__, __LINE__, STRERROR(err, buf), err);
    abort();
  }
  pthread_cond_init(&cond, NULL);
#endif // !Windows
  lock_depth = 0;
}

eventLock::~eventLock()
{
#if defined(os_windows)
  //  need to do something here
  DeleteCriticalSection(&mutex);
  CloseHandle(cond);
#else
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);
#endif
}

int eventLock::_Lock(const char *__file__, unsigned int __line__)
{
  int err = 0;
#if defined(os_windows)
  EnterCriticalSection(&mutex);
  lock_depth++;
#else
  //fprintf(stderr, "%s[%d]: about to lock %p: from %s[%d]\n", FILE__, __LINE__, &mutex, __file__, __line__);
  if(0 != (err = pthread_mutex_lock(&mutex))){
    ERROR_BUFFER;
    fprintf(stderr, "%s[%d]:  failed to lock mutex: %s[%d]\n",
            __file__, __line__, STRERROR(err, buf), err);
  }
  else {
    if (lock_depth) {
      if ((owner_id != getExecThreadID()) && (owner_id != -1UL)) {
        fprintf(stderr, "%s[%d]:  FATAL MUTEX ERROR, lock obtained by 2 threads,\n",
                FILE__, __LINE__); 
        const char *old_owner_name = getThreadStr(owner_id);
        if (!old_owner_name) old_owner_name = "no-name";
        fprintf(stderr, "\tnow: %s[%lu], previous: %s[%lu]\n", getThreadStr(getExecThreadID()),
                getExecThreadID(), old_owner_name, owner_id);
        assert(0);
      }
    }
    owner_id = getExecThreadID();
    lock_depth++;
    lock_stack_elem el;
    el.file  = __file__;
    el.line = __line__;
    lock_stack.push_back(el);
    
  }

#ifdef DEBUG_MUTEX
  fprintf(stderr, "%s[%d]:  lock, depth = %d\n", __file__, __line__, lock_depth);
#endif

#endif
  return err;
}


int eventLock::_Trylock(const char *__file__, unsigned int __line__)
{
  int err = 0;
#if defined(os_windows)
  TryEnterCriticalSection(&mutex);
  lock_depth++;
   //  need to look at result of tryEnter to see if we should increment lock_depth
#else
  if(0 != (err = pthread_mutex_trylock(&mutex))){
    if (EBUSY != err) {
      ERROR_BUFFER;
      //  trylock returns EBUSY immediately when lock cannot be obtained
      fprintf(stderr, "%s[%d]:  failed to trylock mutex: %s[%d]\n",
              __file__, __line__, STRERROR(err, buf), err);
    }
  }
  else {
    if (lock_depth) {
      assert(owner_id == getExecThreadID());
    }
    owner_id = getExecThreadID();
    lock_depth++;
    lock_stack_elem el;
    el.file = __file__;
    el.line = __line__;
    lock_stack.push_back(el);
  }

#endif
  return err;
}

int eventLock::_Unlock(const char *__file__, unsigned int __line__)
{
#if defined(os_windows)
  lock_depth--;
  LeaveCriticalSection(&mutex);

#else
  if (!lock_depth) {
    fprintf(stderr, "%s[%d]:  MUTEX ERROR, attempt to unlock nonlocked mutex, at %s[%d]\n",
            FILE__, __LINE__, __file__, __line__);
  }
  lock_depth--;
  lock_stack_elem el = lock_stack[lock_stack.size() -1];
  lock_stack.pop_back();
  
#ifdef DEBUG_MUTEX
  fprintf(stderr, "%s[%d]:  unlock, lock depth will be %d \n", __file__, __line__, lock_depth);
#endif
  int err = 0;
  if(0 != (err = pthread_mutex_unlock(&mutex))){
    ERROR_BUFFER;
    fprintf(stderr, "%s[%d]:  failed to unlock mutex: %s[%d]\n",
            __file__, __line__, STRERROR(err, buf), err);
    lock_depth++;
    lock_stack.push_back(el);
  }

 if (!lock_depth)
   owner_id = -1UL;

#endif
  return 0;
}

int eventLock::_Broadcast(const char *__file__, unsigned int __line__)
{
#if defined(os_windows)
  int ret = SetEvent(cond);
  assert (ret);
#else
  int err = 0;
  if(0 != (err = pthread_cond_broadcast(&cond))){
    ERROR_BUFFER;
    fprintf(stderr, "%s[%d]:  failed to broadcast cond: %s[%d]\n",
            __file__, __line__, STRERROR(err, buf), err);
  }

#endif
  return 0;
}

int eventLock::_WaitForSignal(const char *__file__, unsigned int __line__)
{
#if defined(os_windows)
  DWORD ret;
  do {
    ret = WaitForSingleObject(cond, INFINITE);
  }while (ret == WAIT_TIMEOUT); 
  if (ret != WAIT_OBJECT_0) {
    assert (0);
  }
  EnterCriticalSection(&mutex);
  ResetEvent(cond);
#else
  int err = 0;
  if (!lock_depth) {
    fprintf(stderr, "%s[%d][%s]: cannot call wait until lock is obtained, see %s[%d]\n", FILE__, __LINE__, getThreadStr(getExecThreadID()), __file__, __line__);
    abort();
  }
  lock_depth--;
  if (lock_depth) {
    const char *thread_name = getThreadStr(getExecThreadID());
    if (!thread_name) thread_name = "unnamed thread";
    assert(__file__);
    assert (FILE__);
    fprintf(stderr, "%s[%d][%s]:  FATAL, cannot wait while recursively locked to depth %d, called from %s[%d]\n",
            FILE__, __LINE__, thread_name, lock_depth +1, __file__, __line__);
    printLockStack();
    abort();
  }
  lock_stack_elem el = lock_stack[lock_stack.size() -1];
  lock_stack.pop_back();
  owner_id = 0;
  if(0 != (err = pthread_cond_wait(&cond, &mutex))){
    ERROR_BUFFER;
    fprintf(stderr, "%s[%d]:  failed to broadcast cond: %s[%d]\n",
            __file__, __line__, STRERROR(err, buf), err);
  }
  lock_depth++;
  lock_stack.push_back(el);
  owner_id = getExecThreadID();

#endif
  return 0;
}

void eventLock::printLockStack()
{
    fprintf(stderr, "%s[%d]:  Lock stack:\n", FILE__, __LINE__);
    for (unsigned int i = 0; i < lock_stack.size(); ++i) {
      fprintf(stderr, "\t[%s][%d]\n", lock_stack[i].file, lock_stack[i].line);
    }
}

eventCond::eventCond(eventLock *l) 
{
  lock = l;
  pthread_cond_init(&cond, NULL);
}
eventCond::~eventCond() 
{
  pthread_cond_destroy(&cond);
}

int eventCond::_Broadcast(const char *__file__, unsigned int __line__)
{
  int err = 0;
  if(0 != (err = pthread_cond_broadcast(&cond))){
    ERROR_BUFFER;
    fprintf(stderr, "%s[%d]:  failed to broadcast cond: %s[%d]\n",
            __file__, __line__, STRERROR(err, buf), err);
  }

  return 0;
}

int eventCond::_WaitForSignal(const char *__file__, unsigned int __line__)
{
  int err = 0;
  if (!lock->depth()) {
    fprintf(stderr, "%s[%d][%s]: cannot call wait until lock is obtained, see %s[%d]\n", FILE__, __LINE__, getThreadStr(getExecThreadID()), __file__, __line__);
    abort();
  }

  if (lock->lock_depth > 1) {
    const char *thread_name = getThreadStr(getExecThreadID());
    if (!thread_name) thread_name = "unnamed thread";
    assert(__file__);
    assert (FILE__);
    fprintf(stderr, "%s[%d][%s]:  FATAL, cannot wait while recursively locked to depth %d, called from %s[%d]\n",
            FILE__, __LINE__, thread_name, lock->lock_depth, __file__, __line__);
    lock->printLockStack();
    abort();
  }

  eventLock::lock_stack_elem el = lock->popLockStack();
  lock->owner_id = 0;

  if(0 != (err = pthread_cond_wait(&cond, &lock->mutex))){
    ERROR_BUFFER;
    fprintf(stderr, "%s[%d]:  failed to broadcast cond: %s[%d]\n",
            __file__, __line__, STRERROR(err, buf), err);
  }
  lock->pushLockStack(el);
  lock->owner_id = getExecThreadID();

  return 0;
}

ThreadMailbox::~ThreadMailbox() 
{
  for (unsigned int i = 0; i < cbs.size(); ++i) {
    delete &(cbs[i]);
  }
  cbs.clear();
}

void ThreadMailbox::executeOrRegisterCallback(CallbackBase *cb) 
{
  //assert(global_mutex->depth());
  CallbackBase *called_cb = executeCallback(cb);
  mb_lock._Lock(FILE__, __LINE__);
  cleanUpCalled();
  if (called_cb) {
    called.push_back(cb);
  } else {
    //  cannot execute now, save for later.
    cbs.push_back(cb);
  }
  mb_lock._Unlock(FILE__, __LINE__);
}

void ThreadMailbox::executeCallbacks(const char *file, unsigned int line) 
{
  if (!global_mutex->depth()) {
    mailbox_printf("%s[%d][%s]: no lock before exec cbs from %s[%d], bad??\n",
            FILE__, __LINE__, getThreadStr(getExecThreadID()), file, line);
  }

  //assert(global_mutex->depth());
  mb_lock._Lock(FILE__, __LINE__);
  mailbox_printf("%s[%d][%s]:  executeCallbacks...  %d in pile\n", FILE__, __LINE__, getThreadStr(getExecThreadID()), cbs.size());
  cleanUpCalled();

  pdvector<CallbackBase *> deferred;

  while (cbs.size())
  {
    CallbackBase *cb = cbs[0], *called_cb;
    cbs.erase(0,0);

    mb_lock._Unlock(FILE__, __LINE__);
    called_cb = executeCallback(cb);
    mb_lock._Lock(FILE__, __LINE__);

    if (called_cb) {
       called.push_back(called_cb);
       mailbox_printf("%s[%d]:  callback executed\n", FILE__, __LINE__);
    }
    else {
      deferred.push_back(cb);
    }
  }
  for (unsigned int i = 0; i < deferred.size(); ++i) {
    cbs.push_back(deferred[i]);
  }
  deferred.clear(); 
  mb_lock._Unlock(FILE__, __LINE__);
}

CallbackBase *ThreadMailbox::executeCallback(CallbackBase *cb)
{
  if (cb->isExecuting()) {
    //  per-callback recursion guard
    mailbox_printf("%s[%d]:  callback is already executing!\n", FILE__, __LINE__);
    return NULL;
  }
  if ((cb->targetThread() != getExecThreadID())
      && (cb->targetThread()  != -1UL)) {
    //  not the right thread for this callback, cannot execute
    mailbox_printf("%s[%d]:  wrong thread for callback: target = %lu(%s), cur = %lu(%s)\n", 
           FILE__, __LINE__, cb->targetThread(), getThreadStr(cb->targetThread()),
            getExecThreadID(), getThreadStr(getExecThreadID()));
    return NULL;
  }
  else {
    mailbox_printf("%s[%d]:  got callback for thread %lu(%s), current: %lu\n",FILE__, __LINE__,
           cb->targetThread(), getThreadStr(cb->targetThread()), getExecThreadID());
  }

  cb->setExecuting(true);
  cb->execute(); 
  cb->setExecuting(false);

 mailbox_printf("%s[%d]:  after executing callback for thread %lu(%s)\n",FILE__, __LINE__,
        getExecThreadID(), getThreadStr(getExecThreadID()));

  CallbackCompletionCallback cleanup_cb = cb->getCleanupCallback();
 mailbox_printf("%s[%d]:  before cleanup for thread %lu(%s), cb is %p\n",FILE__, __LINE__,
        getExecThreadID(), getThreadStr(getExecThreadID()), cleanup_cb);
  if (cleanup_cb) {
    (cleanup_cb)(cb);
  }

 mailbox_printf("%s[%d]:  after executing cleanup for thread %lu(%s)\n",FILE__, __LINE__,
        getExecThreadID(), getThreadStr(getExecThreadID()));
  return cb;
}

void ThreadMailbox::cleanUpCalled()
{
  int startsz = called.size();
  for (int i = startsz -1; i >= 0; i--) {
    if (called[i]->deleteEnabled()) {
      CallbackBase *cb = called[i];
      called.erase(i,i);
      delete (cb);
    }
  }
}

