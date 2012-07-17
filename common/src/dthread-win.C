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
	return (long)::GetCurrentThread();
}
bool DThread::spawn(DThread::initial_func_t func, void *param)
{
	thrd = ::CreateThread(NULL, 0, func, param, 0, NULL);
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
	return (long)thrd;
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
	return (::WaitForSingleObject(mutex, INFINITE) == WAIT_OBJECT_0);
}

bool Mutex::unlock()
{
	return ::ReleaseMutex(mutex);
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
}

bool CondVar::unlock()
{
	mutex->unlock();
	return true;
}
bool CondVar::lock()
{
	mutex->lock();
	return true;
}
bool CondVar::signal()
{
	mutex->lock();
	::EnterCriticalSection(&numWaitingLock);
	bool waitingThreads = (numWaiting > 0);
	::LeaveCriticalSection(&numWaitingLock);
	if(waitingThreads)
	{
		::ReleaseSemaphore(wait_sema, 1, 0);
	}
	mutex->unlock();
	return true;
}
bool CondVar::broadcast()
{
	mutex->lock();
	::EnterCriticalSection(&numWaitingLock);
	bool waitingThreads = (numWaiting > 0);
	was_broadcast = true;
	if(waitingThreads)
	{
		::ReleaseSemaphore(wait_sema, 1, 0);
		::LeaveCriticalSection(&numWaitingLock);
		::WaitForSingleObject(wait_done, INFINITE);
		was_broadcast = false;
	}
	else
	{
		::LeaveCriticalSection(&numWaitingLock);
	}
	mutex->unlock();
	return true;
}
bool CondVar::wait()
{
	mutex->lock();
	::EnterCriticalSection(&numWaitingLock);
	numWaiting++;
	::LeaveCriticalSection(&numWaitingLock);
	::SignalObjectAndWait(mutex->mutex, wait_sema, INFINITE, FALSE);
	::EnterCriticalSection(&numWaitingLock);
	numWaiting--;
	bool last_waiter = (was_broadcast && (numWaiting == 0));
	::LeaveCriticalSection(&numWaitingLock);
	if(last_waiter)
	{
		::SignalObjectAndWait(wait_done, mutex->mutex, INFINITE, FALSE);
	}
	else
	{
		::WaitForSingleObject(mutex->mutex, INFINITE);
	}
	mutex->unlock();
	return true;
}
