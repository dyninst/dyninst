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

#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h>

#ifdef DETACH_ON_THE_FLY
#include <dlfcn.h>
#endif

#include "mutatee_util.h"

#ifdef i386_unknown_nt4_0
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

/*
 * Stop the process (in order to wait for the mutator to finish what it's
 * doing and restart us).
 */
#if defined(alpha_dec_osf4_0) && defined(__GNUC__)
static long long int  beginFP;
#endif

#ifdef DETACH_ON_THE_FLY
/*
 All this to stop ourselves.  We may be detached, but the mutator
 needs to notice the stop.  We must send a SIGILL to ourselves, not
 SIGSTOP, to get the mutator to notice.

 DYNINSTsigill is a runtime library function that does this.  Here we
 obtain a pointer to DYNINSTsigill from the runtime loader and then
 call it.  Note that this depends upon the mutatee having
 DYNINSTAPI_RT_LIB defined (with the same value as mutator) in its
 environment, so this technique does not work for ordinary mutatees.

 We could call kill to send ourselves SIGILL, but this is unsupported
 because it complicates the SIGILL signal handler.  */
static void
dotf_stop_process()
{
     void *h;
     char *rtlib;
     static void (*DYNINSTsigill)() = NULL;

     if (!DYNINSTsigill) {
	  /* Obtain the name of the runtime library linked with this process */
	  rtlib = getenv("DYNINSTAPI_RT_LIB");
	  if (!rtlib) {
	       fprintf(stderr, "ERROR: Mutatee can't find the runtime library pathname\n");
	       assert(0);
	  }

	  /* Obtain a handle for the runtime library */
	  h = dlopen(rtlib, RTLD_LAZY); /* It should already be loaded */
	  if (!h) {
	       fprintf(stderr, "ERROR: Mutatee can't find its runtime library: %s\n",
		       dlerror());
	       assert(0);
	  }

	  /* Obtain a pointer to the function DYNINSTsigill in the runtime library */
	  DYNINSTsigill = (void(*)()) dlsym(h, "DYNINSTsigill");
	  if (!DYNINSTsigill) {
	       fprintf(stderr, "ERROR: Mutatee can't find DYNINSTsigill in the runtime library: %s\n",
		       dlerror());
	       assert(0);
	  }
     }
     DYNINSTsigill();
}
#endif /* DETACH_ON_THE_FLY */

void stop_process_()
{
#ifdef i386_unknown_nt4_0
    DebugBreak();
#else

#if defined(alpha_dec_osf4_0) && defined(__GNUC__)
    /* This GCC-specific hack doesn't compile with other compilers, 
       which is unfortunate, since the native build test fails part 15
       with the error "process did not signal mutator via stop". */
    /* It also doesn't appear to be necessary when compiling with gcc-2.8.1,
       which makes its existance even more curious. */
    register long int fp asm("15");

    beginFP = fp;
#endif

#ifdef DETACH_ON_THE_FLY
    dotf_stop_process();
    return;
#endif

#if !defined(bug_irix_broken_sigstop)
    kill(getpid(), SIGSTOP);
#else
    kill(getpid(), SIGEMT);
#endif

#if defined(alpha_dec_osf4_0) && defined(__GNUC__)
    fp = beginFP;
#endif

#endif
}
