/*
 * Copyright (c) 1996-2011 Barton P. Miller
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


#ifndef __BPATCH_ASYNC_EVENT_HANDLER_H__
#define __BPATCH_ASYNC_EVENT_HANDLER_H__

#include <errno.h>
#include "os.h"
#include "EventHandler.h"
#include "process.h"
#include <BPatch_process.h> // for BPatch_asyncEventType
#include "dyninstAPI_RT/h/dyninstAPI_RT.h" // for BPatch_asyncEventRecord
#include "common/h/Pair.h"
#include "common/h/Vector.h"


typedef struct {
  process *proc;
  int fd;
  PDSOCKET sock;
} process_record;

const char *asyncEventType2Str(BPatch_asyncEventType evtype); 

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_asyncEventHandler

class BPatch_asyncEventHandler : public EventHandler<EventRecord> {
	friend THREAD_RETURN asyncHandlerWrapper(void *);
	friend class BPatch;  // only BPatch constructs & does init
	friend class BPatch_eventMailbox;
	public:
	//  BPatch_asyncEventHandler::connectToProcess()
	//  Tells the async event handler that there is a new process
	//  to listen for.
	bool connectToProcess(process *p);

	//  BPatch_asyncEventHandler::detachFromProcess()
	//  stop paying attention to events from specified process
	bool detachFromProcess(process *p);

	bool startupThread();

    bool registerMonitoredPoint(BPatch_point *);

  private: 
    BPatch_asyncEventHandler();
    pdvector<EventRecord> event_queue;
    bool initialize();  //  want to catch init errors, so we do most init here
    virtual ~BPatch_asyncEventHandler();
	PDSOCKET setup_socket(int mutatee_pid, std::string &sock_fname);

    //  BPatch_asyncEventHandler::shutDown()
    //  Sets a flag that the async thread will check during its next iteration.
    //  When set, the handler thread will shut itself down.
    bool shutDown();


    //  BPatch_asyncEventHandler::waitNextEvent()
    //  Wait for the next event to come from a mutatee.  Essentially 
    //  a big call to select().
   virtual bool waitNextEvent(EventRecord &ev);

    //  BPatch_asyncEventHandler::handleEvent()
    //  called after waitNextEvent, obtains global lock and handles event.
    //  Since event handling needs to be a locked operation (esp. if it 
    //  requires accessing lower level dyninst data structures), this is
    //  where it should be done.
    virtual bool handleEvent(EventRecord &ev)
       { __LOCK; bool ret = handleEventLocked(ev); __UNLOCK; return ret; }
    bool handleEventLocked(EventRecord &ev);

    //  BPatch_asyncEventHandler::readEvent()
    //  Reads from the async fd connection to the mutatee
    //static asyncReadReturnValue_t readEvent(PDSOCKET fd, void *ev, ssize_t sz);
    static readReturnValue_t readEvent(PDSOCKET fd, EventRecord &ev);

    //  BPatch_asyncEventHandler::mutateeDetach()
    //  use oneTimeCode to call a function in the mutatee to handle
    //  closing of the comms socket.

    bool mutateeDetach(process *p);

    //  BPatch_asyncEventHandler::cleanUpTerminatedProcs()
    //  clean up any references to terminated processes in our lists
    //  (including any user specified callbacks).
    bool cleanUpTerminatedProcs();

    //  BPatch_asyncEventHandler::cleanupProc(process *p)
    //  remove a particular process without detaching. Used in 
    //  exec.
    bool cleanupProc(process *p);

#if defined (os_windows)
    //  These vars are only modified as part of init (before/while threads are
    //  created) so we do not need to worry about locking them:
    PDSOCKET windows_sock;
    unsigned int listen_port;
#else
	int control_pipe_read, control_pipe_write;
#endif
    bool shutDownFlag;

    //  The rest:  Data in this class that is not exclusively set during init
    //   will have to be locked.  
    pdvector<process_record> process_fds;

    dictionary_hash<Address, BPatch_point *> monitored_points;
};

BPatch_asyncEventHandler *getAsync();

#endif // __BPATCH_EVENT_HANDLER_H__
