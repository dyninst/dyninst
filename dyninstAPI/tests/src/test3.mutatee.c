
/* Test application (Mutatee) */

/* $Id: test3.mutatee.c,v 1.12 2000/08/07 00:34:45 wylie Exp $ */

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#if defined(i386_unknown_nt4_0) && !defined(__GNUC__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

#if defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0)
#include <dlfcn.h> /* For replaceFunction test */
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

/* Mutatee for multi-process tests.
 */

/* control debug printf statements */
#define dprintf	if (debugPrint) printf
int debugPrint = 0;

#define TRUE    1
#define FALSE   0

#define MAX_TEST 4

/* 
 * Test #1 - just run in a busy wait loop and then exit.
 */
void test1()
{
     int i;

     for (i=0; i < 2000000; i++);
     dprintf("Mutatee exiting.\n");
     exit(0);
}

int test2ret = (int)0xdeadbeef;

volatile int dummy = 1;

void call2_1(int arg1, int arg2)
{
     dprintf("call2_1() called with arg1=%d,arg2=%d\n", arg1, arg2);
}

int func2_1()
{
     return (dummy * 2);
}

/*
 * Test #2 - call a function which should be instrumented to set the 
 *     global variable test2ret to a value (by the mutator).
 */
void test2()
{
     FILE *fp;
     char filename[80];

     func2_1();

     sprintf(filename, "test3.out.%d", (int)getpid());
     fp = fopen(filename, "w");
     assert(fp);
     fprintf(fp, "%d\n", test2ret);
     fclose(fp);
}

/* 
 * Test #4 - terminate with an abort
 */
void test4()
{
     dprintf("Mutatee aborting.\n");
     abort();
}

int main(int iargc, char *argv[])
{                                       /* despite different conventions */
    unsigned argc=(unsigned)iargc;      /* make argc consistently unsigned */
    unsigned int i;
    unsigned int testNum=0;

    for (i=1; i < argc; i++) {
        if (!strcmp(argv[i], "-verbose")) {
            debugPrint = TRUE;
        } else if (!strcmp(argv[i], "-run")) {
            if ((testNum = atoi(argv[i+1]))) {
                if ((testNum <= 0) || (testNum > MAX_TEST)) {
                    printf("invalid test %d requested\n", testNum);
                    exit(-1);
                }
            } else {
                /* end of test list */
                break;
            }
            i++;
        } else {
            fprintf(stderr, "Usage: %s [-verbose] -run <num>\n", argv[0]);
            exit(-1);
        }
    }

    if ((argc==1) || debugPrint)
        printf("Mutatee %s [%s]:\"%s\"\n", argv[0], 
                mutateeCplusplus ? "C++" : "C", Builder_id);
    if (argc==1) exit(0);

    switch (testNum) {
	case 1:
		test1();
		break;
	
	case 2:
		test2();
		break;

        /* subtest 3 uses test1()! */

	case 4:
		test4();
		break;

	default:
		printf("invalid test number %d in mutatee\n", testNum);
		break;
    }
    dprintf("Mutatee %s terminating.\n", argv[0]);
    return(0);
}
