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
#include "windows_process.h"
#include "int_process.h"
#include "GeneratorWindows.h"
#include <numeric>
#include "windows_thread.h"
#include "procpool.h"
#include "irpc.h"
#include <iostream>

using namespace std;

std::string concatArgs(const std::string& lhs, const std::string& rhs)
{
	return lhs + " " + rhs;
}

int_process *int_process::createProcess(Dyninst::PID p, std::string e)
{
	std::vector<std::string> a;
	std::map<int,int> f;
	std::vector<std::string> envp;
	int_process* ret = new windows_process(p, e, a, envp, f);
	//ret->setForceGeneratorBlock(true);
	return ret;
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

int_process::ThreadControlMode int_process::getThreadControlMode() 
{
	return int_process::HybridLWPControl;
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
	wasCreatedViaAttach(false);
	GeneratorWindows* gen = static_cast<GeneratorWindows*>(Generator::getDefaultGenerator());
	gen->enqueue_event(GeneratorWindows::create, this);
	gen->launch();
	return true;
}

HANDLE windows_process::plat_getHandle()
{
	return hproc;
}

void windows_process::plat_setHandle(HANDLE h)
{
	hproc = h;
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
	BOOL result = ::CreateProcess(executable.c_str(), const_cast<char*>(args.c_str()), NULL, NULL, TRUE, 
		DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS,
		mutator_env, 
		directory,
		&startupInfo,
		&procInfo);
	if(result)
	{
		pid = procInfo.dwProcessId;
		hproc = procInfo.hProcess;
	}
#if 0 // We believe this is unnecessary and will happen in the CreateProcess event handler
	{
		int_thread* initialThread = int_thread::createThread(this, (Dyninst::THR_ID)(procInfo.dwThreadId), 
			(Dyninst::LWP)procInfo.dwThreadId, false);
		threadPool()->setInitialThread(initialThread);
		windows_thread* wThread = dynamic_cast<windows_thread*>(initialThread);
		wThread->setHandle(procInfo.hThread);
	}
#endif
	return result ? true : false;
}
bool windows_process::plat_attach(bool)
{
	wasCreatedViaAttach(true);
	GeneratorWindows* gen = static_cast<GeneratorWindows*>(Generator::getDefaultGenerator());
	gen->enqueue_event(GeneratorWindows::attach, this);
	gen->launch();
	return true;
}

bool windows_process::plat_attach_int()
{
	setForceGeneratorBlock(true);
	return (::DebugActiveProcess(pid)) ? true : false;
}

// For each LWP/TID, is it running at attach time?
bool windows_process::plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates) 
{
	vector<Dyninst::LWP> lwps;
	if( !getThreadLWPs(lwps) ) {
		pthrd_printf("Failed to determine lwps for process %d\n", getPid());
		setLastError(err_noproc, "Failed to find /proc files for debuggee");
		return false;
	}
	// MSDN: DebugActiveProcess() does the following:
	// * Suspend all threads
	// * Send CreateProcess
	// * Send LoadLibrary for each loaded lib
	// * Send CreateThread for each thread
	// * Send Breakpoint on the active thread
	// * Resume all threads when ContinueDebugEvent is called after the breakpoint.
	// So, despite the fact that the threads are suspended, we'll treat them all as 'running'
	// because that's where they wind up when we've handled all the events from attach().
	for(vector<Dyninst::LWP>::const_iterator it = lwps.begin();
		it != lwps.end();
		++it)
	{
		runningStates.insert(std::make_pair(*it, true));
	}
	return true;
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
	//fprintf(stderr, "reading %d bytes from %p\n", size, remote);
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
	//fprintf(stderr, "writing %d bytes to %p, first byte %x\n", size, remote, *((unsigned char*)local));
	BOOL ok = ::WriteProcessMemory(hproc, (void*)remote, local, size, NULL);
	if(ok)
	{
		if(FlushInstructionCache(hproc, NULL, 0))
		{
			return true;
		}
	}
	lasterr = GetLastError();
	pthrd_printf("Error writing memory: %d\n", lasterr);
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

bool windows_process::plat_contProcess()
{
	pthrd_printf("plat_contProcess...\n");
	ProcPool()->condvar()->lock();
	for(int_threadPool::iterator i = threadPool()->begin();
		i != threadPool()->end(); ++i)
	{
		if( (*i)->isResumed() ) {
			(*i)->setInternalState(int_thread::running);
			(*i)->setHandlerState(int_thread::running);
			(*i)->setGeneratorState(int_thread::running);
			(*i)->setResumed(false);
		}else if( (*i)->getInternalState() == int_thread::stopped ) {
			pthrd_printf("Suspending before continue %d/%d\n",
				getPid(), (*i)->getLWP());
			if( !(*i)->plat_suspend() ) {
				perr_printf("Failed to suspend thread %d/%d\n",
					getPid(), (*i)->getLWP());
				setLastError(err_internal, "low-level continue failed");
				return false;
			}
		}

		// DEBUGGING
		int_registerPool regs;
		(*i)->plat_getAllRegisters(regs);
		Dyninst::THR_ID tid;
		(*i)->getTID(tid);
		Address a;
		(*i)->getStartFuncAddress(a);
		pthrd_printf("Thread %d at EIP 0x%lx, thread start func is 0x%lx\n", tid, regs.regs[x86::eip], a);

		//


	}
	ProcPool()->condvar()->signal();
	ProcPool()->condvar()->unlock();
	// This implementation presumes that all threads are in correct suspended/resumed state.
	// With HybridLWPControl, this should be correct.
	GeneratorWindows* wGen = static_cast<GeneratorWindows*>(Generator::getDefaultGenerator());


	wGen->wake(pid);
	return true;
}
SymbolReaderFactory *windows_process::plat_defaultSymReader()
{
	// Singleton this jive
	assert(!"Not implemented");
	return NULL;
}

bool windows_process::plat_detach()
{
	pendingDetach = true;
	if(pendingDebugBreak()) {
		return true;
	}
	setPendingDebugBreak();
	BOOL result = ::DebugBreakProcess(hproc);
	if(!result)
	{
		int error = ::GetLastError();
		pthrd_printf("Error in plat_detach: %d\n", error);
	}
	return result ? true : false;
}

bool windows_process::plat_individualRegAccess()
{
	return false;
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
*/         perr_printf("Failed to terminate process %d (error %d)\n", getPid(), GetLastError());
         setLastError(err_internal, "Unexpected failure of TerminateProcess\n");
         return false;
/*      }*/
   }

   needs_sync = true;
   return true;
}

Dyninst::Address windows_process::plat_mallocExecMemory(Dyninst::Address min, unsigned size) 
{
	return (Dyninst::Address)(::VirtualAllocEx(hproc, (LPVOID)min, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE));
}

bool windows_process::plat_convertToBreakpointAddress(Dyninst::psaddr_t& addr)
{
	assert(!"Not implemented");
	return false;
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
	// hard-coded because Windows is boring.
	return 4096;
}

bool windows_process::plat_createDeallocationSnippet(Dyninst::Address addr, unsigned long size,
													 void*& buffer, unsigned long& buffer_size,
													 unsigned long& start_offset)
{
	assert(!"not implemented");
	return false;
}

// Windows lets us do this directly, so we'll just override these entirely on that platform.
Dyninst::Address windows_process::infMalloc(unsigned long size, bool use_addr, Dyninst::Address addr)
{
	if(!use_addr) addr = 0;
	Dyninst::Address result = (Dyninst::Address)(::VirtualAllocEx(hproc, (LPVOID)addr, size, MEM_COMMIT, PAGE_READWRITE));
	mem->inf_malloced_memory[result] = size;
	return result;
}
bool windows_process::infFree(Dyninst::Address addr)
{
	std::map<Dyninst::Address, unsigned long>::iterator i = mem->inf_malloced_memory.find(addr);
	if (i == mem->inf_malloced_memory.end()) {
		setLastError(err_badparam, "Unknown address passed to freeMemory");
		perr_printf("Passed bad address, %lx, to infFree\n", addr);
		return false;
	}

	BOOL result = ::VirtualFreeEx(hproc, (LPVOID)addr, 0, MEM_RELEASE);
	if (!result) {
		fprintf(stderr, "VirtualFreeEx failed at 0x%lx, retval %d", addr, ::GetLastError());
	}
	mem->inf_malloced_memory.erase(i);
	return result ? true : false;
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

bool iRPCMgr::createThreadForRPC(int_process* proc, int_iRPC::ptr rpc)
{
	fprintf(stderr, "Got asked to create thread for IRPC\n");
	rpc->setAllocation(proc->mallocExecMemory(rpc->allocSize));
	return false;
	// This assumes we've already allocated and copied...
/*	windows_process* winProc = dynamic_cast<windows_process*>(proc);
	assert(winProc);
	Dyninst::THR_ID tid;
	HANDLE hthrd = ::CreateRemoteThread(winProc->plat_getHandle(), NULL, 0, (LPTHREAD_START_ROUTINE)rpc->addr(), NULL, 0, (LPDWORD)&tid);
	return hthrd != INVALID_HANDLE_VALUE;*/
}

bool windows_process::addrInSystemLib(Address addr) {
	if (systemLibIntervals_.empty()) {
		findSystemLibs();
	}

	bool found;
	if (systemLibIntervals_.find(addr, found)) {
		return true;
	}
	return false;
}

void windows_process::findSystemLibs() {	
	// Let's try to find ntdll...
	int_library *ntdll = getLibraryByName("ntdll.dll");
	if (!ntdll) return;

	// We have a base addr, but no idea what the memory range is. Whee!
	Address objEnd = 0;
	Address probeAddr = ntdll->getAddr();
	Address base = 0;

    MEMORY_BASIC_INFORMATION probe;
    memset(&probe, 0, sizeof(MEMORY_BASIC_INFORMATION));
    do {
       objEnd = probeAddr;
       SIZE_T size2 = VirtualQueryEx(hproc,
                                     (LPCVOID) ((Address)probeAddr),
                                     &probe,
                                     sizeof(MEMORY_BASIC_INFORMATION));
	   if ((Address) base == 0) base = (Address) probe.AllocationBase;

	   probeAddr = (Address) probe.BaseAddress + (Address) probe.RegionSize;
    } while (((Address) probe.AllocationBase == base) && // we're in the same allocation unit...
 			 (objEnd != probeAddr)); // we're making forward progress
	bool insert = true;
	systemLibIntervals_.insert(base, objEnd, insert);
}
