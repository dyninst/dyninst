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
 * $Id: RTlinux.c,v 1.35 2005/10/17 15:49:27 legendre Exp $
 * RTlinux.c: mutatee-side library function specific to Linux
 ************************************************************************/

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <string.h>

#if defined(arch_ia64) || defined(arch_x86_64)

#include <errno.h>
#include <sys/mman.h>

extern double DYNINSTstaticHeap_32K_lowmemHeap_1[];
extern double DYNINSTstaticHeap_4M_anyHeap_1[];

void _start( void ) 
{
   int result;
	RTprintf("*** Initializing dyninstAPI runtime.\n" );

	/* Grab the page size, to align the heap pointer. */
	long int pageSize = sysconf( _SC_PAGESIZE );
	if( pageSize == 0 || pageSize == - 1 ) {
		fprintf( stderr, "*** Failed to obtain page size, guessing 16K.\n" );
		perror( "_start" );
		pageSize = 1024 * 16;
   } /* end pageSize initialization */

	/* Align the heap pointer. */
	unsigned long int alignedHeapPointer = 
      (unsigned long int) DYNINSTstaticHeap_4M_anyHeap_1;
	unsigned long long int adjustedSize = alignedHeapPointer + (4 * 1024 * 1024);
	alignedHeapPointer = (alignedHeapPointer) & ~(pageSize - 1);
	adjustedSize -= alignedHeapPointer;

	/* Make the heap's page executable. */
   result = mprotect((void *) alignedHeapPointer, (size_t) adjustedSize, 
                     PROT_READ | PROT_WRITE | PROT_EXEC);
   if (result != 0)
   {
		fprintf(stderr, 
        "[%s:%d]: Couldn't make DYNINSTstaticHeap_4M_anyHeap_1 executable!\n",
              __FILE__, __LINE__);
		perror( "_start" );
   }
	RTprintf("*** Marked memory from 0x%lx to 0x%lx executable.\n", 
            alignedHeapPointer, 
            alignedHeapPointer + adjustedSize );

	/* Mark _both_ heaps executable. */
	alignedHeapPointer = (unsigned long int) DYNINSTstaticHeap_32K_lowmemHeap_1;
	adjustedSize = alignedHeapPointer + 32 * 1024;
	alignedHeapPointer = (alignedHeapPointer) & ~(pageSize - 1);
	adjustedSize -= alignedHeapPointer;

	/* Make the heap's page executable. */
   result = mprotect((void *) alignedHeapPointer, (size_t) adjustedSize, 
                     PROT_READ | PROT_WRITE | PROT_EXEC );
   if (result != 0 ) 
   {
		fprintf(stderr, 
        "[%s:%d]: Couldn't make DYNINSTstaticHeap_4M_anyHeap_1 executable!\n",
              __FILE__, __LINE__);
		perror( "_start" );
   }
	RTprintf("*** Marked memory from 0x%lx to 0x%lx executable.\n", 
            alignedHeapPointer, alignedHeapPointer + adjustedSize );
}
#endif

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

void DYNINSTos_init(int calledByFork, int calledByAttach)
{
    RTprintf("DYNINSTos_init(%d,%d)\n", calledByFork, calledByAttach);
}

typedef struct dlopen_args {
  const char *libname;
  int mode;
  void *result;
  void *caller;
} dlopen_args_t;

void *(*DYNINST_do_dlopen)(dlopen_args_t *) = NULL;

char gLoadLibraryErrorString[ERROR_STRING_LENGTH];

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
      return (dyntid_t) -1;
   }
   me = (*DYNINST_pthread_self)();
   return (dyntid_t) me;
}


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
#define POS_ENTRIES 3
static pthread_offset_t positions[POS_ENTRIES] = { { 72, 476, 516, 576 },
                                                   { 72, 76, 516, 84 },
                                                   { 72, 476, 516, 80 } };
#else
//x86_64 and ia64 share structrues
#define POS_ENTRIES 1
static pthread_offset_t positions[POS_ENTRIES] = { { 144, 952, 1008, 160 } };
#endif

int DYNINSTthreadInfo(BPatch_newThreadEventRecord *ev)
{
  static int err_printed = 0;
  int i;
  char *buffer;

  ev->stack_addr = 0x0;
  ev->start_pc = 0x0;
  buffer = (char *) ev->tid;
  
  for (i = 0; i < POS_ENTRIES; i++)
  {
     if ((READ_FROM_BUF(positions[i].pid_pos, pid_t) != ev->ppid) ||
         (READ_FROM_BUF(positions[i].lwp_pos, int) != ev->lwp))
        continue;
     ev->stack_addr = READ_FROM_BUF(positions[i].stck_start_pos, void *);
     ev->start_pc = READ_FROM_BUF(positions[i].start_func_pos, void *);
     return 1;
  }
  
  if (!err_printed)
  {
    //If you get this error, then Dyninst is having trouble figuring out
    //how to read the information from the positions structure above.
    //It needs a new entry filled in.  Running the commented out program
    //that follows this function can help you collect the necessary data.
    fprintf(stderr, "[%s:%d] Unable to parse the pthread_t structure for this"
	    " version of libpthread.  Making a best guess effort.\n", 
	    __FILE__, __LINE__);
    err_printed = 1;
  }
   
  return 1;
}


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
