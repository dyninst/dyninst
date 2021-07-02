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

#if !defined(PCERRORS_H_)
#define PCERRORS_H_

// Only works on posix-compliant systems. IE not windows.
//#define PROCCTRL_PRINT_TIMINGS 1

// Will really change timings
//#define PROCCTRL_LOCK_PRINTS 1

#include <stdio.h>
#include <string.h>
#include "util.h"
#include "dyntypes.h"

PC_EXPORT extern void pc_print_lock();
PC_EXPORT extern void pc_print_unlock();
#if defined(PROCCTRL_LOCK_PRINTS)
#define PC_PRINT_LOCK pc_print_lock()
#define PC_PRINT_UNLOCK pc_print_unlock()
#else
#define PC_PRINT_LOCK
#define PC_PRINT_UNLOCK
#endif

#define pclean_printf(...)                                  \
   do {                                                     \
     if (dyninst_debug_proccontrol) {                       \
        PC_PRINT_LOCK;                                      \
        fprintf(pctrl_err_out, __VA_ARGS__);                \
        PC_PRINT_UNLOCK;                                    \
     }                                                      \
  } while (0)

#if defined(PROCCTRL_PRINT_TIMINGS)

#define pthrd_printf(...)                                               \
   do {                                                                 \
      if (dyninst_debug_proccontrol) {                                  \
         PC_PRINT_LOCK;                                                 \
         fprintf(pctrl_err_out, "[%s:%d-%s@%lu] - ", FILE__, __LINE__,  \
                 thrdName(), gettod());                                 \
         fprintf(pctrl_err_out, __VA_ARGS__);                           \
         PC_PRINT_UNLOCK;                                               \
      }                                                                 \
   } while (0)

#define perr_printf(...)                                                \
   do {                                                                 \
      if (dyninst_debug_proccontrol) {                                  \
         PC_PRINT_LOCK;                                                 \
         fprintf(pctrl_err_out, "[%s:%d-%s@%lu] - Error: ", FILE__, __LINE__, thrdName(), gettod()); \
         fprintf(pctrl_err_out, __VA_ARGS__);                           \
         PC_PRINT_UNLOCK;                                               \
      }                                                                 \
   } while (0)

#else

#define pthrd_printf(...)                                               \
   do {                                                                 \
      if (dyninst_debug_proccontrol) {                                  \
         PC_PRINT_LOCK;                                                 \
         fprintf(pctrl_err_out, "[%s:%d-%s] - ", FILE__, __LINE__, thrdName()); \
         fprintf(pctrl_err_out, __VA_ARGS__);                           \
         PC_PRINT_UNLOCK;                                               \
      }                                                                 \
   } while (0)

#define perr_printf(...)                                                \
   do {                                                                 \
      if (dyninst_debug_proccontrol) {                                  \
         PC_PRINT_LOCK;                                                 \
         fprintf(pctrl_err_out, "[%s:%d-%s] - Error: ", FILE__, __LINE__, thrdName()); \
         fprintf(pctrl_err_out, __VA_ARGS__);                           \
         PC_PRINT_UNLOCK;                                               \
      }                                                                 \
   } while (0)

#define parmerr_printf(...)                                             \
   do {                                                                 \
      if (dyninst_debug_proccontrol) {                                  \
         PC_PRINT_LOCK;                                                 \
         fprintf(pctrl_err_out, "[%s:%d-%s] - Error-ARM: ", FILE__, __LINE__, thrdName()); \
         fprintf(pctrl_err_out, __VA_ARGS__);                           \
         PC_PRINT_UNLOCK;                                               \
      }                                                                 \
   } while (0)

#endif

PC_EXPORT extern bool dyninst_debug_proccontrol;
PC_EXPORT extern const char *thrdName();
PC_EXPORT extern FILE* pctrl_err_out;

PC_EXPORT extern unsigned long gettod();

namespace Dyninst {
namespace ProcControlAPI {

typedef unsigned err_t;

const err_t err_none           = 0x0;
const err_t err_badparam       = 0x10000;
const err_t err_procread       = 0x10001;
const err_t err_internal       = 0x10002;
const err_t err_prem           = 0x10003;
const err_t err_noproc         = 0x10004;
const err_t err_interrupt      = 0x10005;
const err_t err_exited         = 0x10006;
const err_t err_nofile         = 0x10007;
const err_t err_unsupported    = 0x10008;
const err_t err_symtab         = 0x10009;
const err_t err_nothrd         = 0x1000a;
const err_t err_notstopped     = 0x1000b;
const err_t err_notrunning     = 0x1000c;
const err_t err_noevents       = 0x1000d;
const err_t err_incallback     = 0x1000e;
const err_t err_nouserthrd     = 0x1000f;
const err_t err_detached       = 0x10010;
const err_t err_attached       = 0x10011;
const err_t err_pendingirpcs   = 0x10012;
const err_t err_bpfull         = 0x10013;
const err_t err_notfound       = 0x10014;
const err_t err_dstack         = 0x10107;
const err_t err_eof            = 0x10108;

PC_EXPORT err_t getLastError();
PC_EXPORT void clearLastError();
PC_EXPORT const char* getLastErrorMsg();
PC_EXPORT void globalSetLastError(err_t err, const char *msg = NULL);
PC_EXPORT void setDebugChannel(FILE *f);
PC_EXPORT void setDebug(bool enable);
PC_EXPORT const char *getGenericErrorMsg(err_t e);
PC_EXPORT FILE *getDebugChannel();

}
}

#endif
