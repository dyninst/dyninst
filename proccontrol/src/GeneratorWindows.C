#include "proccontrol/h/Generator.h"
#include "GeneratorWindows.h"
#include "DecoderWindows.h"
#include "windows_process.h"
#include "windows_handler.h"
#include "procpool.h"
#include "windows_thread.h"

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

struct value_equal
{
	value_equal(int_process* v) : value(v) {}
	bool operator()(std::pair<int, int_process*> element)
	{
		return element.second == value;
	}
	int_process* value;
};

void GeneratorWindows::removeProcess(int_process *proc)
{
	processes.erase(proc->getPid());
	//if(thread_to_proc[::GetCurrentThreadId()] == proc)
	//{
	std::map<int, int_process*>::iterator found = std::find_if(thread_to_proc.begin(), thread_to_proc.end(),
		value_equal(proc));
	if(found != thread_to_proc.end()) {
		found->second = NULL;
	}
	//}
	//else
	//{
	//	assert(!"Generator deleting process on wrong thread!");
	//}
	waiters.erase(proc->getPid());
}

long long GeneratorWindows::getSequenceNum(Dyninst::PID p)
{
	return alreadyHandled[p];
}

bool GeneratorWindows::hasLiveProc()
{
	pthrd_printf("begin hasLiveProc()\n");
	ProcPool()->condvar()->lock();
	int_process* p = thread_to_proc[::GetCurrentThreadId()];
	if(!p) {
		setState(exiting);
		pthrd_printf("hasLiveProc() found NULL process, returning FALSE\n");
		ProcPool()->condvar()->signal();
		ProcPool()->condvar()->unlock();
		return false;
	}
	bool all_stopped = allStopped(p, NULL);
	bool ret = !all_stopped;
	pthrd_printf("allStopped %s for %d, returning %s\n",
		all_stopped ? "TRUE" : "FALSE",
		p->getPid(),
		ret ? "TRUE" : "FALSE");


	ProcPool()->condvar()->signal();
	ProcPool()->condvar()->unlock();
	return ret;
}

ArchEvent* GeneratorWindows::getCachedEvent()
{
	return m_Events[DThread::self()];
}
void GeneratorWindows::setCachedEvent(ArchEvent* ae)
{
	m_Events[DThread::self()] = ae;
}

// Returns true if we should continue to WaitForDebugEvent, false otherwise
bool GeneratorWindows::plat_continue(ArchEvent* evt)
{
	// evt is null the first time we call this; however, 
	// the process also hasn't called WaitForDebugEvent the first time we call it.
	// thus, do nothing if evt == NULL.
	if(!evt) return true;
	ArchEventWindows* winEvt = static_cast<ArchEventWindows*>(evt);
	int_process* process_for_cur_thread = thread_to_proc[DThread::self()];
	if(!process_for_cur_thread || winEvt->evt.dwProcessId != process_for_cur_thread->getPid())
	{
		pthrd_printf("GeneratorWindows::plat_continue() bailing, called on thread for %d, got event on %d\n", 
			process_for_cur_thread->getPid(),
			winEvt->evt.dwProcessId);
		return true;
	}

	pthrd_printf("GeneratorWindows::plat_continue() for %d waiting\n", winEvt->evt.dwProcessId);
	::WaitForSingleObject(waiters[winEvt->evt.dwProcessId]->gen_wait, INFINITE);
	Waiters::ptr theWaiter = waiters[winEvt->evt.dwProcessId];
	if(!theWaiter) {
		pthrd_printf("Wait object deleted. Process %d should be dead. Returning.\n", winEvt->evt.dwProcessId);
		setState(exiting);
		return false;
	}

	ProcPool()->condvar()->lock();
	// ADDED 9NOV11 - Bernat
	// Move suspend/resume thread behavior to the Generator thread. 
	pthrd_printf("Process %d: setting thread suspend/resume and generator states before ContinueDebugEvent\n", winEvt->evt.dwProcessId);
	int_threadPool *tp = process_for_cur_thread->threadPool(); assert(tp);
	for (int_threadPool::iterator iter = tp->begin(); iter != tp->end(); ++iter) {
		windows_thread *winthr = dynamic_cast<windows_thread *>(*iter); assert(winthr);
		winthr->setGeneratorState(winthr->getHandlerState());
		switch(winthr->getGeneratorState()) {
			case int_thread::neonatal:
			case int_thread::neonatal_intermediate:
			case int_thread::running:
				pthrd_printf("%d/%d: state is %s, setting suspend count to 0\n",
					winthr->llproc()->getPid(), winthr->getLWP(), int_thread::stateStr(winthr->getGeneratorState()));
				winthr->plat_setSuspendCount(0);
				break;
			case int_thread::stopped:
				pthrd_printf("%d/%d: state is %s, setting suspend count to 1\n",
					winthr->llproc()->getPid(), winthr->getLWP(), int_thread::stateStr(winthr->getGeneratorState()));
				winthr->plat_setSuspendCount(1);
				break;
			case int_thread::detached:
			case int_thread::errorstate:
			case int_thread::exited:
				pthrd_printf("%d/%d: state is %s, not setting suspend count\n",
					winthr->llproc()->getPid(), winthr->getLWP(), int_thread::stateStr(winthr->getGeneratorState()));
				break;
			default:
				assert(0);
				break;
		}
	}
	alreadyHandled[process_for_cur_thread->getPid()]++;
	pthrd_printf("%d: incrementing sequence num, now %lld\n", 
		process_for_cur_thread->getPid(), alreadyHandled[process_for_cur_thread->getPid()]);
	windows_process *winproc = dynamic_cast<windows_process *>(process_for_cur_thread);
	winproc->lowlevel_processResumed();
	// End added 9NOV11
	ProcPool()->condvar()->unlock();

	DWORD cont_val = theWaiter->unhandled_exception ? DBG_EXCEPTION_NOT_HANDLED : DBG_CONTINUE;
	pthrd_printf("::ContinueDebugEvent for %d/%d getting called\n", winEvt->evt.dwProcessId, winEvt->evt.dwThreadId);
	//cerr << "continueDebugEvent" << endl;
	BOOL retval = ::ContinueDebugEvent(winEvt->evt.dwProcessId, winEvt->evt.dwThreadId, cont_val);
	if (!retval) {
		std::cerr << "ContinueDebugEvent failed, you're so screwed." << std::endl;
	}
	waiters[winEvt->evt.dwProcessId]->unhandled_exception = false;
	pthrd_printf("GeneratorWindows::plat_continue() for %d done with ::ContinueDebugEvent()\n", winEvt->evt.dwProcessId);
	::SetEvent(theWaiter->user_wait);
	pthrd_printf("GeneratorWindows::plat_continue() for %d done with signal()\n", winEvt->evt.dwProcessId);
	return true;
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

void GeneratorWindows::wake(Dyninst::PID p, long long sequence)
{
	pthrd_printf("GeneratorWindows::wake() for %d\n", p);
	if(sequence < alreadyHandled[p]) {
		pthrd_printf("GeneratorWindows::wake() rejecting sequence %lld for %d (threshold %lld)\n", sequence,
			p, alreadyHandled[p]);
		return;
	}
	Waiters::ptr w = waiters[p];
	if(w) {
		//cerr << "Sending wake event" << endl;
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