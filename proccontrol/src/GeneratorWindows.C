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

#include "Generator.h"
#include "GeneratorWindows.h"
#include "DecoderWindows.h"
#include "Mailbox.h"
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
	pthrd_printf("GeneratorWindows::plat_start\n");
	if(procsToStart.empty())
		return;
	StartInfo todo = procsToStart.front();
	procsToStart.pop_front();
	windows_process* proc = dynamic_cast<windows_process*>(todo.proc);
	switch(todo.mode)
	{
	case create:
		pthrd_printf("Calling plat_create_int...\n");
		if(!proc->plat_create_int()) { 
			proc->setState(int_process::exited);
			setState(error);
			return;
		}
		break;
	case attach:
		if(!proc->plat_attach_int()) { 
			proc->setState(int_process::exited);
			setState(error);
			return;
		}
		break;
	default:
		assert(!"unknown mode in GeneratorWindows::plat_start, expected create or attach");
		break;
	}
	//waiters.insert(std::make_pair(proc->getPid(), Waiters::ptr(new Waiters)));
	//processes.insert(std::make_pair(proc->getPid(), proc));
	processData::ptr p(new processData);
	p->proc = proc;
	p->unhandled_exception = false;
	p->state = none;
	thread_to_proc.insert(std::make_pair(::GetCurrentThreadId(), p));
}

struct value_equal
{
	value_equal(int_process* v) : value(v) {}
	bool operator()(std::pair<int, GeneratorWindows::processData::ptr> element)
	{
		return element.second->proc == value;
	}
	int_process* value;
};

void GeneratorWindows::removeProcess(int_process *proc)
{
	//CriticalSection c(processDataLock);
	std::map<int, processData::ptr>::iterator found = std::find_if(thread_to_proc.begin(), thread_to_proc.end(),
		value_equal(proc));
	if(found != thread_to_proc.end()) {
		thread_to_proc.erase(found);
	}
}

bool GeneratorWindows::isExitingState()
{
	//CriticalSection c(processDataLock);
	if(thread_to_proc.find(DThread::self()) == thread_to_proc.end()) return false;
	return (thread_to_proc[DThread::self()]->state == error || thread_to_proc[DThread::self()]->state == exiting);
}


void GeneratorWindows::setState(Generator::state_t new_state)
{
   pthrd_printf("Setting generator state to %s\n", generatorStateStr(new_state));
    if(thread_to_proc.find(DThread::self()) == thread_to_proc.end()) {
        state = new_state;
	return;
    }
    if (isExitingState())
        return;
    thread_to_proc[DThread::self()]->state = new_state;
}

Generator::state_t GeneratorWindows::getState()
{
	//CriticalSection c(processDataLock);
	if(thread_to_proc.find(DThread::self()) == thread_to_proc.end()) return state;
   return thread_to_proc[DThread::self()]->state;
}

bool GeneratorWindows::hasLiveProc()
{
	//CriticalSection c(processDataLock);
	pthrd_printf("begin hasLiveProc()\n");
	if(thread_to_proc.find(DThread::self()) == thread_to_proc.end())
	{
		pthrd_printf("hasLiveProc() found NULL processInfo, returning FALSE\n");
		return false;
	}
	int_process* p = thread_to_proc[DThread::self()]->proc;
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
	
	pthrd_printf("Returning cached event %p for thread %d\n", m_Events[DThread::self()], DThread::self());
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
	//CriticalSection c(processDataLock);
	processData::ptr d = thread_to_proc[DThread::self()];
	int_process* process_for_cur_thread = d->proc;
	if(!process_for_cur_thread || winEvt->evt.dwProcessId != process_for_cur_thread->getPid())
	{
		pthrd_printf("GeneratorWindows::plat_continue() bailing, called on thread for %d, got event on %d\n", 
			process_for_cur_thread->getPid(),
			winEvt->evt.dwProcessId);
		return true;
	}

	windows_process *winproc = dynamic_cast<windows_process *>(process_for_cur_thread);

	DWORD cont_val = d->unhandled_exception ? DBG_EXCEPTION_NOT_HANDLED : DBG_CONTINUE;
	pthrd_printf("::ContinueDebugEvent for %d/%d getting called, exception is %s\n", winEvt->evt.dwProcessId, winEvt->evt.dwThreadId, (d->unhandled_exception ? "<NOT HANDLED>" : "<HANDLED>"));
	BOOL retval = ::ContinueDebugEvent(winEvt->evt.dwProcessId, winEvt->evt.dwThreadId, cont_val);
	if (!retval) {
		std::cerr << "ContinueDebugEvent failed, you're so screwed." << std::endl;
	}
	d->unhandled_exception = false;
	pthrd_printf("GeneratorWindows::plat_continue() for %d done with ::ContinueDebugEvent()\n", winEvt->evt.dwProcessId);
	if(winEvt->evt.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) {
		pthrd_printf("GeneratorWindows::plat_continue() exiting %d and throwing post-exit event\n", winEvt->evt.dwProcessId);
		ProcPool()->condvar()->lock();
		setState(statesync);
		EventExit::ptr e(new EventExit(EventType::Post, winEvt->evt.u.ExitProcess.dwExitCode));
		e->setProcess(winproc->proc());
		int_thread *thread = ProcPool()->findThread((Dyninst::LWP)(winEvt->evt.dwThreadId));
		e->setThread(thread->thread());		
		e->setSyncType(Event::sync_process);
		e->getProcess()->llproc()->updateSyncState(e, true);
		ProcPool()->condvar()->broadcast();
		ProcPool()->condvar()->unlock();
		setState(queueing);
		mbox()->enqueue(e);
		return true;
	}
	return true;
}

ArchEvent *GeneratorWindows::getEvent(bool block)
{
	DEBUG_EVENT evt;
	pthrd_printf("About to WaitForDebugEvent()\n");
	if(::WaitForDebugEvent(&evt, INFINITE))
	{
		ArchEventWindows* new_evt = new ArchEventWindows(evt);
		pthrd_printf("WaitForDebugEvent returned\n");
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


void GeneratorWindows::markUnhandledException(Dyninst::PID p)
{
	for(std::map<int, processData::ptr>::iterator i = thread_to_proc.begin();
		i != thread_to_proc.end();
		++i)
	{
		if(i->second->proc->getPid() == p) {
			i->second->unhandled_exception = true;
			return;
		}
	}
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
