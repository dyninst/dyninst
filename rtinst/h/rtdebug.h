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

/*
 * $Id: rtdebug.h,v
 * This file contains the extended instrumentation functions that are provided
 *   by the Paradyn run-time instrumentation layer.
 */

#ifndef _RTDEBUG_H
#define _RTDEBUG_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/* The rtdebug code allows one to see a history of the last DB_MAXRECS
   records of the timer values when the "startTimer" and "stopTimer"
   functions are called for DB_NUMPOS threads.

   DAEMON
   -  include rtinst/h/rtdebug.h in process.C
   -  call db_attach(db_shmKey) in the finalizePARADYNinit function
   -  call db_showTrace(char *) in the daemon at some place you want
      to see the history
   -  update daemon make.module.tmpl to build ../../../rtinst/src/RTdebug.c
      (also make sure VPATH includes the rtinst/src directory)

   RTINST
   -  call db_init(db_shmKey, sizeof(db_shmArea_t)) from within
      PARADYNinit() in RTinst.c (also need to include rtdebug.h)
   -  call the following code at the beginning of DYNINSTstartThreadTimer
      and DYNINSTstopThreadTimer in RTthread-timer.c (for MT apps) and
      include rtdebug.h
           unsigned pos = DYNINSTthreadPosSLOW(P_thread_self());
           db_recordTimer(op_start, pos, timer);
        OR
      call the following code at the beginning of DYNINSTstartTimer and
      DYNINSTstopTimer in RTinst.c (for ST apps) (also need to include
      rtdebug.h)
           db_recordTimer(op_start, 0, timer);
   -  you can see the history from the runtime side by calling 
      db_showTrace(char *) from within rtinst
   -  update rtinst make.module.tmpl to build ../../../rtinst/src/RTdebug.c
   -  update rtinst Makefile by adding RTdebug.o as one of the objects
*/

#define DB_NUMPOS 10
#define DB_MAXRECS 20

typedef enum { op_start = 1, op_stop = 2} optype;
typedef struct {
   tTimer *timerAddr;
   tTimer timerVal;
   optype op;
} db_rec_t;

typedef struct {
   int mutex;
   int lastIndex[DB_NUMPOS];
   db_rec_t historyBuf[DB_NUMPOS][DB_MAXRECS];
} db_shmArea_t;

extern db_shmArea_t *db_data;

extern key_t db_shmKey;

#ifdef __cplusplus
extern "C" {
#endif
extern void db_init(key_t shmkey, int shmsize);
extern void db_attach(key_t key);
extern void db_recordTimer(optype op_, int pos, const tTimer *t);
extern void db_showTrace(char *msg);
#ifdef __cplusplus
}
#endif

#endif

