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
 * RTlinux.c: mutatee-side library function specific to Linux
************************************************************************/

#include <signal.h>
#include <assert.h>
#include <stdio.h>
#include <dlfcn.h>
#include <link.h>

#include <sys/ptrace.h>

#include "dyninstAPI_RT/h/rtinst.h"

/************************************************************************
 * void DYNINSTos_init(void)
 *
 * os initialization function
************************************************************************/

void DYNINSTtrapHandler(int sig, struct sigcontext uap );

void
DYNINSTos_init(int calledByFork, int calledByAttach)
{
    /*
       Install trap handler.
       This is currently being used only on the x86 platform.
    */
    
    struct sigaction act;
    act.sa_handler = (void(*)(int))DYNINSTtrapHandler;
    act.sa_flags = 0;
    sigfillset(&act.sa_mask);
    if (sigaction(SIGTRAP, &act, 0) != 0) {
        perror("sigaction(SIGTRAP)");
	assert(0);
	abort();
    }
    
    ptrace( PTRACE_TRACEME, 0, 0, 0 );
}





/****************************************************************************
   The trap handler. Currently being used only on x86 platform.

   Traps are used when we can't insert a jump at a point. The trap
   handler looks up the address of the base tramp for the point that
   uses the trap, and set the pc to this base tramp.
   The paradynd is responsible for updating the tramp table when it
   inserts instrumentation.
*****************************************************************************/

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

void DYNINSTtrapHandler(int sig, struct sigcontext uap ) {
    unsigned pc = uap.eip;
    unsigned nextpc = lookup(pc);

    if (!nextpc) {
      /* kludge: maybe the PC was not automatically adjusted after the trap */
      /* this happens for a forked process */
      pc--;
      nextpc = lookup(pc);
    }

    if (nextpc) {
		/* WARNING -- Remove before using in real use, it could KILL anything
		   that instruments libc */
		/*fprintf( stderr, "DYNINST trap %#.8x -> %#.8x\n", pc, nextpc );*/
      uap.eip = nextpc;
    } else {
      assert(0);
      abort();
    }
    DYNINSTtotalTraps++;
}

int DYNINSTloadLibrary(char *libname)
{
    void *res;

    res = dlopen( libname, RTLD_NOW | RTLD_GLOBAL );

    if( res == NULL ) {
	perror( "DYNINSTloadLibrary -- dlopen" );
	return 0;  // An error has occurred
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
