/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#if !defined(os_windows_test)
#include <sys/time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "mutatee_util.h"

#ifdef os_windows_test
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
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
#if defined(alpha_dec_osf4_0_test) && defined(__GNUC__)
static long long int  beginFP;
#endif

/* New output driver system */
output_t *output = NULL;
char *loginfofn = (char*)"-";
char *logerrfn = (char*)"-";
char *stdoutfn = (char*)"-";
char *stderrfn = (char*)"-";
char *humanfn = (char*)"-";
static char *od_testname = NULL;

void warningLogResult(test_results_t result) {
   fprintf(stderr, "[%s:%u] - WARNING: output object not properly initialized\n", __FILE__, __LINE__);
}

void nullSetTestName(const char *testname) {
}

void stdLogResult(test_results_t result);
void stdSetTestName(const char *testname);

void initOutputDriver() {
   output = (output_t *) malloc(sizeof (*output));
   if (NULL == output) {
      /* This is a nasty error, and the mutatee can't function any more */
      fprintf(stderr, "[%s:%u] - Error allocating output driver object\n", __FILE__, __LINE__);
      abort(); /* Is this the right thing to do? */
   }
   output->log = stdOutputLog;
   output->vlog = stdOutputVLog;
   output->redirectStream = redirectStream;
   output->logResult = stdLogResult;
   output->setTestName = stdSetTestName;
}

/* I want to store *copies* of the filenames */
/* I also want to maintain "-" as a constant, so I know I don't need to free it
 * if it's what's being used.
 */
void redirectStream(output_stream_t stream, const char *filename) {
   size_t length;

   if ((filename != NULL) && (strcmp(filename, "-") != 0)) {
      length = strlen(filename) + 1;
   }

   switch (stream) {
      case STDOUT:
         if ((stdoutfn != NULL) && (strcmp(stdoutfn, "-") != 0)) {
            free(stdoutfn);
         }
         if (NULL == filename) {
            stdoutfn = NULL;
         } else if (strcmp(filename, "-") == 0) {
            stdoutfn = (char *)"-";
         } else {
            stdoutfn = (char *) malloc(length * sizeof (*stdoutfn));
            if (stdoutfn != NULL) {
               strncpy(stdoutfn, filename, length);
            }
         }
         break;

      case STDERR:
         if ((stderrfn != NULL) && (strcmp(stderrfn, "-") != 0)) {
            free(stderrfn);
         }
         if (NULL == filename) {
            stderrfn = NULL;
         } else if (strcmp(filename, "-") == 0) {
            stderrfn = (char *)"-";
         } else {
            stderrfn = (char *) malloc(length * sizeof (*stderrfn));
            if (stderrfn != NULL) {
               strncpy(stderrfn, filename, length);
            }
         }
         break;

      case LOGINFO:
         if ((loginfofn != NULL) && (strcmp(loginfofn, "-") != 0)) {
            free(loginfofn);
         }
         if (NULL == filename) {
            loginfofn = NULL;
         } else if (strcmp(filename, "-") == 0) {
            loginfofn = (char *)"-";
         } else {
            loginfofn = (char *) malloc(length * sizeof (*loginfofn));
            if (loginfofn != NULL) {
               strncpy(loginfofn, filename, length);
            }
         }
         break;

      case LOGERR:
         if ((logerrfn != NULL) && (strcmp(logerrfn, "-") != 0)) {
            free(logerrfn);
         }
         if (NULL == filename) {
            logerrfn = NULL;
         } else if (strcmp(filename, "-") == 0) {
            logerrfn = (char *)"-";
         } else {
            logerrfn = (char *) malloc(length * sizeof (*logerrfn));
            if (logerrfn != NULL) {
               strncpy(logerrfn, filename, length);
            }
         }
         break;

      case HUMAN:
         if ((humanfn != NULL) && (strcmp(humanfn, "-") != 0)) {
            free(humanfn);
         }
         if (NULL == filename) {
            humanfn = NULL;
         } else if (strcmp(filename, "-") == 0) {
            humanfn = (char *)"-";
         } else {
            humanfn = (char *) malloc(length * sizeof (*humanfn));
            if (humanfn != NULL) {
               strncpy(humanfn, filename, length);
            }
         }
         break;
   }
}

void stdOutputLog(output_stream_t stream, const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);
   stdOutputVLog(stream, fmt, args);
   va_end(args);
}

void stdOutputVLog(output_stream_t stream, const char *fmt, va_list args) {
   char *logfn = NULL;
   switch (stream) {
      case STDOUT:
         logfn = stdoutfn;
         break;

      case STDERR:
         logfn = stderrfn;
         break;

      case LOGINFO:
         logfn = loginfofn;
         break;

      case LOGERR:
         logfn = logerrfn;
         break;

      case HUMAN:
         logfn = humanfn;
         break;
   }

   if (logfn != NULL) { /* Print nothing if it is NULL */
      FILE *out = NULL;
      if (strcmp(logfn, "-") == 0) { /* Default output */
         switch (stream) {
            case STDOUT:
            case LOGINFO:
            case HUMAN:
               out = stdout;
               break;

            case STDERR:
            case LOGERR:
               out = stderr;
               break;
         }
      } else { /* Specified output */
         out = fopen(logfn, "a");
         if (NULL == out) {
            fprintf(stderr, "[%s:%u] - Error opening log output file '%s'\n", __FILE__, __LINE__, logfn);
         }
      }

      if (out != NULL) {
         vfprintf(out, fmt, args);
         if ((out != stdout) && (out != stderr)) {
            fclose(out);
         }
      }
   }
} /* stdOutputVLog() */

void stdLogResult(test_results_t result) {
   printResultHumanLog(od_testname, result);
} /* stdLogResult() */

void stdSetTestName(const char *testname) {
   if (NULL == testname) {
      if (od_testname != NULL) {
         free(od_testname);
      }
      od_testname = NULL;
   } else {
      size_t len = strlen(testname) + 1;
      od_testname = (char *) malloc(len * sizeof (char));
      strncpy(od_testname, testname, len);
   }
}


/* Provide standard output functions that respect the specified output files */
FILE *outlog = NULL;
FILE *errlog = NULL;
void logstatus(const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);
   stdOutputVLog(LOGINFO, fmt, args);
   va_end(args);
}
void logerror(const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);
   stdOutputVLog(LOGERR, fmt, args);
   va_end(args);
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

/************************************************/
/* Support functions for database output driver */
/************************************************/

static const char *temp_logfilename = "mutatee_dblog";
static char *dblog_filename = NULL;

/* Redirect all output to a file, so test_driver can pick it up when it's
 * time to report results to the database server
 */
void initDatabaseOutputDriver() {
   output = (output_t *) malloc(sizeof (*output));
   if (output != NULL) {
      output->log = dbOutputLog;
      output->vlog = dbOutputVLog;
      output->redirectStream = dbRedirectStream;
      output->logResult = dbLogResult;
      output->setTestName = dbSetTestName;
   } else {
      fprintf(stderr, "[%s:%u] - Out of memory allocating output object\n", __FILE__, __LINE__);
      exit(-1); /* This is very bad */
   }
}

/* Sets the current test name, which controls the file we log to */
void dbSetTestName(const char *testname) {
   size_t len; /* For string allocation */
   FILE *pretestLog;

   if (NULL == testname) {
      /* TODO Handle error */
      return;
   }

   len = strlen("dblog.") + strlen(testname) + 1;
   dblog_filename = (char *) realloc(dblog_filename, len * sizeof (char));
   if (NULL == dblog_filename) {
      fprintf(stderr, "[%s:%u] - Out of memory allocating mutatee database log file name\n", __FILE__, __LINE__);
      return;
   }

   snprintf(dblog_filename, len, "dblog.%s", testname);

   /* Copy contents of temp log file into the dblog file, if there's
    * anything there.
    */
   pretestLog = fopen(temp_logfilename, "r");
   if (pretestLog != NULL) {
      /* Copy the file */
      size_t count;
      char *buffer[4096]; /* FIXME Magic number */
      FILE *out = fopen(dblog_filename, "a");
      if (out != NULL) {
         do {
            count = fread(buffer, sizeof (char), 4096, pretestLog);
            fwrite(buffer, sizeof (char), count, out);
         } while (4096 == count); /* FIXME Magic number */
      }
   } else if (errno != ENOENT) {
      /* It's not a problem is there is no pretest log file */
      fprintf(stderr, "[%s:%u] - Error opening pretest log: '%s'\n", __FILE__, __LINE__, strerror(errno));
   }
}

void warningRedirectStream(output_stream_t stream, const char *filename) {
   fprintf(stderr, "[%s:%u] - WARNING: output object not properly initialized\n", __FILE__, __LINE__);
}

void warningVLog(output_stream_t stream, const char *fmt, va_list args) {
   fprintf(stderr, "[%s:%u] - WARNING: output object not properly initialized\n", __FILE__, __LINE__);
}

void warningLog(output_stream_t stream, const char * fmt, ...) {
   fprintf(stderr, "[%s:%u] - WARNING: output object not properly initialized\n", __FILE__, __LINE__);
}

void warningSetTestName(const char *testname) {
   fprintf(stderr, "[%s:%u] - WARNING: output object not properly initialized\n", __FILE__, __LINE__);
}

void closeDatabaseOutputDriver() {
   if (dblog_filename != NULL) {
      free(dblog_filename);
   }
   output->log = warningLog;
   output->vlog = warningVLog;
   output->redirectStream = warningRedirectStream;
   output->logResult = warningLogResult;
   output->setTestName = warningSetTestName;
}

/* What do I do with output I receive before the output file name has been
 * set?  I need to store it in a temp. file or something.
 */
void dbOutputLog(output_stream_t stream, const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);
   dbOutputVLog(stream, fmt, args);
   va_end(args);
}

void dbOutputVLog(output_stream_t stream,
                  const char *fmt, va_list args) {
   FILE *out;

   if (NULL == dblog_filename) {
      out = fopen(temp_logfilename, "a");
   } else {
      out = fopen(dblog_filename, "a");
   }

   if (NULL == out) {
      fprintf(stderr, "[%s:%u] - Error opening log file '%s'\n", __FILE__, __LINE__, (dblog_filename) ? dblog_filename : temp_logfilename);
      return;
   }

   vfprintf(out, fmt, args);
   fclose(out);
}

void dbRedirectStream(output_stream_t stream, const char *filename) {
   /* This doesn't do anything */
}

void dbLogResult(test_results_t result) {
   FILE *out;

   if (NULL == dblog_filename) {
      /* TODO Handle error */
   }

   out = fopen(dblog_filename, "a");
   if (NULL == out) {
      /* TODO Handle error */
   }

   fprintf(out, "RESULT: %d\n\n", result);
   fclose(out);
}


/*********************************************************/
/* Support for cleaning up mutatees after a test crashes */
/*********************************************************/

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
   //printf(" "); /* Workaround */
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
const char *humanlog_name = "-";
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
   /* humanlog_name = (char *) new_name; */
   redirectStream(HUMAN, new_name);
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
         output->log(STDERR, "Error opening human log file '%s': %s\n",
                     humanlog_name, strerror(errno));
         human = stdout;
      }
   }

   output->log(HUMAN, "%s: mutatee: %s create_mode: ", testname,
               (NULL == executable_name) ? "(unknown)" : executable_name);
   if (use_attach) {
      output->log(HUMAN, "attach");
   } else {
      output->log(HUMAN, "create");
   }
   output->log(HUMAN, "\tresult: ");
   switch (result) {
      case PASSED:
         output->log(HUMAN, "PASSED\n");
         break;

      case SKIPPED:
         output->log(HUMAN, "SKIPPED\n");
         break;

      case FAILED:
         output->log(HUMAN, "FAILED\n");
         break;
      case CRASHED:
         output->log(HUMAN, "CRASHED");
         break;
      case UNKNOWN:
         output->log(HUMAN, "UNKNOWN");
         break;
   }

   if (stdout == human) {
      fflush(human);
   } else {
      fclose(human);
   }

}

void stop_process_()
{
#if defined(os_windows_test)
   DebugBreak();
#else
   kill(getpid(), SIGSTOP);
#endif
}

/* This function sets a flag noting that testname ran successfully */
/* FIXME Need to give this a handle to the driver's mutatee_info structure
 * somehow
 */
void test_passes(const char *testname) {
   int i;
   for (i = 0; i < max_tests; i++) {
      if (!strcmp(mutatee_funcs[i].testname, testname)) {
         /* Found it */
         passedTest[i] = TRUE;
         break;
      }
   }
}

/* This function sets a flag noting that testname ran unsuccessfully */
void test_fails(const char *testname) {
   int i;
   for (i = 0; i < max_tests; i++) {
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
   int i;
   int retval;
   for (i = 0; i < max_tests; i++) {
      if (!strcmp(mutatee_funcs[i].testname, testname)) {
         /* Found it */
         retval = passedTest[i];
         break;
      }
   }
   if (i >= max_tests) {
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

const char *resumelog_name = "mutatee_resumelog";

void log_testrun(const char *testname)
{
   FILE *f = fopen(resumelog_name, "a");
   if (!f) {
      output->log(STDERR, "Could not write to resume log\n");
      exit(0);
   }
   
   fprintf(f, "%s\n", testname);
   fclose(f);
}

void log_testresult(int passed)
{
   FILE *f = fopen(resumelog_name, "a");
   if (!f) {
      output->log(STDERR, "Could not write to resume log\n");
      exit(0);
   }
   fprintf(f, "%d\n", passed ? 1 : 0);
   fclose(f);
}

/* high precision timer */

#if defined(os_linux_test) || defined(os_freebsd_test)
#if defined(_POSIX_C_SOURCE)
#undef _POSIX_C_SOURCE
#endif
#define _POSIX_C_SOURCE 199309 

#include <time.h>
#include <errno.h>
#elif defined(os_bg_test)
#include <sys/select.h>
#endif

int precisionSleep(int milliseconds) {
#if defined(os_linux_test) || defined(os_freebsd_test)
    struct timespec req;
    struct timespec rem;

    if( milliseconds >= 1000 ) return 0;

    memset(&rem, 0, sizeof(struct timespec));
    req.tv_sec = 0;
    req.tv_nsec = milliseconds*1000*1000;

    int result, error;
    do {
        result = nanosleep(&req, &rem);
        error = errno;
        if (req.tv_nsec == rem.tv_nsec) {
           //Buggy kernel - rem never set.  Just decrement it by 1/10th
           unsigned long decrement = milliseconds*1000*100;
           if (decrement >= (unsigned long) req.tv_nsec)
              break;
           else {
              req.tv_nsec -= decrement;
           }
        }
        else {
           req = rem;
        }
    }while(result == -1 && error == EINTR);

    if( result == -1 ) return 0;
    return 1;
#elif defined(os_bg_test)
    struct timeval timeout;
    
    int result;
    struct timeval start, cur;
    unsigned long long istart, icur, microseconds;
    microseconds = milliseconds * 1000;
    int tresult = gettimeofday(&start, NULL);
    assert(tresult != -1);
    istart = (start.tv_sec * 1000000) + start.tv_usec;
    

    timeout.tv_sec = 0;
    timeout.tv_usec = microseconds;

    for (;;) {
       result = select(1, NULL, NULL, NULL, &timeout);
       if (result == -1 && errno != EINTR) {
          perror("precisionSleep select failed");
          return 0;
       }
       if (result == 0) {
          return 1;
       }
       tresult = gettimeofday(&cur, NULL);
       assert(tresult != -1);
       icur = (cur.tv_sec * 1000000) + cur.tv_usec;
       if (icur - istart >= microseconds) {
          return 1;
       }
       timeout.tv_usec = microseconds - (icur - istart);
    } 

#elif defined(os_windows_test)
    Sleep(milliseconds);
    return 1;
#else
    return 0;
#endif
}

/* Event source interface */

static uint64_t eventCounter = 0;

#if defined(os_linux_test) || defined(os_freebsd_test)
#define TIMER_EVENT_SOURCE
#endif

#if defined(TIMER_EVENT_SOURCE)
#include <signal.h>
#include <sys/time.h>


/* 
 * headers define: 
 *
 * typedef struct event_source_struct event_source
 *
 * but the actual definition needs to be opaque due to platform dependent internals
 */
struct event_source_struct {
    struct itimerval timer;
    struct sigaction action;
    struct sigaction old_action;
};

void handler(int sig, siginfo_t *siginfo, void *context) {
    eventCounter++;
}

#else
struct event_source_struct {
    int unused;
};
#endif

event_source *startEventSource() {
    event_source *retVal = NULL;
#if defined(TIMER_EVENT_SOURCE)
    retVal = (event_source *)malloc(sizeof(struct event_source_struct));

    /* First, register the signal handler */
    memset(&(retVal->action), 0, sizeof(retVal->action));
    retVal->action.sa_sigaction = handler;
    retVal->action.sa_flags = SA_SIGINFO;
    if( sigaction(SIGALRM, &(retVal->action), &(retVal->old_action)) ) {
        free(retVal);
        return NULL;
    }

    /* Next, setup the profiling timer (every 10ms) */
    retVal->timer.it_interval.tv_sec = 0;
    retVal->timer.it_interval.tv_usec = 10000;
    retVal->timer.it_value = retVal->timer.it_interval;
    if( setitimer(ITIMER_REAL, &(retVal->timer), NULL) == -1 ) {
        free(retVal);
        return NULL;
    }
#endif
    return retVal;
}

uint64_t getEventCounter() {
    return eventCounter;
}

int stopEventSource(event_source *eventSource) {
    int retVal = 0;
#if defined(TIMER_EVENT_SOURCE)
    /* First, turn off the timer */
    eventSource->timer.it_value.tv_sec = 0;
    eventSource->timer.it_value.tv_usec = 0;
    eventSource->timer.it_interval.tv_sec = 0;
    eventSource->timer.it_interval.tv_usec = 0;
    if( setitimer(ITIMER_PROF, &(eventSource->timer), NULL) == -1 ) {
        return 0;
    }

    /* Next, unregister the signal handler */
    if( sigaction(SIGPROF, &(eventSource->old_action), NULL) ) {
        return 0;
    }

    free(eventSource);
    retVal = 1;
#else
    eventSource = eventSource;
#endif
    return retVal;
}
