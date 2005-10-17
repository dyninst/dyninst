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
 * RTaix.c: mutatee-side library function specific to AIX
************************************************************************/

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "RTthread.h"
#include <dlfcn.h> /* dlopen constants */
#include <stdio.h>
#include <pthread.h>

/************************************************************************
 * void DYNINSTos_init(void)
 *
 * os initialization function
************************************************************************/

void DYNINSTstaticHeap_1048576_textHeap_libSpace(void);

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

char gLoadLibraryErrorString[ERROR_STRING_LENGTH];
int DYNINSTloadLibrary(char *libname)
{
   void *res;
   char *err_str;
   gLoadLibraryErrorString[0]='\0';
   
   if (NULL == (res = dlopen(libname, RTLD_NOW | RTLD_GLOBAL))) {
      /* An error has occurred */
      perror( "DYNINSTloadLibrary -- dlopen" );
    
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
   unsigned pthread_context;
   int *func_ptr;
   struct __pthrdsinfo *ptr = (struct __pthrdsinfo *) tls;
   *stkbase = (void*) (ptr->__pi_stackaddr);
   /**tid = (int) ptr->__pi_ptid;*/
   /**lwp = (int) ptr->__pi_tid;*/
   *rs = &(ptr->__pi_context);
   /* The PC is a little different. We grab the thread context
      from a partial pthread structure. That +200 gives us the 
      function pointer to the start function. That gives us the
      start */
   pthread_context = DYNINSTthreadContext();
   pthread_context+=92;
   func_ptr = (int *)*((int *)pthread_context);
   *pc = *func_ptr;
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
   pthread_id = dyn_pthread_self();

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
      return -1;
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

