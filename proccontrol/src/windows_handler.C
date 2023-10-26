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

#include <assert.h>
#include <time.h>
#include <numeric>


#include "common/h/dyntypes.h"
#include "symtabAPI/h/Symtab.h"
#include "common/src/pathName.h"
#include "PCErrors.h"
#include "Generator.h"
#include "Event.h"
#include "Handler.h"
#include "Mailbox.h"

#include "procpool.h"
#include "irpc.h"
#include "windows_handler.h"
#include "windows_thread.h"
#include "int_handler.h"
#include "response.h"
#include "int_event.h"

#include "snippets.h"

#include "common/src/parseauxv.h"

#include <sstream>
#include <iostream>






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
	bp_instance* bp = t->isClearingBreakpoint();
	Dyninst::THR_ID tid;
	t->getTID(tid);
	if(bp) {
		pthrd_printf("Clearing breakpoint at 0x%lx (%d/%d)\n", bp->getAddr(), ev->getProcess()->llproc()->getPid(), tid);
		t->markClearingBreakpoint(NULL);
	} else {
		pthrd_printf("Single-step didn't clear breakpoint (%d/%d)\n", ev->getProcess()->llproc()->getPid(), tid);
	}
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
	Dyninst::LWP lwp = static_cast<EventNewThread *>(ev.get())->getLWP();
	ProcPool()->condvar()->lock();
	int_thread* tmp = ProcPool()->findThread(lwp);
	thr = static_cast<windows_thread*>(tmp);
	ProcPool()->condvar()->unlock();
	assert(thr);
                                        
   WinEventNewThread::ptr we = boost::dynamic_pointer_cast<WinEventNewThread>(ev);
   if(we)
   {
		pthrd_printf("WinHandleCreateThread handling thread creation for thread %d, handle %x\n",
			lwp, we->getHandle());
	   thr->setHandle(we->getHandle());
	   thr->setStartFuncAddress((Dyninst::Address)(we->getThreadStart()));
	   thr->setTLSAddress((Dyninst::Address)(we->getTLSBase()));
	   thr->setTID(we->getLWP());
	   if(we->getHandle() == thr->llproc()->plat_getDummyThreadHandle())
	   {
		   thr->llproc()->getStartupTeardownProcs().dec();
	   }
   }

   // Check to see if our start address (if we have one...) is in a system library (if we know
   // where it is...). If it is, set this as a system thread.
   Address start_addr = 0;
   if (thr->getStartFuncAddress(start_addr)) {
	   if (thr->llproc()->addrInSystemLib(start_addr)) {
		   thr->setUser(false);
		   if(thr->llproc()->threadPool()->initialThread()->getPendingStopState().getState() == int_thread::running)
		   {
			   // Move any pending stops from the parent to the child thread.
			   // If the parent thread had a pending stop and this is a system thread, we may have strong confidence that
			   // the child thread is the DebugBreak remote thread.
			   thr->setPendingStop(true);
			   thr->llproc()->threadPool()->initialThread()->setPendingStop(false);
		   }
		   thr->getGeneratorState().setState(int_thread::stopped);
		   thr->getHandlerState().setState(int_thread::stopped);
	   }
   }
   if (!thr->isUser()) {
	   ev->setSuppressCB(true);
   }

   return ret_success;
}

int WindowsHandleNewThr::getPriority() const
{
   return PostPlatformPriority;
}

void WindowsHandleNewThr::getEventTypesHandled(std::vector<EventType> &etypes)
{
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
	pthrd_printf("Windows LWP Destroy handler entered\n");
	
	if (!ev->getThread()->llthrd()->isUser()) {
	   ev->setSuppressCB(true);
    }
	if(ev->getProcess()->llproc()->threadPool()->size() <= 1)
	{
		// Last thread exiting. Do process exit.
		EventExit::ptr exitEvt(new EventExit(EventType::Post, 0));
		exitEvt->setProcess(ev->getProcess());
		exitEvt->setThread(ev->getThread());
		exitEvt->setSyncType(ev->getSyncType());

		ev->getProcess()->llproc()->handlerPool()->addLateEvent(exitEvt);
	}
	return ret_success;
}

int WindowsHandleLWPDestroy::getPriority() const
{
    return PostPlatformPriority;
}

void WindowsHandleLWPDestroy::getEventTypesHandled(std::vector<EventType> &etypes)
{
    etypes.push_back(EventType(EventType::Pre, EventType::LWPDestroy));
	etypes.push_back(EventType(EventType::Pre, EventType::WinStopThreadDestroy));
}

WindowsHandleProcessExit::WindowsHandleProcessExit()
    : Handler("Windows Exit")
{
	do_work = new HandleThreadDestroy();
}

WindowsHandleProcessExit::~WindowsHandleProcessExit()
{
	delete do_work;
}

Handler::handler_ret_t WindowsHandleProcessExit::handleEvent(Event::ptr ev) 
{
	return ret_success;
}

int WindowsHandleProcessExit::getPriority() const
{
	return PrePlatformPriority;
}

void WindowsHandleProcessExit::getEventTypesHandled(std::vector<EventType> &etypes)
{
	etypes.push_back(EventType(EventType::Pre, EventType::Exit));
}


HandlerPool *plat_createDefaultHandlerPool(HandlerPool *hpool)
{
   static bool initialized = false;
   static WindowsHandleNewThr *wnewthread = NULL;
   static WindowsHandleLWPDestroy* wthreaddestroy = NULL;
  static WinHandleSingleStep* wsinglestep = NULL;
   static WinHandleBootstrap* wbootstrap = NULL;
   static WindowsHandleSetThreadInfo* wsti = NULL;
   static WindowsHandleProcessExit* wexit = NULL;
   static WindowsHandleThreadStop *wstop = NULL;
   if (!initialized) {
      wnewthread = new WindowsHandleNewThr();
      wthreaddestroy = new WindowsHandleLWPDestroy();
	  wsinglestep = new WinHandleSingleStep();
	  wbootstrap = new WinHandleBootstrap();
	  wsti = new WindowsHandleSetThreadInfo();
	  wexit = new WindowsHandleProcessExit();
	  wstop = new WindowsHandleThreadStop();
	  initialized = true;
   }
   hpool->addHandler(wnewthread);
   hpool->addHandler(wthreaddestroy);
   hpool->addHandler(wsinglestep);
   hpool->addHandler(wbootstrap);
   hpool->addHandler(wsti);
//   hpool->addHandler(wexit);
   hpool->addHandler(wstop);
   return hpool;
}

bool ProcessPool::LWPIDsAreUnique()
{
	return true;
}


WinHandleBootstrap::WinHandleBootstrap()
{

}

WinHandleBootstrap::~WinHandleBootstrap()
{

}

Handler::handler_ret_t WinHandleBootstrap::handleEvent( Event::ptr ev )
{
	pthrd_printf("WinHandleBootstrap continuing process %d\n", ev->getProcess()->getPid());
	ev->getProcess()->llproc()->setState(int_process::running);	
	if(ev->getProcess()->llproc()->getStartupTeardownProcs().localCount())
	{
		ev->getProcess()->llproc()->getStartupTeardownProcs().dec();
	}
	return ret_success;
}

int WinHandleBootstrap::getPriority() const
{
	return PostPlatformPriority;
}

void WinHandleBootstrap::getEventTypesHandled( std::vector<EventType> &etypes )
{
	etypes.push_back(EventType::Bootstrap);
}

WindowsHandleSetThreadInfo::WindowsHandleSetThreadInfo()
{

}

WindowsHandleSetThreadInfo::~WindowsHandleSetThreadInfo()
{

}

Handler::handler_ret_t WindowsHandleSetThreadInfo::handleEvent( Event::ptr ev )
{
	windows_thread *thr = static_cast<windows_thread*>(ev->getThread()->llthrd());
                                        
   WinEventThreadInfo::ptr we = boost::dynamic_pointer_cast<WinEventThreadInfo>(ev);
   if(we)
   {
	   ProcPool()->rmThread(thr);
		pthrd_printf("WinHandleSetThreadInfo handling thread info update for initial thread %d, handle %x\n",
			we->getLWP(), we->getHandle());
	   thr->setHandle(we->getHandle());
	   thr->setStartFuncAddress((Dyninst::Address)(we->getThreadStart()));
	   thr->setTLSAddress((Dyninst::Address)(we->getTLSBase()));
	   thr->setTID(we->getLWP());
	   thr->setLWP(we->getLWP());
	   we->getProcess()->llproc()->threadPool()->noteUpdatedLWP(thr);
	   ProcPool()->addThread(thr->llproc(), thr);
   }

   // Check to see if our start address (if we have one...) is in a system library (if we know
   // where it is...). If it is, set this as a system thread.
   Address start_addr = 0;
   if (thr->getStartFuncAddress(start_addr)) {
	   if (thr->llproc()->addrInSystemLib(start_addr)) {
		   thr->setUser(false);
		   thr->getUserState().setState(int_thread::running);
		   thr->getGeneratorState().setState(int_thread::stopped);
		   thr->getHandlerState().setState(int_thread::stopped);
	   }
   }
   if (!thr->isUser()) {
	   ev->setSuppressCB(true);
   }

   return ret_success;
}

int WindowsHandleSetThreadInfo::getPriority() const
{
	return DefaultPriority;
}

void WindowsHandleSetThreadInfo::getEventTypesHandled( std::vector<EventType> &etypes )
{
	etypes.push_back(EventType(EventType::Any, EventType::ThreadInfo));
}

WindowsHandleThreadStop::WindowsHandleThreadStop() 
{
}

WindowsHandleThreadStop::~WindowsHandleThreadStop()
{
}

void WindowsHandleThreadStop::getEventTypesHandled(std::vector<EventType> &etypes)
{
	etypes.push_back(EventType(EventType::Any, EventType::WinStopThreadDestroy));
}

int WindowsHandleThreadStop::getPriority() const 
{
	// Needs to hit _after_ the LWP destroy
	return PostPlatformPriority+1;
}
