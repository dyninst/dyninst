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

/* $Id: signalhandler.h,v 1.18 2005/12/01 00:56:25 jaw Exp $
 */

#ifndef _SIGNAL_HANDLER_H
#define _SIGNAL_HANDLER_H

#include "common/h/Vector.h"
#include "common/h/String.h"
#include "dyninstAPI/src/EventHandler.h"
#include "codeRange.h"

class process;
class dyn_lwp;
class SignalHandler;

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
    EventGate(eventLock *l, eventType type, process *p = NULL, dyn_lwp *lwp = NULL,
              eventStatusCode_t status = NULL_STATUS_INITIALIZER);
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

class SignalHandler;
#define MAX_HANDLERS 64  /*This is just arbitrarily large-ish */
#define HANDLER_TRIM_THRESH 2 /*try to delete handlers if we have more than this */

class SignalGenerator : public EventHandler<EventRecord> {
 friend process *ll_attachToCreatedProcess(int, const pdstring &);
 friend class process;

 public:
   virtual ~SignalGenerator(); 
   
   eventType waitForOneOf(pdvector<eventType> &evts);
   eventType waitForEvent(eventType evt, process *p = NULL, dyn_lwp *lwp = NULL, 
                          eventStatusCode_t status = NULL_STATUS_INITIALIZER);
   bool signalEvent(EventRecord &ev);
   bool signalEvent(eventType t);

   bool signalActiveProcess();

   SignalHandler *findSHWithThreadID(unsigned long tid);
   SignalHandler *findSHWaitingForCallback(CallbackBase *cb);
   bool activeHandlerForProcess(process *p);
   bool anyActiveHandlers();
   bool waitingForActiveProcess() {return waiting_for_active_process;}
   protected:
   //  SignalGenerator should only be constructed via derived classes
   SignalGenerator();

    bool handleEvent(EventRecord &ev)
      { LOCK_FUNCTION(bool, handleEventLocked, (ev));}
    bool handleEventLocked(EventRecord &ev);
    virtual bool waitNextEvent(EventRecord &ev) = 0;
    virtual SignalHandler *newSignalHandler(char *name, int shid)  = 0;

    pdvector<EventGate *> wait_list;
    bool waiting_for_active_process;

    pdvector<SignalHandler *> handlers;
};


class SignalHandler : public EventHandler<EventRecord>
{
  friend class SignalGenerator;
  friend class SyncCallback;
  int id;

  public:
    virtual ~SignalHandler();

  bool idle();
  bool waiting();
  process *activeProcess() {return idle_flag ? NULL : active_proc;}
  CallbackBase *waitingForCallback() {return wait_cb;}
  protected:
    //  SignalHandler is only constructed by derived classes
    SignalHandler(char *name, int shid) : 
                  EventHandler<EventRecord>(global_mutex, 
                  name, false /*start thread?*/), id(shid), idle_flag(true),
                  wait_flag(false), wait_cb(NULL), active_proc(NULL) { }
    
  virtual bool handleEvent(EventRecord &ev) = 0;
  bool waitNextEvent(EventRecord &ev);

  bool assignEvent(EventRecord &ev);
  pdvector<EventRecord> events_to_handle;

  bool idle_flag;
  bool wait_flag;
  CallbackBase *wait_cb;
  process *active_proc;

  static void flagBPatchStatusChange() {BPatch::bpatch->mutateeStatusChange = true;}
  static void setBPatchProcessSignal(BPatch_process *p, int t) {p->lastSignal = t;}
};

#if defined(os_windows)
#include "dyninstAPI/src/signalhandler-winnt.h"
#else
#include "dyninstAPI/src/signalhandler-unix.h"
#endif

#endif
