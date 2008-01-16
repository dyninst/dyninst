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

/* $Id: signalhandler.h,v 1.34 2008/01/16 22:01:41 legendre Exp $
 */

#ifndef _SIGNAL_HANDLER_H
#define _SIGNAL_HANDLER_H

#include "common/h/Vector.h"
#include "common/h/String.h"
#include "dyninstAPI/src/EventHandler.h"
#include "codeRange.h"

typedef enum {
    procSysFork,
    procSysExec,
    procSysExit,
    // Library load "syscall". Used by AIX.
    procSysLoad,
    procLwpExit,
    procSysOther
} procSyscall_t;

class process;
class dyn_lwp;
class SignalHandler;
class fileDescriptor;

class signal_handler_location : public codeRange {
 public:
    signal_handler_location(Address addr, unsigned size);
    Address get_address() const { return addr_; }
    unsigned get_size() const { return size_; }

 private:
    Address addr_;
    unsigned size_;
};

class process;

class SignalHandler;

class SignalGenerator;

class BPatch_process;

#define MAX_HANDLERS 64  /*This is just arbitrarily large-ish */
#define HANDLER_TRIM_THRESH 2 /*try to delete handlers if we have more than this */

class SignalHandler : public EventHandler<EventRecord>
{
  friend class SignalGenerator;
  friend class SignalGeneratorCommon;
  friend class SyncCallback;
  friend class process;
  int id;

  public:
    virtual ~SignalHandler();

  bool idle();
  bool waiting();
  bool processing();
  bool waitingForCallback();
  bool isActive(process *p);
  process *activeProcess() {return idle_flag ? NULL : active_proc;}
  CallbackBase *getCallback() {return wait_cb;}
  SignalGenerator *getSignalGenerator() { return sg; }

  protected:
    //  SignalHandler is only constructed by derived classes
  SignalHandler(char *name, int shid, SignalGenerator *sg_) : 
      EventHandler<EventRecord>(BPatch_eventLock::getLock(), 
                                name, 
                                false /*start thread?*/), 
          id(shid), sg(sg_),idle_flag(true),
          wait_flag(false), wait_cb(NULL), active_proc(NULL), 
          waitingForWakeup_(false) {
          waitLock = new eventLock();
      }
      
  SignalGenerator *sg;
  
  bool waitForEvent(pdvector<EventRecord> &events_to_handle);
  bool handleEvent(EventRecord &ev);

  //  SignalHandler::forwardSigToProcess()
  //  continue process with the (unhandled) signal
  bool forwardSigToProcess(EventRecord &ev, bool &continueHint);

  bool handleCritical(EventRecord &ev, bool &continueHint);
  bool handleProcessExit(EventRecord &ev, bool &continueHint);
  bool handleProcessExitPlat(EventRecord &ev, bool &continueHint);
  bool handleSingleStep(EventRecord &ev, bool &continueHint);
  bool handleLoadLibrary(EventRecord &ev, bool &continueHint);
  bool handleProcessStop(EventRecord &ev, bool &continueHint);
  bool notifyBPatchOfStop(EventRecord &ev, bool &continueHint);

  bool handleThreadCreate(EventRecord &ev, bool &continueHint);
  bool handleForkEntry(EventRecord &ev, bool &continueHint);
  bool handleForkExit(EventRecord &ev, bool &continueHint);
  bool handleLwpExit(EventRecord &ev, bool &continueHint);
  bool handleLwpAttach(EventRecord &ev, bool &continueHint);
  bool handleExecEntry(EventRecord &ev, bool &continueHint);
  bool handleExecExit(EventRecord &ev, bool &continueHint);
  bool handleSyscallEntry(EventRecord &ev, bool &continueHint);
  bool handleSyscallExit(EventRecord &ev, bool &continueHint);

  bool handleProcessCreate(EventRecord &ev, bool &continueHint);
  bool handleProcessAttach(EventRecord &ev, bool &continueHint);

  bool assignEvent(EventRecord &ev);
  pdvector<EventRecord> events_to_handle;

  bool idle_flag;
  bool wait_flag;

  CallbackBase *wait_cb;
  process *active_proc;

  static void flagBPatchStatusChange();
  static void setBPatchProcessSignal(BPatch_process *p, int t);

  // For us to block on
  eventLock *waitLock;
  bool waitingForWakeup_;

  void main();
  // Unused but must be defined
  bool waitNextEvent(EventRecord &);
};


#endif
