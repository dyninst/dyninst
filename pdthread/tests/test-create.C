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
