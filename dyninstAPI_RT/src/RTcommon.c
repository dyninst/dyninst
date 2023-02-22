/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* $Id: RTcommon.c,v 1.78 2008/04/15 16:43:44 roundy Exp $ */

#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "RTcommon.h"
#include "RTthread.h"

unsigned int DYNINSTobsCostLow;
DLLEXPORT unsigned int DYNINSThasInitialized = 0;
struct DYNINST_bootstrapStruct DYNINST_bootstrap_info;
char gLoadLibraryErrorString[ERROR_STRING_LENGTH];
int DYNINSTdebugRTlib = 0;

DLLEXPORT int DYNINSTstaticMode = 1;

/**
 * Allocate the Dyninst heaps
 *
 * The IA-64 requires that instruction be 16-byte aligned, so we have to
 * align the heaps if we want to use them for inferior RPCs.
 **/
#define HEAP_TYPE double
#define ALIGN_ATTRIB

#if defined(os_linux) || defined(os_freebsd)
#define HEAP_LOCAL extern
#else
#define HEAP_LOCAL DLLEXPORT
#endif

HEAP_LOCAL HEAP_TYPE DYNINSTglobalData[SYN_INST_BUF_SIZE/sizeof(HEAP_TYPE)] ALIGN_ATTRIB;

#if !defined(target_smallmem)
HEAP_LOCAL HEAP_TYPE DYNINSTstaticHeap_512K_lowmemHeap_1[512*1024/sizeof(HEAP_TYPE)] ALIGN_ATTRIB;
HEAP_LOCAL HEAP_TYPE DYNINSTstaticHeap_16M_anyHeap_1[16*1024*1024/sizeof(HEAP_TYPE)] ALIGN_ATTRIB;
/* These are necessary due to silly C-style 'extern'/linking conventions. */
const unsigned long sizeOfLowMemHeap1 = sizeof( DYNINSTstaticHeap_512K_lowmemHeap_1 );
const unsigned long sizeOfAnyHeap1 = sizeof( DYNINSTstaticHeap_16M_anyHeap_1 );
#else
HEAP_LOCAL HEAP_TYPE DYNINSTstaticHeap_8K_lowmemHeap_1[8*1024/sizeof(HEAP_TYPE)] ALIGN_ATTRIB;
HEAP_LOCAL HEAP_TYPE DYNINSTstaticHeap_32K_anyHeap_1[32*1024/sizeof(HEAP_TYPE)] ALIGN_ATTRIB;
/* These are necessary due to silly C-style 'extern'/linking conventions. */
const unsigned long sizeOfLowMemHeap1 = sizeof( DYNINSTstaticHeap_8K_lowmemHeap_1 );
const unsigned long sizeOfAnyHeap1 = sizeof( DYNINSTstaticHeap_32K_anyHeap_1 );
#endif

/**
 * One some platforms we can tell when a fork or exec is occurring through
 * system-provided events. On others we do it ourselves.  Enumerated type
 * defined in header file
 **/
DLLEXPORT DYNINST_synch_event_t DYNINST_synch_event_id = DSE_undefined;
DLLEXPORT void *DYNINST_synch_event_arg1;
/* Code to read args 2,3 would have to be moved from dyninstAPI's*/
/* process::handleStopThread to signalgenerator::decodeRTSignal in*/
/* order for other signals to make use of them, as currently only the*/
/* stopThread event needs makes use of them.  */
DLLEXPORT void *DYNINST_synch_event_arg2 = NULL; /* not read in dyninst's decodeRTSignal*/
DLLEXPORT void *DYNINST_synch_event_arg3 = NULL; /* not read in dyninst's decodeRTSignal*/
DLLEXPORT int DYNINST_break_point_event = 0;

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

// stopThread cache variables
char cacheLRUflags[TARGET_CACHE_WIDTH];
DLLEXPORT void *DYNINST_target_cache[TARGET_CACHE_WIDTH][TARGET_CACHE_WAYS];
FILE *stOut;
int fakeTickCount;


#ifdef _MSC_VER
#define TLS_VAR __declspec(thread)
#else
// Note, the initial-exec model gives us static TLS which can be accessed
// directly, unlike dynamic TLS that calls __tls_get_addr().  Such calls risk
// recursing back to us if they're also instrumented, ad infinitum.  Static TLS
// must be used very sparingly though, because it is a limited resource.
// *** This case is very special -- do not use IE in general libraries! ***

#if defined(DYNINST_RT_STATIC_LIB)
#define TLS_VAR __thread __attribute__ ((tls_model("local-exec")))
#else
#define TLS_VAR __thread __attribute__ ((tls_model("initial-exec")))
#endif
#endif

// It's tempting to make this a char, but glibc < 2.17 hits a bug:
//   https://sourceware.org/bugzilla/show_bug.cgi?id=14898
static TLS_VAR short DYNINST_tls_tramp_guard = 1;

DLLEXPORT int DYNINST_lock_tramp_guard(void)
{
  if(!DYNINST_tls_tramp_guard) return 0;
  DYNINST_tls_tramp_guard = 0;
  return 1;
}
DLLEXPORT void DYNINST_unlock_tramp_guard(void)
{
  DYNINST_tls_tramp_guard = 1;
}

DECLARE_DYNINST_LOCK(DYNINST_trace_lock);

/**
 * Init the FPU.  We've seen bugs with Linux (e.g., Redhat 6.2 stock kernel on
 * PIIIs) where processes started by Paradyn started with FPU uninitialized.
 * DYNINSTdummydouble is global so the compiler won't optimize away FP code
 * in initFPU
 **/
double DYNINSTdummydouble = 4321.71;
static void initFPU(void)
{
   double x = 17.1234;
   DYNINSTdummydouble *= x;
}

/**
 * This function is called in both static and dynamic rewriting, on
 * all platforms that support binary rewriting, but before DYNINSTinit
 **/
void DYNINSTBaseInit(void)
{
#if defined(cap_mutatee_traps)
   DYNINSTinitializeTrapHandler();
#endif
   DYNINST_unlock_tramp_guard();
   DYNINSThasInitialized = 1;
}

/**
 * The Dyninst API arranges for this function to be called at the entry to
 * main() via libdyninstAPI_RT_init (defined in RTposix.c and RTwinnt.c).
 * libdyninstAPI_RT_init is called by one of the following methods:
 *    GCC: link with gcc -shared, and use __attribute__((constructor));
 *    Linux: ld with -init libdyninstAPI_RT_init
 *           gcc with -Wl,-init -Wl,...
 *    Windows: called from DllMain, which exists in lieu of libdyninstAPI_RT_init
 *
 * This is only called in the Dynamic instrumentation case.  Static
 * libraries don't call this.
 **/
void DYNINSTinit(void)
{
   rtdebug_printf("%s[%d]:  DYNINSTinit:  welcome to DYNINSTinit()\n", __FILE__, __LINE__);
   initFPU();
   mark_heaps_exec();

   tc_lock_init(&DYNINST_trace_lock);
   DYNINSThasInitialized = 1;
   rtdebug_printf("%s[%d]:  welcome to DYNINSTinit\n", __FILE__, __LINE__);

   /* sanity check */
   assert(sizeof(int64_t) == 8);
   assert(sizeof(int32_t) == 4);

   /* defensive stuff */
   memset(DYNINST_target_cache,
          0,
          sizeof(void*) * TARGET_CACHE_WIDTH * TARGET_CACHE_WAYS);
   memset(cacheLRUflags, 1, sizeof(char)*TARGET_CACHE_WIDTH);
   // stOut = fopen("rtdump.txt","w");

   rtdebug_printf("%s[%d]:  leaving DYNINSTinit\n", __FILE__, __LINE__);
   fakeTickCount=0;
   /* Memory emulation */
}

/**
 * Does what it's called. Used by the paradyn daemon as a default in certain
 * cases (MT in particular)
 **/
int DYNINSTreturnZero(void)
{
   return 0;
}

/* Used to by dyninst breakpoint snippet */
void DYNINST_snippetBreakpoint(void) {
   tc_lock_lock(&DYNINST_trace_lock);

   /* Set the state so the mutator knows what's up */
   DYNINST_synch_event_id = DSE_snippetBreakpoint;
   DYNINST_synch_event_arg1 = NULL;
   /* Stop ourselves */
   DYNINSTbreakPoint();
   /* Once the stop completes, clean up */
   DYNINST_synch_event_id = DSE_undefined;

   tc_lock_unlock(&DYNINST_trace_lock);
}

/* Used to instrument (and report) the entry of fork */
DLLEXPORT void DYNINST_instForkEntry(void) {
   tc_lock_lock(&DYNINST_trace_lock);

   /* Set the state so the mutator knows what's up */
   DYNINST_synch_event_id = DSE_forkEntry;
   DYNINST_synch_event_arg1 = NULL;
   /* Stop ourselves */
   DYNINSTbreakPoint();
   /* Once the stop completes, clean up */
   DYNINST_synch_event_id = DSE_undefined;
   DYNINST_synch_event_arg1 = NULL;

   tc_lock_unlock(&DYNINST_trace_lock);
}


/* Used to instrument (and report) the exit of fork */
/* We use the safe breakpoint on the child side of fork
   as we may not be attached at that point. The parent
   side uses the normal version. */
DLLEXPORT void DYNINST_instForkExit(void *arg1) {
//#warning "This function is not implemented for AARCH64 yet!"
#if !defined(arch_aarch64)
   tc_lock_lock(&DYNINST_trace_lock);

   /* Set the state so the mutator knows what's up */
   DYNINST_synch_event_id = DSE_forkExit;
   DYNINST_synch_event_arg1 = arg1;
   /* Stop ourselves */
   if ((long int)arg1 == 0) {
       /* Child... */
       DYNINSTsafeBreakPoint();
   }
   else {
       DYNINSTbreakPoint();
   }
   /* Once the stop completes, clean up */
   DYNINST_synch_event_id = DSE_undefined;
   DYNINST_synch_event_arg1 = NULL;

   tc_lock_unlock(&DYNINST_trace_lock);
#else
   // Silence compiler warnings
   (void)arg1;
   assert(0);
#endif
}


/* Used to instrument (and report) the entry of exec */
DLLEXPORT void DYNINST_instExecEntry(void *arg1) {
//#warning "This function is not implemented for AARCH64 yet!"
#if !defined(arch_aarch64)
   tc_lock_lock(&DYNINST_trace_lock);

   /* Set the state so the mutator knows what's up */
   DYNINST_synch_event_id = DSE_execEntry;
   DYNINST_synch_event_arg1 = arg1;
   /* Stop ourselves */
   DYNINSTbreakPoint();
   /* Once the stop completes, clean up */
   DYNINST_synch_event_id = DSE_undefined;
   DYNINST_synch_event_arg1 = NULL;

   tc_lock_unlock(&DYNINST_trace_lock);
#endif
   // Silence compiler warnings
   (void)arg1;
}


/* Used to instrument (and report) the exit of exec */
DLLEXPORT void DYNINST_instExecExit(void *arg1) {
   tc_lock_lock(&DYNINST_trace_lock);

   /* Set the state so the mutator knows what's up */
   DYNINST_synch_event_id = DSE_execExit;
   DYNINST_synch_event_arg1 = arg1;
   /* Stop ourselves */
   DYNINSTbreakPoint();
   /* Once the stop completes, clean up */
   DYNINST_synch_event_id = DSE_undefined;
   DYNINST_synch_event_arg1 = NULL;

   tc_lock_unlock(&DYNINST_trace_lock);
}


/* Used to instrument (and report) the entry of exit */
DLLEXPORT void DYNINST_instExitEntry(void *arg1) {
   tc_lock_lock(&DYNINST_trace_lock);

   /* Set the state so the mutator knows what's up */
   DYNINST_synch_event_id = DSE_exitEntry;
   DYNINST_synch_event_arg1 = arg1;
   /* Stop ourselves */
   DYNINSTbreakPoint();
   /* Once the stop completes, clean up */
   DYNINST_synch_event_id = DSE_undefined;
   DYNINST_synch_event_arg1 = NULL;

   tc_lock_unlock(&DYNINST_trace_lock);
}

/* Used to instrument (and report) the entry of exit */
DLLEXPORT void DYNINST_instLoadLibrary(void *arg1) {
   tc_lock_lock(&DYNINST_trace_lock);

   /* Set the state so the mutator knows what's up */
   DYNINST_synch_event_id = DSE_loadLibrary;
   DYNINST_synch_event_arg1 = arg1;
   /* Stop ourselves */
   DYNINSTbreakPoint();
   /* Once the stop completes, clean up */
   DYNINST_synch_event_id = DSE_undefined;
   DYNINST_synch_event_arg1 = NULL;

   tc_lock_unlock(&DYNINST_trace_lock);
}

/* Used to instrument (and report) the entry of exit */
DLLEXPORT void DYNINST_instLwpExit(void) {
   tc_lock_lock(&DYNINST_trace_lock);

   /* Set the state so the mutator knows what's up */
   DYNINST_synch_event_id = DSE_lwpExit;
   DYNINST_synch_event_arg1 = NULL;
   /* Stop ourselves */
   DYNINSTbreakPoint();
   /* Once the stop completes, clean up */
   DYNINST_synch_event_id = DSE_undefined;
   DYNINST_synch_event_arg1 = NULL;

   tc_lock_unlock(&DYNINST_trace_lock);
}


// implementation of an N=2 way associative cache to keep access time low
// width = 32
// the cache contains valid addresses
// add to the cache when an instrumented instruction misses in the cache
// update flags for the cache when an instrumented instruction hits in the cache
// instrumentation will take the form of a call to cache check
RT_Boolean cacheLookup(void *calculation)
{
    int index = ((unsigned long) calculation) % TARGET_CACHE_WIDTH;
    if (DYNINST_target_cache[index][0] == calculation) {
        cacheLRUflags[index] = 0;
        return RT_TRUE;
    } else if (DYNINST_target_cache[index][1] == calculation) {
        cacheLRUflags[index] = 1;
        return RT_TRUE;
    } else { //miss
        if (cacheLRUflags[index] == 0) {
            DYNINST_target_cache[index][1] = calculation;
            cacheLRUflags[index] = 1;
        } else {
            DYNINST_target_cache[index][0] = calculation;
            cacheLRUflags[index] = 0;
        }
        return RT_FALSE;
    }
}

/**
 * Receives two snippets as arguments and stops the mutatee
 * while the mutator reads the arguments, saved to
 * DYNINST_synch_event... global variables.
 *
 * if flag useCache==1, does a cache lookup and stops only
 * if there is a cache miss
 *
 * The flags are:
 * bit 0: useCache
 * bit 1: true if interpAsTarget
 * bit 2: true if interpAsReturnAddr
 **/
//#define STACKDUMP
void DYNINST_stopThread (void * pointAddr, void *callBackID,
                         void *flags, void *calculation)
{
	static int reentrant = 0;

    RT_Boolean isInCache = RT_FALSE;


	if (reentrant == 1) {
		return;
	}
	reentrant = 1;
    tc_lock_lock(&DYNINST_trace_lock);
    rtdebug_printf("RT_st: pt[%lx] flags[%lx] calc[%lx] ",
                   (unsigned long)pointAddr, (unsigned long)flags, (unsigned long)calculation);

#if 0 && defined STACKDUMP
    //if (0 && ((unsigned long)calculation == 0x9746a3 ||
    //          (unsigned long)calculation == 0x77dd761b))
    //{
        fprintf(stOut,"RT_st: %lx(%lx)\n", (long)pointAddr,&calculation);
        fprintf(stOut,"at instr w/ targ=%lx\n",(long)calculation);
        for (bidx=0; bidx < 0x100; bidx+=4) {
            fprintf(stOut,"0x%x:  ", (int)stackBase+bidx);
            fprintf(stOut,"%02hhx", stackBase[bidx]);
            fprintf(stOut,"%02hhx", stackBase[bidx+1]);
            fprintf(stOut,"%02hhx", stackBase[bidx+2]);
            fprintf(stOut,"%02hhx", stackBase[bidx+3]);
            fprintf(stOut,"\n");
        }
    //}
    // fsg: read from 40a4aa, how did it become 40a380?
#endif

    if ((((long)flags) & 0x04) ) {
        rtdebug_printf("ret-addr stopThread yields %lx", (unsigned long)calculation);
        //fprintf(stderr,"[$0x%lx]\n", (long)calculation);
    }

    if (0 != (((long)flags) & 0x03)) {
        // do the lookup if the useCache bit is set, or if it represents
        // the address of real code, so that we add the address to the cache
        // even if we will stop the thread if there's a cache hit
        isInCache = cacheLookup(calculation);
    }

    // if the cache flag bit is not set, or if we get a cache miss,
    // stop the thread so that we can call back to the mutatee
    if (0 == (((long)flags) & 0x01) ||
        ! isInCache )
    {
        /* Set vars for Dyninst to read */
        DYNINST_synch_event_id = DSE_stopThread;
        DYNINST_synch_event_arg1 = pointAddr;
        DYNINST_synch_event_arg2 = callBackID;
        DYNINST_synch_event_arg3 = calculation;

        // encode interp as target or as return addr by making
        // callback ID negative
        if (0 != (((long)flags) & 0x06))
        {
            DYNINST_synch_event_arg2 =
                (void*) (-1 * (long)DYNINST_synch_event_arg2);
        }

        rtdebug_printf("stopping! isInCache=%d\n", isInCache);

        /* Stop ourselves */
        DYNINSTbreakPoint();

        /* Once the stop completes, clean up */
        DYNINST_synch_event_id = DSE_undefined;
        DYNINST_synch_event_arg1 = NULL;
        DYNINST_synch_event_arg2 = NULL;
        DYNINST_synch_event_arg3 = NULL;
    }
    fflush(stOut);
    tc_lock_unlock(&DYNINST_trace_lock);
	reentrant = 0;
    return;
}

// zeroes out the useCache flag if the call is interprocedural
void DYNINST_stopInterProc(void * pointAddr, void *callBackID,
                           void *flags, void *calculation,
                           void *objStart, void *objEnd)
{
#if defined STACKDUMP
    fprintf(stOut,"RT_sip: calc=%lx objStart=%lx objEnd=%lx\n",
            calculation, objStart, objEnd);
    fflush(stOut);
#endif
    if (calculation < objStart || calculation >= objEnd) {
       flags = (void*) ((long) (((int)((long)flags)) & 0xfffffffe));
    }
    DYNINST_stopThread(pointAddr, callBackID, flags, calculation);
}

// boundsArray is a sequence of (blockStart,blockEnd) pairs
DLLEXPORT RT_Boolean DYNINST_boundsCheck(void **boundsArray_, void *arrayLen_,
                                         void *writeTarget_)
{
    RT_Boolean callStopThread = RT_FALSE;
    const unsigned long writeTarget = (unsigned long)writeTarget_;
    const long arrayLen = (long)arrayLen_;
    unsigned long *boundsArray = (unsigned long*)boundsArray_;
    // set idx to halfway into the array, ensuring we use a blockStart address
    int idx = (int)arrayLen / 4 * 2;
    int lowIdx = 0;
    int highIdx = (int)arrayLen;
    //fprintf(stderr,"D_bc@%p: boundsArray=%p target=%lx idx=%d arrayLen=%d [%d]\n", (void*)DYNINST_boundsCheck, boundsArray_, writeTarget_, idx, arrayLen, __LINE__);
    //rtdebug_printf("D_bc@%p: boundsArray=%p target=%lx idx=%d arrayLen=%d [%d]\n", (void*)DYNINST_boundsCheck, boundsArray_, writeTarget_, idx, arrayLen, __LINE__);
    if ((unsigned long)boundsArray < 0x10000000) {
        printf("D_bc: boundsArray_ = %lx, returning false\n",(unsigned long) boundsArray);
        return RT_FALSE;
    }
    while (lowIdx < highIdx)
    {
        if (idx > arrayLen || idx < 0)
            rtdebug_printf("ERROR: out of bounds idx=%d, arrayLen = %ld [%d]\n", idx, arrayLen, __LINE__);
        rtdebug_printf("D_bc: low=%d high=%d arr[%d]=%lx [%d]\n", lowIdx, highIdx, idx, boundsArray[idx], __LINE__);
        if (writeTarget < boundsArray[idx]) {
            rtdebug_printf("D_bc: [%d]\n", __LINE__);
            highIdx = idx;
            idx = (highIdx - lowIdx) / 4 * 2 + lowIdx;
        }
        else if (boundsArray[idx+1] <= writeTarget) {
            rtdebug_printf("D_bc: [%d]\n", __LINE__);
            lowIdx = idx+2;
            idx = (highIdx - lowIdx) / 4 * 2 + lowIdx;
        }
        else {
            rtdebug_printf("D_bc: callST=true [%d]\n", __LINE__);
            callStopThread = RT_TRUE;
            break;
        }
    }
    rtdebug_printf("D_bc: boundsArray=%p ret=%d [%d]\n", (void*)boundsArray, callStopThread, __LINE__);
    return callStopThread;
}


/**
 * Used to report addresses of functions called at dynamic call sites
 **/
DLLEXPORT int DYNINSTasyncDynFuncCall (void * call_target, void *call_addr) {
    if (DYNINSTstaticMode) return 0;

    tc_lock_lock(&DYNINST_trace_lock);

    /* Set the state so the mutator knows what's up */
    DYNINST_synch_event_id = DSE_dynFuncCall;
    DYNINST_synch_event_arg1 = call_target;
    DYNINST_synch_event_arg2 = call_addr;
    /* Stop ourselves */
    DYNINSTbreakPoint();
    /* Once the stop completes, clean up */
    DYNINST_synch_event_id = DSE_undefined;
    DYNINST_synch_event_arg1 = NULL;
    DYNINST_synch_event_arg2 = NULL;

    tc_lock_unlock(&DYNINST_trace_lock);

    return 0;
}

int DYNINSTuserMessage(void *msg, unsigned int msg_size) {
    unsigned long msg_size_long = (unsigned long)msg_size;
    if (DYNINSTstaticMode)
	{
		return 0;
	}

    tc_lock_lock(&DYNINST_trace_lock);


    /* Set the state so the mutator knows what's up */
    DYNINST_synch_event_id = DSE_userMessage;
    DYNINST_synch_event_arg1 = msg;
    DYNINST_synch_event_arg2 = (void *)msg_size_long;
    /* Stop ourselves */
    DYNINSTbreakPoint();
    /* Once the stop completes, clean up */
    DYNINST_synch_event_id = DSE_undefined;
    DYNINST_synch_event_arg1 = NULL;
    DYNINST_synch_event_arg2 = NULL;

    tc_lock_unlock(&DYNINST_trace_lock);

    return 0;
}

int tc_lock_init(tc_lock_t *t)
{
  t->mutex = 0;
  t->tid = (dyntid_t) DYNINST_INITIAL_LOCK_PID;
  return 0;
}

int tc_lock_unlock(tc_lock_t *t)
{
  t->tid = (dyntid_t) DYNINST_INITIAL_LOCK_PID;
  t->mutex = 0;
  return 0;
}

int tc_lock_destroy(tc_lock_t *t)
{
  t->tid = (dyntid_t) DYNINST_INITIAL_LOCK_PID;
  t->mutex = 0;
  return 0;
}

void dyninst_init_lock(dyninst_lock_t *lock)
{
   lock->tid = (dyntid_t) DYNINST_INITIAL_LOCK_PID;
   lock->mutex = 0;
}

void dyninst_free_lock(dyninst_lock_t *lock)
{
    (void)lock; /* unused parameter */
}

int dyninst_lock(dyninst_lock_t *lock)
{
   return tc_lock_lock(lock);
}

void dyninst_unlock(dyninst_lock_t *lock)
{
   tc_lock_unlock(lock);
}

int rtdebug_printf(const char *format, ...)
{
  int ret;
  va_list va;
  if (!DYNINSTdebugRTlib) return 0;
  if (NULL == format) return DYNINST_PRINTF_ERRVAL;

  fprintf(stderr, "[RTLIB]");
  va_start(va, format);
  ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

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
}

DLLEXPORT volatile unsigned long dyninstTrapTableUsed;
DLLEXPORT volatile unsigned long dyninstTrapTableVersion;
DLLEXPORT volatile trapMapping_t *dyninstTrapTable;
DLLEXPORT volatile unsigned long dyninstTrapTableIsSorted;

void* dyninstTrapTranslate(void *source,
                           volatile unsigned long *table_used,
                           volatile unsigned long *table_version,
                           volatile trapMapping_t **trap_table,
                           volatile unsigned long *is_sorted)
{
   volatile unsigned local_version;
   unsigned i;
   void *target;

   do {
      local_version = *table_version;
      target = NULL;

      if (*is_sorted)
      {
         unsigned min = 0;
         unsigned mid = 0;
         unsigned max = *table_used;
         unsigned prev = max+1;

         for (;;) {
            mid = (min + max) / 2;
            if (mid == prev) {
                fprintf(stderr,"ERROR: dyninstTrapTranslate couldn't find "
                        "entry for %p: min=%x mid=%x max=%x prev=%x\n",
                        source,min,mid,max,prev);
               break;
            }
            prev = mid;

            if ((*trap_table)[mid].source < source)
               min = mid;
            else if ((*trap_table)[mid].source > source)
               max = mid;
            else {
               target = (*trap_table)[mid].target;
               break;
            }
         }
      }
      else { /*!dyninstTrapTableIsSorted*/
         for (i = 0; i<*table_used; i++) {
            if ((*trap_table)[i].source == source) {
               target = (*trap_table)[i].target;
               break;
            }
         }
      }
   } while (local_version != *table_version);

   return target;
}

DLLEXPORT void DYNINSTtrapFunction(void){
   __asm__ __volatile__(
           "nop\n"
           :::);
}


