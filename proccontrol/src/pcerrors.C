/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#include "proccontrol/h/PCErrors.h"

#include "common/h/dthread.h"

using namespace Dyninst;
using namespace ProcControlAPI;

static err_t last_error;
static const char *last_error_msg;
static signed long gen_thrd_id;
static signed long handler_thrd_id;
static signed long x_thrd_id;

FILE *pctrl_err_out;
bool dyninst_debug_proccontrol = false;

const char *thrdName()
{
   signed long self = DThread::self();
   if (self == gen_thrd_id)
      return "G";
   else if (self == handler_thrd_id) 
      return "H";
   else if (self == x_thrd_id)
      return "X";
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

void Dyninst::ProcControlAPI::setLastError(err_t err, const char *msg)
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

class init_debug_channel
{
public:
   init_debug_channel() 
   {
	  // fprintf(stderr, "Checking DYNINST_DEBUG_PROCCONTROL\n");
      pctrl_err_out = stderr;
	  char *debug = getenv("DYNINST_DEBUG_PROCCONTROL");
	  //fprintf(stderr, "debug is %s\n", debug ? debug : "<NULL>");
	  if (debug && (strcmp(debug, "1") == 0)) {
         setDebug(true);
      }
   }
};
static init_debug_channel idc;

