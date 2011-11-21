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
#include "pcEventHandler.h"
#include "BPatch.h"
#include "debug.h"
#include "eventLock.h"
#include "os.h"
#include "pcProcess.h"
#include "mapped_object.h"
#include "registerSpace.h"
#include "RegisterConversion.h"
#include "function.h"

#include "proccontrol/h/Mailbox.h"
#include "proccontrol/h/PCErrors.h"

#include <set>
#include <queue>
#include <vector>
using std::vector;
using std::queue;
using std::set;

using namespace Dyninst::ProcControlAPI;

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

// Start Callback Thread Code

// Callback thread entry point
unsigned long PCEventHandler::main_wrapper(void *h) {
    PCEventHandler *handler = (PCEventHandler *) h;
    handler->main();
	return 0;
}

// Set by callbacks - ProcControlAPI guarantees only one thread will
// execute a callback at a time
static bool eventsQueued = false;

void PCEventHandler::main() {
    setCallbackThreadID(DThread::self());
    proccontrol_printf("%s[%d]: ProcControlAPI callback handler started on thread %lx\n",
            FILE__, __LINE__, DThread::self());

    assert( exitNotificationOutput_ != -1 );

    int pcFD = evNotify()->getFD();

    // Register callbacks before allowing the user thread to continue

    vector<EventType> standardEvents;
    standardEvents.push_back(EventType(EventType::Any, EventType::Crash));
    standardEvents.push_back(EventType(EventType::Any, EventType::ForceTerminate));
    standardEvents.push_back(EventType(EventType::Any, EventType::ThreadCreate));
    standardEvents.push_back(EventType(EventType::Pre, EventType::ThreadDestroy));
    // Note: we do not care about EventStop's right now (these correspond to internal stops see bug 1121)
    standardEvents.push_back(EventType(EventType::Any, EventType::Signal));
    standardEvents.push_back(EventType(EventType::Any, EventType::Library));
    // Note: we do not care about EventBootstrap's
    standardEvents.push_back(EventType(EventType::Any, EventType::Breakpoint));
    standardEvents.push_back(EventType(EventType::Any, EventType::RPC));
    standardEvents.push_back(EventType(EventType::Any, EventType::SingleStep));

    for(vector<EventType>::iterator i = standardEvents.begin();
            i != standardEvents.end(); ++i)
    {
        // Any error in registration is a programming error
        bool registerResult = Process::registerEventCallback(*i, PCEventHandler::callbackMux);
        assert( registerResult && "Failed to register event callback");
    }

    // Check if callbacks should be registered for syscalls on this platform
    vector<EventType> syscallTypes;
    syscallTypes.push_back(EventType(EventType::Pre, EventType::Exit));
    // unused syscallTypes.push_back(EventType(EventType::Post, EventType::Exit));
    syscallTypes.push_back(EventType(EventType::Pre, EventType::Fork));
    syscallTypes.push_back(EventType(EventType::Post, EventType::Fork));
    syscallTypes.push_back(EventType(EventType::Pre, EventType::Exec));
    syscallTypes.push_back(EventType(EventType::Post, EventType::Exec));

    for(vector<EventType>::iterator i = syscallTypes.begin();
            i != syscallTypes.end(); ++i)
    {
        switch(getCallbackBreakpointCase(*i)) {
            case CallbackOnly:
            case BothCallbackBreakpoint: {
                bool registerResult = Process::registerEventCallback(*i, PCEventHandler::callbackMux);
                assert( registerResult && "Failed to register event callback" );
                break;
            }
            default:
                break;
        }
    }

    initCond_.lock();
    initCond_.signal();
    initCond_.unlock();

    while( true ) {
        proccontrol_printf("%s[%d]: waiting for ProcControlAPI callbacks...\n",
                FILE__, __LINE__);

        int nfds = ( (pcFD < exitNotificationOutput_) ? exitNotificationOutput_ : pcFD ) + 1;
        fd_set readset; FD_ZERO(&readset);
        fd_set writeset; FD_ZERO(&writeset);
        fd_set exceptset; FD_ZERO(&exceptset);
        FD_SET(pcFD, &readset);
        FD_SET(exitNotificationOutput_, &readset);

        int result;
        do {
           result = P_select(nfds, &readset, &writeset, &exceptset, NULL);
        } while( result == -1 && errno == EINTR );

        if( result == 0 || result == -1 ) {
            // Report the error but keep trying anyway
            proccontrol_printf("%s[%d]: select on ProcControlAPI fd failed\n");
            Event::const_ptr evError = Event::const_ptr(new Event(EventType::Error));
            eventMailbox_->enqueue(evError);
        }

        // Give precedence to Dyninst user thread over ProcControlAPI's event handling
        if( FD_ISSET(exitNotificationOutput_, &readset) ) {
            proccontrol_printf("%s[%d]: user thread has signaled exit\n", FILE__, __LINE__);
            break;
        }

        if( !FD_ISSET(pcFD, &readset) ) {
            proccontrol_printf("%s[%d]: ProcControlAPI fd not set, waiting again\n");
            continue;
        }

        proccontrol_printf("%s[%d]: attempting to handle events via ProcControlAPI\n",
                FILE__, __LINE__);
        // Don't block for events -- we have already blocked in the select so
        // we know events are available.
        //
        // Additionally, blocking could trigger a race where the user thread
        // invokes a ProcControlAPI operation that implicitly does event
        // handling and thus causing a deadlock where the callback thread waits
        // indefinitely in ProcControlAPI and the user thread is waiting for 
        // the callback thread to leave ProcControlAPI
        if( !Process::handleEvents(false) ) {
            // Report errors but keep trying anyway
            proccontrol_printf("%s[%d]: error returned by Process::handleEvents: %s\n",
                    FILE__, __LINE__,
                    getLastErrorMsg());
        }

        if( eventsQueued ) {
            // Alert the user that events are now available
            BPatch::bpatch->signalNotificationFD();
            eventsQueued = false;
        }
    }

    // Clean-up after ourselves
    proccontrol_printf("%s[%d]: removing Dyninst's ProcControlAPI callbacks\n",
            FILE__, __LINE__);
    // Ignore error code on purpose
    Process::removeEventCallback(PCEventHandler::callbackMux);

    proccontrol_printf("%s[%d]: callback thread exiting\n", FILE__, __LINE__);
}

Process::cb_ret_t PCEventHandler::callbackMux(Event::const_ptr ev) {
    // Get access to the event mailbox
    PCProcess *process = (PCProcess *)ev->getProcess()->getData();

    // This occurs when creating/attaching to the process
    if( process == NULL ) {
        return Process::cb_ret_t(Process::cbDefault, Process::cbDefault);
    }

    Process::cb_ret_t ret(Process::cbProcStop, Process::cbProcStop);
    PCEventHandler *eventHandler = process->getPCEventHandler();

    bool isCallbackRPC = false;

    bool queueEvent = true;

    // Do some event-specific handling
    switch(ev->getEventType().code()) {
        case EventType::Exit:
            // Anything but the default doesn't make sense for a Post-Exit process
            if( ev->getEventType().time() == EventType::Post ) {
                ret = Process::cb_ret_t(Process::cbDefault, Process::cbDefault);
            }
            break;
        case EventType::Crash:
            // Anything but the default doesn't make sense for a Crash
            if( ev->getEventType().time() != EventType::Pre ) {
                ret = Process::cb_ret_t(Process::cbDefault, Process::cbDefault);
            }
            break;
        case EventType::Signal: {
            EventSignal::const_ptr evSignal = ev->getEventSignal();

            // Don't deliver any signals to the process unless we explicitly choose
            // to forward the signal to the process
            if( !PCEventHandler::isKillSignal(evSignal->getSignal()) )
                evSignal->clearThreadSignal();
            break;
        }
        case EventType::Breakpoint: {
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
                ret = Process::cb_ret_t(Process::cbProcContinue, Process::cbProcContinue);
                queueEvent = false;
            }
            break;
        }
        case EventType::RPC:
        {
            EventRPC::const_ptr evRPC = ev->getEventRPC();
            inferiorRPCinProgress *rpcInProg = (inferiorRPCinProgress *)evRPC->getIRPC()->getData();

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

            // Special handling for callback RPCs
            eventHandler->pendingCallbackLock_.lock();
            if( eventHandler->pendingCallbackRPCs_.count(evRPC->getIRPC()->getID()) ) {
                isCallbackRPC = true;
                eventHandler->pendingCallbackRPCs_.erase(evRPC->getIRPC()->getID());
            }
            eventHandler->pendingCallbackLock_.unlock();
        }
            break;
        default:
            break;
    }

    // If callback RPCs cause other events, need to make sure that the RPC thread is still continued
    eventHandler->pendingCallbackLock_.lock();
    if( eventHandler->pendingCallbackRPCs_.size() ) {
        ret = Process::cb_ret_t(Process::cbThreadContinue);
    }
    eventHandler->pendingCallbackLock_.unlock();

    if( queueEvent ) {
        process->incPendingEvents();
        eventsQueued = true;

        if( !isCallbackRPC ) {
            eventHandler->eventMailbox_->enqueue(ev);
        }else{
            eventHandler->callbackRPCMailbox_->enqueue(ev);
        }
    }

    return ret;
}

int makePipe(int* fds)
{
#if !defined(os_windows)
	return pipe(fds);
#else
	assert(!"not implemented");
	return 0;
#endif
}

bool PCEventHandler::start() {
    if( started_ ) return true;

    // Create the mailboxes
    eventMailbox_ = new PCEventMailbox;
    callbackRPCMailbox_ = new PCEventMailbox;

    // Create the pipe to signal exit
    int pipeFDs[2];
    pipeFDs[0] = pipeFDs[1] = -1;
    if( makePipe(pipeFDs) == 0 ) {
        exitNotificationOutput_ = pipeFDs[0];
        exitNotificationInput_ = pipeFDs[1];
    }else{
        proccontrol_printf("%s[%d]: failed to create pipe for callback thread\n",
                FILE__, __LINE__);
        return false;
    }

    initCond_.lock();
    thrd_.spawn(PCEventHandler::main_wrapper, this);

    // Wait for the callback thread to say its ready
    initCond_.wait();

    started_ = true;
    initCond_.unlock();

    return true;
}

// End Callback Thread Code

// Start User Thread Code

PCEventHandler *PCEventHandler::createPCEventHandler() {
    return new PCEventHandler;
}

PCEventHandler::PCEventHandler() 
    : eventMailbox_(NULL),
      callbackRPCMailbox_(NULL), 
      started_(false),
      exitNotificationOutput_(-1), exitNotificationInput_(-1)
{
}

PCEventHandler::~PCEventHandler()
{
    if( !started_ ) return;

    assert( exitNotificationInput_ != -1 );

    char e = 'e';

    if( write(exitNotificationInput_, &e, sizeof(char)) == -1 ) {
        proccontrol_printf("%s[%d]: failed to tell callback thread to exit, not waiting for it\n",
                FILE__, __LINE__);
        return;
    }

    thrd_.join();

    close(exitNotificationInput_);
    close(exitNotificationOutput_);

    if( eventMailbox_ ) delete eventMailbox_;
    if( callbackRPCMailbox_ ) delete callbackRPCMailbox_;
}

void PCEventHandler::registerCallbackRPC(inferiorRPCinProgress *rpc) {
    pendingCallbackLock_.lock();
    pendingCallbackRPCs_.insert(rpc->rpc->getID());
    pendingCallbackLock_.unlock();
}

PCEventHandler::WaitResult PCEventHandler::waitForCallbackRPC() {
    Event::const_ptr newEvent = callbackRPCMailbox_->dequeue(true);
    if( !newEvent ) return NoEvents;

    if( !eventMux(newEvent) ) {
        proccontrol_printf("%s[%d]: error resulted from handling event: %s\n",
                FILE__, __LINE__, newEvent->getEventType().name().c_str());
        return Error;
    }

    return EventsReceived;
}

PCEventHandler::WaitResult PCEventHandler::waitForEvents(bool block) {
    bool handledEvent = false;

    // Empty the mailbox before returning
    Event::const_ptr newEvent;
    while( (newEvent = eventMailbox_->dequeue(block && !handledEvent)) ) {
        if( !eventMux(newEvent) ) {
            proccontrol_printf("%s[%d]: error resulted from handling event: %s\n",
                               FILE__, __LINE__, newEvent->getEventType().name().c_str());
            return Error;
        }
        handledEvent = true;
    }

    return (handledEvent ? EventsReceived : NoEvents);
}

bool PCEventHandler::eventMux(Event::const_ptr ev) const {
    proccontrol_printf("%s[%d]: attempting to handle event %s on thread %d/%d\n",
            FILE__, __LINE__, ev->getEventType().name().c_str(),
            ev->getProcess()->getPid(), ev->getThread()->getLWP());

    PCProcess *evProc = (PCProcess *)ev->getProcess()->getData();
    if( evProc == NULL ) {
        proccontrol_printf("%s[%d]: ERROR: handle to Dyninst process is invalid\n",
                FILE__, __LINE__);
        return false;
    }


    if( !(   ev->getEventType().code() == EventType::ForceTerminate 
          || ev->getEventType().code() == EventType::Crash
          || (ev->getEventType().code() == EventType::Exit &&
              ev->getEventType().time() == EventType::Pre) ) ) 
    {
        // This means we already saw the entry to exit event and we can no longer
        // operate on the process, so ignore the event
        if( evProc->isTerminated() ) {
            proccontrol_printf("%s[%d]: process already marked terminated, ignoring event\n",
                    FILE__, __LINE__);
            // Still need to make sure ProcControl runs the process until it exits
            if( !ev->getProcess()->isTerminated() ) {
                Process::ptr tmpProc(pc_const_cast<Process>(ev->getProcess()));

                if( !tmpProc->continueProc() ) {
                    proccontrol_printf("%s[%d]: failed to continue exiting process\n",
                            FILE__, __LINE__);
                }
            }
            return true;
        }

        // The process needs to be stopped so we can operate on it
        if( !evProc->isStopped() ) {
            proccontrol_printf("%s[%d]: stopping process for event handling\n", FILE__,
                    __LINE__);
            if( !evProc->stopProcess() ) {
                proccontrol_printf("%s[%d]: failed to stop process for event handling\n", FILE__,
                        __LINE__);
                return false;
            }
        }
    }

    // Need to save state because we could be called recursively
    bool prevEventHandlingState = evProc->isInEventHandling();
    evProc->setInEventHandling(true);

    bool ret = true;
    switch(ev->getEventType().code()) {
        // Errors first
        case EventType::Error:
        case EventType::Unset:
            ret = false;
            break;
        case EventType::SingleStep: // for now, this should be unused
            if( !evProc->isInDebugSuicide() ) ret = false;
            break;
        // for now these events are skipped
        case EventType::Bootstrap:
        case EventType::Stop:
            break;
        // Interesting events
        case EventType::Exit:
            ret = handleExit(ev->getEventExit(), evProc);
            break;
        case EventType::Crash:
            ret = handleCrash(ev->getEventCrash(), evProc);
            break;
        case EventType::ForceTerminate:
            ret = handleForceTerminate(ev->getEventForceTerminate(), evProc);
            break;
        case EventType::Fork:
            ret = handleFork(ev->getEventFork(), evProc);
            break;
        case EventType::Exec:
            // On Post-Exec, a new PCProcess is created
            ret = handleExec(ev->getEventExec(), &evProc);
            break;
        case EventType::UserThreadCreate:
        case EventType::LWPCreate:
        case EventType::ThreadCreate:
            ret = handleThreadCreate(ev->getEventNewThread(), evProc);
            break;
        case EventType::UserThreadDestroy:
        case EventType::LWPDestroy:
        case EventType::ThreadDestroy:
            ret = handleThreadDestroy(ev->getEventThreadDestroy(), evProc);
            break;
        case EventType::Signal:
            ret = handleSignal(ev->getEventSignal(), evProc);
            break;
        case EventType::Breakpoint:
            ret = handleBreakpoint(ev->getEventBreakpoint(), evProc);
            break;
        case EventType::LibraryLoad:
        case EventType::LibraryUnload:
        case EventType::Library:
            ret = handleLibrary(ev->getEventLibrary(), evProc);
            break;
        case EventType::RPC:
            ret = handleRPC(ev->getEventRPC(), evProc);
            break;
        default:
            proccontrol_printf("%s[%d]: ignoring unknown event: %s\n",
                    FILE__, __LINE__, ev->getEventType().name().c_str());
            break;
    }

    evProc->decPendingEvents();
    evProc->setInEventHandling(prevEventHandlingState);

    if( dyn_debug_proccontrol ) {
        proccontrol_printf("%s[%d]: continue condition ( %d %d %d %d %d %d )\n",
                FILE__, __LINE__, 
                (int) ret, 
                (int) (evProc->getDesiredProcessState() == PCProcess::ps_running),
                (int) evProc->isStopped(),
                (int) !evProc->hasReportedEvent(),
                (int) !evProc->isTerminated(),
                (int) !evProc->hasPendingEvents());
    }

    if(    ret // there were no errors
        && evProc->getDesiredProcessState() == PCProcess::ps_running // the user wants the process running
        && evProc->isStopped() // the process is stopped
        && !evProc->hasReportedEvent() // we aren't in the middle of processing an event that we reported to ProcControl
        && !evProc->isTerminated() // If one of the handling routines has marked the process exited
        && !evProc->hasPendingEvents() // Can't continue the process until all pending events handled for all threads
      )
    {
        proccontrol_printf("%s[%d]: user wants process running after event handling\n",
                FILE__, __LINE__);
        if( evProc->hasRunningSyncRPC() ) {
            if( !evProc->continueSyncRPCThreads() ) {
                proccontrol_printf("%s[%d]: failed to continue thread after event handling\n",
                        FILE__, __LINE__);
                ret = false;
            }
        }else{
            if( !evProc->continueProcess() ) {
                proccontrol_printf("%s[%d]: failed to continue process after event handling\n",
                        FILE__, __LINE__);
                ret = false;
            }
        }
    }

    if( evProc->isExiting() ) {
        proccontrol_printf("%s[%d]: pending exit reported to BPatch-level, marking process exited\n",
                FILE__, __LINE__);
        evProc->markExited();
    }

    proccontrol_printf("%s[%d]: finished handling event: %s (error = %s)\n",
            FILE__, __LINE__, ev->getEventType().name().c_str(),
            !ret ? "true" : "false");

    return ret;
}

bool PCEventHandler::handleExit(EventExit::const_ptr ev, PCProcess *evProc) const {
    evProc->setReportingEvent(false);
    if( ev->getEventType().time() == EventType::Pre ) {
        // This is handled as an RT signal on all platforms for now
    }else{
        // Currently don't need to do anything special for this
    }

    return true;
}

bool PCEventHandler::handleCrash(EventCrash::const_ptr ev, PCProcess *evProc) const {
    if( ev->getEventType().time() == EventType::Pre ) {
        // There is no BPatch equivalent for a Pre-Crash
    }else{
        // ProcControlAPI process is going away
        evProc->markExited();
        BPatch::bpatch->registerSignalExit(evProc, ev->getTermSignal());
    }

    return true;
}

bool PCEventHandler::handleForceTerminate(EventForceTerminate::const_ptr ev, PCProcess *evProc) const {
    if( ev->getEventType().time() == EventType::Pre ) {
    }else{
        evProc->setExiting(true);
        evProc->markExited();
        BPatch::bpatch->registerSignalExit(evProc, ev->getTermSignal());
    }

    return true;
}

bool PCEventHandler::handleFork(EventFork::const_ptr ev, PCProcess *evProc) const {
    evProc->setReportingEvent(false);
    if( ev->getEventType().time() == EventType::Pre ) {
        // This is handled as an RT signal on all platforms for now
    }else{
        Process::ptr childPCProc(pc_const_cast<Process>(ev->getChildProcess()));
        PCProcess *childProc = PCProcess::setupForkedProcess(evProc, childPCProc);
        if( childProc == NULL ) {
            proccontrol_printf("%s[%d]: failed to create process representation for child %d of process %d\n",
                    FILE__, __LINE__, ev->getChildProcess()->getPid(), evProc->getPid());
            return false;
        }

        switch(getCallbackBreakpointCase(EventType(EventType::Post, EventType::Fork))) {
            case BreakpointOnly:
            case BothCallbackBreakpoint: {
                Address event_breakpoint_addr = childProc->getRTEventBreakpointAddr();
                if( !event_breakpoint_addr ) {
                    proccontrol_printf("%s[%d]: failed to unset breakpoint event flag in process %d\n",
                            FILE__, __LINE__, childProc->getPid());
                    return false;
                }

                int zero = 0;
                if( !childProc->writeDataWord((void *)event_breakpoint_addr, sizeof(int), &zero) ) {
                    proccontrol_printf("%s[%d]: failed to unset breakpoint event flag in process %d\n",
                            FILE__, __LINE__, childProc->getPid());
                    return false;
                }
                break;
            }
            default:
                break;
        }

        BPatch::bpatch->registerForkedProcess(evProc, childProc);

        childProc->setInEventHandling(false);

        // The callback could have continued the process
        if( childProc->getDesiredProcessState() == PCProcess::ps_running &&
            childProc->isStopped() ) 
        {
            proccontrol_printf("%s[%d]: user wants newly created process running after event handling\n",
                    FILE__, __LINE__);
            if( !childProc->continueProcess() ) {
                proccontrol_printf("%s[%d]: failed to continue newly created process %d\n",
                        FILE__, __LINE__, childProc->getPid());
                return false;
            }
        }
    }

    return true;
}

bool PCEventHandler::handleExec(EventExec::const_ptr ev, PCProcess **evProc) const {
    (*evProc)->setReportingEvent(false);
    if( ev->getEventType().time() == EventType::Pre ) {
        // This is handled as an RT signal on all platforms for now
    }else{
        PCProcess *newProc = PCProcess::setupExecedProcess(*evProc, ev->getExecPath());
        if( newProc == NULL ) {
            proccontrol_printf("%s[%d]: failed to setup newly execed process %d\n",
                    FILE__, __LINE__, (*evProc)->getPid());
            return false;
        }

        *evProc = newProc;
    }

    return true;
}

bool PCEventHandler::handleThreadCreate(EventNewThread::const_ptr ev, PCProcess *evProc) const {
    if( !ev->getNewThread()->haveUserThreadInfo() ) {
        proccontrol_printf("%s[%d]: no user thread info for thread %d/%d, postponing thread create\n",
                FILE__, __LINE__, evProc->getPid(), ev->getLWP());
        return true;
    }

    Thread::ptr pcThr = pc_const_cast<Thread>(ev->getNewThread());
    if( pcThr == Thread::ptr() ) {
        proccontrol_printf("%s[%d]: failed to locate ProcControl thread for new thread %d/%d\n",
                FILE__, __LINE__, evProc->getPid(), ev->getLWP());
        return false;
    }

    // Ignore events for the initial thread
    if( pcThr->isInitialThread() ) {
        proccontrol_printf("%s[%d]: event corresponds to initial thread, ignoring thread create\n",
                FILE__, __LINE__, evProc->getPid(), ev->getLWP());
        return true;
    }

    if( evProc->getThread(pcThr->getTID()) != NULL ) {
        proccontrol_printf("%s[%d]: thread already created with TID 0x%lx, ignoring thread create\n",
                FILE__, __LINE__, pcThr->getTID());
        return true;
    }

    BPatch_process *bpproc = BPatch::bpatch->getProcessByPid(evProc->getPid());
    if( bpproc == NULL ) {
        proccontrol_printf("%s[%d]: failed to locate BPatch_process for process %d\n",
                FILE__, __LINE__, evProc->getPid());
        return false;
    }

    PCThread *newThr = PCThread::createPCThread(evProc, pcThr);
    if( newThr == NULL ) {
        proccontrol_printf("%s[%d]: failed to create internal thread representation for new thread %d/%d\n",
                FILE__, __LINE__, evProc->getPid(), ev->getLWP());
        return false;
    }

    evProc->addThread(newThr);

    if( !evProc->registerThread(newThr) ) return false;

    bpproc->triggerThreadCreate(newThr);

    return true;
}

bool PCEventHandler::handleThreadDestroy(EventThreadDestroy::const_ptr ev, PCProcess *evProc) const {
    if( ev->getEventType().time() == EventType::Pre ) {
        BPatch_process *bpproc = BPatch::bpatch->getProcessByPid(evProc->getPid());
        if( bpproc == NULL ) {
            proccontrol_printf("%s[%d]: failed to locate BPatch_process for process %d\n",
                    FILE__, __LINE__, evProc->getPid());
            return false;
        }

        PCThread *exitThread = evProc->getThread(ev->getThread()->getTID());
        if( exitThread == NULL ) {
            // Depending on the platform, we could get lwp and user thread events, so ignore any
            // unknown thread destroy events (they correspond to lwp destroy events)
            proccontrol_printf("%s[%d]: failed to locate internal thread representation for thread %d/%d, ignoring event\n",
                    FILE__, __LINE__, evProc->getPid(), ev->getThread()->getLWP());
            return true;
        }

        BPatch::bpatch->registerThreadExit(evProc, exitThread);
    }
    // Don't do anything for post-ThreadDestroy right now

    return true;
}

bool PCEventHandler::handleSignal(EventSignal::const_ptr ev, PCProcess *evProc) const {
    proccontrol_printf("%s[%d]: thread %d/%d received signal %d\n",
            FILE__, __LINE__, ev->getProcess()->getPid(), ev->getThread()->getLWP(),
            ev->getSignal());

    // Check whether it is a signal from the RT library (note: this will internally
    // handle any entry/exit to syscalls and make the necessary up calls as appropriate)
    RTSignalResult result = handleRTSignal(ev, evProc);
    if( result == ErrorInDecoding ) {
        proccontrol_printf("%s[%d]: failed to determine whether signal came from RT library\n",
                FILE__, __LINE__);
        return false;
    }

    if( result == IsRTSignal ) {
        // handleRTSignal internally does all handling for the events in order to keep
        // related logic in one place
        proccontrol_printf("%s[%d]: signal came from RT library\n", FILE__, __LINE__);
        return true;
    }

    bool shouldForwardSignal = true;

    BPatch_process *bpproc = BPatch::bpatch->getProcessByPid(evProc->getPid());
    if( bpproc == NULL ) {
        proccontrol_printf("%s[%d]: failed to locate BPatch_process for process %d\n",
                FILE__, __LINE__, evProc->getPid());
    }

    if( shouldStopForSignal(ev->getSignal()) ) {
        proccontrol_printf("%s[%d]: signal %d is stop signal, leaving process stopped\n",
                FILE__, __LINE__, ev->getSignal());
        evProc->setDesiredProcessState(PCProcess::ps_stopped);
        shouldForwardSignal = false;
    }

    // Tell the BPatch layer we received a signal
    if( bpproc ) bpproc->setLastSignal(ev->getSignal());

    // Debugging only
    if(    (dyn_debug_proccontrol || dyn_debug_crash)
            && isCrashSignal(ev->getSignal()) ) {
        fprintf(stderr, "Caught crash signal %d for thread %d/%d\n",
                ev->getSignal(), ev->getProcess()->getPid(), ev->getThread()->getLWP());

        RegisterPool regs;
        if( !ev->getThread()->getAllRegisters(regs) ) {
            fprintf(stderr, "%s[%d]: Failed to get registers for crash\n", FILE__, __LINE__);
        }else{
            fprintf(stderr, "Registers at crash:\n");
            for(RegisterPool::iterator i = regs.begin(); i != regs.end(); i++) {
                fprintf(stderr, "\t%s = 0x%lx\n", (*i).first.name().c_str(), (*i).second);
            }
        }

        // Dump the stacks
        pdvector<pdvector<Frame> > stackWalks;
        evProc->walkStacks(stackWalks);
        for (unsigned walk_iter = 0; walk_iter < stackWalks.size(); walk_iter++) {
            fprintf(stderr, "Stack for pid %d, lwpid %d\n",
                    stackWalks[walk_iter][0].getProc()->getPid(),
                    stackWalks[walk_iter][0].getThread()->getLWP());
            for( unsigned i = 0; i < stackWalks[walk_iter].size(); i++ ) {
                cerr << stackWalks[walk_iter][i] << endl;
            }
        }

        // User specifies the action, defaults to core dump
        // (which corresponds to standard Dyninst behavior)
        if(dyn_debug_crash_debugger) {
            if( string(dyn_debug_crash_debugger).find("gdb") != string::npos ) {
                evProc->launchDebugger();

                // If for whatever reason this fails, fall back on sleep
                dyn_debug_crash_debugger = "sleep";
            }

            if( string(dyn_debug_crash_debugger) == string("sleep") ) {
                static volatile int spin = 1;
                while(spin) sleep(1);
            }
        }
    }

    if( shouldForwardSignal ) {
        // Now, explicitly set the signal to be delivered to the process
        ev->setThreadSignal(ev->getSignal());
    }

    return true;
}

PCEventHandler::RTSignalResult
PCEventHandler::handleRTSignal(EventSignal::const_ptr ev, PCProcess *evProc) const {
    // Check whether the signal was sent from the RT library by checking variables
    // in the library -- if we cannot be determine whether this signal came from
    // the RT library, assume it did not.
    
    if( evProc->runtime_lib.size() == 0 ) return NotRTSignal;

    Address sync_event_breakpoint_addr = evProc->getRTEventBreakpointAddr();
    Address sync_event_id_addr = evProc->getRTEventIdAddr();
    Address sync_event_arg1_addr = evProc->getRTEventArg1Addr();

    int breakpoint = 0;
    int status = 0;
    Address arg1 = 0;
    int zero = 0;

    // Check that all addresses could be determined
    if(    sync_event_breakpoint_addr == 0 
        || sync_event_id_addr == 0 
        || sync_event_arg1_addr == 0 ) 
    {
        return NotRTSignal;
    }

    // First, check breakpoint...
    if( !evProc->readDataWord((const void *)sync_event_breakpoint_addr,
                sizeof(int), &breakpoint, false) ) return NotRTSignal;

    switch(breakpoint) {
        case NoRTBreakpoint:
            proccontrol_printf("%s[%d]: signal is not RT library signal\n",
                    FILE__, __LINE__);
            return NotRTSignal;
        case NormalRTBreakpoint:
        case SoftRTBreakpoint:
            // More work to do
            break;
        default:
            proccontrol_printf("%s[%d]: invalid value for RT library breakpoint variable\n",
                    FILE__, __LINE__);
            return NotRTSignal;
    }

    // Make sure we don't get this event twice....
    if( !evProc->writeDataWord((void *)sync_event_breakpoint_addr, sizeof(int), &zero) ) {
        proccontrol_printf("%s[%d]: failed to reset RT library breakpoint variable\n",
                FILE__, __LINE__);
        return NotRTSignal;
    }

    // Get the type of the event
    if( !evProc->readDataWord((const void *)sync_event_id_addr, sizeof(int),
                &status, false) ) return NotRTSignal;

    if( status == DSE_undefined ) {
        proccontrol_printf("%s[%d]: signal is not RT library signal\n", FILE__, __LINE__);
        return NotRTSignal;
    }

    // get runtime library arg1 address
    if( !evProc->readDataWord((const void *)sync_event_arg1_addr,
                evProc->getAddressWidth(), &arg1, false) ) {
        proccontrol_printf("%s[%d]: failed to read RT library arg1 variable\n",
                FILE__, __LINE__);
        return NotRTSignal;
    }

    if( !isValidRTSignal(ev->getSignal(), (RTBreakpointVal) breakpoint, arg1, status) ) return NotRTSignal;

    BPatch_process *bproc = BPatch::bpatch->getProcessByPid(evProc->getPid());
    if( bproc == NULL ) {
        proccontrol_printf("%s[%d]: no corresponding BPatch_process for process %d\n",
                FILE__, __LINE__, evProc->getPid());
        return ErrorInDecoding;
    }

    // See pcEventHandler.h (SYSCALL HANDLING) for a description of what
    // is going on here

    Event::ptr newEvt;

    switch(status) {
    case DSE_forkEntry:
        proccontrol_printf("%s[%d]: decoded forkEntry, arg = %lx\n",
                      FILE__, __LINE__, arg1);
        switch(getCallbackBreakpointCase(EventType(EventType::Pre, EventType::Fork))) {
            case BreakpointOnly:
                proccontrol_printf("%s[%d]: reporting fork entry event to BPatch layer\n",
                        FILE__, __LINE__);
                BPatch::bpatch->registerForkingProcess(evProc->getPid(), NULL);
                break;
            case BothCallbackBreakpoint:
                // Cannot create events that ProcControl currently doesn't support
                assert(!"Pre-Fork events are currently not implemented in ProcControlAPI");
                break;
            default:
                break;
        }
        break;
    case DSE_forkExit:
        proccontrol_printf("%s[%d]: decoded forkExit, arg = %lx\n",
                      FILE__, __LINE__, arg1);
        switch(getCallbackBreakpointCase(EventType(EventType::Post, EventType::Fork))) {
            case BreakpointOnly:
                assert(!"Post-Fork via just a breakpoint is invalid");
                break;
            case BothCallbackBreakpoint:
                proccontrol_printf("%s[%d]: reporting fork exit event to ProcControlAPI\n",
                        FILE__, __LINE__);
                newEvt = Event::ptr(new EventFork(EventType::Pre, (Dyninst::PID)arg1));
                break;
            default:
                break;
        }
        break;
    case DSE_execEntry:
        proccontrol_printf("%s[%d]: decoded execEntry, arg = %lx\n",
                      FILE__, __LINE__, arg1);
        // For now, just note that the process is going to exec
        evProc->setExecing(true);
        break;
    case DSE_execExit:
        proccontrol_printf("%s[%d]: decoded execExit, arg = %lx\n",
                      FILE__, __LINE__, arg1);
        // This is not currently used by Dyninst internals for anything
        // We rely on ProcControlAPI for this and it should be impossible
        // to get this via a breakpoint
        return ErrorInDecoding;
    case DSE_exitEntry:
        proccontrol_printf("%s[%d]: decoded exitEntry, arg = %lx\n",
                      FILE__, __LINE__, arg1);
        /* Entry of exit, used for the callback. We need to trap before
           the process has actually exited as the callback may want to
           read from the process */
        switch(getCallbackBreakpointCase(EventType(EventType::Pre, EventType::Exit))) {
            case BreakpointOnly:
                proccontrol_printf("%s[%d]: reporting exit entry event to BPatch layer\n",
                        FILE__, __LINE__);
                evProc->triggerNormalExit((int)arg1);
                break;
            case BothCallbackBreakpoint:
                proccontrol_printf("%s[%d]: reporting exit entry event to ProcControlAPI\n",
                        FILE__, __LINE__);
                newEvt = Event::ptr(new EventExit(EventType::Pre, (int)arg1));
                break;
            default:
                break;
        }
        break;
    case DSE_loadLibrary:
        proccontrol_printf("%s[%d]: decoded loadLibrary (error), arg = %lx\n",
                      FILE__, __LINE__, arg1);
        // This is no longer used
        return ErrorInDecoding;
    case DSE_lwpExit:
        proccontrol_printf("%s[%d]: decoded lwpExit (error), arg = %lx\n",
                      FILE__, __LINE__, arg1);
        // This is not currently used on any platform
        return ErrorInDecoding;
    case DSE_snippetBreakpoint:
        proccontrol_printf("%s[%d]: decoded snippetBreak, arg = %lx\n",
                      FILE__, __LINE__, arg1);
        bproc->setLastSignal(ev->getSignal());
        evProc->setDesiredProcessState(PCProcess::ps_stopped);
        break;
    case DSE_stopThread:
        proccontrol_printf("%s[%d]: decoded stopThread, arg = %lx\n",
                      FILE__, __LINE__, arg1);
        bproc->setLastSignal(ev->getSignal());
        if( !handleStopThread(evProc, arg1) ) {
            proccontrol_printf("%s[%d]: failed to handle stopped thread event\n",
                    FILE__, __LINE__);
            return ErrorInDecoding;
        }
        break;
    case DSE_dynFuncCall:
        proccontrol_printf("%s[%d]: decoded dynamic callsite event, arg = %lx\n",
                FILE__, __LINE__, arg1);
        if( !handleDynFuncCall(evProc, bproc, arg1) ) {
            proccontrol_printf("%s[%d]: failed to handle dynamic callsite event\n",
                    FILE__, __LINE__);
            return ErrorInDecoding;
        }
        break;
    case DSE_userMessage:
        proccontrol_printf("%s[%d]: decoded user message event, arg = %lx\n",
                FILE__, __LINE__, arg1);
        if( !handleUserMessage(evProc, bproc, arg1) ) {
            proccontrol_printf("%s[%d]: failed to handle user message event\n",
                    FILE__, __LINE__);
            return ErrorInDecoding;
        }
        break;
    default:
        return NotRTSignal;
    }

    // Behavior common to all syscalls
    if( newEvt != NULL ) {
        // Report the event to ProcControlAPI, make sure process remains stopped
        evProc->setReportingEvent(true);

        newEvt->setProcess(ev->getProcess());
        newEvt->setThread(ev->getThread());

        // In the callback thread, the process and thread are stopped
        newEvt->setSyncType(Event::sync_process);
        newEvt->setUserEvent(true);

        ProcControlAPI::mbox()->enqueue(newEvt);
    }

    return IsRTSignal;
}

bool PCEventHandler::handleUserMessage(PCProcess *evProc, BPatch_process *bpProc, 
        Address rt_arg) const 
{
    // First argument is the pointer of the message in the mutatee
    // Second argument is the size of the message
    Address sync_event_arg2_addr = evProc->getRTEventArg2Addr();

    if( sync_event_arg2_addr == 0 ) {
        return false;
    }

    unsigned long msgSize = 0;
    if( !evProc->readDataWord((const void *)sync_event_arg2_addr,
                evProc->getAddressWidth(), &msgSize, false) )
    {
        return false;
    }

    unsigned char *buffer = new unsigned char[msgSize];

    // readDataSpace because we are reading a block of data
    if( !evProc->readDataSpace((const void *)rt_arg, msgSize, buffer, false) ) {
        return false;
    }

    BPatch::bpatch->registerUserEvent(bpProc, buffer, (unsigned int)msgSize);

    delete[] buffer;

    return true;
}

bool PCEventHandler::handleDynFuncCall(PCProcess *evProc, BPatch_process *bpProc, 
        Address rt_arg) const
{
    // First argument is the call target
    // Second argument is the address of the call
    Address sync_event_arg2_addr = evProc->getRTEventArg2Addr();

    if( sync_event_arg2_addr == 0 ) {
        return false;
    }

    Address callAddress = 0;
    if( !evProc->readDataWord((const void *)sync_event_arg2_addr,
                evProc->getAddressWidth(), &callAddress, false) )
    {
        return false;
    }

    BPatch::bpatch->registerDynamicCallsiteEvent(bpProc, rt_arg, callAddress);

    return true;
}

bool PCEventHandler::handleStopThread(PCProcess *evProc, Address rt_arg) const {
    // 1. Need three pieces of information:

    /* 1a. The instrumentation point that triggered the stopThread event */
    Address pointAddress = rt_arg;

    // Read args 2,3 from the runtime library, as in decodeRTSignal,
    // didn't do it there since this is the only RT library event that
    // makes use of them
    int callbackID = 0; //arg2
    void *calculation = NULL; // arg3

/* 1b. The ID of the callback function given at the registration
       of the stopThread snippet */
    // get runtime library arg2 address from runtime lib
    Address sync_event_arg2_addr = evProc->getRTEventArg2Addr();
    if (sync_event_arg2_addr == 0) return false;

    //read arg2 (callbackID)
    if ( !evProc->readDataWord((const void *)sync_event_arg2_addr,
                             evProc->getAddressWidth(), &callbackID, false) ) 
    {
        return false;
    }

/* 1c. The result of the snippet calculation that was given by the user,
       if the point is a return instruction, read the return address */
    // get runtime library arg3 address from runtime lib
    Address sync_event_arg3_addr = evProc->getRTEventArg3Addr();
    if (sync_event_arg3_addr == 0) return false;

    //read arg3 (calculation)
    if ( !evProc->readDataWord((const void *)sync_event_arg3_addr,
                       evProc->getAddressWidth(), &calculation, false) )
    {
        return false;
    }

    return evProc->triggerStopThread(pointAddress, callbackID, calculation);
} 

bool PCEventHandler::handleLibrary(EventLibrary::const_ptr ev, PCProcess *evProc) const {
    const fileDescriptor &execFd = evProc->getAOut()->getFileDesc();

    if( ev->libsAdded().size() == 0 && ev->libsRemoved().size() == 0 ) {
        proccontrol_printf("%s[%d]: library event contains no changes to library state\n",
                FILE__, __LINE__);
        return true;
    }

    // Create new mapped objects for all the new loaded libraries
    const set<Library::ptr> &added = ev->libsAdded();
    for(set<Library::ptr>::const_iterator i = added.begin(); i != added.end(); ++i) {
        Address dataAddress = (*i)->getLoadAddress();
        if( evProc->usesDataLoadAddress() ) dataAddress = (*i)->getDataLoadAddress();

        fileDescriptor tmpDesc((*i)->getName(), (*i)->getLoadAddress(),
                    dataAddress, true);
        if( execFd == tmpDesc ) {
            proccontrol_printf("%s[%d]: ignoring Library event for executable %s\n",
                    FILE__, __LINE__, (*i)->getName().c_str());
            continue;
        }

        mapped_object *newObj = mapped_object::createMappedObject(tmpDesc,
                evProc, evProc->getHybridMode());
        if( newObj == NULL ) {
            proccontrol_printf("%s[%d]: failed to create mapped object for library %s\n",
                    FILE__, __LINE__, (*i)->getName().c_str());
            return false;
        }

        proccontrol_printf("%s[%d]: new mapped object: %s\n", FILE__, __LINE__, newObj->debugString().c_str());
        evProc->addASharedObject(newObj);

        // TODO special handling for libc on Linux (breakpoint at __libc_start_main if cannot find main)

        // special handling for the RT library
        dataAddress = (*i)->getLoadAddress();
        if( evProc->usesDataLoadAddress() ) dataAddress = (*i)->getDataLoadAddress();
        fileDescriptor rtLibDesc(evProc->dyninstRT_name, (*i)->getLoadAddress(),
            dataAddress, true);
        if( rtLibDesc == tmpDesc ) {
            assert( !evProc->hasReachedBootstrapState(PCProcess::bs_initialized) );
            proccontrol_printf("%s[%d]: library event contains RT library load\n", FILE__, __LINE__);

            // In the dynamic case, we can only work with dynamic binaries at
            // this point and thus the RT library is a shared library, so the
            // runtime_lib structure should be empty
            assert( evProc->runtime_lib.size() == 0 );

            evProc->runtime_lib.insert(newObj);
            // Don't register the runtime library with the BPatch layer
        }else{
            // Register the new modules with the BPatch layer
            const pdvector<mapped_module *> &modlist = newObj->getModules();
            for(unsigned i = 0; i < modlist.size(); ++i) {
                BPatch::bpatch->registerLoadedModule(evProc, modlist[i]);
            }
        }
    }

    // Create descriptors for all the deleted objects and find the corresponding
    // mapped objects using these descriptors
    vector<fileDescriptor> deletedDescriptors;
    const set<Library::ptr> &deleted = ev->libsRemoved();
    for(set<Library::ptr>::const_iterator i = deleted.begin(); i != deleted.end(); ++i) {
        Address dataAddress = (*i)->getLoadAddress();
        if( evProc->usesDataLoadAddress() ) dataAddress = (*i)->getDataLoadAddress();
        deletedDescriptors.push_back(fileDescriptor((*i)->getName(), (*i)->getLoadAddress(),
                    dataAddress, true));
    }

    const pdvector<mapped_object *> &currList = evProc->mappedObjects();
    pdvector<mapped_object *> toDelete;
    for(unsigned i = 0; i < currList.size(); ++i) {
        for(unsigned j = 0; j < deletedDescriptors.size(); ++j) {
            if( deletedDescriptors[j] == currList[i]->getFileDesc() ) {
                toDelete.push_back(currList[i]);
            }
        }
    }

    // Register the deletion with the BPatch layer before removing the modules
    // from the address space
    for(unsigned i = 0; i < toDelete.size(); ++i) {
        const pdvector<mapped_module *> &modlist = toDelete[i]->getModules();
        for(unsigned j = 0; j < modlist.size(); ++j) {
            BPatch::bpatch->registerUnloadedModule(evProc, modlist[j]);
        }

        proccontrol_printf("%s[%d]: removed map object: %s\n", FILE__, __LINE__, toDelete[i]->debugString().c_str());
        evProc->removeASharedObject(toDelete[i]);
    }

    // A thread library may have been loaded -- mt_cache state needs to be re-evaluated
    evProc->invalidateMTCache();

    return true;
}

bool PCEventHandler::handleBreakpoint(EventBreakpoint::const_ptr ev, PCProcess *evProc) const {
    if( dyn_debug_proccontrol && evProc->isBootstrapped() ) {
        RegisterPool regs;
        if( !ev->getThread()->getAllRegisters(regs) ) {
            fprintf(stderr, "%s[%d]: Failed to get registers at breakpoint\n", FILE__, __LINE__);
        }else{
            fprintf(stderr, "Registers at breakpoint:\n");
            for(RegisterPool::iterator i = regs.begin(); i != regs.end(); i++) {
                fprintf(stderr, "\t%s = 0x%lx\n", (*i).first.name().c_str(), (*i).second);
            }
        }
    }

    return true;
}

bool PCEventHandler::handleRPC(EventRPC::const_ptr ev, PCProcess *evProc) const {
    inferiorRPCinProgress *rpcInProg = (inferiorRPCinProgress *) ev->getIRPC()->getData();

    if( rpcInProg == NULL ) {
        proccontrol_printf("%s[%d]: ERROR: handle to Dyninst rpc container is invalid\n",
                FILE__, __LINE__);
        return false;
    }

    proccontrol_printf("%s[%d]: handling completion of RPC %lu on thread %d/%d\n",
            FILE__, __LINE__, ev->getIRPC()->getID(), ev->getProcess()->getPid(),
            ev->getThread()->getLWP());

    int callbackResult = RPC_LEAVE_AS_IS;
    if( rpcInProg->deliverCallbacks ) {
        proccontrol_printf("%s[%d]: delivering callbacks for RPC %lu\n",
                FILE__, __LINE__, ev->getIRPC()->getID());
        callbackResult = BPatch_process::oneTimeCodeCallbackDispatch(evProc, 
                rpcInProg->rpc->getID(), rpcInProg->userData, rpcInProg->returnValue);
    }

    if( rpcInProg->runProcWhenDone || callbackResult == RPC_RUN_WHEN_DONE ) {
        proccontrol_printf("%s[%d]: continue requested after RPC %lu\n",
                FILE__, __LINE__, ev->getIRPC()->getID());
        evProc->setDesiredProcessState(PCProcess::ps_running);
    }else{
        proccontrol_printf("%s[%d]: stop requested after RPC %lu\n",
                FILE__, __LINE__, ev->getIRPC()->getID());
        evProc->setDesiredProcessState(PCProcess::ps_stopped);
    }

    if( rpcInProg->memoryAllocated ) {
        evProc->inferiorFree(ev->getIRPC()->getAddress());
    }

    // If it is synchronous, the caller is responsible for de-allocating the object
    if( rpcInProg->synchronous ) {
        rpcInProg->isComplete = true;
        evProc->removeSyncRPCThread(rpcInProg->thread);
    }else{
        delete rpcInProg;
    }

    return true;
}
