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
 * $Id: RTsolaris.c,v 1.18 2005/02/09 03:27:50 jaw Exp $
 * RTsolaris.c: mutatee-side library function specific to Solaris
 ************************************************************************/

#if !defined (EXPORT_SPINLOCKS_AS_HEADER)
/* everything should be under this flag except for the assembly code
   that handles the runtime spinlocks  -- this is imported into the
   test suite for direct testing */


#include <signal.h>
#include <sys/ucontext.h>
#include <assert.h>
#include <stdio.h>
#include <dlfcn.h>

#include <sys/procfs.h> /* /proc PIOCUSAGE */
#include <fcntl.h> /* O_RDONLY */
#include <unistd.h> /* getpid() */

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"

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
#ifdef i386_unknown_solaris2_5
    /*
       Install trap handler.
       This is currently being used only on the x86 platform.
    */
    DYNINSTactTrap.sa_handler = DYNINSTtrapHandler;
    DYNINSTactTrap.sa_flags = 0;
    sigfillset(&DYNINSTactTrap.sa_mask);
    if (sigaction(SIGTRAP, &DYNINSTactTrap, &DYNINSTactTrapApp) != 0) {
        perror("sigaction(SIGTRAP) install");
	assert(0);
	abort();
    }

    RTprintf("DYNINSTtrapHandler installed @ 0x%08X\n", DYNINSTactTrap.sa_handler);

    if (DYNINSTactTrapApp.sa_flags&SA_SIGINFO) {
        if (DYNINSTactTrapApp.sa_sigaction != NULL) {
            RTprintf("App's TRAP sigaction @ 0x%08X displaced!\n",
                   DYNINSTactTrapApp.sa_sigaction);
        }
    } else {
        if (DYNINSTactTrapApp.sa_handler != NULL) {
            RTprintf("App's TRAP handler @ 0x%08X displaced!\n",
                   DYNINSTactTrapApp.sa_handler);
        }
    }
#endif

    DYNINSTheap_setbounds();
    /* uncomment this if you want instrumentation written out in core files */
    /* setmemwrite(); */
}





/****************************************************************************
   The trap handler. Currently being used only on x86 platform.

   Traps are used when we can't insert a jump at a point. The trap
   handler looks up the address of the base tramp for the point that
   uses the trap, and set the pc to this base tramp.
   The paradynd is responsible for updating the tramp table when it
   inserts instrumentation.
*****************************************************************************/

#ifdef i386_unknown_solaris2_5
trampTableEntry DYNINSTtrampTable[TRAMPTABLESZ];
/*static unsigned*/ int DYNINSTtotalTraps = 0;
/* native CC type/name decorations aren't handled yet by dyninst */

static unsigned lookup(unsigned key) {
    unsigned u;
    unsigned k;
    for (u = HASH1(key); 1; u = (u + HASH2(key)) % TRAMPTABLESZ) {
      k = DYNINSTtrampTable[u].key;
      if (k == 0)
        return 0;
      else if (k == key)
        return DYNINSTtrampTable[u].val;
    }
    /* not reached */
}

void DYNINSTtrapHandler(int sig, siginfo_t *info, ucontext_t *uap) {
    unsigned pc = uap->uc_mcontext.gregs[PC];
    unsigned nextpc;

    /* If we're in the process of running an inferior RPC, we'll
       ignore the trap here and have the daemon rerun the trap
       instruction when the inferior rpc is done.  Because the default
       behavior is for the daemon to reset the PC to it's previous
       value and the PC is still at the trap instruction, we don't
       need to make any additional adjustments to the PC in the
       daemon.

       This is used only on x86 platforms, so if multithreading is
       ever extended to x86 platforms, then perhaps this would need to
       be modified for that.  */

    if(curRPC.runningInferiorRPC == 1) {
      /* If the current PC is somewhere in the RPC then it's a trap that
	 occurred just before the RPC and is just now getting delivered.
	 That is we want to ignore it here and regenerate it later. */
      if(curRPC.begRPCAddr <= pc && pc <= curRPC.endRPCAddr) {
      /* If a previous trap didn't get handled on this next irpc (assumes one 
	 trap per irpc) then we have a bug, a trap didn't get regenerated */
	/* printf("trapHandler, begRPCAddr: %x, pc: %x, endRPCAddr: %x\n",
	   curRPC.begRPCAddr, pc, curRPC.endRPCAddr);
	*/
	assert(trapNotHandled==0);
	trapNotHandled = 1; 
	return;
      }
      else  ;   /* a trap occurred as a result of a function call within the */ 
	        /* irpc, these traps we want to handle */
    }
    else { /* not in an irpc */
      if(trapNotHandled == 1) {
	/* Ok good, the trap got regenerated.
	   Check to make sure that this trap is the one corresponding to the one
	   that needs to get regenerated.
	*/
	assert(pcAtLastIRPC == pc);
	trapNotHandled = 0;
	/* we'll then continue to process the trap */
      }
    }
    nextpc = lookup(pc);

    if (!nextpc) {
      /* kludge: maybe the PC was not automatically adjusted after the trap */
      /* this happens for a forked process */
      pc--;
      nextpc = lookup(pc);
    }

    if (nextpc) {
      RTprintf("DYNINST trap [%d] 0x%08X -> 0x%08X\n",
               DYNINSTtotalTraps, pc, nextpc);
      uap->uc_mcontext.gregs[PC] = nextpc;
    } else {
      if ((DYNINSTactTrapApp.sa_flags&SA_SIGINFO)) {
          if (DYNINSTactTrapApp.sa_sigaction != NULL) {
              void (*handler)(int,siginfo_t*,ucontext_t*) =
                  (void(*)(int,siginfo_t*,ucontext_t*))DYNINSTactTrapApp.sa_sigaction;
              RTprintf("DYNINST trap [%d] 0x%08X DEFERED to 0x%08X!\n",
                       DYNINSTtotalTraps, pc, DYNINSTactTrapApp.sa_sigaction);
              sigprocmask(SIG_SETMASK, &DYNINSTactTrapApp.sa_mask, NULL);
              (*handler)(sig,info,uap);
              sigprocmask(SIG_SETMASK, &DYNINSTactTrap.sa_mask, NULL);
          } else {
              printf("DYNINST trap [%d] 0x%08X missing SA_SIGACTION!\n",
                    DYNINSTtotalTraps, pc);
              abort();
          }
      } else {
          if (DYNINSTactTrapApp.sa_handler != NULL) {
              void (*handler)(int,siginfo_t*,ucontext_t*) =
                  (void(*)(int,siginfo_t*,ucontext_t*))DYNINSTactTrapApp.sa_sigaction;
              RTprintf("DYNINST trap [%d] 0x%08X DEFERED to 0x%08X!\n",
                    DYNINSTtotalTraps, pc, DYNINSTactTrapApp.sa_sigaction);
              sigprocmask(SIG_SETMASK, &DYNINSTactTrapApp.sa_mask, NULL);
              (*handler)(sig,info,uap);
              sigprocmask(SIG_SETMASK, &DYNINSTactTrap.sa_mask, NULL);
          } else {
              printf("DYNINST trap [%d] 0x%08X missing SA_HANDLER!\n",
                    DYNINSTtotalTraps, pc);
              abort();
          }
      }
    }
    DYNINSTtotalTraps++;
}

#endif

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
int DYNINSTloadLibrary(char *libname)
{
    if (dlopen(libname, RTLD_NOW | RTLD_GLOBAL) != NULL)
	return 1;
    else
	return 0;
}
*/


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
#endif /* EXPORT SPINLOCKS */
void DYNINSTlock_spinlock(dyninst_spinlock *mut)
{

#if (os_solaris == 9)
  /*  This might only work for solaris 2.9 since it uses cas. */
  /*  The command -Av9 needs to be sent to the solaris assembler */
  /*  also works with -Av8plus, and analagous solaris cc command option */
#if defined __GNUC__
 asm (
     " 1:                       \n"
     " mov      1, %%g2         \n" /* l0 <- 1 */
     " mov      0, %%g0         \n" /* g0 <- 0 */
     " cas      [%0],%%g0,%%g2  \n" /* if (lock == g0) lock = %l0, atomic compare/swap */
     " brz,pn   %%g2,out        \n" /* if (l0 == 0) after the swap, it worked, goto done */

     " 2:                       \n" /* if not, spin until lock is not set anymore */
     " ld       [%0], %%g2      \n" /* l0 <- mut->lock */
     " brnz,pt  %%g2,2b         \n" /* if (lock != 0) spin*/
     " b,a      1b              \n" /* else try to obtain lock again (goto 1) */

     " out:                     \n" /* last step, memory barrier */
     " membar   #LoadLoad | #LoadStore | #StoreLoad | #StoreStore\n"
     : :  "r" (mut) :  "g0", "g2", "memory");
#else
 asm (
     " 1:                       \n"
     " mov      1, %l2          \n" /* l0 <- 1 */
     " mov      0, %l0          \n" /* g0 <- 0 */
     " cas      [%i0],%l0,%l2   \n" /* if (lock == g0) lock = %l0, atomic compare/swap */
     " cmp      %l2,0           \n" /* */
     " be       out             \n"  /* else, got lock, done*/

     " 2:                       \n" /* if not, spin until lock is not set anymore */
     " ld       [%i0], %l2      \n" /* l0 <- mut->lock */
     " cmp      %l2,0           \n" /* */
     " bne      2b              \n"  /* else, got lock, done*/
     " b,a      1b              \n" /* else try to obtain lock again (goto 1) */

     " out:                     \n" /* last step, memory barrier */
     " membar   #LoadLoad | #LoadStore | #StoreLoad | #StoreStore\n"
     );

#endif

#else /* solaris 8 */

#if defined __GNUC__

 asm (
     " 1:                       \n"
     " ldstub   [%0],%%g3       \n" /* get old lock value, store lock val of all ones */
     " cmp      %%g3,0          \n" /* if (old lock != 0), someone has lock, spin*/
     " be       out             \n"  /* else, got lock, done*/
     " nop                      \n"

     " 2:                       \n"  /* if not, spin until lock is not set anymore */
     " ld       [%0], %%g3      \n" /* l0 <- mut->lock */
     " cmp      %%g3,0          \n"  /* if (old lock != 0), someone has lock, spin*/

     " bne      2b              \n" /* if (lock != 0) spin*/
     " nop                      \n"
     " b,a      1b              \n"  /* else try to obtain lock again (goto 1) */
     " nop                      \n"

     " out:                     \n" /* last step, memory barrier */
     : : "r" (mut) : "g3", "memory");

#else /* sun cc */

 asm (
     " 1:                       \n"
     " ldstub   [%i0],%g3       \n" /* get old lock value, store lock val of all ones */
     " cmp      %g3,0           \n" /* if (old lock != 0), someone has lock, spin*/
     " be       out             \n"  /* else, got lock, done*/
     " nop                      \n"

     " 2:                       \n"  /* if not, spin until lock is not set anymore */
     " ld       [%i0], %g3      \n" /* l0 <- mut->lock */
     " cmp      %g3,0           \n"  /* if (old lock != 0), someone has lock, spin*/

     " bne      2b              \n" /* if (lock != 0) spin*/
     " nop                      \n"
     " b,a      1b              \n"  /* else try to obtain lock again (goto 1) */
     " nop                      \n"

     " out:                     \n" /* last step, memory barrier */
     );

#endif /* gnuc vs suncc switch */

#endif /* solaris 8-9 switch */

}

