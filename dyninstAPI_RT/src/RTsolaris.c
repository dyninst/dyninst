/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * $Id: RTsolaris.c,v 1.32 2006/12/14 20:39:18 legendre Exp $
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
#include <string.h>
#include <stdlib.h>


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
    while (DYNINST_break_point_event)
        kill(getpid(), SIGSTOP);
}

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
  
  if (NULL == (res = dlopen(libname, RTLD_NOW | RTLD_GLOBAL))) {
    /* An error has occurred */
    //perror( "DYNINSTloadLibrary -- dlopen" );
    
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

int dyn_pid_self()
{
   return getpid();
}

int dyn_lwp_self()
{
   return _lwp_self();
}

/* New approach - from Linux. Store offsets rather than
   structures. Mo' easia. */

typedef struct pthread_offset_t
{
  unsigned thr_pos;
  unsigned lwp_pos;
  unsigned start_func_pos;
  unsigned stck_start_pos;
} pthread_offset_t;


/* Increase when we get more types... like Solaris 2.10 */
#define POS_ENTRIES 2

static pthread_offset_t positions[POS_ENTRIES] = {
  { 18, 19, 17, 2 }, /* shemesh.cs.wisc.edu, Sol 2.8 */
  { 16, 17, 50, 11} /* adams.cs.umd.edu, Sol 2.9. thr_pos and lwp_pos
                        may be reversed, as the values were identical
                        in tests. */
};

void DYNINST_ThreadPInfo(void* tls, void** stkbase, long *pc)
{
  static int w = -1;
  dyntid_t tid;
  int lwp;
  int i;
  int *tls_int = (int *)tls;

  *stkbase = 0;
  *pc = 0;

  /*if (w) return w;*/
  tid = dyn_pthread_self();
  if (tid == (dyntid_t) -1)
      return;
  lwp = dyn_lwp_self();

  /* Find the right type... */
  if (w == -1) {
    for (i = 0; i < POS_ENTRIES; i++) {
      if ((tls_int[positions[i].thr_pos] == (int) tid) &&
          (tls_int[positions[i].lwp_pos] == lwp)) {
        w = i;

        break;
      }
    }
  }
  if (w == -1) {
    /* Couldn't find... */
    return;
  }

  *stkbase = (void *) tls_int[positions[w].stck_start_pos];
  *pc = tls_int[positions[w].start_func_pos];

  return;
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

/* 
   We reserve index 0 for the initial thread. This value varies by
   platform but is always constant for that platform. Wrap that
   platform-ness here. 
*/
int DYNINST_am_initial_thread(dyntid_t tid) {
    return (tid == (dyntid_t) 1);
}

