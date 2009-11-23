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

#include "dyninstAPI/src/mailbox.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/debug.h"
#include "common/h/headers.h"
#include "dyninstAPI/src/os.h"
#if defined (os_windows)
#include <windows.h>
#endif
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
#if 0
//HANDLE CreateMutex(
//  LPSECURITY_ATTRIBUTES lpMutexAttributes,
//  BOOL bInitialOwner,
//  LPCTSTR lpName )
  mutex = CreateMutex(NULL, false /*no initial owner*/, NULL);
  assert(mutex);
#endif
  InitializeCriticalSection(&mutex);
  InitializeCriticalSection(&waiter_lock);
  num_waiters = 0;
//HANDLE CreateEvent(
//  LPSECURITY_ATTRIBUTES lpEventAttributes,
//  BOOL bManualReset,
//  BOOL bInitialState,
//  LPCTSTR lpName )

  cond = CreateEvent(NULL, true /*true is manual reset, false for auto*/,
                    false /*initially not in signalled state */,
                    NULL /*name*/);
  assert (cond);
#else
  int err = 0;
  pthread_mutexattr_t mutex_type;
  if (0 != pthread_mutexattr_init(&mutex_type)) {
     assert(0);
  }
  if (0 != pthread_mutexattr_settype(&mutex_type, PTHREAD_MUTEX_TYPE)) {
     assert(0);
  }
  if (0 != pthread_mutex_init(&mutex, &mutex_type)) {
   ERROR_BUFFER;
    fprintf(stderr, "%s[%d]:  failed to init mutex: %s[%d]\n",
            FILE__, __LINE__, STRERROR(err, buf), err);
     assert(0);
  }
  pthread_cond_init(&cond, NULL);
#endif // !Windows
  lock_depth = 0;
}

eventLock::~eventLock()
{
#if defined(os_windows)
  DeleteCriticalSection(&mutex);  
  DeleteCriticalSection(&waiter_lock);
  CloseHandle(cond);
#else
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);
#endif
}

int eventLock::_Lock(const char *__file__, unsigned int __line__)
{
  int err = 0;
  //fprintf(stderr, "%s[%d]: about to lock %p: from %s[%d]\n", FILE__, __LINE__, &mutex, __file__, __line__);

#if defined(os_windows)
  EnterCriticalSection(&mutex);
#if 0
  DWORD res = WaitForSingleObject(mutex, INFINITE);
  assert(res == WAIT_OBJECT_0);
  //  Other possible results for res are WAIT_TIMEOUT and WAIT_ABANDONED
#endif

#else
  if(0 != (err = pthread_mutex_lock(&mutex))){
    ERROR_BUFFER;
    fprintf(stderr, "%s[%d]:  failed to lock mutex: %s[%d]\n",
            __file__, __line__, STRERROR(err, buf), err);
    return err;
  }
#endif

    if (lock_depth) {
      if ((owner_id != getExecThreadID()) && (owner_id != (unsigned long) -1 *1L)) {
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
    

  mutex_printf("%s[%d]:  lock obtained from %s[%d], depth = %d\n", FILE__, __LINE__, __file__, __line__, lock_depth);

  return err;
}


int eventLock::_Trylock(const char *__file__, unsigned int __line__)
{
  int err = 0;
#if defined(os_windows)
  assert(0); 
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
  unsigned long old_owner_id = owner_id;
  if (!lock_depth) {
    fprintf(stderr, "%s[%d]:  MUTEX ERROR, attempt to unlock nonlocked mutex, at %s[%d]\n",
            FILE__, __LINE__, __file__, __line__);
    assert(0);
  }
  lock_depth--;
  lock_stack_elem el = lock_stack[lock_stack.size() -1];
  lock_stack.pop_back();
  if (!lock_depth)
    owner_id = (unsigned long) -1 * 1L;

  mutex_printf("%s[%d]:  unlock issued from %s[%d], depth = %d\n", FILE__, __LINE__, __file__, __line__, lock_depth);

#if defined(os_windows)
  LeaveCriticalSection(&mutex);
  return 0;
#else
  int err = 0;
  if(0 != (err = pthread_mutex_unlock(&mutex))){
    ERROR_BUFFER;
    fprintf(stderr, "%s[%d]:  failed to unlock mutex: %s[%d]\n",
            __file__, __line__, STRERROR(err, buf), err);
    lock_depth++;
    lock_stack.push_back(el);
    owner_id = old_owner_id;
  }
  return err;
#endif
}

int eventLock::_Broadcast(const char *__file__, unsigned int __line__)
{
#if defined(os_windows)
  EnterCriticalSection(&waiter_lock);

  if (num_waiters > 0) {
    int ret = SetEvent(cond);
    assert (ret);
    release_num = num_waiters;
    generation_num++;
  }

  LeaveCriticalSection(&waiter_lock);
  //ret = SetEvent(mutex);
  //assert (ret);
#else
  if (!this) {
    fprintf(stderr, "%s[%d]:  lock is broken:\n", FILE__, __LINE__);
    //  most likely the lock stack will crash here...  but this is pretty fatal anyways.
    printLockStack();
    return 1;
  }
  int err = 0;
  if(0 != (err = pthread_cond_broadcast(&cond))){
    ERROR_BUFFER;
    fprintf(stderr, "From: %s[%d]:  failed to broadcast cond: %s[%d]\n",
            __file__, __line__, STRERROR(err, buf), err);
    return 1;
  }

#endif
  return 0;
}

/**
 * The windows locking algorithm is described at 
 * http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
 **/
int eventLock::_WaitForSignal(const char *__file__, unsigned int __line__)
{
  int err = 0;
  pdvector<lock_stack_elem> lstack;
  unsigned cached_lock_depth = lock_depth;
  if (!lock_depth) {
    fprintf(stderr, "%s[%d][%s]: cannot call wait until lock is obtained, see %s[%d]\n", FILE__, __LINE__, getThreadStr(getExecThreadID()), __file__, __line__);
     assert(0);
  }
  
  lstack = lock_stack;
  for (unsigned i=0; i<cached_lock_depth-1; i++) 
     __UNLOCK;
  lock_depth = 0;
  lock_stack.clear();
  owner_id = 0;

  assert(lock_stack.size() == 0);
#if defined(os_windows)
  EnterCriticalSection(&waiter_lock);
  num_waiters++;
  int my_generation = generation_num;
  LeaveCriticalSection(&waiter_lock);

  LeaveCriticalSection(&mutex);
 
  while (1) {
    DWORD res = WaitForSingleObject(cond, INFINITE);

    if (res != WAIT_OBJECT_0) {
      switch(res) {
        case WAIT_ABANDONED:  fprintf(stderr, "%s[%d]:  WAIT_ABANDONED\n", FILE__, __LINE__); break;
        case WAIT_TIMEOUT:  fprintf(stderr, "%s[%d]:  WAIT_TIMEOUT\n", FILE__, __LINE__); break;
        case WAIT_FAILED:  fprintf(stderr, "%s[%d]:  WAIT_FAILED\n", FILE__, __LINE__); 
        default:
          DWORD err = GetLastError();
          extern void printSysError(unsigned errNo);
          printSysError(err);
       };
     }
     EnterCriticalSection(&waiter_lock);
     bool wait_done = (release_num > 0) && (generation_num != my_generation);
     LeaveCriticalSection(&waiter_lock);
     if (wait_done) break;
   }  

   EnterCriticalSection(&mutex);
   EnterCriticalSection(&waiter_lock);
   num_waiters--;
   release_num--;
   if (num_waiters < 0) {
      fprintf(stderr, "%s[%d]: FIXME!\n", FILE__, __LINE__);
      num_waiters = 0;
   }
   if (release_num < 0) {
      fprintf(stderr, "%s[%d]: FIXME!\n", FILE__, __LINE__);
      num_waiters = 0;
   }

   bool do_reset  = (release_num == 0);
   LeaveCriticalSection(&waiter_lock);

   if (do_reset) {
      ResetEvent(cond);
   }

#else
  mutex_printf("%s[%d]:  unlock/wait issued from %s[%d], depth = %d\n", FILE__, __LINE__, __file__, __line__, lock_depth);
  if(0 != (err = pthread_cond_wait(&cond, &mutex))){
    ERROR_BUFFER;
    fprintf(stderr, "%s[%d]:  failed to broadcast cond: %s[%d]\n",
            __file__, __line__, STRERROR(err, buf), err);
    return 1;
  }
#endif

  for (unsigned i=0; i<cached_lock_depth-1; i++) 
     __LOCK;
  lock_stack = lstack;
  lock_depth = cached_lock_depth;
  owner_id = getExecThreadID();
  mutex_printf("%s[%d]:  wait/re-loc issued from %s[%d], depth = %d\n", FILE__, __LINE__, __file__, __line__, lock_depth);
  return 0;
}

void eventLock::printLockStack()
{
    fprintf(stderr, "%s[%d]:  Lock stack:\n", FILE__, __LINE__);
    for (unsigned int i = 0; i < lock_stack.size(); ++i) {
      fprintf(stderr, "\t[%s][%d]\n", lock_stack[i].file, lock_stack[i].line);
    }
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
    VECTOR_ERASE(cbs,0,0);

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
      && (cb->targetThread()  != (unsigned long) -1L)) {
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

  cb->setExecuting(true, getExecThreadID());
  running.push_back(cb);
  cb->execute(); 

  //  remove callback from the running pile
  bool erased_from_running_pile = false;
  for (unsigned int i = 0; i < running.size(); ++i) {
    if (running[i] == cb) {
       VECTOR_ERASE(running,i,i);
       erased_from_running_pile = true;
       break;
    }
  }
  assert(erased_from_running_pile);
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

CallbackBase *ThreadMailbox::runningInsideCallback()
{
  //  if there is a callback executing on the current thread, then caller 
  //  must be, by extension, running as a result being inside that callback.
  for (unsigned int i = 0; i < running.size(); ++i) {
    assert(running[i]->isExecuting());
    if (running[i]->execThread() == getExecThreadID())
      return running[i];
  }
  return NULL;
}

void ThreadMailbox::cleanUpCalled()
{
  int startsz = called.size();
  for (int i = startsz -1; i >= 0; i--) {
    if (called[i]->deleteEnabled()) {
      CallbackBase *cb = called[i];
      VECTOR_ERASE(called,i,i);
      delete (cb);
    }
  }
}

