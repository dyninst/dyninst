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
		::ReleaseSemaphore(wait_sema, 1, &prev_count);
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
