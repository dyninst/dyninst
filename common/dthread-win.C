/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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
#include "common/h/dthread.h"
#include <assert.h>

DThread::DThread()
{
}

DThread::~DThread()
{
}

long DThread::self()
{
	return ::GetCurrentThreadId();
}
bool DThread::spawn(DThread::initial_func_t func, void *param)
{
	thrd = ::CreateThread(NULL, 0, func, param, 0, &tid);
	live = (thrd != INVALID_HANDLE_VALUE);
	return live;
}
bool DThread::join()
{
	assert(live && self() != id());
	::WaitForSingleObject(thrd, INFINITE);
	return true;
}

long DThread::id()
{
	return tid;
}


Mutex::Mutex(bool recursive)
{
	mutex = ::CreateMutex(NULL, false, NULL);
}

Mutex::~Mutex()
{
	::CloseHandle(mutex);
}

bool Mutex::lock()
{
//	fprintf(stderr, "[%d]: Mutex::lock() waiting for 0x%lx\n", ::GetCurrentThreadId(), mutex);
	bool ok = (::WaitForSingleObject(mutex, INFINITE) == WAIT_OBJECT_0);
//	fprintf(stderr, "[%d]: Mutex::lock() holding 0x%lx\n", ::GetCurrentThreadId(), mutex);
	return ok;
}

bool Mutex::unlock()
{
//	fprintf(stderr, "[%d]: Mutex::unlock() for 0x%lx\n", ::GetCurrentThreadId(), mutex);
	bool ok = (::ReleaseMutex(mutex) != 0);
	if(!ok) {
		fprintf(stderr, "Failed to release mutex: %d\n", ::GetLastError());
	}
	return ok;
}

CondVar::CondVar(Mutex *m) :
	numWaiting(0),
	was_broadcast(false),
	mutex(m),
	created_mutex(false)
{
	if(mutex == NULL)
	{
		mutex = new Mutex();
		created_mutex = true;
	}
	wait_sema = ::CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
	::InitializeCriticalSection(&numWaitingLock);
	wait_done = ::CreateEvent(NULL, false, false, NULL);
}
CondVar::~CondVar()
{
	if(created_mutex)
	{
		delete mutex;
	}
	::CloseHandle(wait_sema);
	::CloseHandle(wait_done);
	::DeleteCriticalSection(&numWaitingLock);
}

bool CondVar::unlock()
{
	bool ret = mutex->unlock();
	return ret;
}
bool CondVar::lock()
{
	bool ret = mutex->lock();
	return ret;	
}
bool CondVar::signal()
{
	::EnterCriticalSection(&numWaitingLock);
	bool waitingThreads = (numWaiting > 0);
	::LeaveCriticalSection(&numWaitingLock);
	if(waitingThreads)
	{
		long prev_count;
		::ReleaseSemaphore(wait_sema, 1, &prev_count);
//		fprintf(stderr, "[%d]: CondVar::signal() signaled 0x%lx, prev_count = %d\n", ::GetCurrentThreadId(), wait_sema, prev_count);
	}
	return true;
}
bool CondVar::broadcast()
{
	::EnterCriticalSection(&numWaitingLock);
	bool waitingThreads = (numWaiting > 0);
	was_broadcast = true;
	if(waitingThreads)
	{
		long prev_count;
		::ReleaseSemaphore(wait_sema, numWaiting, &prev_count);
//		fprintf(stderr, "[%d]: CondVar::broadcast() signaled 0x%lx, prev_count = %d\n", ::GetCurrentThreadId(), wait_sema, prev_count);
		::LeaveCriticalSection(&numWaitingLock);
		::WaitForSingleObject(wait_done, INFINITE);
		was_broadcast = false;
	}
	else
	{
		::LeaveCriticalSection(&numWaitingLock);
	}
	return true;
}
bool CondVar::wait()
{
	::EnterCriticalSection(&numWaitingLock);
	numWaiting++;
	::LeaveCriticalSection(&numWaitingLock);
//	fprintf(stderr, "[%d]: CondVar::wait() signalling for 0x%lx, waiting for 0x%lx\n", ::GetCurrentThreadId(), mutex->mutex, wait_sema);
	unsigned long result = ::SignalObjectAndWait(mutex->mutex, wait_sema, INFINITE, FALSE);
	assert(result != WAIT_TIMEOUT);
	::EnterCriticalSection(&numWaitingLock);
	numWaiting--;
	bool last_waiter = (was_broadcast && (numWaiting == 0));
	::LeaveCriticalSection(&numWaitingLock);
	if(last_waiter)
	{
//		fprintf(stderr, "[%d]: CondVar::wait() waiting for 0x%lx, signaling 0x%lx\n", ::GetCurrentThreadId(), mutex->mutex, wait_done);
		result = ::SignalObjectAndWait(wait_done, mutex->mutex, INFINITE, FALSE);
		assert(result != WAIT_TIMEOUT);
	}
	else
	{
		::WaitForSingleObject(mutex->mutex, INFINITE);
	}
	return true;
}
