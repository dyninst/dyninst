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

/* $Id: signalhandler-winnt.h,v 1.9 2006/03/29 21:35:11 bernat Exp $
 */

/*
 * This file describes the entry points to the signal handling
 * routines. This is meant to provide a single interface to bother
 * the varied UNIX-style handlers and the NT debug event system.
 * Further platform-specific details can be found in the
 * signalhandler-unix.h and signalhandler-winnt.h files.
 */

#ifndef _SIGNAL_HANDLER_NT_H
#define _SIGNAL_HANDLER_NT_H

#include "common/h/headers.h"

#define CAN_DUMP_CORE false
#define SLEEP_ON_MUTATEE_CRASH 0 /*seconds*/
#define sleep Sleep

#define INFO_TO_EXIT_CODE(info) info.u.ExitProcess.dwExitCode
#define INFO_TO_PID(info) -1
class process;

/*
 * Enumerated types of "why" -- why we received a process event
 */

// NT types:
// procException = EXCEPTION_DEBUG_EVENT
// procThreadCreate = CREATE_THREAD_DEBUG_EVENT
// procProcessCreate = CREATE_PROCESS_DEBUG_EVENT
// procThreadExit = EXIT_THREAD_DEBUG_EVENT
// procProcessExit = EXIT_PROCESS_DEBUG_EVENT
// procDllLoad = LOAD_DLL_DEBUG_EVENT

typedef enum {
    procException,
    procThreadCreate,
    procProcessCreate,
    procThreadExit,
    procProcessExit,
    procProcessSelfTermination,  // caused by BPatch_thread::terminateExec...
    procDllLoad,
    procUndefined
} procSignalWhy_t;


/*
 * What:
 * procException: Exception type (breakpoint, illegal, access violation)
 */

typedef DWORD procSignalWhat_t;

/*
 * Info: any other necessary information. Here, the DEBUG_EVENT
 *       structure
 */

typedef DEBUG_EVENT procSignalInfo_t;

#endif


