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
 * $Id: RTinst.c,v 1.88 2005/03/23 04:34:23 legendre Exp $
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

#if defined(i386_unknown_nt4_0) /* ccw 4 jun 2002 */
#include <process.h>
#define getpid _getpid
#endif

#if !defined(i386_unknown_nt4_0)
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#include "kludges.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#if defined(MT_THREAD)
#include "RTthread.h"
#endif

#ifdef PARADYN_MPI
#include "/usr/lpp/ppe.poe/include/mpi.h"
/* #include <mpi.h> */
#endif

#include "RTcompat.h"
#include "pdutil/h/resource.h"

#if defined(MT_THREAD)
extern const char V_libparadynMT[];
#else
extern const char V_libparadynRT[];
#endif

#ifdef DEBUG_PRINT_RT
int PARADYNdebugPrintRT = 1;
#else
int PARADYNdebugPrintRT = 0;
#endif

virtualTimer *virtualTimers = 0;
unsigned *RTobserved_cost = 0;

unsigned *tramp_guards = 0;
unsigned *indexToThreads = 0;

/* And max # of threads allowed */
unsigned MAX_NUMBER_OF_THREADS;

/*extern void   DYNINSTos_init(int calledByFork, int calledByAttach); ccw 22 apr 2002 : SPLIT */
extern void   PARADYNos_init(int calledByFork, int calledByAttach);

/* platform dependent functions */
extern void PARADYNbreakPoint(void);
extern void DYNINST_install_ualarm(unsigned interval);
extern void DYNINSTinitTrace(int);
extern void DYNINSTflushTrace(void);
extern int DYNINSTwriteTrace(void *, unsigned);
extern void DYNINSTcloseTrace(void);
extern void *DYNINST_shm_init(int, int, int *);

void DYNINSTprintCost(void);
int DYNINSTGroup_CreateUniqueId(int,int);
void DYNINSTrecordTagGroupInfo(int,unsigned);
void DYNINSTtagGroupInfo_Init( void );

//for MPI RMA Window support
void DYNINSTWinInfo_Init( void );
unsigned int DYNINSTWindow_CreateUniqueId(unsigned int, int);
struct DynInstWin_st;
void DYNINSTreportNewWindow(const struct DynInstWin_st *);

int DYNINSTCalleeSearch(unsigned int callSiteAddr,                                                        unsigned int calleeAddr);
/************************************************************************/

static float  DYNINSTsamplingRate   = 0;
static int    DYNINSTtotalSamples   = 0;
static tTimer DYNINSTelapsedCPUTime;
static tTimer DYNINSTelapsedTime;

static int DYNINSTnumReported = 0;
static rawTime64 DYNINSTtotalSampleTime = 0;

char DYNINSThasInitialized = 0; /* 0 : has not initialized
				   2 : initialized by Dyninst
				   3 : initialized by Paradyn */

/************************************************************************/

/************************************************************************/

/* these vrbles are global so that fork() knows the attributes of the
   shm segs we've been using */
int DYNINST_shmSegKey;
int DYNINST_shmSegNumBytes;
int DYNINST_shmSegShmId; /* needed? */
void *DYNINST_shmSegAttachedPtr;
int DYNINST_mutatorPid = -1; /* set in DYNINSTinit(); pass to connectToDaemon();
				 needed if we fork */
int local_DYNINST_mutatorPid = -1;

#ifdef USE_PROF
int DYNINSTbufsiz;
int DYNINSTprofile;
int DYNINSTprofScale;
int DYNINSTtoAddr;
short *DYNINSTprofBuffer;
#endif


/*
 * New heap naming scheme: we encode the size and type of the heap
 * in the name, instead of having it implicitly known. The format is:
 * DYNINSTstaticHeap_<size>_<type>_garbage
 */

/* ccw 22 apr 2002 : SPLIT removed these heaps because they are in
 	DYNINST */
/* 
double DYNINSTstaticHeap_32K_lowmemHeap_1[(32*1024)/sizeof(double)];
double DYNINSTstaticHeap_4M_anyHeap_1[(4*1024*1024)/sizeof(double)];
*/
/* As DYNINSTinit() completes, it has information to pass back
   to paradynd.  The data can differ; more stuff is needed
   when SHM_SAMPLING is defined, for example.  But in any event,
   the following gives us a general framework.  When DYNINSTinit()
   is about to complete, we used to send a TR_START trace record back and then
   DYNINSTbreakPoint().  But we've seen strange behavior; sometimes
   the breakPoint is received by paradynd first.  The following should
   work all the time:  DYNINSTinit() writes to the following vrble
   and does a DYNINSTbreakPoint(), instead of sending a trace
   record.  Paradynd reads this vrble using ptrace(), and thus has
   the info that it needs.
   Note: we use this framework for DYNINSTfork(), too -- so the TR_FORK
   record is now obsolete, along with the TR_START record.
   Additionally, we use this framework for DYNINSTexec() -- so the
   TR_EXEC record is now obsolete, too */
/* struct DYNINST_bootstrapStruct DYNINST_bootstrap_info; ccw 18 apr 2002 : SPLIT */
struct PARADYN_bootstrapStruct PARADYN_bootstrap_info;

#define N_FP_REGS 33

volatile int DYNINSTsampleMultiple    = 1;
   /* written to by dynRPC::setSampleRate() (paradynd/dynrpc.C)
      (presumably, upon a fold) */

/* Written to by daemon just before launching an inferior RPC */
/*rpcInfo curRPC = { 0, 0, 0 }; ccw 18 apr 2002 SPLIT */

unsigned pcAtLastIRPC;  /* just used to check for errors */
/* 1 = a trap was ignored and needs to be regenerated
   0 = there is not a trap that hasn't been processed */
int trapNotHandled = 0;

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

/************************************************************************
 * void DYNINSTstartProcessTimer(tTimer* timer)
************************************************************************/
void
DYNINSTstartProcessTimer(tTimer* timer) {

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
void
DYNINSTstopProcessTimer(tTimer* timer) {

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

void
DYNINSTstartWallTimer(tTimer* timer) {

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
void
DYNINSTstopWallTimer(tTimer* timer) {

  int i;
  
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
 * void saveFPUstate(float* base)
 * void restoreFPUstate(float* base)
 *
 * save and restore state of FPU on signals.  these are null functions
 * for most well designed and implemented systems.
************************************************************************/
static void
saveFPUstate(float* base) {
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0)
    /* kludge for the pentium: we need to reset the FPU here, or we get 
       strange results on fp operations.
    */
    asm("finit");
#endif
}

static void
restoreFPUstate(float* base) {
}

/************************************************************************
 * void DYNINSTgenerateTraceRecord(traceStream sid, short type,
 *   short length, void* data, int flush, rawTime64 wall_time, 
 *   rawTime64 process_time)
************************************************************************/
void
DYNINSTgenerateTraceRecord(traceStream sid, short type, short length,
			   void *eventData, int flush,
			   rawTime64 wall_time, rawTime64 process_time) {
    static unsigned pipe_gone = 0;
    static unsigned inDYNINSTgenerateTraceRecord = 0;
    traceHeader     header;
    int             count;
    char            buffer[1024];

#if defined(MT_THREAD)
    if (DYNINST_DEAD_LOCK == tc_lock_lock(&DYNINST_traceLock)){
      return ;
    }
#endif /*MT_THREAD*/

    if (inDYNINSTgenerateTraceRecord) return;
    inDYNINSTgenerateTraceRecord = 1;

    if (pipe_gone) {
        inDYNINSTgenerateTraceRecord = 0;
        return;
    }

    header.wall    = wall_time;
    header.process = process_time;
#ifdef ndef
    if(type == TR_SAMPLE){
	 traceSample *s = (traceSample*)eventData;
         printf("wall time = %f processTime = %f value = %f\n",
		(double)(header.wall/1000000.0), 
		(double)(header.process/1000000.0),
		s->value);
    }
#endif

    length = ALIGN_TO_WORDSIZE(length);

    header.type   = type;
    header.length = length;

    count = 0;
    memcpy(&buffer[count], &sid, sizeof(traceStream));
    count += sizeof(traceStream);

    memcpy(&buffer[count], &header, sizeof(header));
    count += sizeof(header);

    count = ALIGN_TO_WORDSIZE(count);
    memcpy(&buffer[count], eventData, length);
    count += length;

    
    if (!DYNINSTwriteTrace(buffer, count))
      pipe_gone = 1;

    if (flush) DYNINSTflushTrace();

    inDYNINSTgenerateTraceRecord = 0;
#if defined(MT_THREAD)
    tc_lock_unlock(&DYNINST_traceLock) ;
#endif /*MT_THRAED*/

}

static void shmsampling_printf(const char *fmt, ...) {
#ifdef SHM_SAMPLING_DEBUG
   va_list args;
   va_start(args, fmt);

   vfprintf(stderr, fmt, args);

   va_end(args);

   fflush(stderr);
#endif
}

double DYNINSTdummydouble = 4321.71; /* Global so the compiler won't
					optimize away FP code in initFPU */
static void initFPU()
{
       /* Init the FPU.  We've seen bugs with Linux (e.g., Redhat 6.2
	  stock kernel on PIIIs) where processes started by Paradyn
	  started with FPU uninitialized. */
       double x = 17.1234;
       DYNINSTdummydouble *= x;
}



/************************************************************************
 * void DYNINSTinit()
 *
 * initialize the DYNINST library.  this function is called at the start
 * of the application program, as well as after a fork, and after an
 * attach.
 *
 ************************************************************************/
void PARADYNinit(int paradyndPid,
                 int creationMethod,
                 int numThreads,
                 virtualTimer *virtualTimersAddr,
                 int *obsCostAddr) {
    /* On many platforms we can't call this function directly, so
       at initialization we write values into global variables and
       then call a wrapper function (below) */
    
    int i;
    int calledFromAttachToCreated = (creationMethod == 3);
    int calledFromFork = (creationMethod == 2);
    int calledFromAttach = (creationMethod == 1);
    int calledFromExec = (creationMethod == 4);
    
    MAX_NUMBER_OF_THREADS = numThreads;

#ifdef SHM_SAMPLING_DEBUG
    char thehostname[80];
    extern int gethostname(char*,int);
    
    (void)gethostname(thehostname, 80);
    thehostname[79] = '\0';
   
   /*  
       shmsampling_printf("WELCOME to DYNINSTinit (%s, pid=%d), args are %d, %d, %d\n",
       thehostname, (int)getpid(), theKey, shmSegNumBytes,
       paradyndPid);
   */
#endif
   
   initFPU();
   DYNINSThasInitialized = 3;

   /* sanity check */
   assert(sizeof(rawTime64) == 8);
   assert(sizeof(int64_t) == 8);
   assert(sizeof(int32_t) == 4);
 
   DYNINST_mutatorPid = paradyndPid; /* important -- needed in case we fork() */
  
   /* initialize the tag and group info */
   DYNINSTtagGroupInfo_Init();
   DYNINSTWinInfo_Init();

   tramp_guards = (unsigned *)malloc(numThreads * sizeof(unsigned));
   for (i = 0; i < numThreads; i++)
      tramp_guards[i] = 1; /* default value */

   indexToThreads = (unsigned *)malloc(numThreads * sizeof(unsigned));
   // Initialization is done in the thread code

   virtualTimers = virtualTimersAddr;
   RTobserved_cost = obsCostAddr;
   
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
     saying that DYNINSTinit() (or at least this part of it) shouldn't be
     invoked until stdio has been initialized!
   */
  
   if (!calledFromAttach) {
      setvbuf(stdout, NULL, _IOLBF, 0);
      /* make stdout line-buffered.  "setlinebuf(stdout)" is cleaner but HP
         doesn't seem to have it */
      setvbuf(stderr, NULL, _IONBF, 0);
      /* make stderr non-buffered */
   }
  
#if defined(MT_THREAD)
   RTprintf("%s\n", V_libparadynMT);
#else
   RTprintf("%s\n", V_libparadynRT);
#endif

#ifdef PAPI
   initPapi(); 
#endif

   PARADYNos_init(calledFromFork, calledFromAttach);

#ifdef USE_PROF
   {
      extern int end;
    
      DYNINSTprofScale = sizeof(short);
      DYNINSTtoAddr = sizeof(short);
      DYNINSTbufsiz = (((unsigned int) &end)/DYNINSTprofScale) + 1;
      DYNINSTprofBuffer = (short *) calloc(sizeof(short), DYNINSTbufsiz);
      profil(DYNINSTprofBuffer, DYNINSTbufsiz*sizeof(short), 0, 0xffff);
      DYNINSTprofile = 1;
   }
#endif

   /* Fill in info for paradynd to receive: */
  
   PARADYN_bootstrap_info.ppid = -1; /* not needed really */ /* was DYNINST_ ccw 18 apr 2002 SPLIT */

  
   PARADYN_bootstrap_info.pid = getpid(); /* was DYNINST_ ccw 18 apr 2002 SPLIT */ 
#if !defined(i386_unknown_nt4_0)
   if (calledFromFork)
      PARADYN_bootstrap_info.ppid = getppid(); /* was DYNINST_ ccw 18 apr 2002 SPLIT */
#else
   PARADYN_bootstrap_info.ppid = 0; /* was DYNINST_ ccw 18 apr 2002 SPLIT */
#endif 

   PARADYN_bootstrap_info.tramp_guard_base = tramp_guards;
   PARADYN_bootstrap_info.thread_index_base = indexToThreads;
   
   for (i = 0; i < numThreads; i++)
      PARADYN_bootstrap_info.tramp_guard_base[i] = 1; /* default value */

   /* We do this field last as a way to synchronize; paradynd will ignore what it
      sees in this structure until the event field is nonzero */
   /* was DYNINST_ ccw 18 apr 2002 SPLIT */
   if (calledFromFork)
      PARADYN_bootstrap_info.event = 2; /* 2 --> end of DYNINSTinit (forked process) */
   else if (calledFromAttach)
      PARADYN_bootstrap_info.event = 3; /* 3 --> end of DYNINSTinit (attached proc) */
   else if (calledFromExec)
      PARADYN_bootstrap_info.event = 4;
   else				   
      PARADYN_bootstrap_info.event = 1; /* 1 --> end of DYNINSTinit (normal or when
                                           called by exec'd proc or attachedTocreated case) */

   /* If attaching, now's the time where we set up the trace stream connection fd */
   if (calledFromAttach) {
      int pid = getpid();
#if !defined(i386_unknown_nt4_0)
      int ppid = getppid();
#else
      int ppid = 0;
#endif
      unsigned attach_cookie = 0x22222222;
      int32_t ptr_size;
    
      DYNINSTinitTrace(paradyndPid);
    
      DYNINSTwriteTrace(&attach_cookie, sizeof(attach_cookie));
      DYNINSTwriteTrace(&pid, sizeof(pid));
      DYNINSTwriteTrace(&ppid, sizeof(ppid));
      DYNINSTwriteTrace(&DYNINST_shmSegKey, sizeof(DYNINST_shmSegKey));
      ptr_size = sizeof(DYNINST_shmSegAttachedPtr);
      DYNINSTwriteTrace(&ptr_size, sizeof(int32_t));
      DYNINSTwriteTrace(&DYNINST_shmSegAttachedPtr, ptr_size);
      DYNINSTflushTrace();
   }
   else if (!calledFromFork) {
      /* either normal startup or startup via a process having exec'd or attachtoCreated case */
#if !defined(i386_unknown_nt4_0)
      if (calledFromAttachToCreated){
         /* unique case identified as attachToCreated */
         int cookie = 0;  /*  not attach, not fork */
         int pid = getpid();
         int ppid = paradyndPid; /* paradynd is seen as the father because it is attached
                                    to the application */
         int32_t ptr_size;

         DYNINSTinitTrace(paradyndPid);
         DYNINSTwriteTrace(&cookie, sizeof(cookie));
         DYNINSTwriteTrace(&pid, sizeof(pid));
         DYNINSTwriteTrace(&ppid, sizeof(ppid));
         DYNINSTflushTrace();/* we needed it */
         DYNINSTwriteTrace(&DYNINST_shmSegKey, sizeof(DYNINST_shmSegKey)); 
         ptr_size = sizeof(DYNINST_shmSegAttachedPtr);
         DYNINSTwriteTrace(&ptr_size, sizeof(int32_t));
         DYNINSTwriteTrace(&DYNINST_shmSegAttachedPtr, ptr_size);
         DYNINSTflushTrace(); /* we needed it */
      }
      else {/* trace stream is already open */ 
         DYNINSTinitTrace(-1);
      }
#else
      /* need to get a connection to daemon */
      int cookie = 0;
      int pid = getpid();
      int ppid = paradyndPid;
      int32_t ptr_size;

      DYNINSTinitTrace(paradyndPid);
      DYNINSTwriteTrace(&cookie, sizeof(cookie));
      DYNINSTwriteTrace(&pid, sizeof(pid));
      DYNINSTwriteTrace(&ppid, sizeof(ppid));
      DYNINSTwriteTrace(&DYNINST_shmSegKey, sizeof(DYNINST_shmSegKey));
      ptr_size = sizeof(DYNINST_shmSegAttachedPtr);
      DYNINSTwriteTrace(&ptr_size, sizeof(int32_t));
      DYNINSTwriteTrace(&DYNINST_shmSegAttachedPtr, ptr_size);
#endif
   }
   else
      /* calledByFork -- DYNINSTfork already called DYNINSTinitTrace */
      ;
  
   /* MT_THREAD */
#if defined(MT_THREAD)
   {
      /* Note: until this point we are not multithread-safe. */
      DYNINST_initialize_once((char*) DYNINST_shmSegAttachedPtr, numThreads);

      DYNINST_reportThreadUpdate(calledFromAttach?FLAG_ATTACH:FLAG_INIT) ;
   }
#endif

   /* db_init(db_shmKey, sizeof(db_shmArea_t)); */

   //DYNINSTstartWallTimer(&DYNINSTelapsedTime);
   //DYNINSTstartProcessTimer(&DYNINSTelapsedCPUTime);

   shmsampling_printf("leaving DYNINSTinit (pid=%d) --> the process is running freely now\n", (int)getpid());
}


/*
 * These are used by DllMain/_init to call PARADYNinit
 */
int libparadynRT_init_localparadynPid=-1;
int libparadynRT_init_localcreationMethod=-1;
int libparadynRT_init_localmaxThreads=-1;
virtualTimer *libparadynRT_init_localVirtualTimers=NULL;
int *libparadynRT_init_localObservedCost=NULL;

#if defined(i386_unknown_nt4_0)  /*ccw 4 jun 2002*/
#include <windows.h>

/* this function is automatically called when windows loads this dll
 if we are launching a mutatee to instrument, dyninst will place
 the correct values in libparadynRT_init_local* 
 and they will be passed to
 PARADYNinit to correctly initialize the dll.  this keeps us
 from having to instrument two steps from the mutator (load and then 
 the execution of PARADYNinit()
*/
int pDllMainCalledOnce=0;
BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  /* handle to DLL module */
  DWORD fdwReason,     /* reason for calling function */
  LPVOID lpvReserved   /* reserved */
){
#ifdef NOTDEF // PDSEP
	if(pDllMainCalledOnce){
	}else{
		if(libparadynRT_init_localparadynPid != -1 &&
           libparadynRT_init_localcreationMethod != -1) {
            pDllMainCalledOnce++;
            PARADYNinit(libparadynRT_init_localparadynPid,
                        libparadynRT_init_localcreationMethod,
                        libparadynRT_init_localmaxThreads, /* Number of threads */
                        libparadynRT_init_localVirtualTimers,
			libparadynRT_init_localObservedCost);
		}
	}
#endif
	return 1; 
}
 

//#else
#endif // windows

#ifdef __GNUC
__attribute__ ((constructor)) void libparadynRT_init(void);
#endif


// Unix platforms
int PARADYNinitCalledOnce=0;

void libparadynRT_init() {
    if(PARADYNinitCalledOnce) return;
    else {
        if(libparadynRT_init_localparadynPid != -1 &&
           libparadynRT_init_localcreationMethod != -1) {
            
            PARADYNinitCalledOnce++;
            PARADYNinit(libparadynRT_init_localparadynPid,
                        libparadynRT_init_localcreationMethod,
                        libparadynRT_init_localmaxThreads, /* Number of threads */
                        libparadynRT_init_localVirtualTimers,
                        libparadynRT_init_localObservedCost);
		}
	}
}
//#endif



/* bootstrap structure extraction info (see rtinst/h/trace.h) */
static struct PARADYN_bootstrapStruct _bs_dummy;

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

#if !defined(i386_unknown_nt4_0)
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
   ptr_size = sizeof(DYNINST_shmSegAttachedPtr);

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

   DYNINSTinitTrace(DYNINST_mutatorPid);

   DYNINSTwriteTrace(&cookie, sizeof(cookie));
   DYNINSTwriteTrace(&pid, sizeof(pid));
   DYNINSTwriteTrace(&ppid, sizeof(ppid));
   DYNINSTwriteTrace(&DYNINST_shmSegKey, sizeof(DYNINST_shmSegKey));
   ptr_size = sizeof(DYNINST_shmSegAttachedPtr);
   DYNINSTwriteTrace(&ptr_size, sizeof(int32_t));
   DYNINSTwriteTrace(&DYNINST_shmSegAttachedPtr, ptr_size);
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

void
DYNINSTexecFailed() {
    /* DYNINSTgenerateTraceRecord resets errno back to zero */
    int saved = errno;
    rawTime64 process_time = DYNINSTgetCPUtime();
    rawTime64 wall_time = DYNINSTgetWalltime();
    int pid = getpid();
    
    forkexec_printf("DYNINSTexecFAILED errno = %d\n", errno);
    
    DYNINSTgenerateTraceRecord(0, TR_EXEC_FAILED, sizeof(int), &pid, 1,
			       process_time, wall_time);
    errno = saved;
}



/************************************************************************
 * void DYNINSTprintCost(void)
 *
 * print a detailed summary of the cost of the application's run.
 * Note:  Used to be called by DYNINSTexit, since 
************************************************************************/
void
DYNINSTprintCost(void) {
    FILE *fp;
    struct endStatsRec stats;

    //DYNINSTstopProcessTimer(&DYNINSTelapsedCPUTime);
    //DYNINSTstopWallTimer(&DYNINSTelapsedTime);

    stats.instCycles = *RTobserved_cost;
    stats.alarms      = 0;
    stats.numReported = DYNINSTnumReported;
    /* used only by alarm sampling */
    stats.handlerCost = 0;

    stats.totalCpuTime  = DYNINSTelapsedCPUTime.total;
    stats.totalWallTime = DYNINSTelapsedTime.total;

    stats.samplesReported = DYNINSTtotalSamples;

    stats.userTicks = 0;
    stats.instTicks = 0;

#if defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
    stats.totalTraps = 0; /* value in DYNINSTtotalTraps, defined in
			     libdyninstAPI_RT.so */
#endif


#ifdef USE_PROF
    fp = fopen("stats.out", "w");

    if (DYNINSTprofile) {
	int i;
	int limit;
	int pageSize;
	int startInst;
	extern void DYNINSTfirst();


	limit = DYNINSTbufsiz;
	/* first inst code - assumes data area above code space in virtual
	 * address */
	startInst = (int) &DYNINSTfirst;
	fprintf(fp, "startInst = %x\n", startInst);
	fprintf(fp, "limit = %x\n", limit * DYNINSTprofScale);
	for (i=0; i < limit; i++) {
#ifdef notdef
	    if (DYNINSTprofBuffer[i]) {
		fprintf(fp, "%x %d\n", i * DYNINSTtoAddr, 
			DYNINSTprofBuffer[i]);
	    }
#endif
	    if (i * DYNINSTtoAddr > startInst) {
		stats.instTicks += DYNINSTprofBuffer[i];
	    } else {
		stats.userTicks += DYNINSTprofBuffer[i];
	    }
	}

	/* fwrite(DYNINSTprofBuffer, DYNINSTbufsiz, 1, fp); */
	fprintf(fp, "stats.instTicks %d\n", stats.instTicks);
	fprintf(fp, "stats.userTicks %d\n", stats.userTicks);
    }
#endif

#ifdef notdef
    fprintf(fp, "DYNINSTtotalAlarmExpires %d\n", stats.alarms);
    fprintf(fp, "DYNINSTnumReported %d\n", stats.numReported);
    fprintf(fp,"Raw cycle count = %lld\n", stats.instCycles);
    fprintf(fp,"Total handler cost = %lld\n", stats.handlerCost);
    fprintf(fp,"Total cpu time of program %lld\n", stats.totalCpuTime);
    fprintf(fp,"Elapsed wall time of program %lld\n",stats.totalWallTime);
    fprintf(fp,"total data samples %d\n", stats.samplesReported);
    fprintf(fp,"Application program ticks %d\n", stats.userTicks);
    fprintf(fp,"Instrumentation ticks %d\n", stats.instTicks);

    fclose(fp);
#endif

    /* record that the exec is done -- should be somewhere better. */
    DYNINSTgenerateTraceRecord(0, TR_EXIT, sizeof(stats), &stats, 1,
			       DYNINSTgetWalltime(), DYNINSTgetCPUtime());
}


/*****************************************************************************
 * Support for throttling the resource creation mechanism when resources
 * are being used in an ephemeral manner (e.g., lots of message tags used,
 * but only one time each).
 ****************************************************************************/

/*
 * maximum number of resource creations to track when checking for
 * a high resource creation rate
 *
 * this value should be large enough that if we stop tracking creations
 * because we've run out of slots, we would likely throttle the 
 * resource creation anyway
 */
#define	DYNINSTmaxResourceCreates			1000

/* 
 * length of the sliding window used to check for high rates of
 * message tag creation
 * units are seconds
 */
#define	DYNINSTtagCreateRateWindowLength	(10)

/* the value used to indicate an undefined tag */
#define	DYNINSTGroupUndefinedTag			(-999)


/*
 * limit on the creation rate of message tags within a group
 * units are number of creations per second
 */
#define	DYNINSTtagCreationRateLimit			(1.0)


/*
 * DYNINSTresourceCreationInfo
 * 
 * Maintains info about the number of resource discovery events 
 * within a sliding window.
 */
typedef struct DYNINSTresourceCreationInfo_
{
	rawTime64		creates[DYNINSTmaxResourceCreates];	/* time of create events */
	unsigned int	nCreatesInWindow;
	double			windowLength;		/* in seconds */
	unsigned int	head;
	unsigned int	tail;
} DYNINSTresourceCreationInfo;

/*
 * DYNINSTresourceCreationInfo_Init
 *
 * Initialize a DYNINSTresourceCreationInfo to be empty.
 */
void
DYNINSTresourceCreationInfo_Init( DYNINSTresourceCreationInfo* ci,
								double windowLength )
{
	ci->windowLength = windowLength;
	ci->nCreatesInWindow = 0;
	ci->head = 0;
	ci->tail = 0;
}

/*
 * DYNINSTresourceCreationInfo_Add
 *
 * Record a resource creation event.  Has the effect of sliding
 * the sliding window forward to the indicated time.
 */
int
DYNINSTresourceCreationInfo_Add( DYNINSTresourceCreationInfo* ci,
									rawTime64 ts )
{
	int ret = 0;

	if( ci->nCreatesInWindow < (DYNINSTmaxResourceCreates - 1) )
	{
		/*
		 * ts is the leading edge of the sliding window
		 * determine new trailing edge of the sliding window
		 *
		 * Note: assumes windowLength is in seconds, and ts in nanoseconds
		 */
		rawTime64 trailingEdge = (rawTime64)(ts - (ci->windowLength * 1000000000));

		/* drop any creation events that have fallen out of the window */
		while( (ci->nCreatesInWindow > 0) && (ci->creates[ci->tail] < trailingEdge))
		{
			ci->tail = (ci->tail + 1) % DYNINSTmaxResourceCreates;
			ci->nCreatesInWindow--;
		}

		/* add the new creation event */
		if( ci->nCreatesInWindow == 0 )
		{
			/* window was empty - reset to beginning of array */
			ci->head = 0;
			ci->tail = 0;
		}
		else
		{
			ci->head = (ci->head + 1) % DYNINSTmaxResourceCreates;
		}
		ci->creates[ci->head] = ts;
		ci->nCreatesInWindow++;

		ret = 1;
	}
	return ret;
}


/*****************************************************************************
 * Support for discovering new communication groups and message tags.
 * and MPI Windows, too
 ****************************************************************************/

#define DYNINSTagsLimit      1000
#define DYNINSTagGroupsLimit 100
#define DYNINSTNewTagsLimit   50 /* don't want to overload the system */

#define DYNINSTWindowsLimit   500

typedef struct DynInstWin_st {
   unsigned int   WinId;        /* winId as used by the program */
   unsigned int   WinUniqueId;     /* our unique identifier for the window */
   /* A Window is identifed by a combination of WinId and WinUniqueId. */
   char * WinName;                   /* name of window given by the user */
   struct DynInstWin_st* Next;       /* next defined window info struct */
} DynInstWinSt;

#define LAM 0
#define MPICH 1

int whichMPI = -1;
char *whichMPIenv = NULL;

typedef struct {
   int            NumWins;     /* Number of Windows */
   struct DynInstWin_st*  WindowTable[DYNINSTWindowsLimit]; /* Window table */
/* WindowCounters is an array that maintains the 'next' unique number
   for windows that fall into the
   same index in WindowTable.  This is because the MPI implementation may reuse
   a WinId if a window is destroyed with MPI_Win_free.  When a window is freed
   we will remove its entry from WindowTable.  That way when the WinId is
  reused in a MPI_Win_create call, we won't find it in the WindowTable. We will   know it is a new resource.
 */
  int WindowCounters[DYNINSTWindowsLimit];
} DynInstWinArraySt;


typedef struct DynInstTag_st {
  int      TagGroupId;                /* group as used by the program */
  unsigned TGUniqueId;                /* our unique identifier for the group */
  int      NumTags;                   /* number of tags in our TagTable */
  int      TagTable[DYNINSTagsLimit];/* known tags for this group */
  DYNINSTresourceCreationInfo	tagCreateInfo;	/* record of recent tag creations */

  struct DynInstTag_st* Next;       /* next defined group info struct */
} DynInstTagSt;



typedef struct DynInstNewTagInfo_
{
	int tagId;                   /* id of new tag */
	unsigned groupId;            /* group tag belongs to */
        int TGUniqueId;              /* our unique identifier for the group */
	int isNewGroup;              /* is this the first time we saw this group? */
} DynInstNewTagInfo;


typedef struct {
  int            TagHierarchy; /* True if hierarchy, false single level */
  int            NumGroups;     /* Number of groups, tag arrays */
  /* Group table, each index pointing to an array of tags */
  DynInstTagSt*  GroupTable[DYNINSTagGroupsLimit]; 
  //for unique representation of communicators - they may be deallocated
  //and the identifier reused in the implmenentation
  int GroupCounters[DYNINSTagGroupsLimit];
  int                NumNewTags;   /* number of entries in the NewTags array */
  DynInstNewTagInfo  NewTags[DYNINSTNewTagsLimit];
} DynInstTagGroupSt;




static DynInstTagGroupSt  TagGroupInfo;
static DynInstWinArraySt  WinInfo;




/************************************************************************
 * DYNINSTtagGroupInfo_Init
 *
 * Initialize the singleton TagGroupInfo struct.
 *
 ************************************************************************/
void
DYNINSTtagGroupInfo_Init( void )
{
  unsigned int dx;

  TagGroupInfo.TagHierarchy = 0; /* FALSE; */
  TagGroupInfo.NumGroups = 0;
  TagGroupInfo.NumNewTags = 0;
  for(dx=0; dx < DYNINSTagGroupsLimit; dx++) {
    TagGroupInfo.GroupTable[dx] = NULL;
    TagGroupInfo.GroupCounters[dx] = 0;
  } 
}

/************************************************************************
 * DYNINSTWinInfo_Init
 *
 * Initialize the singleton WinInfo struct for RMA Windows
 ************************************************************************/
void
DYNINSTWinInfo_Init( void )
{
  unsigned int dx;
  static int warned = 0;

  WinInfo.NumWins = 0;
  for(dx=0; dx < DYNINSTWindowsLimit; dx++) {
    WinInfo.WindowTable[dx] = NULL;
    WinInfo.WindowCounters[dx] = 0;
  }
/* determine which MPI implementation we are using - necessary because they
   use different data structures to represent things like communicators and
   windows.
*/
  if(whichMPI == -1){
    whichMPIenv = getenv("PARADYN_MPI");
    //fprintf(stderr,"got it %s\n",whichMPIenv);
    if(whichMPIenv){
      if (!strcmp(whichMPIenv,"LAM"))
         whichMPI = LAM;
      else if(!strcmp(whichMPIenv, "MPICH"))
         whichMPI = MPICH;
      else
         if(!warned){
            warned = 1;
            fprintf(stderr,"ENV variable PARADYN_MPI not set. Unable to \
	            determine MPI implementation.\n");
	 }
    }
  }
}

/************************************************************************
 * void DYNINSTreportNewTags(void)
 *
 * Inform the paradyn daemons of new message tags and/or groups.
 *
************************************************************************/
void DYNINSTreportNewTags(void)
{
  int    dx;
  rawTime64 process_time;
  rawTime64 wall_time;

  if(TagGroupInfo.NumNewTags > 0) {
    /* not used by consumer [createProcess() in perfStream.C], so can prob.
     * be set to a dummy value to save a little time.  */
    process_time = DYNINSTgetCPUtime();
    /* this _is_ needed; paradynd keeps the 'creation' time of each resource
     *  (resource.h) */
    wall_time = DYNINSTgetWalltime();
  }
  
  for(dx=0; dx < TagGroupInfo.NumNewTags; dx++) {
    struct _newresource newRes;
    
    if((TagGroupInfo.TagHierarchy) && (TagGroupInfo.NewTags[dx].isNewGroup)) {
      memset(&newRes, '\0', sizeof(newRes));
      sprintf(newRes.name, "SyncObject/Message/%d-%d",
        TagGroupInfo.NewTags[dx].groupId, TagGroupInfo.NewTags[dx].TGUniqueId);
      strcpy(newRes.abstraction, "BASE");
      newRes.mdlType = RES_TYPE_STRING;
      newRes.btype = MessageGroupResourceType;

      DYNINSTgenerateTraceRecord(0, TR_NEW_RESOURCE,
				 sizeof(struct _newresource), &newRes, 1,
				 wall_time,process_time);
    }
    
    memset(&newRes, '\0', sizeof(newRes));
    if(TagGroupInfo.TagHierarchy) {
      sprintf(newRes.name, "SyncObject/Message/%d-%d/%d",
              TagGroupInfo.NewTags[dx].groupId,TagGroupInfo.NewTags[dx].TGUniqueId, TagGroupInfo.NewTags[dx].tagId);
      newRes.btype = MessageTagResourceType;
    } else {//if there is no Group/Tag hierarchy - when is this true?
            //I believe this was for PVM support, I don't see it used anywhere
            //else
      sprintf(newRes.name, "SyncObject/Message/%d", 
	      TagGroupInfo.NewTags[dx].tagId);
      newRes.btype = MessageGroupResourceType;
    }
    strcpy(newRes.abstraction, "BASE");
    newRes.mdlType = RES_TYPE_INT;

    DYNINSTgenerateTraceRecord(0, TR_NEW_RESOURCE, 
			       sizeof(struct _newresource), &newRes, 1,
			       wall_time,process_time);
  }
  TagGroupInfo.NumNewTags = 0;
}


/************************************************************************
 * DYNINSTrecordTagGroupInfo
 *
 * Handle a (possibly) new group and/or message tag.  This routine
 * checks whether we've seen the given group and tag before, and
 * if not, reports them to the Paradyn daemon.
 *
 * We use a sort of hash table to keep tags and groups, so this routine
 * involves looking up the table entry to see whether we've seen
 * the tag or group before.
 *
 * There are two issues to consider -
 *
 * 1. We limit the total number of tags and groups so that an application
 *    that uses many of them will not overwhelm Paradyn by reporting
 *    new tags or groups.
 *
 * 2. We throttle the reporting of tags and groups whenever their
 *    creation rate is too high, again to avoid overwhelming the
 *    rest of Paradyn with the creation of new resources.
 *
 ************************************************************************/
static unsigned int rateCheck = 0;

void DYNINSTrecordTagGroupInfo(int tagId, unsigned groupId)
{
  DynInstTagSt* tagSt;
  int           dx;
  int           newGroup;
  int		        tagDx = (tagId % DYNINSTagsLimit);
  unsigned		groupDx = (groupId % DYNINSTagGroupsLimit);
  double		recentTagCreateRate;
  rawTime64		ts;
  

  /* 
   * check if we've reached the limit on the number of new tags 
   * to be reported at one time
   * (In the current implementation, this should never happen.  All
   * platforms support shared memory sampling, so we report new tags 
   * and groups at the end of this function whenever we are called.
   */
  if(TagGroupInfo.NumNewTags == DYNINSTNewTagsLimit) return;


  /*
   * Find the info about the group we were given
   */
  tagSt = TagGroupInfo.GroupTable[groupDx];
  while((tagSt != NULL) && (tagSt->TagGroupId != groupId)) {
    tagSt = tagSt->Next;
  }
  if(tagSt == NULL) {
    /* We have not seen this group before, so add a group info struct for it. */
    tagSt = (DynInstTagSt *)malloc(sizeof(DynInstTagSt));
    assert(tagSt != NULL);

    tagSt->TagGroupId = groupId;
    tagSt->TGUniqueId = DYNINSTGroup_CreateUniqueId(groupId, groupDx);
    tagSt->NumTags = 0;
    for(dx=0; dx < DYNINSTagsLimit; dx++) {
      tagSt->TagTable[dx] = DYNINSTGroupUndefinedTag;
    }
	DYNINSTresourceCreationInfo_Init( &(tagSt->tagCreateInfo),
										DYNINSTtagCreateRateWindowLength );
    tagSt->Next = TagGroupInfo.GroupTable[groupDx];
    TagGroupInfo.GroupTable[groupDx] = tagSt;
    TagGroupInfo.NumGroups++;
    newGroup = 1;
  } else {
    /* We have seen this group before - nothing to do */
    assert(tagSt->TagGroupId == groupId);
    newGroup = 0;
  }


  /*
   * Check if we've reached the limit on the number of tags to track.
   */
  if(tagSt->NumTags == DYNINSTagsLimit) return;


  /*
   * Find the info about the tag we were given
   */
  dx = tagDx;
  while((tagSt->TagTable[dx] != tagId) && (tagSt->TagTable[dx] != DYNINSTGroupUndefinedTag)) {
    dx++;
    if(dx == DYNINSTagsLimit)
	{
       dx = 0;
    }
    assert(dx != tagId);
  }

  if(tagSt->TagTable[dx] == tagId)
  {
    /* We've already seen this tag - nothing to do */
	return;
  }


  /*
   * check if the program is using new tags too quickly...
   */

  /* ...record that a tag was discovered for this group at this time
   * (we chose to use wall time to compute creation rate because the
   * daemon and front end are separate processes from this one, and the
   * issue here is how quickly they are able to handle new resource
   * notifications)...
   */
  ts = DYNINSTgetWalltime();
  if( DYNINSTresourceCreationInfo_Add( &(tagSt->tagCreateInfo), ts ) == 0 )
  {
		/*
		 * we ran out of space in the creation info -
		 * most likely, the application is creating resources too
		 * quickly, so we will throttle
		 *
	 	 * don't report this tag to the daemon now (in fact,
	 	 * don't record that we ever saw this tag so that we will
		 * handle it correctly in case we see the tag in the future
		 * when the program is not coming up with new tags so quickly
		 */
		return;
  }

  /* ...determine the creation rate during this group's sliding window... */
  recentTagCreateRate = (((double)tagSt->tagCreateInfo.nCreatesInWindow) /
							tagSt->tagCreateInfo.windowLength);

  /* ...check whether the creation rate is too high */
  if( recentTagCreateRate > DYNINSTtagCreationRateLimit )
  {
    /* 
	 * the program is using new tags too quickly -
	 * don't report this tag to the daemon now (in fact,
	 * don't record that we ever saw this tag so that we will
	 * handle it correctly in case we see the tag in the future
	 * when the program is not coming up with new tags so quickly
	 */
	 return;
  }


  assert(tagSt->TagTable[dx] == DYNINSTGroupUndefinedTag);

  /* allocate and initialize a new tag info struct */
  tagSt->TagTable[dx] = tagId;
  tagSt->NumTags++;
  
  TagGroupInfo.NewTags[TagGroupInfo.NumNewTags].tagId = tagId;
  TagGroupInfo.NewTags[TagGroupInfo.NumNewTags].groupId = tagSt->TagGroupId;
  TagGroupInfo.NewTags[TagGroupInfo.NumNewTags].TGUniqueId = tagSt->TGUniqueId;
  TagGroupInfo.NewTags[TagGroupInfo.NumNewTags].isNewGroup = newGroup;
  TagGroupInfo.NumNewTags++;

  /* report new tag and/or group to our daemon */
  DYNINSTreportNewTags();
}

//The following structures are for lam support. The c_contextid field is the
//context id of the communicator or window we are interested in. Another option
//for getting the context id would be to use the LAM specific MPIL_Comm_id
//function. However, that would require statically linking the appropriate
//MPI library into libparadynRT.so.1...

struct LAM_Comm{
   int one;
   int c_contextid;
};
struct LAM_Win{
   struct LAM_Comm * comm;
};


//Record new window information and report it as a new resource
void DYNINSTrecordWindowInfo(unsigned int * WinId)
{
  int           newWindow;
  int WindowId;
  static int warned = 0;
  int WindowDx;
  rawTime64             ts;
  struct DynInstWin_st* WinSt;

//get context id of window
  if(whichMPI == LAM){
    struct LAM_Win * myWin = *(struct LAM_Win **)WinId;
    WindowId =  myWin->comm->c_contextid;
  }
  else if(whichMPI == MPICH){
    WindowId =(short) *WinId;
  }
  //else
    //fprintf(stderr, "Error: Unable to determine MPI implementation");

  WindowDx = (WindowId % DYNINSTWindowsLimit);

  // check if we've reached the limit on the number of new windows

  if(WinInfo.NumWins == DYNINSTWindowsLimit){
    if(!warned){
       fprintf(stderr,"The limit of the number of RMA windows supported by Paradyn has been reached - The max number of windows is set at %d.\n",DYNINSTWindowsLimit);
       warned = 1;
    }
    return;
  }


  /*
   * Find the info about the window we were given
   */  
   //fprintf(stderr, "in Record: windowDx is %d for WindowId %u\n",
             //WindowDx,WindowId);

/* search for already existing window, because although right now this function   is only called by MPI_Win_create, in the future, we may call it from any
   functions that use windows, e.g. MPI_Win_fence, MPI_Win_start... */

  WinSt = WinInfo.WindowTable[WindowDx];
  while((WinSt != NULL) && (WinSt->WinId != WindowId)) {
    WinSt = WinSt->Next;
  }
  if(WinSt == NULL) {
    /* We have not seen this Window before, so add a WinInfo struct for it. */
    WinSt = (DynInstWinSt *)malloc(sizeof(DynInstWinSt));
    assert(WinSt != NULL);

    WinSt->WinId = WindowId;
    WinSt->WinUniqueId = DYNINSTWindow_CreateUniqueId(WindowId, WindowDx);
    //fprintf(stderr,"WinUniqueId %d for WindowId %d\n",
             // WinSt->WinUniqueId,WindowId);

//have to wait until we see MPI_Win_set_name on this window to get WinName
    WinSt->WinName = NULL;
    WinSt->Next = WinInfo.WindowTable[WindowDx];
    WinInfo.WindowTable[WindowDx] = WinSt;
    WinInfo.NumWins++;
    newWindow = 1;
  } else {
    /* We have seen this window before - nothing to do */
    assert(WinSt->WinId == WindowId);
    newWindow = 0;
  }

  /* report new window to our daemon */
  if(newWindow)
    DYNINSTreportNewWindow(WinSt);
}

/************************************************************************
 * void DYNINSTreportNewWindow(void)
 *
 * Inform the paradyn daemons of new  RMA Windows
 *
************************************************************************/
void DYNINSTreportNewWindow(const struct DynInstWin_st * WinSt )
{
  int    dx;
  rawTime64 process_time;
  rawTime64 wall_time;
  struct _newresource newRes;

  process_time = DYNINSTgetCPUtime_hw();
  wall_time = DYNINSTgetWalltime();

    memset(&newRes, '\0', sizeof(newRes));
    sprintf(newRes.name, "SyncObject/Window/%d-%d",
              WinSt->WinId,WinSt->WinUniqueId);
      strcpy(newRes.abstraction, "BASE");
      newRes.mdlType = RES_TYPE_STRING;
      newRes.btype = WindowResourceType;

      DYNINSTgenerateTraceRecord(0, TR_NEW_RESOURCE,
                                 sizeof(struct _newresource), &newRes, 1,
                                 wall_time,process_time);
}
// report user-defined name for RMA window to the front-end
void DYNINSTnameWindow(unsigned int WindowId, char * name){
  struct DynInstWin_st* WinSt;
  int           WindowDx ;
  rawTime64 process_time;
  rawTime64 wall_time;  static int warned = 0;
  int WinId;
  struct _updtresource Res;

// get implementation dependent context id for window
  if(whichMPI == LAM){
    struct LAM_Win * lw = (struct LAM_Win *)WindowId;
    WinId = lw->comm->c_contextid;
  } 
  else if(whichMPI == MPICH){
    WinId =(short) WindowId;
  } 
  else{
    //fprintf(stderr, "Unable to determine MPI implementation.\n");
    return;
  }
  
  WindowDx = (WinId % DYNINSTWindowsLimit);

  process_time = DYNINSTgetCPUtime();
  wall_time = DYNINSTgetWalltime();
 
  WinSt = WinInfo.WindowTable[WindowDx];
  //fprintf(stderr, "in Name: windowDx is %d for WindowId %u name is %s\n",
            //WindowDx,WindowId, name);
  while((WinSt != NULL) && (WinSt->WinId != WinId)) {
    WinSt = WinSt->Next;
  }
  if(WinSt == NULL) {
     if(!warned && WinInfo.NumWins >= DYNINSTWindowsLimit){
         fprintf(stderr, "The number of RMA Windows supported by Paradyn has been reached. %u\n",WinId);
         warned = 1;
         return;
     }
     else if(WinInfo.NumWins < DYNINSTWindowsLimit){
        //hmmm this shouldn't happen
        fprintf(stderr, "Attempting to name an RMA Window not already picked up by MPI_Win_create: %u\n",WinId);
        return;
     }
     return;
  }   
  if(!name){
     //I would hope this wouldn't happen either
     fprintf(stderr, "Attempting to name an RMA Window a null value\n");
     return;                     
  }
  WinSt->WinName = (char*)malloc(strlen(name) +1);
  strcpy(WinSt->WinName,name);

  memset(&Res, '\0', sizeof(Res));
  sprintf(Res.name, "SyncObject/Window/%d-%d",
             WinSt->WinId, WinSt->WinUniqueId);
  sprintf(Res.displayname, "SyncObject/Window/%s",
             WinSt->WinName);
  strcpy(Res.abstraction, "BASE");
  Res.mdlType = RES_TYPE_STRING;
  Res.btype = WindowResourceType;  
  Res.retired = 0;

  DYNINSTgenerateTraceRecord(0,TR_UPDATE_RESOURCE,
                                 sizeof(struct _updtresource), &Res, 1,
                                 wall_time,process_time);
}

// report user-defined name for communicator to the frontend
void DYNINSTnameGroup(unsigned int gId, char * name){
  struct DynInstTag_st* tagSt;
  int           groupDx ;
  rawTime64 process_time;
  rawTime64 wall_time;
  static int warned = 0;
  int groupId;
  struct _updtresource Res;

//get implementation dependent context id for communicator
  if(whichMPI == LAM){
    struct LAM_Comm * lc = (struct LAM_Comm *)gId;
    groupId = lc->c_contextid;
  }
  else if(whichMPI == MPICH){
    groupId =(short) gId;
  }
  else{
    //fprintf(stderr, "Unable to determine MPI implementation.\n");
    return;
  }

  wall_time = DYNINSTgetWalltime ();
  process_time = DYNINSTgetCPUtime();

  //find the communicator
  groupDx = (groupId % DYNINSTagGroupsLimit);
  tagSt = TagGroupInfo.GroupTable[groupDx];
  while((tagSt != NULL) && (tagSt->TagGroupId != groupId)) {
      tagSt = tagSt->Next;
  }

  if(tagSt == NULL) {
     if(!warned && TagGroupInfo.NumGroups >= DYNINSTagGroupsLimit){
         fprintf(stderr, "The number of Commicators supported by Paradyn has been reached. %u\n",groupId);
         warned = 1;
         return;
     }
     else if(TagGroupInfo.NumGroups < DYNINSTagGroupsLimit){
        //we get here if they havent' sent any messages on the communicator yet
        //and the communicator wasn't created with MPI_Comm_create
          TagGroupInfo.TagHierarchy = 1;  /* TRUE; */
          DYNINSTrecordTagGroupInfo(-1, groupId);
          tagSt = TagGroupInfo.GroupTable[groupDx];
          while((tagSt != NULL) && (tagSt->TagGroupId != groupId)) {
              tagSt = tagSt->Next; 
          }
          assert(tagSt); //we just put it there. it should be there.
      }
  }                              
  if(!name){                     
     //I would hope this wouldn't happen either
     fprintf(stderr, "Attempting to name a Communicator a null value\n");
     return;
  }
  if(!strcmp(name, "")){
     //It seems that LAM names a communicator "" when it deallocates it
     //so this happens on MPI_Comm_free
   return;
  }
//fprintf(stderr,"name for group %d-%d \n", tagSt->TagGroupId, tagSt->TGUniqueId);
  memset(&Res, '\0', sizeof(Res));
  sprintf(Res.name, "SyncObject/Message/%d-%d",
             tagSt->TagGroupId,tagSt->TGUniqueId );
  sprintf(Res.displayname, "SyncObject/Message/%s", name);
  strcpy(Res.abstraction, "BASE");
  Res.mdlType = RES_TYPE_STRING;
  Res.btype = MessageGroupResourceType;
  Res.retired = 0;
    
  DYNINSTgenerateTraceRecord(0,TR_UPDATE_RESOURCE,
                                 sizeof(struct _updtresource), &Res, 1,
                                 wall_time,process_time);
} 

// when a communicator is deallocated with MPI_Comm_free report it as retired
// to the front end
void DYNINSTretireGroupTag(unsigned int * gId){
  struct DynInstTag_st* tagSt;
  struct DynInstTag_st* prevtagSt = NULL;
  int           groupDx ;
  rawTime64 process_time;
  rawTime64 wall_time;
  int groupId;
  int i = 0;
  struct _updtresource Res;
  int count = 0;

// get implementation dependent context id for communicator
  if(whichMPI == LAM){ 
     struct LAM_Comm * lc = (struct LAM_Comm *)*gId;
     groupId = lc->c_contextid;
  }
  else if(whichMPI == MPICH){
    groupId =(short)* gId;
  }
  else{
    //fprintf(stderr, "Unable to determine MPI implementation.\n");
    return;
  }
  
  groupDx = (groupId % DYNINSTagGroupsLimit);
  tagSt = TagGroupInfo.GroupTable[groupDx];
  while((tagSt != NULL) && (tagSt->TagGroupId != groupId)) {
      prevtagSt = tagSt;
      tagSt = tagSt->Next;
  }
  
  process_time = DYNINSTgetCPUtime();
  wall_time = DYNINSTgetWalltime();
  
  if(tagSt == NULL) {     //fprintf(stderr, "Attempting to retire a communicator we haven't seen yet %u\n",groupId);
     /* this could happen if a communicator is freed that we didn't see any
        messages for */
     return;
  }//fprintf(stderr,"found group %u-%d and sending retire request\n",tagSt->TagGroupId, tagSt->TGUniqueId);
  
/* retire tags first */
  
  for(i = 0 ; i < DYNINSTagsLimit && count < tagSt->NumTags; ++i){
//fprintf(stderr,"tagSt->NumTags is %d for group %d-%d tag is %d\n",tagSt->NumTags, tagSt->TagGroupId, tagSt->TGUniqueId, tagSt->TagTable[i]);
    if(tagSt->TagTable[i] != DYNINSTGroupUndefinedTag){
       ++count;
       memset(&Res, '\0', sizeof(Res));
 sprintf(Res.name, "SyncObject/Message/%d-%d/%d",
             tagSt->TagGroupId, tagSt->TGUniqueId, tagSt->TagTable[i]);
       sprintf(Res.displayname, "SyncObject/Message/%d-%d/%s",
             tagSt->TagGroupId, tagSt->TGUniqueId, "");
       strcpy(Res.abstraction, "BASE");
       Res.mdlType = RES_TYPE_INT;
       Res.btype = MessageTagResourceType;
       Res.retired = 1;

       DYNINSTgenerateTraceRecord(0,TR_UPDATE_RESOURCE,
                                 sizeof(struct _updtresource), &Res, 1,
                                 wall_time,process_time);
    }
  }

/* retire communicator */
  memset(&Res, '\0', sizeof(Res));
  sprintf(Res.name, "SyncObject/Message/%d-%d",
             tagSt->TagGroupId, tagSt->TGUniqueId);
  sprintf(Res.displayname, "SyncObject/Message/%s", "");
  strcpy(Res.abstraction, "BASE");
  Res.mdlType = RES_TYPE_STRING;
  Res.btype = MessageTagResourceType;
  Res.retired = 1;

  DYNINSTgenerateTraceRecord(0,TR_UPDATE_RESOURCE,
                                 sizeof(struct _updtresource), &Res, 1,
                                 wall_time,process_time);

  /* remove the group from the GroupTable */

assert(tagSt);
  if(!prevtagSt)
    TagGroupInfo.GroupTable[groupDx] = tagSt->Next;
  else
    prevtagSt->Next = tagSt->Next;
  free(tagSt);
  tagSt = NULL;
  --TagGroupInfo.NumGroups;
}


// when a window is deallocated with MPI_Win_free, report it to the frontend
//as retired
void DYNINSTretireWindow(unsigned int * WindowId){
  struct DynInstWin_st* WinSt;
  struct DynInstWin_st* prevWinSt = NULL;
  int           WindowDx ;
  rawTime64 process_time;
  rawTime64 wall_time;
  int WinId;
  struct _updtresource Res; 

//get implementation dependent context id for window
  if(whichMPI == LAM){
    struct LAM_Win * lw = (struct LAM_Win *)*WindowId;
    WinId = lw->comm->c_contextid;
  }    
  else if(whichMPI == MPICH){
    WinId =(short) *WindowId;
  }    
  else{
    //fprintf(stderr,"Unable to determine MPI implementation.\n");
    return;
  }    
                                 
  WindowDx = (WinId % DYNINSTWindowsLimit);
    
  process_time = DYNINSTgetCPUtime();
  wall_time = DYNINSTgetWalltime();

  WinSt = WinInfo.WindowTable[WindowDx];
  //fprintf(stderr, "in retire: windowDx is %d for WinId %u\n",WindowDx,WinId);
  while((WinSt != NULL) && (WinSt->WinId != WinId)) {
    prevWinSt = WinSt;
    WinSt = WinSt->Next;
  }
  if(WinSt == NULL) {
     fprintf(stderr, "Attempting to retire a window we haven't seen yet %u\n",WinId);
     return;
  }                              
//fprintf(stderr,"found window %u-%d and sending retire request\n",WinSt->WinId, WinSt->WinUniqueId);
  memset(&Res, '\0', sizeof(Res));
  sprintf(Res.name, "SyncObject/Window/%u-%d",
             WinSt->WinId, WinSt->WinUniqueId);
  if (WinSt->WinName)
    sprintf(Res.displayname, "SyncObject/Window/%s", WinSt->WinName);
  else
    sprintf(Res.displayname, "SyncObject/Window/%s", "");
  strcpy(Res.abstraction, "BASE");
  Res.mdlType = RES_TYPE_STRING;
  Res.btype = WindowResourceType; 
  Res.retired = 1;

  DYNINSTgenerateTraceRecord(0,TR_UPDATE_RESOURCE, 
                                 sizeof(struct _updtresource), &Res, 1,
                                 wall_time,process_time);
  
  /* remove the window from the WindowTable */
  assert(WinSt);
if(!prevWinSt)
    WinInfo.WindowTable[WindowDx] = WinSt->Next;
  else
    prevWinSt->Next = WinSt->Next;
  free(WinSt);
  WinSt = NULL;
  --WinInfo.NumWins; 
}

/************************************************************************
 * 
 * DYNINSTrecordTag
 * DYNINSTrecordTagAndGroup
 * DYNINSTrecordGroup
 *
 * Handle recording of (possibly) new tag and/or group.
 * Message-passing routines such as MPI_Send are instrumented
 * to call these functions.
 *
 ************************************************************************/

void DYNINSTrecordTag(int tagId)
{
  assert(tagId >= 0);
  assert(TagGroupInfo.TagHierarchy == 0); /* FALSE); */
  DYNINSTrecordTagGroupInfo(tagId, -1);
}

void DYNINSTrecordTagAndGroup(int tagId, unsigned groupId)
{
  int gId = -1;
  assert(tagId >= 0);
  if(whichMPI == LAM){
    struct LAM_Comm * lc = (struct LAM_Comm*)groupId;
    gId = lc->c_contextid;
  }
  else if(whichMPI == MPICH){
    gId = (short)groupId;
  }
  else{
    gId = (int)groupId;
  }
  TagGroupInfo.TagHierarchy = 1; /* TRUE; */
  DYNINSTrecordTagGroupInfo(tagId, gId );
}

void DYNINSTrecordGroup(unsigned groupId)
{
  int gId = -1;
  if(whichMPI == LAM){
    struct LAM_Comm * lc = (struct LAM_Comm*)groupId;
    gId = lc->c_contextid;
  }
  else if(whichMPI == MPICH){
    gId = (short)groupId;
  }
  else{
    gId = (int)groupId;
  }
  TagGroupInfo.TagHierarchy = 1;  /* TRUE; */
  DYNINSTrecordTagGroupInfo(-1, gId);
}

//record new RMA window
void DYNINSTrecordWindow(unsigned int WindowId)
{
  DYNINSTrecordWindowInfo((unsigned int *)WindowId);
}

/************************************************************************
 * DYNINST test functions.
 *
 * Since these routines aren't used regularly, shouldn't they be surrounded
 * with an ifdef? --ari
************************************************************************/
#ifdef notused
void DYNINSTsimplePrint(void) {
    printf("inside dynamic inst function\n");
}

void DYNINSTentryPrint(int arg) {
    printf("enter %d\n", arg);
}

void DYNINSTcallFrom(int arg) {
    printf("call from %d\n", arg);
}

void DYNINSTcallReturn(int arg) {
    printf("return to %d\n", arg);
}

void DYNINSTexitPrint(int arg) {
    printf("exit %d\n", arg);
}
#endif

/************************************************************************
 *
 ************************************************************************/
int DYNINSTGroup_CreateUniqueId(int commId, int dx)
{
#ifdef PARADYN_MPI
  MPI_Group commGroup;
  MPI_Group worldGroup;
  int       ranks1[1];
  int       ranks2[1];
  int       rc;

  /* assert(commId < (1<<16)); */
  assert(commId < 100000);
  
  rc = MPI_Comm_group(commId, &commGroup);
  assert(rc == 0);
  rc = MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);
  assert(rc == 0);
  
  ranks1[0] = 0;
  rc = MPI_Group_translate_ranks(commGroup, 1, ranks1, worldGroup, ranks2);
  assert(rc == 0);

  assert(ranks2[0] != MPI_UNDEFINED);
  /* assert(ranks2[0] < (1 << 16));
     return(commId + (ranks2[0] * (1<<16)));
  */
  assert(ranks2[0] < 20000);
  return(commId + (ranks2[0] * 100000));
#else
// maintain unique identifiers for communicators as their implementation
//dependent context ids may be reused by the MPI implementation
  return TagGroupInfo.GroupCounters[dx]++;

#endif
}

/*
 * RMA windows
 * maintain a unique identifier for each window because the MPI implementation
 * may reuse a window's context id after it is deallocated
*/
unsigned int DYNINSTWindow_CreateUniqueId(unsigned int WindowId, int dx){
  return WinInfo.WindowCounters[dx]++;
}

/************************************************************************
 *
 ************************************************************************/
int DYNINSTGroup_CreateLocalId(int tgUniqueId)
{
#ifdef PARADYN_MPI
  /* return(tgUniqueId | ((1<<16)-1)); */
  return(tgUniqueId % 100000);
#else
  return(tgUniqueId);
#endif
}


/************************************************************************
 *
 ************************************************************************/
int DYNINSTGroup_FindUniqueId(unsigned gId, char * constraint)
{
  int           groupDx;
  int           groupId;
  DynInstTagSt* tagSt;

//get implementation dependent context id for communicator
  if(whichMPI == LAM){
    struct LAM_Comm * lc = (struct LAM_Comm *)gId;
    groupId = lc->c_contextid;
  }
  else if(whichMPI == MPICH){
    groupId = (short)gId;
  }
  //else{
    //fprintf(stderr, "Unable to determine MPI implementation.\n");
  //}

  groupDx = groupId % DYNINSTagGroupsLimit;

  for(tagSt = TagGroupInfo.GroupTable[groupDx]; tagSt != NULL;
      tagSt = tagSt->Next)
  {
    if(tagSt->TagGroupId == groupId){
 
      // allows two numbers 1024 digits long + a dash and a '\0'
      char temp[2051];
      sprintf(temp,"%d-%d",tagSt->TagGroupId, tagSt->TGUniqueId);
      assert(constraint);

      //fprintf(stderr,"constraint is %s temp is %s\n", constraint, temp);
      if (!strcmp(temp, constraint)){
        //match!
         return 1;
      }
      return 0;
    }
  }

  return(0);
}

//find the indentifier of the RMA window in question
int DYNINSTWindow_FindUniqueId(unsigned int WindowId, char * constraint)
{
  int           WinDx;
  DynInstWinSt* WinSt = NULL;
  int WinId;

//get the implementation dependent context id of the window
  if(whichMPI == LAM){
    struct LAM_Win * lw = (struct LAM_Win *)WindowId;
    WinId = lw->comm->c_contextid;
  }
  else if(whichMPI == MPICH){
    WinId = (short)WindowId;
  }
  //else{
    //fprintf(stderr, "Unable to determine MPI implementation.\n");
  //}

  WinDx = (WinId % DYNINSTWindowsLimit);

  WinSt = WinInfo.WindowTable[WinDx];
  while((WinSt != NULL) && (WinSt->WinId != WinId)) {
    WinSt = WinSt->Next;
  }
  if(WinSt == NULL) {
     //hmmm this shouldn't happen unless there is a bug somewhere
     // or if paradyn supports attach for MPI programs
     fprintf(stderr, "trying to FIND a window we haven't seen before %u\n",WinId);
     return (-1);
  }

  if(WinSt->WinId == WinId){

      // allows two numbers 1024 digits long + a dash and a '\0'
      char temp[2051];
      sprintf(temp,"%d-%d",WinSt->WinId, WinSt->WinUniqueId);
      //fprintf(stderr,"constraint is %s temp is %s\n", constraint, temp);
      assert(constraint);
      if (!strcmp(temp, constraint)){
         //match
         return 1;
      }
      return 0;
  }

  return(-1);
}

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

void DYNINSTRegisterCallee(unsigned int calleeAddress,
			   unsigned callSiteAddress){
  if(DYNINSTCalleeSearch(callSiteAddress, calleeAddress) == 0){
    struct callercalleeStruct c;
    c.caller = callSiteAddress;
    c.callee = calleeAddress;
    DYNINSTgenerateTraceRecord(0, TR_DYNAMIC_CALLEE_FOUND,
			       sizeof(struct callercalleeStruct), 
			       &c, 1, DYNINSTgetWalltime(), 
			       DYNINSTgetCPUtime());
  }
}

/*Needs to return 1 if the call site has been seen, 0 otherwise*/
int DYNINSTCalleeSearch(unsigned int callSiteAddr,
			unsigned int calleeAddr){
  
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
      curr->next_callee = (struct callee *) 
	malloc(sizeof(struct callee));
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
      
      curr_callee = (struct callee *)
	malloc(sizeof(struct callee));
      curr_callee->next = curr->next_callee;
      curr_callee->address = calleeAddr;
      curr->next_callee = curr_callee;
      return 0;
    }
  }
}

#define KNOWN_MPI_TYPES 34

/*
  This is an approximation to the real MPI_Pack_size routine,
  which may not be present in an MPI application
  Constants are given for MPICH-1.2.0 on x86/Linux
*/
int MPI_Pack_size (int incount, int datatype, int comm, int *size)
{
	const int type_size[KNOWN_MPI_TYPES+1] = {
		0, 1, 1, 1, 2, 2, 4, 4, 4, 4, 
		4, 8,12, 8, 1, 0, 0, 8,12, 8, 
		6, 8,16, 8,16, 4, 4, 8, 4, 8,
		16,32, 8,16};
	static int warned = 0;

	if (!warned) {
		fprintf(stderr, "WARNING: Bogus MPI_Pack_size: relink "
			"the application with \"-u MPI_Pack_size\" "
			"for accurate results\n");
		warned = 1;
	}
	if (datatype <= 0 || datatype > KNOWN_MPI_TYPES) {
		fprintf(stderr, "WARNING: Paradyn_Pack_size: unknown type\n");
		*size = 0;
		return (-1);
	}
	*size = incount * type_size[datatype] + 16;
	
	return 0;
}




