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
 * RTthread-solaris.c: platform dependent runtime instrumentation functions for threads
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

#include <pthread.h>
#include "RTthread.h"

unsigned DYNINST_threadContext();

void DYNINST_ThreadPInfo(void* tls, void** stkbase, int* tid, 
			 long *pc, int* lwp, void** rs) {
  unsigned pthread_context;
  int *func_ptr;
  struct __pthrdsinfo *ptr = (struct __pthrdsinfo *) tls ;
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

int DYNINST_ThreadInfo(void** stkbase, int* tidp, 
		       long *startpc, int* lwpidp, 
		       void** rs_p)
{
  struct __pthrdsinfo pthread_desc;
  int pthread_desc_size = sizeof(struct __pthrdsinfo);
  int registers[1024];
  int regsize = 1024*sizeof(int);
  pthread_t pthread_id;
  pthread_id = pthread_self();
  
  if (pthread_getthrds_np(&pthread_id, PTHRDSINFO_QUERY_ALL,
			  &pthread_desc, pthread_desc_size,
			  registers, &regsize)) {
    perror("RTthread-aix:DYNINST_ThreadInfo");
    return 0;
  }

  DYNINST_ThreadPInfo((void *)&pthread_desc,stkbase,tidp,startpc,lwpidp,rs_p);
  /*
  fprintf(stderr, "DYNINST_ThreadInfo, stkbase=%x, tid=%d, startpc=%x, lwp=%d, resumestate=%x\n",
	  (unsigned) *stkbase,
	  *tidp, *startpc, *lwpidp, (unsigned)*rs_p);
	  */
  return 1 ;
}
