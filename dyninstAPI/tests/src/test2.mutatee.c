
/* Test application (Mutatee) */

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
#include <unistd.h>
#endif
#if defined(i386_unknown_nt4_0)
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_solaris2_5)
#include <dlfcn.h>
#endif

#include "test2.h"

/* XXX Currently, there's a bug in the library that prevents a subroutine call
 * instrumentation point from being recognized if it is the first instruction
 * in a function.  The following variable is used in this program in a number
 * of kludges to get around this.
 */
int kludge;

/* control debug printf statements */
#define dprintf	if (debugPrint) printf
int debugPrint = 0;

#define TRUE    1
#define FALSE   0

int isAttached = 0;

/*
 * Check to see if the mutator has attached to us.
 */
int checkIfAttached()
{
    return isAttached;
}


/*
 * Stop the process (in order to wait for the mutator to finish what it's
 * doing and restart us).
 */
void stop_process()
{
#ifdef i386_unknown_nt4_0
    DebugBreak();
#else
    kill(getpid(), SIGSTOP);
#endif
}

void func10_1()
{
    /* Does nothing.  Will be instrumented with a BPatch_breakPointExpr */
}

void func11_1()
{
    /* Does nothing. */
}

void main(int argc, char *argv[])
{
    int i;
    void *ref;
    int useAttach = FALSE;
 
    for (i=1; i < argc; i++) {
        if (!strcmp(argv[i], "-verbose")) {
            debugPrint = 1;
        } else if (!strcmp(argv[i], "-attach")) {
            useAttach = TRUE;
        } else {
            fprintf(stderr, "Usage: test2.mutatee [-attach] [-verbose]\n");
            exit(-1);
        }
    }

    /* see if we should wait for the attach */
    if (useAttach) {
	printf("Waiting for mutator to attach...\n");
	while (!checkIfAttached()) ;
	printf("Mutator attached.  Mutatee continuing.\n");
    }

#ifdef sparc_sun_solaris2_4
    /* now use the dlopen interface to force an object to load. */
    ref = dlopen(TEST_DYNAMIC_LIB, RTLD_NOW);
    if (!ref) {
	fprintf(stderr, "%s\n", dlerror());
	fflush(stderr);
    }

    /* Stop and wait for the mutator to check that we linked the library */
    stop_process();
#endif

    func10_1();

    while(1);

    exit(0);
}
