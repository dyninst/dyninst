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

#include "util/h/String.h"

typedef double timeStamp;
timeStamp getCurrentTime(bool firstRecordRelative);
   // Return the current wall time --
   //    If firstRecordRelative is true, time starts at the arrival of record 0.
   //    otherwise it starts at 1/1/1970 0:00 GMT.

typedef long long int time64;
time64 getCurrWallTime();
unsigned long long getCurrWallTimeULL();
   // Like the above routine but doesn't return # of seconds as a double; instead,
   // returns # of microseconds as a long long int
unsigned long long userAndSysTime2uSecs(const timeval &uTime,
                                        const timeval &sysTime);

extern void logLine(const char *line);
extern void statusLine(const char *line);
extern char errorLine[];

inline unsigned uiHash(const unsigned &val) {
  return val % 23;
}

unsigned addrHash16(const unsigned &addr);

inline unsigned intHash(const int &val) {
  return val;
}

void
pd_log_perror(const char* msg);

typedef struct sym_data {
  string name;
  bool must_find;
} sym_data;


#endif /* UTIL_H */
