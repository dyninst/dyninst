/*
 * Copyright (c) 1996-2002 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

/* $Id: signalhandler.h,v 1.1 2003/03/08 02:13:30 bernat Exp $
 */

#ifndef _SIGNAL_HANDLER_H
#define _SIGNAL_HANDLER_H

#if defined(mips_sgi_irix6_4)
#include <sys/procfs.h>
#elif defined(alpha_dec_osf4_0)
#include <sys/procfs.h>
#elif defined(sparc_sun_solaris2_4)
#include <procfs.h>
#endif
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
    // NT-o-rama
    procThreadCreate,
    procProcessCreate,
    procThreadExit,
    procDllLoad,
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


// Enumerated types of system calls we have particular
// reponses for. Used to convert a large if-then tree
// to a switch statement.
typedef enum {
    procSysFork,
    procSysExec,
    procSysExit,
    procSysLoad,
    procSysOther
} procSyscall_t;

procSyscall_t decodeSyscall(process *p, procSignalWhat_t syscall);


// Functions which do things to signals

// The new checkProcStatus()
// Does all work internally. 
void decodeAndHandleProcessEvent(bool block);

// decodeProcessEvent: check whether there is an event
// on any process we're debugging. If one is found decode
// it and return
// Return code:
// 0: no event
// 1: event fount

// pid: -1 for all processes
// why: Why the process stopped (return parameter)
// what: what caused the stop (return parameter)
// block: block waiting for a signal?
// waitProcs replacement

process *decodeProcessEvent(int pid,
                            procSignalWhy_t &why,
                            procSignalWhat_t &what,
                            int &retval,
                            bool block);

// waitPid status -> what/why format
typedef int waitPidStatus_t;
int decodeWaitPidStatus(process *proc,
                        waitPidStatus_t status,
                        procSignalWhy_t &why,
                        procSignalWhat_t &what);

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
                     int &retval);


// Takes the data above and performs whatever handling is necessary
int handleProcessEvent(process *proc,
                       procSignalWhy_t why,
                       procSignalWhat_t what,
                       int retval);

#endif


