
/* Test application (Mutatee) */

/* $Id: test4b.mutatee.c,v 1.5 2000/08/07 00:33:30 wylie Exp $ */

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>


#if defined(i386_unknown_nt4_0) && !defined(__GNUC__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define getpid _getpid
#else
#include <unistd.h>
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
#define MAX_TEST 5
int runTest[MAX_TEST+1];

int globalVariable3_1;

void func3_2() 
{
    dprintf("in func3_2\n");
    globalVariable3_1 = 3000002;
}

void func3_1()
{
    dprintf("in func3_1\n");
}

int globalVariable4_1;

void func4_4()
{
    dprintf("in test4b func4_4\n");

    globalVariable4_1 = 4000003;
}

void func4_2()
{
    dprintf("in test4b func4_2\n");

    /* a call to func4_4 gets hooked onto the exit of this */
}

void func4_1()
{
#ifndef i386_unknown_nt4_0
    dprintf("in test4b func4_1\n");

    func4_2();

    exit(getpid());
#endif
}

int main(int argc, char *argv[])
{                                       
    int i, j;

    for (i=1; i < argc; i++) {
        if (!strcmp(argv[i], "-verbose")) {
            debugPrint = TRUE;
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

    if (runTest[3]) func3_1();
    if (runTest[4]) func4_1();

    dprintf("Mutatee %s terminating.\n", argv[0]);
    return 0;
}
