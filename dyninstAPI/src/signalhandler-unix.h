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

/* $Id: signalhandler-unix.h,v 1.21 2006/02/10 02:25:25 jaw Exp $
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

#define CAN_DUMP_CORE true
#define SLEEP_ON_MUTATEE_CRASH 300 /*seconds*/

#define INFO_TO_EXIT_CODE(info) info
#define INFO_TO_PID(info) info
// process exits do not cause poll events on alpha-osf, so we have a timeout
#if defined (os_osf)
#define POLL_TIMEOUT 1000 /*ms*/
#define POLL_FD get_fd()
#else
#define POLL_FD status_fd()
#define POLL_TIMEOUT -1
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

class SignalGenerator : public SignalGeneratorCommon
{
  friend class SignalHandler;
  friend class SignalGeneratorCommon;
  friend class process;

  public:
   virtual ~SignalGenerator() {}

   bool checkForExit(EventRecord &ev, bool block =false);
  private:
  //  SignalGenerator should only be constructed by process
  SignalGenerator(char *idstr, pdstring file, pdstring dir,
                      pdvector<pdstring> *argv,
                      pdvector<pdstring> *envp,
                      pdstring inputFile,
                      pdstring outputFile,
                      int stdin_fd, int stdout_fd,
                      int stderr_fd)
    : SignalGeneratorCommon(idstr, file, dir, argv, envp, inputFile, outputFile, 
                      stdin_fd, stdout_fd, stderr_fd) {}
  SignalGenerator(char *idstr, pdstring file, int pid);

  virtual SignalHandler *newSignalHandler(char *name, int id);

  virtual bool forkNewProcess();
  virtual bool attachProcess();
  virtual bool waitForStopInline();
  virtual bool waitNextEventLocked(EventRecord &ev);
  virtual bool decodeEvent(EventRecord &ev);

  //  decodeSignal_NP is called by decodeSignal before decodeSignal does any
  //  decoding.  It allows platform specific actions to be taken for signal
  //  decoding.  If it returns true, decodeSignal assumes that a valid decode 
  //  has taken place and it  will not do any further decoding.
  bool decodeSignal_NP(EventRecord &ev);
  bool decodeSignal(EventRecord &ev);
  bool decodeRTSignal(EventRecord &ev);
  bool decodeSigTrap(EventRecord &ev);
  bool decodeSigStopNInt(EventRecord &ev);
  bool decodeSigIll(EventRecord &ev);

  //  decodeSyscall changes the field ev.what from a platform specific
  //  syscall representation, eg, SYS_fork, to a platform indep. one,
  //  eg. procSysFork.  returns false if there is no available mapping.
  bool decodeSyscall(EventRecord &ev);

#if !defined (os_linux) 
   bool updateEvents(pdvector<EventRecord> &events, process *p, int lwp_to_use);
   bool decodeProcStatus(procProcStatus_t status, EventRecord &ev);
   bool updateEventsWithLwpStatus(process *curProc, dyn_lwp *lwp,
                                  pdvector<EventRecord> &events);
#endif

#if defined (os_linux)
   public:
   bool waitingForStop(process *p);
   bool notWaitingForStop(process *p);
   bool add_lwp_to_poll_list(dyn_lwp *lwp);
   bool remove_lwp_from_poll_list(int lwp_id);
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
   bool attachToChild(int pid);
   int find_dead_lwp();
   pid_t waitpid_kludge(pid_t, int *, int, int *);
   pdvector<pdstring> attached_lwp_ids;
#endif

};

/////////////////////
// Translation mechanisms
/////////////////////

inline bool didProcEnterSyscall(eventType t) {
   return (t == evtSyscallEntry);
}

inline bool didProcExitSyscall(eventType t) {
   return (t == evtSyscallExit);
}

inline bool didProcReceiveInstTrap(eventType t) {
    return (t == evtInstPointTrap);
}


#endif


