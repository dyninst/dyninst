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

#ifdef SHM_SAMPLING
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

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
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"

#ifdef rs6000_ibm_aix4_1
#include <pthread.h>
#else
#include <thread.h>
#include <sys/lwp.h>
#endif
#include <stdlib.h>
#include "RTthread.h"

unsigned          DYNINST_initialize_done=0;
unsigned          DYNINSTthreadTable[MAX_NUMBER_OF_THREADS][MAX_NUMBER_OF_LEVELS];
void*             DYNINST_allthreads_p = NULL ;

DECLARE_TC_LOCK(DYNINST_removeLock) ;
DECLARE_TC_LOCK(DYNINST_traceLock) ;
DECLARE_TC_LOCK(DYNINST_posLock) ;
DECLARE_TC_LOCK(DYNINST_initLock) ;

struct nlist      *DYNINST_ThreadTable[MAX_NUMBER_OF_THREADS];
tc_lock_t          DYNINST_ThreadTableLock[MAX_NUMBER_OF_THREADS] ;
int                DYNINST_ThreadTids[MAX_NUMBER_OF_THREADS];
int                ThreadMap[MAX_NUMBER_OF_THREADS];

#define MAX_REMOVED_THREADS MAX_NUMBER_OF_THREADS*4

#define VIRTUAL_TIMER_MARK_CREATION(t)     (t)->vtimer=t;
#define VIRTUAL_TIMER_MARK_LWPID(t, lwpid) (t)->lwp_id=lwpid;

#define THREAD_UPDATE           0
#define THREAD_CREATE           1
#define THREADP_CREATE          2
#define VIRTUAL_TIMER_CREATE    3
#define VIRTUAL_TIMER_START     4
#define THREAD_TIMER_START      5
#define THREAD_DETECT   6
void   _VirtualTimerStart(tTimer *timer, int context) ;

/*============================
       Per-Thread Data
 ============================= */
RTINSTsharedData *RTsharedData = NULL ;
rpcToDo          *DYNINSTthreadRPC = NULL ;
tTimer           *DYNINST_thr_vtimers = NULL ; /* per-Thread virtual Timers */

static thread_key_t  DYNINST_thread_key ;

/* pointer to thread's tls */
static void** DYNINSTthreadSpecific[MAX_NUMBER_OF_THREADS] ;

/* To synchronize the rpcThread with the daemon */
cond_t   *rpc_cv_ptr ;
mutex_t  *rpc_mutex_ptr ;
int      *rpc_pending_ptr ;
unsigned *rpc_maxIndex_ptr ;
thread_t  DYNINSTthreadRPC_threadId ;

void DYNINSTthread_init(char *DYNINST_shmSegAttachedPtr) {
  void DYNINST_initialize_RPCthread(void) ;
  RTsharedData = (RTINSTsharedData*)((char*) DYNINST_shmSegAttachedPtr + 16) ;
  DYNINST_initialize_RPCthread();

  DYNINST_thr_vtimers = RTsharedData->virtualTimers ;
  memset((char*)DYNINST_thr_vtimers, '\0', sizeof(tTimer)*MAX_NUMBER_OF_THREADS) ;
}

#define HASH(x) (x % MAX_NUMBER_OF_THREADS)
struct nlist {
  unsigned key;
  unsigned val;
  struct nlist *next;
};

struct elemList {
  unsigned pos;
  struct elemList *next;
};

struct elemList *DYNINST_freeList=NULL;
struct elemList *DYNINST_freeList_Tail=NULL;

/***************************************
 * pre-allocation an array, so that we do not have to call malloc and free
 ***************************************/
static struct elemList Position_Bank[MAX_NUMBER_OF_THREADS] ;

void DYNINST_free_pos(unsigned pos) {
  struct elemList *tmp;
  tmp = &(Position_Bank[pos]) ;
  tmp->next = NULL ;

  if (DYNINST_DEAD_LOCK == tc_lock_lock(&DYNINST_posLock)) {
    /* fprintf(stderr, "*** DEAD LOCK in DYNINST_free_pos...\n") ; */
    return ;
  }

  if (DYNINST_freeList_Tail) {
    DYNINST_freeList_Tail->next = tmp ;
  }
  DYNINST_freeList_Tail = tmp ;

  if (!DYNINST_freeList)
    DYNINST_freeList = tmp;
  tc_lock_unlock(&DYNINST_posLock);
}

void DYNINST_VirtualTimerDestroy(tTimer* vt);

unsigned DYNINST_get_pos() {
  unsigned pos;
  struct elemList *tmp;
  if (DYNINST_freeList) {
    if (DYNINST_DEAD_LOCK == tc_lock_lock(&DYNINST_posLock)) {
      /* fprintf(stderr, "*** DEAD LOCK in DYNINST_get_pos...\n") ; */
      return -1 ;
    }
    pos = DYNINST_freeList->pos;
    DYNINST_freeList = DYNINST_freeList->next;
    if (!DYNINST_freeList)
      DYNINST_freeList_Tail = NULL ;
    tc_lock_unlock(&DYNINST_posLock) ;

    if (DYNINST_thr_vtimers)
      DYNINST_VirtualTimerDestroy(DYNINST_thr_vtimers+pos) ;
    return(pos);
  }
  assert(0);
}

void DYNINST_initialize_free(unsigned total) {
  int i ;
  assert(total <= MAX_NUMBER_OF_THREADS) ;
  for (i=0; i<total-1; i++) {
    Position_Bank[i].pos = i ;
    Position_Bank[i].next = &(Position_Bank[i+1]);
  }
  Position_Bank[total-1].pos = total-1 ;
  Position_Bank[total-1].next = NULL ;

  DYNINST_freeList = &(Position_Bank[0]) ;
  DYNINST_freeList_Tail = &(Position_Bank[total-1]);
}

void DYNINST_initialize_hash(unsigned total) {
  int i;
  void init_ThreadTable_cell() ;
  void DYNINST_initialize_SyncObj(void) ;

  for (i=0;i<total;i++)  {
    DYNINST_ThreadTids[i] = -1 ;
    DYNINST_ThreadTable[i] = NULL;
    tc_lock_init(&(DYNINST_ThreadTableLock[i]));
  }
  init_ThreadTable_cell() ;
  DYNINST_initialize_SyncObj() ;
}

int DYNINST_hash_lookup(unsigned k) {
  unsigned pos;
  struct nlist *p;

  assert(DYNINST_initialize_done==1) ;
  pos = HASH(k);

  if (0!=tc_lock_trylock(DYNINST_ThreadTableLock + pos)){
    return -2 ;
  }

  p=DYNINST_ThreadTable[pos];
  while (p!=NULL) {
    if ((p->key)==k) break;
    p=p->next;
  }

  tc_lock_unlock(DYNINST_ThreadTableLock + pos) ;
  if (p!=NULL) {
    return p->val ;
  } else {
    return  -1;
  }
}

/* We manange the hash cells by ourselves */
static struct nlist DYNINST_ThreadTable_cells[MAX_NUMBER_OF_THREADS*MAX_NUMBER_OF_THREADS] ;
static struct nlist* DYNINST_ThreadTable_FreeLists[MAX_NUMBER_OF_THREADS];
void init_ThreadTable_cell(){
  unsigned i,k ;
  for (k=0; k< MAX_NUMBER_OF_THREADS; k++) {
    for (i=0; i<MAX_NUMBER_OF_THREADS-1; i++) {
      DYNINST_ThreadTable_cells[k*MAX_NUMBER_OF_THREADS+i].next
        = & (DYNINST_ThreadTable_cells[k*MAX_NUMBER_OF_THREADS+i+1]) ;
    }
    DYNINST_ThreadTable_cells[(k+1)*MAX_NUMBER_OF_THREADS-1].next = NULL ;
    DYNINST_ThreadTable_FreeLists[k] = &(DYNINST_ThreadTable_cells[k*MAX_NUMBER_OF_THREADS]);
  }
}

struct nlist* alloc_ThreadTable_cell(unsigned pos) {
  struct nlist* ptr = DYNINST_ThreadTable_FreeLists[pos] ;
  DYNINST_ThreadTable_FreeLists[pos] = DYNINST_ThreadTable_FreeLists[pos]->next
;
  return ptr ;
}

void free_ThreadTable_cell(unsigned pos, struct nlist* ptr) {
  ptr->next = DYNINST_ThreadTable_FreeLists[pos] ;
  DYNINST_ThreadTable_FreeLists[pos] = ptr ;
}

int DYNINST_hash_insert(unsigned k) {
  unsigned pos;
  struct nlist *p, *tmp;
  unsigned val ;

  assert(DYNINST_initialize_done==1) ;
  pos = HASH(k);

  if (DYNINST_DEAD_LOCK == tc_lock_lock(DYNINST_ThreadTableLock+pos)){
    /* fprintf(stderr, "*** DEAD LOCK in DYNINST_hash_insert...\n") ; */
    return -2 ;
  }

  p=DYNINST_ThreadTable[pos];
  if (p != NULL) {
    while (p!=NULL) {
      tmp=p;
      if (p->key==k) break;
      p=p->next;
    }
    if (NULL == p) {
      p = alloc_ThreadTable_cell(pos);
      assert(p) ;
      p->next=NULL;
      p->key=k;
      p->val=DYNINST_get_pos();
      tmp->next=p;
    }
  } else {
    p = alloc_ThreadTable_cell(pos) ;
    assert(p) ;
    p->next=NULL;
    p->key=k;
    p->val=DYNINST_get_pos();
    DYNINST_ThreadTable[pos]=p;
  }
  tc_lock_unlock(DYNINST_ThreadTableLock+pos) ;

  return(p->val);
}

int DYNINST_hash_insertPOS(unsigned k, int oldpos) {
  unsigned pos;
  struct nlist *p, *tmp;
  unsigned val ;

  assert(DYNINST_initialize_done==1) ;
  pos = HASH(k);

  /* yield to DYNINST_ThreadPCreate */
  if (DYNINST_DEAD_LOCK == tc_lock_lock(DYNINST_ThreadTableLock+pos)){
    /* fprintf(stderr, "*** DEAD LOCK in DYNINST_hash_insertPOS...\n") ; */
    return -2 ;
  }
  p=DYNINST_ThreadTable[pos];
  if (p != NULL) {
    while (p!=NULL) {
      tmp=p;
      if (p->key==k) break;
      p=p->next;
    }
    if (NULL == p) {
      p = alloc_ThreadTable_cell(pos);
      assert(p) ;
      p->next=NULL;
      p->key=k;
      p->val=DYNINST_get_pos();
      tmp->next=p;
    }
  } else {
    p = alloc_ThreadTable_cell(pos) ;
    assert(p) ;
    p->next=NULL;
    p->key=k;
    p->val=DYNINST_get_pos();
    DYNINST_ThreadTable[pos]=p;
  }
  tc_lock_unlock(DYNINST_ThreadTableLock+pos) ;

  return(p->val);
}

/*
  DYNINSthreadPos has three levels,
  [1] a global register, see DYNINSTthreadPos in RTsolaris_asm.s
  [2] thread specific data, _threadPos
  [3] search the hash table: DYNINST_hash_insertPOS ...
*/

#define PROFILE_BASETRAMP
#ifdef PROFILE_BASETRAMP
/* profile the baseTramp */
static int btramp_total_calls = 0 ;
static int btramp_hash_table = 0;
static int btramp_slow = 0 ;
void btramp_total(void) {
 btramp_total_calls ++ ;
}

void report_btramp_stat(void) {
  fprintf(stderr, "Of total calls to btramp = %d\n", btramp_total_calls) ;
  fprintf(stderr, " %d times use SLOW methods\n", btramp_slow) ;
  fprintf(stderr, " %d times use HASH_TABLE seacrching\n", btramp_hash_table) ;
}
#endif PROFILE_BASETRAMP

/* We statically allocate the thread specific data */
static int POS_thread_local[MAX_NUMBER_OF_THREADS] ;

/* search threadspecific area for pos */
int _threadPos(int tid, int oldpos) {
  int *pos_p ;
  /*  fprintf(stderr, "_threadPos(%x, %x)\n", tid, oldpos); */
  pos_p = (int *)pthread_getspecific(DYNINST_thread_key);
  if (pos_p == NULL) {
    int pos = DYNINST_hash_insertPOS(tid, oldpos) ;
    fprintf(stderr, "new POS == %x\n", pos);
#ifdef PROFILE_BASETRAMP
    btramp_hash_table++ ;
#endif
    pos_p = POS_thread_local + pos ;
    *pos_p = pos ;
    pthread_setspecific(DYNINST_thread_key, pos_p) ;
  }
#ifdef PROFILE_BASETRAMP
  btramp_slow++;
#endif
  /*  fprintf(stderr, "Returning value of pos_p, %x\n", *pos_p); */
  return *pos_p  ;
}

void DYNINST_hash_delete(unsigned k) {
  unsigned pos;
  struct nlist  *p, *tmp ;

  assert(DYNINST_initialize_done==1) ;
  p=NULL, tmp=NULL ;
  pos = HASH(k);
  assert(pos>=0 && pos<MAX_NUMBER_OF_THREADS);

  if (DYNINST_DEAD_LOCK == tc_lock_lock(DYNINST_ThreadTableLock+pos)){
    /* fprintf(stderr, "*** DEAD LOCK in DYNINST_hash_delete...\n") ; */
    return ;
  }
  p=DYNINST_ThreadTable[pos];
  assert(p);
  if (p->key==k) {
    DYNINST_ThreadTable[pos] = p->next;
    DYNINST_free_pos(p->val);
    free_ThreadTable_cell(pos, p);
  }
  else {
    while (p!=NULL) {
      if (p->key==k) break;
      else {
        tmp=p;
        p=p->next;
      }
    }
    assert(p!=NULL && tmp!=NULL);
    tmp->next = p->next;
    DYNINST_free_pos(p->val);
    free_ThreadTable_cell(pos, p);
  }
  tc_lock_unlock(DYNINST_ThreadTableLock + pos) ;
}

int DYNINST_not_deletedTID(int tid, unsigned pos) {
  fprintf(stderr, "DYNINST_not_deletedTID:2 tid(%d), pos(%d)\n",
	  tid, pos);
  
  if (tid < 0) return 0 ;
  return ( ThreadMap[pos]==tid);
}

/* could be called from another thread */
int DYNINST_was_deleted(int k) {
  return 0;
}

void DYNINST_initialize_ThreadCreate(void) {
  int i;
  for (i=0; i< MAX_NUMBER_OF_THREADS; i++)
    ThreadMap[i] = -1 ;
}

void DYNINST_dummy_free(void* v) {
}

#ifdef PROFILE_CONTEXT_SWITCH
long DYNINST_VirtualTimer_on[MAX_NUMBER_OF_THREADS];
long DYNINST_VirtualTimer_off[MAX_NUMBER_OF_THREADS];
long DYNINST_preempt ;
long DYNINST_resume ;
void DYNINST_initialize_context_prof(void) {
  int i;
  for (i=0; i< MAX_NUMBER_OF_THREADS; i++) {
    DYNINST_preempt = 0 ;
    DYNINST_resume = 0 ;
    DYNINST_VirtualTimer_on[i] = 0 ;
    DYNINST_VirtualTimer_off[i] = 0 ;
  }
}
void report_context_prof(void) {
  int i;
  fprintf(stderr, "****total  preempted=%ld, resumed=%ld\n",
               DYNINST_preempt, DYNINST_resume) ;
  for (i=0; i< 16; i++) {
    fprintf(stderr, "****VitrualTimer[%d], stop=%ld, start=%ld\n", i,
              DYNINST_VirtualTimer_off[i], DYNINST_VirtualTimer_on[i]) ;
  }
}
#endif

void DYNINST_initialize_once(void) {
  if (!DYNINST_initialize_done) {
    if (DYNINST_DEAD_LOCK == tc_lock_lock(&DYNINST_initLock))
      return;
    DYNINST_initialize_done=1;
    DYNINST_initialize_free(MAX_NUMBER_OF_THREADS);
    DYNINST_initialize_hash(MAX_NUMBER_OF_THREADS);
    DYNINST_initialize_ThreadCreate() ;
    tc_lock_unlock(&DYNINST_initLock);
    pthread_key_create(&DYNINST_thread_key, DYNINST_dummy_free) ;
#ifdef PROFILE_CONTEXT_SWITCH
    DYNINST_initialize_context_prof() ;
#endif
  }
}

void DYNINST_initialize_once_per_process(void) {
  DYNINST_initialize_done = 0 ;
  if (!DYNINST_initialize_done) {
    if (DYNINST_DEAD_LOCK == tc_lock_lock(&DYNINST_initLock))
      return;
    DYNINST_initialize_done=1;
    DYNINST_initialize_free(MAX_NUMBER_OF_THREADS);
    DYNINST_initialize_hash(MAX_NUMBER_OF_THREADS);
    DYNINST_initialize_ThreadCreate() ;
    tc_lock_unlock(&DYNINST_initLock);
    pthread_key_create(&DYNINST_thread_key, DYNINST_dummy_free) ;
#ifdef PROFILE_CONTEXT_SWITCH
    DYNINST_initialize_context_prof() ;
#endif
  }
}

int DYNINSTthreadPosTID(int tid, unsigned pos) {
  fprintf(stderr, "DYNINSTthreadPosTID, tid: %d, pos: %d, map contents: %d\n",
	  tid, pos, ThreadMap[pos]);
  if (tid < 0 || tid != ThreadMap[pos])
    return -2 ;

  /* called by inferiorRPC, the thread must already been created */
  return pos ;
}
/*
 * called by inferiorRPC,
 */
int DYNINSTloopTID(int tid, unsigned pos) {
  if (tid < 0 || tid != ThreadMap[pos])
     return 1 ;
  return 0 ;
}

/************************
 *  Store and retrieve Thread Specific Data
 ***********************/
void DYNINSTsaveFuncARG(void **arg) {
  unsigned pos = DYNINSTthreadPosFAST(); /* called in mini tramp */
  assert(pos>=0 && pos <= MAX_NUMBER_OF_THREADS) ;
  DYNINSTthreadSpecific[pos] = arg ;
}

void DYNINSTthreadRetrieveARGs(void **stackbase, int *tidp,
  long* startpc_p, int* lwpidp, void** resumestate_p) {
  unsigned pos = DYNINSTthreadPosFAST(); /* called in mini tramp */
  assert(pos>=0 && pos <= MAX_NUMBER_OF_THREADS) ;
  DYNINST_ThreadPInfo(*(DYNINSTthreadSpecific[pos]), stackbase, tidp, startpc_p,
 lwpidp, resumestate_p) ;
  return;
}

/*
 * CALLED by the Parent Thread
 */
void DYNINST_ThreadPCreate() {
    traceThread traceRec ;
    int pos, tid ;
    void* stackbase ;
    long startpc ;
    int lwpid ;
    void* resumestate_p ;

    DYNINSTthreadRetrieveARGs(&stackbase, &tid, &startpc, &lwpid, &resumestate_p);

    assert (DYNINST_initialize_done) ;
    pos=traceRec.pos=DYNINST_hash_lookup(tid) ;
    while(pos < 0) {
      if (pos == -1) {
        pos=traceRec.pos = DYNINST_hash_insert(tid);
      } else if (pos == -2) {
        pos=traceRec.pos=DYNINST_hash_lookup(tid) ;
      }
    }

    if (ThreadMap[pos] != tid) {
      ThreadMap[pos] = tid ;
      traceRec.ppid   = getpid();
      traceRec.tid    = tid;
      traceRec.ntids  = 1;
      traceRec.stride = 0;
      traceRec.stack_addr = (unsigned) stackbase;
      traceRec.start_pc = (unsigned) startpc ;
      traceRec.resumestate_p = resumestate_p ;
      traceRec.context = FLAG_PARENT ;

      VIRTUAL_TIMER_MARK_LWPID(DYNINST_thr_vtimers+pos, lwpid) ;
      DYNINSTgenerateTraceRecord(0,
        TR_THR_CREATE,
        sizeof(traceRec),
        &traceRec,
        1,
        DYNINSTgetWalltime(),
        DYNINSTgetCPUtime());
    }
}

/*
 * CALLED in DYNINSTinit, update the info of the first thread
 * by the Thread itself
 */

int DYNINST_ThreadUpdate(int flag) {
  traceThread traceRec ;
  int pos,  tid;
  void* stackbase ;
  long  startpc ;
  int   lwpid ;
  void*  resumestate_p ;

  if (DYNINST_ThreadInfo(&stackbase, &tid, &startpc, &lwpid, &resumestate_p)) {
    pos=traceRec.pos=DYNINST_hash_lookup(tid) ;
    while(pos < 0) {
      if (pos == -1) {
        pos=traceRec.pos = DYNINST_hash_insert(tid);
      } else if (pos == -2) {
        pos=traceRec.pos=DYNINST_hash_lookup(tid) ;
      }
    }
    if (ThreadMap[pos] != tid) {
      ThreadMap[pos] = tid ;
      traceRec.ppid   = getpid();
      traceRec.tid    = tid;
      traceRec.ntids  = 1;
      traceRec.stride = 0;
      traceRec.stack_addr = (unsigned) stackbase;
      traceRec.start_pc = (unsigned) startpc ;
      traceRec.resumestate_p = resumestate_p ;
      traceRec.context = flag ;

      VIRTUAL_TIMER_MARK_LWPID(DYNINST_thr_vtimers+pos, lwpid) ;
      VIRTUAL_TIMER_MARK_CREATION(DYNINST_thr_vtimers+pos);
      _VirtualTimerStart(DYNINST_thr_vtimers+pos, THREAD_UPDATE) ;

      DYNINSTgenerateTraceRecord(0,
        TR_THR_SELF,
        sizeof(traceRec),
        &traceRec,
        1,
        DYNINSTgetWalltime(),
        DYNINSTgetCPUtime());
    } 
    /* else {
      fprintf(stderr, "ThreadUpdate, %d has already been reported ...\n", tid);
    }*/
    return pos ;
  }
}

/*
 * Called in DYNINSTthreadPos in RTsolaris_asm.s.
 * The thread reports itself
 */
void DYNINST_ThreadCreate(int pos, int tid) {
  void* stackbase ;
  long  startpc ;
  int   lwpid ;
  void*  resumestate_p ;
  extern int pipeOK(void); /* RTposix.c */

  if (pipeOK() && pos >= 0 && ThreadMap[pos] != tid) {
    if (DYNINST_ThreadInfo(&stackbase, &tid, &startpc, &lwpid, &resumestate_p)) {
      traceThread traceRec ;
      ThreadMap[pos] = tid ;
      traceRec.ppid   = getpid();
      traceRec.tid    = tid;
      traceRec.ntids  = 1;
      traceRec.stride = 0;
      traceRec.stack_addr = (unsigned) stackbase;
      traceRec.start_pc = (unsigned) startpc ;
      traceRec.resumestate_p = resumestate_p ;
      traceRec.pos=pos ;
      traceRec.context = FLAG_SELF ;

      if (!(DYNINST_thr_vtimers+pos)->vtimer) {
        VIRTUAL_TIMER_MARK_LWPID(DYNINST_thr_vtimers+pos, lwpid) ;
        VIRTUAL_TIMER_MARK_CREATION(DYNINST_thr_vtimers+pos);
#ifdef DEBUG_THREAD_TIMER
        fprintf(stderr, "***perThread virtual timer(0x%x) start, pos=%d, tid=%d, lwpid=%d\n",
           DYNINST_thr_vtimers+pos, pos, tid, lwpid);
#endif
        _VirtualTimerStart(DYNINST_thr_vtimers+pos, THREAD_CREATE) ;
      }

      DYNINSTgenerateTraceRecord(0,
        TR_THR_CREATE,sizeof(traceRec),
        &traceRec,
        1,
        DYNINSTgetWalltime(),
        DYNINSTgetCPUtime());
      fprintf(stderr, "Trace record generated\n");
    }
  }
}

int DYNINSTthreadCheckRPC(void) {
  unsigned k ;
  if (DYNINSTthreadRPC) {
    for (k=0; k<MAX_PENDING_RPC; k++) {
      rpcToDo* p = DYNINSTthreadRPC+k ;
      if (p->flag == 1) {
        if (0 != tc_lock_trylock(&(p->lock)))
          return ;
 
        if (p->rpc) {
          (*p->rpc)() ;  /* do the inferiorRPC */
        }
        (p->flag) = 2  ; /* done execution */
        tc_lock_unlock(&(p->lock)) ; ;
      }
    }
  }
}

int DYNINSTthreadCheckRPC2(void) {
  unsigned k ;
  if (DYNINSTthreadRPC) {
    for (k=0; k<MAX_PENDING_RPC; k++) {
      rpcToDo* p = DYNINSTthreadRPC+k ;
      if (p->flag == 1) {
        if (p->rpc) {
          (*p->rpc)() ;  /* do the inferiorRPC */
        }
        (p->flag) = 2  ; /* done execution */
      }
    }
  }
}

void
DYNINSTthreadDelete(void) {
  unsigned level ;
  traceThread traceRec;
  rawTime64 process_time, wall_time ;
  process_time = DYNINSTgetCPUtime();
  wall_time = DYNINSTgetWalltime();
  traceRec.ppid   = getpid();
  traceRec.tid    = DYNINSTthreadSelf();
  traceRec.ntids  = 1;
  traceRec.stride = 0;
  traceRec.pos = DYNINSTthreadPosFAST(); /* called in mini */
  DYNINST_hash_delete(traceRec.tid);
  ThreadMap[traceRec.pos] = -1;
  DYNINSTgenerateTraceRecord(0,TR_THR_DELETE,sizeof(traceRec),
                             &traceRec, 1,
                             wall_time,process_time);
}


void DEBUG_VIRTUAL_TIMER_START(tTimer *timer, int context) {
    switch(context) {
      case THREAD_UPDATE: {
        fprintf(stderr, "THREAD_UPDATE--->start virtual timer(%d), lwp_id=%d\n", timer-DYNINST_thr_vtimers, timer->lwp_id);
        break ;
      }
      case THREAD_CREATE: {
        fprintf(stderr, "THREAD_CREATE--->start virtual timer(%d), lwp_id=%d\n", timer-DYNINST_thr_vtimers, timer->lwp_id);
        break ;
      }
      case THREAD_DETECT: {
        fprintf(stderr, "THREAD_DETECT--->start virtual timer(%d), lwp_id=%d\n", timer-DYNINST_thr_vtimers, timer->lwp_id);
        break ;
      }
      case VIRTUAL_TIMER_CREATE: {
        fprintf(stderr, "VIRTUAL_TIMER_CREATE--->start virtual timer(%d), lwp_id=%d\n", timer-DYNINST_thr_vtimers, timer->lwp_id);
        break ;
      }
      case VIRTUAL_TIMER_START: {
        fprintf(stderr, "VIRTUAL_TIMER_START--->start virtual timer(%d), lwp_id=%d\n", timer-DYNINST_thr_vtimers, timer->lwp_id);
        break ;
      }
      case THREAD_TIMER_START: {
        fprintf(stderr, "THREAD_TIMER_START--->start virtual timer(%d), lwp_id=%d\n", timer-DYNINST_thr_vtimers, timer->lwp_id);
        break ;
      }
    }
}

/* ONLY after the timer has been created                  */
/* This call DO NOT change the LWP_ID of the Virtual Timer */

void _VirtualTimerStart(tTimer *timer, int context){
  int lwp_id = thread_self() ;
  /*  fprintf(stderr, "VirtualTimerStart: (total: %lld, start: %lld, lwp: %d), %d, for thread %d\n", 
      timer->total, timer->start, timer->lwp_id, context, lwp_id); */
  /* assert(timer->protector1 == timer->protector2);  */
  timer->protector1++;

  if (timer->vtimer && timer->counter ==0) {
    /* getCPUtime might be for all threads
    if (lwp_id == timer->lwp_id)
      timer->start = DYNINSTgetCPUtime();
    else
    */
    timer->start = DYNINSTgetCPUtime_LWP(timer->lwp_id);
    timer->counter++;
  }

  timer->protector2++;
  /* assert(timer->protector1 == timer->protector2); */
}

void _VirtualTimerStop(tTimer* timer) {
  /*  fprintf(stderr, "VirtualTimerStart: (total: %lld, start: %lld, lwp: %d), for thread %d\n", 
      timer->total, timer->start, timer->lwp_id, 0); */
    if (!timer) { return; }

    /* assert(timer->protector1 == timer->protector2);  */
    timer->protector1++;

    if (timer->counter == 1) {
      rawTime64 now;
      int lwp_id = thread_self();
      /* if (lwp_id != timer->lwp_id) { */
      now = DYNINSTgetCPUtime_LWP(timer->lwp_id);
      /*
      } else {
        now = DYNINSTgetCPUtime();
      }
      */
      if (now < timer->start) {
	fprintf(stderr, "WARNING: rtinst, cpu timer rollback. start=%lld, now=%lld, current_lwp=%d, previous_lwp=%d\n",timer->start
		,now,lwp_id,timer->lwp_id);
      }
      else
        timer->total += (now - timer->start);
      timer->counter--;
    }
    timer->protector2++;
    /* assert(timer->protector1 == timer->protector2);  */
}


/* called when the virtual timer is reused by another thread */
void DYNINST_VirtualTimerDestroy(tTimer* vt) {
  if (vt->vtimer)
    memset((char*) vt, '\0', sizeof(tTimer)) ;
}

/* CALLED by _thread_start, create and start the Virtual Timer */
void DYNINST_VirtualTimerCREATE() {
  int pos;
  pos = DYNINSTthreadPosFAST() ; 
  fprintf(stderr, "TimerCREATE called for thread %d\n", pthread_self());
  if (pos >= 0) {
    int lwpid = thread_self() ;
    VIRTUAL_TIMER_MARK_LWPID(DYNINST_thr_vtimers+pos, lwpid) ;
    VIRTUAL_TIMER_MARK_CREATION(DYNINST_thr_vtimers+pos);
    _VirtualTimerStart(DYNINST_thr_vtimers+pos, VIRTUAL_TIMER_CREATE) ;
  }
}


/* CALLED at thread context switches */
void DYNINST_VirtualTimerStart() {
  int pos; /* in mini */
  pos = DYNINSTthreadPosFAST() ; /* in mini */
  /*  fprintf(stderr, "DYNINST_VirtualTimerStart: pos = %d\n", pos); */
#ifdef PROFILE_CONTEXT_SWITCH
DYNINST_resume++ ;
#endif
  if (pos >= 0) {
    int lwpid = thread_self() ;
    /*    fprintf(stderr, "timerStart: pos=0x%x, lwpid = %d, timer = 0x%x\n", pos, lwpid, (unsigned) DYNINST_thr_vtimers+pos); */
#ifdef PROFILE_CONTEXT_SWITCH
DYNINST_VirtualTimer_on[pos]++ ;
#endif
    /* Account for thread migration */ 
    VIRTUAL_TIMER_MARK_CREATION(DYNINST_thr_vtimers+pos);
    VIRTUAL_TIMER_MARK_LWPID(DYNINST_thr_vtimers+pos, lwpid) ;
    _VirtualTimerStart(DYNINST_thr_vtimers+pos, VIRTUAL_TIMER_START) ;
  }
}

/* CALLED at thread context switches */
void DYNINST_VirtualTimerStop() {
  int pos; /* in mini */
  pos = DYNINSTthreadPosFAST() ; /* in mini */
  /*  fprintf(stderr, "DYNINST_VirtualTimerStop, pos = %d\n", pos); */
#ifdef PROFILE_CONTEXT_SWITCH
DYNINST_preempt++ ;
#endif
  if (pos >=0) {
#ifdef PROFILE_CONTEXT_SWITCH
DYNINST_VirtualTimer_off[pos]++ ;
#endif
    _VirtualTimerStop(DYNINST_thr_vtimers+pos) ;
  }
  /*  fprintf(stderr, "DYNINSTVirtualTimerStop_end\n"); */
}


/* getThreadCPUTime */
rawTime64 getThreadCPUTime(tTimer *vt, int *valid) {
  volatile int protector1, protector2;
  rawTime64 total, start ;
  int    count, vt_lwp_id;
  tTimer *vtimer ;

  fprintf(stderr, "getThreadCPUTime\n");
  
  if (!vt->vtimer)
    return 0 ;

  protector1 = vt->protector1 ;
  count = vt->counter;
  total = vt->total;
  vt_lwp_id = vt->lwp_id;
  start = vt->start ;
  protector2 = vt->protector2 ;

  if (protector1 != protector2) {
    *valid = 0 ; /* not a valid value */
    fprintf(stderr, "Protector vrble error\n");
    return 0 ;
  }

  *valid = 1 ;
  if (count > 0) {
    unsigned long long now;
    int lwp_id = thread_self() ;

    /* always read the timer of the lwp of the vtimer*/
    /* Since this could be called by inferior RPC */
    /*    if (lwp_id != vt_lwp_id){ */
    now=DYNINSTgetCPUtime_LWP(vt_lwp_id);
    /*
      } else{
      now=DYNINSTgetCPUtime();
      }
    */
    if (now >= start) {
       return total + (now-start) ;
    } else  {
      fprintf(stderr, "Timer goes back in getThreadCPUTime, lwp_id=%d, thread_self=%d ...\n",
	      vt_lwp_id, lwp_id) ;
      return total ;
    }
  } else { /* count <= 0 */
    fprintf(stderr, "Else clause in getCPUtime\n");
    return total ;
  }
}

 /*
 //THREAD TIMER   
 */ 
/* use the flag in_inferiorRPC to mark that ThreadTimer routine is being called */
DYNINSTstartThreadTimer(tTimer* timer) {/*called by a thread itself*/
  rawTime64 start, old_start ;
  tTimer* vt ;
  int lwp_id ;
  int valid, attempts=0 ;

  fprintf(stderr, "DYNINSTstartThreadTimer (%x, %d, %d, %x)\n", 
	  (unsigned) timer, 
	  (int) timer->in_inferiorRPC,
	  (unsigned) timer->counter,
	  (unsigned) timer->vtimer);
  if (!timer || timer->in_inferiorRPC) { return; }
  fprintf(stderr, "Running on thread %d, lwp %d\n",
	  pthread_self(), thread_self());
  timer->in_inferiorRPC=1;

  lwp_id = thread_self();
  /* assert(timer->protector1 == timer->protector2); */
  timer->protector1++;
  if (timer->counter == 0) {
    if (!(timer->vtimer)) {
      int pos = (int) DYNINSTthreadPosFAST() ; /* in mini */
      fprintf(stderr, "POS = %d\n", pos);
      timer->vtimer = (struct tTimerRec *)DYNINST_thr_vtimers + pos;
      vt = (tTimer *)timer->vtimer;
      fprintf(stderr, "vt set to %x\n", vt);
      /* start the per-thread virtual timer if needed */
      if  (vt->vtimer == NULL) {
         VIRTUAL_TIMER_MARK_CREATION(vt) ;
      }
    }

    vt = (tTimer*) timer->vtimer ;
    /* a Hack */
    if ( vt->counter == 0 ) {
      VIRTUAL_TIMER_MARK_LWPID(vt, lwp_id) ;
      _VirtualTimerStart(vt, THREAD_TIMER_START);
    }
    /* end of a hack */
    fprintf(stderr, ".....\n");
    old_start = timer->start; /* set by DYNINSTstopThreadTimer */
    start = timer->start = getThreadCPUTime(vt, &valid);
    fprintf(stderr, "Timer start value is %lld (valid %d)\n", timer->start, valid);
    while ((!valid || start < old_start) && ++attempts < 5) {
      start = timer->start = getThreadCPUTime(vt, &valid);
    }
    if (start < old_start) {
      timer->start = old_start ;
    }
    fprintf(stderr, "Before incrementing counter: %d\n", timer->counter);
    timer->counter++;
    fprintf(stderr, "After incrementing counter: %d\n", timer->counter);
  }
  fprintf(stderr, "Counter: %d\n", timer->counter);
  timer->protector2++;
  /* assert(timer->protector1 == timer->protector2); */
  timer->in_inferiorRPC=0;
  fprintf(stderr, "Start: %lld\n", timer->start);
  fprintf(stderr, "Counter: %d\n", timer->counter);
  
}

void
DYNINSTstopThreadTimer(tTimer* timer) {/* called by a thread itself*/
  int valid, attempts=0 ;

  /*  fprintf(stderr, "DYNINSTstopThreadTimer\n"); */
  if (!timer || timer->in_inferiorRPC) { return; }
  timer->in_inferiorRPC=1;

  while(timer->protector1 != timer->protector2);
  timer->protector1++;

  if (timer->counter == 1) {
    tTimer *vt = (tTimer*) timer->vtimer ;
    rawTime64 now = getThreadCPUTime(vt, &valid);

    if (!valid) {
      fprintf(stderr, "WARNING: DYNINSTstopThreadTimer,timer0x%x invalid value!\n", timer);
    } else if (now < timer->start) {
      fprintf(stderr, "WARNING: DYNINSTstopThreadTimer,timer0x%x rollback, now=%lld, start=%lld\n",
              timer, now, timer->start) ;
    }
    if (now >= timer->start) {
      timer->total += (now - timer->start);
    }
    timer->start = now; /* just to make sure that, startThreadTimer does not roll back */
    timer->counter--;
  }
  timer->protector2++;
  /* ALERT! assert(timer->protector1 == timer->protector2); */
  timer->in_inferiorRPC=0;
}

/* called before the timer is reused by another Thread */
/* Could race with DYNINSTstartThreadTimer_inferiorRPC */
void DYNINSTdestroyThreadTimer(tTimer* timer) {

  if (!timer) {
    return;
  }

  timer->counter= 0 ;
  timer->total = 0 ;
  timer->start = 0 ;
  timer->vtimer = NULL ;
  timer->protector1 = timer->protector2 = 0 ;
}

/* Crash data
   timer: 0x480033
   tid: 0
   pos: 0x100159ed
r0             0xd13ecf9c       -784412772
r1             0x20766a70       544631408
r2             0x20023c24       537017380
r3             0x480033 4718643
r4             0x0      0
r5             0x100159ed       268524013
r6             0x0      0
r7             0x0      0
r8             0x480033 4718643
r9             0x100159ed       268524013
r10            0x0      0
lr             0xd13ecf9c       -784412772
*/

/* CAN be called by ANY thread */
void DYNINSTstartThreadTimer_inferiorRPC(tTimer* timer, int tid, unsigned pos) {
  tTimer *vt ;
  int valid;

  /* Dereference a bad pointer */
  valid = (int) timer->in_inferiorRPC;

  fprintf(stderr, "DYNINSTstartThreadTimer_infRPC (%x, %d, %x, %d, %d, %x)\n", 
	  (unsigned) tid, (unsigned) pos,
	  (unsigned) timer, 
	  (int) timer->in_inferiorRPC,
	  (unsigned) timer->counter,
	  (unsigned) timer->vtimer);

  if (!timer || timer->in_inferiorRPC)
    return;
  timer->in_inferiorRPC=1;

  vt =  DYNINST_thr_vtimers + pos;

  /* assert(timer->protector1 == timer->protector2); */
  timer->protector1++;

  if ((timer->counter)==0) {
    if (!(timer->vtimer)) {
      timer->vtimer = (struct tTimerRec* ) vt ;
      /* We should NOT start it, since it can be INACTIVE!!! */
      if  (vt->vtimer == NULL){
         VIRTUAL_TIMER_MARK_CREATION(vt) ;
      }
    }
    timer->start = getThreadCPUTime(vt, &valid);
    timer->total = 0 ;
  }
  timer->counter++;

  timer->protector2++;
  /* assert(timer->protector1 == timer->protector2); */
  timer->in_inferiorRPC=0;
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

void DYNINSTreportNewCondVar(cond_t* cvp){

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

void DYNINSTreportNewMutex(mutex_t* m){

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

void DYNINSTreportNewRwLock(rwlock_t* p){

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

void DYNINSTreportNewSema(sema_t* p){

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

void DYNINST_initialize_SyncObj(void) {
  initialize_SyncObj_of_1_kind(DYNINSTcvRes, DYNINSTcvResLock, MAX_CV_RES);
  initialize_SyncObj_of_1_kind(DYNINSTmutexRes, DYNINSTmutexResLock, MAX_MUTEX_RES);
  initialize_SyncObj_of_1_kind(DYNINSTrwlockRes, DYNINSTrwlockResLock, MAX_RWLOCK_RES);
  initialize_SyncObj_of_1_kind(DYNINSTsemaRes, DYNINSTsemaResLock, MAX_SEMA_RES);
}

void* DYNINST_RPC_Thread(void * garbage) {

  while(1) {
    pthread_mutex_lock(rpc_mutex_ptr);
    while (!(*rpc_pending_ptr)) { 
      /* fprintf(stderr, "RPC Thread is Waiting ...\n"); */
      pthread_cond_wait(rpc_cv_ptr, rpc_mutex_ptr);  
    }
    /* fprintf(stderr, "RPC Thread, Signaled....\n"); */
    DYNINSTthreadCheckRPC2();
    /* fprintf(stderr, "RPC Thread, RPC performed....\n"); */
    *rpc_pending_ptr = 0 ;
    pthread_mutex_unlock(rpc_mutex_ptr);
  }
}

#ifdef USE_RPC_TO_TRIGGER_RPC
void* DYNINSTsignalRPCthread(void) {
  int ret;
  pthread_mutex_lock(rpc_mutex_ptr);
  (*rpc_pending_ptr) = 1;
  ret = pthread_cond_signal(rpc_cv_ptr);
  pthread_mutex_unlock(rpc_mutex_ptr);
  return (void*) ret;
}
#endif

void DYNINSTlaunchRPCthread(void) {
#ifdef rs6000_ibm_aix4_1
  pthread_mutex_init(rpc_mutex_ptr, NULL);
  pthread_cond_init(rpc_cv_ptr, NULL);
#else
  mutex_init(rpc_mutex_ptr, USYNC_PROCESS, NULL);
  cond_init(rpc_cv_ptr, USYNC_PROCESS, NULL);
#endif
  /* thr_create(NULL, 0, DYNINST_RPC_Thread, NULL, THR_NEW_LWP|THR_BOUND, &DYNINSTthreadRPC_threadId) ; */
}

void DYNINST_initialize_RPCthread(void) {
  DYNINSTthreadRPC = RTsharedData->rpcToDoList ;
  rpc_mutex_ptr  = &(RTsharedData->rpc_mutex) ;
  rpc_cv_ptr = &(RTsharedData->rpc_cv);
  rpc_pending_ptr = &(RTsharedData->rpc_pending);
  rpc_maxIndex_ptr = &(RTsharedData->rpc_indexMax);
  *rpc_pending_ptr = 0 ;
  *rpc_maxIndex_ptr = 0 ;
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

