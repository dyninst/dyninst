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

// $Id: pdwinnt.h,v 1.21 2008/05/09 00:25:38 jaw Exp $

#if !defined(PDWINNT_H)
#define PDWINNT_H

#if !defined(i386_unknown_nt4_0)
#error "invalid architecture-os inclusion"
#endif

#ifndef PDWINNT_HDR
#define PDWINNT_HDR
#include "common/src/headers.h"

typedef HANDLE handleT;

struct dyn_saved_regs {
    CONTEXT cont;
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

static const auto sleep = Sleep;

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
