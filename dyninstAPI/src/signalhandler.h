/*
 * Copyright (c) 1996-2002 Barton P. Miller
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

/* $Id: signalhandler.h,v 1.7 2003/12/18 17:15:35 schendel Exp $
 */

/*
 * This file describes the entry points to the signal handling
 * routines. This is meant to provide a single interface to bother
 * the varied UNIX-style handlers and the NT debug event system.
 * Further platform-specific details can be found in the
 * signalhandler-unix.h and signalhandler-winnt.h files.
 */

#ifndef _SIGNAL_HANDLER_H
#define _SIGNAL_HANDLER_H

#if defined(i386_unknown_nt4_0)
#include "dyninstAPI/src/signalhandler-winnt.h"
#else
#include "dyninstAPI/src/signalhandler-unix.h"
#endif

class process;
class dyn_lwp;

/* Included from the unix/NT file:
 * procSignalWhy_t: What event
 * procSignalWhat_t: Which signal in particular
 * procSignalInfo_t: Any information required by the handler
 */

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

process *decodeProcessEvent(dyn_lwp **pertinantLWP, int wait_arg, 
                            procSignalWhy_t &why, procSignalWhat_t &what,
                            procSignalInfo_t &info, bool block);

// Takes the data above and performs whatever handling is necessary
int handleProcessEvent(process *proc,
                       dyn_lwp *relevantLWP,
                       procSignalWhy_t why,
                       procSignalWhat_t what,
                       procSignalInfo_t info);

/////////////////////
// Callbacks.
/////////////////////

// Note: Replace with BPatch callbacks when Paradyn can use them
// Fork entry
typedef void (*forkEntryCallback_t)(process *p, void *data);
// Fork exit: include new pid
typedef void (*forkExitCallback_t)(process *p, void *data, process *child);
// Exec entry: include program argument
typedef void (*execEntryCallback_t)(process *p, void *data, char *arg0);
// Exec exit
typedef void (*execExitCallback_t)(process *p, void *data);
// Exit entry: include exit code
typedef void (*exitEntryCallback_t)(process *p, void *data, int code);


#endif


