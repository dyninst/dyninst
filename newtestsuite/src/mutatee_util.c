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

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "mutatee_util.h"

#ifdef os_windows
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#endif

#include "mutatee_call_info.h"
extern mutatee_call_info_t mutatee_funcs[];
extern int passedTest[];
extern unsigned int MAX_TEST;

#include "test_results.h"

#ifdef __cplusplus
}
#endif

/*
 * Stop the process (in order to wait for the mutator to finish what it's
 * doing and restart us).
 */
#if defined(alpha_dec_osf4_0) && defined(__GNUC__)
static long long int  beginFP;
#endif

/* global variable that stores mutatee information */
/* mutatee_info_t g_info; */

/* Provide standard output functions that respect the specified output files */
FILE *outlog = NULL;
FILE *errlog = NULL;
int logstatus(const char *fmt, ...) {
  int retval;
  va_list args;
  va_start(args, fmt);
  if (outlog != NULL) {
    retval = vfprintf(outlog, fmt, args);
  } else {
    retval = vprintf(fmt, args);
  }
  va_end(args);
  flushOutputLog();
  return retval;
}
int logerror(const char *fmt, ...) {
  int retval;
  va_list args;
  va_start(args, fmt);
  if (errlog != NULL) {
    retval = vfprintf(errlog, fmt, args);
  } else {
    retval = vfprintf(stderr, fmt, args);
  }
  va_end(args);
  flushErrorLog();
  return retval;
}
void flushOutputLog() {
  if (outlog != NULL) {
    fflush(outlog);
  }
}
void flushErrorLog() {
  if (errlog != NULL) {
    fflush(errlog);
  }
}

static int saved_stdout_fd = -1;
static int stdout_fd = 1;
/* setupFortranOutput() redirects stdout to point to outlog.  This seemed
 * like the easiest way to make the Fortran mutatees output the same way that
 * the rest of the mutatees do.
 * Returns 0 on success, <0 on failure.
 */
/* So there's a bug here, or else some behavior that I just don't understand,
 * where unless we write something to stdout before the dup2() redirection
 * it doesn't actually get redirected.  That's what the printf(" ") is in
 * there for.
 */
int setupFortranOutput() {
  int outlog_fd = fileno(outlog);
  if (-1 == outlog_fd) {
    return -1; /* Error */
  }
  printf(" "); /* Workaround */
  stdout_fd = fileno(stdout);
  saved_stdout_fd = dup(stdout_fd); /* Duplicate stdout */
  if (-1 == saved_stdout_fd) {
    return -2; /* Error */
  }
  if (dup2(outlog_fd, stdout_fd) == -1) {
    return -3; /* Error */
  }
  return 0;
}
/* cleanupFortranOutput() restores stdout so it points where it did before
 * a call to setupFortranOutput().  If setupFortranOutput() was never called,
 * then it just returns failure.
 * Returns 0 on successs, <0 on failure.
 */
int cleanupFortranOutput() {
  if (-1 == saved_stdout_fd) {
    return -1;
  }
  if (dup2(saved_stdout_fd, stdout_fd) == -1) {
    return -2; /* Error */
  }
  return 0;
}

int fastAndLoose = 0;

/* executable_name holds the name of the current mutatee binary */
char *executable_name = NULL;

void setExecutableName(const char *new_name) {
  executable_name = (char *) new_name;
}

/* Flag that says whether we're running in create or attach mode */
int use_attach = FALSE;
void setUseAttach(int v) {
  if (v) { /* normalize to {TRUE, FALSE} */
    use_attach = TRUE;
  } else {
    use_attach = FALSE;
  }
}

/* Filename for single line test results output */
char *humanlog_name = "-";
/* Change this to default to FALSE once test_driver is passing -humanlog
 * parameter to the mutatee
 */
int use_humanlog = TRUE;
void setHumanLog(const char *new_name) {
  if (NULL == new_name) {
    use_humanlog = FALSE;
  } else {
    use_humanlog = TRUE;
  } 
  humanlog_name = (char *) new_name;
}

/* Print out single line message reporting whether a test passed, failed, was
 * skipped, or crashed.
 */
void printResultHumanLog(const char *testname, test_results_t result)
{
  FILE *human;

  if ((NULL == humanlog_name) || !strcmp(humanlog_name, "-")) {
    human = stdout;
  } else {
    human = fopen(humanlog_name, "a");
    if (NULL == human) {
      fprintf(stderr, "Error opening human log file '%s': %s\n",
	      humanlog_name, strerror(errno));
      human = stdout;
    }
  }

  fprintf(human, "%s: mutatee: %s create_mode: ", testname,
	  (NULL == executable_name) ? "(unknown)" : executable_name);
  if (use_attach) {
    fprintf(human, "attach");
  } else {
    fprintf(human, "create");
  }
  fprintf(human, "\tresult: ");
  switch (result) {
  case PASSED:
    fprintf(human, "PASSED\n");
    break;

  case SKIPPED:
    fprintf(human, "SKIPPED\n");
    break;

  case FAILED:
    fprintf(human, "FAILED\n");
    break;
  }

  if (stdout == human) {
    fflush(human);
  } else {
    fclose(human);
  }
}

#ifdef DETACH_ON_THE_FLY
/*
 All this to stop ourselves.  We may be detached, but the mutator
 needs to notice the stop.  We must send a SIGILL to ourselves, not
 SIGSTOP, to get the mutator to notice.

 DYNINSTsigill is a runtime library function that does this.  Here we
 obtain a pointer to DYNINSTsigill from the runtime loader and then
 call it.  Note that this depends upon the mutatee having
 DYNINSTAPI_RT_LIB defined (with the same value as mutator) in its
 environment, so this technique does not work for ordinary mutatees.

 We could call kill to send ourselves SIGILL, but this is unsupported
 because it complicates the SIGILL signal handler.  */
static void
dotf_stop_process()
{
     void *h;
     char *rtlib;
     static void (*DYNINSTsigill)() = NULL;

     if (!DYNINSTsigill) {
	  /* Obtain the name of the runtime library linked with this process */
	  rtlib = getenv("DYNINSTAPI_RT_LIB");
	  if (!rtlib) {
	       fprintf(stderr, "ERROR: Mutatee can't find the runtime library pathname\n");
	       assert(0);
	  }

	  /* Obtain a handle for the runtime library */
	  h = dlopen(rtlib, RTLD_LAZY); /* It should already be loaded */
	  if (!h) {
	       fprintf(stderr, "ERROR: Mutatee can't find its runtime library: %s\n",
		       dlerror());
	       assert(0);
	  }

	  /* Obtain a pointer to the function DYNINSTsigill in the runtime library */
	  DYNINSTsigill = (void(*)()) dlsym(h, "DYNINSTsigill");
	  if (!DYNINSTsigill) {
	       fprintf(stderr, "ERROR: Mutatee can't find DYNINSTsigill in the runtime library: %s\n",
		       dlerror());
	       assert(0);
	  }
     }
     DYNINSTsigill();
}
#endif /* DETACH_ON_THE_FLY */

void stop_process_()
{
#ifdef i386_unknown_nt4_0
    DebugBreak();
#else

#if defined(alpha_dec_osf4_0) && defined(__GNUC__)
    /* This GCC-specific hack doesn't compile with other compilers, 
       which is unfortunate, since the native build test fails part 15
       with the error "process did not signal mutator via stop". */
    /* It also doesn't appear to be necessary when compiling with gcc-2.8.1,
       which makes its existance even more curious. */
    register long int fp asm("15");

    beginFP = fp;
#endif

#ifdef DETACH_ON_THE_FLY
    dotf_stop_process();
    return;
#endif

#if !defined(bug_irix_broken_sigstop)
    kill(getpid(), SIGSTOP);
#else
    kill(getpid(), SIGEMT);
#endif

#if defined(alpha_dec_osf4_0) && defined(__GNUC__)
    fp = beginFP;
#endif

#endif
}

/* This function sets a flag noting that testname ran successfully */
/* FIXME Need to give this a handle to the driver's mutatee_info structure
 * somehow
 */
void test_passes(const char *testname) {
  unsigned int i;
  for (i = 0; i < MAX_TEST; i++) {
    if (!strcmp(mutatee_funcs[i].testname, testname)) {
      /* Found it */
      passedTest[i] = TRUE;
      break;
    }
  }
}

/* This function sets a flag noting that testname ran unsuccessfully */
void test_fails(const char *testname) {
  unsigned int i;
  for (i = 0; i < MAX_TEST; i++) {
    if (!strcmp(mutatee_funcs[i].testname, testname)) {
      /* Found it */
      passedTest[i] = FALSE;
      break;
    }
  }
}

/* This function returns TRUE if the test has been marked as passing, and false
 * otherwise
 */
static int test_passed(const char *testname) {
  unsigned int i;
  int retval;
  for (i = 0; i < MAX_TEST; i++) {
    if (!strcmp(mutatee_funcs[i].testname, testname)) {
      /* Found it */
      retval = passedTest[i];
      break;
    }
  }
  if (i >= MAX_TEST) {
    retval = FALSE; /* Not found */
  }
  return retval;
}

/*
 * Verify that a scalar value of a variable is what is expected
 * returns TRUE on success, FALSE on failure
 */
int verifyScalarValue(const char *name, int a, int value,
		      const char *testName, const char *testDesc)
{
  if (a != value) {
    if (test_passed(testName)) {
      logerror("**Failed** test %s (%s)\n", testName, testDesc);
    }
    logerror("  %s = %d, not %d\n", name, a, value);
    return FALSE;
  }
  return TRUE;
}
