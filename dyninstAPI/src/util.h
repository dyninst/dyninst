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
 * util.h - support functions.
 *
 * $Log: util.h,v $
 * Revision 1.20  1997/09/28 22:22:36  buck
 * Added some more #ifdef BPATCH_LIBRARYs to eliminate some Dyninst API
 * library dependencies on files in rtinst.
 *
 * Revision 1.19  1997/08/18 01:34:33  buck
 * Ported the Dyninst API to Windows NT.
 *
 * Revision 1.1.1.3  1997/07/08 20:03:24  buck
 * Bring latest changes from Wisconsin over to Maryland repository.
 *
 * Revision 1.18  1997/06/23 17:13:40  tamches
 * some additional hash functions
 *
 * Revision 1.17  1997/04/29 23:16:35  mjrg
 * Changes for WindowsNT port
 * Delayed check for DYNINST symbols to allow linking libdyninst dynamically
 * Changed way paradyn and paradynd generate resource ids
 * Changes to instPoint class in inst-x86.C to reduce size of objects
 * Added initialization for process->threads to fork and attach constructors
 *
 * Revision 1.16  1997/02/26 23:43:11  mjrg
 * First part on WindowsNT port: changes for compiling with Visual C++;
 * moved unix specific code to unix.C
 *
 * Revision 1.15  1997/02/21 20:14:00  naim
 * Moving files from paradynd to dyninstAPI + moving references to dataReqNode
 * out of the ast class. The is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.14  1997/01/27 19:41:19  naim
 * Part of the base instrumentation for supporting multithreaded applications
 * (vectors of counter/timers) implemented for all current platforms +
 * different bug fixes - naim
 *
 * Revision 1.13  1996/10/31 08:54:04  tamches
 * added headers for some time routines and vrbles
 *
 * Revision 1.12  1996/08/16 21:20:15  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.11  1996/06/01 00:01:18  tamches
 * addrHash --> addrHash16
 *
 * Revision 1.10  1996/05/11 23:16:07  tamches
 * added addrHash
 *
 * Revision 1.9  1996/05/09 22:46:51  karavan
 * changed uiHash.
 *
 * Revision 1.8  1995/02/16  08:54:30  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.7  1995/02/16  08:35:05  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 */

#ifndef UTIL_H
#define UTIL_H

#include "util/h/headers.h"
#ifdef BPATCH_LIBRARY
#include "dyninstAPI_RT/h/rtinst.h" // for time64
#else
#include "rtinst/h/rtinst.h" // for time64
#endif
#include "util/h/String.h"

typedef double timeStamp;
timeStamp getCurrentTime(bool firstRecordRelative);
   // Return the current wall time --
   //    If firstRecordRelative is true, time starts at the arrival of record 0.
   //    otherwise it starts at 1/1/1970 0:00 GMT.

time64 getCurrWallTime();
//unsigned long long getCurrWallTimeULL();
   // Like the above routine but doesn't return # of seconds as a double; instead,
   // returns # of microseconds as a long long int
time64 userAndSysTime2uSecs(const timeval &uTime,
                                        const timeval &sysTime);

extern void logLine(const char *line);
extern void statusLine(const char *line);
extern char errorLine[];

inline unsigned uiHash(const unsigned &val) {
  return val;
}

inline unsigned CThash(const unsigned &val) {
  return val % 1048573;
}

unsigned addrHash4(const unsigned &addr);
   // use when you know the address is divisible by 4 (lo 2 bits 0)
unsigned addrHash16(const unsigned &addr);
   // use when you know the address is divisible by 16 (lo 4 bits 0)
unsigned addrHash(const unsigned &addr);
   // use when you cannot assume anything about the address

inline unsigned intHash(const int &val) {
  return val;
}

void
pd_log_perror(const char* msg);

typedef struct sym_data {
  string name;
  bool must_find;
} sym_data;

typedef enum { counter, procTimer, wallTimer } CTelementType;

// TIMING code

#if defined(sparc_sun_solaris2_4)

#define MAX_N_TIMERS 10

extern hrtime_t TIMINGtime1[MAX_N_TIMERS];
extern hrtime_t TIMINGtime2[MAX_N_TIMERS]; 
extern hrtime_t TIMINGcur[MAX_N_TIMERS];
extern hrtime_t TIMINGtot[MAX_N_TIMERS];
extern hrtime_t TIMINGmax[MAX_N_TIMERS];
extern hrtime_t TIMINGmin[MAX_N_TIMERS];
extern int TIMINGcounter[MAX_N_TIMERS];

extern void begin_timing(int id);
extern void end_timing(int id, char *func);

#endif

// TIMING code

#endif /* UTIL_H */
