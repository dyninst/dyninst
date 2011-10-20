#include "DecoderWindows.h"
#include "windows_process.h"
#include "windows_handler.h"
#include "windows_thread.h"
#include "ProcPool.h"
#include <iostream>
#include "external/boost/scoped_ptr.hpp"
#include "irpc.h"

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
	fprintf(stderr, "segfault in mutatee at %p\n", problemArea);
	for(unsigned i = problemArea-16; i < problemArea+16; i++)
	{
		unsigned char tmp = 0;

		if(proc->plat_readMem(NULL, &tmp, i, 1)) {
			fprintf(stderr, "%p: %x\n", i, tmp);
		}
		else {
			fprintf(stderr, "failed to read from %p\n", i);
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
		char* asciiLibName = new char[(MAX_PATH+1)*sizeof(wchar_t)];
		wchar_t* libName = (wchar_t*)(asciiLibName);
		void* libnameaddr;
		std::string result = "";
		if(details.u.LoadDll.lpImageName && // NULL lpImageName = done
			p->plat_readMem(NULL, &libnameaddr, (Dyninst::Address)(details.u.LoadDll.lpImageName), 4) && 
			libnameaddr) // NULL libnameaddr = done
		{
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
				delete[] asciiLibName;
				asciiLibName = tmp;
			}
			else
			{
				// the DLL path can be cast correctly
				asciiLibName = (char*)libName;
			}
			result = asciiLibName;

		}
		delete[] asciiLibName;

		
		int_library* lib = new int_library(result,
			(Dyninst::Address)(details.u.LoadDll.lpBaseOfDll),
			(Dyninst::Address)(details.u.LoadDll.lpBaseOfDll));
		addedLibs.insert(lib->getUpPtr());
		// FIXME
		//std::cerr << "Theoretically adding " << std::hex << lib->getUpPtr() << std::dec << std::endl;
		//std::cerr << proc->memory()->libs.size() << " current libs" << std::endl; 
		//proc->memory()->libs.insert(lib);		
		//std::cerr << proc->memory()->libs.size() << " new libs" << std::endl; 
		pthrd_printf("Load DLL event, loading %s (at 0x%lx)\n",
			lib->getName().c_str(), lib->getAddr());

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

	installed_breakpoint *ibp = proc->getBreakpoint(adjusted_addr);

	installed_breakpoint *clearingbp = thread->isClearingBreakpoint();
	if(thread->singleStep() && clearingbp) {
		pthrd_printf("Decoded event to breakpoint cleanup\n");
		evt = Event::ptr(new EventBreakpointClear(clearingbp));
		return evt;
	}

	// Need to distinguish case where the thread is single-stepped to a
	// breakpoint and when a single step hits a breakpoint.
	//
	// If no forward progress was made due to a single step, then a
	// breakpoint was hit
	Dyninst::MachRegisterVal pre_ss_pc = thread->getPreSingleStepPC();
	if (thread->singleStep() && ( (pre_ss_pc != 0 && pre_ss_pc != adjusted_addr) || !ibp)) {
	   pthrd_printf("Decoded event to single step on %d/%d\n",
			   proc->getPid(), thread->getLWP());
	   evt = Event::ptr(new EventSingleStep());
	   return evt;
	}

	if (ibp && ibp != clearingbp) {
	   pthrd_printf("Decoded breakpoint on %d/%d at %lx\n", proc->getPid(), 
					thread->getLWP(), adjusted_addr);
	   EventBreakpoint::ptr event_bp = EventBreakpoint::ptr(new EventBreakpoint(adjusted_addr, ibp));
	   evt = event_bp;
	   evt->setThread(thread->thread());
	   ibp->addClearingThread(thread);
	}
	return evt;

}

#define EXCEPTION_DEBUGGER_IO 0x406D1388

bool DecoderWindows::decode(ArchEvent *ae, std::vector<Event::ptr> &events)
{
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
	windows_proc->clearPendingDebugBreak();

	DEBUG_EVENT e = winEvt->evt;

	switch(e.dwDebugEventCode)
	{
	case CREATE_PROCESS_DEBUG_EVENT:
		{
			pthrd_printf("Decoded CreateProcess event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
			newEvt = EventPreBootstrap::ptr(new EventPreBootstrap());
			newEvt->setSyncType(Event::sync_process);
			windows_proc->plat_setHandle(e.u.CreateProcessInfo.hProcess);
			newEvt->setProcess(proc->proc());
			events.push_back(newEvt);
			Event::ptr threadEvent;
			bool ok = decodeCreateThread(e, threadEvent, proc, events);
			if(threadEvent)
				newEvt->addSubservientEvent(threadEvent);
			return ok;
		}
	case CREATE_THREAD_DEBUG_EVENT:
		{
			return decodeCreateThread(e, newEvt, proc, events);
		}
	case EXCEPTION_DEBUG_EVENT:
		switch(e.u.Exception.ExceptionRecord.ExceptionCode)
		{
		case EXCEPTION_SINGLE_STEP:
			//fprintf(stderr, "Decoded Single-step event at 0x%lx, PID: %d, TID: %d\n", e.u.Exception.ExceptionRecord.ExceptionAddress, e.dwProcessId, e.dwThreadId);
		case EXCEPTION_BREAKPOINT:
			// Case 1: breakpoint is real breakpoint
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


					// Case 2: breakpoint from debugBreak() being used for a stop on one/all threads.
					pthrd_printf("Possible stop, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
				   for (int_threadPool::iterator i = proc->threadPool()->begin(); i != proc->threadPool()->end(); ++i)
				   {
					  int_thread *thr = *i;
					  if (thr->hasPendingStop()) {
						  pthrd_printf("Suspended %d/%d\n", proc->getPid(), thr->getLWP());
						   thr->plat_suspend();
					  }
					  didSomething = true;
				   }   

					if(didSomething)
					{
						pthrd_printf("Decoded Stop event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
						newEvt = Event::ptr(new EventStop());
					}
					else if(proc->hasPendingDetach())
					{
						pthrd_printf("Decoded Detach event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
						ProcPool()->condvar()->lock();
						proc->setState(int_process::exited);
						ProcPool()->rmProcess(proc);
						GeneratorWindows* winGen = static_cast<GeneratorWindows*>(GeneratorWindows::getDefaultGenerator());
						winGen->removeProcess(proc);
						delete proc;
						ProcPool()->condvar()->signal();
						ProcPool()->condvar()->unlock();
						events.push_back(newEvt);
						return true;
					}
					else
					{
						pthrd_printf("Decoded unhandled exception (breakpoint) event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
						// Case 3: breakpoint that's not from us, while running. Pass on exception.
						GeneratorWindows* winGen = static_cast<GeneratorWindows*>(GeneratorWindows::getDefaultGenerator());
						winGen->markUnhandledException(e.dwProcessId);
						newEvt = EventSignal::ptr(new EventSignal(e.u.Exception.ExceptionRecord.ExceptionCode));
					}
				}
				else
				{
					// Case 4: breakpoint in ntdll.dll due to startup. This should be skipped and we should bootstrap.
					pthrd_printf("Breakpoint due to startup, ignoring\n");
					proc->setForceGeneratorBlock(false);
					newEvt = Event::ptr(new EventBootstrap());
				}
			}
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			{
				pthrd_printf("SIGILL in mutatee\n");
				GeneratorWindows* winGen = static_cast<GeneratorWindows*>(GeneratorWindows::getDefaultGenerator());
				winGen->markUnhandledException(e.dwProcessId);
				newEvt = EventSignal::ptr(new EventSignal(e.u.Exception.ExceptionRecord.ExceptionCode));
			}
			break;
		case EXCEPTION_ACCESS_VIOLATION:
			{
				pthrd_printf("segfault in mutatee\n");
				unsigned problemArea = (unsigned int)(e.u.Exception.ExceptionRecord.ExceptionAddress);
				dumpSurroundingMemory(problemArea, proc);
				GeneratorWindows* winGen = static_cast<GeneratorWindows*>(GeneratorWindows::getDefaultGenerator());
				winGen->markUnhandledException(e.dwProcessId);
				newEvt = EventSignal::ptr(new EventSignal(e.u.Exception.ExceptionRecord.ExceptionCode));
			}
			break;
			// Thread naming exception. Ignore.
		case EXCEPTION_DEBUGGER_IO:
			pthrd_printf("Debugger I/O exception: %lx\n", e.u.Exception.ExceptionRecord.ExceptionInformation[0]);
			break;
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
				newEvt = EventExit::ptr(new EventExit(EventType::Post, e.u.ExitProcess.dwExitCode));
			}
			GeneratorWindows* winGen = static_cast<GeneratorWindows*>(GeneratorWindows::getDefaultGenerator());
			winGen->removeProcess(proc);
			newEvt->setSyncType(Event::sync_process);
			newEvt->setProcess(proc->proc());
			if(thread)
				newEvt->setThread(thread->thread());
			// We do this here because the generator thread will exit before updateSyncState otherwise
			  int_threadPool::iterator i = proc->threadPool()->begin();
			  for (; i != proc->threadPool()->end(); ++i) {
				(*i)->setGeneratorState(int_thread::exited);
				(*i)->setExitingInGenerator(true);
			  }
			  events.push_back(newEvt);

			return true;
		}
		break;
	case EXIT_THREAD_DEBUG_EVENT:
		pthrd_printf("Decoded ThreadExit event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
		if(thread) {
			thread->setGeneratorState(int_thread::exited);
			thread->setExitingInGenerator(true);
			newEvt = EventLWPDestroy::ptr(new EventLWPDestroy(EventType::Post));
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
			unsigned long bytes_read = 0;
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
		break;
	default:
		assert(!"invalid event type");
		return false;
	}
	if(newEvt)
	{
		//assert(thread || (e.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT));
		assert(proc);
		if(newEvt->getSyncType() == Event::unset)
			newEvt->setSyncType(Event::sync_process);
		if(thread)
			newEvt->setThread(thread->thread());
		newEvt->setProcess(proc->proc());
		events.push_back(newEvt);
	}
	return true;
}

bool DecoderWindows::checkForFullString( DEBUG_EVENT &details, int chunkSize, wchar_t* libName, bool gotString, char* & asciiLibName )
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

	if(e.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
		newEvt = WinEventNewThread::ptr(new WinEventNewThread((Dyninst::LWP)(e.dwThreadId), e.u.CreateProcessInfo.hThread,
			e.u.CreateProcessInfo.lpStartAddress, e.u.CreateProcessInfo.lpThreadLocalBase));
	} else {
		newEvt = WinEventNewThread::ptr(new WinEventNewThread((Dyninst::LWP)(e.dwThreadId), e.u.CreateThread.hThread,
			e.u.CreateThread.lpStartAddress, e.u.CreateThread.lpThreadLocalBase));
	}

	ProcPool()->condvar()->lock();
	proc = ProcPool()->findProcByPid(e.dwProcessId);
	assert(proc);
	ProcPool()->condvar()->unlock();
	newEvt->setProcess(proc->proc());
	newEvt->setSyncType(Event::sync_process);
	events.push_back(newEvt);
	return true;
}
