/*
 * Copyright (c) 1996-2000 Barton P. Miller
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
 * $Id: RTlinux.c,v 1.20 2003/02/21 20:06:08 bernat Exp $
 * RTlinux.c: mutatee-side library function specific to Linux
 ************************************************************************/

#include <signal.h>
#include <assert.h>
#include <stdio.h>
#include <dlfcn.h>
#include <link.h>
#include <errno.h>

#include <sys/ptrace.h>

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"

#ifndef ia64_unknown_linux2_4
extern struct sigaction DYNINSTactTrap;
extern struct sigaction DYNINSTactTrapApp;
#else
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>

extern double DYNINSTstaticHeap_4M_anyHeap_1[4*1024*1024/sizeof(double)];

void _start( void ) {
	fprintf( stderr, "*** Initializing dyninstAPI runtime.\n" );

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
	fprintf( stderr, "*** Marked memory from 0x%lx to 0x%lx executable.\n", alignedHeapPointer, alignedHeapPointer + adjustedSize );
	}

void R_BRK_TARGET() { // We may (have) want(ed) to make this an array, but I think it's better to make sure it's executable, for now.
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
    unsigned pc = uap.eip;
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
      uap.eip = nextpc;
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

char gLoadLibraryErrorString[ERROR_STRING_LENGTH];
int DYNINSTloadLibrary(char *libname)
{
  void *res;
  char *err_str;
  gLoadLibraryErrorString[0]='\0';
  
  if (NULL == (res = dlopen(libname, RTLD_NOW | RTLD_GLOBAL))) {
    // An error has occurred
    perror( "DYNINSTloadLibrary -- dlopen" );
    
    if (NULL != (err_str = dlerror()))
      strncpy(gLoadLibraryErrorString, err_str, ERROR_STRING_LENGTH);
    else 
      sprintf(gLoadLibraryErrorString,"unknown error with dlopen");
    
    //fprintf(stderr, "%s[%d]: %s\n",__FILE__,__LINE__,gLoadLibraryErrorString);
    return 0;  
  } else
    return 1;
  
  /*
   * All of this is necessary because on linux, dlopen is not in libc, but
   * in a separate library libdl.  Not all programs are linked with libdl,
   * but libc does contain the underlying functions.  This is gross and
   * may break with new versions of glibc.  It is based on glibc 2.0.6
   */
  /*
    struct link_map *new;
    char *errstr;
    int err;
    
    void doit (void) {
    new = _dl_open( libname ?: "", RTLD_NOW | RTLD_GLOBAL );
    }
    
    err = _dl_catch_error( &errstr, doit );
    
    if( errstr == NULL )
    return 1;
    else {
    fprintf( stderr, errstr );
    free( errstr );
    return 0;
    }
  */
}
