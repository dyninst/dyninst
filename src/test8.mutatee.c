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

/* $Id: */

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "mutatee_util.h"

#define do_dyninst_breakpoint() stop_process_()

#if defined(os_windows) && !defined(__GNUC__)
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
#define MAX_TEST 3
int runTest[MAX_TEST+1];

int passedTest[MAX_TEST + 1];

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
    fprintf(stderr, "test8.mutatee: stopping in func2_4()\n");
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
#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(sparc_sun_solaris2_4) \
 || defined(ia64_unknown_linux2_4)
    void (*old_handler)(int) = signal(SIGALRM, sigalrm_handler);

    globalVariable2_1++;
    func2_2();

    signal(SIGALRM, old_handler);
#endif
}

void func3_4() {
	;
	}

void func3_3() {
	do_dyninst_breakpoint();
	} /* end func3_3() */

void func3_2() {
	/* This function will be instrumented to call func3_3, which
	   stops the mutatee and allows the mutator to walk the stack. */
	   
	/* This is to give us a third place to instrument. */
	func3_4();
	} /* end func3_2() */
	
void func3_1() {
	/* Stop myself.  The mutator will instrument func3_2() at this point. */
	do_dyninst_breakpoint();
	
	/* This function will be instrumented. */
	func3_2();
	} /* end func3_1() */

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

int main(int argc, char *argv[])
{                                       
    int i, j;
    char *logfilename = NULL;

    for (j=0; j <= MAX_TEST; j++)
	runTest[j] = TRUE;

    for (i=1; i < argc; i++) {
        if (!strcmp(argv[i], "-verbose")) {
            debugPrint = TRUE;
	} else if (!strcmp(argv[i], "-log")) {
	  if ((i + 1) >= argc) {
	    fprintf(stderr, "Missing log file name\n");
	    fprintf(stderr,
		    "Usage: %s [-verbose] [-log <file>] -run <num> ..\n",
		    argv[0]);
	    exit(-1);
	  }
	  i += 1;
	  logfilename = argv[i];
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
            fprintf(stderr,
		    "Usage: %s [-verbose] [-log <file>] -run <num> ..\n",
		    argv[0]);
            exit(-1);
        }
    }

    if ((logfilename != NULL) && (strcmp(logfilename, "-") != 0)) {
      outlog = fopen(logfilename, "a");
      if (NULL == outlog) {
	fprintf(stderr, "Error opening log file %s\n", logfilename);
	exit(-1);
      }
      errlog = outlog;
    } else {
      outlog = stdout;
      errlog = stderr;
    }

    if ((argc==1) || debugPrint)
        logstatus("Mutatee %s [%s]:\"%s\"\n", argv[0], 
		  mutateeCplusplus ? "C++" : "C", Builder_id);
    if (argc==1) exit(0);

    memset(passedTest, 0, sizeof(passedTest));

    if (runTest[1]) {
      func1_1();
      if (fastAndLoose && runAnyAfter(1, 3)) {
	fprintf(stderr, "test8.mutatee: stopping\n");
	stop_process_();
	fprintf(stderr, "test8.mutatee: starting again\n");
      }
    }
    if (runTest[2]) {
      func2_1();
      if (fastAndLoose && runAnyAfter(2, 3)) {
	fprintf(stderr, "test8.mutatee: stopping\n");
	stop_process_();
	fprintf(stderr, "test8.mutatee: starting again\n");
      }
    }
    if (runTest[3]) func3_1();
	
    dprintf("Mutatee %s terminating.\n", argv[0] );
    if ((outlog != NULL) && (outlog != stdout)) {
      fclose(outlog);
    }
    return 0;
}
