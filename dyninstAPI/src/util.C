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

/* $Id: util.C,v 1.24 2000/07/28 17:21:39 pcroth Exp $
 * util.C - support functions.
 */

#include "common/h/headers.h"
#ifndef BPATCH_LIBRARY
#include "rtinst/h/rtinst.h" // for time64
#endif
#include "dyninstAPI/src/util.h"

// TIMING code

#if defined(sparc_sun_solaris2_4)

#include <values.h>
#define EVERY 1
#define TIMINGunit 1.0e+09 // 1.0e+09 ns -> secs
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

#if defined(i386_unknown_solaris2_5) || defined(sparc_sun_solaris2_4)
#include <sys/time.h>  // for gethrtime()
#endif


// TIMING code

time64 firstRecordTime = 0; // time64 = long long int (rtinst/h/rtinst.h)
timeStamp getCurrentTime(bool firstRecordRelative)
{
    static double previousTime=0.0;

    double result = 0.0;

#if defined(i386_unknown_nt4_0)
    static double freq=0.0; // the counter frequency
    LARGE_INTEGER time;

    if (freq == 0.0)
      if (QueryPerformanceFrequency(&time))
        freq = (double)time.QuadPart;
      else
        assert(0);

    if (QueryPerformanceCounter(&time))
       result = (double)time.QuadPart/freq;
    else
       assert(0);
#elif defined(i386_unknown_solaris2_5) || defined(sparc_sun_solaris2_4)
    result = (double)gethrtime() / 1000000000.0F;    
             // converts nanoseconds to seconds

    if (result < previousTime)  result = previousTime;
    else  previousTime = result;
#else
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
#endif

    if (firstRecordRelative)
       result -= firstRecordTime;

    return result;
}

#ifndef BPATCH_LIBRARY
time64 getCurrWallTime() {
   // like the above routine but doesn't return a double value representing
   // # of seconds; instead, it returns a long long int representing the # of
   // microseconds since the beginning of time.
 
   static time64 previousTime = 0;
   time64 result;

#if defined(i386_unknown_nt4_0)
    static double freq=0.0; // the counter frequency
    LARGE_INTEGER time;

    if (freq == 0.0)
      if (QueryPerformanceFrequency(&time))
        freq = (double)time.QuadPart;
      else
        assert(0);

    if (QueryPerformanceCounter(&time))
       result = (time64)(((double)time.QuadPart/freq)*1000000.0);
    else
       assert(0);
#elif defined(i386_unknown_solaris2_5) || defined(sparc_sun_solaris2_4)
    result = gethrtime() / 1000;    // converts nanoseconds to microseconds

    if (result < previousTime)  result = previousTime;
    else  previousTime = result;
#else
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
#endif

   return result;
}

time64 userAndSysTime2uSecs(const timeval &uTime,
                                        const timeval &sysTime) {
   time64 result = uTime.tv_sec + sysTime.tv_sec;
   result *= 1000000;

   result += uTime.tv_usec + sysTime.tv_usec;

   return result;
}
#endif

static unsigned addrHashCommon(Address addr) {
   // inspired by hashs of string class

   register unsigned result = 5381;

   register Address accumulator = addr;
   while (accumulator > 0) {
      // We use 3 bits at a time from the address
      result = (result << 4) + result + (accumulator & 0x07);
      accumulator >>= 3;
   }

   return result;
}

unsigned addrHash(const Address & iaddr) {
   return addrHashCommon(iaddr);
}

unsigned addrHash4(const Address &iaddr) {
   // call when you know that the low 2 bits are 0 (meaning they contribute
   // nothing to an even hash distribution)
   return addrHashCommon(iaddr >> 2);
}

unsigned addrHash16(const Address &iaddr) {
   // call when you know that the low 4 bits are 0 (meaning they contribute
   // nothing to an even hash distribution)
   return addrHashCommon(iaddr >> 4);
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
  static bool first_time=true;
  static FILE *fp=NULL;
  if (first_time) {
    first_time=false;
    fp = fopen("timing.out","w");
  }
  TIMINGtime2[id]=gethrtime();
  TIMINGcur[id]=TIMINGtime2[id]-TIMINGtime1[id];
  if (TIMINGcur[id] > TIMINGmax[id]) TIMINGmax[id]=TIMINGcur[id];
  if (TIMINGcur[id] < TIMINGmin[id]) TIMINGmin[id]=TIMINGcur[id];
  TIMINGtot[id] += TIMINGcur[id];
  TIMINGcounter[id]++;
  if (!(TIMINGcounter[id]%EVERY)) {
    sprintf(errorLine,"<%s> cur=%5.2f, avg=%5.2f, max=%5.2f, min=%5.2f, tot=%5.2f\n",func,TIMINGcur[id]/TIMINGunit,(TIMINGtot[id]/TIMINGcounter[id])/TIMINGunit,TIMINGmax[id]/TIMINGunit,TIMINGmin[id]/TIMINGunit,TIMINGtot[id]/TIMINGunit);
    if (fp) {
      fprintf(fp,"%s",P_strdup(errorLine));
      fflush(fp);
    } else {
      logLine(errorLine);
    }
  }
}

#endif

// TIMING code


