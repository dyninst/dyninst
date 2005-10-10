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

/************************************************************************
 *
 * RTthread.c: platform independent runtime instrumentation functions 
 * for threads
 *
 ************************************************************************/

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "RTthread.h"

#include "pdutil/h/resource.h"
#include "dyninstAPI_RT/h/dyninstRTExport.h"

/**************************** INITIALIZATION ********************************/

/*======================================
 *
 *   SyncObject for MT applications
 *
 *======================================*/

#define MAX_SYNC_OBJS 400
typedef struct SyncObj_s {
  unsigned id;
  struct SyncObj_s *next;
} SyncObj ;

static SyncObj syncObj_cells[MAX_SYNC_OBJS];
static int next_free_syncObj_cell = 0 ;
static DECLARE_DYNINST_LOCK(PARADYN_syncObjLock) ;

static int next_syncObj_index(void) 
{
  int next;
  dyninst_lock(&PARADYN_syncObjLock);
  next = next_free_syncObj_cell++;
  dyninst_unlock(&PARADYN_syncObjLock);
  return next;
}

static int lookup_syncobject(SyncObj* Res[], dyninst_lock_t L[], int max, 
                             unsigned res) 
{
  int ret ;
  int pos = res%max ;
  SyncObj* ptr = Res[pos] ;
  dyninst_lock(&(L[pos])) ;
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
  dyninst_unlock(&(L[pos])) ;
  return ret ;
}


/*----------------------------*
 *  Conditional Variables     *
 *----------------------------*/
#define MAX_CV_RES 20
static SyncObj*    PARADYNcvRes[MAX_CV_RES] ;
static dyninst_lock_t PARADYNcvResLock[MAX_CV_RES] ;

void PARADYNreportNewCondVar(dyninst_cond_t* cvp){

  if (lookup_syncobject(PARADYNcvRes, PARADYNcvResLock, MAX_CV_RES, (unsigned) cvp)){
    struct _newresource newRes ;
    memset(&newRes, '\0', sizeof(newRes));
    sprintf(newRes.name, "SyncObject/CondVar/%d", (unsigned) cvp) ;
    strcpy(newRes.abstraction, "BASE");
    newRes.mdlType = RES_TYPE_INT;
    newRes.btype = CondVarResourceType;
    PARADYNgenerateTraceRecord(TR_NEW_RESOURCE,
                               sizeof(struct _newresource), &newRes,
                               0, 0);
  }
}

/*-----------------*
 *  Mutex lock     *
 *-----------------*/
#define MAX_MUTEX_RES 20
static SyncObj*    PARADYNmutexRes[MAX_MUTEX_RES] ;
static dyninst_lock_t PARADYNmutexResLock[MAX_MUTEX_RES] ;

void PARADYNreportNewMutex(dyninst_mutex_t* m)
{
  if (lookup_syncobject(PARADYNmutexRes, PARADYNmutexResLock,
      MAX_MUTEX_RES, (unsigned )m)){
    struct _newresource newRes ;
    memset(&newRes, '\0', sizeof(newRes));
    sprintf(newRes.name, "SyncObject/Mutex/%d", (unsigned) m) ;
    strcpy(newRes.abstraction, "BASE");
    newRes.mdlType = RES_TYPE_INT;
    newRes.btype = MutexResourceType;
    PARADYNgenerateTraceRecord(TR_NEW_RESOURCE,
                        sizeof(struct _newresource), &newRes,
                        0, 0);
  }
}

/*-----------------*
 *  rw_lock        *
 *-----------------*/
#define MAX_RWLOCK_RES 20
static SyncObj*    PARADYNrwlockRes[MAX_RWLOCK_RES] ;
static dyninst_lock_t PARADYNrwlockResLock[MAX_RWLOCK_RES] ;

void PARADYNreportNewRwLock(dyninst_rwlock_t* p){

  if (lookup_syncobject(PARADYNrwlockRes, PARADYNrwlockResLock, 
                        MAX_RWLOCK_RES, (unsigned)p))
  {
     struct _newresource newRes ;
     memset(&newRes, '\0', sizeof(newRes));
     sprintf(newRes.name, "SyncObject/RwLock/%d", (unsigned) p) ;
     strcpy(newRes.abstraction, "BASE");
     newRes.mdlType = RES_TYPE_INT;
     newRes.btype = RWLockResourceType;
     PARADYNgenerateTraceRecord(TR_NEW_RESOURCE,
                                sizeof(struct _newresource), &newRes,
                                0,0);
  }
}

/*-----------------*
 *  Semaphore      *
 *-----------------*/
#define MAX_SEMA_RES 20
static SyncObj*    PARADYNsemaRes[MAX_SEMA_RES] ;
static dyninst_lock_t PARADYNsemaResLock[MAX_SEMA_RES] ;

void PARADYNreportNewSema(dyninst_sema_t* p)
{
  if (lookup_syncobject(PARADYNsemaRes, PARADYNsemaResLock, 
                        MAX_SEMA_RES, (unsigned)p))
  {
     struct _newresource newRes ;
     memset(&newRes, '\0', sizeof(newRes));
     sprintf(newRes.name, "SyncObject/Semaphore/%d", (unsigned) p) ;
     strcpy(newRes.abstraction, "BASE");
     newRes.mdlType = RES_TYPE_INT;
     newRes.btype = SemaphoreResourceType;
     PARADYNgenerateTraceRecord(TR_NEW_RESOURCE,
                                sizeof(struct _newresource), &newRes,
                                0, 0);
  }
}

void newPDThread(int index)
{
   if (virtualTimers) {
      _VirtualTimerDestroy(&(virtualTimers[index]));
      _VirtualTimerStart(&(virtualTimers[index]), THREAD_CREATE) ;
   }
}
