#include "DecoderWindows.h"
#include "windows.h"
#include "ProcPool.h"
#include <iostream>
#include "external/boost/scoped_ptr.hpp"

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
		std::string result;
		if(p->plat_readMem(NULL, libnameaddr, (Dyninst::Address)(details.u.LoadDll.lpImageName), 4))
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
				if(p->plat_readMem(NULL, libName, (Dyninst::Address)libnameaddr, chunkSize))
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
					if(gotString)
					{
						doneReading = true;
					}
					else
					{
						if(chunkSize == highReadSize)
						{
							lowReadSize = highReadSize + 1;
							highReadSize = highReadSize + 128;
							changeSize = 128;
							if(lowReadSize > (MAX_PATH * sizeof(wchar_t)))
							{
								doneReading = true;
							}
							else
							{
								chunkSize = highReadSize;
							}
						}
						else
						{
							chunkSize = chunkSize + changeSize;
						}
					}
				}
				else
				{
					if(chunkSize > lowReadSize)
					{
						unsigned int nextReadSize = chunkSize - changeSize;
						if(nextReadSize == chunkSize)
						{
							doneReading = true;
						}
						else
						{
							chunkSize = nextReadSize;
						}
					}
					else
					{
						doneReading = true;
					}
				}
				changeSize /= 2;
				
			}
			if(details.u.LoadDll.fUnicode)
			{
				char* tmp = new char[MAX_PATH];
				::WideCharToMultiByte(CP_ACP, 0, libName, -1, tmp, sizeof(asciiLibName), NULL, NULL);
				delete[] asciiLibName;
				asciiLibName = tmp;
			}
			else
			{
				asciiLibName = (char*)libName;
			}
			result = asciiLibName;

		}
		delete[] asciiLibName;

		
		int_library* lib = new int_library(result,
			(Dyninst::Address)(details.u.LoadDll.lpBaseOfDll),
			(Dyninst::Address)(details.u.LoadDll.lpBaseOfDll));
		addedLibs.insert(lib->getUpPtr());
		proc->memory()->libs.insert(lib);		
		pthrd_printf("Load DLL event, loading %s (at 0x%lx)\n",
			lib->getName(), lib->getAddr());

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
					(*foundLib)->getName(), (*foundLib)->getAddr());
				removedLibs.insert((*foundLib)->getUpPtr());
				proc->memory()->libs.erase(foundLib);
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
	Dyninst::Address adjusted_addr = (Dyninst::Address)(e.u.Exception.ExceptionRecord.ExceptionAddress);
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
	if (thread->singleStep() && ( /*(pre_ss_pc != 0 && pre_ss_pc != addr) ||*/ !ibp)) {
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

	   if (thread->isEmulatingSingleStep() && thread->isEmulatedSingleStep(ibp)) {
		   pthrd_printf("Breakpoint is emulated single step\n");
		   installed_breakpoint *ibp = thread->isClearingBreakpoint();
		   if( ibp ) {
			   pthrd_printf("Decoded emulated single step to breakpoint cleanup\n");
			   EventBreakpointClear::ptr bc_event = EventBreakpointClear::ptr(new EventBreakpointClear(ibp));
			   bc_event->setThread(thread->thread());
			   bc_event->setProcess(proc->proc());
			   evt->addSubservientEvent(bc_event);
		   }else{
			   pthrd_printf("Decoded emulated single step to normal single step\n");
			   EventSingleStep::ptr ss_event = EventSingleStep::ptr(new EventSingleStep());
			   ss_event->setThread(thread->thread());
			   ss_event->setProcess(proc->proc());
			   evt->addSubservientEvent(ss_event);
		   }
	   }
	}
	return evt;

}

#define EXCEPTION_DEBUGGER_IO 0x406D1388

bool DecoderWindows::decode(ArchEvent *ae, std::vector<Event::ptr> &events)
{
	assert(ae);
//	boost::scoped_ptr<ArchEvent> ae_destructor(ae);
	ArchEventWindows* winEvt = static_cast<ArchEventWindows*>(ae);
	assert(winEvt);
	Event::ptr newEvt = Event::ptr();
	int_thread *thread = ProcPool()->findThread((Dyninst::LWP)(winEvt->evt.dwThreadId));
	int_process* proc = NULL;
	if (thread) {
      proc = thread->llproc();
	  windows_process* windows_proc = dynamic_cast<windows_process*>(proc);
	  windows_proc->clearPendingDebugBreak();
    }
	DEBUG_EVENT e = winEvt->evt;
	switch(e.dwDebugEventCode)
	{
	case CREATE_PROCESS_DEBUG_EVENT:
		pthrd_printf("Decoded CreateProcess event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
		newEvt = EventPreBootstrap::ptr(new EventPreBootstrap());
		newEvt->setSyncType(Event::sync_process);
		break;
	case CREATE_THREAD_DEBUG_EVENT:
		{
			pthrd_printf("Decoded CreateThread event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
			newEvt = WinEventNewThread::ptr(new WinEventNewThread((Dyninst::LWP)(e.dwThreadId), e.u.CreateThread.hThread,
				e.u.CreateThread.lpStartAddress, e.u.CreateThread.lpThreadLocalBase));
		    ProcPool()->condvar()->lock();
			proc = ProcPool()->findProcByPid(e.dwProcessId);
			assert(proc);
		    ProcPool()->condvar()->unlock();
			newEvt->setProcess(proc->proc());
			newEvt->setSyncType(Event::sync_process);
			events.push_back(newEvt);
			return true;
		}
	case EXCEPTION_DEBUG_EVENT:
		switch(e.u.Exception.ExceptionRecord.ExceptionCode)
		{
		case EXCEPTION_BREAKPOINT:
			// Case 1: breakpoint is real breakpoint
			newEvt = decodeBreakpointEvent(e, proc, thread);
			if(newEvt)
			{
				pthrd_printf("Decoded Breakpoint event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
			}
			else
			{
				if(proc->getState() != int_process::neonatal &&
					proc->getState() != int_process::neonatal_intermediate)
				{
					bool didSomething = false;


					// Case 2: breakpoint from debugBreak() being used for a stop on one/all threads.
				   int_thread *result = NULL;
					pthrd_printf("Possible stop, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
				   for (int_threadPool::iterator i = proc->threadPool()->begin(); i != proc->threadPool()->end(); i++)
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
			newEvt = EventExit::ptr(new EventExit(EventType::Post, e.u.ExitProcess.dwExitCode));
			GeneratorWindows* winGen = static_cast<GeneratorWindows*>(GeneratorWindows::getDefaultGenerator());
			winGen->removeProcess(proc);
			newEvt->setSyncType(Event::sync_process);
			newEvt->setProcess(proc->proc());
			newEvt->setThread(thread->thread());
			// We do this here because the generator thread will exit before updateSyncState otherwise
			thread->setExitingInGenerator(true);
			  int_threadPool::iterator i = proc->threadPool()->begin();
			  for (; i != proc->threadPool()->end(); i++) {
				 (*i)->setGeneratorState(int_thread::exited);
			  }

			return true;
		}
		break;
	case EXIT_THREAD_DEBUG_EVENT:
		pthrd_printf("Decoded ThreadExit event, PID: %d, TID: %d\n", e.dwProcessId, e.dwThreadId);
		thread->setGeneratorState(int_thread::exited);
		newEvt = EventUserThreadDestroy::ptr(new EventUserThreadDestroy(EventType::Post));
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
			bool result = ::ReadProcessMemory(winProc->plat_getHandle(), e.u.DebugString.lpDebugStringData, buf, 
				e.u.DebugString.nDebugStringLength, &bytes_read);
			pthrd_printf("Decoded DebugString event, string: %s\n", buf);
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
		assert(thread);
		assert(proc);
		if(newEvt->getSyncType() == Event::unset)
			newEvt->setSyncType(Event::sync_process);
		newEvt->setThread(thread->thread());
		newEvt->setProcess(proc->proc());
		events.push_back(newEvt);
	}
	return true;
}
