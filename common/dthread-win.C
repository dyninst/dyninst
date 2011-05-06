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
	assert(!"Not implemented");
	return -1;
}
bool DThread::spawn(DThread::initial_func_t func, void *param)
{
	assert(!"Not implemented");
	return false;
}
bool DThread::join()
{
	assert(!"Not implemented");
	return false;
}

long DThread::id()
{
	assert(!"Not implemented");
	return -1;
}


Mutex::Mutex(bool recursive)
{
}

Mutex::~Mutex()
{
}

bool Mutex::lock()
{
	assert(!"Not implemented");
	return false;
}

bool Mutex::unlock()
{
	assert(!"Not implemented");
	return false;
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
