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



#include "BPatch_eventLock.h"
#include "BPatch_asyncEventHandler.h"
#include "BPatch_thread.h"
#include "BPatch_function.h"
#include "BPatch_point.h"
#include "common/h/Vector.h"

//#define DEBUG_MUTEX 1


MUTEX_TYPE global_mutex;
bool mutex_created = false;

BPatch_eventMailbox *event_mailbox;
unsigned long primary_thread_id = (unsigned long) -1;

int lock_depth = 0;

BPatch_eventLock::BPatch_eventLock() 
{
  if (mutex_created) return;
#if defined(os_windows)
  InitializeCriticalSection(&global_mutex);
  primary_thread_id = _threadid;
#else
  pthread_mutexattr_t mutex_type;
  pthread_mutexattr_init(&mutex_type);
  primary_thread_id = (unsigned long) pthread_self();
#if defined(arch_ia64)
  try {
    pthread_mutexattr_settype(&mutex_type, PTHREAD_MUTEX_RECURSIVE_NP);
  }catch (...) {
    fprintf(stderr, "%s[%d]: exception %d\n", __FILE__, __LINE__);
  }
  try {
  pthread_mutex_init(&global_mutex, &mutex_type);
  }catch (...) {
    fprintf(stderr, "%s[%d]: exception%d\n", __FILE__, __LINE__);
  }
#else
  pthread_mutexattr_settype(&mutex_type, PTHREAD_MUTEX_TYPE);
  pthread_mutex_init(&global_mutex, &mutex_type);
#endif
#endif // !Windows
  event_mailbox = new BPatch_eventMailbox();
  
  mutex_created = true;
}

#ifdef NOTDEF
bool BPatch_eventLock::init() 
{
  if (mutex_created) return;
#if defined(os_windows)
  InitializeCriticalSection(&global_mutex);
#else
  pthread_mutexattr_t mutex_type;
  pthread_mutexattr_init(&mutex_type);
#if defined(arch_ia64)
  if (0 != pthread_mutexattr_setkind_np(&mutex_type, PTHREAD_MUTEX_TYPE))
    return false;
#else
  if (0 != pthread_mutexattr_settype(&mutex_type, PTHREAD_MUTEX_TYPE))
    return false;
#endif
  if (0 != pthread_mutex_init(&global_mutex, &mutex_type))
    return false;
#endif // !Windows
  mutex_created = true;
  return true;
}
#endif
BPatch_eventLock::~BPatch_eventLock() {};

#if defined(os_windows)

int BPatch_eventLock::_Lock(const char *__file__, unsigned int __line__) const
{
  EnterCriticalSection(&global_mutex);
  lock_depth++;
  if ((threadID() == primary_thread_id) && (lock_depth == 1))
    event_mailbox->executeUserCallbacks();
  return 0;
}

unsigned long BPatch_eventLock::threadID() const
{
  return (unsigned long) _threadid;
}

int BPatch_eventLock::_Trylock(const char *__file__, unsigned int __line__) const
{
  TryEnterCriticalSection(&global_mutex);
  return 0;
}

int BPatch_eventLock::_Unlock(const char *__file__, unsigned int __line__) const
{
  lock_depth--;
  LeaveCriticalSection(&global_mutex);
  return 0;
}

#else
unsigned long BPatch_eventLock::threadID() const
{
  return (unsigned long) pthread_self();
}

int BPatch_eventLock::_Lock(const char *__file__, unsigned int __line__) const
{
  int err = 0;
  if(0 != (err = pthread_mutex_lock(&global_mutex))){
    ERROR_BUFFER;
    fprintf(stderr, "%s[%d]:  failed to lock mutex: %s[%d]\n",
            __file__, __line__, STRERROR(err, buf), err);
  }
  lock_depth++;
#ifdef DEBUG_MUTEX
  fprintf(stderr, "%s[%d]:  lock, depth = %d\n", __file__, __line__, lock_depth);
#endif

  unsigned long tid = threadID();
  if ((tid == primary_thread_id) && (lock_depth == 1))
    event_mailbox->executeUserCallbacks();

  return err;
}

int BPatch_eventLock::_Trylock(const char *__file__, unsigned int __line__) const
{
  int err = 0;
  if(0 != (err = pthread_mutex_trylock(&global_mutex))){
    if (EBUSY != err) {
      ERROR_BUFFER;
      //  trylock returns EBUSY immediately when lock cannot be obtained
      fprintf(stderr, "%s[%d]:  failed to trylock mutex: %s[%d]\n",
              __file__, __line__, STRERROR(err, buf), err);
    }
  }
  return err;
}

int BPatch_eventLock::_Unlock(const char *__file__, unsigned int __line__) const
{
  int err = 0;
  lock_depth--;
#ifdef DEBUG_MUTEX
  fprintf(stderr, "%s[%d]:  unlock, lock depth will be %d \n", __file__, __line__, lock_depth);
#endif
  if(0 != (err = pthread_mutex_unlock(&global_mutex))){
    ERROR_BUFFER;
    fprintf(stderr, "%s[%d]:  failed to unlock mutex: %s[%d]\n",
            __file__, __line__, STRERROR(err, buf), err);
  }
  return err;
}

#endif


