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
 * $Id: RTsolaris.c,v 1.21 2005/09/09 18:05:21 legendre Exp $
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

int dyn_pthread_self()
{
     return 0;
}

void DYNINST_initialize_index_list()
{
}

int dyn_pid_self()
{
   return getpid();
}
