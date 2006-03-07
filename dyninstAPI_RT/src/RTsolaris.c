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
 * $Id: RTsolaris.c,v 1.24 2006/03/07 23:18:31 bernat Exp $
 * RTsolaris.c: mutatee-side library function specific to Solaris
 ************************************************************************/

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"

#include <signal.h>
#include <sys/ucontext.h>
#include <assert.h>
#include <stdio.h>
#include <dlfcn.h>

#include <sys/procfs.h> /* /proc PIOCUSAGE */
#include <fcntl.h> /* O_RDONLY */
#include <unistd.h> /* getpid() */


#ifdef i386_unknown_solaris2_5
void DYNINSTtrapHandler(int sig, siginfo_t *info, ucontext_t *uap);

extern struct sigaction DYNINSTactTrap;
extern struct sigaction DYNINSTactTrapApp;
#endif

/************************************************************************
 * void DYNINSTos_init(void)
 *
 * OS initialization function
************************************************************************/

extern void DYNINSTheap_setbounds();  /* RTheap-solaris.c */

void
DYNINSTos_init(int calledByFork, int calledByAttach)
{
    RTprintf("DYNINSTos_init(%d,%d)\n", calledByFork, calledByAttach);
    DYNINSTheap_setbounds();
    /* uncomment this if you want instrumentation written out in core files */
    /* setmemwrite(); */
}

int DYNINSTloadLibrary(char *libname)
{
  void *res;
  char *err_str;
  gLoadLibraryErrorString[0]='\0';
  gBRKptr = sbrk(0);
  
  if (NULL == (res = dlopen(libname, RTLD_NOW | RTLD_GLOBAL))) {
    /* An error has occurred */
    perror( "DYNINSTloadLibrary -- dlopen" );
    
    if (NULL != (err_str = dlerror()))
      strncpy(gLoadLibraryErrorString, err_str, ERROR_STRING_LENGTH);
    else 
      sprintf(gLoadLibraryErrorString,"unknown error with dlopen");
    
    fprintf(stderr, "%s[%d]: %s\n",__FILE__,__LINE__,gLoadLibraryErrorString);
    return 0;  
  } else
    return 1;
}


/*
We can get Solaris to put instrumented code in the core file of dumped
mutatees by setting setting WRITE protection on all pages in the
process (SHARED text pages cannot have WRITE protect set).

To use, compile and link this code with the runtime library, and call
setmemwrite from DYNINSTinit.
*/

/* Set every page in this process to be writable to
   cause pages with instrumented code to be saved in core dumps. */
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/procfs.h>
#define maxpmap 512
static
int setmemwrite()
{
    int pfd, numpmap, i;
    prmap_t pmap[maxpmap];

    char buf[32];
    sprintf(buf, "/proc/%05d", getpid());
    pfd = open(buf, O_RDONLY);
    if (0 > pfd) {
	 perror("open (in setmemwrite)");
	 fprintf(stderr, "Can't open /proc on myself\n");
	 exit(1);
    }
    if (0 > ioctl(pfd, PIOCNMAP, &numpmap)) {
	 perror("PIOCNMAP (in setmemwrite)");
	 exit(1);
    }
    if (numpmap + 1 > maxpmap) {
	 fprintf(stderr, "Too many memory mappings\n");
	 exit(1);
    }
    if (0 > ioctl(pfd, PIOCMAP, pmap)) {
	 perror("PIOCMAP (in setmemwrite)");
	 exit(1);
    }
    for (i = 0; i < numpmap; i++) {
	 prmap_t *p = &pmap[i];
	 /* Enable WRITE if this region does not have it already and
	    we won't get in trouble for setting it (i.e., it is not
	    SHARED). */
	 if (~p->pr_mflags & MA_WRITE
	     && ~p->pr_mflags & MA_SHARED)
	      if (0 > mprotect(p->pr_vaddr, p->pr_size,
			       PROT_WRITE
			       | PROT_READ
			       | (p->pr_mflags & MA_EXEC ? PROT_EXEC : 0))) {
		   perror("mprotect (in setmemwrite)");
		   fprintf(stderr, "mprotect (it %d) args: %#010x, %x, %x\n",
			   i,
			   p->pr_vaddr, p->pr_size, 
			   PROT_WRITE
			   | PROT_READ
			   | (p->pr_mflags & MA_EXEC ? PROT_EXEC : 0));
		   exit(1);
	      }
    }
    close(pfd);
    return 0;
}


/*********************************************************
 *** MULTITHREAD
 *********************************************************/

#include <thread.h>
#include <sys/lwp.h>


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

int which(void *tls) {
  static int w = 0;
  int i;
  if (w) return w;
  dyntid_t tid = dyn_pthread_self();
  if (tid == (dyntid_t) -1)
      return 0;

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

void DYNINST_ThreadPInfo(void* tls, void** stkbase, long *pc)
{
    switch (which(tls)) {
    case LIBTHR_SOL29: {
        thread_sol29 *ptr = (thread_sol29 *) tls;
        *stkbase = (void*) (ptr->thread_stack);
        *pc = (long) ptr->start_pc ;
        break;
    }
        
    case LIBTHR_SOL28PATCH: {
        thread_sol28patch *ptr = (thread_sol28patch *) tls ;
        *stkbase = (void*) (ptr->thread_stack);
        *pc = (long) ptr->start_pc ;
        break;
    }  
    case LIBTHR_SOL28: {
        thread_sol28 *ptr = (thread_sol28 *) tls ;
        *stkbase = (void*) (ptr->thread_stack);
        *pc = (long) ptr->start_pc ;
        break;
    }
    case LIBTHR_SOL27: {
        thread_sol27 *ptr = (thread_sol27 *) tls ;
        *stkbase = (void*) (ptr->thread_stack);
        *pc = (long) ptr->start_pc ;
        break;
    }
    case LIBTHR_SOL26: {
        thread_sol26 *ptr = (thread_sol26 *) tls ;
        *stkbase = (void*) (ptr->thread_stack);
        *pc = (long) ptr->start_pc ;
        break;
    }
    case LIBTHR_SOL29_PINKY: {
        thread_sol29_pinky *ptr = (thread_sol29_pinky *) tls;
        *stkbase = (void*) (ptr->stackbottom);
        *pc = (long) ptr->start_pc;
        break;
    }
    default:
        fprintf(stderr, "Unknown thread type: %d\n", which(tls));
        assert(0);
    }
}

/* Defined in RTthread-sparc-asm.s, access %g7 */
void *DYNINST_curthread(void) ;

int DYNINSTthreadInfo(BPatch_newThreadEventRecord *ev) {
    dyntid_t tidp;
    int lwpid;
    long startpc;
    void *stkbase, *rs_p;
    void *curthread = DYNINST_curthread();
    if ( curthread != NULL ) {
        DYNINST_ThreadPInfo(curthread, &stkbase, &startpc);
        ev->stack_addr = stkbase;
        ev->start_pc = (void *)startpc;
        return 1;
    }
    return 0;
}

int dyn_pid_self()
{
   return getpid();
}

int dyn_lwp_self()
{
   return _lwp_self();
}

