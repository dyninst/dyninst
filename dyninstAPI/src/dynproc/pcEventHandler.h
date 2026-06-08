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

#ifndef PCEVENTHANDLER_H
#define PCEVENTHANDLER_H

#include "Event.h"
#include "dyninstAPI/h/BPatch_process.h"

#include <unordered_map>
#include "dyntypes.h"
#include "common/src/dthread.h"

#include "syscallNotification.h"

#include <queue>
#include <set>

class PCProcess;
class inferiorRPCinProgress;
class PCEventMuxer;

/*
 * pcEventHandler.h
 *
 * The entry point for event and callback handling.
 *
 * 1:1 class with PCProcess that encapsulates all event handling, including waiting for
 * events and callbacks. 
 */

class PCEventHandler {
	typedef Dyninst::ProcControlAPI::Event::const_ptr EventPtr;
	// Why syscallNotification is a friend:
    //
    // It is a friend because it reaches in to determine whether to install
    // breakpoints at specific system calls. I didn't want to expose this to
    // the rest of Dyninst.
    
    friend class syscallNotification;
	friend class PCEventMuxer;
public:

	static PCEventHandler &handler() { return handler_; }
	static bool handle(EventPtr ev);

protected:
    PCEventHandler();

	bool handle_internal(EventPtr ev);

    bool handleExit(Dyninst::ProcControlAPI::EventExit::const_ptr ev, PCProcess *evProc) const;
    bool handleFork(Dyninst::ProcControlAPI::EventFork::const_ptr ev, PCProcess *evProc) const;
    bool handleExec(Dyninst::ProcControlAPI::EventExec::const_ptr ev, PCProcess *&evProc) const;
    bool handleCrash(Dyninst::ProcControlAPI::EventCrash::const_ptr ev, PCProcess *evProc) const;
    bool handleForceTerminate(Dyninst::ProcControlAPI::EventForceTerminate::const_ptr ev, PCProcess *evProc) const;
    bool handleThreadCreate(Dyninst::ProcControlAPI::EventNewThread::const_ptr ev, PCProcess *evProc) const;
    bool handleThreadDestroy(Dyninst::ProcControlAPI::EventThreadDestroy::const_ptr ev, PCProcess *evProc) const;
    bool handleSignal(Dyninst::ProcControlAPI::EventSignal::const_ptr ev, PCProcess *evProc) const;
    bool handleLibrary(Dyninst::ProcControlAPI::EventLibrary::const_ptr ev, PCProcess *evProc) const;
    bool handleBreakpoint(Dyninst::ProcControlAPI::EventBreakpoint::const_ptr ev, PCProcess *evProc) const;
    bool handleRPC(Dyninst::ProcControlAPI::EventRPC::const_ptr ev, PCProcess *evProc) const;

    enum RTBreakpointVal {
        NoRTBreakpoint,
        NormalRTBreakpoint,
        SoftRTBreakpoint
    };

    bool handleRTBreakpoint(Dyninst::ProcControlAPI::EventBreakpoint::const_ptr ev, PCProcess *evProc) const;
    bool handleStopThread(PCProcess *evProc, Dyninst::Address rt_arg) const;
    bool handleUserMessage(PCProcess *evProc, BPatch_process *bpProc, Dyninst::Address rt_arg) const;
    bool handleDynFuncCall(PCProcess *evProc, BPatch_process *bpProc, Dyninst::Address rt_arg) const;

    // platform-specific
    static bool shouldStopForSignal(int signal);
    static bool isValidRTSignal(int signal, RTBreakpointVal breakpointVal, Dyninst::Address arg1, int status);
    static bool isCrashSignal(int signal);
    static bool isKillSignal(int signal);

	static PCEventHandler handler_;
};

#endif
