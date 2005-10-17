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

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"

#include <thread.h>
#include <sys/lwp.h>
#include <stdlib.h>
#include "RTthread.h"

/* Thread structure for libthread sparc-solaris2.6 */
typedef struct {
        long    sp;
        long    pc;
        long    dontcare1;
        long    dontcare2;
        long    g2;
        long    g3;
        long    g4;
} rs_sol26;

typedef struct thread_sol26_s {
        struct thread_sol26_s   *dontcare1;  
        char                    *thread_stack;        
        unsigned int             thread_stacksize;     
        char                    *dontcare2;         
        rs_sol26                 t_resumestate; 
        long                     start_pc;      
        thread_t                 thread_id;     
        lwpid_t                  lwp_id;        
        int                      opts;     

} thread_sol26;

/* Thread structure for libthread sparc-solaris2.7 */
typedef struct {
        int     sp;
        int     pc;
        int     fsr;
        int     fpu_en;
        int     g2;
        int     g3;
        int     g4;
        uint8_t dontcare3; 
} rs_sol27;

typedef struct thread_sol27_s {
        struct thread_sol27_s   *dontcare1;   
        caddr_t         thread_stack;    
        size_t          thread_stacksize;
        size_t          dontcare2; 
        caddr_t         dontcare3;      
        rs_sol27        t_resumestate;  
        void            (*start_pc)(); 
        thread_t        thread_id;     
        lwpid_t         lwp_id;       
        int             opts;      
        int             flag;       
} thread_sol27;


/* Thread structure for libthread sparc-solaris2.8 */
typedef struct {
        int     sp;
        int     pc;
        int     fsr;
        int     fpu_en;
        int     g2;
        int     g3;
        int     g4;
        uint8_t dontcare5;
} rs_sol28;

/* stack structure for libthread sparc-solaris2.8 */
typedef struct {
        char   *sp;
        int     size;
        int     flags;
} stack_sol28;

typedef struct thread_sol28_s {
        struct thread_sol28_s   *dontcare1;   
        caddr_t         thread_stack;    
        size_t          thread_stacksize;
        size_t          dontcare2;
        stack_sol28     dontcare3;
        caddr_t         dontcare4;      
        rs_sol28        t_resumestate;
        void            (*start_pc)(); 
        thread_t        thread_id;     
        lwpid_t         lwp_id;       
        int             opts;      
        int             flag; 
} thread_sol28;

/* Thread structure for libthread sparc-solaris2.8 */
typedef struct {
        int     sp;
        int     pc;
} rs_sol28patch;

typedef struct thread_sol28patch_s {
   char            dontcare[20];
   caddr_t         thread_stack;    
   size_t          thread_stacksize;
   char            dontcare2[8];
   rs_sol28patch   t_resumestate;
   char            dontcare3[24];
   void            (*start_pc)();    
   thread_t        thread_id;     
   lwpid_t         lwp_id;
} thread_sol28patch;


typedef struct thread_sol29_s {
    void *t_resumestate; /* 0xff2775a8 */
    int dontcare1;   /* 0xff2775a8 */
    int dontcare2;   /* 0 */
    int dontcare3;   /* 0 */
    int dontcare4;   /* 0 */
    int stackbottom; /* 0xff100000 */
    int stacksize;   /* 0xfc000 */
    int dontcare5;   /* 0 */
    int thread_stack;/* 0xff1fc000 */
    int stacksize2;  /* 0xfc000 */
    thread_t lwp_id; /* 2 */
    lwpid_t thread_id;  /* 2 */
    int dontcare[42];
    void (*start_pc)();
} thread_sol29;

typedef struct thread_sol29_pinky_s {
    /* This is the structure on pinky as determined by fprintf */
    int zero[2];
    void *base_thr_structs; /* 2 */
    void *next_thr_struct;  /* 3 */
    int zero2[3]; 
    void *stackbottom;      /* 7 */
    int fc000;
    int zero3;
    void *t_resumestate;    /* 10 */
    int fc000_2;
    thread_t lwp_id;    /* 12 */
    lwpid_t thread_id;  /* 13 */
    int zero4[32];
    void (*start_pc)();
} thread_sol29_pinky;


/*
// A simple test to determine the right thread package
*/
#define LIBTHR_UNKNOWN 0
#define LIBTHR_SOL26   1
#define LIBTHR_SOL27   2
#define LIBTHR_SOL28   3
#define LIBTHR_SOL29   4
#define LIBTHR_SOL28PATCH   5
#define LIBTHR_SOL29_PINKY 6

int which(void *tls) {
  static int w = 0;
  int i;
  dyntid_t tid = P_thread_self();
  if (w) return w;

  if ( ((thread_sol29_pinky*)tls)->thread_id == tid) {
      w = LIBTHR_SOL29_PINKY;
  }
  if ( ((thread_sol29*)tls)->thread_id == tid) {
      w = LIBTHR_SOL29;
  }  
  if ( ((thread_sol28patch*)tls)->thread_id == tid) {
      w = LIBTHR_SOL28PATCH;
  }  
  if ( ((thread_sol28*)tls)->thread_id == tid) {
    w = LIBTHR_SOL28;
  }
  if ( ((thread_sol27*)tls)->thread_id == tid) {
    w = LIBTHR_SOL27;
  }
  if ( ((thread_sol26*)tls)->thread_id == tid) {
    w = LIBTHR_SOL26;
  }
  return w;
}

extern void* DYNINST_allthreads_p ;
void idtot(dyntid_t tid) {
/*
  if ( DYNINST_allthreads_p ) {
    sparc_thread_t *ct = (sparc_thread_t*) DYNINST_idtot(tid, DYNINST_allthreads_p);
    fprintf(stderr, "stk=0x%x, startpc=0x%x, lwpid=0x%x, resumestate=0x%x",
	    ct->thread_stack, ct->start_pc, ct->lwp_id, &(ct->t_resumestate));
  }
*/
}

void DYNINST_ThreadPInfo(void* tls, void** stkbase, dyntid_t* tid, long *pc, int* lwp, void** rs)
{
    switch (which(tls)) {
  case LIBTHR_SOL29: {
      thread_sol29 *ptr = (thread_sol29 *) tls;
      *stkbase = (void*) (ptr->thread_stack);
      *tid = (int) ptr->thread_id ;
      *pc = (long) ptr->start_pc ;
      *lwp = (int) ptr->lwp_id ;
      *rs = &(ptr->t_resumestate);
      break;
  }

  case LIBTHR_SOL28PATCH: {
      thread_sol28patch *ptr = (thread_sol28patch *) tls ;
      *stkbase = (void*) (ptr->thread_stack);
      *tid = (int) ptr->thread_id ;
      *pc = (long) ptr->start_pc ;
      *lwp = (int) ptr->lwp_id ;
      *rs = &(ptr->t_resumestate);
      break;
  }  
  case LIBTHR_SOL28: {
      thread_sol28 *ptr = (thread_sol28 *) tls ;
      *stkbase = (void*) (ptr->thread_stack);
      *tid = (int) ptr->thread_id ;
      *pc = (long) ptr->start_pc ;
      *lwp = (int) ptr->lwp_id ;
      *rs = &(ptr->t_resumestate);
      break;
  }
  case LIBTHR_SOL27: {
      thread_sol27 *ptr = (thread_sol27 *) tls ;
      *stkbase = (void*) (ptr->thread_stack);
      *tid = (int) ptr->thread_id ;
      *pc = (long) ptr->start_pc ;
      *lwp = (int) ptr->lwp_id ;
      *rs = &(ptr->t_resumestate);
      break;
  }
  case LIBTHR_SOL26: {
      thread_sol26 *ptr = (thread_sol26 *) tls ;
      *stkbase = (void*) (ptr->thread_stack);
      *tid = (int) ptr->thread_id ;
      *pc = (long) ptr->start_pc ;
      *lwp = (int) ptr->lwp_id ;
      *rs = &(ptr->t_resumestate);
      break;
  }
  case LIBTHR_SOL29_PINKY: {
      thread_sol29_pinky *ptr = (thread_sol29_pinky *) tls;
      *stkbase = (void*) (ptr->stackbottom);
      *tid = (int) ptr->thread_id;
      *pc = (long) ptr->start_pc;
      *lwp = (int) ptr->lwp_id;
      *rs = &(ptr->t_resumestate);
      break;
  }
  default:
      fprintf(stderr, "Unknown thread type: %d\n", which(tls));
      assert(0);
    }
}

int DYNINST_ThreadInfo(void** stkbase, dyntid_t* tidp, long *startpc, int* lwpidp, void** rs_p) {
  extern void *DYNINST_curthread(void) ;
  void *curthread ;
  if ( (curthread = DYNINST_curthread()) ) {
    DYNINST_ThreadPInfo(curthread,stkbase,tidp,startpc,lwpidp,rs_p);
    return 1 ;
  }
  return 0;
}

int dyn_pthread_self()
{
   return 0;
}
