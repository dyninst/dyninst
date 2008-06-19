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

/* $Id: util.C,v 1.36 2008/06/19 22:13:43 jaw Exp $
 * util.C - support functions.
 */

#include "common/h/headers.h"
#include "common/h/Time.h"
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

#if defined(i386_unknown_solaris2_5) || defined(sparc_sun_solaris2_4)
#include <sys/time.h>  // for gethrtime()
#endif

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
      fprintf(stderr, "%s[%d]:  stat failed with %s\n", FILE__, __LINE__, strerror(errno));
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





