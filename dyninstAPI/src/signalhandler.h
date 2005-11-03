/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/* $Id: signalhandler.h,v 1.16 2005/11/03 05:21:07 jaw Exp $
 */

#ifndef _SIGNAL_HANDLER_H
#define _SIGNAL_HANDLER_H

#include "common/h/Vector.h"
#include "common/h/String.h"
#include "dyninstAPI/src/EventHandler.h"

#if defined(i386_unknown_nt4_0)
#include "dyninstAPI/src/signalhandler-winnt.h"
#else
#include "dyninstAPI/src/signalhandler-unix.h"
#endif

#include "codeRange.h"
#if defined (AIX_PROC)
extern int SYSSET_MAP(int, int);
#else
#define SYSSET_MAP(x, pid)  (x)
#endif

#if defined (os_osf)
#define GETREG_INFO(x) 0
#define V0_REGNUM 0
#define A0_REGNUM 16
#endif
class process;
class dyn_lwp;


// Return code:
// 0: no event
// 1: event fount

// pid: -1 for all processes
// why: Why the process stopped (return parameter)
// what: what caused the stop (return parameter)
// block: block waiting for a signal?
// waitProcs replacement
class SignalHandler;
SignalHandler *getSH();

class signal_handler_location : public codeRange {
 public:
    signal_handler_location(Address addr, unsigned size);
    Address get_address_cr() const { return addr_; }
    unsigned get_size_cr() const { return size_; }

 private:
    Address addr_;
    unsigned size_;
};

class process;

class EventGate {
  public:
    EventGate(eventLock *l, eventType type, process *p = NULL, dyn_lwp *lwp = NULL);
    EventGate(eventLock *l, eventType type, unsigned long id);
    bool addEvent(eventType type, process *p = NULL);
    ~EventGate();

    EventRecord &wait();
    bool signalIfMatch(EventRecord &ev);

  private:
    pdvector<EventRecord> evts;
    eventLock *lock;
    eventCond *cond;
    EventRecord trigger;
    bool waiting;
};

class SignalHandler : public EventHandler<EventRecord> {
 friend SignalHandler *getSH();
 friend process *ll_attachToCreatedProcess(int, const pdstring &);
 friend class process;

 public:
   virtual ~SignalHandler() {}
   
   eventType waitForOneOf(pdvector<eventType> &evts);
   eventType waitForEvent(eventType evt, process *p = NULL, dyn_lwp *lwp = NULL);
   bool signalEvent(EventRecord &ev);
   bool signalEvent(eventType t);

   bool signalActiveProcess();

#if defined (os_linux)
   bool waitingForStop(process *p); 
   bool notWaitingForStop(process *p); 
#endif

   //  This is here due to a legacy kluge for aix, where a 
   //  Signal event is faked at startup.
   //void fakeEvent(EventRecord &);

   private:
   //  SignalHandler should only be constructed via getSH(), 
   //  which is a friend.
   SignalHandler() : EventHandler<EventRecord>(BPatch_eventLock::getLock(), 
                                  "SYNC",/*start thread?*/ false) { }

#if !defined (os_windows)
     //  functions specific to the unix polling mechanism.
     bool createPollEvent(pdvector<EventRecord> &events, struct pollfd fds, process *curProc);
     bool getFDsForPoll(pdvector<unsigned int> &fds);
     process *findProcessByFD(unsigned int fd);
   bool translateEvent(EventRecord &ev);
#endif

#if !defined (os_linux) && !defined (os_windows)
   bool updateEvents(pdvector<EventRecord> &events, process *p, int lwp_to_use);
   bool decodeProcStatus(process *p, procProcStatus_t status, EventRecord &ev);
   bool updateEventsWithLwpStatus(process *curProc, dyn_lwp *lwp,
                                  pdvector<EventRecord> &events);
#endif
     bool handleEvent(EventRecord &ev)
      { LOCK_FUNCTION(bool, handleEventLocked, (ev));}
     bool handleEventLocked(EventRecord &ev);
     bool waitNextEvent(EventRecord &ev);

     //  event handling helpers
#if defined (os_windows)
    DWORD handleBreakpoint(EventRecord &ev);
    DWORD handleException(EventRecord &ev);
    DWORD handleIllegal(EventRecord &ev);
    DWORD handleViolation(EventRecord &ev);
    DWORD handleThreadCreate(EventRecord &ev);
    DWORD handleThreadExit(EventRecord &ev);
    DWORD handleProcessCreate(EventRecord &ev);
    DWORD handleProcessExitWin(EventRecord &ev);
    DWORD handleProcessSelfTermination(EventRecord &ev);
    DWORD handleDllLoad(EventRecord &ev);
#else
     procSyscall_t decodeSyscall(process *p, eventWhat_t what);
     bool handleSignal(EventRecord &ev);
     bool handleSIGILL(EventRecord &ev);
     bool handleSIGCHLD(EventRecord &ev);
     bool handleSIGTRAP(EventRecord &ev);
     bool handleSigTrap(EventRecord &ev);
   
     bool handleSIGSTOP(EventRecord &ev);
     bool decodeRTSignal(EventRecord &ev);
     bool handleForkEntry(EventRecord &ev);
     bool handleExecEntry(EventRecord &ev);
     bool handleLwpExit(EventRecord &ev);
     bool handleSyscallEntry(EventRecord &ev);
     bool handleSyscallExit(EventRecord &ev);
     bool handleForkExit(EventRecord &ev);
     bool handleExecExit(EventRecord &ev);
     bool handleLoadExit(EventRecord &ev);
     bool handleSingleStep(const EventRecord &ev);
#endif

     pdvector<EventGate *> wait_list;
    // eventType last_event;
     bool waiting_for_active_process;

    
#if defined (os_linux)
    typedef struct {
      process *proc;
      pdvector<int> suppressed_sigs;
      pdvector<dyn_lwp *> suppressed_lwps;
    } stopping_proc_rec;
    pdvector<stopping_proc_rec> stoppingProcs;
    //  SignalHandler::suppressSignalWhenStopping
    //  needed on linux platforms.  Allows the signal handler function
    //  to ignore most non SIGSTOP signals when waiting for a process to stop
    //  Returns true if signal is to be suppressed.
    bool suppressSignalWhenStopping(EventRecord &ev);
    //  SignalHandler::resendSuppressedSignals
    //  called upon receipt of a SIGSTOP.  Sends all deferred signals to the stopped process.
    bool resendSuppressedSignals(EventRecord &ev);
#endif
};


#endif


