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

/* $Id: signalhandler-unix.h,v 1.12 2005/01/11 22:46:37 legendre Exp $
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

// Need procfs for /proc platforms
#if defined(mips_sgi_irix6_4)
#include <sys/procfs.h>
#elif defined(alpha_dec_osf4_0)
#include <sys/procfs.h>
#elif defined(sparc_sun_solaris2_4)
#include <procfs.h>
#endif
// AIX will include sys/procfs.h

#include "common/h/Types.h"

class process;

/*
 * Enumerated types of "why" -- why we received a process event
 */

// Global list: from /proc and waitpid both
// Process exited normally (WIFEXITED)
// Process exited on a signal (WIFSIGNALED)
// Process was signalled (WIFSTOPPED/PR_SIGNALLED)
// Process entering traced syscall (PR_SYSENTRY)
// Process exiting traced syscall (PR_SYSEXIT)
// Process stopped via request (PR_REQUESTED)
// PR_FAULTED, PR_JOBCONTROL, and PR_SUSPENDED are not caught

typedef enum {
    procExitedNormally,
    procExitedViaSignal,
    procSignalled,
    procSyscallEntry,
    procSyscallExit,
    procRequested,
    procSuspended,
    procInstPointTrap,
    procForkSigChild,
    procUndefined
} procSignalWhy_t;

/*
 * What:
 *  procExited: exit code
 *  procExitedViaSignal: uncaught signal
 *  procSignalled: signal
 *  procSyscallEntry: system call
 *  procSyscallExit: system call
 *  procRequested: not defined
 */

typedef unsigned int procSignalWhat_t;

/* On /proc platforms we have predefined system call mappings (SYS_fork, etc).
   Define them here for platforms which don't have them */
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

/*
 * Info parameter: return value, address, etc.
 * May be augmented by a vector of active frames for
 *   more efficient signal handling, or library information.
 */

typedef Address procSignalInfo_t;

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
    procSysOther
} procSyscall_t;

#include "dyninstAPI/src/signalhandler-event.h"

procSyscall_t decodeSyscall(process *p, procSignalWhat_t syscall);

// waitPid status -> what/why format
typedef int procWaitpidStatus_t;
int decodeWaitPidStatus(process *proc,
                        procWaitpidStatus_t status,
                        procSignalWhy_t *why,
                        procSignalWhat_t *what);

// proc decode
// There are two possible types of data structures:
#if defined(sparc_sun_solaris2_4)
typedef lwpstatus_t procProcStatus_t;
#elif defined(mips_sgi_irix6_4) || defined(alpha_dec_osf4_0)
typedef prstatus_t procProcStatus_t;
#else
// No /proc, dummy function
typedef int procProcStatus_t;
#endif

int decodeProcStatus(process *proc,
                     procProcStatus_t status,
                     procSignalWhy_t &why,
                     procSignalWhat_t &what,
                     procSignalInfo_t &retval);

///////////////////////////////////////////////////////////////////
////////// Handler section
///////////////////////////////////////////////////////////////////

// These are the prototypes for the UNIX-style signal handler

// Note: these functions all have an int return type, which is generally
// used to indicate whether the signal was consumed (>0 return) or still
// needs to be handled (0 return)

// forwardSigToProcess: continue the process with the (unhandled) signal
int forwardSigToProcess(const procevent &event);


/////////////////////
// Handle individual signal types
/////////////////////

int handleSigTrap(const procevent &event);

int handleSigStopNInt(const procevent &event);

// A signal where we may want to dump proc core/debug it
int handleSigCritical(const procevent &event);

// And the dispatcher
int handleSignal(const procevent &event);


/////////////////////
// Handle syscall entries
/////////////////////

int handleForkEntry(const procevent &event);
int handleExecEntry(const procevent &event);

// And the dispatcher
int handleSyscallEntry(const procevent &event);


/////////////////////
// Handle syscall exits
/////////////////////

int handleForkExit(const procevent &event);
int handleExecExit(const procevent &event);
int handleLoadExit(const procevent &event);

// And the dispatcher
int handleSyscallExit(const procevent &event);

/////////////////////
// Translation mechanisms
/////////////////////

inline bool didProcReceiveSignal(procSignalWhy_t why) {
    return (why == procSignalled); 
}

inline bool didProcReceiveInstTrap(procSignalWhy_t why) {
    return (why == procInstPointTrap);
}

inline bool didProcExit(procSignalWhy_t why) {
    return (why == procExitedNormally);
}

inline bool didProcExitOnSignal(procSignalWhy_t why) {
    return (why == procExitedViaSignal);
}

#endif


