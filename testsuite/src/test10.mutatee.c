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

int dummy = 0;

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>


#define USAGE "Usage: test10.mutatee [-attach] [-verbose] -run <num> .."

#define MAX_TEST 4
#define TRUE 1
#define FALSE 0
int debugPrint = 0;
#define dprintf if (debugPrint) printf
int runTest[MAX_TEST+1];
int passedTest[MAX_TEST+1];
const char Builder_id[]=COMPILER; /* defined on compile line */


#if defined(sparc_sun_solaris2_4) 

extern void func1();
extern void func2();
extern void func3();
extern void func4();

//
// force jmp %reg; nop tail code function
//
asm("		.align 8");
asm("		.stabs  \"func1:F(0,1)\",36,0,16,func1");
asm("		.global func1");
asm("		.type   func1,#function");
asm("func1:");
asm("		sethi   %hi(call0),%g1");
asm("		or      %g1,%lo(call0),%g1");
asm("		jmp     %g1");
asm("		nop	");


//
// force call; move to %07 tail code function
//
asm("		.stabs  \"func2:F(0,1)\",36,0,16,func2");
asm("		.global func2");
asm("		.type   func2,#function");
asm("func2:");
asm("		sethi %hi(call0),%l0");
asm("		or %l0,%lo(call0),%l0");
asm("		mov %o7, %l0");
asm("		call call0");
asm("		mov %l0, %o7");


//
// use call; load %07 with current PC 
//
asm("		.stabs  \"func3:F(0,1)\",36,0,16,func3");
asm("		.global func3");
asm("		.type   func3,#function");
asm("func3:");
asm("		save %sp, -112, %sp");
asm("		nop");
asm("		nop");
asm("		call label3");		// external call to set pc into o7
asm("		nop");
asm("		call call0");
asm("		nop");
asm("		ret");
asm("		restore");

asm("		.stabs  \"func4:F(0,1)\",36,0,16,func4");
asm("		.global func4");
asm("		.type   func4,#function");
asm("func4:");
asm("		save %sp, -112, %sp");
asm("		nop");
asm("		call .+8");		// call to set pc into o7
asm("		restore");
asm("		nop");
asm("		ret");
asm("		restore");

// moved after func4 to make sure it appears to be "outside" func3
asm("label3:");
asm("		retl");
asm("		nop");
asm("		call abort");
asm("		nop");

void call0()
{
}

void call1()
{
    passedTest[1] = TRUE;
    printf("\nPassed test #1 \n");
}

void call2()
{
    passedTest[2] = TRUE;
    printf("\nPassed test #2 \n");
}

void call3()
{
    passedTest[3] = TRUE;
    printf("\nPassed test #3 \n");
}

void call4()
{
    passedTest[4] = TRUE;
    printf("\nPassed test #4 \n");
}

#else

void func1() 
{ 
    passedTest[1] = TRUE; 
}

void func2() 
{ 
    passedTest[2] = TRUE; 
}

void func3() 
{ 
    passedTest[3] = TRUE; 
}

void func4() 
{ 
    passedTest[4] = TRUE; 
}


void call0()
{
}

void call1()
{
    passedTest[1] = TRUE;
    printf("\Skipped test #1 N/A\n");
}

void call2()
{
    passedTest[2] = TRUE;
    printf("\nSkipped test #2 N/A\n");
}

void call3()
{
    passedTest[3] = TRUE;
    printf("\nSkipped test #3 N/A\n");
}

void call4()
{
    passedTest[4] = TRUE;
    printf("\nSkipped test #4 N/A\n");
}

#endif

void runTests()
{
    int j;

    for (j=0; j <= MAX_TEST; j++) {
        passedTest [j] = FALSE;
    }

    if (runTest[1]) {
	func1();
	if (!passedTest[1]) printf("\n **Failed** test #1\n");
    }

    if (runTest[2]) {
	func2();
	if (!passedTest[1]) printf("\n **Failed** test #2\n");
    }

    if (runTest[3]) {
	func3();
	if (!passedTest[1]) printf("\n **Failed** test #3\n");
    }

    if (runTest[4]) {
	func4();
	if (!passedTest[1]) printf("\n **Failed** test #4\n");
    }
}


int main(int iargc, char *argv[])
{                                       /* despite different conventions */
    unsigned argc=(unsigned)iargc;      /* make argc consistently unsigned */
    unsigned int i, j;
    unsigned int testsFailed = 0;
    int useAttach = FALSE;
#ifndef i386_unknown_nt4_0
    int pfd;
#endif

    for (j=0; j <= MAX_TEST; j++) {
	runTest [j] = FALSE;
    }

    for (i=1; i < argc; i++) {
        if (!strcmp(argv[i], "-verbose")) {
            debugPrint = TRUE;
        } else if (!strcmp(argv[i], "-attach")) {
            useAttach = TRUE;
#ifndef i386_unknown_nt4_0
	    if (++i >= argc) {
		fprintf(stderr, "%s\n", USAGE);
		exit(-1);
	    }
	    pfd = atoi(argv[i]);
#endif
        } else if (!strcmp(argv[i], "-runall")) {
            dprintf("selecting all tests\n");
            for (j=1; j <= MAX_TEST; j++) runTest[j] = TRUE;
        } else if (!strcmp(argv[i], "-run")) {
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if ((testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        dprintf("selecting test %d\n", testId);
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
            fprintf(stderr, "%s\n", USAGE);
            exit(-1);
        }
    }

    if ((argc==1) || debugPrint)
        printf("Mutatee %s [C]:\"%s\"\n", argv[0], Builder_id);
    if (argc==1) exit(0);

    /* actually invoke the tests.  This function is implemented in two
     *   different locations (one for C and one for Fortran). Look in
     *     test1.mutatee.c and test1.mutateeFortC.c for the code.
     */
    runTests();

    /* See how we did running the tests. */
    for (i=1; i <= MAX_TEST; i++) {
        if (runTest[i] && !passedTest[i]) {
	    testsFailed++;
	}
    }

    if (!testsFailed) {
	printf("All tests passed\n");
    } else {
	printf("**Failed** %d test%c\n",testsFailed,(testsFailed>1)?'s':' ');
    }
 

    fflush(stdout);
    dprintf("Mutatee %s terminating.\n", argv[0]);
    exit(testsFailed ? 127 :1); /* 1 is success! 127 is failure*/ 
}
