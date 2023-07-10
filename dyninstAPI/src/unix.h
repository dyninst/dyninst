/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#define POLL_FD status_fd()
#define POLL_TIMEOUT -1

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

#define SYSSET_MAP(x, pid)  (x)

typedef unsigned long eventInfo_t;
typedef void * eventMoreInfo_t;
typedef int eventWhat_t;
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

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#define PDSOCKET_ERRNO errno
#define INVALID_PDSOCKET (-1)
#define SOCKET_TYPE PF_UNIX
#define THREAD_RETURN void *
#define DO_THREAD_RETURN return NULL

#define SOCKLEN_T socklen_t

#include "common/h/dyntypes.h"

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE -1
#endif

// Hybrid Analysis Compatibility definitions
#define PAGE_READ 1
#define PAGE_WRITE 2
#define PAGE_EXECUTE 4
#define PAGE_READONLY PAGE_READ
#define PAGE_READWRITE (PAGE_READ | PAGE_WRITE)
#define PAGE_EXECUTE_READ (PAGE_READ | PAGE_EXECUTE)
#define PAGE_EXECUTE_READWRITE (PAGE_READ | PAGE_EXECUTE | PAGE_WRITE)

#endif
