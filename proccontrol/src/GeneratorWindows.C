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
		if(!proc->plat_create_int()) { 
			proc->setState(int_process::exited);
			return;
		}
		break;
	case attach:
		if(!proc->plat_attach_int()) { 
			proc->setState(int_process::exited);
			return;
		}
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
	int_process* p = thread_to_proc[::GetCurrentThreadId()];
	if(!p) {
		setState(exiting);
		pthrd_printf("hasLiveProc() found NULL process, returning FALSE\n");
		return false;
	}

   int num_running_threads = Counter::processCount(Counter::GeneratorRunningThreads, p);
   int num_non_exited_threads = Counter::processCount(Counter::GeneratorNonExitedThreads, p);
   int num_force_generator_blocking = Counter::processCount(Counter::ForceGeneratorBlock, p);

   if (num_running_threads) {
      pthrd_printf("Generator has running threads, returning true from hasLiveProc\n");
      return true;
   }
   if (!num_non_exited_threads) {
      pthrd_printf("Generator has all exited threads, returning false from hasLiveProc\n");
      return false;
   }
   if(num_force_generator_blocking) {
      pthrd_printf("Generator forcing blocking, returning true from hasLiveProc\n");
      return true;
   }
	return false;
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

	windows_process *winproc = dynamic_cast<windows_process *>(process_for_cur_thread);

	Waiters::ptr theWaiter = waiters[winEvt->evt.dwProcessId];
	DWORD cont_val = theWaiter->unhandled_exception ? DBG_EXCEPTION_NOT_HANDLED : DBG_CONTINUE;
	pthrd_printf("::ContinueDebugEvent for %d/%d getting called, exception is %s\n", winEvt->evt.dwProcessId, winEvt->evt.dwThreadId, (theWaiter->unhandled_exception ? "<NOT HANDLED>" : "<HANDLED>"));
	//cerr << "continueDebugEvent" << endl;
	BOOL retval = ::ContinueDebugEvent(winEvt->evt.dwProcessId, winEvt->evt.dwThreadId, cont_val);
//	BOOL retval = ::ContinueDebugEvent(winEvt->evt.dwProcessId, winEvt->evt.dwThreadId, DBG_CONTINUE);
	if (!retval) {
		std::cerr << "ContinueDebugEvent failed, you're so screwed." << std::endl;
	}
	theWaiter->unhandled_exception = false;
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
		setState(error);
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