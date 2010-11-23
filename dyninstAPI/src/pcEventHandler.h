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

#include "common/h/Dictionary.h"
#include "common/h/Types.h"
#include "common/h/dthread.h"

#include <queue>

class PCEventMailbox {
public:
    PCEventMailbox();
    ~PCEventMailbox();

    void enqueue(ProcControlAPI::Event::const_ptr ev);
    ProcControlAPI::Event::const_ptr dequeue(bool block);

protected:
    std::queue<ProcControlAPI::Event::const_ptr> eventQueue;
    CondVar queueCond;
};

class PCProcess;

/*
 * pcEventHandler.h
 *
 * The entry point for event and callback handling.
 */
class PCEventHandler {
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

    // This information is stored here to avoid having to reference
    // the dictionary_hash class in the BPatch headers -- otherwise
    // this would be in the BPatch layer
    int getStopThreadCallbackID(Address cb);

protected:
    PCEventHandler();

    int stopThreadIDCounter_;
    dictionary_hash<Address, unsigned> stopThreadCallbacks_;

    // Event Handling
    static ProcControlAPI::Process::cb_ret_t callbackMux(ProcControlAPI::Event::const_ptr ev);
    ProcControlAPI::Event::const_ptr extractInfo(ProcControlAPI::Event::const_ptr ev);

    bool eventMux(ProcControlAPI::Event::const_ptr ev) const;
    bool handleExit(ProcControlAPI::EventExit::const_ptr ev, PCProcess *evProc) const;
    bool handleCrash(ProcControlAPI::EventCrash::const_ptr ev, PCProcess *evProc) const;
    bool handleFork(ProcControlAPI::EventFork::const_ptr ev, PCProcess *evProc) const;
    bool handleExec(ProcControlAPI::EventExec::const_ptr ev, PCProcess *evProc) const;
    bool handleThreadCreate(ProcControlAPI::EventNewThread::const_ptr ev, PCProcess *evProc) const;
    bool handleThreadDestroy(ProcControlAPI::EventThreadDestroy::const_ptr ev, PCProcess *evProc) const;
    bool handleSignal(ProcControlAPI::EventSignal::const_ptr ev, PCProcess *evProc) const;
    bool handleLibrary(ProcControlAPI::EventLibrary::const_ptr ev, PCProcess *evProc) const;
    bool handleBreakpoint(ProcControlAPI::EventBreakpoint::const_ptr ev, PCProcess *evProc) const;
    bool handleRPC(ProcControlAPI::EventRPC::const_ptr ev, PCProcess *evProc) const;

    PCEventMailbox *eventMailbox_;

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
