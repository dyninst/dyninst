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
#include "mailbox.h"
#include "BPatch_asyncEventHandler.h"
#include "BPatch_thread.h"
#include "BPatch_function.h"
#include "BPatch_point.h"
#include "common/h/Vector.h"

#if !defined(os_windows)
#include <pthread.h> // Trying native windows threads for now
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#endif

#if defined(os_windows)
#define MUTEX_TYPE CRITICAL_SECTION
extern MUTEX_TYPE global_mutex;
#else
#define MUTEX_TYPE eventLock *
extern MUTEX_TYPE global_mutex;
#endif

#if defined(os_linux) && defined (arch_x86)
#define PTHREAD_MUTEX_TYPE PTHREAD_MUTEX_RECURSIVE_NP
#define STRERROR_BUFSIZE 512
#define ERROR_BUFFER char buf[STRERROR_BUFSIZE]
#define STRERROR(x,y) strerror_r(x,y,STRERROR_BUFSIZE)
#else
#define ERROR_BUFFER
#define PTHREAD_MUTEX_TYPE PTHREAD_MUTEX_RECURSIVE
#define STRERROR_BUFSIZE 0
#define STRERROR(x,y) strerror(x)
#endif

eventLock *global_mutex = NULL;
bool mutex_created = false;

unsigned long primary_thread_id = (unsigned long) -1;
int lock_depth = 0;

BPatch_eventLock::BPatch_eventLock() 
{
  if (mutex_created) return;
    global_mutex = new eventLock();
#if defined (os_windows)
  primary_thread_id = _threadid;
#else
  primary_thread_id = (unsigned long)pthread_self();
#endif
  mutex_created = true;
}

BPatch_eventLock::~BPatch_eventLock() {};

int BPatch_eventLock::_Lock(const char *__file__, unsigned int __line__) const
{
  return global_mutex->_Lock(__file__, __line__);
}
int BPatch_eventLock::_Trylock(const char *__file__, unsigned int __line__) const
{
  return global_mutex->_Trylock(__file__, __line__);
}

int BPatch_eventLock::_Unlock(const char *__file__, unsigned int __line__) const
{
  return global_mutex->_Unlock(__file__, __line__);
}


int BPatch_eventLock::_Broadcast(const char *__file__, unsigned int __line__) const
{
  return global_mutex->_Broadcast(__file__, __line__);
}

int BPatch_eventLock::_WaitForSignal(const char *__file__, unsigned int __line__) const
{
  return global_mutex->_WaitForSignal(__file__, __line__);
}


#if defined (os_windows)
unsigned long BPatch_eventLock::threadID() const
{
  return (unsigned long) _threadid;
}
#else
unsigned long BPatch_eventLock::threadID() const
{
  return (unsigned long) pthread_self();
}
#endif


