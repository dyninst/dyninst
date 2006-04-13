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

/* $Id: RTcommon.c,v 1.58 2006/04/13 19:20:37 jaw Exp $ */

#include <assert.h>
#include <stdlib.h>
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "RTcommon.h"
#include "RTthread.h"

#if defined(rs6000_ibm_aix4_1)
#include <sys/mman.h>
#include <sys/types.h>
#endif

unsigned int DYNINSTobsCostLow;
unsigned int DYNINSThasInitialized;
unsigned DYNINST_max_num_threads;
struct DYNINST_bootstrapStruct DYNINST_bootstrap_info;
char gLoadLibraryErrorString[ERROR_STRING_LENGTH];
void *gBRKptr;
int DYNINSTdebugRTlib;

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
int libdyninstAPI_RT_init_debug_flag=0;

int DYNINST_mutatorPid;
int DYNINSTdebugPrintRT = 0;
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
#if defined(rs6000_ibm_aix4_1)
	/* 	WHAT IS GOING ON HERE?
		For save the world to work, the DYNINST_tramp_guards must be reloaded in the
		exact same place when the mutated binary is run.  Normally, DYNINST_tramp_guards
		is allocated by malloc (See below).  This does not work for AIX because before
		we get to the init section of the RT lib, which reloads DYNINST_tramp_guards, the
		address DYNINST_tramp_guards needs to be loaded at has been allocated and is in
		use.  To get around this, I am using mmap to allocate DYNINST_tramp_guards on
		AIX.  I am allocating to a MAP_VARIABLE location because there is no address that
		is always known to be free that we can allocate.  This way, when the init section
		of the RT lib is run this address will probably be free (with a little magic in 
		loadFile() in RTmutatedBinary_XCOFF.c.

		Just in case we cannot mmap the space we need here (at this point in execution I
		cannot imagine this happening, if you want to do anything useful with Dyninst) I fall
		back to malloc and print a warning that save the world will probably not work.
	*/
	
/* http://publibn.boulder.ibm.com/doc_link/en_US/a_doc_lib/aixprggd/genprogc/sys_mem_alloc.htm#HDRA9E4A4C9921SYLV */
	unsigned int memLoc = sbrk(0);
	int pageSize = getpagesize();
	int arraySize = ((maxthreads +1) * sizeof(unsigned));
	arraySize -= (arraySize  % pageSize);
	arraySize +=  pageSize;

	/*fprintf(stderr,"TRAMP GUARDS (sbrk 0x%x)\n",sbrk(0));*/

	/*fprintf(stderr, "MMAP: 0x%x,0x%x,,,1,0x0\n",memLoc,arraySize);*/
	DYNINST_tramp_guards = (unsigned *) mmap(0x0, arraySize, PROT_READ|PROT_WRITE, MAP_VARIABLE|MAP_ANONYMOUS, -1,0x0);

	if (DYNINST_tramp_guards == -1){
		perror("mmap: DYNINST_tramp_guards ");
		fprintf(stderr,"Warning: DYNINST_tramp_guards may not be allocated correctly for save the world code!\n");
   		DYNINST_tramp_guards = (unsigned *) malloc((maxthreads+1)*sizeof(unsigned));
	}

	/*fprintf(stderr,"TRAMP GUARDS 0x%x (sbrk 0x%x)\n", DYNINST_tramp_guards,sbrk(0));*/

#else
   //We allocate maxthreads+1 because maxthreads is sometimes used as an
   //error value to mean we don't know what thread this is.  Sometimes used
   //during the early setup phases.
   DYNINST_tramp_guards = (unsigned *) malloc((maxthreads+1)*sizeof(unsigned));
#endif
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
void DYNINSTinit(int cause, int pid, int maxthreads, int debug_flag)
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
   DYNINSTdebugRTlib = debug_flag; /* set by mutator on request */

   /* sanity check */
   assert(sizeof(int64_t) == 8);
   assert(sizeof(int32_t) == 4);
   
   initTrampGuards(DYNINST_max_num_threads);

#if defined(cap_async_events)
   /* Done mutator-side to remove race */
   /*DYNINSTasyncConnect(DYNINST_mutatorPid);*/
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
   DYNINSTsafeBreakPoint();
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
   DYNINSTsafeBreakPoint();
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
   DYNINSTsafeBreakPoint();
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
   DYNINSTsafeBreakPoint();
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
   DYNINSTsafeBreakPoint();
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
   DYNINSTsafeBreakPoint();
   /* Once the stop completes, clean up */
   DYNINST_synch_event_id = DSE_undefined;
   DYNINST_synch_event_arg1 = NULL;
}

/* Used to instrument (and report) the entry of exit */
void DYNINST_instLwpExit(void) {
   /* Set the state so the mutator knows what's up */
   DYNINST_synch_event_id = DSE_lwpExit;
   DYNINST_synch_event_arg1 = NULL;
   /* Stop ourselves */
   DYNINSTsafeBreakPoint();
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

  rtdebug_printf("%s[%d]:  welcome to DYNINSTasyncDynFuncCall\n", __FILE__, __LINE__);
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

  rtdebug_printf("%s[%d]:  leaving DYNINSTasyncDynFuncCall: status = %s\n", 
                 __FILE__, __LINE__, err ? "error" : "ok");
  return err;
}

int DYNINSTuserMessage(void *msg, unsigned int msg_size)
{
  int err = 0, result;
  rtBPatch_asyncEventRecord ev;

  rtdebug_printf("%s[%d]:  welcome to DYNINSTuserMessage\n", __FILE__, __LINE__);
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
  rtdebug_printf("%s[%d]:  leaving DYNINSTuserMessage: status = %s\n", 
                 __FILE__, __LINE__, err ? "error" : "ok");
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

#if !(os_solaris==8)
int rtdebug_printf(const char *format, ...)
{
  if (!DYNINSTdebugRTlib) return 0;
  if (NULL == format) return -1;

  fprintf(stderr, "[RTLIB]");
  va_list val;
  va_start(val, format);
  int ret = vfprintf(stderr, format, val);
  va_end(val);

  return ret;
}
#endif

#ifndef CASE_RETURN_STR
#define CASE_RETURN_STR(x) case x: return #x
#endif

const char *asyncEventType2str(rtBPatch_asyncEventType t)
{
  switch (t) {
  CASE_RETURN_STR(rtBPatch_nullEvent);
  CASE_RETURN_STR(rtBPatch_newConnectionEvent);
  CASE_RETURN_STR(rtBPatch_internalShutDownEvent);
  CASE_RETURN_STR(rtBPatch_threadCreateEvent);
  CASE_RETURN_STR(rtBPatch_threadDestroyEvent);
  CASE_RETURN_STR(rtBPatch_dynamicCallEvent);
  CASE_RETURN_STR(rtBPatch_userEvent);
  default: return "bad_async_event_type";
  }
  return "bad_async_event_type";
} 

