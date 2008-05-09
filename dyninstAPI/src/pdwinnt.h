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

// $Id: pdwinnt.h,v 1.21 2008/05/09 00:25:38 jaw Exp $

#if !defined(PDWINNT_H)
#define PDWINNT_H

#if !defined(i386_unknown_nt4_0) && !(defined mips_unknown_ce2_11) //ccw 20 july 2000 : 29 mar 2001
#error "invalid architecture-os inclusion"
#endif

#ifndef PDWINNT_HDR
#define PDWINNT_HDR
#include "common/h/headers.h"
#include "dyninstAPI/src/w32CONTEXT.h" //ccw 30 mar 2001

typedef HANDLE handleT;

struct dyn_saved_regs {
    w32CONTEXT cont;
};

struct EXCEPTION_REGISTRATION {
    EXCEPTION_REGISTRATION *prev;
    Address handler;
};

#define EXIT_NAME "_exit"
#define SIGNAL_HANDLER "no_signal_handler"
#endif

// Number of bytes to save in an overwrite operation
#define BYTES_TO_SAVE 256

#define CAN_DUMP_CORE false
#define SLEEP_ON_MUTATEE_CRASH 0 /*seconds*/
#define sleep Sleep

#define INFO_TO_EXIT_CODE(info) info.u.ExitProcess.dwExitCode
#define INFO_TO_ADDRESS(info) info.u.Exception.ExceptionRecord.ExceptionAddress
#define INFO_TO_PID(info) -1

typedef DEBUG_EVENT eventInfo_t;
typedef DWORD eventWhat_t;
typedef void * eventMoreInfo_t;
#define THREAD_RETURN void
#define DO_THREAD_RETURN return

typedef void (*thread_main_t)(void *);
typedef unsigned long internal_thread_t;

#define VSNPRINTF _vsnprintf
#define SNPRINTF _snprinf

#define INDEPENDENT_LWP_CONTROL true

typedef CRITICAL_SECTION EventLock_t;
typedef HANDLE EventCond_t;

#define ssize_t int
#define DYNINST_ASYNC_PORT 28003
#define PDSOCKET_ERRNO WSAGetLastError()
#define INVALID_PDSOCKET (INVALID_SOCKET)
#define SOCKET_TYPE PF_INET
#define THREAD_RETURN void
#define DO_THREAD_RETURN return
#define SOCKLEN_T unsigned int


/* We don't compile with gcc on Windows.  *sigh*  This will be slower,
   but should be functionally identical. */
#include <set>
#include <vector>

#endif /* PDWINNT_H */
