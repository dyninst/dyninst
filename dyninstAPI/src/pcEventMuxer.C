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
#include "pcEventMuxer.h"
#include "pcEventHandler.h"
#include "BPatch.h"
#include "debug.h"
#include "os.h"
#include "dynProcess.h"
#include "mapped_object.h"
#include "registerSpace.h"
#include "RegisterConversion.h"
#include "function.h"

#include "Mailbox.h"
#include "PCErrors.h"

#include <queue>
#include <vector>
#include <mutex>

using namespace Dyninst;
using namespace ProcControlAPI;

PCEventMuxer PCEventMuxer::muxer_;
Process::cb_ret_t PCEventMuxer::ret_stopped(Process::cbProcStop, Process::cbProcStop);
Process::cb_ret_t PCEventMuxer::ret_continue(Process::cbProcContinue, Process::cbProcContinue);
Process::cb_ret_t PCEventMuxer::ret_default(Process::cbDefault, Process::cbDefault);

PCEventMuxer::PCEventMuxer() : callbacksRegistered_(false), started_(false) {
}

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

bool PCEventMuxer::handle(PCProcess *proc) {
   return muxer().handle_internal(proc);
}

PCEventMuxer::WaitResult PCEventMuxer::wait_internal(bool block) {
   proccontrol_printf("[%s/%d]: PCEventMuxer waiting for events, %s\n",
                      FILE__, __LINE__, (block ? "blocking" : "non-blocking"));
   if (!block) {
	  const bool err = !Process::handleEvents(false);
	  const bool no_events = ProcControlAPI::getLastError() == err_noevents;
      if(err && !no_events) {
    	  proccontrol_printf("[%s:%d] PC event handling failed\n", FILE__, __LINE__);
    	  return Error;
      }
      if (mailbox_.size() == 0) {
    	  proccontrol_printf("[%s:%d] The mailbox is empty\n", FILE__, __LINE__);
    	  return NoEvents;
      }
      if (!handle(NULL)) {
         proccontrol_printf("[%s:%d] Failed to handle event\n", FILE__, __LINE__);
         return Error;
      }
      proccontrol_printf("[%s:%d] PC event handling completed; mailbox size is %u\n",
    		  	  FILE__, __LINE__, mailbox_.size());
      return EventsReceived;
   }
      // It's really annoying from a user design POV that ProcControl methods can
      // trigger callbacks; it means that we can't just block here, because we may
      // have _already_ gotten a callback and just not finished processing...
     proccontrol_printf("[%s:%d] PCEventMuxer::wait_internal, blocking, mailbox size is %u\n", 
			FILE__, __LINE__, mailbox_.size());
     while (mailbox_.size() == 0) {
       if (!Process::handleEvents(true)) {
         proccontrol_printf("[%s:%d] Failed to handle event, returning error\n", FILE__, __LINE__);
	 return Error;
       }
     }
     proccontrol_printf("[%s:%d] after PC event handling, %u events in mailbox\n", FILE__, __LINE__, mailbox_.size());
     if (!handle(NULL)) {
    	 proccontrol_printf("[%s:%d] PC event handling failed\n", FILE__, __LINE__);
    	 return Error;
     }
     proccontrol_printf("[%s:%d] PC event handling completed\n", FILE__, __LINE__);
     return EventsReceived;
}

bool PCEventMuxer::handle_internal(PCProcess *) {
   bool ret = true;
   while (mailbox_.size()) {
      EventPtr ev = dequeue(false);
      if (!ev) continue;
      if (!handle(ev)) ret = false;
   }
   return ret;
}
   

bool PCEventMuxer::hasPendingEvents(PCProcess *proc) {
   return mailbox_.find(proc);
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
	ret &= Process::registerEventCallback(EventType(EventType::Any, EventType::ThreadDestroy), threadDestroyCallback);
	ret &= Process::registerEventCallback(EventType(EventType::Any, EventType::Signal), signalCallback);
	ret &= Process::registerEventCallback(EventType(EventType::Any, EventType::Library), defaultCallback);
	ret &= Process::registerEventCallback(EventType(EventType::Any, EventType::Breakpoint), breakpointCallback);
	ret &= Process::registerEventCallback(EventType(EventType::Any, EventType::RPC), RPCCallback);
	ret &= Process::registerEventCallback(EventType(EventType::Any, EventType::SingleStep), SingleStepCallback);

	// Fork/exec/exit
	if (useCallback(EventType(EventType::Pre, EventType::Exit))) {
		ret &= Process::registerEventCallback(EventType(EventType::Pre, EventType::Exit), exitCallback);
		assert(ret);
		
	}
	if (useCallback(EventType(EventType::Post, EventType::Exit))) {
		ret &= Process::registerEventCallback(EventType(EventType::Post, EventType::Exit), exitCallback);
		assert(ret);
		
	}
	if (useCallback(EventType(EventType::Pre, EventType::Fork))) {
		ret &= Process::registerEventCallback(EventType(EventType::Pre, EventType::Fork), defaultCallback);
		assert(ret);
		
	}
	if (useCallback(EventType(EventType::Post, EventType::Fork))) {
		ret &= Process::registerEventCallback(EventType(EventType::Post, EventType::Fork), defaultCallback);
		assert(ret);
		
	}
	if (useCallback(EventType(EventType::Pre, EventType::Exec))) {
		ret &= Process::registerEventCallback(EventType(EventType::Pre, EventType::Exec), defaultCallback);
		assert(ret);
		
	}
	if (useCallback(EventType(EventType::Post, EventType::Exec))) {
		ret &= Process::registerEventCallback(EventType(EventType::Post, EventType::Exec), defaultCallback);
		assert(ret);
		
	}


	callbacksRegistered_ = ret;
	return ret;
}

// Apologies for the #define, but I get tired of copying the same text over and over, and since this has
// a shortcut return it can't be a subfunction. 
#define INITIAL_MUXING \
   PCProcess *process = static_cast<PCProcess *>(ev->getProcess()->getData()); \
   proccontrol_printf("%s[%d]: Begin callbackMux, process pointer = %p, event %s\n", \
                      FILE__, __LINE__, (void*)process, ev->name().c_str());     \
   if( process == NULL ) {                                              \
      proccontrol_printf("%s[%d]: NULL process = default/default\n", FILE__, __LINE__); \
      return ret_default;                                               \
   }                                                                    \
   Process::cb_ret_t ret = ret_stopped;                                 

#define DEFAULT_RETURN \
	PCEventMuxer &m = PCEventMuxer::muxer(); \
	m.enqueue(ev); \
        proccontrol_printf("%s[%d]: after muxing event, mailbox size is %u\n", \
                           FILE__, __LINE__, m.mailbox_.size()); \
	return ret;


PCEventMuxer::cb_ret_t PCEventMuxer::defaultCallback(EventPtr ev) {
	INITIAL_MUXING;

	DEFAULT_RETURN;
}

#include "InstructionDecoder.h"
using namespace InstructionAPI;
//#sasha Remove this callback afterwards
PCEventMuxer::cb_ret_t PCEventMuxer::SingleStepCallback(EventPtr ev) {
    INITIAL_MUXING;
    cerr << "  ==== SingleStep Callback ====" << endl;
    ret = ret_continue;
    ev->getThread()->setSingleStepMode(true);

    char command;
    cin >> command;
    if(command!='n')
        return ret;

    MachRegister pcReg = MachRegister::getPC(ev->getProcess()->getArchitecture());
    MachRegisterVal loc;
    bool result = ev->getThread()->getRegister(pcReg, loc);
    if (!result) {
        fprintf(stderr,"Failed to read PC register\n");
        return Process::cbDefault;
    }

    Address pc = 0;
    ProcControlAPI::RegisterPool regs;
    ev->getThread()->getAllRegisters(regs);

    for (ProcControlAPI::RegisterPool::iterator iter = regs.begin(); iter != regs.end(); ++iter) {
        //cerr << "\t Reg " << (*iter).first.name() << ": " << hex << (*iter).second << dec << endl;
        if (((*iter).first.isPC())) {
            pc = (*iter).second;
        }
    }

    unsigned disass[1024];
    Address base = pc;
    unsigned size = 4;
    process->readDataSpace((void *) base, size, disass, false);
    InstructionDecoder deco(disass,size,process->getArch());
    Instruction insn = deco.decode();
    while(insn.isValid()) {
      cerr << "\t" << hex << base << ": " << insn.format(base) << dec << endl;
      base += insn.size();
      insn = deco.decode();
    }


    DEFAULT_RETURN;
}

PCEventMuxer::cb_ret_t PCEventMuxer::exitCallback(EventPtr ev) {
	INITIAL_MUXING;
	proccontrol_printf("[%s:%d] Exit callback\n", FILE__, __LINE__);
	//if (ev->getEventType().time() == EventType::Post) {
	//	ret = ret_default;
	//}
	DEFAULT_RETURN;
}

PCEventMuxer::cb_ret_t PCEventMuxer::crashCallback(EventPtr ev) {
	INITIAL_MUXING;

	cerr << "Crash callback" << endl;

	if (ev->getEventType().time() != EventType::Pre) {
		ret = ret_default;
	}
	DEFAULT_RETURN;
}

PCEventMuxer::cb_ret_t PCEventMuxer::signalCallback(EventPtr ev) {
  INITIAL_MUXING;
  
  EventSignal::const_ptr evSignal = ev->getEventSignal();
  
#if defined(DEBUG)
  if (evSignal->getSignal() == 11) {
    Address esp = 0;
    Address pc = 0;
    ProcControlAPI::RegisterPool regs;
    evSignal->getThread()->getAllRegisters(regs);
    for (ProcControlAPI::RegisterPool::iterator iter = regs.begin(); iter != regs.end(); ++iter) {
      cerr << "\t Reg " << (*iter).first.name() << ": " << hex << (*iter).second << dec << endl;
      if ((*iter).first.isStackPointer()) {
	esp = (*iter).second;
      }
      if (((*iter).first.isPC())) {
	pc = (*iter).second;
      }
    }
    
    std::vector<std::vector<Frame> > stacks;
    process->walkStacks(stacks);
    for (unsigned i = 0; i < stacks.size(); ++i) {
      for (unsigned j = 0; j < stacks[i].size(); ++j) {
	cerr << "Frame " << i << "/" << j << ": " << stacks[i][j] << endl;
      }
      cerr << endl << endl;
    }
    for (unsigned i = 0; i < 20; ++i) {
      unsigned tmp = 0;
      process->readDataSpace((void *) (esp + (i * 4)),
			     4, 
			     &tmp,
			     false);
      cerr << "Stack " << hex << esp + (i*4) << ": " << tmp << dec << endl;
    }
    
    unsigned disass[1024];
    Address base = pc - 128;
    unsigned size = 640;
    process->readDataSpace((void *) base, size, disass, false);
    InstructionDecoder deco(disass,size,process->getArch());
    Instruction::Ptr insn = deco.decode();
    while(insn) {
      cerr << "\t" << hex << base << ": " << insn->format(base) << dec << endl;
      base += insn->size();
      insn = deco.decode();
    }
  }
#endif

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
    if (!rpcInProg) {
       // Not us!
       return ret;
    }

    if( rpcInProg->resultRegister == Null_Register ) {
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


PCEventMuxer::cb_ret_t PCEventMuxer::threadDestroyCallback(EventPtr ev) {
	INITIAL_MUXING;

        ret = Process::cb_ret_t(Process::cbThreadStop);

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
	std::lock_guard<CondVar<>> l{queueCond};

    PCProcess *evProc = static_cast<PCProcess *>(ev->getProcess()->getData());
	if(evProc) {
	    // Only add the event to the queue if the underlying process is still valid
	    eventQueue.push(ev);
	    procCount[evProc->getPid()]++;
		proccontrol_printf("%s[%d]: Added event %s from process %d to mailbox, size now %lu\n",
						   FILE__, __LINE__, ev->name().c_str(), evProc->getPid(), eventQueue.size());
	} else {
		proccontrol_printf("%s[%d]: Got bad process: event %s not added\n",
						   FILE__, __LINE__, ev->name().c_str());
		assert(false);
	}

	proccontrol_printf("--------- Enqueue for Process ID [%d] -------------\n", evProc->getPid());
	for(auto const& p : procCount) {
		proccontrol_printf("\t%d -> %d\n", p.first, p.second);
	}
	proccontrol_printf("---------------------------------------------------\n");

    queueCond.broadcast();
}

Event::const_ptr PCEventMailbox::dequeue(bool block) {
    /* NB: This procedure assumes the queue is not modified while we are dequeueing an event */

	/* Holding the lock the entire time isn't efficient, but it's needed to
	 * guarantee that the process-count table is updated synchronously with
	 * the queue.
	 */
	std::lock_guard<CondVar<>> l{queueCond};

    if(!block && eventQueue.empty()) {
		proccontrol_printf("%s[%d]: Event queue is empty, but not blocking\n", FILE__, __LINE__);
		return Event::const_ptr{};
    }

    while( eventQueue.empty() ) {
        proccontrol_printf("%s[%d]: Blocking for events from mailbox\n", FILE__, __LINE__);
        queueCond.wait();
    }

    // Dequeue an event
    Event::const_ptr event_ptr = eventQueue.front();
	eventQueue.pop();

	// Update the process-count table
	auto* evProc = static_cast<PCProcess *>(event_ptr->getProcess()->getData());
	if(!evProc) {
		proccontrol_printf("%s[%d]: Found event %s, but process is invalid\n",
							FILE__, __LINE__, event_ptr->name().c_str());
		namespace pc = Dyninst::ProcControlAPI;
		auto const& et = event_ptr->getEventType();
		bool const is_exit = et.code() == pc::EventType::Exit;
		bool const is_post = et.time() == pc::EventType::Time::Post;
		if(is_exit && is_post) {
			// post-exit events handled after process destruction are irrelevant
			return Event::const_ptr{};
		} else {
			assert(false);
		}
	}
	procCount[evProc->getPid()]--;
	assert(procCount[evProc->getPid()] >= 0);
    proccontrol_printf("%s[%d]: Returning event %s from mailbox for process %d\n",
    				   FILE__, __LINE__, event_ptr->name().c_str(), evProc->getPid());

	proccontrol_printf("--------- Dequeue for Process ID [%d] -------------\n", evProc->getPid());
	for(auto const& p : procCount) {
		proccontrol_printf("\t%d -> %d\n", p.first, p.second);
	}
	proccontrol_printf("---------------------------------------------------\n");

	return event_ptr;
}

unsigned int PCEventMailbox::size() {
    std::lock_guard<CondVar<>> l{queueCond};
    return eventQueue.size();
}

bool PCEventMailbox::find(PCProcess *proc) {
    std::lock_guard<CondVar<>> l{queueCond};
    proccontrol_printf("Calling find for process %p (%d)\n", (void*)proc, proc->getPid());
    assert(proc != nullptr);
    auto it = procCount.find(proc->getPid());
    if (it != procCount.end()) {
        return it->second > 0;
    }
    return false;
}
