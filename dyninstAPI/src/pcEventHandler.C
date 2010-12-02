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

// TODO this needs to be more general
#if defined(arch_x86) || defined(arch_x86_64)
#include "RegisterConversion-x86.h"
#endif

using namespace Dyninst::ProcControlAPI;
using std::queue;
using std::set;

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

// Start Callback Thread Code

// Callback thread entry point
void PCEventHandler::main_wrapper(void *h) {
    PCEventHandler *handler = (PCEventHandler *) h;
    handler->main();
}

void PCEventHandler::main() {
    setCallbackThreadID(DThread::self());
    proccontrol_printf("%s[%d]: ProcControlAPI callback handler started on thread %lx\n",
            FILE__, __LINE__, DThread::self());

    assert( exitNotificationOutput_ != -1 );

    int pcFD = evNotify()->getFD();

    // Register callbacks before allowing the user thread to continue

    // Any error in registration is a programming error

    // Note: we only care about Post-Exit via ProcControlAPI (we use instrumentation for Pre-Exit)
    assert( Process::registerEventCallback(EventType(EventType::Post, EventType::Exit), PCEventHandler::callbackMux) );
    assert( Process::registerEventCallback(EventType(EventType::Any, EventType::Crash), PCEventHandler::callbackMux) );
    // Note: we don't care about Fork and Exec via ProcControlAPI (we use instrumentation)
    assert( Process::registerEventCallback(EventType(EventType::Any, EventType::ThreadCreate), PCEventHandler::callbackMux) );
    assert( Process::registerEventCallback(EventType(EventType::Any, EventType::ThreadDestroy), PCEventHandler::callbackMux) );
    // Note: we do not care about EventStop's right now (these correspond to internal stops see bug 1121)
    assert( Process::registerEventCallback(EventType(EventType::Any, EventType::Signal), PCEventHandler::callbackMux) );
    assert( Process::registerEventCallback(EventType(EventType::Any, EventType::Library), PCEventHandler::callbackMux) );
    // Note: we do not care about EventBootstrap's
    assert( Process::registerEventCallback(EventType(EventType::Any, EventType::Breakpoint), PCEventHandler::callbackMux) );
    assert( Process::registerEventCallback(EventType(EventType::Any, EventType::RPC), PCEventHandler::callbackMux) );
    assert( Process::registerEventCallback(EventType(EventType::Any, EventType::SingleStep), PCEventHandler::callbackMux) );

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
        if( !Process::handleEvents(true) ) {
            // Report errors but keep trying anyway
            proccontrol_printf("%s[%d]: error returned by Process::handleEvents\n",
                    FILE__, __LINE__);
        }
    }

    proccontrol_printf("%s[%d]: callback thread exiting %lx\n", FILE__, __LINE__);
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
        case EventType::RPC:
        {
            EventRPC::const_ptr evRPC = ev->getEventRPC();
            inferiorRPCinProgress *rpcInProg = (inferiorRPCinProgress *)evRPC->getIRPC()->getData();

            // Get the result out of a register
            MachRegister resultReg = convertRegID(rpcInProg->resultRegister);
            MachRegisterVal resultVal;
            if( !ev->getThread()->getRegister(resultReg, resultVal) ) {
                proccontrol_printf("%s[%d]: failed to retrieve register from thread %d/%d\n",
                        FILE__, __LINE__,
                        ev->getProcess()->getPid(), ev->getThread()->getLWP());
                ev = Event::const_ptr(new Event(EventType::Error));
            }else{
                rpcInProg->returnValue = (void *)resultVal;
            }
        }
           break;
        default:
            break;
    }

    // Queue the event with no processing for now
    eventHandler->eventMailbox_->enqueue(ev);

    // Alert the user that events are now available
    BPatch::bpatch->signalNotificationFD();

    return ret;
}

bool PCEventHandler::start() {
    if( started_ ) return true;

    // Create the mailbox
    eventMailbox_ = new PCEventMailbox;

    // Create the pipe to signal exit
    int pipeFDs[2];
    pipeFDs[0] = pipeFDs[1] = -1;
    if( pipe(pipeFDs) == 0 ) {
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
    : stopThreadIDCounter_(0), stopThreadCallbacks_(addrHash),
      eventMailbox_(NULL), started_(false),
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
}

int PCEventHandler::getStopThreadCallbackID(Address cb) {
    if (stopThreadCallbacks_.defines(cb)) {
        return stopThreadCallbacks_[cb];
    } else {
        int cb_id = ++stopThreadIDCounter_;
        stopThreadCallbacks_[cb] = cb_id;
        return cb_id;
    }
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
    proccontrol_printf("%s[%d]: attempting to handle event: %s\n",
            FILE__, __LINE__, ev->getEventType().name().c_str());

    PCProcess *evProc = (PCProcess *)ev->getProcess()->getData();
    if( evProc == NULL ) {
        proccontrol_printf("%s[%d]: ERROR: handle to Dyninst process is invalid\n",
                FILE__, __LINE__);
        return false;
    }

    bool ret = true;
    bool processDeleted = false;
    switch(ev->getEventType().code()) {
        // Errors first
        case EventType::Error:
        case EventType::Unset:
        // We currently don't use ProcControlAPI's Fork and Exec events
        // as they are not available on every platform
        case EventType::Fork:
        case EventType::Exec:
            ret = false;
            break;
        case EventType::SingleStep: // for now, this should be unused
            if( !evProc->isInDebugSuicide() ) ret = false;
            break;
        // TODO for now these events are skipped
        case EventType::Bootstrap:
        case EventType::Stop:
            break;
        // Interesting events
        case EventType::Exit:
            // We currently don't use ProcControl's Pre-Exit event
            // for the same reasons as above
            if( ev->getEventType().time() == EventType::Pre ) {
                ret = false;
                break;
            }

            ret = handleExit(ev->getEventExit(), evProc);
            if( ev->getEventType().time() == EventType::Post ) {
                processDeleted = true;
            }
            break;
        case EventType::Crash:
            ret = handleCrash(ev->getEventCrash(), evProc);
            if( ev->getEventType().time() != EventType::Pre ) {
                processDeleted = true;
            }
            break;
        case EventType::ThreadCreate:
            ret = handleThreadCreate(ev->getEventNewThread(), evProc);
            break;
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

    if( ret && !processDeleted && evProc->getDesiredProcessState() == PCProcess::ps_running &&
            evProc->isStopped() )
    {
        proccontrol_printf("%s[%d]: user wants process running after event handling\n",
                FILE__, __LINE__);
        if( !evProc->continueProcess() ) {
            proccontrol_printf("%s[%d]: failed to continue process after event handling\n",
                    FILE__, __LINE__);
            ret = false;
        }
    }

    proccontrol_printf("%s[%d]: finished handling event: %s (error = %s)\n",
            FILE__, __LINE__, ev->getEventType().name().c_str(),
            !ret ? "true" : "false");

    return ret;
}

bool PCEventHandler::handleExit(EventExit::const_ptr ev, PCProcess *evProc) const {
    assert( ev->getEventType().time() != EventType::Pre );

    BPatch::bpatch->cleanupProcess(evProc);
    return true;
}

bool PCEventHandler::handleCrash(EventCrash::const_ptr ev, PCProcess *evProc) const {
    if( ev->getEventType().time() == EventType::Pre ) {
        // There is no BPatch equivalent for a Pre-Crash
    }else{ 
        BPatch::bpatch->registerSignalExit(evProc, ev->getTermSignal());
        BPatch::bpatch->cleanupProcess(evProc);
    }

    return true;
}

bool PCEventHandler::handleThreadCreate(EventNewThread::const_ptr /*ev*/, PCProcess * /*evProc*/) const {
    return false;
}

bool PCEventHandler::handleThreadDestroy(EventThreadDestroy::const_ptr /*ev*/, PCProcess * /*evProc*/) const {
    return false;
}

bool PCEventHandler::handleSignal(EventSignal::const_ptr ev, PCProcess *evProc) const {
    proccontrol_printf("%s[%d]: process %d/%d received signal %d\n",
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

    // ProcControlAPI internally forwards signals to processes, just make a note at the
    // BPatch layer that the signal was received

    BPatch_process *bpproc = BPatch::bpatch->getProcessByPid(evProc->getPid());
    if( bpproc == NULL ) {
        proccontrol_printf("%s[%d]: failed to locate BPatch_process for process %d\n",
                FILE__, __LINE__, evProc->getPid());
        return false;
    }

    if( PCEventHandler::shouldStopForSignal(ev->getSignal()) ) {
        evProc->setDesiredProcessState(PCProcess::ps_stopped);

        // Don't deliver stop signals to the process
        ev->clearSignal();
    }

    bpproc->setLastSignal(ev->getSignal());

    return true;
}

PCEventHandler::RTSignalResult
PCEventHandler::handleRTSignal(EventSignal::const_ptr ev, PCProcess *evProc) const {
    // Check whether the signal was sent from the RT library by checking variables
    // in the library

    Address sync_event_breakpoint_addr = evProc->getRTEventBreakpointAddr();
    Address sync_event_id_addr = evProc->getRTEventIdAddr();
    Address sync_event_arg1_addr = evProc->getRTEventArg1Addr();

    int breakpoint;
    int status;
    Address arg1;
    int zero = 0;

    // First, check breakpoint...
    if( sync_event_breakpoint_addr == 0 ) {
        std::string status_str("DYNINST_break_point_event");

        pdvector<int_variable *> vars;
        if( !evProc->findVarsByAll(status_str, vars) ) {
            proccontrol_printf("%s[%d]: failed to find variable %s\n",
                    FILE__, __LINE__, status_str.c_str());
            return ErrorInDecoding;
        }

        if( vars.size() != 1 ) {
            proccontrol_printf("%s[%d]: WARNING: multiple copies of %s found\n",
                    FILE__, __LINE__, status_str.c_str());
        }

        sync_event_breakpoint_addr = vars[0]->getAddress();
        evProc->setRTEventBreakpointAddr(sync_event_breakpoint_addr);
    }

    if( !evProc->readDataWord((const void *)sync_event_breakpoint_addr,
                sizeof(int), &breakpoint, false) ) return ErrorInDecoding;

    const int NO_BREAKPOINT = 0;
    const int NORMAL_BREAKPOINT = 1;
    const int SOFT_BREAKPOINT = 2;
    switch(breakpoint) {
        case NO_BREAKPOINT:
            proccontrol_printf("%s[%d]: signal is not RT library signal\n",
                    FILE__, __LINE__);
            return NotRTSignal;
        case NORMAL_BREAKPOINT:
            if( ev->getSignal() != DYNINST_BREAKPOINT_SIGNUM )
                return NotRTSignal;
            break;
        case SOFT_BREAKPOINT:
            if( ev->getSignal() != SIGSTOP )
                return NotRTSignal;
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
        return ErrorInDecoding;
    }

    if( sync_event_id_addr == 0 ) {
        std::string status_str("DYNINST_synch_event_id");

        pdvector<int_variable *> vars;
        if( !evProc->findVarsByAll(status_str, vars) ) {
            proccontrol_printf("%s[%d]: failed to find variable %s\n",
                    FILE__, __LINE__, status_str.c_str());
            return ErrorInDecoding;
        }

        if( vars.size() != 1 ) {
            proccontrol_printf("%s[%d]: WARNING: multiple copies of %s found\n",
                    FILE__, __LINE__, status_str.c_str());
        }

        sync_event_id_addr = vars[0]->getAddress();
        evProc->setRTEventIdAddr(sync_event_id_addr);
    }

    if( !evProc->readDataWord((const void *)sync_event_id_addr, sizeof(int),
                &status, false) ) return ErrorInDecoding;

    if( status == DSE_undefined ) {
        proccontrol_printf("%s[%d]: signal is not RT library signal\n", FILE__, __LINE__);
        return NotRTSignal;
    }

    // Make sure we don't get this event twice....
    if( !evProc->writeDataWord((void *)sync_event_id_addr, sizeof(int), &zero) ) {
        proccontrol_printf("%s[%d]: failed to reset RT library event id variable\n",
                FILE__, __LINE__);
        return ErrorInDecoding;
    }

    // get runtime library arg1 address
    if( sync_event_arg1_addr == 0 ) {
        std::string arg_str("DYNINST_synch_event_arg1");

        pdvector<int_variable *> vars;
        if( evProc->findVarsByAll(arg_str, vars) ) {
            proccontrol_printf("%s[%d]: failed to find %s\n",
                    FILE__, __LINE__, arg_str.c_str());
            return ErrorInDecoding;
        }

        if( vars.size() != 1 ) {
            proccontrol_printf("%s[%d]: WARNING: multiple copies of %s found\n",
                    FILE__, __LINE__, arg_str.c_str());
            return ErrorInDecoding;
        }

        sync_event_arg1_addr = vars[0]->getAddress();
        evProc->setRTEventArg1Addr(sync_event_arg1_addr);
    }

    if( !evProc->readDataWord((const void *)sync_event_arg1_addr,
                evProc->getAddressWidth(), &arg1, false) ) {
        proccontrol_printf("%s[%d]: failed to read RT library arg1 variable\n",
                FILE__, __LINE__);
        return ErrorInDecoding;
    }

    return handleRTSignal_NP(ev, evProc, arg1, status);
}

bool PCEventHandler::handleStopThread(PCProcess *evProc, Address rt_arg) const {
    Address sync_event_arg2_addr = evProc->getRTEventArg2Addr();
    Address sync_event_arg3_addr = evProc->getRTEventArg3Addr();

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
    if (sync_event_arg2_addr == 0) {
        pdvector<int_variable *> vars;
        std::string arg_str ("DYNINST_synch_event_arg2");
        if (!evProc->findVarsByAll(arg_str, vars)) {
            proccontrol_printf("%s[%d]: cannot find var %s\n",
                    FILE__, __LINE__, arg_str.c_str());
            return false;
        }

        if (vars.size() != 1) {
            proccontrol_printf("%s[%d]: ERROR: %u vars matching %s, not 1\n",
                    FILE__, __LINE__, vars.size(), arg_str.c_str());
            return false;
        }
        sync_event_arg2_addr = vars[0]->getAddress();
        evProc->setRTEventArg2Addr(sync_event_arg2_addr);
    }

    //read arg2 (callbackID)
    if ( !evProc->readDataWord((const void *)sync_event_arg2_addr,
                             evProc->getAddressWidth(), &callbackID, false) ) 
    {
        return false;
    }

/* 1c. The result of the snippet calculation that was given by the user,
       if the point is a return instruction, read the return address */
    // get runtime library arg3 address from runtime lib
    if (sync_event_arg3_addr == 0) {
        pdvector<int_variable *> vars;
        std::string arg_str ("DYNINST_synch_event_arg3");
        if (!evProc->findVarsByAll(arg_str, vars)) {
            proccontrol_printf("%s[%d]: cannot find var %s\n",
                    FILE__, __LINE__, arg_str.c_str());
            return false;
        }
        if (vars.size() != 1) {
            proccontrol_printf("%s[%d]: ERROR: %u vars matching %s, not 1\n",
                    FILE__, __LINE__, vars.size(), arg_str.c_str());
            return false;
        }
        sync_event_arg3_addr = vars[0]->getAddress();
        evProc->setRTEventArg3Addr(sync_event_arg3_addr);
    }
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

        // special handling for the RT library
        dataAddress = (*i)->getLoadAddress();
        if( evProc->usesDataLoadAddress() ) dataAddress = (*i)->getDataLoadAddress();
        fileDescriptor rtLibDesc(evProc->dyninstRT_name, (*i)->getLoadAddress(),
            dataAddress, true);
        if( rtLibDesc == tmpDesc ) {
            assert( !evProc->hasReachedBootstrapState(PCProcess::bs_initialized) );
            proccontrol_printf("%s[%d]: library event contains RT library load\n", FILE__, __LINE__);

            evProc->runtime_lib.insert(newObj);
        }

        // TODO special handling for libc on Linux (breakpoint at __libc_start_main if cannot find main)

        // Register the new modules with the BPatch layer
        const pdvector<mapped_module *> &modlist = newObj->getModules();
        for(unsigned i = 0; i < modlist.size(); ++i) {
            BPatch::bpatch->registerLoadedModule(evProc, modlist[i]);
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

    return true;
}

bool PCEventHandler::handleBreakpoint(EventBreakpoint::const_ptr ev, PCProcess *evProc) const {
    vector<Breakpoint::const_ptr> bps;
    ev->getBreakpoints(bps);
    
    // Breakpoint dispatch
    for(vector<Breakpoint::const_ptr>::const_iterator i = bps.begin(); i != bps.end(); ++i) {
        if( (*i) == evProc->getBreakpointAtMain() ) {
            startup_printf("%s[%d]: removing breakpoint at main\n", FILE__, __LINE__);
            if( !evProc->removeBreakpointAtMain() ) {
                proccontrol_printf("%s[%d]: failed to remove main breakpoint in event handling\n",
                        FILE__, __LINE__);
                return false;
            }

            // If we are in the midst of bootstrapping, update the state to indicate
            // that we have hit the breakpoint at main
            if( !evProc->hasReachedBootstrapState(PCProcess::bs_readyToLoadRTLib) ) {
                evProc->setBootstrapState(PCProcess::bs_readyToLoadRTLib);
            }
        }
        // TODO handle other types of breakpoints (probably syscall stuff)
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
            ev->getThread()->getTid());

    int callbackResult = RPC_LEAVE_AS_IS;
    if( rpcInProg->deliverCallbacks ) {
        proccontrol_printf("%s[%d]: delivering callbacks for RPC %lu\n",
                FILE__, __LINE__, ev->getIRPC()->getID());
        callbackResult = BPatch_process::oneTimeCodeCallbackDispatch(evProc, 
                rpcInProg->rpc->getID(), rpcInProg->userData, rpcInProg->returnValue);
    }

    if( rpcInProg->runProcWhenDone || callbackResult == RPC_RUN_WHEN_DONE ) {
        proccontrol_printf("%s[%d]: continuing process after RPC %lu\n",
                FILE__, __LINE__, ev->getIRPC()->getID());
        if( !evProc->continueProcess() ) {
            proccontrol_printf("%s[%d]: failed to continue process %d after RPC completion\n",
                    FILE__, __LINE__, evProc->getPid());
            return false;
        }
    }

    // If it is synchronous, the caller is responsible for de-allocating the object
    if( rpcInProg->synchronous ) {
        rpcInProg->isComplete = true;
    }else{
        evProc->removeOrigRange(rpcInProg);
        delete rpcInProg;
    }

    return true;
}
