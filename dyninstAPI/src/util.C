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

/* $Id: util.C,v 1.36 2008/06/19 22:13:43 jaw Exp $
 * util.C - support functions.
 */

#include "common/src/headers.h"
#include "dyninstAPI/src/util.h"
#include "debug.h"

using namespace Dyninst;
CntStatistic trampBytes;
CntStatistic pointsUsed;
CntStatistic insnGenerated;
CntStatistic totalMiniTramps;
double timeCostLastChanged=0;
// HTable<resourceListRec*> fociUsed;
// HTable<metric*> metricsUsed;
CntStatistic ptraceOtherOps, ptraceOps, ptraceBytes;

void printDyninstStats()
{
   sprintf(errorLine, "    %ld total points used\n", pointsUsed.value());
   logLine(errorLine);
   sprintf(errorLine, "    %ld mini-tramps used\n", totalMiniTramps.value());
   logLine(errorLine);
   sprintf(errorLine, "    %ld tramp bytes\n", trampBytes.value());
   logLine(errorLine);
   sprintf(errorLine, "    %ld ptrace other calls\n", ptraceOtherOps.value());
   logLine(errorLine);
   sprintf(errorLine, "    %ld ptrace write calls\n",
         ptraceOps.value()-ptraceOtherOps.value());
   logLine(errorLine);
   sprintf(errorLine, "    %ld ptrace bytes written\n", ptraceBytes.value());
   logLine(errorLine);
   sprintf(errorLine, "    %ld instructions generated\n",
         insnGenerated.value());
   logLine(errorLine);
}

// TIMING code

#if !defined(os_windows)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

bool waitForFileToExist(char *fname, int timeout_seconds)
{
   int timeout = 0; // milliseconds
   int sleep_increment = 10; // ms
   int timeout_milliseconds = 1000 *timeout_seconds;

   struct stat statbuf;

   int err = 0;
  while (0 != (err = stat(fname, &statbuf))) {
    if (err != ENOENT) {
      //fprintf(stderr, "%s[%d]:  stat failed with %s\n", FILE__, __LINE__, strerror(errno));
      return false;
    }

    struct timeval slp;
    slp.tv_sec = 0;
    slp.tv_usec = sleep_increment /*ms*/ * 1000;
    select(0, NULL, NULL, NULL, &slp);
    timeout += sleep_increment;
    if (timeout >= timeout_milliseconds) {
      fprintf(stderr, "%s[%d]:  timeout waiting for file %s to exist\n", FILE__, __LINE__, fname);
      return false;
    }
  }

  assert (!err);
  return true;
}

#if !defined(os_windows)
int openFileWhenNotBusy(char *fname, int flags, int mode, int timeout_seconds)
{

  int timeout = 0; // milliseconds
  int sleep_increment = 10; // ms
  int timeout_milliseconds = 1000 *timeout_seconds;

  
  int fd = 0;
  while (0 >= (fd = P_open(fname, flags, mode))) {
    if ((errno != EBUSY) && (errno != ETXTBSY) && (errno != 0)){
      fprintf(stderr, "%s[%d]:  open(%s) failed with %s\n", FILE__, __LINE__, fname, strerror(errno));
      return -1;
    }

    struct timeval slp;
    slp.tv_sec = 0;
    slp.tv_usec = sleep_increment /*ms*/ * 1000;
    select(0, NULL, NULL, NULL, &slp);
    timeout += sleep_increment;
    if (timeout >= timeout_milliseconds) {
      fprintf(stderr, "%s[%d]:  timeout waiting for file %s to exist\n", FILE__, __LINE__, fname);
      return -1;
    }
  }

  assert (fd > 0);
  return fd;
}
#endif





