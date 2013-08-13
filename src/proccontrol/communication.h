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
#if !defined(COMMUNICATION_H_)
#define COMMUNICATION_H_

#define MAX_POSSIBLE_THREADS 512

#if defined(os_bg_test)
#define DEFAULT_NUM_THREADS 3
#else
#define DEFAULT_NUM_THREADS 8
#endif

#if defined(os_windows_test)
#include <winsock2.h>
#include <windows.h>
#include "external/stdint-win.h"
#include "external/inttypes-win.h"
typedef int pid_t;
#endif


#if defined(__cplusplus)
extern "C" {
#endif

#if defined(os_linux_test) || defined(os_bg_test) || defined(os_bgq_test)
#include <stdint.h>
#endif

#define SEND_PID_CODE 0xBEEF0001
typedef struct {
   uint32_t code;
   pid_t pid;
} send_pid;

#define HANDSHAKE_CODE 0xBEEF0002
typedef struct {
   uint32_t code;
} handshake;

#define ALLOWEXIT_CODE 0xBEEF0003
typedef struct {
   uint32_t code;
} allow_exit;

#define SENDADDR_CODE 0xBEEF0004
typedef struct {
  uint32_t code;
  uint32_t dummy;
  uint64_t addr;
} send_addr;

#define SYNCLOC_CODE 0xBEEF0005
typedef struct {
  uint32_t code;
} syncloc;

#define FORKINFO_CODE 0xBEEF0006
typedef struct {
   uint32_t code;
   uint32_t pid;
   uint32_t is_threaded;
   uint32_t is_done;
} forkinfo;

#define THREADINFO_CODE 0xBEEF0007
typedef struct {
   uint64_t code;
   uint64_t pid;
#if !defined(os_windows_test)
   uint64_t lwp;
#else
   HANDLE lwp;
#endif
   uint64_t tid;
   uint64_t a_stack_addr;
   uint64_t initial_func;
   uint64_t tls_addr;
} threadinfo;

#if defined(__cplusplus)
}
#endif

#endif
