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

/************************************************************************
 *
 * RTthread.c: platform independent runtime instrumentation functions for threads
 *
 ************************************************************************/

#include "RTthread.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "kludges.h"

#include <stdlib.h>

unsigned          DYNINST_initialize_done=0;
DECLARE_TC_LOCK(DYNINST_removeLock) ;
DECLARE_TC_LOCK(DYNINST_traceLock) ;
DECLARE_TC_LOCK(DYNINST_posLock) ;
DECLARE_TC_LOCK(DYNINST_initLock) ;
/*============================
       Per-Thread Data
 ============================= */
/* Used for storing appropriate thread-specific data */
dyninst_key_t  DYNINST_thread_key ;

/**************************** INITIALIZATION ********************************/

/*
  DYNINSthreadPos has three levels,
  [1] a global register, see DYNINSTthreadPos in RTsolaris_asm.s
  [2] thread specific data, _threadPos
  [3] search the hash table: DYNINST_hash_insertPOS ...
*/

/*
 * Run from the DYNINSTinit?
 */

void DYNINST_initialize_once(char *DYNINST_shmSegAttachedPtr) {
  if (DYNINST_DEAD_LOCK == tc_lock_lock(&DYNINST_initLock))
    return;
  if (!DYNINST_initialize_done) {
    unsigned i;
    DYNINST_initialize_pos_list();
    DYNINST_initialize_done=1;
    P_thread_key_create(&DYNINST_thread_key, NULL /* no destructor */) ;
    for (i = 0; i < MAX_NUMBER_OF_THREADS; i++) {
      RTsharedData.virtualTimers[i].total = 0;
      RTsharedData.virtualTimers[i].start = 0;
      RTsharedData.virtualTimers[i].counter = 0;
      RTsharedData.virtualTimers[i].pos = 0;
      RTsharedData.virtualTimers[i].protector1 = 0;
      RTsharedData.virtualTimers[i].protector2 = 0;
    }    
    

  }
  tc_lock_unlock(&DYNINST_initLock);
}


/*======================================
 *
 *   SyncObject for MT applications
 *
 *======================================*/
typedef struct SyncObj_s {
  unsigned id;
  struct SyncObj_s *next;
} SyncObj ;

#define MAX_SYNC_OBJS 400
SyncObj syncObj_cells[MAX_SYNC_OBJS];
int next_free_syncObj_cell = 0 ;
DECLARE_TC_LOCK(DYNINST_syncObjLock) ;

void initialize_SyncObj_of_1_kind(SyncObj* Res[], tc_lock_t L[], int max){
  int i;
  for (i=0; i<max; i++) {
    Res[i]= NULL ;
    tc_lock_init(&(L[i]));
  }
}

int next_syncObj_index(void) {
  int next ;
  tc_lock_lock(&DYNINST_syncObjLock);
  next = next_free_syncObj_cell++;
  tc_lock_unlock(&DYNINST_syncObjLock);
  return next ;
}

int lookup_syncobject(SyncObj* Res[], tc_lock_t L[], int max, unsigned res) {
  int ret ;
  int pos = res%max ;
  SyncObj* ptr = Res[pos] ;
  tc_lock_lock(&(L[pos])) ;
  while(ptr) {
    if (ptr->id == res) break ;
    ptr = ptr->next ;
  }
  if (!ptr) {
    if (next_free_syncObj_cell < MAX_SYNC_OBJS) {
      int next = next_syncObj_index() ;
      SyncObj *p = syncObj_cells + next ;
      p->id = res ;
      p->next = Res[pos] ;
      Res[pos] = p ;
      ret =  1;
    } else
      ret = 0 ;
  } else
    ret =  0 ;
  tc_lock_unlock(&(L[pos])) ;
  return ret ;
}


/*----------------------------*
 *  Conditional Variables     *
 *----------------------------*/

#define MAX_CV_RES 20
SyncObj*    DYNINSTcvRes[MAX_CV_RES] ;
tc_lock_t DYNINSTcvResLock[MAX_CV_RES] ;

void DYNINSTreportNewCondVar(dyninst_cond_t* cvp){

  if (lookup_syncobject(DYNINSTcvRes, DYNINSTcvResLock, MAX_CV_RES, (unsigned) cvp)){
    struct _newresource newRes ;
    memset(&newRes, '\0', sizeof(newRes));
    sprintf(newRes.name, "SyncObject/CondVar/%d", (unsigned) cvp) ;
    strcpy(newRes.abstraction, "BASE");
    newRes.type = RES_TYPE_INT;
    DYNINSTgenerateTraceRecord(0, TR_NEW_RESOURCE,
                        sizeof(struct _newresource), &newRes, 1,
                        0,0);
  }
}

/*-----------------*
 *  Mutex lock     *
 *-----------------*/
#define MAX_MUTEX_RES 20
SyncObj*    DYNINSTmutexRes[MAX_MUTEX_RES] ;
tc_lock_t DYNINSTmutexResLock[MAX_MUTEX_RES] ;

void DYNINSTreportNewMutex(dyninst_mutex_t* m){

  if (lookup_syncobject(DYNINSTmutexRes, DYNINSTmutexResLock,
      MAX_MUTEX_RES, (unsigned )m)){
    struct _newresource newRes ;
    memset(&newRes, '\0', sizeof(newRes));
    sprintf(newRes.name, "SyncObject/Mutex/%d", (unsigned) m) ;
    strcpy(newRes.abstraction, "BASE");
    newRes.type = RES_TYPE_INT;
    DYNINSTgenerateTraceRecord(0, TR_NEW_RESOURCE,
                        sizeof(struct _newresource), &newRes, 1,
                        0,0);
  }
}

/*-----------------*
 *  rw_lock        *
 *-----------------*/

#define MAX_RWLOCK_RES 20
SyncObj*    DYNINSTrwlockRes[MAX_RWLOCK_RES] ;
tc_lock_t DYNINSTrwlockResLock[MAX_RWLOCK_RES] ;

void DYNINSTreportNewRwLock(dyninst_rwlock_t* p){

  if (lookup_syncobject(DYNINSTrwlockRes, DYNINSTrwlockResLock, MAX_RWLOCK_RES, (unsigned)p)){
    struct _newresource newRes ;
    memset(&newRes, '\0', sizeof(newRes));
    sprintf(newRes.name, "SyncObject/RwLock/%d", (unsigned) p) ;
    strcpy(newRes.abstraction, "BASE");
    newRes.type = RES_TYPE_INT;
    DYNINSTgenerateTraceRecord(0, TR_NEW_RESOURCE,
                        sizeof(struct _newresource), &newRes, 1,
                        0,0);
  }
}

/*-----------------*
 *  Semaphore      *
 *-----------------*/
#define MAX_SEMA_RES 20
SyncObj*    DYNINSTsemaRes[MAX_SEMA_RES] ;
tc_lock_t DYNINSTsemaResLock[MAX_SEMA_RES] ;

#if !defined(rs6000_ibm_aix4_1)
void DYNINSTreportNewSema(dyninst_sema_t* p){

  if (lookup_syncobject(DYNINSTsemaRes, DYNINSTsemaResLock, MAX_SEMA_RES, (unsigned)p)){
    struct _newresource newRes ;
    memset(&newRes, '\0', sizeof(newRes));
    sprintf(newRes.name, "SyncObject/Semaphore/%d", (unsigned) p) ;
    strcpy(newRes.abstraction, "BASE");
    newRes.type = RES_TYPE_INT;
    DYNINSTgenerateTraceRecord(0, TR_NEW_RESOURCE,
                        sizeof(struct _newresource), &newRes, 1,
                        0,0);
  }
}
#endif

void DYNINST_initialize_SyncObj(void) {
  initialize_SyncObj_of_1_kind(DYNINSTcvRes, DYNINSTcvResLock, MAX_CV_RES);
  initialize_SyncObj_of_1_kind(DYNINSTmutexRes, DYNINSTmutexResLock, MAX_MUTEX_RES);
  initialize_SyncObj_of_1_kind(DYNINSTrwlockRes, DYNINSTrwlockResLock, MAX_RWLOCK_RES);
  initialize_SyncObj_of_1_kind(DYNINSTsemaRes, DYNINSTsemaResLock, MAX_SEMA_RES);
}

void mt_printf(const char *fmt, ...) {
#ifdef MT_DEBUG
   va_list args;
   va_start(args, fmt);

   vfprintf(stderr, fmt, args);

   va_end(args);

   fflush(stderr);
#endif
}

