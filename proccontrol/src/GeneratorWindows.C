#include "proccontrol/h/Generator.h"
#include "GeneratorWindows.h"
#include "DecoderWindows.h"
#include "windows.h"
#include "procpool.h"

#include <iostream>

using namespace Dyninst;
using namespace std;

static GeneratorWindows *gen = NULL;

Generator *Generator::getDefaultGenerator()
{
   if (!gen) {
      gen = new GeneratorWindows();
      assert(gen);
   }
   return gen;
}

void Generator::stopDefaultGenerator()
{
    if(gen) delete gen;
}

bool GeneratorWindows::initialize()
{
   return true;
}

bool GeneratorWindows::canFastHandle()
{
   return false;
}

void GeneratorWindows::plat_start()
{
	if(procsToStart.empty())
		return;
	StartInfo todo = procsToStart.front();
	procsToStart.pop_front();
	windows_process* proc = dynamic_cast<windows_process*>(todo.proc);
	switch(todo.mode)
	{
	case create:
		proc->plat_create_int();
		break;
	case attach:
		proc->plat_attach_int();
		break;
	default:
		assert(!"unknown mode in GeneratorWindows::plat_start, expected create or attach");
		break;
	}
	waiters.insert(std::make_pair(proc->getPid(), Waiters::ptr(new Waiters)));
	processes.insert(std::make_pair(proc->getPid(), proc));
	thread_to_proc.insert(std::make_pair(::GetCurrentThreadId(), proc));
}

void GeneratorWindows::removeProcess(int_process *proc)
{
	processes.erase(proc->getPid());
	//if(thread_to_proc[::GetCurrentThreadId()] == proc)
	//{
		thread_to_proc.erase(::GetCurrentThreadId());
	//}
	//else
	//{
	//	assert(!"Generator deleting process on wrong thread!");
	//}
	waiters.erase(proc->getPid());
}

bool GeneratorWindows::hasLiveProc()
{
	int_process* p = thread_to_proc[::GetCurrentThreadId()];
	if(!p) {
		setState(exiting);
		return false;
	}
	return !allStopped(p, NULL);
}

void GeneratorWindows::plat_continue(ArchEvent* evt)
{
	// evt is null the first time we call this; however, 
	// the process also hasn't called WaitForDebugEvent the first time we call it.
	// thus, do nothing if evt == NULL.
	if(!evt) return;
	ArchEventWindows* winEvt = static_cast<ArchEventWindows*>(evt);
	pthrd_printf("GeneratorWindows::plat_continue() for %d waiting\n", winEvt->evt.dwProcessId);
	//::ResetEvent(waiters[winEvt->evt.dwProcessId]->user_wait);
	//::WaitForSingleObject(waiters[winEvt->evt.dwProcessId]->gen_wait, INFINITE);
	Waiters::ptr theWaiter = waiters[winEvt->evt.dwProcessId];
	if(!theWaiter) {
		pthrd_printf("Wait object deleted. Process %d should be dead. Returning.\n", winEvt->evt.dwProcessId);
		return;
	}
	DWORD cont_val = theWaiter->unhandled_exception ? DBG_EXCEPTION_NOT_HANDLED : DBG_CONTINUE;
	/*int_thread* t = ProcPool()->findThread((Dyninst::LWP)(winEvt->evt.dwThreadId));
	windows_thread* w = static_cast<windows_thread*>(t);
	if(w) {
		std::cerr << "ContinueDebugEvent on process " << winEvt->evt.dwProcessId << std::endl;
		std::cerr << w->dumpThreadContext() << std::endl;
	}*/
	::ContinueDebugEvent(winEvt->evt.dwProcessId, winEvt->evt.dwThreadId, cont_val);
	//cerr << "Thread " << winEvt->evt.dwThreadId << " is " << w->getSuspendedStatus() << " at ContinueDebugEvent() call"
	//	<< endl;
	waiters[winEvt->evt.dwProcessId]->unhandled_exception = false;
	pthrd_printf("GeneratorWindows::plat_continue() for %d done with ::ContinueDebugEvent()\n", winEvt->evt.dwProcessId);
	::SetEvent(theWaiter->user_wait);
	pthrd_printf("GeneratorWindows::plat_continue() for %d done with signal()\n", winEvt->evt.dwProcessId);
}

ArchEvent *GeneratorWindows::getEvent(bool block)
{
	DEBUG_EVENT evt;
	pthrd_printf("About to WaitForDebugEvent()\n");
	if(::WaitForDebugEvent(&evt, INFINITE))
	{
		ArchEventWindows* new_evt = new ArchEventWindows(evt);
		return new_evt;
	}
	else
	{
		long errCode = ::GetLastError();
		static const int size = 1024;
		char buffer[size];
		::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, size-1, NULL);
		fprintf(stderr, "Error in WaitForDebugEvent: %s\n", buffer);
		return NULL;
	}
}

void GeneratorWindows::wait(Dyninst::PID p)
{
	pthrd_printf("GeneratorWindows::wait() for %d\n", p);
	Waiters::ptr w = waiters[p];
	if(w) {
		::WaitForSingleObject(w->user_wait, INFINITE);
	}
	pthrd_printf("GeneratorWindows::wait() done for %d\n", p);
}

void GeneratorWindows::wake(Dyninst::PID p)
{
	pthrd_printf("GeneratorWindows::wake() for %d\n", p);
	Waiters::ptr w = waiters[p];
	if(w) {
		::SetEvent(w->gen_wait);
	}
}

void GeneratorWindows::markUnhandledException(Dyninst::PID p)
{
	waiters[p]->unhandled_exception = true;
}


GeneratorWindows::GeneratorWindows() :
   GeneratorMT(std::string("Windows Generator"))
{
   decoders.insert(new DecoderWindows());
   procsToStart.clear();
}

GeneratorWindows::~GeneratorWindows()
{
}

void GeneratorWindows::enqueue_event(start_mode m, int_process* p)
{
	StartInfo todo;
	todo.mode = m;
	todo.proc = p;
	lock();
	procsToStart.push_back(todo);
	unlock();
}