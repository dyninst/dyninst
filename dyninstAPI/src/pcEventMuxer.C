/*
 * Copyright (c) 1996-2010 Barton P. Miller
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
#include "pcEventMuxer.h"
#include "pcEventHandler.h"
#include "BPatch.h"
#include "debug.h"
#include "eventLock.h"
#include "os.h"
#include "dynProcess.h"
#include "mapped_object.h"
#include "registerSpace.h"
#include "RegisterConversion.h"
#include "function.h"

#include "proccontrol/h/Mailbox.h"
#include "proccontrol/h/PCErrors.h"

#include <set>
#include <queue>
#include <vector>

using namespace Dyninst;
using namespace ProcControlAPI;

PCEventMuxer PCEventMuxer::muxer_;
Process::cb_ret_t PCEventMuxer::ret_stopped(Process::cbProcStop, Process::cbProcStop);
Process::cb_ret_t PCEventMuxer::ret_continue(Process::cbProcContinue, Process::cbProcContinue);
Process::cb_ret_t PCEventMuxer::ret_default(Process::cbDefault, Process::cbDefault);

PCEventMuxer::PCEventMuxer() : callbacksRegistered_(false) {
};

bool PCEventMuxer::start() {
	if (muxer().started_) return true;

	// Register callbacks with ProcControl
	if (!muxer().registerCallbacks()) {
		assert(0 && "Unable to register callbacks with ProcControl, fatal error");
	}

	// Spawn off a thread to do the actual work
	// For testing, don't do this. We'll just be slower to respond.
	//muxer().thrd_.spawn((DThread::initial_func_t) PCEventMuxer::main, NULL);
	return true;
}

PCEventMuxer::WaitResult PCEventMuxer::wait(bool block) {
	return muxer().wait_internal(block);
}

PCEventMuxer::WaitResult PCEventMuxer::wait_internal(bool block) {
	proccontrol_printf("[%s/%d]: PCEventMuxer waiting for events, %s\n",
			FILE__, __LINE__, (block ? "blocking" : "non-blocking"));
	if (!block) {
		Process::handleEvents(false);
		proccontrol_printf("[%s:%d] after PC event handling, %d events in mailbox\n", FILE__, __LINE__, mailbox_.size());
		while (mailbox_.size()) {
			EventPtr ev = dequeue(false);
#if defined(os_windows)
			// Windows does early handling of exit, so if we see an exit come through here
			// don't call handle(), just return success
			if (ev->getEventType().code() == EventType::Exit) {
				continue;
			}
#endif
			if (!ev) return NoEvents;
			if (!handle(ev)) return Error;
		}
	}
	else {
		// It's really annoying from a user design POV that ProcControl methods can
		// trigger callbacks; it means that we can't just block here, because we may
		// have _already_ gotten a callback and just not finished processing...
		while (mailbox_.size() == 0) {
			if (!Process::handleEvents(true)) {
				return Error;
			}
		}
		proccontrol_printf("[%s:%d] after PC event handling, %d events in mailbox\n", FILE__, __LINE__, mailbox_.size());
		EventPtr ev = dequeue(false);
#if defined(os_windows)
		// Windows does early handling of exit, so if we see an exit come through here
		// don't call handle(), just return success
		if (ev->getEventType().code() == EventType::Exit) {
			return EventsReceived;
		}
#endif
		if (!ev) {
           proccontrol_printf("[%s:%u] - PCEventMuxer::wait is returning NoEvents\n", FILE__, __LINE__);
           return NoEvents;
        }
        if (!handle(ev)) {
           proccontrol_printf("[%s:%u] - PCEventMuxer::wait is returning error after event handling\n", FILE__, __LINE__);
           return Error;
        }
	}
	return EventsReceived;
}

bool PCEventMuxer::handle(EventPtr ev) {
	// Find the correct PCEventHandler and dispatch
	return PCEventHandler::handle(ev);
}

DThread::dthread_ret_t PCEventMuxer::main(void *) {
	// Loop on wait, decode, handle/enqueue until termination
	// Since we're callback-based, the decode and handle/enqueue will be taken care of
	// by the callback functions. 
	while(1) {
		if (!Process::handleEvents(true)) {
			// Complain
			proccontrol_printf("%s[%d]: error returned by Process::handleEvents: %s\n",
				FILE__, __LINE__, getLastErrorMsg());
			continue;
		}
	}

	return DTHREAD_RET_VAL;
}

bool PCEventMuxer::registerCallbacks() {
	if (callbacksRegistered_) return true;

	bool ret = true;

	ret &= Process::registerEventCallback(EventType(EventType::Any, EventType::Crash), defaultCallback);
	ret &= Process::registerEventCallback(EventType(EventType::Any, EventType::ForceTerminate), defaultCallback);
	ret &= Process::registerEventCallback(EventType(EventType::Any, EventType::ThreadCreate), threadCreateCallback);
	ret &= Process::registerEventCallback(EventType(EventType::Any, EventType::ThreadDestroy), defaultCallback);
	ret &= Process::registerEventCallback(EventType(EventType::Any, EventType::Signal), signalCallback);
	ret &= Process::registerEventCallback(EventType(EventType::Any, EventType::Library), defaultCallback);
	ret &= Process::registerEventCallback(EventType(EventType::Any, EventType::Breakpoint), breakpointCallback);
	ret &= Process::registerEventCallback(EventType(EventType::Any, EventType::RPC), RPCCallback);
	ret &= Process::registerEventCallback(EventType(EventType::Any, EventType::SingleStep), defaultCallback);

	// Fork/exec/exit
	if (useCallback(EventType(EventType::Pre, EventType::Exit))) {
		ret &= Process::registerEventCallback(EventType(EventType::Pre, EventType::Exit), exitCallback);
	}
	if (useCallback(EventType(EventType::Post, EventType::Exit))) {
		ret &= Process::registerEventCallback(EventType(EventType::Post, EventType::Exit), exitCallback);
	}
	if (useCallback(EventType(EventType::Pre, EventType::Fork))) {
		ret &= Process::registerEventCallback(EventType(EventType::Pre, EventType::Fork), defaultCallback);
	}
	if (useCallback(EventType(EventType::Post, EventType::Fork))) {
		ret &= Process::registerEventCallback(EventType(EventType::Post, EventType::Fork), defaultCallback);
	}
	if (useCallback(EventType(EventType::Pre, EventType::Exec))) {
		ret &= Process::registerEventCallback(EventType(EventType::Pre, EventType::Exec), defaultCallback);
	}
	if (useCallback(EventType(EventType::Post, EventType::Exec))) {
		ret &= Process::registerEventCallback(EventType(EventType::Post, EventType::Exec), defaultCallback);
	}


	callbacksRegistered_ = ret;
	return ret;
};

// Apologies for the #define, but I get tired of copying the same text over and over, and since this has
// a shortcut return it can't be a subfunction. 
#define INITIAL_MUXING \
	PCProcess *process = static_cast<PCProcess *>(ev->getProcess()->getData()); \
    proccontrol_printf("%s[%d]: Begin callbackMux, process pointer = %p\n", FILE__, __LINE__, process); \
    if( process == NULL ) { \
	    proccontrol_printf("%s[%d]: NULL process = default/default\n", FILE__, __LINE__); \
        return Process::cb_ret_t(Process::cbDefault, Process::cbDefault); \
    } \
    Process::cb_ret_t ret = ret_stopped; 

#define DEFAULT_RETURN \
	PCEventMuxer &m = PCEventMuxer::muxer(); \
	m.enqueue(ev); \
	return ret;


PCEventMuxer::cb_ret_t PCEventMuxer::defaultCallback(EventPtr ev) {
	INITIAL_MUXING;

	DEFAULT_RETURN;
}


PCEventMuxer::cb_ret_t PCEventMuxer::exitCallback(EventPtr ev) {
	INITIAL_MUXING;
	proccontrol_printf("[%s:%d] Exit callback\n", FILE__, __LINE__);
	if (ev->getEventType().time() == EventType::Post) {
		ret = ret_default;
	}
#if defined(os_windows)
	// On Windows we only receive post-exit, and as soon as the callback completes
	// we terminate the process. Thus, we must handle things _right now_. 
	muxer().handle(ev);
#endif
	DEFAULT_RETURN;
}

PCEventMuxer::cb_ret_t PCEventMuxer::crashCallback(EventPtr ev) {
	INITIAL_MUXING;

	if (ev->getEventType().time() != EventType::Pre) {
		ret = ret_default;
	}
	DEFAULT_RETURN;
}

PCEventMuxer::cb_ret_t PCEventMuxer::signalCallback(EventPtr ev) {
	INITIAL_MUXING;

	EventSignal::const_ptr evSignal = ev->getEventSignal();

	if (!PCEventHandler::isKillSignal(evSignal->getSignal())) {
		evSignal->clearThreadSignal();
	}
	DEFAULT_RETURN;
}

PCEventMuxer::cb_ret_t PCEventMuxer::breakpointCallback(EventPtr ev) {
	INITIAL_MUXING;

    // Control transfer breakpoints are used for trap-based instrumentation
    // No user interaction is required
    EventBreakpoint::const_ptr evBreak = ev->getEventBreakpoint();

    bool hasCtrlTransfer = false;
    vector<Breakpoint::const_ptr> breakpoints;
    evBreak->getBreakpoints(breakpoints);
    Breakpoint::const_ptr ctrlTransferPt;
    for(vector<Breakpoint::const_ptr>::iterator i = breakpoints.begin();
            i != breakpoints.end(); ++i)
    {
        if( (*i)->isCtrlTransfer() ) {
            hasCtrlTransfer = true;
            ctrlTransferPt = *i;
            break;
        }

        // Explicit synchronization unnecessary here
        if( (*i) == process->getBreakpointAtMain() ) {
            // We need to remove the breakpoint in the ProcControl callback to ensure
            // the breakpoint is not automatically suspended and resumed
            startup_printf("%s[%d]: removing breakpoint at main\n", FILE__, __LINE__);
            if( !process->removeBreakpointAtMain() ) {
                proccontrol_printf("%s[%d]: failed to remove main breakpoint in event handling\n",
                        FILE__, __LINE__);
                ev = Event::const_ptr(new Event(EventType::Error));
            }

            // If we are in the midst of bootstrapping, update the state to indicate
            // that we have hit the breakpoint at main
            if( !process->hasReachedBootstrapState(PCProcess::bs_readyToLoadRTLib) ) {
                process->setBootstrapState(PCProcess::bs_readyToLoadRTLib);
            }

            // Need to pass the event on the user thread to indicate to that the breakpoint
            // at main was hit
        }
    }

    if( hasCtrlTransfer ) {
        proccontrol_printf("%s[%d]: received control transfer breakpoint on thread %d/%d (0x%lx => 0x%lx)\n",
                FILE__, __LINE__, ev->getProcess()->getPid(), ev->getThread()->getLWP(),
                evBreak->getAddress(), ctrlTransferPt->getToAddress());
        ret = ret_continue;
		return ret;
	}	

	DEFAULT_RETURN;
}

PCEventMuxer::cb_ret_t PCEventMuxer::RPCCallback(EventPtr ev) {
	INITIAL_MUXING;
    EventRPC::const_ptr evRPC = ev->getEventRPC();
    inferiorRPCinProgress *rpcInProg = static_cast<inferiorRPCinProgress *>(evRPC->getIRPC()->getData());

    if( rpcInProg->resultRegister == REG_NULL ) {
        // If the resultRegister isn't set, the returnValue shouldn't matter
        rpcInProg->returnValue = NULL;
    }else{
        // Get the result out of a register
        MachRegister resultReg = convertRegID(rpcInProg->resultRegister, 
                ev->getProcess()->getArchitecture());
        MachRegisterVal resultVal;
        if( !ev->getThread()->getRegister(resultReg, resultVal) ) {
            proccontrol_printf("%s[%d]: failed to retrieve register from thread %d/%d\n",
                    FILE__, __LINE__,
                    ev->getProcess()->getPid(), ev->getThread()->getLWP());
            ev = Event::const_ptr(new Event(EventType::Error));
        }else{
            rpcInProg->returnValue = (void *)resultVal;

            proccontrol_printf("%s[%d]: iRPC %lu return value = 0x%lx\n",
                FILE__, __LINE__, rpcInProg->rpc->getID(),
                resultVal);
        }
    }

	DEFAULT_RETURN;
}

PCEventMuxer::cb_ret_t PCEventMuxer::threadCreateCallback(EventPtr ev) {
	INITIAL_MUXING;

	ret = ret_default;

	DEFAULT_RETURN;
}

void PCEventMuxer::enqueue(EventPtr ev) {
	mailbox_.enqueue(ev);
}

PCEventMuxer::EventPtr PCEventMuxer::dequeue(bool block) {
	return mailbox_.dequeue(block);
}


PCEventMailbox::PCEventMailbox()
{
}

PCEventMailbox::~PCEventMailbox()
{
}

void PCEventMailbox::enqueue(Event::const_ptr ev) {
    queueCond.lock();
    eventQueue.push(ev);
    queueCond.broadcast();

    proccontrol_printf("%s[%d]: Added event %s to mailbox\n", FILE__, __LINE__,
            ev->name().c_str());

    queueCond.unlock();
}

Event::const_ptr PCEventMailbox::dequeue(bool block) {
    queueCond.lock();

    if( eventQueue.empty() && !block ) {
        queueCond.unlock();
        return Event::const_ptr();
    }

    while( eventQueue.empty() ) {
        proccontrol_printf("%s[%d]: Blocking for events from mailbox\n", FILE__, __LINE__);
        queueCond.wait();
    }

    Event::const_ptr ret = eventQueue.front();
    eventQueue.pop();
    queueCond.unlock();

    proccontrol_printf("%s[%d]: Returning event %s from mailbox\n", FILE__, __LINE__, ret->name().c_str());
    return ret;
}

unsigned int PCEventMailbox::size() {
    unsigned result = 0;
    queueCond.lock();
    result = (unsigned int) eventQueue.size();
    queueCond.unlock();
    return result;
}

