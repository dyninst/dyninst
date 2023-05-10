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

#include "DecoderWindows.h"
#include "windows_process.h"
#include "windows_handler.h"
#include "windows_thread.h"
#include "ProcPool.h"
#include <iostream>
#include "irpc.h"
#include <psapi.h>
#include "procControl/h/Mailbox.h"
#include "int_event.h"

using namespace std;

DecoderWindows::DecoderWindows()
{
}

DecoderWindows::~DecoderWindows()
{
}

unsigned DecoderWindows::getPriority() const 
{
   return Decoder::default_priority;
}

Dyninst::Address DecoderWindows::adjustTrapAddr(Dyninst::Address addr, Dyninst::Architecture arch)
{
  if (arch == Dyninst::Arch_x86 || arch == Dyninst::Arch_x86_64) {
    return addr-1;
  }
  return addr;
}

void DecoderWindows::dumpSurroundingMemory( unsigned problemArea, int_process* proc )
{
	fprintf(stderr, "segfault in mutatee at %ux\n", problemArea);
	for(unsigned i = problemArea-16; i < problemArea+16; i++)
	{
		unsigned char tmp = 0;

		if(proc->plat_readMem(NULL, &tmp, i, 1)) {
			fprintf(stderr, "%ux: %x\n", i, tmp);
		}
		else {
			fprintf(stderr, "failed to read from %ux\n", i);
		}
	}
}


EventLibrary::ptr DecoderWindows::decodeLibraryEvent(DEBUG_EVENT details, int_process* p)
{
	windows_process* proc = dynamic_cast<windows_process*>(p);
	EventLibrary::ptr newEvt = EventLibrary::ptr(new EventLibrary());
	std::set<Library::ptr> addedLibs, removedLibs;
	HANDLE hproc = proc->plat_getHandle();
	
	if(details.dwDebugEventCode == LOAD_DLL_DEBUG_EVENT)
	{
		void* libnameaddr;
		std::string result = "";
		bool read = false;
		if (details.u.LoadDll.lpImageName) {
			read = p->plat_readMem(NULL, &libnameaddr, (Dyninst::Address)(details.u.LoadDll.lpImageName), 4);
		}
		//cerr << "imageName @ " << hex << details.u.LoadDll.lpImageName << " and read " << read << dec << endl;
		if(details.u.LoadDll.lpImageName && // NULL lpImageName = done
			read &&
			libnameaddr) // NULL libnameaddr = done
		{
			result = readLibNameFromProc((Address) libnameaddr, details, p);
		}
		else {
			result = HACKreadFromFile(details, p);
		}
		//cerr << "\t ... " << result << endl;
		int_library* lib = new int_library(result,
                                                   true,
                                                   (Dyninst::Address)(details.u.LoadDll.lpBaseOfDll),
                                                   (Dyninst::Address)(details.u.LoadDll.lpBaseOfDll));
		addedLibs.insert(lib->getUpPtr());

		pthrd_printf("Load DLL event, loading %s (at 0x%lx)\n",
			lib->getName().c_str(), lib->getAddr());
		//fprintf(stderr, "Load DLL event, loading %s (at 0x%lx)\n",
		//	lib->getName().c_str(), lib->getAddr());

	}
	else if(details.dwDebugEventCode == UNLOAD_DLL_DEBUG_EVENT)
	{
		std::set<int_library*>::iterator foundLib;
		for(foundLib = proc->memory()->libs.begin();
			foundLib != proc->memory()->libs.end();
			++foundLib)
		{
			if((*foundLib)->getAddr() == (Dyninst::Address)(details.u.UnloadDll.lpBaseOfDll))
			{
				pthrd_printf("Unload DLL event, unloading %s (was at 0x%lx)\n",
					(*foundLib)->getName().c_str(), (*foundLib)->getAddr());
				removedLibs.insert((*foundLib)->getUpPtr());
				//proc->memory()->libs.erase(foundLib);
				break;
			}
		}
	}
	newEvt->setLibs(addedLibs, removedLibs);
	newEvt->setSyncType(Event::sync_process);
	return newEvt;
}

Event::ptr DecoderWindows::decodeSingleStepEvent(DEBUG_EVENT e, int_process* proc, int_thread* thread)
{
	Event::ptr evt;
	// This handles user-level breakpoints; if we can't associate this with a thread we know about, it's not
	// something the user installed (or we're in an inconsistent state). Forward to the mutatee.
	if(!thread) return evt;

	assert(thread->singleStep());
	bp_instance *clearingbp = thread->isClearingBreakpoint();
	if(clearingbp) {
		pthrd_printf("Decoded event to breakpoint cleanup\n");
		evt = Event::ptr(new EventBreakpointRestore(new int_eventBreakpointRestore(clearingbp)));
	} else {
		evt = Event::ptr(new EventSingleStep());
	}

	CONTEXT verification;
	verification.ContextFlags = CONTEXT_FULL;
	::GetThreadContext(((windows_thread *)thread)->plat_getHandle(), &verification);

	if (verification.EFlags & TF_BIT) {
		pthrd_printf("BUG ENCOUNTERED: OS handled us a thread with TF set, clearing manually\n");
		verification.EFlags &= (~TF_BIT);
		::SetThreadContext(((windows_thread *)thread)->plat_getHandle(), &verification);
	}

	return evt;
}

Event::ptr DecoderWindows::decodeBreakpointEvent(DEBUG_EVENT e, int_process* proc, int_thread* thread)
{
	Event::ptr evt;
	// This handles user-level breakpoints; if we can't associate this with a thread we know about, it's not
	// something the user installed (or we're in an inconsistent state). Forward to the mutatee.
	if(!thread) return evt;
	Dyninst::Address adjusted_addr = (Dyninst::Address)(e.u.Exception.ExceptionRecord.ExceptionAddress);

	if (rpcMgr()->isRPCTrap(thread, adjusted_addr)) {
		pthrd_printf("Decoded event to rpc completion on %d/%d at %lx\n",
			proc->getPid(), thread->getLWP(), adjusted_addr);
		evt = Event::ptr(new EventRPC(thread->runningRPC()->getWrapperForDecode()));
		return evt;
	}

	sw_breakpoint *ibp = proc->getBreakpoint(adjusted_addr);

	if (ibp) {
	   pthrd_printf("Decoded breakpoint on %d/%d at %lx\n", proc->getPid(), 
					thread->getLWP(), adjusted_addr);
       EventBreakpoint::ptr event_bp = EventBreakpoint::ptr(new EventBreakpoint(new int_eventBreakpoint(adjusted_addr, ibp, thread)));
	   evt = event_bp;
	   evt->setThread(thread->thread());
	}
	return evt;

}

#define EXCEPTION_DEBUGGER_IO 0x406D1388

bool DecoderWindows::decode(ArchEvent *ae, std::vector<Event::ptr> &events)
{
	static Address ntdll_ignore_breakpoint_address = 0;

	assert(ae);
	ArchEventWindows* winEvt = static_cast<ArchEventWindows*>(ae);
	assert(winEvt);
	Event::ptr newEvt = Event::ptr();
	int_thread *thread = ProcPool()->findThread((Dyninst::LWP)(winEvt->evt.dwThreadId));
	int_process* proc = ProcPool()->findProcByPid(winEvt->evt.dwProcessId);
	if(!proc) { return false; }
	windows_process* windows_proc = dynamic_cast<windows_process*>(proc);
	if(!windows_proc)
	{
		perr_printf("DecoderWindows::decode() called on nonexistent/deleted process %d/%d\n", 
			winEvt->evt.dwProcessId, winEvt->evt.dwThreadId);
		return false;
	}
	//windows_proc->clearPendingDebugBreak();

	DEBUG_EVENT e = winEvt->evt;

	switch(e.dwDebugEventCode)
	{
	case CREATE_PROCESS_DEBUG_EVENT:
	case CREATE_THREAD_DEBUG_EVENT:
		pthrd_printf("decodeProcess/decodeThreadEvent\n");
		return decodeCreateThread(e, newEvt, proc, events);
	case EXCEPTION_DEBUG_EVENT:
		pthrd_printf("decodeException\n");
		switch(e.u.Exception.ExceptionRecord.ExceptionCode)
		{
		case EXCEPTION_SINGLE_STEP:
			pthrd_printf("Decoding singleStep event on PID %d, TID %d\n", e.dwProcessId, e.dwThreadId);
			newEvt = decodeSingleStepEvent(e, proc, thread);
			break;
			//fprintf(stderr, "Decoded Single-step event at 0x%lx, PID: %d, TID: %d\n", e.u.Exception.ExceptionRecord.ExceptionAddress, e.dwProcessId, e.dwThreadId);
		case EXCEPTION_BREAKPOINT:
			// Case 1: breakpoint is real breakpoint
			pthrd_printf("Caught breakpoint for pid %d, tid %d, PC 0x%lx\n", e.dwProcessId, e.dwThreadId, e.u.Exception.ExceptionRecord.ExceptionAddress);
			newEvt = decodeBreakpointEvent(e, proc, thread);
			if(newEvt)
			{
				pthrd_printf("Decoded Breakpoint event at 0x%lx, PID: %d, TID: %d\n", e.u.Exception.ExceptionRecord.ExceptionAddress, e.dwProcessId, e.dwThreadId);
				//fprintf(stderr, "Decoded Breakpoint event at 0x%lx, PID: %d, TID: %d\n", e.u.Exception.ExceptionRecord.ExceptionAddress, e.dwProcessId, e.dwThreadId);
				break;
			}
			else
			{
				if(proc->getState() != int_process::neonatal &&
					proc->getState() != int_process::neonatal_intermediate)
				{
					bool didSomething = false;

				   if (windows_proc->pendingDebugBreak())
					{
						pthrd_printf("Decoded Stop event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
						//newEvt = Event::ptr(new EventStop());
						windows_proc->setStopThread(e.dwThreadId);
						return true;
					}
					else
					{
						pthrd_printf("Decoded unhandled exception (breakpoint) event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
						// Case 3: breakpoint that's not from us, while running. Pass on exception.
						GeneratorWindows* winGen = static_cast<GeneratorWindows*>(GeneratorWindows::getDefaultGenerator());
						newEvt = EventSignal::ptr(new EventSignal(e.u.Exception.ExceptionRecord.ExceptionCode));
					}
				}
				else
				{
					// Case 4: breakpoint in ntdll.dll due to startup. This should be skipped and we should bootstrap.
					pthrd_printf("Breakpoint due to startup, ignoring\n");
					newEvt = Event::ptr(new EventBootstrap());
					ntdll_ignore_breakpoint_address = (Address) e.u.Exception.ExceptionRecord.ExceptionAddress;
					if(!thread) {
						pthrd_printf("DEBUG: Missing thread on startup breakpoint\n");
					}
				}
			}
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			{
				pthrd_printf("SIGILL in mutatee\n");
				GeneratorWindows* winGen = static_cast<GeneratorWindows*>(GeneratorWindows::getDefaultGenerator());
				winGen->markUnhandledException(e.dwProcessId);
				newEvt = EventSignal::ptr(new EventSignal(e.u.Exception.ExceptionRecord.ExceptionCode));
				assert(0);
			}
			break;
		case EXCEPTION_ACCESS_VIOLATION:
			{
                // check if this is first chance or second chance:
                if (e.u.Exception.dwFirstChance != 0) {
                    int sig = e.u.Exception.ExceptionRecord.ExceptionCode;
                    int cause = e.u.Exception.ExceptionRecord.ExceptionInformation[0];
                    Address addr = e.u.Exception.ExceptionRecord.ExceptionInformation[1];
                    EventSignal* evSig = NULL;
                    switch (cause) {
                    case 0: evSig = new EventSignal(sig, addr, EventSignal::ReadViolation, true); break;
                    case 1: evSig = new EventSignal(sig, addr, EventSignal::WriteViolation, true); break;
                    case 8: evSig = new EventSignal(sig, addr, EventSignal::ExecuteViolation, true); break;
                    default: evSig = new EventSignal(sig, addr, EventSignal::Unknown, true); break;
                    }
                    newEvt = EventSignal::ptr(evSig);
                } else {
                    pthrd_printf("segfault in mutatee, thread %d/%d\n", e.dwProcessId, e.dwThreadId);
				    unsigned problemArea = (unsigned int)(e.u.Exception.ExceptionRecord.ExceptionAddress);
				    cerr << "SEGFAULT @ " << hex << problemArea << dec << endl;
				    dumpSurroundingMemory(problemArea, proc);
				    GeneratorWindows* winGen = static_cast<GeneratorWindows*>(GeneratorWindows::getDefaultGenerator());
				    winGen->markUnhandledException(e.dwProcessId);
				    newEvt = EventSignal::ptr(new EventSignal(e.u.Exception.ExceptionRecord.ExceptionCode));
				    cerr << "Signal is " << e.u.Exception.ExceptionRecord.ExceptionCode << endl;
                }
			}
			break;
			// Thread naming exception. Ignore.
		case EXCEPTION_DEBUGGER_IO: {
			pthrd_printf("Debugger I/O exception: %lx\n", e.u.Exception.ExceptionRecord.ExceptionInformation[0]);
			newEvt = EventNop::ptr(new EventNop());
			newEvt->setSyncType(Event::async);
			break;
		}
		default:
			{
				pthrd_printf("Decoded unhandled exception event, PID: %d, TID: %d, Exception code = 0x%lx, Exception addr = 0x%lx\n", e.dwProcessId, e.dwThreadId,
					e.u.Exception.ExceptionRecord.ExceptionCode, e.u.Exception.ExceptionRecord.ExceptionAddress);
				GeneratorWindows* winGen = static_cast<GeneratorWindows*>(GeneratorWindows::getDefaultGenerator());
				winGen->markUnhandledException(e.dwProcessId);
				newEvt = EventSignal::ptr(new EventSignal(e.u.Exception.ExceptionRecord.ExceptionCode));
			}
			break;
		}
		break;
	case EXIT_PROCESS_DEBUG_EVENT:
		{
			pthrd_printf("Decoded ProcessExit event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
			if(proc->wasForcedTerminated()) {
				newEvt = EventForceTerminate::ptr(new EventForceTerminate(e.u.ExitProcess.dwExitCode));
			}
			else {
				newEvt = EventExit::ptr(new EventExit(EventType::Pre, e.u.ExitProcess.dwExitCode));
			}
//			GeneratorWindows* winGen = static_cast<GeneratorWindows*>(GeneratorWindows::getDefaultGenerator());
//			winGen->removeProcess(proc);
			newEvt->setSyncType(Event::sync_process);
			newEvt->setProcess(proc->proc());
			// Since we're doing thread exit/proc exit, and this means the thread will go away first,
			// we don't set the thread here
			if(thread)
				newEvt->setThread(thread->thread());
			// We do this here because the generator thread will exit before updateSyncState otherwise
//			int_threadPool::iterator i = proc->threadPool()->begin();
//			for (; i != proc->threadPool()->end(); i++) {
//				(*i)->getGeneratorState().setState(int_thread::exited);
//		 		(*i)->setExitingInGenerator(true);
//			}
			events.push_back(newEvt);

			return true;
		}
		break;
	case EXIT_THREAD_DEBUG_EVENT:
		pthrd_printf("Decoded ThreadExit event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
		if(thread) {
			thread->getGeneratorState().setState(int_thread::exited);
			thread->setExitingInGenerator(true);
			if (e.dwThreadId == windows_proc->getStopThread()) {
				newEvt = EventWinStopThreadDestroy::ptr(new EventWinStopThreadDestroy(EventType::Pre));
				windows_proc->clearStopThread();
			}
			else {
				newEvt = EventLWPDestroy::ptr(new EventLWPDestroy(EventType::Pre));
			}
		} else {
			// If the thread is NULL, we can't give the user an event with a valid thread object anymore.
			// So fail the decode.
			return false;
		}
		break;
	case LOAD_DLL_DEBUG_EVENT:
		pthrd_printf("Decoded LoadDLL event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
		newEvt = decodeLibraryEvent(e, proc);
		break;
	case UNLOAD_DLL_DEBUG_EVENT:
		pthrd_printf("Decoded UnloadDLL event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
		newEvt = decodeLibraryEvent(e, proc);
		break;
	case OUTPUT_DEBUG_STRING_EVENT:
		{
			TCHAR buf[1024];
			SIZE_T bytes_read = 0;
			windows_process* winProc = dynamic_cast<windows_process*>(proc);
			BOOL result = ::ReadProcessMemory(winProc->plat_getHandle(), e.u.DebugString.lpDebugStringData, buf, 
				e.u.DebugString.nDebugStringLength, &bytes_read);
			if(result) {
				pthrd_printf("Decoded DebugString event, string: %s\n", buf);
			} else {
				pthrd_printf("Decoded DebugString event, but string was not readable!\n");
			}

			break;
		}
	case RIP_EVENT:
		pthrd_printf("Decoded RIP event, PID: %d, TID: %d, error = 0x%lx\n", e.dwProcessId, e.dwThreadId,
			e.u.RipInfo.dwError);
		newEvt = EventCrash::ptr(new EventCrash(e.u.RipInfo.dwError));
		assert(0);
		break;
	default:
		assert(!"invalid event type");
		return false;
	}
	if(newEvt)
	{
		assert(proc);
		if(newEvt->getSyncType() == Event::unset)
			newEvt->setSyncType(Event::sync_process);
		if(thread) {
			newEvt->setThread(thread->thread());
		}
		newEvt->setProcess(proc->proc());
		events.push_back(newEvt);
	}
	return true;
}

bool DecoderWindows::checkForFullString( DEBUG_EVENT &details, int chunkSize, wchar_t* libName, bool gotString, char* asciiLibName )
{
	if(details.u.LoadDll.fUnicode)
	{
		unsigned int lastCharIndex = chunkSize / sizeof(wchar_t);
		libName[lastCharIndex] = L'\0';
		wchar_t* first_null = wcschr(libName, L'\0');
		gotString = (first_null != &(libName[lastCharIndex]));
	}
	else
	{
		asciiLibName[chunkSize] = '0';
		char* first_null = strchr(asciiLibName, '\0');
		gotString = (first_null != &(asciiLibName[chunkSize]));

	}
	return gotString;
}

bool DecoderWindows::decodeCreateThread( DEBUG_EVENT &e, Event::ptr &newEvt, int_process* &proc, std::vector<Event::ptr> &events )
{
	pthrd_printf("Decoded CreateThread event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
	proc = ProcPool()->findProcByPid(e.dwProcessId);
	assert(proc);

	// FIXME once things actually work
	windows_process* wproc = dynamic_cast<windows_process*>(proc);
	assert(wproc);

	if(e.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
		newEvt = WinEventThreadInfo::ptr(new WinEventThreadInfo((Dyninst::LWP)(e.dwThreadId), e.u.CreateProcessInfo.hThread,
			e.u.CreateProcessInfo.lpStartAddress, e.u.CreateProcessInfo.lpThreadLocalBase));
		newEvt->setThread(proc->threadPool()->initialThread()->thread());
		wproc->plat_setHandles(e.u.CreateProcessInfo.hProcess, e.u.CreateProcessInfo.hFile, (Dyninst::Address)e.u.CreateProcessInfo.lpBaseOfImage);
	} else {
		newEvt = WinEventNewThread::ptr(new WinEventNewThread((Dyninst::LWP)(e.dwThreadId), e.u.CreateThread.hThread,
			e.u.CreateThread.lpStartAddress, e.u.CreateThread.lpThreadLocalBase));
	}


	newEvt->setProcess(proc->proc());
	newEvt->setSyncType(Event::sync_process);
	events.push_back(newEvt);
	return true;
}


std::string DecoderWindows::readLibNameFromProc(Address libnameaddr, DEBUG_EVENT details, int_process *p) {
	char asciiLibName[(MAX_PATH+1)*sizeof(wchar_t)];
	wchar_t* libName = (wchar_t*)(asciiLibName);	
	int lowReadSize = 1;
	int highReadSize = 128;
	bool doneReading = false;
	bool gotString = false;
	int chunkSize = highReadSize;
	int changeSize = highReadSize / 2;

	while(!doneReading)
	{
		pthrd_printf("Trying read for libname at 0x%lx, size %d\n", libnameaddr, chunkSize);
		// try to read with the current byte count
		if(p->plat_readMem(NULL, libName, (Dyninst::Address)libnameaddr, chunkSize))
		{
			// the read succeeded -
			// did we get the full string?
			gotString = checkForFullString(details, chunkSize, libName, gotString, asciiLibName);
			if(gotString)
			{
				doneReading = true;
			}
			else
			{
				// we didn't get the full string,
				// need to try a larger read
				if(chunkSize == highReadSize)
				{
					// we were are the high end of the current range -
					// move to the next range
					lowReadSize = highReadSize + 1;
					highReadSize = highReadSize + 128;
					changeSize = 128; // we will half this before we read again
					if(lowReadSize > (MAX_PATH * sizeof(wchar_t)))
					{
						// we've tried every range but still failed
						doneReading = true;
					}
					else
					{
						chunkSize = highReadSize;
					}
				}
				else
				{
					// we were within the current range -
					// try something higher but still within the range
					chunkSize = chunkSize + changeSize;
				}
			}
		}
		else
		{
			// the read failed -
			// we need to try a smaller read
			if(chunkSize > lowReadSize)
			{
				unsigned int nextReadSize = chunkSize - changeSize;
				if(nextReadSize == chunkSize)
				{
					// we can't subdivide any further
					doneReading = true;
				}
				else
				{
					chunkSize = nextReadSize;
				}
			}
			else
			{
				// there are no smaller reads to try in this range,
				// and by induction, in any range
				doneReading = true;
			}
		}
		// update the amount that we use to change the read request
		changeSize /= 2;

	}
	if(details.u.LoadDll.fUnicode)
	{
		pthrd_printf("Converting unicode into single-byte characters\n");
		// the DLL path is a Unicode string
		// we have to convert it to single-byte characters
		char* tmp = new char[MAX_PATH];
		::WideCharToMultiByte(CP_ACP, 0, libName, -1, tmp, MAX_PATH, NULL, NULL);
		// swap buffers so that asciiLibName points to the single-byte string
		// when we're out of this code block
		return std::string(tmp);
	}
	else
	{
		// the DLL path can be cast correctly
		return std::string(asciiLibName);
	}
} 


std::string DecoderWindows::HACKreadFromFile(DEBUG_EVENT details, int_process *p) {
	std::string ret;
	// Copied from Dyninst, 20OCT11 - Bernat 
	// And comment preserved for posterity. Hello, posterity!

	//I'm embarassed to be writing this.  We didn't get a name for the image, 
    // but we did get a file handle.  According to MSDN, the best way to turn
    // a file handle into a file name is to map the file into the address space
    // (using the handle), then ask the OS what file we have mapped at that location.
    // I'm sad now.
    
    void *pmap = NULL;
    HANDLE fmap = CreateFileMapping(details.u.LoadDll.hFile, NULL, 
                                    PAGE_READONLY, 0, 1, NULL);
    if (fmap) {
        pmap = MapViewOfFile(fmap, FILE_MAP_READ, 0, 0, 1);
        if (pmap) {   
            char filename[MAX_PATH+1];
			BOOL result = ::GetMappedFileName(GetCurrentProcess(), pmap, filename, MAX_PATH);
            if (result)
                ret = std::string(filename);
            UnmapViewOfFile(pmap);
        }
        CloseHandle(fmap);
    }
	return ret;
}
