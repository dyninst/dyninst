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
 * $Id: RTlinux.c,v 1.29 2005/03/16 22:59:49 bernat Exp $
 * RTlinux.c: mutatee-side library function specific to Linux
 ************************************************************************/

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#if !defined (EXPORT_SPINLOCKS_AS_HEADER)
/* everything should be under this flag except for the assembly code
   that handles the runtime spinlocks  -- this is imported into the
   test suite for direct testing */

#include <signal.h>
#include <assert.h>
#include <stdio.h>
#include <dlfcn.h>
#include <link.h>
#include <errno.h>
#include <unistd.h>

#include <sys/ptrace.h>


#ifndef ia64_unknown_linux2_4
extern struct sigaction DYNINSTactTrap;
extern struct sigaction DYNINSTactTrapApp;
#else
#include <errno.h>
#include <sys/mman.h>

extern double DYNINSTstaticHeap_32K_lowmemHeap_1[];
extern double DYNINSTstaticHeap_4M_anyHeap_1[];

void _start( void ) {
	/* fprintf( stderr, "*** Initializing dyninstAPI runtime.\n" ); */

	/* Grab the page size, to align the heap pointer. */
	long int pageSize = sysconf( _SC_PAGESIZE );
	if( pageSize == 0 || pageSize == - 1 ) {
		fprintf( stderr, "*** Failed to obtain page size, guessing 16K.\n" );
		perror( "_start" );
		pageSize = 1024 * 1024 * 16;
		} /* end pageSize initialization */

	/* Align the heap pointer. */
	unsigned long int alignedHeapPointer = (unsigned long int)DYNINSTstaticHeap_4M_anyHeap_1;
	unsigned long long adjustedSize = alignedHeapPointer + 4 * 1024 * 1024;
	alignedHeapPointer = (alignedHeapPointer) & ~(pageSize - 1);
	adjustedSize -= alignedHeapPointer;

	/* Make the heap's page executable. */
	if( mprotect( (void *)alignedHeapPointer, adjustedSize, PROT_READ | PROT_WRITE | PROT_EXEC ) != 0 ) {
		fprintf( stderr, "*** Unable to mark DYNINSTstaticHeap_4M_anyHeap_1 executable!\n" );
		perror( "_start" );
		}
	/* fprintf( stderr, "*** Marked memory from 0x%lx to 0x%lx executable.\n", alignedHeapPointer, alignedHeapPointer + adjustedSize ); */

	/* Mark _both_ heaps executable. */
	alignedHeapPointer = (unsigned long int)DYNINSTstaticHeap_32K_lowmemHeap_1;
	adjustedSize = alignedHeapPointer + 32 * 1024;
	alignedHeapPointer = (alignedHeapPointer) & ~(pageSize - 1);
	adjustedSize -= alignedHeapPointer;

	/* Make the heap's page executable. */
	if( mprotect( (void *)alignedHeapPointer, adjustedSize, PROT_READ | PROT_WRITE | PROT_EXEC ) != 0 ) {
		fprintf( stderr, "*** Unable to mark DYNINSTstaticHeap_4M_anyHeap_1 executable!\n" );
		perror( "_start" );
		}
	/* fprintf( stderr, "*** Marked memory from 0x%lx to 0x%lx executable.\n", alignedHeapPointer, alignedHeapPointer + adjustedSize ); */
	}

/* Ensure we an executable block of memory. */
void R_BRK_TARGET() {
        /* Make sure we've got room for two bundles. */
        asm( "nop 0" ); asm( "nop 0" ); asm( "nop 0" );
        asm( "nop 0" ); asm( "nop 0" ); asm( "nop 0" );
        }
#endif

/************************************************************************
 * void DYNINSTos_init(void)
 *
 * OS initialization function
************************************************************************/

/* this handler's sigcontext arg is an undocumented feature of Linux    */
/* (requires the non-siginfo handler to be installed by sigaction.)     */
void DYNINSTtrapHandler(int sig, struct sigcontext uap);

void
DYNINSTos_init(int calledByFork, int calledByAttach)
{
    RTprintf("DYNINSTos_init(%d,%d)\n", calledByFork, calledByAttach);

    /*
       Install trap handler.  Currently being used only on x86 platforms.
    */
    
#ifndef ia64_unknown_linux2_4
    DYNINSTactTrap.sa_handler = (void(*)(int))DYNINSTtrapHandler;
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
#endif /* !ia64_unknown_linux2_4 */

    ptrace(PTRACE_TRACEME, 0, 0, 0);
}

/****************************************************************************
   The trap handler. Currently being used only on x86 platform.

   Traps are used when we can't insert a jump at a point. The trap
   handler looks up the address of the base tramp for the point that
   uses the trap, and set the pc to this base tramp.
   The paradynd is responsible for updating the tramp table when it
   inserts instrumentation.
*****************************************************************************/

#ifndef ia64_unknown_linux2_4

#ifndef IP_REG
#if defined(__x86_64__) && __WORDSIZE == 64
#define IP_REG rip	// 64-bit x86 ip
#else
#define IP_REG eip
#endif
#endif

trampTableEntry DYNINSTtrampTable[TRAMPTABLESZ];
unsigned DYNINSTtotalTraps = 0;

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
    assert(0);
    abort();
}

void DYNINSTtrapHandler(int sig, struct sigcontext uap) {
    unsigned pc = uap.IP_REG;
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
       be modified for that.

       I haven't seen the irpc trap bug with linux version.  This is
       probably because on linux we have the application's traps sent
       to the daemon and forwarded back to the application.  However,
       if trap signals are ever changed to be handled locally by the
       application, we'll be ready for it.  */

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
	   Check to make sure that this trap is the one corresponding to the 
	   one that needs to get regenerated.
	*/
	assert(pcAtLastIRPC == pc);
	trapNotHandled = 0;
	/* we'll then continue to process the trap */
      }
    }
    nextpc = lookup(--pc);

    if (!nextpc) {
      /* kludge: the PC may have been at or right after the trap */
      pc++;
      nextpc = lookup(pc);
    }

    if (nextpc) {
      RTprintf("DYNINST trap [%d] 0x%08X -> 0x%08X\n",
               DYNINSTtotalTraps, pc, nextpc);
      uap.IP_REG = nextpc;
    } else {
      if ((DYNINSTactTrapApp.sa_flags&SA_SIGINFO)) {
        if (DYNINSTactTrapApp.sa_sigaction != NULL) {
          siginfo_t info; /* dummy */
          void (*handler)(int,siginfo_t*,void*) =
                (void(*)(int,siginfo_t*,void*))DYNINSTactTrapApp.sa_sigaction;
          RTprintf("DYNINST trap [%d] 0x%08X DEFERED to A0x%08X!\n",
                DYNINSTtotalTraps, pc, DYNINSTactTrapApp.sa_sigaction);
          memset(&info,0,sizeof(info));
          sigprocmask(SIG_SETMASK, &DYNINSTactTrapApp.sa_mask, NULL);
          (*handler)(sig,&info,NULL);
          sigprocmask(SIG_SETMASK, &DYNINSTactTrap.sa_mask, NULL);
        } else {
          printf("DYNINST trap [%d] 0x%08X missing SA_SIGACTION!\n",
                  DYNINSTtotalTraps, pc);
          abort();
        }
      } else {
        if (DYNINSTactTrapApp.sa_handler != NULL) {
          void (*handler)(int,struct sigcontext) =
              (void(*)(int,struct sigcontext))DYNINSTactTrapApp.sa_handler;
          RTprintf("DYNINST trap [%d] 0x%08X DEFERED to H0x%08X!\n",
                DYNINSTtotalTraps, pc, DYNINSTactTrapApp.sa_handler);
          sigprocmask(SIG_SETMASK, &DYNINSTactTrapApp.sa_mask, NULL);
          (*handler)(sig,uap);
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
#endif /* !ia64_unknown_linux2_4 */

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
    strncpy(gLoadLibraryErrorString, err_str, ERROR_STRING_LENGTH);
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
  char *err_str;
  gLoadLibraryErrorString[0]='\0';
  if (res = dlopen(libname, RTLD_NOW | RTLD_GLOBAL)) {
    return 1;
  }
  else {
    get_dlopen_error();
#ifdef i386_unknown_linux2_0
    if (strstr(gLoadLibraryErrorString, "invalid caller") != NULL &&
	DYNINST_do_dlopen != NULL) {
      /* dlopen on recent glibcs has a "security check" so that
	 only registered modules can call it. Unfortunately, progs
	 that don't include libdl break this check, so that we
	 can only call _dl_open (the dlopen worker function) from
	 within glibc. We do this by calling do_dlopen
	 We fool this check by calling an addr written by the
	 mutator */
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
	return 1;
      else
	get_dlopen_error();
    }
#endif
  }
  return 0;
}


#endif /* EXPORT SPINLOCKS */

void DYNINSTlock_spinlock(dyninst_spinlock *mut)
{
#if defined (arch_x86) || (defined(arch_x86_64) && __WORDSIZE == 32)
  /*  same assembly as for x86 windows, just different format for asm stmt */
  /*  so if you change one, make the same changes in the other, please */

 asm (
         "  .Loop: \n"
         "  movl        8(%ebp), %ecx  # &mut in ecx \n"
         "  movl        $0, %eax       # 0 (unlocked) in eax\n"
         "  movl        $1, %edx       # 1 (locked) in edx \n"
         "  lock  \n"
         "  cmpxchgl    %edx, (%ecx)   # try to atomically store edx (1 = locked) \n"
         "                             # only if we are unlocked (ecx == eax) \n"
         "  jnz         .Loop          # if failure, zero flag set, spin again. \n"
     );

#elif defined(arch_x86_64) && __WORDSIZE == 64
  /*  same assembly as for x86 windows, just different format for asm stmt */
  /*  so if you change one, make the same changes in the other, please */

 asm (
         "  .Loop: \n"
         "  movq        8(%rbp), %rcx  # &mut in rcx \n"
         "  movq        $0, %rax       # 0 (unlocked) in rax\n"
         "  movq        $1, %rdx       # 1 (locked) in rdx \n"
         "  lock  \n"
         "  cmpxchgq    %rdx, (%rcx)   # try to atomically store rdx (1 = locked) \n"
         "                             # only if we are unlocked (rcx == rax) \n"
         "  jnz         .Loop          # if failure, zero flag set, spin again. \n"
     );

#elif defined (arch_ia64)

 asm (
         "  1:                                      \n"
         "  ld4                 r31=[%0]            \n" /* r31 <- lock value*/
         " ;;                                       \n"
         "  cmp.ne              p15,p0=r31,r0       \n" /* test lock set */
         "  (p15) br.cond.sptk  1b                  \n" /* yes:  spin */
         " ;;                                       \n" /* no: try to obtain lock */
         "  mov                 r31=1               \n" /* r31 <- 1, desired lock val */
         "  mov                 ar.ccv=0            \n" /* set value to compare to */
         "                                          \n" /* unlocked lock ( = 0) */
         " ;;                                       \n"
         "  cmpxchg4.acq        r31=[%0],r31,ar.ccv \n" /* if (lock == ar.ccv) lock = r31 */
         " ;;                                       \n"
         "  cmp.ne              p15,p0=r31,r0       \n" /* test r31 to see if xchg worked */
         "  (p15) br.cond.sptk  1b                  \n" /* if r31 != 0, lock failed, spin*/
         " ;;                                       \n"
         "  end:                                    \n" /* else, we got the lock, done */
 ::"r"(mut):"ar.ccv", "p15", "p0", "r31","memory");


#else
#error
#endif
}
