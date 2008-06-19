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

/* $Id: unix.h,v 1.6 2008/06/19 19:53:52 legendre Exp $
 */

#ifndef _UNIX_H_
#define _UNIX_H_

#define CAN_DUMP_CORE true
#define SLEEP_ON_MUTATEE_CRASH 300 /*seconds*/

#define INFO_TO_EXIT_CODE(info) info
#define INFO_TO_PID(info) info
#define INFO_TO_ADDRESS(info) (Address) 0

// process exits do not cause poll events on alpha-osf, so we have a timeout
#if defined (os_osf)
#define POLL_TIMEOUT 1000 /*ms*/
#define POLL_FD get_fd()
#else
#define POLL_FD status_fd()
#define POLL_TIMEOUT -1
#endif

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

typedef int procWaitpidStatus_t;
class EventRecord;
bool decodeWaitPidStatus(procWaitpidStatus_t status, EventRecord &ev);

#if defined (os_aix) && defined(cap_proc)
extern int SYSSET_MAP(int, int);
#else
#define SYSSET_MAP(x, pid)  (x)
#endif

typedef unsigned long eventInfo_t;
typedef void * eventMoreInfo_t;
typedef unsigned int eventWhat_t;
#define THREAD_RETURN void *
#define DO_THREAD_RETURN return NULL

typedef void *(*thread_main_t)(void *);
typedef pthread_t internal_thread_t;

#define VSNPRINTF vsnprintf
#define SNPRINTF snprinf

typedef pthread_mutex_t EventLock_t;
typedef pthread_cond_t EventCond_t;

#if defined(os_linux) 
#define PTHREAD_MUTEX_TYPE PTHREAD_MUTEX_RECURSIVE_NP
#define STRERROR_BUFSIZE 512
#define ERROR_BUFFER char buf[STRERROR_BUFSIZE]
#define STRERROR(x,y) strerror_r(x,y,STRERROR_BUFSIZE)
#else
#define ERROR_BUFFER
#define PTHREAD_MUTEX_TYPE PTHREAD_MUTEX_RECURSIVE
#define STRERROR_BUFSIZE 0
#define STRERROR(x,y) strerror(x)
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#define PDSOCKET_ERRNO errno
#define INVALID_PDSOCKET (-1)
#define SOCKET_TYPE PF_UNIX
#define THREAD_RETURN void *
#define DO_THREAD_RETURN return NULL

#if defined(os_osf)
#define SOCKLEN_T size_t 
#else
#define SOCKLEN_T socklen_t 
#endif

#include "dynutil/h/dyntypes.h"

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE -1
#endif

#endif


