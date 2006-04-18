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

/* $Id: signalgenerator.h,v 1.6 2006/04/18 22:06:36 bernat Exp $
 */

#ifndef _SIGNAL_GENERATOR_H_
#define _SIGNAL_GENERATOR_H_

#include "common/h/Vector.h"
#include "common/h/String.h"
#include "dyninstAPI/src/EventHandler.h"
#include "codeRange.h"

class SignalHandler;
class EventGate;
class SignalGenerator;
class fileDescriptor;

// Various "do we want this to stop or run" requests; tri-state
// so that we can have "unset" as well as stop/run.
typedef enum { unsetRequest, stopRequest, runRequest, ignoreRequest } processRunState_t;

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

   // Setting the process to running, and whether to do into poll/waitpid, turns
   // out to be a slightly complex problem. We have a couple of inputs into the
   // decision:
   //   1) we can force entry into waitpid/poll, and not continue the process, 
   //       with a call to signalActiveProcess (AKA "we know it's really running"). 
   //   2) if there is a running signal handler, we _always_ wait for it to finish.
   //   3) BPatch can "asynchronously" stop or continue the process
   //   4) Completing signal handlers can stop or continue the process; currently,
   //     we either run it or leave it "as before" (and assume someone else is going
   //     to do something interesting). 

   // And the "just wake up and see what's going on" version
   bool signalActiveProcess();

   // Continue methods; we need to have agreement between the signalHandler
   // threads and the BPatch layer. The SH's use the async call, which basically says
   // "When everyone is done processing, then run the process". The blocking call
   // is for BPatch, and says "Return when the process is running".

   bool continueProcessAsync(int signalToContinueWith = -1, dyn_lwp *lwp = NULL);
   bool continueProcessBlocking(int signalToContinueWith = -1);

   bool pauseProcessAsync();
   bool pauseProcessBlocking();

   processRunState_t overrideSyncContinueState(processRunState_t state); 
   processRunState_t overrideAsyncContinueState(processRunState_t state); 

   void markProcessStop() { independentLwpStop_ = true; }
   void unmarkProcessStop() { independentLwpStop_ = false; }
 
   SignalHandler *findSHWithThreadID(unsigned long tid);
   SignalHandler *findSHWaitingForCallback(CallbackBase *cb);

   // Returns true if the signal generator is active, or if a child
   // signal handler is active. 
   bool isActivelyProcessing();

   // Subfunctions
   bool isDecoding() const { return decodingEvent_; }
   bool isDispatching() const { return dispatchingEvent_; }
   bool isWaitingForOS() const { return waitingForOS_; }

   bool wakeUpThreadForShutDown();

   int getPid() {return pid_;}

   void setProcess(process *p) {proc = p;}

   //  SignalGeneratorCommon should only be constructed via derived classes
   SignalGeneratorCommon(char *idstr);
   bool setupCreated(pdstring file, pdstring dir,
                     pdvector<pdstring> *argv,
                     pdvector<pdstring> *envp,
                     pdstring inputFile,
                     pdstring outputFile,
                     int stdin_fd, int stdout_fd,
                     int stderr_fd);
   bool setupAttached(pdstring file, int pid);

 protected:
   // Main loop functionality
   
   void main(); // Override from InternalThread - don't release lock

   void waitForActivation(); // Wait until process is marked running

   bool continueRequired();
   bool continueProcessInternal(dyn_lwp *contLWP = NULL);
   void setContinueSig(int signalToContinueWith);
   
   bool getEvents(pdvector<EventRecord> &events); // Fill in ev with raw data.
   virtual bool waitForEventsInternal(pdvector<EventRecord> &events) = 0; // OS-specific

   // We may need to mix 'n match events during decode
   virtual bool decodeEvents(pdvector<EventRecord> &events) = 0; // Decode ev to proper high-level info

   bool dispatchEvent(EventRecord &ev); // Dispatch
   bool assignOrCreateSignalHandler(EventRecord &ev); // Get someone to take care of this sig.

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
   
   virtual bool forkNewProcess() = 0;
   virtual bool attachProcess() = 0;
   virtual bool waitForStopInline() = 0;
   //  process creation parameters
   pdstring file_;
   pdstring dir_;

   pdstring inputFile_, outputFile_;
   int stdin_fd_, stdout_fd_, stderr_fd_;
   pdvector<pdstring> *argv_;
   pdvector<pdstring> *envp_;
   
   process *proc;
   pid_t pid_;
   int traceLink_;
   bool requested_wait_until_active;

   static pdstring createExecPath(pdstring &file, pdstring &dir);
   bool getExecFileDescriptor(pdstring filename,
                              int pid,
                              bool waitForTrap, // Should we wait for process init
                              int &status,
                              fileDescriptor &desc);
   
   bool waitingForActivation_;

   bool processPausedDuringOSWait_; 

   // If true, don't continue the process... delay continue until later.
   bool pendingDecode_;
   bool decodingEvent_;
   bool dispatchingEvent_;
   bool waitingForOS_;
   bool continueDesired_;

   // We barrier-continue; if there is a signal to use, it's stored in 
   // here until the actual continue is executed.
   int continueSig_;
   
   // The BPatch layer can (basically) asynchronously stop the process
   // without knowledge of the current signal handling state. We mark that
   // here.
   processRunState_t syncRunWhenFinished_;
   
   // And signal handlers... if any of them want a continue, this gets set
   // to true. For now, we're going with "if anyone wants a continue the process
   // gets continued". We may want "if anyone wants it stopped it stays stopped"...
   // not sure yet. 
   processRunState_t asyncRunWhenFinished_;

   // And sometimes we need to override everything - for example, attaching to 
   // a new process...
   bool activeProcessSignalled_;

   // Whee... we generally don't go into waitpid unless all of our lwps (independent
   // case) are running. However, if we're trying to stop the process, we need to call
   // waitpid until _none_ are running. 
   bool independentLwpStop_;

  // On platforms that depend on instrumentation to catch system call exits,
  // we may accidentally receive multiple "syscall exit" signals. In particular,
  // this causes problems on fork. We set this variable to true in the child and
  // use it to ignore further signals.
  bool childForkStopAlreadyReceived_;

   /////////////////
   // See above discussion (synchronizing with UI thread
   /////////////////
   void enterOSBlock();
   void exitOSBlock();

   virtual bool postSignalHandler();
#if defined(os_windows)
   bool SuspendThreadFromEvent(LPDEBUG_EVENT ev, dyn_lwp *lwp);
#endif

 private:
   
   bool initialize_event_handler();
   
   bool exitRequested();

   bool pauseAttemptedOnProcess();

   bool processIsPaused();
   
   bool haveLock() { return (eventlock->depth() > 0); }
   
   // eventLock *eventlock -- derived from parent class
   // stop_request -- derived from parent class

   // Other side: when we stop the process, we want to wait for this
   // to get back through the signalgenerator so we can be sure that
   // we don't race in process control. On Linux this is handled through
   // a highly complex loop; on /proc-based platforms, you can wait on 
   // this event lock.
   eventLock *waitlock;

   // Continuing is tough... we want agreement between everyone that
   // wants to continue the process.
   // 1) If the UI wants a stop, don't allow signal handlers to continue;
   // 2) If there is an event in decode, don't allow UI to continue.
   eventLock *activationLock;

   // Used for blocking continue requests
   eventLock *waitForContinueLock;

   ///////// Unused functions
   // Virtualized from parent class; we don't use them.
   bool waitNextEvent(EventRecord &);
   bool handleEvent(EventRecord &);

};

#if defined(os_windows)
#include "dyninstAPI/src/signalgenerator-winnt.h"
#else
#include "dyninstAPI/src/signalgenerator-unix.h"
#endif


#endif
