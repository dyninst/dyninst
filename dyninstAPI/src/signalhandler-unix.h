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

/* $Id: signalhandler-unix.h,v 1.17 2005/11/21 17:16:14 jaw Exp $
 */

/*
 * This file describes the entry points to the signal handling
 * routines. This is meant to provide a single interface to bother
 * the varied UNIX-style handlers and the NT debug event system.
 * Further platform-specific details can be found in the
 * signalhandler-unix.h and signalhandler-winnt.h files.
 */

#ifndef _SIGNAL_HANDLER_UNIX_H
#define _SIGNAL_HANDLER_UNIX_H

#include "common/h/Types.h"

// Need procfs for /proc platforms
#if defined(alpha_dec_osf4_0)
#include <sys/procfs.h>
#elif defined(sparc_sun_solaris2_4)
#include <procfs.h>
#elif defined(os_aix)
#include <sys/procfs.h>
#endif

class process;


//  On /proc platforms we have predefined system call mappings (SYS_fork, etc).
//  Define them here for platforms which don't have them 

#if !defined(SYS_fork)
#define SYS_fork 1001
#endif
#if !defined(SYS_exec)
#define SYS_exec 1002
#endif
#if !defined(SYS_exit)
#define SYS_exit 1003
#endif
#if !defined(SYS_load)
#define SYS_load 1004
#endif
#if !defined(SYS_execve)
#define SYS_execve 1005
#endif
#if !defined(SYS_fork1)
#define SYS_fork1 1006
#endif
#if !defined(SYS_vfork)
#define SYS_vfork 1007
#endif
#if !defined(SYS_execv)
#define SYS_execv 1008
#endif
#if !defined(SYS_lwp_exit)
#define SYS_lwp_exit 1009
#endif

/*
 * Info parameter: return value, address, etc.
 * May be augmented by a vector of active frames for
 *   more efficient signal handling, or library information.
 */


///////////////////////////////////////////////////////////////////
////////// Decoder section
///////////////////////////////////////////////////////////////////

// These functions provide a platform-independent decoder layer.

// Enumerated types of system calls we have particular
// reponses for. Used to convert a large if-then tree
// to a switch statement.
typedef enum {
    procSysFork,
    procSysExec,
    procSysExit,
    // Library load "syscall". Used by AIX.
    procSysLoad,
    procLwpExit,
    procSysOther
} procSyscall_t;


procSyscall_t decodeSyscall(process *p, eventWhat_t info);

// waitPid status -> what/why format
typedef int procWaitpidStatus_t;
bool decodeWaitPidStatus(procWaitpidStatus_t status, EventRecord &ev);

// proc decode
// There are two possible types of data structures:
#if defined(sparc_sun_solaris2_4) || defined (os_aix)
typedef lwpstatus_t procProcStatus_t;
#elif defined(mips_sgi_irix6_4) || defined(alpha_dec_osf4_0)
typedef prstatus_t procProcStatus_t;
#else
// No /proc, dummy function
typedef int procProcStatus_t;
#endif

///////////////////////////////////////////////////////////////////
////////// Handler section
///////////////////////////////////////////////////////////////////

// These are the prototypes for the UNIX-style signal handler

// Note: these functions all have an int return type, which is generally
// used to indicate whether the signal was consumed (>0 return) or still
// needs to be handled (0 return)

/////////////////////
// Handle individual signal types
/////////////////////

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

class SignalGeneratorUnix : public SignalGenerator
{
  friend SignalGeneratorUnix *getSH();
  friend class SignalHandlerUnix;
  friend class process;
  friend process *ll_attachToCreatedProcess(int, const pdstring &);

  public:
   virtual ~SignalGeneratorUnix() {}

  private:
  //  SignalGenerator should only be constructed via getSH()
  SignalGeneratorUnix() : SignalGenerator() {}
  SignalHandler *newSignalHandler(char *name, int id);

  bool waitNextEvent(EventRecord &ev);

  bool decodeEvent(EventRecord &ev);
  bool decodeRTSignal(EventRecord &ev);
  bool decodeSigTrap(EventRecord &ev);
  bool decodeSigStopNInt(EventRecord &ev);
  bool decodeSigIll(EventRecord &ev);

  //  functions specific to the unix polling mechanism.
  bool createPollEvent(pdvector<EventRecord> &events, struct pollfd fds, process *curProc);
  bool getFDsForPoll(pdvector<unsigned int> &fds);
  process *findProcessByFD(unsigned int fd);

#if !defined (os_linux) 
   bool updateEvents(pdvector<EventRecord> &events, process *p, int lwp_to_use);
   bool decodeProcStatus(process *p, procProcStatus_t status, EventRecord &ev);
   bool updateEventsWithLwpStatus(process *curProc, dyn_lwp *lwp,
                                  pdvector<EventRecord> &events);
#endif

#if defined (os_linux)
   public:
   bool waitingForStop(process *p);
   bool notWaitingForStop(process *p);
   private:
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

class SignalHandlerUnix : public SignalHandler
{
  friend class SignalGeneratorUnix;
  friend class process;
  friend process *ll_attachToCreatedProcess(int, const pdstring &);
  public:

  private:
  //  SignalHandler should only be constructed by SignalGenerator
  SignalHandlerUnix(char *shname, int shid) : SignalHandler(shname, shid) {}
  virtual ~SignalHandlerUnix() {}

  bool handleEvent(EventRecord &);
  bool handleEventLocked(EventRecord &);
  bool handleSigStopNInt(EventRecord &event);
  bool handleSigCritical(EventRecord &event);
  procSyscall_t decodeSyscall(process *p, eventWhat_t what);
  bool handleSignal(EventRecord &ev);
  bool handleSIGCHLD(EventRecord &ev);
  static bool handleSigTrap(EventRecord &ev);

  bool handleSIGSTOP(EventRecord &ev);
  bool handleForkEntry(EventRecord &ev);
  bool handleExecEntry(EventRecord &ev);
  bool handleLwpExit(EventRecord &ev);
  bool handleSyscallEntry(EventRecord &ev);
  bool handleSyscallExit(EventRecord &ev);
  bool handleForkExit(EventRecord &ev);
  static bool handleExecExit(EventRecord &ev);
  bool handleLoadExit(EventRecord &ev);
  bool handleSingleStep(const EventRecord &ev);
  static bool handleProcessCreate(EventRecord &ev);


  // forwardSigToProcess: continue the process with the (unhandled) signal
  static int forwardSigToProcess(EventRecord &ev);
};
/////////////////////
// Translation mechanisms
/////////////////////

inline bool didProcReceiveSignal(eventType t) {
    return (t == evtSignalled); 
}

inline bool didProcEnterSyscall(eventType t) {
   return (t == evtSyscallEntry);
}

inline bool didProcExitSyscall(eventType t) {
   return (t == evtSyscallExit);
}

inline bool didProcReceiveInstTrap(eventType t) {
    return (t == evtInstPointTrap);
}

inline bool didProcExit(EventRecord &ev) {
    return ((ev.type == evtProcessExit) && (ev.status == statusNormal));
}

#endif


