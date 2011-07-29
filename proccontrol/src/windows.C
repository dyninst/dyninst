/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

#include <assert.h>
#include <time.h>
#include <numeric>


#include "dynutil/h/dyn_regs.h"
#include "dynutil/h/dyntypes.h"
#include "symtabAPI/h/Symtab.h"
#include "common/h/pathName.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Handler.h"
#include "proccontrol/h/Mailbox.h"

#include "proccontrol/src/procpool.h"
#include "proccontrol/src/irpc.h"
#include "proccontrol/src/windows.h"
#include "proccontrol/src/int_handler.h"
#include "proccontrol/src/response.h"
#include "proccontrol/src/int_event.h"

#include "proccontrol/src/snippets.h"

#include "common/h/parseauxv.h"

#include <sstream>
#include <iostream>



int_process *int_process::createProcess(Dyninst::PID p, std::string e)
{
	assert(!"Not implemented");
	return NULL;
}

std::string concatArgs(const std::string& lhs, const std::string& rhs)
{
	return lhs + " " + rhs;
}

int_process *int_process::createProcess(std::string e, std::vector<std::string> a, std::vector<std::string> envp, 
        std::map<int,int> f)
{
	return new windows_process(0, e, a, envp, f);
}

int_process *int_process::createProcess(Dyninst::PID pid_, int_process *p)
{
	assert(!"Not implemented");
	return NULL;
}

Dyninst::Architecture windows_process::getTargetArch()
{
	// Fix this when we add 64-bit windows support...
	return Dyninst::Arch_x86;
}

windows_process::windows_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
                             std::vector<std::string> envp,  std::map<int,int> f) :
   int_process(p, e, a, envp, f),
   arch_process(p, e, a, envp, f),
   pendingDetach(false),
   pendingDebugBreak_(false)
{
}

windows_process::windows_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   arch_process(pid_, p),
   pendingDetach(false),
   pendingDebugBreak_(false)
{
}

windows_process::~windows_process()
{
	GeneratorWindows* winGen = static_cast<GeneratorWindows*>(GeneratorWindows::getDefaultGenerator());
	winGen->removeProcess(this);
}


bool windows_process::plat_create()
{
	GeneratorWindows* gen = static_cast<GeneratorWindows*>(Generator::getDefaultGenerator());
	gen->enqueue_event(GeneratorWindows::create, this);
	gen->launch();
	return true;
}

HANDLE windows_process::plat_getHandle()
{
	return hproc;
}

bool windows_process::plat_create_int()
{
	std::string args = std::accumulate(argv.begin(), argv.end(), std::string(), &concatArgs);
	LPCH mutator_env = ::GetEnvironmentStrings();
	static const int dir_size = 1024;
	char directory[dir_size];
	::GetCurrentDirectory(dir_size, directory);
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION procInfo;
	memset(&startupInfo, 0, sizeof(STARTUPINFO));
	startupInfo.cb = sizeof(STARTUPINFO);
	bool result = ::CreateProcess(executable.c_str(), const_cast<char*>(args.c_str()), NULL, NULL, TRUE, 
		DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS,
		mutator_env, 
		directory,
		&startupInfo,
		&procInfo);
	if(result)
	{
		pid = procInfo.dwProcessId;
		hproc = procInfo.hProcess;
		int_thread* initialThread = int_thread::createThread(this, (Dyninst::THR_ID)(procInfo.dwThreadId), 
			(Dyninst::LWP)procInfo.dwThreadId, false);
		threadPool()->setInitialThread(initialThread);
		windows_thread* wThread = dynamic_cast<windows_thread*>(initialThread);
		wThread->setHandle(procInfo.hThread);
	}
	return result;
}

void windows_thread::setHandle(HANDLE h)
{
	hthread = h;
}

bool windows_process::plat_attach_int()
{
	assert(!"not implemented");
	return false;
}

bool windows_process::plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates) 
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::plat_attach(bool)
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::plat_attachWillTriggerStop() 
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::plat_execed()
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::plat_forked()
{
	assert(!"Not implemented");
	return false;
}

bool windows_process::plat_readMem(int_thread *thr, void *local, 
                                 Dyninst::Address remote, size_t size)
{
	int errcode = ::ReadProcessMemory(hproc, (unsigned char*)remote, (unsigned char*)local, size, NULL);
	if(!errcode) {
		errcode = ::GetLastError();
		return false;
	}
	return true;
}

bool windows_process::plat_writeMem(int_thread *thr, const void *local, 
                                  Dyninst::Address remote, size_t size)
{
	int lasterr = 0;
	bool ok = ::WriteProcessMemory(hproc, (void*)remote, local, size, NULL);
	if(ok)
	{
		if(FlushInstructionCache(hproc, NULL, 0))
		{
			return true;
		}
	}
	lasterr = GetLastError();
	return false;
}


bool windows_process::needIndividualThreadAttach()
{
	return false;
}

bool windows_process::plat_supportLWPEvents() const
{
	return true;
}

bool windows_process::getThreadLWPs(std::vector<Dyninst::LWP> &lwps)
{
	for(int_threadPool::iterator i = threadPool()->begin();
		i != threadPool()->end();
		++i)
	{
		lwps.push_back((*i)->getLWP());
	}
	return true;
}

int_process::ThreadControlMode int_process::getThreadControlMode() 
{
    return int_process::IndependentLWPControl;
}
#if !defined(TF_BIT)
#define TF_BIT 0x100
#endif

WinHandleSingleStep::WinHandleSingleStep() :
   Handler("Windows Single Step")
{
}

WinHandleSingleStep::~WinHandleSingleStep()
{
}

Handler::handler_ret_t WinHandleSingleStep::handleEvent(Event::ptr ev)
{
	int_thread* t = ev->getThread()->llthrd();
	assert(t);
	t->setSingleStepMode(false);
	t->markClearingBreakpoint(NULL);
	return ret_success;
}

int WinHandleSingleStep::getPriority() const
{
   return PostPlatformPriority;
}

void WinHandleSingleStep::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::SingleStep));
}

bool windows_thread::plat_cont()
{
	GeneratorWindows* wGen = static_cast<GeneratorWindows*>(Generator::getDefaultGenerator());
	if(singleStep())
	{
		CONTEXT context;
		int result;
		context.ContextFlags = CONTEXT_FULL;
		result = GetThreadContext(hthread, &context);
		if(!result) {
			pthrd_printf("Couldn't get thread context\n");
			return false;
		}

		context.ContextFlags = CONTEXT_FULL;
		pthrd_printf("Enabling single-step on %d/%d\n", proc()->getPid(), tid);
		context.EFlags |= TF_BIT;
		result = SetThreadContext(hthread, &context);
		if(!result)
		{
			pthrd_printf("Couldn't set thread context to single-step thread\n");
			return false;
		}
	}
	::ResumeThread(hthread);
	windows_process* wproc = dynamic_cast<windows_process*>(llproc());
	wGen->wake(llproc()->getPid());
	//wGen->wait(llproc()->getPid());
	return true;
}

SymbolReaderFactory *windows_process::plat_defaultSymReader()
{
	// Singleton this jive
	assert(!"Not implemented");
	return NULL;
}


int_thread *int_thread::createThreadPlat(int_process *proc, 
                                         Dyninst::THR_ID thr_id, 
                                         Dyninst::LWP lwp_id,
                                         bool initial_thrd)
{
   if (initial_thrd) {
	   return NULL;
   }
   windows_thread *wthrd = new windows_thread(proc, thr_id, lwp_id);
   assert(wthrd);
   return static_cast<int_thread *>(wthrd);
}

windows_thread::windows_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l),
	   hthread(INVALID_HANDLE_VALUE)
{
}

windows_thread::~windows_thread()
{
}

bool windows_thread::plat_stop()
{
	pthrd_printf("Stopping thread %d (0x%lx)\n", getLWP(),
		hthread);
	windows_process* wproc = dynamic_cast<windows_process*>(llproc());
	int result = -1;
	if(wproc->pendingDebugBreak())
	{
		return true;
	}
	else
	{
		result = ::DebugBreakProcess(wproc->plat_getHandle());
		if(result == -1) {
			int err = ::GetLastError();
		}
		wproc->setPendingDebugBreak();
	}
	return result != -1;
}

void windows_thread::setOptions()
{
	// Should be a no-op on Windows...
}

bool windows_process::plat_individualRegAccess()
{
   return false;
}

bool windows_process::plat_detach()
{
	pendingDetach = true;
	if(pendingDebugBreak()) {
		return true;
	}
	setPendingDebugBreak();
	int result = ::DebugBreakProcess(hproc);
	if(!result)
	{
		int error = ::GetLastError();
		pthrd_printf("Error in plat_detach: %d\n", error);
	}
	return result;
}

bool windows_process::plat_terminate(bool &needs_sync)
{
   pthrd_printf("Terminating process %d\n", getPid());
   int result = ::TerminateProcess(hproc, 0);
   if (result == 0) {
/*	   if (::GetLastError() == ) {
         perr_printf("Process %d no longer exists\n", getPid());
         setLastError(err_noproc, "Process no longer exists");
      }
      else {
*/         perr_printf("Failed to terminate(%d) process\n", getPid());
         setLastError(err_internal, "Unexpected failure of TerminateProcess\n");
         return false;
/*      }*/
   }

   needs_sync = true;
   return true;
}

Dyninst::Address windows_process::plat_mallocExecMemory(Dyninst::Address min, unsigned size) 
{
	assert(!"Not implemented");
	return 0;
}


bool windows_thread::plat_getAllRegisters(int_registerPool &regpool)
{
	CONTEXT c;
	c.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	if(::GetThreadContext(hthread, &c))
	{
		regpool.regs[x86::eax] = c.Eax;
		regpool.regs[x86::ebx] = c.Ebx;
		regpool.regs[x86::ecx] = c.Ecx;
		regpool.regs[x86::edx] = c.Edx;
		regpool.regs[x86::esp] = c.Esp;
		regpool.regs[x86::ebp] = c.Ebp;
		regpool.regs[x86::esi] = c.Esi;
		regpool.regs[x86::edi] = c.Edi;
		regpool.regs[x86::flags] = c.ContextFlags;
		regpool.regs[x86::dr0] = c.Dr0;
		regpool.regs[x86::dr1] = c.Dr1;
		regpool.regs[x86::dr2] = c.Dr2;
		regpool.regs[x86::dr3] = c.Dr3;
		regpool.regs[x86::dr6] = c.Dr6;
		regpool.regs[x86::dr7] = c.Dr7;
		regpool.regs[x86::cs] = c.SegCs;
		regpool.regs[x86::ds] = c.SegDs;
		regpool.regs[x86::es] = c.SegEs;
		regpool.regs[x86::fs] = c.SegFs;
		regpool.regs[x86::gs] = c.SegGs;
		regpool.regs[x86::ss] = c.SegSs;
		regpool.regs[x86::eip] = c.Eip;
		return true;
	}
	else
	{
		return false;
	}
}

bool windows_thread::plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val)
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::plat_setAllRegisters(int_registerPool &regpool) 
{
	CONTEXT c;
	c.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
	c.Eax = regpool.regs[x86::eax];
	c.Ebx = regpool.regs[x86::ebx];
	c.Ecx = regpool.regs[x86::ecx];
	c.Edx = regpool.regs[x86::edx];
	c.Esp = regpool.regs[x86::esp];
	c.Ebp = regpool.regs[x86::ebp];
	c.Esi = regpool.regs[x86::esi];
	c.Edi = regpool.regs[x86::edi];
	c.ContextFlags = regpool.regs[x86::flags];
	c.Dr0 = regpool.regs[x86::dr0];
	c.Dr1 = regpool.regs[x86::dr1];
	c.Dr2 = regpool.regs[x86::dr2];
	c.Dr3 = regpool.regs[x86::dr3];
	c.Dr6 = regpool.regs[x86::dr6];
	c.Dr7 = regpool.regs[x86::dr7];
	c.SegCs = regpool.regs[x86::cs];
	c.SegDs = regpool.regs[x86::ds];
	c.SegEs = regpool.regs[x86::es];
	c.SegFs = regpool.regs[x86::fs];
	c.SegGs = regpool.regs[x86::gs];
	c.SegSs = regpool.regs[x86::ss];
	c.Eip = regpool.regs[x86::eip];
	return ::SetThreadContext(hthread, &c);

}

bool windows_thread::plat_convertToSystemRegs(const int_registerPool &regpool, unsigned char *user_area) 
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val)
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::attach()
{
	// All threads on windows are attached automatically.
	assert(getInternalState() == neonatal);
	return true;
}

bool windows_thread::plat_getThreadArea(int val, Dyninst::Address &addr)
{
	assert(!"Not implemented");
	return false;
}

ArchEventWindows::ArchEventWindows(DEBUG_EVENT e) :
evt(e)
{
}
      
ArchEventWindows::~ArchEventWindows()
{
}

std::vector<ArchEventWindows *> ArchEventWindows::pending_events;

bool ArchEventWindows::findPairedEvent(ArchEventWindows* &parent, ArchEventWindows* &child)
{
	assert(!"Not implemented");
	return false;
}

void ArchEventWindows::postponePairedEvent()
{
   pending_events.push_back(this);
}

WindowsHandleNewThr::WindowsHandleNewThr() :
   Handler("Windows New Thread")
{
}

WindowsHandleNewThr::~WindowsHandleNewThr()
{
}

Handler::handler_ret_t WindowsHandleNewThr::handleEvent(Event::ptr ev)
{
   windows_thread *thr = NULL;
   if (ev->getEventType().code() == EventType::Bootstrap) {
	  int_thread* tmp = ev->getThread()->llthrd();
	  thr = static_cast<windows_thread*>(tmp);
   }
   else {
      Dyninst::LWP lwp = static_cast<EventNewThread *>(ev.get())->getLWP();
      ProcPool()->condvar()->lock();
	  int_thread* tmp = ProcPool()->findThread(lwp);
	  thr = static_cast<windows_thread*>(tmp);
      ProcPool()->condvar()->unlock();
   }
   assert(thr);
                                        
   WinEventNewThread::ptr we = dyn_detail::boost::shared_dynamic_cast<WinEventNewThread>(ev);
   if(we)
   {
	   thr->setHandle(we->getHandle());
   }
   return ret_success;
}

int WindowsHandleNewThr::getPriority() const
{
   return PostPlatformPriority;
}

int WinHandleThreadStop::getPriority() const
{
   return PostPlatformPriority;
}


void WindowsHandleNewThr::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::ThreadCreate));
   etypes.push_back(EventType(EventType::None, EventType::Bootstrap));
   etypes.push_back(EventType(EventType::None, EventType::LWPCreate));
}

WindowsHandleLWPDestroy::WindowsHandleLWPDestroy()
    : Handler("Windows LWP Destroy")
{
}

WindowsHandleLWPDestroy::~WindowsHandleLWPDestroy()
{
}

Handler::handler_ret_t WindowsHandleLWPDestroy::handleEvent(Event::ptr ev) 
{
	assert(!"Not implemented");
    return ret_success;
}

int WindowsHandleLWPDestroy::getPriority() const
{
    return PostPlatformPriority;
}

void WindowsHandleLWPDestroy::getEventTypesHandled(std::vector<EventType> &etypes)
{
    etypes.push_back(EventType(EventType::Pre, EventType::LWPDestroy));
}

HandlerPool *plat_createDefaultHandlerPool(HandlerPool *hpool)
{
   static bool initialized = false;
   static WindowsHandleNewThr *wbootstrap = NULL;
   static WindowsHandleLWPDestroy* wthreaddestroy = NULL;
   static WinHandleThreadStop* winthreadstop = NULL;
   static WinHandleSingleStep* wsinglestep = NULL;
   if (!initialized) {
      wbootstrap = new WindowsHandleNewThr();
      wthreaddestroy = new WindowsHandleLWPDestroy();
	  winthreadstop = new WinHandleThreadStop();
	  wsinglestep = new WinHandleSingleStep();
      initialized = true;
   }
   hpool->addHandler(wbootstrap);
   hpool->addHandler(wthreaddestroy);
   hpool->addHandler(winthreadstop);
   hpool->addHandler(wsinglestep);
   return hpool;
}

bool ProcessPool::LWPIDsAreUnique()
{
	return true;
}


bool windows_process::plat_convertToBreakpointAddress(Dyninst::psaddr_t& addr)
{
	assert(!"Not implemented");
	return false;
}

bool windows_thread::plat_needsPCSaveBeforeSingleStep()
{
	// FIXME: is this actually right?
	return false;
}

bool windows_thread::plat_needsEmulatedSingleStep(std::vector<Dyninst::Address> &result)
{
	// NOTE: this modifies the vector of addresses with locations that need emulation.
	// true/false is an error return, not a "do we need emulation" return!
	return true;
}

bool windows_process::plat_createAllocationSnippet(Dyninst::Address addr, bool use_addr,
												   unsigned long size, void*& buffer, unsigned long& buffer_size,
												   unsigned long& start_offset)
{
	assert(!"not implemented");
	return false;
}

unsigned int windows_process::getTargetPageSize()
{
	assert(!"not implemented");
	return 0;
}

bool windows_process::plat_createDeallocationSnippet(Dyninst::Address addr, unsigned long size,
													 void*& buffer, unsigned long& buffer_size,
													 unsigned long& start_offset)
{
	assert(!"not implemented");
	return false;
}

bool windows_process::plat_collectAllocationResult(int_thread* thr, reg_response::ptr resp)
{
	assert(!"not implemented");
	return false;
}

bool windows_process::refresh_libraries(std::set<int_library *> &added_libs,
                                  std::set<int_library *> &rmd_libs,
                                  std::set<response::ptr> &async_responses)
{
	// no-op on Windows; this is handled at event time
	return true;
}

int_library* windows_process::getExecutableLib()
{
	assert(!"not implemented");
	return NULL;
}

bool windows_process::initLibraryMechanism()
{
	// Not needed on Windows
	return true;
}

bool windows_process::plat_isStaticBinary()
{
	assert(!"not implemented");
	return false;
}

bool windows_thread::plat_suspend()
{
	int result = ::SuspendThread(hthread);
	return result != -1;
}

bool windows_thread::plat_resume()
{
	int result = ::ResumeThread(hthread);
	return result != -1;
}

bool windows_thread::haveUserThreadInfo()
{
	assert(!"not implemented");
	return false;
}

bool windows_thread::getTID(Dyninst::THR_ID& tid)
{
	tid = this->tid;
	return true;
}

bool windows_thread::getStartFuncAddress(Dyninst::Address& start_addr)
{
	assert(!"not implemented");
	return false;
}

bool windows_thread::getStackBase(Dyninst::Address& stack_base)
{
	assert(!"not implemented");
	return false;
}

bool windows_thread::getStackSize(unsigned long& stack_size)
{
	assert(!"not implemented");
	return false;
}

bool windows_thread::getTLSPtr(Dyninst::Address& tls_ptr)
{
	assert(!"not implemented");
	return false;
}
