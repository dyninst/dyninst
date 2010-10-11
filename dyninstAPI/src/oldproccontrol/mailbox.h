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
#ifndef __EVENT_MAILBOX_H__
#define __EVENT_MAILBOX_H__
#include "os.h"
#include "EventHandler.h"
//#include "BPatch_process.h"
//#include "BPatch.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"

class BPatch_eventLock;
class DebuggerInterface;
class DBIEvent;

class eventLock {
  friend class BPatch_eventLock;
  friend class DebuggerInterface;

  public:

  eventLock();
  virtual ~eventLock();

  public:
  unsigned int depth() {return lock_depth;}
  int _Lock(const char *__file__, unsigned int __line__);
  int _Trylock(const char *__file__, unsigned int __line__);
  int _Unlock(const char *__file__, unsigned int __line__);
  int _Broadcast(const char *__file__, unsigned int __line__);
  int _WaitForSignal(const char *__file__, unsigned int __line__);

  void printLockStack();

  private:

  EventLock_t mutex;
  EventCond_t cond;

  unsigned int lock_depth;

  typedef struct {
    const char *file;
    unsigned int line;
  } lock_stack_elem;

  inline lock_stack_elem popLockStack() {
    lock_stack_elem el = lock_stack[lock_stack.size() -1];
    lock_stack.pop_back();
    lock_depth--;
    return el;
  }

  inline void pushLockStack(lock_stack_elem elm) {
    lock_stack.push_back(elm);
    lock_depth++;
  }

  pdvector<lock_stack_elem> lock_stack;
  unsigned long owner_id;

#if defined (os_windows)
  EventLock_t waiter_lock;
  int num_waiters;

  int generation_num;
  int release_num;
#endif
};


class BPatch_asyncEventHandler;
class SignalHandler;

class CallbackBase;
typedef void (*CallbackCompletionCallback)(CallbackBase *);

class CallbackBase
{
  public:
    CallbackBase(unsigned long target = (unsigned long) -1L, 
                 CallbackCompletionCallback cb = NULL) 
                 : target_thread (target),
                   exec_flag(false),
                   ok_to_delete(true),
                   cleanup_callback(cb) {}
    virtual ~CallbackBase() {}
    virtual bool execute()=0;
    virtual CallbackBase *copy()=0;
    unsigned long targetThread() {return target_thread;}
    unsigned long execThread() {return execution_thread;}
    void setTargetThread(unsigned long t) {target_thread = t;}
    bool isExecuting() {return exec_flag;}

    void setExecuting(bool flag = true, 
                      unsigned long exec_thread_id = (unsigned long) -1L) {
      execution_thread = exec_thread_id;
      exec_flag = flag;
    }
    
    void enableDelete(bool flag = true) {ok_to_delete = flag;}
    bool deleteEnabled() {return ok_to_delete;}

    CallbackCompletionCallback getCleanupCallback() {return cleanup_callback;}
  private:
    unsigned long target_thread;
    unsigned long execution_thread;
    bool exec_flag;
    bool ok_to_delete;
    CallbackCompletionCallback cleanup_callback;
};

class ThreadMailbox
{
  public:

    ThreadMailbox() {}
    ~ThreadMailbox(); 

     void executeOrRegisterCallback(CallbackBase *cb); 
     void executeCallbacks(const char *file, unsigned int line); 
     CallbackBase *runningInsideCallback();

  private:

    CallbackBase *executeCallback(CallbackBase *cb);
    void cleanUpCalled();
    pdvector<CallbackBase *> cbs;
    pdvector<CallbackBase *> running;
    pdvector<CallbackBase *> called;
    eventLock mb_lock;
};

ThreadMailbox *getMailbox();

#define TARGET_ANY_THREAD (unsigned long) -1L
#define TARGET_UI_THREAD  primary_thread_id
#define TARGET_DBI_THREAD  dbi_thread_id
#define TARGET_SYNC_THREAD  sync_thread_id


#endif
