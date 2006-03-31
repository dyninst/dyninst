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

/* $Id: signalgenerator.h,v 1.2 2006/03/31 02:09:31 bernat Exp $
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
   bool signalPausedProcess();

   SignalHandler *findSHWithThreadID(unsigned long tid);
   SignalHandler *findSHWaitingForCallback(CallbackBase *cb);

   // Returns true if the signal generator is active, or if a child
   // signal handler is active. 
   bool isActivelyProcessing();

   // Subfunctions
   bool isDecoding() const { return decodingEvent_; }
   bool isDispatching() const { return dispatchingEvent_; }

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

   /////////////////////////////
   // If the SignalGenerator is in poll() (or waitpid, theoretically) and
   // the UI thread attempts a pause, we need to wait for the stop to be
   // received and handled - otherwise it is possible to get a signal handler
   // running in parallel with UI operations (since the UI thread doesn't need
   // to keep the global lock). We handle this by using an eventLock around the
   // call to poll that can be waited on by the UI. These are functions to wrap
   // this call. 

   bool isBlockedInOS() const;
   bool waitForOSReturn();


 protected:
   // Main loop functionality
   
   void main(); // Override from InternalThread - don't release lock

   void waitForActiveProcess(); // Wait until process is marked running
   
   bool getEvent(EventRecord &ev); // Fill in ev with raw data.
   virtual bool waitForEventInternal(EventRecord &ev) = 0; // OS-specific

   virtual bool decodeEvent(EventRecord &ev) = 0; // Decode ev to proper high-level info

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
   pdvector<EventRecord> events_to_handle;
   
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
   
   static pdstring createExecPath(pdstring &file, pdstring &dir);
   bool getExecFileDescriptor(pdstring filename,
                              int pid,
                              bool waitForTrap, // Should we wait for process init
                              int &status,
                              fileDescriptor &desc);
   
   bool waitingForActiveProcess_;

   bool processPausedDuringOSWait_; 
   
   bool decodingEvent_;
   bool dispatchingEvent_;
   bool waitingForOS_;
   
   /////////////////
   // See above discussion (synchronizing with UI thread
   /////////////////
   void enterOSBlock();
   void exitOSBlock();
   
 private:
   
   bool initialize_event_handler();
   
   bool exitRequested(EventRecord &ev);

   bool pauseAttemptedOnProcess();

   bool processIsPaused();
   
   bool haveLock() { return (eventlock->depth() > 0); }
   
   // eventLock *eventlock -- derived from parent class
   // stop_request -- derived from parent class

   // If the process is paused, we don't want to waitpid/poll on it. 
   // We create a new lock here that we can block on and that can
   // be signalled by others. Eventually we can stop using the global
   // lock (eventlock, above) -- however, that's being kept around to
   // minimize changes.
   eventLock *runlock;


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
