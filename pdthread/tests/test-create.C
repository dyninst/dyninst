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
#include <sys/types.h>

#if !defined(i386_unknown_nt4_0)
#include <sys/time.h>
#else
#include <windows.h>
#endif /* !defined(i386_unknown_nt4_0) */

#include "thread.h"

#define MILLION 1000000.0
#define NCREATES 16
unsigned ncreates = NCREATES;

#if !defined(i386_unknown_nt4_0)
struct timeval	tstart;
struct timeval	tend;
#else
LARGE_INTEGER	tstart;
LARGE_INTEGER	tend;
#endif !defined(i386_unknown_nt4_0)

double clat, usecs;

static void* foo(void* arg) {
    int i = 0;
    for(i = 0; i < 5; i++)
        fprintf(stderr, "thread %d created with arg %p\n", thr_self(), arg);
    return 0;
}

int main(void) {
    unsigned i;
    thread_t t[16];

//    (void) thr_self();
    fprintf(stderr, "%u thread creation timings\n", ncreates);

#if !defined(i386_unknown_nt4_0)
	gettimeofday( &tstart, NULL );
#else
	QueryPerformanceCounter( &tstart );
#endif /* !defined(i386_unknown_nt4_0) */

    for (i = 0; i < ncreates; i++) {
        thr_create(0, 0, &foo, 0, THR_SUSPENDED, &(t[i]));
    }

    for (i = 0; i < ncreates; i++)
        thr_join(t[i], NULL, NULL) != THR_OKAY;
    
    
#if !defined(i386_unknown_nt4_0)
	gettimeofday( &tend, NULL );
#else
	QueryPerformanceCounter( &tend );
#endif /* !defined(i386_unknown_nt4_0) */


	/* report our results */
#if !defined(i386_unknown_nt4_0)
	usecs = (tend.tv_sec * MILLION + tend.tv_usec) -
			(tstart.tv_sec * MILLION + tstart.tv_usec);
#else
	{
		LARGE_INTEGER freq;
		QueryPerformanceFrequency( &freq );

		usecs = MILLION * (((double)tend.QuadPart) - tstart.QuadPart)/freq.QuadPart;
	}
#endif /* !defined(i386_unknown_nt4_0) */
	clat = usecs / ncreates;
	fprintf(stderr, "create latency: %g us\n", clat);

    return 0;
}
