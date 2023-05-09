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
#include "windows_process.h"
#include "int_process.h"
#include "GeneratorWindows.h"
#include <numeric>
#include "windows_thread.h"
#include "procpool.h"
#include "irpc.h"
#include "Mailbox.h"
#include <iostream>
#include <psapi.h>
#include <winNT.h>
#include "symtabAPI/h/SymtabReader.h"

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
	int_process* ret = new windows_process(0, e, a, envp, f);
	return ret;
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
hybrid_lwp_control_process(p, e, a, envp, f),
x86_process(p, e, a, envp, f),
pendingDetach(false),
pendingDebugBreak_(false),
hproc(INVALID_HANDLE_VALUE),
hfile(INVALID_HANDLE_VALUE),
dummyRPCThread_(NULL),
m_executable(NULL),
stopthr_(0)
{
}

windows_process::windows_process(Dyninst::PID pid_, int_process *p) :
int_process(pid_, p),
hybrid_lwp_control_process(pid_, p),
x86_process(pid_, p),
pendingDetach(false),
pendingDebugBreak_(false),
hproc(INVALID_HANDLE_VALUE),
hfile(INVALID_HANDLE_VALUE),
dummyRPCThread_(NULL),
m_executable(NULL),
stopthr_(0)
{
}


windows_process::~windows_process()
{
	ProcessPool *pp = ProcPool();
	GeneratorWindows* winGen = static_cast<GeneratorWindows*>(GeneratorWindows::getDefaultGenerator());
	pp->condvar()->lock();
	winGen->removeProcess(this);
	pp->condvar()->unlock();
	// Do NOT close the process handle; that's handled by ContinueDebugEvent. Closing the file is okay.
	::CloseHandle(hfile);
	
}


bool windows_process::plat_create()
{
	wasCreatedViaAttach(false);
	GeneratorWindows* gen = static_cast<GeneratorWindows*>(Generator::getDefaultGenerator());
	gen->enqueue_event(GeneratorWindows::create, this);
	gen->launch();
	return gen->getState() != Generator::error;
}

HANDLE windows_process::plat_getHandle()
{
	return hproc;
}

void windows_process::plat_setHandles(HANDLE hp, HANDLE hf, Address eb)
{
	hproc = hp;
	hfile = hf;
	execBase = eb;
	std::string fileName;
#if 1 // vista or greater
	char filename[MAX_PATH+1];
	int bytes_obtained = GetFinalPathNameByHandle(hf, filename, MAX_PATH, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	if(bytes_obtained < MAX_PATH) fileName = std::string(filename+4); // skip \\?\ 
#else
	void *pmap = NULL;
    HANDLE fmap = CreateFileMapping(hfile, NULL, 
                                    PAGE_READONLY, 0, 1, NULL);
    if (fmap) {
        pmap = MapViewOfFile(fmap, FILE_MAP_READ, 0, 0, 1);
        if (pmap) {   
            char filename[MAX_PATH+1];
            int result = GetMappedFileName(GetCurrentProcess(), pmap, filename, MAX_PATH);
            if (result)
                fileName = std::string(filename);
            UnmapViewOfFile(pmap);
        }
        CloseHandle(fmap);
    }
#endif
	m_executable = new int_library(fileName, false, execBase, execBase);

}

bool windows_process::plat_create_int()
{
	pthrd_printf("windows_process::plat_create_int\n");
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
		pthrd_printf("Created mutatee process: pid %d\n", pid);
		hproc = procInfo.hProcess;
		int_thread* initialThread = int_thread::createThread(this, (Dyninst::THR_ID)(procInfo.dwThreadId), 
                                                           (Dyninst::LWP)procInfo.dwThreadId, true,
                                                           int_thread::as_created_attached);
		windows_thread* wThread = dynamic_cast<windows_thread*>(initialThread);
		wThread->setHandle(procInfo.hThread);
	}
	else
	{
		printf("Create process failed (%d) \n", GetLastError());
	}
	
	return result ? true : false;
}
bool windows_process::plat_attach(bool, bool &)
{
	wasCreatedViaAttach(true);
	GeneratorWindows* gen = static_cast<GeneratorWindows*>(Generator::getDefaultGenerator());
	gen->enqueue_event(GeneratorWindows::attach, this);
	gen->launch();
	return gen->getState() != Generator::error;
}

bool windows_process::plat_attach_int()
{
	getStartupTeardownProcs().inc();
	bool ret = false;
	if (::DebugActiveProcess(pid)) ret = true;
	return ret;
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

bool windows_process::plat_decodeMemoryRights(Process::mem_perm& perm,
                                              unsigned long rights) {
    switch (rights) {
      default:                     return false;
      case PAGE_NOACCESS:          perm.clrR().clrW().clrX();
      case PAGE_READONLY:          perm.setR().clrW().clrX();
      case PAGE_EXECUTE:           perm.clrR().clrW().setX();
      case PAGE_READWRITE:         perm.setR().setW().clrX();
      case PAGE_EXECUTE_READ:      perm.setR().clrW().setX();
      case PAGE_EXECUTE_READWRITE: perm.setR().setW().setX();
    }

    return true;
}

bool windows_process::plat_encodeMemoryRights(Process::mem_perm perm,
                                              unsigned long& rights) {
    if (perm.isNone()) { 
        rights = PAGE_NOACCESS;
    } else if (perm.isR()) {
        rights = PAGE_READONLY;
    } else if (perm.isX()) {
        rights = PAGE_EXECUTE;
    } else if (perm.isRW()) {
        rights = PAGE_READWRITE;
    } else if (perm.isRX()) {
        rights = PAGE_EXECUTE_READ;
    } else if (perm.isRWX()) {
        rights = PAGE_EXECUTE_READWRITE;
    } else {
        return false;
    }

    return true;
}

bool windows_process::plat_getMemoryAccessRights(Dyninst::Address addr,
                                                 Process::mem_perm& perm) {
    MEMORY_BASIC_INFORMATION meminfo;
    memset(&meminfo, 0, sizeof(MEMORY_BASIC_INFORMATION));
    if (!VirtualQueryEx(hproc, (LPCVOID)addr, &meminfo,
                        sizeof(MEMORY_BASIC_INFORMATION))) {
        pthrd_printf("ERROR: failed to get access rights for page %lx, "
                     "error code %d\n",
                     addr, GetLastError());
        return false;
    }

    if (plat_decodeMemoryRights(perm, meminfo.Protect))
        return true;

    pthrd_printf("ERROR: unsupported rights for page %lx\n", addr);
    return false;
}

bool windows_process::plat_setMemoryAccessRights(Dyninst::Address addr,
                                                 size_t size,
                                                 Process::mem_perm perm,
                                                 Process::mem_perm& oldPerm) {
    DWORD rights;
    if (!plat_encodeMemoryRights(perm, rights)) {
        pthrd_printf("ERROR: unsupported rights for page %lx\n", addr);
        return false;
    }

    DWORD oldRights;

    if (!VirtualProtectEx(hproc, (LPVOID)(addr), (SIZE_T)size,
                          (DWORD)rights, (PDWORD)&oldRights)) {
        pthrd_printf("ERROR: failed to set access rights for page %lx, "
                     "error code %d\n",
                     addr, GetLastError());
        MEMORY_BASIC_INFORMATION meminfo;
        memset(&meminfo, 0, sizeof(MEMORY_BASIC_INFORMATION));
        VirtualQueryEx(hproc, (LPCVOID)(addr), &meminfo,
                       sizeof(MEMORY_BASIC_INFORMATION));
        pthrd_printf("ERROR DUMP: baseAddr 0x%lx, AllocationBase 0x%lx, "
                     "AllocationProtect 0x%lx, RegionSize 0x%lx, State 0x%lx, "
                     "Protect 0x%lx, Type 0x%lx\n",
                     meminfo.BaseAddress, meminfo.AllocationBase,
                     meminfo.AllocationProtect, meminfo.RegionSize,
                     meminfo.State, meminfo.Protect, meminfo.Type);
        return false;
    }

   if (plat_decodeMemoryRights(oldPerm, oldRights))
       return true;

   pthrd_printf("ERROR: unsupported rights for page %lx\n", addr);
   return false;
}

bool windows_process::plat_findAllocatedRegionAround(Dyninst::Address addr,
                                                     Process::MemoryRegion& memRegion) {
    MEMORY_BASIC_INFORMATION meminfo;
    memset(&meminfo, 0, sizeof(MEMORY_BASIC_INFORMATION));
    if (!VirtualQueryEx(hproc, (LPCVOID)addr, &meminfo, 
                        sizeof(MEMORY_BASIC_INFORMATION))) {
        pthrd_printf("ERROR: failed to get access rights for page %lx, "
                     "error code %d\n",
                     addr, GetLastError());
        return false;
    }

    assert(meminfo.State == MEM_COMMIT);
    pthrd_printf("VirtualQuery reports baseAddr 0x%lx, allocBase 0x%lx, "
                 "RegionSize 0x%lx, State 0x%lx\n", meminfo.BaseAddress,
                 meminfo.AllocationBase, meminfo.RegionSize, meminfo.State);

    memRegion.first = (Address) meminfo.AllocationBase;
    Address probeAddr = (Address) meminfo.BaseAddress + (Address) meminfo.RegionSize;
    Address endAddr;

    MEMORY_BASIC_INFORMATION probe;
    memset(&probe, 0, sizeof(MEMORY_BASIC_INFORMATION));
    do {
        endAddr = probeAddr;
        VirtualQueryEx(hproc, (LPCVOID)probeAddr, &probe,
                       sizeof(MEMORY_BASIC_INFORMATION));
        pthrd_printf("VirtualQuery reports baseAddr 0x%lx, allocBase 0x%lx, "
                     "RegionSize 0x%lx, State 0x%lx\n", probe.BaseAddress,
                     probe.AllocationBase, probe.RegionSize, probe.State);

        probeAddr = (Address) probe.BaseAddress + (Address) probe.RegionSize;
    } while ((probe.AllocationBase == meminfo.AllocationBase) &&
             // we're in the same allocation unit...
             (endAddr != probeAddr)); // we're making forward progress
    memRegion.second = endAddr;

    return true;
}

bool windows_process::plat_readMem(int_thread *thr, void *local, 
								   Dyninst::Address remote, size_t size)
{
	int errcode = 0;
	if(!::ReadProcessMemory(hproc, (unsigned char*)remote, (unsigned char*)local, size, NULL)) 
	{
		errcode = ::GetLastError();
		pthrd_printf("ReadProcessMemory() failed to get %zu bytes from 0x%lx, error %d\n",
			size, remote, errcode);
		return false;
	}
	return true;
}

void windows_process::dumpMemoryMap()
{
	for(LibraryPool::iterator i = libpool.begin();
		i != libpool.end();
		++i)
	{
		pthrd_printf("Library %s:\t0x%lx\n",
			(*i)->getName().c_str(), (*i)->getLoadAddress());
	}
}

bool windows_process::plat_writeMem(int_thread *thr, const void *local, 
                                    Dyninst::Address remote, size_t size, bp_write_t)
{
	assert(local || !size);
	if(!local || !size) return false;
	int lasterr = 0;
	SIZE_T written = 0;
	written = 0;

	BOOL ok = ::WriteProcessMemory(hproc, (void*)remote, local, size, &written);
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

bool windows_process::plat_supportLWPCreate()
{
	return true;
}

bool windows_process::plat_supportLWPPreDestroy()
{
	return true;
}

bool windows_process::plat_supportLWPPostDestroy()
{
	return false;
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

SymbolReaderFactory *getPEReader()
{
  static SymbolReaderFactory *symreader_factory = NULL;
  if (symreader_factory)
    return symreader_factory;

  symreader_factory = new Dyninst::SymtabAPI::SymtabReaderFactory();
  return symreader_factory;
}


SymbolReaderFactory *windows_process::plat_defaultSymReader()
{
	return getPEReader();
}

bool windows_process::plat_detach(result_response::ptr did_detach, bool)
{
/*	pendingDetach = true;
	if(pendingDebugBreak()) {
		return true;
	}
	setPendingDebugBreak();
	pthrd_printf(".... detaching, calling DebugBreakProcess\n");
	BOOL result = ::DebugBreakProcess(hproc);
	if(!result)
	{
		int error = ::GetLastError();
		pthrd_printf("Error in plat_detach: %d\n", error);
	}
	return result ? true : false;
	*/
	int result = ::DebugActiveProcessStop(getPid());
	if(!result)
	{
		pthrd_printf("plat_detach() failed, DebugActiveProcessStop error %d\n", GetLastError());
		return false;
	}
	did_detach->getResultResponse()->setResponse(true);
	did_detach->getResultResponse()->markReady();
	return true;
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
	Dyninst::Address alloc_result = (Dyninst::Address)::VirtualAllocEx(hproc, (LPVOID)min, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if(alloc_result == 0) {
		pthrd_printf("mallocExecMemory failed to VirtualAllocEx %u bytes, error code %d\n", size, ::GetLastError());
	}
	return alloc_result;
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
Dyninst::Address windows_process::direct_infMalloc(unsigned long size, bool use_addr, Dyninst::Address addr)
{
	if(!use_addr) addr = 0;
	
	// TODO: share with find_free_mem
	static Address min_specific_size = 0;

	if (!min_specific_size) {
		SYSTEM_INFO sysinfo;
		::GetSystemInfo(&sysinfo);
		min_specific_size = (Address) sysinfo.dwAllocationGranularity;
	}

	if (addr) {
		size = (((size + min_specific_size - 1) / min_specific_size) * min_specific_size);
	}

	Dyninst::Address result = (Dyninst::Address)(::VirtualAllocEx(hproc, (LPVOID)addr, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
	if(result == 0) {
		pthrd_printf("infMalloc failed to VirtualAllocEx %u bytes, error code %d\n", size, ::GetLastError());
		fprintf(stderr, "infMalloc failed to VirtualAllocEx %u bytes, error code %d\n", size, ::GetLastError());
		MEMORY_BASIC_INFORMATION info;
		memset(&info, 0, sizeof(MEMORY_BASIC_INFORMATION));
		VirtualQueryEx(hproc, (LPCVOID) (Address) addr,
						&info,
						sizeof(MEMORY_BASIC_INFORMATION));
		cerr << "Mutator side: " << hex << addr << "/" << size << " / " << info.BaseAddress << " / " << info.AllocationBase
			<< " / " << info.RegionSize << " / " << info.State << dec << endl;

	} else {
		mem->inf_malloced_memory[result] = size;
	}
	return result;
}
bool windows_process::direct_infFree(Dyninst::Address addr)
{
	std::map<Dyninst::Address, unsigned long>::iterator i = mem->inf_malloced_memory.find(addr);
	if (i == mem->inf_malloced_memory.end()) {
		setLastError(err_badparam, "Unknown address passed to freeMemory");
		perr_printf("Passed bad address, %lx, to infFree\n", addr);
		return false;
	}
	// HACK: short-circuit and don't deallocate.
	return true;
	cerr << "FREEING " << hex << addr << dec << endl;
	BOOL result = ::VirtualFreeEx(hproc, (LPVOID)addr, 0, MEM_RELEASE);
	if (!result) {
		fprintf(stderr, "VirtualFreeEx failed at 0x%lx, retval %d", addr, ::GetLastError());
	}
	mem->inf_malloced_memory.erase(i);
	return result ? true : false;
}

Address windows_process::plat_findFreeMemory(size_t size) {
	
	static Address low = 0;
	static Address hi = 0;
	static Address size_req = 0;

	if (!low) {
		SYSTEM_INFO sysinfo;
		::GetSystemInfo(&sysinfo);
		low = (Address) sysinfo.lpMinimumApplicationAddress;
		hi = (Address) sysinfo.lpMaximumApplicationAddress;
		size_req = (Address) sysinfo.dwAllocationGranularity;
	}

	size = (size > size_req) ? size : size_req;

	MEMORY_BASIC_INFORMATION info;
	Address result = low;
	bool done = false;
	while (!done && (result < hi)) {
		::VirtualQueryEx(hproc, (LPCVOID) result, &info, sizeof(MEMORY_BASIC_INFORMATION));
		if ((info.State == MEM_FREE) && 
			(size <= info.RegionSize)) {
			done = true;
		}
		else {
			result += size_req; // We may query the same range multiple times, but we need to 
			// stay aligned. 
		}
	}
	if (result >= hi) return 0;
	return result;
}

bool windows_process::plat_collectAllocationResult(int_thread* thr, reg_response::ptr resp)
{
	assert(!"not implemented");
	return false;
}

bool windows_process::refresh_libraries(std::set<int_library *> &added_libs,
										std::set<int_library *> &rmd_libs,
										bool& /* ignored */,
										std::set<response::ptr> &async_responses)
{
	// no-op on Windows; this is handled at event time
	return true;
}

int_library* windows_process::plat_getExecutable()
{
	return m_executable;
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

int_thread *iRPCMgr::createThreadForRPC(int_process* proc, int_thread* best_candidate)
{
	windows_process* winProc = dynamic_cast<windows_process*>(proc);
	assert(winProc);

	return winProc->createRPCThread(best_candidate);
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

int_thread *windows_process::RPCThread() {
	pthrd_printf("Query for RPC thread: ret %p\n",
		dummyRPCThread_);
	return dummyRPCThread_;
}

int_thread *windows_process::createRPCThread(int_thread* best_candidate) {
	if(best_candidate) return best_candidate;
	if (!dummyRPCThread_) {
		dummyRPCThread_ = static_cast<windows_thread *>(int_thread::createRPCThread(this));
		pthrd_printf("Creating RPC thread: %p\n", dummyRPCThread_);
	}
	else {
		pthrd_printf("Create RPC thread returning previous copy\n");
	}
	return dummyRPCThread_;
}


void windows_process::instantiateRPCThread() {
	assert(dummyRPCThread_);

	if (!dummyRPCThread_->isRPCpreCreate())
		return;

	pthrd_printf("Promoting dummy RPC thread %p to a real thread\n", dummyRPCThread_);

	// We want to:
	// 1) Take the dummy thread
	// 2) Create a real thread in the mutatee
	// 3) Fill in the dummy thread's values
	// 4) Set it as a system thread
	// 5) Force the generator to consume events 

	// 1)
	Address dummyStart = dummyRPCThread_->getDummyRPCStart();

	// 2)
	Dyninst::LWP lwp;
	HANDLE hthrd = ::CreateRemoteThread(plat_getHandle(), NULL, 0, (LPTHREAD_START_ROUTINE)dummyStart, NULL, 0, (LPDWORD)&lwp); // not create_suspended anymore
	pthrd_printf("*********************** Created actual thread with lwp %d, hthrd %x for dummy RPC thread, start at 0x%lx\n",
		(int) lwp, hthrd, dummyStart);

	// 3)
	dummyRPCThread_->updateThreadHandle((Dyninst::THR_ID) lwp, lwp);
	dummyRPCThread_->setHandle(hthrd);

	// 4) 
	dummyRPCThread_->setUser(false);
	dummyRPCThread_->markRPCRunning();

	getStartupTeardownProcs().inc();
	// And match whether we're actually suspended
	dummyRPCThread_->setSuspended(true);

}

void windows_process::destroyRPCThread() {
	if (!dummyRPCThread_) return;
	dummyRPCThread_ = NULL;
}


void* windows_process::plat_getDummyThreadHandle() const
{
	if(!dummyRPCThread_) return NULL;
	return dummyRPCThread_->plat_getHandle();
}

bool windows_process::plat_suspendThread(int_thread *thr)
{
	windows_thread* wthr = static_cast<windows_thread*>(thr);
	assert(wthr);
	return wthr->plat_suspend();
}

bool windows_process::plat_resumeThread(int_thread *thr)
{
	windows_thread* wthr = static_cast<windows_thread*>(thr);
	assert(wthr);
	return wthr->plat_resume();
}

ExecFileInfo* windows_process::plat_getExecutableInfo() const
{
	ExecFileInfo* ret = new ExecFileInfo();
	ret->fileHandle = (void*)hfile;
	ret->processHandle = (void*)hproc;
	ret->fileBase = execBase;
	return ret;
}

bool windows_process::pendingDebugBreak() const {
	pthrd_printf("win_proc: pending debug break %s\n",
		pendingDebugBreak_ ? "<true>" : "<false");
	return pendingDebugBreak_;
}

void windows_process::setPendingDebugBreak() {
	pthrd_printf("win_proc: setting pending debug break\n");
	pendingDebugBreak_ = true;
}

void windows_process::clearPendingDebugBreak() {
	pthrd_printf("win_proc: clearing pending debug break\n");
	pendingDebugBreak_ = false;
}

std::string int_process::plat_canonicalizeFileName(std::string path)
{
   return path;
}
