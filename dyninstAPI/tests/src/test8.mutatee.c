
/* Test application (Mutatee) */

/* $Id: */

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "mutatee_util.h"

#define do_dyninst_breakpoint() stop_process_()

#if defined(i386_unknown_nt4_0) && !defined(__GNUC__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define getpid _getpid
#else
#include <unistd.h>
#include <dlfcn.h>
#endif

#ifdef __cplusplus
int mutateeCplusplus = 1;
#else
int mutateeCplusplus = 0;
#endif

#ifndef COMPILER
#define COMPILER ""
#endif
const char *Builder_id=COMPILER; /* defined on compile line */

/* control debug printf statements */
#define dprintf if (debugPrint) printf
int debugPrint = 0;

#define TRUE    1
#define FALSE   0

int runAllTests = TRUE;
#define MAX_TEST 2
int runTest[MAX_TEST+1];

int globalVariable1_1 = 0;
int globalVariable2_1 = 0;
volatile int globalVariable2_2 = 0;

void func1_3()
{
    globalVariable1_1++;
    do_dyninst_breakpoint();
}

void func1_2()
{
    globalVariable1_1++;
    func1_3();
}

void func1_1()
{
    globalVariable1_1++;
    func1_2();
}

#if !defined(i386_unknown_nt4_0)
void func2_4()
{
    globalVariable2_1++;
    do_dyninst_breakpoint();
}

void sigalrm_handler(int signum)
{
    globalVariable2_1++;
    globalVariable2_2 = 1;
    func2_4();
}

void func2_3()
{
    globalVariable2_1++;

    /* Cause a SIGALRM */
    alarm(1);
    while (globalVariable2_2 == 0) ;
}

void func2_2()
{
    globalVariable2_1++;
    func2_3();
}
#endif /* !i386_unknown_nt4_0 */

void func2_1()
{
#if defined(i386_unknown_linux2_0) || defined(sparc_sun_solaris2_4) || defined( ia64_unknown_linux2_4 )
    void (*old_handler)(int) = signal(SIGALRM, sigalrm_handler);

    globalVariable2_1++;
    func2_2();

    signal(SIGALRM, old_handler);
#endif
}

int main(int argc, char *argv[])
{                                       
    int i, j;

    for (j=0; j <= MAX_TEST; j++)
	runTest[j] = TRUE;

    for (i=1; i < argc; i++) {
        if (!strcmp(argv[i], "-verbose")) {
            debugPrint = TRUE;
        } else if (!strcmp(argv[i], "-runall")) {
	    runAllTests = TRUE;
            for (j=0; j <= MAX_TEST; j++) runTest[j] = TRUE;
        } else if (!strcmp(argv[i], "-run")) {
            runAllTests = FALSE;
            for (j=0; j <= MAX_TEST; j++) runTest[j] = FALSE;
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        runTest[testId] = TRUE;
                    } else {
                        printf("invalid test %d requested\n", testId);
                        exit(-1);
                    }
                } else {
                    /* end of test list */
                    break;
                }
            }
            i=j-1;
        } else {
            fprintf(stderr, "Usage: %s [-verbose] -run <num> ..\n", argv[0]);
            exit(-1);
        }
    }

    if ((argc==1) || debugPrint)
        printf("Mutatee %s [%s]:\"%s\"\n", argv[0], 
                mutateeCplusplus ? "C++" : "C", Builder_id);
    if (argc==1) exit(0);

    if (runTest[1]) func1_1();
    if (runTest[2]) func2_1();

	
    fprintf( stderr, "Mutatee %s terminating.\n", argv[0] );
    return 0;
}
