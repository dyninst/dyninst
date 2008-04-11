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
 * $Id: RTlinux.c,v 1.54 2008/04/11 23:30:44 legendre Exp $
 * RTlinux.c: mutatee-side library function specific to Linux
 ************************************************************************/

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "dyninstAPI_RT/src/RTthread.h"
#include "dyninstAPI_RT/src/RTcommon.h"
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>

extern double DYNINSTstaticHeap_512K_lowmemHeap_1[];
extern double DYNINSTstaticHeap_16M_anyHeap_1[];
extern unsigned long sizeOfLowMemHeap1;
extern unsigned long sizeOfAnyHeap1;

/************************************************************************
 * void DYNINSTbreakPoint(void)
 *
 * stop oneself.
************************************************************************/

void DYNINSTbreakPoint()
{
   /* We set a global flag here so that we can tell
      if we're ever in a call to this when we get a 
      SIGBUS */
   DYNINST_break_point_event = 1;
   while (DYNINST_break_point_event)  {
      kill(dyn_lwp_self(), DYNINST_BREAKPOINT_SIGNUM);
   }
   /* Mutator resets to 0... */
}

static int failed_breakpoint = 0;
void uncaught_breakpoint(int sig)
{
   failed_breakpoint = 1;
}

void DYNINSTlinuxBreakPoint()
{
   struct sigaction act, oldact;
   int result;
   
   memset(&oldact, 0, sizeof(struct sigaction));
   memset(&act, 0, sizeof(struct sigaction));

   result = sigaction(DYNINST_BREAKPOINT_SIGNUM, NULL, &act);
   if (result == -1) {
      perror("DyninstRT library failed sigaction1");
      return;
   }
   act.sa_handler = uncaught_breakpoint;

   result = sigaction(DYNINST_BREAKPOINT_SIGNUM, &act, &oldact);
   if (result == -1) {
      perror("DyninstRT library failed sigaction2");
      return;
   }

   DYNINST_break_point_event = 1;
   failed_breakpoint = 0;
   kill(dyn_lwp_self(), DYNINST_BREAKPOINT_SIGNUM);
   if (failed_breakpoint) {
      DYNINST_break_point_event = 0;
      failed_breakpoint = 0;
   }

   result = sigaction(DYNINST_BREAKPOINT_SIGNUM, &oldact, NULL);
   if (result == -1) {
      perror("DyninstRT library failed sigaction3");
      return;
   }
}

void DYNINSTsafeBreakPoint()
{
    DYNINST_break_point_event = 2; /* Not the same as above */
    while (DYNINST_break_point_event)
        kill(dyn_lwp_self(), SIGSTOP);
}

void mark_heaps_exec() {
	RTprintf( "*** Initializing dyninstAPI runtime.\n" );

	/* Grab the page size, to align the heap pointer. */
	long int pageSize = sysconf( _SC_PAGESIZE );
	if( pageSize == 0 || pageSize == - 1 ) {
		fprintf( stderr, "*** Failed to obtain page size, guessing 16K.\n" );
		perror( "mark_heaps_exec" );
		pageSize = 1024 * 16;
		} /* end pageSize initialization */

	/* Align the heap pointer. */
	unsigned long int alignedHeapPointer = (unsigned long int) DYNINSTstaticHeap_16M_anyHeap_1;
	alignedHeapPointer = (alignedHeapPointer) & ~(pageSize - 1);
	unsigned long int adjustedSize = (unsigned long int) DYNINSTstaticHeap_16M_anyHeap_1 - alignedHeapPointer + sizeOfAnyHeap1;

	/* Make the heap's page executable. */
	int result = mprotect( (void *) alignedHeapPointer, (size_t) adjustedSize, PROT_READ | PROT_WRITE | PROT_EXEC );
	if( result != 0 ) {
		fprintf( stderr, "%s[%d]: Couldn't make DYNINSTstaticHeap_16M_anyHeap_1 executable!\n", __FILE__, __LINE__);
		perror( "mark_heaps_exec" );
		}
	RTprintf( "*** Marked memory from 0x%lx to 0x%lx executable.\n", alignedHeapPointer, alignedHeapPointer + adjustedSize );

	/* Mark _both_ heaps executable. */
	alignedHeapPointer = (unsigned long int) DYNINSTstaticHeap_512K_lowmemHeap_1;
	alignedHeapPointer = (alignedHeapPointer) & ~(pageSize - 1);
	adjustedSize = (unsigned long int) DYNINSTstaticHeap_512K_lowmemHeap_1 - alignedHeapPointer + sizeOfLowMemHeap1;

	/* Make the heap's page executable. */
	result = mprotect( (void *) alignedHeapPointer, (size_t) adjustedSize, PROT_READ | PROT_WRITE | PROT_EXEC );
	if( result != 0 ) {
		fprintf( stderr, "%s[%d]: Couldn't make DYNINSTstaticHeap_512K_lowmemHeap_1 executable!\n", __FILE__, __LINE__ );
		perror( "mark_heaps_exec" );
		}
	RTprintf( "*** Marked memory from 0x%lx to 0x%lx executable.\n", alignedHeapPointer, alignedHeapPointer + adjustedSize );
	} /* end mark_heaps_exec() */

#if defined(arch_ia64)
/* Ensure we an executable block of memory. */
void R_BRK_TARGET() {
        /* Make sure we've got room for two bundles. */
        asm( "nop 0" ); asm( "nop 0" ); asm( "nop 0" );
        asm( "nop 0" ); asm( "nop 0" ); asm( "nop 0" );
        }
#endif /*arch_ia64*/

/************************************************************************
 * void DYNINSTos_init(void)
 *
 * OS initialization function
************************************************************************/
int DYNINST_sysEntry;
void DYNINSTos_init(int calledByFork, int calledByAttach)
{
   RTprintf("DYNINSTos_init(%d,%d)\n", calledByFork, calledByAttach);

#if defined(arch_x86)
   /**
    * The following line reads the vsyscall entry point directly from
    * it's stored, which can then be used by the mutator to locate
    * the vsyscall page.
    * The problem is, I don't know if this memory read is valid on
    * older systems--It could cause a segfault.  I'm going to leave
    * it commented out for now, until further investigation.
    * -Matt 1/18/06
    **/
   //__asm("movl %%gs:0x10, %%eax\n" : "=r" (DYNINST_sysEntry))
#endif
}

typedef struct dlopen_args {
  const char *libname;
  int mode;
  void *result;
  void *caller;
} dlopen_args_t;

void *(*DYNINST_do_dlopen)(dlopen_args_t *) = NULL;

static int get_dlopen_error() {
   char *err_str;
   err_str = dlerror();
   if (err_str) {
      strncpy(gLoadLibraryErrorString, err_str, (size_t) ERROR_STRING_LENGTH);
      return 1;
   }
   else {
      sprintf(gLoadLibraryErrorString,"unknown error with dlopen");
      return 0;
   }
   return 0;
}

int DYNINSTloadLibrary(char *libname)
{
   void *res;
   gLoadLibraryErrorString[0]='\0';
   gBRKptr = sbrk(0);
   res = dlopen(libname, RTLD_NOW | RTLD_GLOBAL);
   if (res)
   {
      return 1;
   }
 
   get_dlopen_error();
#if defined(arch_x86)
   /* dlopen on recent glibcs has a "security check" so that
      only registered modules can call it. Unfortunately, progs
      that don't include libdl break this check, so that we
      can only call _dl_open (the dlopen worker function) from
      within glibc. We do this by calling do_dlopen
      We fool this check by calling an addr written by the
      mutator */
   if (strstr(gLoadLibraryErrorString, "invalid caller") != NULL &&
       DYNINST_do_dlopen != NULL) {
      dlopen_args_t args;
      args.libname = libname;
      args.mode = RTLD_NOW | RTLD_GLOBAL;
      args.result = 0;
      args.caller = (void *)DYNINST_do_dlopen;
      // There's a do_dlopen function in glibc. However, it's _not_
      // exported; thus, getting the address is a bit of a pain. 
      
      (*DYNINST_do_dlopen)(&args);
      // Duplicate the above
      if (args.result != NULL)
      {
         return 1;
      }
      else
         get_dlopen_error();
   }
#endif
   return 0;
}

//Define this value so that we can compile on a system that doesn't have
// gettid and still run on one that does.
#if !defined(SYS_gettid)

#if defined(arch_x86)
#define SYS_gettid 224
#elif defined(arch_x86_64)
#define SYS_gettid 186
#elif defined(arch_ia64)
#define SYS_gettid 1105
#endif

#endif

int dyn_lwp_self()
{
   static int gettid_not_valid = 0;
   int result;

   if (gettid_not_valid)
      return getpid();

   result = syscall((long int) SYS_gettid);
   if (result == -1 && errno == ENOSYS)
   {
      gettid_not_valid = 1;
      return getpid();
   }
   return result;  
}

int dyn_pid_self()
{
   return getpid();
}

dyntid_t (*DYNINST_pthread_self)(void);
dyntid_t dyn_pthread_self()
{
   dyntid_t me;
   if (!DYNINST_pthread_self) {
      return (dyntid_t) DYNINST_SINGLETHREADED;
   }
   me = (*DYNINST_pthread_self)();
   return (dyntid_t) me;
}



/* 
   We reserve index 0 for the initial thread. This value varies by
   platform but is always constant for that platform. Wrap that
   platform-ness here. 
*/
int DYNINST_am_initial_thread( dyntid_t tid ) {
	if( dyn_lwp_self() == getpid() ) {
		return 1;
   }
	return 0;
} /* end DYNINST_am_initial_thread() */

/**
 * We need to extract certain pieces of information from the usually opaque 
 * type pthread_t.  Unfortunately, libc doesn't export this information so 
 * we're reverse engineering it out of pthread_t.  The following structure
 * tells us at what offsets it look off of the return value of pthread_self
 * for the lwp, pid, initial start function (the one passed to pthread_create)
 * and the top of this thread's stack.  We have multiple entries in positions, 
 * one for each different version of libc we've seen.
 **/
#define READ_FROM_BUF(pos, type) *((type *)(buffer+pos))

typedef struct pthread_offset_t
{
   unsigned lwp_pos;
   unsigned pid_pos;
   unsigned start_func_pos;
   unsigned stck_start_pos;
} pthread_offset_t;

#if defined(arch_x86)
#define POS_ENTRIES 4
static pthread_offset_t positions[POS_ENTRIES] = { { 72, 476, 516, 576 },
                                                   { 72, 76, 516, 84 },
                                                   { 72, 76, 532, 96 },
                                                   { 72, 476, 516, 80 } };
#else
//x86_64 and ia64 share structrues
#define POS_ENTRIES 3
static pthread_offset_t positions[POS_ENTRIES] = { { 144, 952, 1008, 160 },
                                                   { 144, 148, 1000, 160 },
                                                   { 144, 148, 1000, 688 } };

#if defined(MUTATEE_32)
/* ccw 28 apr 2006: the offsets for 32 bit mutatees on amd64*/
#define POS_ENTRIES32 3
static pthread_offset_t positions32[POS_ENTRIES32] = { { 72, 476, 516, 576 },
                                                   	{ 72, 76, 516, 84 },
							{ 72, 476, 516, 80 }}; 
#endif
#endif

int DYNINSTthreadInfo(BPatch_newThreadEventRecord *ev)
{
  static int err_printed = 0;
  int i;
  char *buffer;

  ev->stack_addr = 0x0;
  ev->start_pc = 0x0;
  buffer = (char *) ev->tid;

#if !defined(MUTATEE_32)  
  for (i = 0; i < POS_ENTRIES; i++)
  {
     pid_t pid = READ_FROM_BUF(positions[i].pid_pos, pid_t);
     int lwp = READ_FROM_BUF(positions[i].lwp_pos, int);

     if( pid != ev->ppid || lwp != ev->lwp ) {
        // /* DEBUG */ fprintf( stderr, "%s[%d]: pid %d != ev->ppid %d or lwp %d != ev->lwp %d\n", __FILE__, __LINE__, pid, ev->ppid, lwp, ev->lwp );
        continue;
        }

        void * stack_addr = READ_FROM_BUF(positions[i].stck_start_pos, void *);
        void * start_pc = READ_FROM_BUF(positions[i].start_func_pos, void *);

        // Sanity checking. There are multiple different places that we have
        // found the stack address for a given pair of pid/lwpid locations,
        // so we need to do the best job we can of verifying that we've
        // identified the real stack. 
        //
        // Currently we just check for known incorrect values. We should
        // generalize this to check for whether the address is in a valid
        // memory region, but it is not clear whether that information is
        // available at this point.
        //
        
        if(stack_addr == (void*)0 || stack_addr == (void*)0xffffffec) {
            continue;
        }

        ev->stack_addr = stack_addr;
        ev->start_pc = start_pc;

           /* DEBUG */ /* fprintf( stderr, "%s[%d]: pid_pos %d, lwp_pos %d " 
                                       "stck_start_pos: %d start_func_pos %d\n",
            __FILE__, __LINE__, 
            positions[i].pid_pos, 
            positions[i].lwp_pos,
            positions[i].stck_start_pos,
            positions[i].start_func_pos); */

         // /* DEBUG */ fprintf( stderr, "%s[%d]: stack_addr %p, start_pc %p\n", __FILE__, __LINE__, ev->stack_addr, ev->start_pc );
     return 1;
  }

#else  /* the offsets for 32 bit mutatees on amd64*/
    for (i = 0; i < POS_ENTRIES32; i++)
  {
     pid_t pid = READ_FROM_BUF(positions32[i].pid_pos, pid_t);
     int lwp = READ_FROM_BUF(positions32[i].lwp_pos, int);
     /*unsigned int start_pc =  (unsigned) READ_FROM_BUF(positions32[i].start_func_pos, void*);*/
     if( pid != ev->ppid || lwp != ev->lwp){
        // /* DEBUG */ fprintf( stderr, "%s[%d]: pid %d != ev->ppid %d or lwp %d != ev->lwp %d\n", __FILE__, __LINE__, pid, ev->ppid, lwp, ev->lwp );
        continue;
        }
     ev->stack_addr = READ_FROM_BUF(positions32[i].stck_start_pos, void *);
     ev->start_pc = READ_FROM_BUF(positions32[i].start_func_pos, void *);
     // /* DEBUG */ fprintf( stderr, "%s[%d]: stack_addr %p, start_pc %p\n", __FILE__, __LINE__, ev->stack_addr, ev->start_pc );
     return 1;
  }
#endif
  if (!err_printed)
  {
    //If you get this error, then Dyninst is having trouble figuring out
    //how to read the information from the positions structure above.
    //It needs a new entry filled in.  Running the commented out program
    //at the end of this file can help you collect the necessary data.
    RTprintf( "[%s:%d] Unable to parse the pthread_t structure for this version of libpthread.  Making a best guess effort.\n",  __FILE__, __LINE__ );
    err_printed = 1;
  }
   
  return 1;
}

#if defined(cap_mutatee_traps)

#include <ucontext.h>

#if defined(arch_x86) || defined(MUTATEE_32)

#if !defined(REG_EIP)
#define REG_EIP 14
#endif
#if !defined(REG_ESP)
#define REG_ESP 7
#endif
#define REG_IP REG_EIP
#define REG_SP REG_ESP
#define MAX_REG 18

#elif defined(arch_x86_64)

#if !defined(REG_RIP)
#define REG_RIP 16
#endif
#if !defined(REG_RSP)
#define REG_RSP 15
#endif
#define REG_IP REG_RIP
#define REG_SP REG_RSP
#define MAX_REG 15
#if !defined(REG_RAX)
#define REG_RAX 13
#endif
#if !defined(REG_R10)
#define REG_R10 2
#endif
#if !defined(REG_R11)
#define REG_R11 3
#endif

#endif

extern void dyninstSetupContext(ucontext_t *context, unsigned long flags, void *retPoint);

/**
 * Called by the SIGTRAP handler, dyninstTrapHandler.  This function is 
 * closly intwined with dyninstTrapHandler, don't modify one without 
 * understanding the other.
 *
 * This function sets up the calling context that was passed to the
 * SIGTRAP handler so that control will be redirected to our instrumentation
 * when we do the setcontext call.
 * 
 * There are a couple things that make this more difficult than it should be:
 *   1. The OS provided calling context is similar to the GLIBC calling context,
 *      but not compatible.  We'll create a new GLIBC compatible context and
 *      copy the possibly stomped registers from the OS context into it.  The
 *      incompatiblities seem to deal with FP and other special purpose registers.
 *   2. setcontext doesn't restore the flags register.  Thus dyninstTrapHandler
 *      will save the flags register first thing and pass us its value in the
 *      flags parameter.  We'll then push the instrumentation entry and flags
 *      onto the context's stack.  Instead of transfering control straight to the
 *      instrumentation, we'll actually go back to dyninstTrapHandler, which will
 *      do a popf/ret to restore flags and go to instrumentation.  The 'retPoint'
 *      parameter is the address in dyninstTrapHandler the popf/ret can be found.
 **/
void dyninstSetupContext(ucontext_t *context, unsigned long flags, void *retPoint)
{
   ucontext_t newcontext;
   unsigned i;
   unsigned long *orig_sp;
   void *orig_ip;

   getcontext(&newcontext);
   
   //Set up the 'context' parameter so that when we restore 'context' control
   // will get transfered to our instrumentation.
   for (i=0; i<MAX_REG; i++) {
      newcontext.uc_mcontext.gregs[i] = context->uc_mcontext.gregs[i];
   }

   orig_sp = (unsigned long *) context->uc_mcontext.gregs[REG_SP];
   orig_ip = (void *) context->uc_mcontext.gregs[REG_IP];

   assert(orig_ip);

   //Set up the PC to go to the 'ret_point' in RTsignal-x86.s
   newcontext.uc_mcontext.gregs[REG_IP] = (unsigned long) retPoint;

   //simulate a "push" of the flags and instrumentation entry points onto
   // the stack.
   *(orig_sp - 1) = (unsigned long) dyninstTrapTranslate(orig_ip);
   *(orig_sp - 2) = flags;
   unsigned shift = 2;
#if defined(arch_x86_64) && !defined(MUTATEE_32)
   *(orig_sp - 3) = context->uc_mcontext.gregs[REG_R11];
   *(orig_sp - 4) = context->uc_mcontext.gregs[REG_R10];
   *(orig_sp - 5) = context->uc_mcontext.gregs[REG_RAX];
   shift = 5;
#endif
   newcontext.uc_mcontext.gregs[REG_SP] = (unsigned long) (orig_sp - shift);

   //Restore the context.  This will move all the register values of 
   // context into the actual registers and transfer control away from
   // this function.  This function shouldn't actually return.
   setcontext(&newcontext);
   assert(0);
}

#endif /* cap_mutatee_traps */

/*
//Small program for finding the correct values to fill in pos_in_pthreadt
// above
#include <pthread.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#define gettid() syscall(SYS_gettid)

pthread_attr_t attr;

void *foo(void *f) {
  pid_t pid, tid;
  unsigned stack_addr;
  unsigned best_stack = 0xffffffff;
  int best_stack_pos = 0;
  void *start_func;
  int *p;
  int i = 0;
  pid = getpid();
  tid = gettid();
  start_func = foo;
  //x86 only.  
  asm("movl %%ebp,%0" : "=r" (stack_addr));
  p = (int *) pthread_self();
  while (i < 1000)
  {
    if (*p == (unsigned) pid)
      printf("pid @ %d\n", i);
    if (*p == (unsigned) tid)
      printf("lwp @ %d\n", i);
    if (*p > stack_addr && *p < best_stack)
    {
      best_stack = *p;
      best_stack_pos = i;
    }
    if (*p == (unsigned) start_func)
      printf("func @ %d\n", i);
    i += sizeof(int);
    p++;
  }  
  printf("stack @ %d\n", best_stack_pos);
  return NULL;
}

int main(int argc, char *argv[])
{
  pthread_t t;
  void *result;
  pthread_attr_init(&attr);
  pthread_create(&t, &attr, foo, NULL);
  pthread_join(t, &result);
  return 0;
}
*/

