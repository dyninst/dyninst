/*
 * Copyright (c) 1996 Barton P. Miller
 *
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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
#include "util/h/sys.h"

#include <thread.h>
#include <sys/lwp.h>
#include <stdlib.h>
#include "RTthread.h"


/* The following must be cleanned up */
typedef struct {
        long    sp;
        long    pc;
        long    dontcare1;
        long    dontcare2;
        long    g2;
        long    g3;
        long    g4;
} resumestruct_t;

typedef struct uthread {
        struct uthread   *dontcare1;  
        char            *thread_stack;       
        unsigned int    thread_stacksize;     
        char            *dontcare2;         
        resumestruct_t   t_resumestate; 
        long            start_pc;      
        thread_t        thread_id;     
        lwpid_t         lwp_id;        
        int             opts;     

} sparc_thread_t ;

extern void* DYNINST_allthreads_p ;
void idtot(int tid) {
/*
  if ( DYNINST_allthreads_p ) {
    sparc_thread_t *ct = (sparc_thread_t*) DYNINST_idtot(tid, DYNINST_allthreads_p);
    fprintf(stderr, "stk=0x%x, startpc=0x%x, lwpid=0x%x, resumestate=0x%x",
	    ct->thread_stack, ct->start_pc, ct->lwp_id, &(ct->t_resumestate));
  }
*/
}

void DYNINST_ThreadPInfo(
    void* tls,
    void** stackbase, 
    int* tidp, 
    long *startpc, 
    int* lwpidp,
    void** resumestate_p) {
  sparc_thread_t *ptr = (sparc_thread_t *) tls ;
  *stackbase = (void*) (ptr->thread_stack);
  *tidp = (int) ptr->thread_id ;
  *startpc = ptr->start_pc ;
  *lwpidp = ptr->lwp_id ;
  *resumestate_p = &(ptr->t_resumestate);
/* fprintf(stderr, "------ tid=%d, stk=0x%x, stksize=0x%x\n",  ptr->thread_id, ptr->thread_stack, ptr->thread_stacksize); */
}

int DYNINST_ThreadInfo(void** stackbase, 
int* tidp, 
long *startpc, 
int* lwpidp,
void** resumestate_p) {
  extern sparc_thread_t *DYNINST_curthread(void) ;
  sparc_thread_t *curthread ;
  if ( (curthread = DYNINST_curthread()) ) {
    DYNINST_ThreadPInfo((void*)curthread,stackbase,tidp,startpc,lwpidp,resumestate_p);
    return 1 ;
  }
  return 0;
}
