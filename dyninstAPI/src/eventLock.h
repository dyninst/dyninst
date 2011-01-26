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
#ifndef __EVENT_LOCK_H__
#define __EVENT_LOCK_H__
#include "os.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"

class BPatch_eventLock;

unsigned long getExecThreadID();
const char *getThreadStr(unsigned long tid);
void setCallbackThreadID(unsigned long tid);

class eventLock {
  friend class BPatch_eventLock;

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

extern eventLock *global_mutex;

#endif
