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

/* $Id: signalhandler.h,v 1.21 2006/03/12 23:32:18 legendre Exp $
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
    EventRecord trigger;
    bool waiting;
};

class SignalHandler;
class SignalGenerator;
#define MAX_HANDLERS 64  /*This is just arbitrarily large-ish */
#define HANDLER_TRIM_THRESH 2 /*try to delete handlers if we have more than this */

class SignalGeneratorCommon : public EventHandler<EventRecord> {
 friend class process;
 friend class SignalHandler;

 public:
   static process *newProcess(pdstring file_, pdstring dir, 
                                            pdvector<pdstring> *argv,
                                            pdvector<pdstring> *envp,
                                            int stdin_fd, int stdout_fd, 
                                            int stderr_fd);
   static process *newProcess(pdstring &progpath, int pid);
   process *newProcess(process *parent, int pid_, int traceLink);

   static void deleteSignalGenerator(SignalGenerator *sg);
   static void stopSignalGenerator(SignalGenerator *sg);

    //  newSignalGenerator provides a way to make a signal generator in
    //  a platform indep manner.
    static SignalGenerator *newSignalGenerator(pdstring file, pdstring dir,
                                                pdvector<pdstring> *argv,
                                                pdvector<pdstring> *envp,
                                                pdstring inputFile,
                                                pdstring outputFile,
                                                int stdin_fd, int stdout_fd,
                                                int stderr_fd);
    static SignalGenerator *newSignalGenerator(pdstring file, int pid);

   virtual ~SignalGeneratorCommon(); 
   
   eventType waitForOneOf(pdvector<eventType> &evts);
   static eventType globalWaitForOneOf(pdvector<eventType> &evts);
   eventType waitForEvent(eventType evt, process *p = NULL, dyn_lwp *lwp = NULL, 
                          eventStatusCode_t status = NULL_STATUS_INITIALIZER);
   bool signalEvent(EventRecord &ev);
   bool signalEvent(eventType t);

   bool signalActiveProcess();

   SignalHandler *findSHWithThreadID(unsigned long tid);
   SignalHandler *findSHWaitingForCallback(CallbackBase *cb);
   bool activeHandlerForProcess(process *p);
   bool anyActiveHandlers();
   bool wakeUpThreadForShutDown();
   bool waitingForActiveProcess() {return waiting_for_active_process;}
   int getPid() {return pid;}

   void setProcess(process *p) {proc = p;}

   pdstring file;
   pdstring dir;
   //  SignalGeneratorCommon should only be constructed via derived classes
   SignalGeneratorCommon(char *idstr, pdstring file, pdstring dir,
                         pdvector<pdstring> *argv,
                         pdvector<pdstring> *envp,
                         pdstring inputFile,
                         pdstring outputFile,
                         int stdin_fd, int stdout_fd,
                         int stderr_fd);
   SignalGeneratorCommon(char *idstr, pdstring file, int pid);

   protected:
    bool handleEvent(EventRecord &ev)
      { LOCK_FUNCTION(bool, handleEventLocked, (ev));}
    bool handleEventLocked(EventRecord &ev);
    bool waitNextEvent(EventRecord &ev);
    virtual bool waitNextEventLocked(EventRecord &ev) = 0;

    virtual bool decodeEvent(EventRecord &ev) = 0;
    bool decodeIfDueToProcessStartup(EventRecord &ev);

    virtual SignalHandler *newSignalHandler(char *name, int shid)  = 0;
    void deleteSignalHandler(SignalHandler *sh);

    // A list of things waiting on this particular signal handler...
    pdvector<EventGate *> wait_list;
    bool waiting_for_active_process;

    // And for all signal handlers.
    static pdvector<EventGate *> global_wait_list;
    static eventLock global_wait_list_lock;


    pdvector<SignalHandler *> handlers;
    pdvector<EventRecord> events_to_handle;

    virtual bool forkNewProcess() = 0;
    virtual bool attachProcess() = 0;
    virtual bool waitForStopInline() = 0;
    //  process creation parameters
    pdstring inputFile, outputFile;
    int stdin_fd, stdout_fd, stderr_fd;
    pdvector<pdstring> *argv;
    pdvector<pdstring> *envp;

    process *proc;
    pid_t pid;
    int traceLink;

    static pdstring createExecPath(pdstring &file, pdstring &dir);
    bool getExecFileDescriptor(pdstring filename,
                               int pid,
                               bool waitForTrap, // Should we wait for process init
                               int &status,
                               fileDescriptor &desc);

    bool waiting_for_event;
   private:

    bool initialize_event_handler();
};


class SignalHandler : public EventHandler<EventRecord>
{
  friend class SignalGenerator;
  friend class SignalGeneratorCommon;
  friend class SyncCallback;
  int id;

  public:
    virtual ~SignalHandler();

  bool idle();
  bool waiting();
  bool isActive(process *p);
  process *activeProcess() {return idle_flag ? NULL : active_proc;}
  CallbackBase *waitingForCallback() {return wait_cb;}
  protected:
    //  SignalHandler is only constructed by derived classes
    SignalHandler(char *name, int shid, SignalGenerator *sg_) : 
                  EventHandler<EventRecord>(global_mutex, 
                  name, false /*start thread?*/), id(shid), sg(sg_),idle_flag(true),
                  wait_flag(false), wait_cb(NULL), active_proc(NULL) { }
    SignalGenerator *sg;
    
  bool handleEvent(EventRecord &ev);
  bool handleEventLocked(EventRecord &ev);

  bool waitNextEvent(EventRecord &ev);

  //  SignalHandler::forwardSigToProcess()
  //  continue process with the (unhandled) signal
  static bool forwardSigToProcess(EventRecord &ev);

  bool handleCritical(EventRecord &ev);
  bool handleProcessExit(EventRecord &ev);
  bool handleProcessExitPlat(EventRecord &ev);
  bool handleSingleStep(EventRecord &ev);
  bool handleLoadLibrary(EventRecord &ev);
  bool handleProcessStop(EventRecord &ev);

  bool handleThreadCreate(EventRecord &ev);
  bool handleForkEntry(EventRecord &ev);
  bool handleForkExit(EventRecord &ev);
  bool handleLwpExit(EventRecord &ev);
  bool handleExecEntry(EventRecord &ev);
  bool handleExecExit(EventRecord &ev);
  bool handleSyscallEntry(EventRecord &ev);
  bool handleSyscallExit(EventRecord &ev);

  bool handleProcessCreate(EventRecord &ev);
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
