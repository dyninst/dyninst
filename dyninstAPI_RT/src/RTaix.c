/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

/************************************************************************
 * RTaix.c: mutatee-side library function specific to AIX
************************************************************************/

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "RTthread.h"
#include <dlfcn.h> /* dlopen constants */
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

/************************************************************************
 * void DYNINSTos_init(void)
 *
 * os initialization function
************************************************************************/

void DYNINSTstaticHeap_1048576_textHeap_libSpace(void);

void DYNINSTbreakPoint()
{
    /* We set a global flag here so that we can tell
       if we're ever in a call to this when we get a 
       SIGBUS */
   /*
   int thread_index = DYNINSTthreadIndex();
   */
    DYNINST_break_point_event = 1;
    while (DYNINST_break_point_event)  {
        kill(getpid(), DYNINST_BREAKPOINT_SIGNUM);
    }
    /* Mutator resets to 0... */
}


void DYNINSTsafeBreakPoint()
{
    DYNINST_break_point_event = 2; /* Not the same as above */

    // We cannot send SIGSTOPs to ourselves - it triggers kernel
    // bugs on exit -> see bug 1081

    // We can now use SIGTRAPs in this situation because we
    // can now trust that the child will be traced on exit from
    // fork
    kill(getpid(), SIGTRAP);
}

void DYNINSTos_init(int calledByFork, int calledByAttach)
{
   /* Text heap initialization. Call this to force the library to
      be included by the linker */
#ifdef USES_LIB_TEXT_HEAP
   /* Dummy call to get the library space actually included
      (not pruned by an optimizing linker) */
   DYNINSTstaticHeap_1048576_textHeap_libSpace();
#endif
}

#define NOT_SETUP_ERR 0x2468ace0

int DYNINSTloadLibrary(char *libname)
{
   void *res;
   char *err_str;
   gLoadLibraryErrorString[0]='\0';
   
   if (NULL == (res = dlopen(libname, RTLD_NOW | RTLD_GLOBAL))) {
      /* An error has occurred */
      //perror( "DYNINSTloadLibrary -- dlopen" );
    
      if (NULL != (err_str = dlerror()))
         strncpy(gLoadLibraryErrorString, err_str, ERROR_STRING_LENGTH);
      else 
         sprintf(gLoadLibraryErrorString,"unknown error with dlopen");
      
      /*fprintf(stderr, "%s[%d]: %s\n", __FILE__, __LINE__,
                gLoadLibraryErrorString);*/
      return 0;  
   } else
      return 1;
}

void DYNINST_ThreadPInfo(void* tls, void** stkbase, dyntid_t* tid, 
                         long *pc, int* lwp, void** rs) 
{
   /* Use the __pthrdsinfo struct defined in /usr/include/pthread.h */
   struct __pthrdsinfo *ptr = (struct __pthrdsinfo *) tls;
   /* Want: 
      Stack base
      TID
      initial PC (entry to provided "start" function)
      LWP
      RS (thread context)
   */
   *stkbase = (void *) ptr->__pi_stackaddr;
   *tid = ptr->__pi_ptid; /* pthread_t as opposed to tid_t */
   *pc = ptr->__pi_func;
   *lwp = ptr->__pi_tid;
   *rs = (void *) (&ptr->__pi_context);
}

int DYNINSTthreadInfo(BPatch_newThreadEventRecord *ev)
{
   struct __pthrdsinfo pthread_desc;
   int pthread_desc_size = sizeof(struct __pthrdsinfo);
   int registers[1024];
   int regsize = 1024*sizeof(int);

   dyntid_t tidp;
   int lwpid;
   long startpc;
   void *stkbase, *rs_p;

   int result;

   pthread_t pthread_id;
   pthread_id = (pthread_t) dyn_pthread_self();

   result = dyn_pthread_getthrds_np(&pthread_id, PTHRDSINFO_QUERY_ALL,
                                    &pthread_desc, pthread_desc_size,
                                    registers, &regsize);
   if (result)
   {
      if (result != NOT_SETUP_ERR)
         perror("RTthread-aix:DYNINST_ThreadInfo");
      else
         fprintf(stderr, "[%s:%u] - TInfo not yet setup\n", __FILE__, __LINE__);
      return 0;
   }

   DYNINST_ThreadPInfo((void *)&pthread_desc, &stkbase, &tidp, &startpc, 
                       &lwpid, &rs_p);

   ev->stack_addr = stkbase;
   ev->start_pc = (void *) startpc;

   /*
   fprintf(stderr, "DYNINST_ThreadInfo, stkbase=%x, tid=%d, " 
           "startpc=%x, lwp=%d, resumestate=%x\n",
           (unsigned) *stkbase, *tidp, *startpc, *lwpid, (unsigned)*rs_p);
   */
	  
   return 1;
}

int dyn_pid_self()
{
   return getpid();
}

int dyn_lwp_self()
{
   return thread_self();
}

typedef struct {
   void *func;
   unsigned toc;
   void *dummy;
} call_record_t;
typedef dyntid_t (*DYNINST_pself_t)(void);
typedef int (*DYNINST_pgetthrds_np_t)(pthread_t *, int, struct __pthrdsinfo *, 
                                      int, void *, int *);

call_record_t DYNINST_pthread_self_record;
DYNINST_pself_t DYNINST_pthread_self = 
    (DYNINST_pself_t) &DYNINST_pthread_self_record;
dyntid_t dyn_pthread_self()
{
   if (!DYNINST_pthread_self_record.func)
   {
      return (void *) DYNINST_SINGLETHREADED;
   }
   return (*DYNINST_pthread_self)();
}

call_record_t DYNINST_pthread_getthrds_np_record;
DYNINST_pgetthrds_np_t DYNINST_pthread_getthrds_np =
   (DYNINST_pgetthrds_np_t) &DYNINST_pthread_getthrds_np_record;
int dyn_pthread_getthrds_np(pthread_t *thread, int mode, 
                            struct __pthrdsinfo *buf, int bufsize,
                            void *regbuf, int *regbufsize)
{
   if (!DYNINST_pthread_getthrds_np_record.func)
   {
      return NOT_SETUP_ERR;
   }
   return (*DYNINST_pthread_getthrds_np)(thread, mode, buf, bufsize, 
                                         regbuf, regbufsize);
}

/* 
   We reserve index 0 for the initial thread. This value varies by
   platform but is always constant for that platform. Wrap that
   platform-ness here. 
*/
int DYNINST_am_initial_thread(dyntid_t tid) {
    return (tid == (dyntid_t) 1);
}
