
/* Test application (Mutatee) */

/* $Id: test2.mutatee.c,v 1.12 1999/06/17 14:58:41 wylie Exp $ */

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

#if defined(sparc_sun_solaris2_4)  || defined(i386_unknown_solaris2_5) || \
    defined(i386_unknown_linux2_0) || defined(mips_sgi_irix6_4) || \
    defined(alpha_dec_osf4_0)
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

void doFork();

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

void func12_1()
{
    /* Does nothing. */
}

#ifdef i386_unknown_nt4_0
#define USAGE "Usage: test2.mutatee [-attach] [-verbose]"
#else
#define USAGE "Usage: test2.mutatee [-attach <fd>] [-verbose]"
#endif

int main(int argc, char *argv[])
{
    int i;
#if !defined(i386_unknown_nt4_0)
    void *ref;
    int pfd;
#endif
    int useAttach = FALSE;
 
    for (i=1; i < argc; i++) {
        if (!strcmp(argv[i], "-verbose")) {
            debugPrint = 1;
        } else if (!strcmp(argv[i], "-attach")) {
            useAttach = TRUE;
#ifndef i386_unknown_nt4_0
	    if (++i >= argc) {
		fprintf(stderr, "%s\n", USAGE);
		exit(-1);
	    }
	    pfd = atoi(argv[i]);
#endif
        } else if (!strcmp(argv[i], "-fork")) {
	    doFork();
        } else {
            fprintf(stderr, "%s\n", USAGE);
            exit(-1);
        }
    }

    /* see if we should wait for the attach */
    if (useAttach) {
#ifndef i386_unknown_nt4_0
	char ch = 'T';
	if (write(pfd, &ch, sizeof(char)) != sizeof(char)) {
	    fprintf(stderr, "*ERROR*: Writing to pipe\n");
	    exit(-1);
	}
	close(pfd);
#endif
	printf("Waiting for mutator to attach...\n");
	while (!checkIfAttached()) ;
	printf("Mutator attached.  Mutatee continuing.\n");
    }

#if defined(sparc_sun_solaris2_4)  || defined(i386_unknown_solaris2_5) || \
    defined(i386_unknown_linux2_0) || defined(mips_sgi_irix6_4) || \
    defined(alpha_dec_osf4_0)
    /* now use the dlopen interface to force an object to load. */
#if defined(alpha_dec_osf4_0)
    ref = dlopen(TEST_DYNAMIC_LIB, RTLD_NOW);
#else
    ref = dlopen(TEST_DYNAMIC_LIB, RTLD_NOW | RTLD_GLOBAL);
#endif
    if (!ref) {
	fprintf(stderr, "%s\n", dlerror());
	fflush(stderr);
    }

    /* Stop and wait for the mutator to check that we linked the library */
    stop_process();

#endif

    func10_1();

    while(1);

    return(0);
}


void doFork() { 
    // XXX To be completed...
    while(1);
}
