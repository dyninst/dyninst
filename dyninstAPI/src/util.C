/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

/*
 * util.C - support functions.
 *
 * $Log: util.C,v $
 * Revision 1.13  1997/02/21 20:13:59  naim
 * Moving files from paradynd to dyninstAPI + moving references to dataReqNode
 * out of the ast class. The is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.12  1997/01/27 19:41:18  naim
 * Part of the base instrumentation for supporting multithreaded applications
 * (vectors of counter/timers) implemented for all current platforms +
 * different bug fixes - naim
 *
 * Revision 1.11  1996/10/31 08:54:23  tamches
 * added some time routines
 *
 * Revision 1.10  1996/08/16 21:20:14  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.9  1996/06/01 00:01:48  tamches
 * addrHash replaced by addrHash16
 *
 * Revision 1.8  1996/05/11 23:16:17  tamches
 * added addrHash
 *
 * Revision 1.7  1995/02/16 08:54:28  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.6  1995/02/16  08:35:03  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.5  1994/11/02  11:18:54  markc
 * Remove old malloc wrappers.
 *
 * Revision 1.4  1994/09/22  02:27:37  markc
 * Changed signature to intComp
 *
 */

#include "util/h/headers.h"
#include "dyninstAPI/src/util.h"

// TIMING code

#if defined(sparc_sun_solaris2_4)

#include <values.h>
#define EVERY 1
#define TIMINGunit 1.0e+03 // 1.0e+09 ns -> secs
                           // 1.0e+06 ns -> mils
                           // 1.0e+03 ns -> us 
                           // 1.0e+00 ns

hrtime_t TIMINGtime1[MAX_N_TIMERS];
hrtime_t TIMINGtime2[MAX_N_TIMERS]; 
hrtime_t TIMINGcur[MAX_N_TIMERS];
hrtime_t TIMINGtot[MAX_N_TIMERS];
hrtime_t TIMINGmax[MAX_N_TIMERS];
hrtime_t TIMINGmin[MAX_N_TIMERS];
int TIMINGcounter[MAX_N_TIMERS];

#endif

// TIMING code

typedef long long int time64;
time64 firstRecordTime = 0; // time64 = long long int (rtinst/h/rtinst.h)
timeStamp getCurrentTime(bool firstRecordRelative)
{
    static double previousTime=0.0;

    double result = 0.0;

    do {
       struct timeval tv;
       if (-1 == gettimeofday(&tv, NULL)) {
	  perror("getCurrentTime gettimeofday()");
	  return 0;
       }

       result = tv.tv_sec * 1.0;

       assert(tv.tv_usec < 1000000);
       double useconds_dbl = tv.tv_usec * 1.0;

       result += useconds_dbl / 1000000.0;
    } while (result < previousTime); // retry if we've gone backwards

    previousTime = result;

    if (firstRecordRelative)
       result -= firstRecordTime;

    return result;
}

time64 getCurrWallTime() {
   // like the above routine but doesn't return a double value representing
   // # of seconds; instead, it returns a long long int representing the # of
   // microseconds since the beginning of time.
 
   static time64 previousTime = 0;
   time64 result;
    
   do {
      struct timeval tv;
      if (-1 == gettimeofday(&tv, NULL)) {
	 perror("getCurrWallTime gettimeofday()");
	 return 0;
      }

      result = tv.tv_sec;
      result *= 1000000;
      result += tv.tv_usec;
   } while (result < previousTime);

   previousTime = result;

   return result;
}

unsigned long long getCurrWallTimeULL() {
   // like the above routine but doesn't return a double value representing
   // # of seconds; instead, it returns a long long int representing the # of
   // microseconds since the beginning of time.
 
   static unsigned long long previousTime = 0;
   unsigned long long result;
    
   do {
      struct timeval tv;
      if (-1 == gettimeofday(&tv, NULL)) {
	 perror("getCurrWallTimeULL");
	 return 0;
      }

      result = tv.tv_sec;
      result *= 1000000;
      result += tv.tv_usec;
   } while (result < previousTime);

   previousTime = result;

   return result;
}

unsigned long long userAndSysTime2uSecs(const timeval &uTime,
                                        const timeval &sysTime) {
   unsigned long long result = uTime.tv_sec + sysTime.tv_sec;
   result *= 1000000;

   result += uTime.tv_usec + sysTime.tv_usec;

   return result;
}

unsigned addrHash16(const unsigned &iaddr) {
   // inspired by hashs of string class
   // NOTE: this particular hash fn assumes that "addr" is divisible by 16

   unsigned addr = iaddr >> 4;
      // since the address is divisible by 16, the low 4 bits are always the same
      // for each address and hence contribute nothing to an even hash distribution.
      // Hence, we zap those bits right now.

   unsigned result = 5381;

   unsigned accumulator = addr;
   while (accumulator > 0) {
      // We use 3 bits at a time from the address
      result = (result << 4) + result + (accumulator & 0x07);
      accumulator >>= 3;
   }

   return result;
}

#ifdef notdef
void
log_printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsprintf(errorLine, fmt, ap);
    va_end(ap);
    logLine(errorLine);
    // printf("%s", log_buffer);
}
#endif

void
pd_log_perror(const char* msg) {
    sprintf(errorLine, "%s: %s\n", msg, sys_errlist[errno]);
    logLine(errorLine);
    // fprintf(stderr, "%s", log_buffer);
}

// TIMING code

#if defined(sparc_sun_solaris2_4)

void begin_timing(int id)
{
  static bool first_time=true;
  if (first_time) {
    if (TIMINGunit==1.0e+09) sprintf(errorLine,"TIME in seconds\n");
    else if (TIMINGunit==1.0e+06) sprintf(errorLine,"TIME in mil-seconds\n");
    else if (TIMINGunit==1.0e+03) sprintf(errorLine,"TIME in micro-seconds\n");
    else if (TIMINGunit==1.0e+00) sprintf(errorLine,"TIME in nano-seconds\n");
    else sprintf(errorLine,"TIME with UNKNOWN units\n");
    logLine(errorLine);
    for (unsigned i=0;i<MAX_N_TIMERS;i++) {
      TIMINGtime1[i]=0;
      TIMINGtime2[i]=0;
      TIMINGcur[i]=0;
      TIMINGtot[i]=0;
      TIMINGmax[i]=0;
      TIMINGmin[i]=MAXINT;
      TIMINGcounter[i]=0;
    }
    first_time=false;
  }
  TIMINGtime1[id]=gethrtime();
}

void end_timing(int id, char *func)
{
  TIMINGtime2[id]=gethrtime();
  TIMINGcur[id]=TIMINGtime2[id]-TIMINGtime1[id];
  if (TIMINGcur[id] > TIMINGmax[id]) TIMINGmax[id]=TIMINGcur[id];
  if (TIMINGcur[id] < TIMINGmin[id]) TIMINGmin[id]=TIMINGcur[id];
  TIMINGtot[id] += TIMINGcur[id];
  TIMINGcounter[id]++;
  if (!(TIMINGcounter[id]%EVERY)) {
    sprintf(errorLine,"<%s> cur=%5.2f, avg=%5.2f, max=%5.2f, min=%5.2f, tot=%5.2f\n",func,TIMINGcur[id]/TIMINGunit,(TIMINGtot[id]/TIMINGcounter[id])/TIMINGunit,TIMINGmax[id]/TIMINGunit,TIMINGmin[id]/TIMINGunit,TIMINGtot[id]/TIMINGunit);
    logLine(errorLine);
  }
}

#endif

// TIMING code
