/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/* Test application (Mutatee) */

/* $Id: test1.mutateeCommon.c,v 1.12 2008/02/19 13:38:43 rchen Exp $ */

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "test1.mutateeCommon.h"


extern int mutateeCplusplus;

#ifdef i386_unknown_nt4_0
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "test1.h"
#ifdef __cplusplus
#include "cpp_test.h"
#include <iostream>
#endif

#ifndef COMPILER
#define COMPILER ""
#endif
const char Builder_id[]=COMPILER; /* defined on compile line */

#if defined(sparc_sun_solaris2_4) \
 || defined(alpha_dec_osf4_0) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_solaris2_5) \
 || defined(ia64_unknown_linux2_4) \
 || defined(ppc64_linux)
#include <dlfcn.h> /* For replaceFunction test */
#endif

/* XXX Currently, there's a bug in the library that prevents a subroutine call
 * instrumentation point from being recognized if it is the first instruction
 * in a function.  The following variable is used in this program in a number
 * of kludges to get around this.
 */
int kludge;

#ifdef __cplusplus
extern "C" void runTests();
#else
extern void runTests();
#endif


int debugPrint;
int runTest[MAX_TEST+1];
int passedTest[MAX_TEST+1];

int isAttached = 0;

#if defined(mips_sgi_irix6_4) \
 || defined(x86_64_unknown_linux2_4) \
 || defined(rs6000_ibm_aix5_1) \
 || defined(ppc64_linux)
int pointerSize = sizeof(void *);
#endif

/*
 * Check to see if the mutator has attached to us.
 */
int checkIfAttached()
{
    return isAttached;
}

/*
 * Verify that a scalar value of a variable is what is expected
 *
 */
void verifyScalarValue(const char *name, int a, int value, int testNum, 
                       const char *testName)
{
    if (a != value) {
	if (passedTest[testNum])
	    printf("**Failed** test %d (%s)\n", testNum, testName);
	printf("  %s = %d, not %d\n", name, a, value);
	passedTest[testNum] = FALSE;
    }
}

/*
 * Verify that a passed array has the correct value in the passed element.
 *
 */
void verifyValue(const char *name, int *a, int index, int value, int tst,
                 const char *tn)
{
    if (a[index] != value) {
	if (passedTest[tst]) printf("**Failed** test #%d (%s)\n", tst, tn);
	printf("  %s[%d] = %d, not %d\n", 
	    name, index, a[index], value);
	passedTest[tst] = FALSE;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#ifdef i386_unknown_nt4_0
#define USAGE "Usage: test1.mutatee [-attach] [-verbose] -run <num> .."
#else
#define USAGE "Usage: test1.mutatee [-attach <fd>] [-verbose] -run <num> .."
#endif

int main(int iargc, char *argv[])
{                                       /* despite different conventions */
    unsigned argc=(unsigned)iargc;      /* make argc consistently unsigned */
    unsigned int i, j;
    unsigned int testsFailed = 0;
    int useAttach = FALSE;
    unsigned int e;
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
                fprintf(stderr, "Offending invocation:\n\t");
                for (e = 0; e < argc; e++) {
                  fprintf(stderr, "%s ", argv[e]);
                }
                fprintf(stderr, "\n");
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
             fprintf(stderr, "Offending invocation:\n\t");
             for (e = 0; e < argc; e++) {
               fprintf(stderr, "%s ", argv[e]);
             }
             fprintf(stderr, "\n");
            exit(-1);
        }
    }

    if ((argc==1) || debugPrint)
        printf("Mutatee %s [%s]:\"%s\"\n", argv[0],
                mutateeCplusplus ? "C++" : "C", Builder_id);
    if (argc==1) exit(0);

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

    /* actually invoke the tests.  This function is implemented in two
     *   different locations (one for C and one for Fortran). Look in
     *     test1.mutatee.c and test1.mutateeFortC.c for the code.
     */
    runTests();

    /* See how we did running the tests. */
    for (i=1; i <= MAX_TEST; i++) {
        if (runTest[i] && !passedTest[i]) {
	    printf("failure on %d\n", i);
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
    return (testsFailed ? 127 : 0);
}
