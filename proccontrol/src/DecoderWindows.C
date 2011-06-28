#include "DecoderWindows.h"
#include "windows.h"
#include "ProcPool.h"
#include <iostream>

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
		wchar_t libName[512];
		char asciiLibName[512];
		char* string_addr = NULL;
		void* libnameaddr;
		if(p->plat_readMem(NULL, libnameaddr, (Dyninst::Address)(details.u.LoadDll.lpImageName), p->getAddressWidth()))
		{	
			if(p->plat_readMem(NULL, libName, (Dyninst::Address)libnameaddr, sizeof(libName)))
			{
				::WideCharToMultiByte(CP_ACP, 0, libName, -1, asciiLibName, sizeof(asciiLibName), NULL, NULL);
			}
		}

		
		int_library* lib = new int_library(asciiLibName,
			(Dyninst::Address)(details.u.LoadDll.lpBaseOfDll),
			(Dyninst::Address)(details.u.LoadDll.lpBaseOfDll));
		addedLibs.insert(lib->getUpPtr());
		proc->memory()->libs.insert(lib);		
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

bool DecoderWindows::decode(ArchEvent *ae, std::vector<Event::ptr> &events)
{
	assert(ae);
	ArchEventWindows* winEvt = static_cast<ArchEventWindows*>(ae);
	assert(winEvt);
	Event::ptr newEvt = Event::ptr();
	int_thread *thread = ProcPool()->findThread((Dyninst::LWP)(winEvt->evt.dwThreadId));
	int_process* proc = NULL;
	if (thread) {
      proc = thread->llproc();
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
			if(!newEvt)
			{
				if(proc->getState() != int_process::neonatal &&
					proc->getState() != int_process::neonatal_intermediate)
				{
					ProcPool()->condvar()->lock();
					bool didSomething = false;


					// Case 2: breakpoint from debugBreak() being used for a stop on one/all threads.
				   int_thread *result = NULL;
				   for (int_threadPool::iterator i = proc->threadPool()->begin(); i != proc->threadPool()->end(); i++)
				   {
					  int_thread *thr = *i;
					  thr->setGeneratorState(int_thread::stopped);
					  thr->setHandlerState(int_thread::stopped);
					  if (thr->hasPendingStop()) {
						  didSomething = true;
						   thr->setPendingStop(false);

						   thr->setInternalState(int_thread::stopped);
						   if (thr->hasPendingUserStop()) {
							  thr->setUserState(int_thread::stopped);
							  thr->setPendingUserStop(false);
						   }
						   thr->plat_suspend();
					  }
				   }   
   					ProcPool()->condvar()->signal();
					ProcPool()->condvar()->unlock();

					if(didSomething)
					{
						events.push_back(Event::ptr());
						return true;
					}
					else if(proc->hasPendingDetach())
					{
						ProcPool()->condvar()->lock();
						proc->setState(int_process::exited);
						ProcPool()->rmProcess(proc);
						delete proc;
						ProcPool()->condvar()->signal();
						ProcPool()->condvar()->unlock();
						events.push_back(newEvt);
						return true;
					}
					else
					{
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
		default:
			{
				GeneratorWindows* winGen = static_cast<GeneratorWindows*>(GeneratorWindows::getDefaultGenerator());
				winGen->markUnhandledException(e.dwProcessId);
				newEvt = EventSignal::ptr(new EventSignal(e.u.Exception.ExceptionRecord.ExceptionCode));
			}
			break;
		}
		break;
	case EXIT_PROCESS_DEBUG_EVENT:
		newEvt = EventExit::ptr(new EventExit(EventType::Post, e.u.ExitProcess.dwExitCode));
		break;
	case EXIT_THREAD_DEBUG_EVENT:
		newEvt = EventUserThreadDestroy::ptr(new EventUserThreadDestroy(EventType::Post));
		break;
	case LOAD_DLL_DEBUG_EVENT:
	case UNLOAD_DLL_DEBUG_EVENT:
		newEvt = decodeLibraryEvent(e, proc);
		break;
	case OUTPUT_DEBUG_STRING_EVENT:
		{
			std::cerr << "Debug string event" << std::endl;
			TCHAR buf[1024];
			unsigned long bytes_read = 0;
			windows_process* winProc = dynamic_cast<windows_process*>(proc);
			bool result = ::ReadProcessMemory(winProc->plat_getHandle(), e.u.DebugString.lpDebugStringData, buf, 
				e.u.DebugString.nDebugStringLength, &bytes_read);
			std::cerr << buf << std::endl;
			break;
		}
	case RIP_EVENT:
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
