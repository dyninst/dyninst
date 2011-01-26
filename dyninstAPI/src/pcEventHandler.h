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

#ifndef PCEVENTHANDLER_H
#define PCEVENTHANDLER_H

#include "proccontrol/h/Event.h"
#include "dyninstAPI/h/BPatch_process.h"

#include "common/h/Dictionary.h"
#include "common/h/Types.h"
#include "common/h/dthread.h"

#include "syscallNotification.h"

#include <queue>
#include <set>

class PCEventMailbox {
public:
    PCEventMailbox();
    ~PCEventMailbox();

    void enqueue(ProcControlAPI::Event::const_ptr ev);
    ProcControlAPI::Event::const_ptr dequeue(bool block);
    unsigned int size();

protected:
    std::queue<ProcControlAPI::Event::const_ptr> eventQueue;
    CondVar queueCond;
};

class PCProcess;
class inferiorRPCinProgress;

/*
 * pcEventHandler.h
 *
 * The entry point for event and callback handling.
 */
class PCEventHandler {
    // Why syscallNotification is a friend:
    //
    // It is a friend because it reaches in to determine whether to install
    // breakpoints at specific system calls. I didn't want to expose this to
    // the rest of Dyninst.
    
    friend class syscallNotification;
public:
    typedef enum {
        EventsReceived,
        NoEvents,
        Error
    } WaitResult;

    // Force heap allocation
    static PCEventHandler *createPCEventHandler();

    ~PCEventHandler();

    WaitResult waitForEvents(bool block);
    bool start();

    // Special handling for sync. RPCs issued from callbacks because they result in
    // recursive event handling -- this approach limits the events that can be
    // handled recursively to those associated with the completion of the callback RPC
    void registerCallbackRPC(inferiorRPCinProgress *rpc);
    WaitResult waitForCallbackRPC();

protected:
    PCEventHandler();

    // Event Handling
    static ProcControlAPI::Process::cb_ret_t callbackMux(ProcControlAPI::Event::const_ptr ev);

    bool eventMux(ProcControlAPI::Event::const_ptr ev) const;
    bool handleExit(ProcControlAPI::EventExit::const_ptr ev, PCProcess *evProc) const;
    bool handleFork(ProcControlAPI::EventFork::const_ptr ev, PCProcess *evProc) const;
    bool handleExec(ProcControlAPI::EventExec::const_ptr ev, PCProcess **evProc) const;
    bool handleCrash(ProcControlAPI::EventCrash::const_ptr ev, PCProcess *evProc) const;
    bool handleForceTerminate(ProcControlAPI::EventForceTerminate::const_ptr ev, PCProcess *evProc) const;
    bool handleThreadCreate(ProcControlAPI::EventNewThread::const_ptr ev, PCProcess *evProc) const;
    bool handleThreadDestroy(ProcControlAPI::EventThreadDestroy::const_ptr ev, PCProcess *evProc) const;
    bool handleSignal(ProcControlAPI::EventSignal::const_ptr ev, PCProcess *evProc) const;
    bool handleLibrary(ProcControlAPI::EventLibrary::const_ptr ev, PCProcess *evProc) const;
    bool handleBreakpoint(ProcControlAPI::EventBreakpoint::const_ptr ev, PCProcess *evProc) const;
    bool handleRPC(ProcControlAPI::EventRPC::const_ptr ev, PCProcess *evProc) const;

    enum RTSignalResult {
        ErrorInDecoding,
        NotRTSignal,
        IsRTSignal
    };

    enum RTBreakpointVal {
        NoRTBreakpoint,
        NormalRTBreakpoint,
        SoftRTBreakpoint
    };

    RTSignalResult handleRTSignal(ProcControlAPI::EventSignal::const_ptr ev, PCProcess *evProc) const;
    bool handleStopThread(PCProcess *evProc, Address rt_arg) const;
    bool handleUserMessage(PCProcess *evProc, BPatch_process *bpProc, Address rt_arg) const;
    bool handleDynFuncCall(PCProcess *evProc, BPatch_process *bpProc, Address rt_arg) const;

    // platform-specific
    static bool shouldStopForSignal(int signal);
    static bool isValidRTSignal(int signal, RTBreakpointVal breakpointVal, Address arg1, int status);
    static bool isCrashSignal(int signal);

    /*
     * SYSCALL HANDLING
     *
     * The logic for handling system call entry/exit is complicated by the
     * fact that ProcControlAPI doesn't provide events for all system call
     * entry/exit events we are interested in on all platforms (it doesn't 
     * provide them due to lacking OS debug interfaces). Additionally,
     * for some events we need to alert ProcControl that they have occurred
     * to allow it to update its Process/Thread structures (e.g., Post-Fork
     * on FreeBSD).
     *
     * The following approach is used to manage this situation in a 
     * sane, clean way.
     *
     * There are two events that are used to indicate syscall entry/exit:
     * 1) A Dyninst breakpoint via the RT library
     * 2) A ProcControlAPI event for the syscall
     *
     * There are three cases we need to handle related to these events:
     * case 1: Event 2 is provided by ProcControl -> 1 is not necessary,
     * the BPatch-level event is reported when 2 is received.
     *
     * case 2: Event 2 is not provided by ProcControl -> 1 is necessary,
     * 2 is reported to ProcControl at 1, the BPatch-level event is
     * reported when 2 is received via ProcControl (after it updates its
     * internal data structures)
     *
     * case 3: Event 2 is not provided by ProcControl -> 1 is necessary,
     * the BPatch-level event is reported at 1
     *
     * These cases translate to the following in terms of registering
     * ProcControlAPI callbacks and inserting breakpoints via the
     * syscallNotification class.
     *
     * case 1: register the callback, don't insert the breakpoint
     * case 2: register the callback, insert the breakpoint
     * case 3: don't register the callback, insert the breakpoint
     *
     * The following enum encodes this information, each platform
     * defines a translation from an EventType to this enum.
     */
    enum CallbackBreakpointCase {
        CallbackOnly, // case 1
        BothCallbackBreakpoint, // case 2
        BreakpointOnly, // case 3
        NoCallbackOrBreakpoint // default
    };
    static CallbackBreakpointCase getCallbackBreakpointCase(ProcControlAPI::EventType et);

    PCEventMailbox *eventMailbox_;

    // Callback RPCs
    Mutex pendingCallbackLock_;
    std::set<unsigned long> pendingCallbackRPCs_;
    PCEventMailbox *callbackRPCMailbox_;

    // Callback Thread Management
    static void main_wrapper(void *);
    void main(); // Callback thread main loop

    DThread thrd_;
    CondVar initCond_; //Start-up synchronization
    bool started_;

    int exitNotificationOutput_;
    int exitNotificationInput_;
};

#endif
