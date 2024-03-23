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

	static bool useCallback(Dyninst::ProcControlAPI::EventType et);
	static bool useBreakpoint(Dyninst::ProcControlAPI::EventType et);

private:
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
