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
 * $Id: RTinst.c,v 1.90 2005/10/10 18:46:02 legendre Exp $
 * RTinst.c: platform independent runtime instrumentation functions
 *
 ************************************************************************/

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>

#if defined(os_windows)
#include <process.h>
#define getpid _getpid
#else
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "RTcompat.h"
#include "pdutil/h/resource.h"
#include "dyninstAPI_RT/h/dyninstRTExport.h"

extern const char V_libparadyn[];

virtualTimer *virtualTimers = NULL;
unsigned *RTobserved_cost = NULL;

extern void (*newthr_cb)(int);
extern void newPDThread(int index);

/* platform dependent functions */
extern void PARADYNos_init(int calledByFork, int calledByAttach);

static void *PARADYN_shmSegAttachedPtr;

/* These variables are used for the rtinst library to suggest to the daemon
   which timer level the daemon should setup for the rtinst library.  The
   daemon has the responsibility for making the final decision.  The purpose
   for this is that sometimes the rtinst library only has access to needed to
   make decision and sometimes the decision is best handled by the daemon. */
int hintBestCpuTimerLevel  = UNASSIGNED_TIMER_LEVEL;
int hintBestWallTimerLevel = UNASSIGNED_TIMER_LEVEL;

/* These are used to assign to PARADYNgetCPUtime.  On some systems like AIX,
   a function pointer is actually a pointer to a structure which then points
   to the function.  In the case of AIX, these variables will contain the
   address of the structure (with a member that points to the function).  The
   daemon can then read these variables to find the address of the structure.
   On other systems, these variables just store the actual function address.
   FPtrInfo = the address of the object which gives information as to how to
              call a function pointer
*/
timeQueryFuncPtr_t swCpuTimeFPtrInfo  = &DYNINSTgetCPUtime_sw;
timeQueryFuncPtr_t hwCpuTimeFPtrInfo  = &DYNINSTgetCPUtime_hw;
timeQueryFuncPtr_t swWallTimeFPtrInfo = &DYNINSTgetWalltime_sw;
timeQueryFuncPtr_t hwWallTimeFPtrInfo = &DYNINSTgetWalltime_hw;

/* Set time retrieval functions to the software level.  The daemon
   will reset these to a "better" time querying function if available. */
timeQueryFuncPtr_t PARADYNgetCPUtime  = &DYNINSTgetCPUtime_sw;
timeQueryFuncPtr_t PARADYNgetWalltime = &DYNINSTgetWalltime_sw;

void PARADYN_initialize_once();


/************************************************************************
 * void DYNINSTstartProcessTimer(tTimer* timer)
************************************************************************/
void DYNINSTstartProcessTimer(tTimer* timer) 
{
   /* For shared-mem sampling only: bump protector1, do work, then bump
      protector2 */
   assert(timer->protector1 == timer->protector2);
   timer->protector1++;
   MEMORY_BARRIER;
   if (timer->counter == 0) {
      timer->start     =  DYNINSTgetCPUtime();
   }
   timer->counter++;
   MEMORY_BARRIER;
   timer->protector2++; /* ie. timer->protector2 == timer->protector1 */
   assert(timer->protector1 == timer->protector2);
}


/************************************************************************
 * void DYNINSTstopProcessTimer(tTimer* timer)
 ************************************************************************/
void DYNINSTstopProcessTimer(tTimer* timer) 
{
   assert(timer->protector1 == timer->protector2);
   timer->protector1++;
   MEMORY_BARRIER;
   if (timer->counter == 0) {
      /* a strange condition; shouldn't happen.  Should we make it an assert
         fail? */
   }
   else {
      if (timer->counter == 1) {
         const rawTime64  now = DYNINSTgetCPUtime();
         timer->total += (now - timer->start);
         
         if (now < timer->start) {
            abort();
         }
      }
      timer->counter--;
   }
   MEMORY_BARRIER;
   timer->protector2++; /* ie. timer->protector2 == timer->protector1 */
   assert(timer->protector1 == timer->protector2);
}

/************************************************************************
 * void DYNINSTstartWallTimer(tTimer* timer)
 ************************************************************************/
void DYNINSTstartWallTimer(tTimer* timer) 
{
   //unsigned index = DYNINSTthreadIndexSLOW(P_thread_self());
   //virtualTimer *vt = &(virtualTimers[index]);
   //fprintf(stderr, "start wall-timer, lwp is %d\n", vt->lwp);
   
   assert(timer->protector1 == timer->protector2);
   timer->protector1++;
   MEMORY_BARRIER;
   if (timer->counter == 0) {
      timer->start = DYNINSTgetWalltime();
      //XXX
      //    fprintf(stderr, "timer 0x%x start\n", &(*timer));
   }
   timer->counter++;
   MEMORY_BARRIER;
   timer->protector2++; /* ie. timer->protector2 == timer->protector1 */
   assert(timer->protector1 == timer->protector2);
}


/************************************************************************
 * void DYNINSTstopWallTimer(tTimer* timer)
************************************************************************/
void DYNINSTstopWallTimer(tTimer* timer) 
{
   //unsigned index = DYNINSTthreadIndexSLOW(P_thread_self());
   //virtualTimer *vt = &(virtualTimers[index]);
   //fprintf(stderr, "stop wall-timer, lwp is %d\n", vt->lwp);

   assert(timer->protector1 == timer->protector2);
   timer->protector1++;
   MEMORY_BARRIER;
   if (timer->counter == 0)
      ;
   else if (--timer->counter == 0) {
      const rawTime64 now = DYNINSTgetWalltime();
      
      timer->total += (now - timer->start);
      
      //XXX
      //fprintf(stderr, "timer 0x%x stop [total %u]\n", &(*timer), timer->total);
   }
   MEMORY_BARRIER;
   timer->protector2++; /* ie. timer->protector2 == timer->protector1 */
   assert(timer->protector1 == timer->protector2);
}

/************************************************************************
 * void PARADYNinit()
 *
 * initialize the Paradyn library.  this function is called at the start
 * of the application program, as well as after a fork, and after an
 * attach.
 *
 ************************************************************************/
void PARADYNinit(int paradyndPid, int creationMethod,
                 virtualTimer *virtualTimersAddr, int *obsCostAddr) 
{
    int calledFromAttachToCreated = (creationMethod == 3);
    int calledFromFork = (creationMethod == 2);
    int calledFromAttach = (creationMethod == 1);
    int calledFromExec = (creationMethod == 4);
    
   /* sanity check */
   assert(sizeof(rawTime64) == 8);
   assert(sizeof(int64_t) == 8);
   assert(sizeof(int32_t) == 4);
 
   /* initialize the tag and group info */
#if !defined(os_windows)
   PARADYNtagGroupInfo_Init();
   PARADYNWinInfo_Init();
#endif

   virtualTimers = virtualTimersAddr;
   RTobserved_cost = obsCostAddr;

#if defined(cap_threads)
   newthr_cb = newPDThread;
   newPDThread(0);
#endif

   /*
     In accordance with usual stdio rules, stdout is line-buffered and stderr
     is non-buffered.  Unfortunately, stdio is a little clever and when it
     detects stdout/stderr redirected to a pipe/file/whatever, it changes to
     fully-buffered.  This indeed occurs with us (see paradynd/src/process.C
     to see how a program's stdout/stderr are redirected to a pipe). So we
     reset back to the desired "bufferedness" here.  See stdio.h for these
     calls.  When attaching, stdio isn't under paradynd control, so we don't
     do this stuff.

     Note! Since we are messing with stdio stuff here, it should go without
     saying that PARADYNinit() (or at least this part of it) shouldn't be
     invoked until stdio has been initialized!
   */
   if (!calledFromAttach) {
      setvbuf(stdout, NULL, _IOLBF, 0);
      /* make stdout line-buffered.  "setlinebuf(stdout)" is cleaner but HP
         doesn't seem to have it */
      setvbuf(stderr, NULL, _IONBF, 0);
      /* make stderr non-buffered */
   }
  
   PARADYNos_init(calledFromFork, calledFromAttach);

   /* Note: until this point we are not multithread-safe. */
   PARADYN_initialize_once();
}

/* debug code should call forkexec_printf as a way of avoiding
   putting #ifdef FORK_EXEC_DEBUG around their fprintf(stderr, ...) statements,
   which can lead to horribly ugly code --ari */
/* similar for SHM_SAMPLING_DEBUG */
void forkexec_printf(const char *fmt, ...) {
#ifdef FORK_EXEC_DEBUG
   va_list args;
   va_start(args, fmt);

   vfprintf(stderr, fmt, args);

   va_end(args);

   fflush(stderr);
#endif
}

#if !defined(os_windows)
int PARADYN_init_child_after_fork() {
   int pid;
   int ppid;
   int ptr_size;
   int cookie = 0;
   /* we are the child process */

   /* don't want to call alloc_pos with fork, but want to reuse existing
      positions.  This is setup through DYNINSTregister_running_thread() */

   /* Used to unset DYNINST_initialize_done; this is now taken care of
      by only using ST-style RPCs */
   
   pid = getpid();
   ppid = getppid();
   ptr_size = sizeof(PARADYN_shmSegAttachedPtr);

   setvbuf(stdout, NULL, _IOLBF, 0);
   /* make stdout line-buffered.  "setlinebuf(stdout)" is cleaner but HP
      doesn't seem to have it */
   setvbuf(stderr, NULL, _IONBF, 0);
   /* make stderr non-buffered */
   
   /* Some aspects of initialization need to occur right away (such
      as resetting the PMAPI counters on AIX) because the daemon
      may use aspects of the process before pDYNINSTinit is called */
   PARADYN_forkEarlyInit();
   PARADYNos_init(1, 0);

   DYNINSTinitTrace(0);

   DYNINSTwriteTrace(&cookie, sizeof(cookie));
   DYNINSTwriteTrace(&pid, sizeof(pid));
   DYNINSTwriteTrace(&ppid, sizeof(ppid));
   ptr_size = sizeof(PARADYN_shmSegAttachedPtr);
   DYNINSTwriteTrace(&ptr_size, sizeof(int32_t));
   DYNINSTwriteTrace(&PARADYN_shmSegAttachedPtr, ptr_size);
   DYNINSTflushTrace();

   return 123;
}

void DYNINSTmpi_fork(int pid) {
	if (pid == 0) {
		forkexec_printf("DYNINSTmpi_fork: child\n");

		if (DYNINSTnullTrace() < 0) {
			return;
		}
		forkexec_printf("DYNINSTmpi_fork: child done\n");
	}
}
#endif

void DYNINSTexecFailed() {
    /* PARADYNgenerateTraceRecord resets errno back to zero */
    int saved = errno;
    rawTime64 process_time = DYNINSTgetCPUtime();
    rawTime64 wall_time = DYNINSTgetWalltime();
    int pid = getpid();
    
    forkexec_printf("DYNINSTexecFAILED errno = %d\n", errno);
    
    PARADYNgenerateTraceRecord(TR_EXEC_FAILED, sizeof(int), &pid,
                               process_time, wall_time);
    errno = saved;
}


/*****************************************************************************
 * Support for throttling the resource creation mechanism when resources
 * are being used in an ephemeral manner (e.g., lots of message tags used,
 * but only one time each).
 ****************************************************************************/

int DYNINSTTwoComparesAndedExpr(int arg1, int arg2, int arg3, int arg4)
{
  return((arg1 == arg2) && (arg3 == arg4));
}

int DYNINSTCountElmsInArray(int* array, int num)
{
  int sum = 0;

  for(num--; num >= 0; num--) {
    sum += array[num];
  }
  return(sum);
}

int DYNINSTSub(int num1, int num2)
{
  return(num1-num2);
}

int DYNINSTAdd(int num1, int num2)
{
  return(num1+num2);
}

int DYNINSTMult(int num1, int num2)
{
  return(num1*num2);
}

int DYNINSTArrayField(int* array, int index)
{
  return(array[index]);
}

struct callee{
  unsigned address;
  struct callee *next;
};

struct call_site_info{
  unsigned address;
  struct callee *next_callee;
  struct call_site_info *next;
};

struct call_site_info *callSiteList = NULL;

/*Needs to return 1 if the call site has been seen, 0 otherwise*/
int PARADYNCalleeSearch(unsigned int callSiteAddr, unsigned int calleeAddr)
{
   if(callSiteList == NULL){
      callSiteList = (struct call_site_info *) 
         malloc(sizeof(struct call_site_info));
      callSiteList->address = callSiteAddr;
      callSiteList->next = NULL;
      callSiteList->next_callee = (struct callee *) 
         malloc(sizeof(struct callee));
      callSiteList->next_callee->next = NULL;
      callSiteList->next_callee->address = calleeAddr;
      return 0;
   }
   else {
      struct call_site_info *curr = callSiteList;
      while(curr != NULL){
         if(curr->address == callSiteAddr)
            break;
         else curr = curr->next;
      }
    
      if(curr == NULL){
         /*need to create new call_site_info, add link to new callee_struct,
           and send message back to the daemon*/
         curr = callSiteList;
         while(curr->next != NULL)
            curr = curr->next;
         curr->next = (struct call_site_info *) 
            malloc(sizeof(struct call_site_info));
         curr = curr->next;
         curr->address = callSiteAddr;
         curr->next = NULL;
         curr->next_callee = (struct callee *) malloc(sizeof(struct callee));
         curr->next_callee->next = NULL;
         curr->next_callee->address = calleeAddr;
         return 0;
      }
      else {
         /*in any event, curr should now point to the appropriate call
           site struct, so we should do a search in the callee list*/
         struct callee *curr_callee = curr->next_callee;
         while(curr_callee != NULL){
            if(curr_callee->address == calleeAddr){
               return 1;
            }
            curr_callee = curr_callee->next;
         }
         
         curr_callee = (struct callee *) malloc(sizeof(struct callee));
         curr_callee->next = curr->next_callee;
         curr_callee->address = calleeAddr;
         curr->next_callee = curr_callee;
         return 0;
      }
   }
}

void DYNINSTRegisterCallee(unsigned int calleeAddress,
                           unsigned callSiteAddress){
  if(PARADYNCalleeSearch(callSiteAddress, calleeAddress) == 0){
    struct callercalleeStruct c;
    c.caller = callSiteAddress;
    c.callee = calleeAddress;
    PARADYNgenerateTraceRecord(TR_DYNAMIC_CALLEE_FOUND,
                               sizeof(struct callercalleeStruct), &c, 
                               DYNINSTgetWalltime(), DYNINSTgetCPUtime());
  }
}

void PARADYNgenerateTraceRecord(short type, short length, void *eventData,
                                rawTime64 wall_time, rawTime64 process_time)
{
   static DECLARE_DYNINST_LOCK(trace_lock);

   static char *trace_buffer = NULL;
   static unsigned trace_buffer_size = 0;

   int result;
   traceHeader header;
   
   header.wall = wall_time;
   header.process = process_time;
   header.type = type;
   header.length = length;

   result = dyninst_lock(&trace_lock);
   if (result == DYNINST_LIVE_LOCK || result == DYNINST_DEAD_LOCK)
      return;

   if (sizeof(traceHeader) + length > trace_buffer_size)
   {
      trace_buffer_size = sizeof(traceHeader) + length + sizeof(int);
      trace_buffer = realloc(trace_buffer, trace_buffer_size);
      if (!trace_buffer) 
      {
         fprintf(stderr, "[%s:%u] - Memory allocation error when sending trace\n",
                 __FILE__, __LINE__);
         return;
      }
   }
   memcpy(trace_buffer, &header, sizeof(traceHeader));
   memcpy(trace_buffer + sizeof(traceHeader), eventData, length);
   DYNINSTuserMessage(trace_buffer, length+sizeof(traceHeader));
   dyninst_unlock(&trace_lock);                      
}


static DECLARE_DYNINST_LOCK(PARADYN_initLock);

/*
 * Run from the PARADYNinit
 */
void PARADYN_initialize_once() 
{
   static unsigned PARADYN_initialize_done;
   int result;
   unsigned i;
   unsigned max_threads = dyninst_maxNumOfThreads();

   if (PARADYN_initialize_done) 
      return;

   result = dyninst_lock(&PARADYN_initLock);
   if (result == DYNINST_DEAD_LOCK || DYNINST_LIVE_LOCK)
      return;

   for (i = 0; i < max_threads; i++) {
      virtualTimers[i].total = 0;
      virtualTimers[i].start = 0;
      virtualTimers[i].counter = 0;
      virtualTimers[i].lwp = 0;
      virtualTimers[i].rt_fd = 0;
      virtualTimers[i].protector1 = 0;
      virtualTimers[i].protector2 = 0;
      virtualTimers[i].rt_previous = 0;
   }    
   PARADYN_initialize_done=1;

 done:  
   dyninst_unlock(&PARADYN_initLock);
}
