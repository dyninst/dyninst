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

#ifndef PCEVENTMUXER_H
#define PCEVENTMUXER_H

#include "common/src/dthread.h"

#include "Event.h"
#include "dyninstAPI/h/BPatch_process.h"
#include "dyninstAPI/src/syscallNotification.h"

#include <map>
#include <queue>
/*
 * Overall design comment
 * 
 * ProcControl events have two items of interest: the event type (which tells us what happened)
 * and the event process (which tells us who it happened to). ProcControl internally muxes a
 * callback based on the type, as you can register a different callback for each event type. 
 * However, it does not mux based on process. Therefore, we have to do the muxing here in a
 * multiprocess situation.
 *
 * To make matters worse, we want to handle some events internally, without waiting for
 * the user to explicitly wait() for an event. For example, traps corresponding to 
 * tramp jumps; such traps should be handled as quickly as possible. 
 *
 * This leads to the following design:
 *
 * pcEventMuxer - unique class (static and globally accessible). 
 *   - Registers callbacks with ProcControl
 *   - Contains a list of all active pcEventHandlers
 *   - Loops in ProcControl::wait(), waiting for events to be received
 *   - When an event is received, decode and determine if it is "fast" or "slow"
 *   - Fast events are handled immediately
 *   - Slow events are entered on a queue to be handled when the user calls
 *     pcEventMuxer::wait. 
 * pcEventHandler - one per active process and a friend of pcProcess
 *   - Is called by pcEventMuxer to handle fast and slow process events. 
 *
 * So the flow of events is twofold:
 *   pcEM::main() -> wait() -> decode() -> handle (fast) or enqueue (slow)
 *   pcEH::wait -> dequeue -> handle
 * 
 */

class PCEventMailbox {
public:
    PCEventMailbox();
    ~PCEventMailbox();

    void enqueue(Dyninst::ProcControlAPI::Event::const_ptr ev);
    Dyninst::ProcControlAPI::Event::const_ptr dequeue(bool block);
    unsigned int size();
    bool find(PCProcess *proc);

protected:
    std::map<int, int> procCount;
    std::queue<Dyninst::ProcControlAPI::Event::const_ptr> eventQueue;
    CondVar<> queueCond;
};

class PCEventHandler;

class PCEventMuxer {
	friend class PCEventHandler;
public:
	typedef enum {
        EventsReceived,
        NoEvents,
        Error
    } WaitResult;
	typedef Dyninst::ProcControlAPI::Process::cb_ret_t cb_ret_t;
	typedef Dyninst::ProcControlAPI::Event::const_ptr EventPtr;
        
	static PCEventMuxer &muxer() { return muxer_; }

	static bool start();

	static WaitResult wait(bool block);

        bool hasPendingEvents(PCProcess *proc);

        static bool handle(PCProcess *proc = NULL);

// Commented out callbacks are handled by the default instead
	static cb_ret_t defaultCallback(EventPtr);
    static cb_ret_t exitCallback(EventPtr);
    static cb_ret_t crashCallback(EventPtr);
    static cb_ret_t signalCallback(EventPtr);
    static cb_ret_t breakpointCallback(EventPtr);
    static cb_ret_t RPCCallback(EventPtr);
    static cb_ret_t threadCreateCallback(EventPtr);
    static cb_ret_t threadDestroyCallback(EventPtr);
    static cb_ret_t forkCallback(EventPtr);
    static cb_ret_t execCallback(EventPtr);
//    static cb_ret_t forceTerminateCallback(EventPtr);
//    static cb_ret_t libraryCallback(EventPtr);
    static cb_ret_t SingleStepCallback(EventPtr);

	// Platform-specific capability functions
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
     */
	static bool useCallback(Dyninst::ProcControlAPI::EventType et);
	static bool useBreakpoint(Dyninst::ProcControlAPI::EventType et);

private:
	// We need to use a separate thread so that we can fast-handle certain
	// events, such as trapping for PC changes
	static DThread::dthread_ret_t WINAPI main(void *);
	bool registerCallbacks();
	WaitResult wait_internal(bool block);
        bool handle_internal(PCProcess *proc);

	PCEventMuxer();
	bool callbacksRegistered_;
	bool started_;
	DThread thrd_;
	static PCEventMuxer muxer_;
	
	void enqueue(EventPtr);
	EventPtr dequeue(bool block);
	bool handle(EventPtr);

	static Dyninst::ProcControlAPI::Process::cb_ret_t ret_stopped;
	static Dyninst::ProcControlAPI::Process::cb_ret_t ret_continue;
	static Dyninst::ProcControlAPI::Process::cb_ret_t ret_default;


	PCEventMailbox mailbox_;
};

#endif
