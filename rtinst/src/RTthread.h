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

#ifndef _RT_THREAD_H_
#define _RT_THREAD_H_

/* Compatibility layer between pthreads and solaris threads */
#if defined(rs6000_ibm_aix4_1)
#include <pthread.h>
#else /* solaris threads */
#include <thread.h>
#include <synch.h>
#endif

#include "thread-compat.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"

/* Function prototypes */

/* RTthread-timer.c */

/* Context data for why virtualTimerStart was called */
#define THREAD_UPDATE           0
#define THREAD_CREATE           1
#define THREADP_CREATE          2
#define VIRTUAL_TIMER_CREATE    3
#define VIRTUAL_TIMER_START     4
#define THREAD_TIMER_START      5
#define THREAD_DETECT           6

void _VirtualTimerStart(virtualTimer *timer, int context);
void _VirtualTimerStop(virtualTimer *timer);
void _VirtualTimerDestroy(virtualTimer *timer);
unsigned PARADYNgetFD(unsigned lwp);

rawTime64 getThreadCPUTime(unsigned pos, int *valid);
void DYNINSTstartThreadTimer(tTimer *timer);
void DYNINSTstopThreadTimer(tTimer *timer);

/* RTthread-management.c */
int DYNINST_reportThreadUpdate(int flag);
void DYNINST_reportNewThread(unsigned pos, int tid);
void DYNINST_reportThreadDeletion(unsigned pos, int tid);
void DYNINSTthreadDelete(void);
unsigned DYNINSTthreadCreate(int tid);
void DYNINST_dummy_create(void);
void DYNINSTthreadStart(void);
void DYNINSTthreadStop(void);

/* RTthread-pos.c */
void DYNINST_initialize_pos_list();
unsigned DYNINST_alloc_pos(int tid);
void DYNINST_free_pos(unsigned pos, int tid);
unsigned DYNINST_lookup_pos(int tid);
unsigned DYNINSTthreadPosSLOW(int tid);

/* RTthread-<arch> */
unsigned DYNINSTthreadPosFAST();
unsigned DYNINSTthreadContext();
int DYNINSTthreadPos();

/* RTthread-<os> */
void DYNINST_ThreadPInfo(void*, void**, int *, long*, int*, void**/*&resumestate_t*/);
int  DYNINST_ThreadInfo(void**, int *, long*, int*, void** /*&resumestate_t*/);

/* RTetc-<os> */
rawTime64 DYNINSTgetCPUtime_LWP(unsigned lwp_id, unsigned fd);

/* RTthread.c */
extern dyninst_key_t  DYNINST_thread_key ;
extern unsigned DYNINST_initialize_done;
void DYNINST_initialize_once();
extern tc_lock_t DYNINST_traceLock;

/* RTinst.c */
unsigned MAX_NUMBER_OF_THREADS;

#endif

