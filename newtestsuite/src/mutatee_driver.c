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

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>

#if defined(i386_unknown_nt4_0)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
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

#ifdef __cplusplus
extern "C" {
#endif
#include "mutatee_util.h"
#ifdef __cplusplus
}
#endif

volatile int isAttached = 0;

#include "mutatee_call_info.h"
/* #include "mutatee_glue.h" */

#if defined(USE_GROUP_MUTATEE)
#include "group_driver.gen.h"
#else
#include "solo_driver.h"
#endif

#ifdef SOLO_MUTATEE
#include "solo_driver.h"
#endif

/* Global copies of argc and argv, for tests that require them */
int gargc;
char **gargv;

int debugPrint = 0;

#define USAGE "Usage: mutatee [-attach <fd>] [-verbose] -run <num> .."

void setRunTest(const char *testname) {
  unsigned int i;
  if (NULL == testname) { /* Sanity check */
    return;
  }

  /* fprintf(stderr, "[%s:%u] - mutatee contains %u tests\n", __FILE__, __LINE__, info->size); /*DEBUG*/
  for (i = 0; i < MAX_TEST; i++) {
    /* fprintf(stderr, "[%s:%u] - comparing '%s' to target '%s'\n", __FILE__, __LINE__, info->funcs[i].testname, testname); /*DEBUG*/
    if (!strcmp(testname, mutatee_funcs[i].testname)) {
      runTest[i] = TRUE;
      break;
    }
  }
  if (i >= MAX_TEST) {
    fprintf(stderr, "%s is not a valid test for this mutatee\n", testname);
  }
} /* setRunTest() */

/*
 * Check to see if the mutator has attached to us.
 */
int checkIfAttached()
{
  return isAttached;
}

/* Note which test we're running so we can detect crashes */
void updateResumeLog(const char *resumelogname, const char *testname) {
  FILE *resumelog;

  resumelog = fopen(resumelogname, "w");
  if (resumelog != NULL) {
    fprintf(resumelog, "%s\n", testname);
    fclose(resumelog);
  } else {
    fprintf(stderr, "Error opening mutatee resumelog: %s\n",
	    strerror(errno));
  }
}

void updateResumeLogCompleted(const char *resumelogname) {
  FILE *resumelog;
  resumelog = fopen(resumelogname, "a");
  if (resumelog != NULL) {
    fprintf(resumelog, "+\n");
    fclose(resumelog);
  } else {
    fprintf(stderr, "Error opening mutatee resumelog: %s\n",
	    strerror(errno));
  }
}

void setLabel(unsigned int label_ndx, char *label) {
  /* TODO Fix me so group mutatees work */
  if (MAX_TEST > 1) {
    fprintf(stderr, "Group mutatees not enabled yet\n");
    return;
  }

  mutatee_funcs[0].testlabel = label;
}

int main(int iargc, char *argv[])
{                                       /* despite different conventions */
    unsigned argc = (unsigned) iargc;   /* make argc consistently unsigned */
    unsigned int i, j;
#if !defined(i386_unknown_nt4_0)
    int pfd;
#endif
    int useAttach = FALSE;
    char *logfilename = NULL;
    int allTestsPassed = TRUE;
    int retval;
    char *mutatee_name = NULL;
    char *resumelog_name = "mutatee_resumelog";
    unsigned int label_count = 0;
    int print_labels = FALSE;

    gargc = argc;
    gargv = argv;

    /* Begin *DEBUG* */
/*     for (i = 0; i < argc; i++) { */
/*       fprintf(stderr, "[%s:%u] - argv[%u]: '%s'\n", __FILE__, __LINE__, i, argv[i]); */
/*     } */
    /* End *DEBUG* */

    /* Extract the name of the mutatee binary from argv[0] */
    /* Find the last '/' in argv[0]; we want everything after that */
    mutatee_name = strrchr(argv[0], '/');
    if (NULL == mutatee_name) {
      /* argv[0] contains just the filename for the executable */
      mutatee_name = argv[0];
      setExecutableName(argv[0]);
    } else {
      mutatee_name += 1; /* Skip past the '/' */
      setExecutableName(mutatee_name);
    }
    /* fprintf(stderr, "[%s:%u] - mutatee name: '%s'\n", __FILE__, __LINE__, mutatee_name); /*DEBUG*/

    /* Initialize the test structure for this mutatee */
/*     if (init_tests(&g_info)) { */
/*       fprintf(stderr, "[%s:%u] - Error initializing mutatee tests!\n", __FILE__, __LINE__); */
/*       return -1; */
/*     } */

    for (j=0; j < MAX_TEST; j++) {
      runTest[j] = FALSE;
    }

    /* Parse command line arguments */
    for (i=1; i < argc; i++) {
        if (!strcmp(argv[i], "-verbose")) {
            debugPrint = 1;
	} else if (!strcmp(argv[i], "-log")) {
	  /* fprintf(stderr, "[%s:%u] - parsing log parameter\n", __FILE__, __LINE__); /*DEBUG*/
	  /* Read the log file name so we can set it up later */
	  if ((i + 1) >= argc) {
	    /* fprintf(stderr, "[%s:%u] - No log file name?\n", __FILE__, __LINE__); /*DEBUG*/
	    fprintf(stderr, "Missing log file name\n");
	    exit(-1);
	  }
	  i += 1;
	  /* fprintf(stderr, "[%s:%u] - Setting log file name to '%s'\n", __FILE__, __LINE__, argv[i]); /*DEBUG*/
	  logfilename = argv[i];
        } else if (!strcmp(argv[i], "-attach")) {
            useAttach = TRUE;
#ifndef i386_unknown_nt4_0
	    if (++i >= argc) {
		fprintf(stderr, "attach usage\n");
		fprintf(stderr, "%s\n", USAGE);
		exit(-1);
	    }
	    pfd = atoi(argv[i]);
#endif
        } else if (!strcmp(argv[i], "-run")) {
	  char *tests;
	  char *name;

	  if (i + 1 >= argc) {
	    fprintf(stderr, "-run must be followed by a test name\n");
	    exit(-1);
	  }
	  i += 1;
	  tests = strdup(argv[i]);
	  /* FIXME I think strtok is frowned on */
	  name = strtok(tests, ",");
	  setRunTest(name); /* Enables the named test to run */
	  while (name != NULL) {
	    name = strtok(NULL, ",");
	    if (name != NULL) {
	      setRunTest(name);
	    }
	  }
	  free(tests);
	} else if (!strcmp(argv[i], "-label")) {
	  if (i + 1 >= argc) {
	    fprintf(stderr, "-label must be followed by a label string\n");
	    exit(-1);
	  }
	  i += 1;
	  setLabel(label_count, argv[i]);
	  label_count += 1;
	} else if (!strcmp(argv[i], "-print-labels")) {
	  print_labels = TRUE;
	} else if (!strcmp(argv[i], "-humanlog")) {
	  if (i + 1 >= argc) {
	    fprintf(stderr, "-humanlog must be followed by a file name or '-'\n");
	    exit(-1);
	  }
	  i += 1;
	  setHumanLog(argv[i]);
	} else if (!strcmp(argv[i], "-runall")) {
	  for (j = 0; j < MAX_TEST; j++) {
	    runTest[j] = TRUE;
	  }
	} else if (!strcmp(argv[i], "-fast")) {
	  fastAndLoose = 1;
        } else {
	  /* Let's just ignore unrecognized parameters.  They might be
	   * important to a specific test.  Well, not ignore.  I'll print
	   * a warning.
	   */
	  /* fprintf(stderr, "WARNING: unexpected parameter '%s'\n", argv[i]); */
            /* fprintf(stderr, "%s\n", USAGE); */
            /* exit(-1); */
        }
    }

    if ((logfilename != NULL) && (strcmp(logfilename, "-") != 0)) {
      /* Set up the log file */
      /* fprintf(stderr, "[%s:%u] - Opening log file '%s'\n", __FILE__, __LINE__, logfilename); /*DEBUG*/
      outlog = fopen(logfilename, "a");
      if (NULL == outlog) {
	fprintf(stderr, "Error opening log file %s\n", logfilename);
	exit(-1);
      }
      errlog = outlog;
    } else {
      /* fprintf(stderr, "[%s:%u] - Logging to default file\n", __FILE__, __LINE__); /*DEBUG*/
      outlog = stdout;
      errlog = stderr;
    }

    if ((argc==1) || debugPrint)
        logstatus("Mutatee %s [%s]:\"%s\"\n", argv[0], 
		  mutateeCplusplus ? "C++" : "C", Builder_id);
    if (argc==1) exit(0);

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
      setUseAttach(TRUE);
      logstatus("Waiting for mutator to attach...\n");
      flushOutputLog();
      while (!checkIfAttached()) {
	/* Do nothing */
      }
      /* fprintf(stderr, "[%s:%u] - Mutator attached\n", __FILE__, __LINE__); /*DEBUG*/
      fflush(stderr);
      logstatus("Mutator attached.  Mutatee continuing.\n");
    } else {
      setUseAttach(FALSE);
    }

    /* Run the tests and keep track of return values in case of test failure */
    /* TODO Fix this so that it prints out human log messages in the correct
     * circumstances.  Decide policy for printing out human log messages in
     * the mutatee.  I think I want to print from the mutatee for group tests
     * and from the mutator for non-group tests.
     * TODO Add a group test flag to the boilerplate?  Or something like that?
     */
    for (i = 0; i < MAX_TEST; i++) {
      if (runTest[i]) {
	updateResumeLog(resumelog_name, mutatee_funcs[i].testname);
	/* fprintf(stderr, "[%s:%u] - calling %s_mutateeTest()\n", __FILE__, __LINE__, g_info.funcs[i].testname); /*DEBUG*/
	if (print_labels && (mutatee_funcs[i].testlabel != NULL)) {
	  logstatus("%s\n", mutatee_funcs[i].testlabel);
	}
	mutatee_funcs[i].func();
	updateResumeLogCompleted(resumelog_name);
	/* TODO Do I need to print out a success message here for groupable
	 * tests that pass?
	 */
	if (passedTest[i] && groupable_mutatee) {
	  /* FIXME This will also print that skipped tests passed.  But I
	   * shouldn't be running skipped tests in the first place, so it
	   * should be okay.
	   */
	  printResultHumanLog(mutatee_funcs[i].testname, PASSED);
	}
	if (!passedTest[i]) {
	  if (groupable_mutatee) {
	    /* Only print test failure messages from the mutatee for group
	     * mutatees */
	    printResultHumanLog(mutatee_funcs[i].testname, FAILED);
	  }
	  allTestsPassed = FALSE;
	}
      }

      flushOutputLog();
      flushErrorLog();
    }

    if (allTestsPassed) {
      logstatus("All tests passed.\n");
      retval = 0;
    } else {
      retval = -1;
    }

    /* Clean up after ourselves */
    if ((outlog != NULL) && (outlog != stdout)) {
      fclose(outlog);
    }

    /* fprintf(stderr, "[%s:%u] - Mutatee driver exiting\n", __FILE__, __LINE__); /*DEBUG*/
    return retval;
}
