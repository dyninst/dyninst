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

typedef thread_key_t dyninst_key_t;
typedef mutex_t dyninst_mutex_t;
typedef cond_t dyninst_cond_t;
typedef thread_t dyninst_t;
typedef rwlock_t dyninst_rwlock_t;
typedef sema_t dyninst_sema_t;
#define pthread_setspecific(key,val) thr_setspecific(key, val)
#define pthread_key_create(key,dest) thr_keycreate(key, dest)
#define pthread_self() thr_self()
#define pthread_mutex_lock(mutex) mutex_lock(mutex)
#define pthread_mutex_unlock(mutex) mutex_unlock(mutex)
#define pthread_cond_wait(cond, mutex) cond_wait(cond, mutex)
#define pthread_mutex_init(mutex, arg) mutex_init(mutex, USYNC_PROCESS, arg)
#define pthread_cond_init(cond, arg) cond_init(cond, USYNC_PROCESS, arg)
#endif

#include "tc-lock.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"

#define MAX_PENDING_RPC  (20)
typedef struct rpcToDo_s {
  int flag; /* Indicates when done, and how */
  void (*rpc) (void); /* Function to run */
  tc_lock_t lock;
} rpcToDo;

typedef struct sharedData_s {
  tTimer virtualTimers[MAX_NUMBER_OF_THREADS] ;
  rpcToDo rpcToDoList [MAX_PENDING_RPC];
  unsigned rpc_indexMax ;
  pthread_mutex_t rpc_mutex ;
  pthread_cond_t  rpc_cv ;
  int     rpc_pending ;
  rpcToDo pendingIRPCs [MAX_NUMBER_OF_THREADS][MAX_PENDING_RPC];
} RTINSTsharedData ;

extern int  DYNINSTthreadSelf(void);  
extern void DYNINSTthreadDeletePos(void);
extern int  DYNINSTthreadPos(void);
extern int  DYNINSTthreadPosFAST(void);
extern int  DYNINST_not_deleted(void);
extern int  DYNINSTloop(void);
extern int  DYNINSTthreadPosTID(int, unsigned); 
extern int  DYNINST_was_deleted(int);
extern void DYNINSTthread_init(char *DYNINST_shmSegAttachedPtr) ;

extern void mt_printf(const char *fmt, ...) ;
extern RTINSTsharedData *RTsharedData;
EXTERN_DECLARE_TC_LOCK(DYNINST_traceLock);

extern void DYNINST_ThreadPInfo(void*, void**, int *, long*, int*, void**/*&resumestate_t*/);
extern int  DYNINST_ThreadInfo(void**, int *, long*, int*, void** /*&resumestate_t*/);

/* Compatibility: redefine pthread primitives as solaris primitives */
#if !defined(rs6000_ibm_aix4_1)
#endif
#endif
