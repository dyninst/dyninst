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

/* $Id: RTcommon.c,v 1.49 2005/10/17 15:49:26 legendre Exp $ */

#include <assert.h>
#include <stdlib.h>
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "RTcommon.h"
#include "RTthread.h"

unsigned int DYNINSTobsCostLow;
unsigned int DYNINSThasInitialized;
unsigned DYNINST_max_num_threads;
struct DYNINST_bootstrapStruct DYNINST_bootstrap_info;

/**
 * Allocate the Dyninst heaps
 * 
 * The IA-64 requires that instruction be 16-byte aligned, so we have to 
 * align the heaps if we want to use them for inferior RPCs. 
 **/
#if defined(arch_ia64)
typedef struct { uint64_t low; uint64_t high; } ia64_bundle_t;
#define HEAP_TYPE ia64_bundle_t
#define ALIGN_ATTRIB __attribute((aligned))
#else
#define HEAP_TYPE double
#define ALIGN_ATTRIB 
#endif

HEAP_TYPE DYNINSTglobalData[SYN_INST_BUF_SIZE/sizeof(HEAP_TYPE)] ALIGN_ATTRIB;
HEAP_TYPE DYNINSTstaticHeap_32K_lowmemHeap_1[32*1024/sizeof(HEAP_TYPE)] ALIGN_ATTRIB;
HEAP_TYPE DYNINSTstaticHeap_4M_anyHeap_1[4*1024*1024/sizeof(HEAP_TYPE)] ALIGN_ATTRIB;


/**
 * One some platforms we can tell when a fork or exec is occurring through 
 * system-provided events. On others we do it ourselves.  Enumerated type 
 * defined in header file
 **/
DYNINST_synch_event_t DYNINST_synch_event_id = DSE_undefined;
void *DYNINST_synch_event_arg1;

/**
 * These variables are used to pass arguments into DYNINSTinit
 * when it is called as an _init function 
 **/
int libdyninstAPI_RT_init_localCause=-1;
int libdyninstAPI_RT_init_localPid=-1;
int libdyninstAPI_RT_init_maxthreads=-1;

int DYNINST_mutatorPid;
int DYNINSTdebugPrintRT;
int isMutatedExec = 0;

unsigned *DYNINST_tramp_guards;

tc_lock_t DYNINST_trace_lock;

/**
 * Init the FPU.  We've seen bugs with Linux (e.g., Redhat 6.2 stock kernel on 
 * PIIIs) where processes started by Paradyn started with FPU uninitialized. 
 * DYNINSTdummydouble is global so the compiler won't optimize away FP code
 * in initFPU
 **/
double DYNINSTdummydouble = 4321.71;                                    
static void initFPU()
{
   double x = 17.1234;
   DYNINSTdummydouble *= x;
}

static void initTrampGuards(unsigned maxthreads)
{
   unsigned i;
   //We allocate maxthreads+1 because maxthreads is sometimes used as an
   //error value to mean we don't know what thread this is.  Sometimes used
   //during the early setup phases.
   DYNINST_tramp_guards = (unsigned *) malloc((maxthreads+1)*sizeof(unsigned));
   for (i=0; i<maxthreads; i++)
   {
      DYNINST_tramp_guards[i] = 1;
   }
}

/**
 * The Dyninst API arranges for this function to be called at the entry to
 * main() via libdyninstAPI_RT_init (defined in RTposix.c and RTwinnt.c).
 * libdyninstAPI_RT_init is called by one of the following methods:
 *    GCC: link with gcc -shared, and use __attribute__((constructor));
 *    AIX: ld with -binitfini:libdyninstAPI_RT_init
 *    Solaris: ld with -z initarray=libdyninstAPI_RT_init
 *    Linux: ld with -init libdyninstAPI_RT_init
 *           gcc with -Wl,-init -Wl,...
 **/
void DYNINSTinit(int cause, int pid, int maxthreads)
{
   int calledByFork = 0, calledByAttach = 0;
   initFPU();

   tc_lock_init(&DYNINST_trace_lock);
   DYNINST_mutatorPid = pid;
   
   if (isMutatedExec) {
      fflush(stdout);
      cause = 9;
   }

   calledByFork = (cause == 2);
   calledByAttach = (cause == 3);
   DYNINSThasInitialized = 1;
   DYNINST_max_num_threads = maxthreads;

   /* sanity check */
   assert(sizeof(int64_t) == 8);
   assert(sizeof(int32_t) == 4);
   
   RTprintf("%s\n", V_libdyninstAPI_RT);
   initTrampGuards(DYNINST_max_num_threads);

#if defined(cap_async_events)
   DYNINSTasyncConnect(DYNINST_mutatorPid);
#endif
   DYNINSTos_init(calledByFork, calledByAttach);
   DYNINST_initialize_index_list();
      

   DYNINST_bootstrap_info.pid = dyn_pid_self();
   DYNINST_bootstrap_info.ppid = pid;    
   DYNINST_bootstrap_info.event = cause;
}

 
/**
 * Does what it's called. Used by the paradyn daemon as a default in certain 
 * cases (MT in particular)
 **/
int DYNINSTreturnZero()
{
   return 0;
}

/* Used to instrument (and report) the entry of fork */
void DYNINST_instForkEntry() {
   /* Set the state so the mutator knows what's up */
   DYNINST_synch_event_id = DSE_forkEntry;
   DYNINST_synch_event_arg1 = NULL;
   /* Stop ourselves */
   DYNINSTbreakPoint();
   /* Once the stop completes, clean up */
   DYNINST_synch_event_id = DSE_undefined;
   DYNINST_synch_event_arg1 = NULL;
}

       
/* Used to instrument (and report) the exit of fork */
void DYNINST_instForkExit(void *arg1) {
   /* Set the state so the mutator knows what's up */    
   DYNINST_synch_event_id = DSE_forkExit;
   DYNINST_synch_event_arg1 = arg1;
   /* Stop ourselves */
   DYNINSTbreakPoint();
   /* Once the stop completes, clean up */
   DYNINST_synch_event_id = DSE_undefined;
   DYNINST_synch_event_arg1 = NULL;
}

       
/* Used to instrument (and report) the entry of exec */
void DYNINST_instExecEntry(void *arg1) {
   /* Set the state so the mutator knows what's up */
   DYNINST_synch_event_id = DSE_execEntry;
   DYNINST_synch_event_arg1 = arg1;
   /* Stop ourselves */
   DYNINSTbreakPoint();
   /* Once the stop completes, clean up */
   DYNINST_synch_event_id = DSE_undefined;
   DYNINST_synch_event_arg1 = NULL;
}

       
/* Used to instrument (and report) the exit of exec */
void DYNINST_instExecExit(void *arg1) {
   /* Set the state so the mutator knows what's up */
   DYNINST_synch_event_id = DSE_execExit;
   DYNINST_synch_event_arg1 = arg1;
   /* Stop ourselves */
   DYNINSTbreakPoint();
   /* Once the stop completes, clean up */
   DYNINST_synch_event_id = DSE_undefined;
   DYNINST_synch_event_arg1 = NULL;
}

       
/* Used to instrument (and report) the entry of exit */
void DYNINST_instExitEntry(void *arg1) {
   /* Set the state so the mutator knows what's up */
   DYNINST_synch_event_id = DSE_exitEntry;
   DYNINST_synch_event_arg1 = arg1;
   /* Stop ourselves */
   DYNINSTbreakPoint();
   /* Once the stop completes, clean up */
   DYNINST_synch_event_id = DSE_undefined;
   DYNINST_synch_event_arg1 = NULL;
}

/* Used to instrument (and report) the entry of exit */
void DYNINST_instLoadLibrary(void *arg1) {
   /* Set the state so the mutator knows what's up */
   DYNINST_synch_event_id = DSE_loadLibrary;
   DYNINST_synch_event_arg1 = arg1;
   /* Stop ourselves */
   DYNINSTbreakPoint();
   /* Once the stop completes, clean up */
   DYNINST_synch_event_id = DSE_undefined;
   DYNINST_synch_event_arg1 = NULL;
}

/**
 * Used to report addresses of functions called at dynamic call sites 
 **/     
int DYNINSTasyncDynFuncCall (void * call_target, void *call_addr)
{
  int err = 0;
  int result;
  rtBPatch_asyncEventRecord ev;
  BPatch_dynamicCallRecord call_ev;

  result = tc_lock_lock(&DYNINST_trace_lock);
  if (result == DYNINST_DEAD_LOCK)
  {
     fprintf(stderr, "[%s:%d] - Error in libdyninstAPI_RT: trace pipe deadlock\n",
             __FILE__, __LINE__);
     return -1;
  }
 
  ev.type = rtBPatch_dynamicCallEvent;
  ev.pid = dyn_pid_self();
  err = DYNINSTwriteEvent((void *) &ev, sizeof(rtBPatch_asyncEventRecord));

  if (err) {
    fprintf(stderr, "%s[%d]:  write error\n",
            __FILE__, __LINE__);
    goto done;
  }

  call_ev.call_site_addr = call_addr;
  call_ev.call_target = call_target;
  err = DYNINSTwriteEvent((void *) &call_ev, sizeof(BPatch_dynamicCallRecord));

  if (err) {
    fprintf(stderr, "%s[%d]:  write error\n",
            __FILE__, __LINE__);
    goto done;
  }

 done:
  tc_lock_unlock(&DYNINST_trace_lock);
  return err;
}

int DYNINSTuserMessage(void *msg, unsigned int msg_size)
{
  int err = 0, result;
  rtBPatch_asyncEventRecord ev;

  result = tc_lock_lock(&DYNINST_trace_lock);
  if (result == DYNINST_DEAD_LOCK)
  {
     fprintf(stderr, "[%s:%d] - Error in libdyninstAPI_RT: trace pipe deadlock\n",
             __FILE__, __LINE__);
     return -1;
  }

  ev.type = rtBPatch_userEvent;
  ev.pid = dyn_pid_self();
  ev.size = msg_size;
  err = DYNINSTwriteEvent((void *) &ev, sizeof(rtBPatch_asyncEventRecord));

  if (err) {
    fprintf(stderr, "%s[%d]:  write error\n",
            __FILE__, __LINE__);
    goto done;
  }

  err = DYNINSTwriteEvent(msg, msg_size);

  if (err) {
    fprintf(stderr, "%s[%d]:  write error\n",
            __FILE__, __LINE__);
    goto done;
  }

 done:
  tc_lock_unlock(&DYNINST_trace_lock);
  return err;
}

int tc_lock_init(tc_lock_t *t)
{
  t->mutex = 0;
  t->tid = (dyntid_t) -1;
  return 0;
}

int tc_lock_unlock(tc_lock_t *t)
{
  t->mutex = 0;
  t->tid = (dyntid_t) -1;
  return 0;
}
    
int tc_lock_destroy(tc_lock_t *t)
{
  t->mutex = 0;
  t->tid = (dyntid_t) -1;
  return 0;
}

void dyninst_init_lock(dyninst_lock_t *lock)
{
   lock->mutex = 0;
   lock->tid = (dyntid_t) -1;
}

void dyninst_free_lock(dyninst_lock_t *lock)
{
}

int dyninst_lock(dyninst_lock_t *lock)
{
   return tc_lock_lock(lock);
}

void dyninst_unlock(dyninst_lock_t *lock)
{
   tc_lock_unlock(lock);
}

unsigned dyninst_maxNumOfThreads()
{
#if !defined(cap_threads)
   return 1;
#else
   if (!DYNINSThasInitialized)
      return 0;
   if (!DYNINST_multithread_capable)
      return 1;
   return DYNINST_max_num_threads;
#endif
}
