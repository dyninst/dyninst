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

// $Id: util.h,v 1.25 2000/10/17 17:42:25 schendel Exp $

#ifndef UTIL_H
#define UTIL_H

#include "common/h/headers.h"
#include "common/h/Types.h"
#include "common/h/String.h"
#include "common/h/Time.h"

#ifndef BPATCH_LIBRARY
// DON'T access through this global variable
extern timeStamp *pFirstRecordTime;

// ACCESS through these functions
void setFirstRecordTime(const timeStamp &ts);
bool isInitFirstRecordTime();
const timeStamp &getFirstRecordTime();
#endif

extern void logLine(const char *line);
extern void statusLine(const char *line);
extern char errorLine[];

inline unsigned uiHash(const unsigned &val) {
  return val;
}

inline unsigned CThash(const unsigned &val) {
  return val % 1048573;
}

unsigned addrHash4(const Address &addr);
   // use when you know the address is divisible by 4 (lo 2 bits 0)
unsigned addrHash16(const Address &addr);
   // use when you know the address is divisible by 16 (lo 4 bits 0)
unsigned addrHash(const Address &addr);
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
