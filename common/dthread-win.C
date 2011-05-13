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

CondVar::CondVar(Mutex *m)
{
}
CondVar::~CondVar()
{
}

bool CondVar::unlock()
{
	assert(!"Not implemented");
	return false;
}
bool CondVar::lock()
{
	assert(!"Not implemented");
	return false;
}
bool CondVar::signal()
{
	assert(!"Not implemented");
	return false;
}
bool CondVar::broadcast()
{
	assert(!"Not implemented");
	return false;
}
bool CondVar::wait()
{
	assert(!"Not implemented");
	return false;
}
