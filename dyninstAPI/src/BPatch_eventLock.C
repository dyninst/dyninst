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

#include "BPatch_eventLock.h"
#include "BPatch_thread.h"
#include "BPatch_function.h"
#include "common/h/Vector.h"
#include "eventLock.h"

#if !defined(os_windows)
#include <pthread.h> // Trying native windows threads for now
#else
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#endif

#if defined(os_windows)
#define MUTEX_TYPE eventLock *
extern MUTEX_TYPE global_mutex;
#else
#define MUTEX_TYPE eventLock *
extern MUTEX_TYPE global_mutex;
#endif

#if defined(os_linux) //&& defined (arch_x86)
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

BPatch_eventLock::BPatch_eventLock() 
{
//  if (mutex_created) return;
//  global_mutex = new eventLock();
//  mutex_created = true;

  //  Assume that this ctor is being called on the primary (UI) thread
  //  and set its value accordingly.
  //primary_thread_id = getExecThreadID();
  primary_thread_id = 0;
}

BPatch_eventLock::~BPatch_eventLock() {};

eventLock *BPatch_eventLock::getLock() 
{
//	assert(global_mutex); 
//	return global_mutex;
	return NULL;
}

int BPatch_eventLock::_Lock(const char * /*__file__*/, unsigned int /*__line__*/) const
{
	return 0;
  //return global_mutex->_Lock(__file__, __line__);
}
int BPatch_eventLock::_Trylock(const char * /*__file__*/, unsigned int /*__line__*/) const
{
	return 0;
//  return global_mutex->_Trylock(__file__, __line__);
}

int BPatch_eventLock::_Unlock(const char * /*__file__*/, unsigned int /*__line__*/) const
{
	return 0;
//  return global_mutex->_Unlock(__file__, __line__);
}


int BPatch_eventLock::_Broadcast(const char * /*__file__*/, unsigned int /*__line__*/) const
{
	return 0;
//  return global_mutex->_Broadcast(__file__, __line__);
}

int BPatch_eventLock::_WaitForSignal(const char * /*__file__*/, unsigned int /*__line__*/) const
{
	return 0;
//  return global_mutex->_WaitForSignal(__file__, __line__);
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

unsigned int BPatch_eventLock::lockDepth() const
{
	return 0;
//  return global_mutex->depth();
}
