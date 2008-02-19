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

/* Test application (Mutatee) */

/* $Id: test1.mutateeCommon.c,v 1.7 2008/02/19 13:39:01 rchen Exp $ */

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "test1.mutateeCommon.h"

#include "mutatee_util.h"

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
 || defined(ia64_unknown_linux2_4)
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
 || defined(arch_x86_64) \
 || defined(rs6000_ibm_aix64)
int pointerSize = sizeof(void *);
#endif

/*
 * Check to see if the mutator has attached to us.
 */
int checkIfAttached()
{
    return isAttached;
}

/* Check to see whether we're running any tests after testno
 * (in range (testno, maxtest]) */
int runAnyAfter(int testno, int maxtest) {
  int retval = 0;
  int i;
  for (i = testno + 1; i <= maxtest; i++) {
    if (runTest[i]) {
      retval = 1;
      break;
    }
  }
  return retval;
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
	    logerror("**Failed** test %d (%s)\n", testNum, testName);
	logerror("  %s = %d, not %d\n", name, a, value);
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
	if (passedTest[tst]) {
	  logerror("**Failed** test #%d (%s)\n", tst, tn);
	}
	logerror("  %s[%d] = %d, not %d\n", 
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
    char *logfilename = NULL;

    /* fprintf(stderr, "Starting test1.mutatee\n"); */

    for (j=0; j <= MAX_TEST; j++) {
	runTest [j] = FALSE;
    }

    for (i=1; i < argc; i++) {
        if (!strcmp(argv[i], "-verbose")) {
            debugPrint = TRUE;
        } else if (!strcmp(argv[i], "-log")) {
	  /* A file was specified that this program should send output to */
	  i += 1;
	  if (i >= argc) {
	    /* We reached the end of the parameters; no filename */
	    fprintf(stderr, "Missing log file name\n");
	    exit(-1);
	  }
	  logfilename = argv[i];
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
                        fprintf(stderr, "invalid test %d requested\n", testId);
                        exit(-1);
                    }
                } else {
                    /* end of test list */
		    break;
                }
            }
            i=j-1;
	} else if (!strcmp(argv[i], "-fast")) {
	  fastAndLoose = 1;
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
        fprintf(stderr, "Mutatee %s [%s]:\"%s\"\n", argv[0],
                mutateeCplusplus ? "C++" : "C", Builder_id);
    if (argc==1) exit(0);

    if ((logfilename != NULL) && (strcmp(logfilename, "-") != 0)) {
      outlog = fopen(logfilename, "a");
      if (NULL == outlog) {
	fprintf(stderr, "Error opening log file %s\n", logfilename);
	exit(-1);
      }
      errlog = outlog;
    } else {
      errlog = stderr;
      outlog = stdout;
    }

    if (useAttach) {
#ifndef i386_unknown_nt4_0
	char ch = 'T';
	if (write(pfd, &ch, sizeof(char)) != sizeof(char)) {
	    fprintf(stderr, "*ERROR*: Writing to pipe\n");
	    exit(-1);
	}
	close(pfd);
#endif
    	while (!checkIfAttached());
    }

    /* actually invoke the tests.  This function is implemented in two
     *   different locations (one for C and one for Fortran). Look in
     *     test1.mutatee.c and test1.mutateeFortC.c for the code.
     */
    runTests();

    /* See how we did running the tests. */
    for (i=1; i <= MAX_TEST; i++) {
        if (runTest[i] && !passedTest[i]) {
	    logerror("failure on %d\n", i);
	    testsFailed++;
	}
    }

    if (!testsFailed) {
        logstatus("All tests passed\n");
    } else {
	logstatus("**Failed** %d test%c\n",testsFailed,(testsFailed>1)?'s':' ');
    }

    dprintf("Mutatee %s terminating.\n", argv[0]);
    if ((outlog != NULL) && (outlog != stdout)) {
      fclose(outlog);
    }
    return (testsFailed ? 127 : 0);
}
