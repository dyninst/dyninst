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
#include <stdio.h>
#include <stdlib.h>

#include "PCErrors.h"

#include "common/src/dthread.h"

using namespace Dyninst;
using namespace ProcControlAPI;

static err_t last_error;
static const char *last_error_msg;
static signed long gen_thrd_id;
static signed long handler_thrd_id;
static signed long x_thrd_id;
static signed long r_thrd_id;
static signed long w_thrd_id;

FILE *pctrl_err_out;
#if 0
bool dyninst_debug_proccontrol = true;
#else
bool dyninst_debug_proccontrol = false;
#endif
static Mutex<true> print_lock;

void pc_print_lock()
{
#if defined(PROCCTRL_LOCK_PRINTS)
   print_lock.lock();
#endif
}

void pc_print_unlock()
{
#if defined(PROCCTRL_LOCK_PRINTS)
   print_lock.unlock();
#endif
}

const char *thrdName()
{
   signed long self = DThread::self();
   if (self == gen_thrd_id)
      return "G";
   else if (self == handler_thrd_id)
      return "H";
   else if (self == x_thrd_id)
      return "X";
   else if (self == w_thrd_id)
      return "W";
   else if (self == r_thrd_id)
      return "R";
   else
      return "U";
}

#if defined(PROCCTRL_PRINT_TIMINGS)
#include <sys/time.h>
unsigned long gettod()
{
   static unsigned long long start = 0;
   static bool start_set = false;
   struct timeval val;
   int result = gettimeofday(&val, NULL);
   if (result == -1)
      return 0;
   unsigned long long t = (unsigned long long) ((val.tv_sec * 1000) + (val.tv_usec / 1000));
   if (!start_set) {
      start_set = true;
      start = t;
   }
   return (unsigned long) (t - start);
}
#endif

void setGeneratorThread(long t)
{
   gen_thrd_id = t;
}

void setHandlerThread(long t)
{
   handler_thrd_id = t;
}

void setXThread(long t)
{
   x_thrd_id = t;
}

void setRThread(long t)
{
   r_thrd_id = t;
}

void setWThread(long t)
{
   w_thrd_id = t;
}

bool isGeneratorThread() {
   return DThread::self() == gen_thrd_id;
}

bool isHandlerThread() {
   return DThread::self() == handler_thrd_id;
}

bool isUserThread() {
   return !isGeneratorThread() && !isHandlerThread();
}
err_t Dyninst::ProcControlAPI::getLastError()
{
   return last_error;
}

void Dyninst::ProcControlAPI::clearLastError()
{
   last_error = 0;
}

const char *Dyninst::ProcControlAPI::getLastErrorMsg()
{
   return last_error_msg;
}

void Dyninst::ProcControlAPI::globalSetLastError(err_t err, const char *msg)
{
   last_error = err;
   last_error_msg = msg;
}

void Dyninst::ProcControlAPI::setDebugChannel(FILE *f)
{
   pctrl_err_out = f;
}

void Dyninst::ProcControlAPI::setDebug(bool enable)
{
   dyninst_debug_proccontrol = enable;
}

#define STR_RET(C, S) case C: return S
const char *Dyninst::ProcControlAPI::getGenericErrorMsg(err_t e) {
   switch (e) {
      STR_RET(err_none, "None");
      STR_RET(err_badparam, "Bad Parameter");
      STR_RET(err_procread, "Bad Address");
      STR_RET(err_internal, "Internal Error");
      STR_RET(err_prem, "Premission Denied");
      STR_RET(err_noproc, "No such process");
      STR_RET(err_interrupt, "Operation Interrupted");
      STR_RET(err_exited, "Process or Thread is Exited");
      STR_RET(err_nofile, "No such file or directory");
      STR_RET(err_unsupported, "Unsupported feature on this platform");
      STR_RET(err_symtab, "Error during symbol table reading");
      STR_RET(err_nothrd, "No such thread");
      STR_RET(err_notstopped, "Process or Thread is not stopped");
      STR_RET(err_notrunning, "Process or Thread is not running");
      STR_RET(err_noevents, "No events were available to be handled");
      STR_RET(err_incallback, "Illegal operation issued from callback");
      STR_RET(err_nouserthrd, "User thread information is not avaiable");
      STR_RET(err_detached, "Process is detached");
      STR_RET(err_attached, "Process is already attached");
      STR_RET(err_pendingirpcs, "IRPCs are pending");
      default: return "Unknown";
   }
}

class init_debug_channel
{
public:
   init_debug_channel()
   {
      pctrl_err_out = stderr;
      char *debug = getenv("DYNINST_DEBUG_PROCCONTROL");
      if (debug && atoi(debug)) {
         setDebug(true);
      }
   }
};
static init_debug_channel idc;

