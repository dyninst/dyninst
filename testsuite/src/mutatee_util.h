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

/* Utility functions used by some test program mutatees. */

#ifndef _mutatee_util_h_
#define _mutatee_util_h_

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(os_windows_test)
#include <stdint.h>
#else
	typedef unsigned __int64 uint64_t;
#endif

#include <stdio.h>

#if defined(os_windows_test)
#include <winsock2.h>
#include <windows.h>

#if !defined(INVALID_HANDLE)
#define INVALID_HANDLE ((HANDLE) -1)
#endif

typedef CRITICAL_SECTION testlock_t;
typedef struct  {
    int threadid;
    HANDLE hndl;
} thread_t;

// TODO needs to be implemented if a barrier
// is ever needed in the tests on Windows
typedef int testbarrier_t;

#else

#include <pthread.h>
#include <unistd.h>
typedef pthread_mutex_t testlock_t;
typedef pthread_t thread_t;

#if defined(os_aix_test)
typedef int testbarrier_t; // Older versions of AIX don't define pthread_barrier_t
#else
typedef pthread_barrier_t testbarrier_t;
#endif

#endif

#include <stdarg.h>

#define TRUE 1
#define FALSE 0

#include "test_results.h"

/* Include mutatee_glue.h as a convenience to the user, so they don't need
 * to
 */
/* Not using this any more; using sed to preprocess again */
/* #include "mutatee_glue.h" */

extern int debugPrint;
/* control debug printf statements */
#define dprintf	if (debugPrint) printf

/* Empty functions are sometimes compiled too tight for entry and exit
   points.  The following macro is used to flesh out these
   functions. (expanded to use on all platforms for non-gcc compilers jkh 10/99)
 */
static volatile int dummy3__ = 0;
#define DUMMY_FN_BODY \
  int dummy1__ = 1; \
  int dummy2__ = 2; \
  dummy3__ = dummy1__ + dummy2__

#include "mutatee_call_info.h"
extern mutatee_info_t g_info;

/* mutatee_driver.c defines these two; they're just copies of main()'s argc
 * and argv.
 */
extern int gargc;
extern char **gargv;

/* New logging system */
typedef enum {
  STDOUT,
  STDERR,
  LOGINFO,
  LOGERR,
  HUMAN
} output_stream_t;
typedef void (*log_f)(output_stream_t, const char *, ...);
typedef void (*vlog_f)(output_stream_t, const char *, va_list);
typedef void (*log_result_f)(test_results_t);
typedef void (*redirect_stream_f)(output_stream_t, const char *);
typedef void (*set_testname_f)(const char *);
typedef struct {
  log_f log;
  vlog_f vlog;
  redirect_stream_f redirectStream;
  log_result_f logResult;
  set_testname_f setTestName;
} output_t;
extern output_t *output;

/* This guy initializes the output object with pointers to the correct
 * functions
 */
extern void initOutputDriver();
extern void stdOutputLog(output_stream_t stream, const char *fmt, ...);
extern void stdOutputVLog(output_stream_t stream, const char *fmt, va_list args);
extern void redirectStream(output_stream_t stream, const char *filename);
extern void setTestName(const char *name);

/* Support functions for the database output driver */
extern void initDatabaseOutputDriver();
extern void dbOutputLog(output_stream_t stream, const char *fmt, ...);
extern void dbOutputVLog(output_stream_t stream, const char *fmt, va_list args);
extern void dbRedirectStream(output_stream_t stream, const char *filename);
extern void dbLogResult(test_results_t result);
extern void dbSetTestName(const char *name);
extern void closeDatabaseOutputDriver();

extern FILE *outlog;
extern FILE *errlog;
extern void logstatus(const char *fmt, ...);
extern void logerror(const char *fmt, ...);
extern void flushOutputLog();
extern void flushErrorLog();

extern void log_testrun(const char *testname);
extern void log_testresult(int passed);

/* Mutatee cleanup: PID registration */
extern void registerPID(int pid);


extern int setupFortranOutput();
extern int cleanupFortranOutput();

extern char *executable_name;
extern void setExecutableName(const char *new_name);
extern int use_attach;
extern void setUseAttach(int v);
extern const char *humanlog_name;
extern int use_humanlog;
extern void setHumanLog(const char *new_name);
extern void printResultHumanLog(const char *testname, test_results_t result);

/* Each mutatee defines testname as a global variable */
#define REPORT_TEST_PASSED() \
  do { \
    if (use_humanlog) printResultHumanLog(testname, PASSED); \
  } while (0)
#define REPORT_TEST_SKIPPED() \
  do { \
    if (use_humanlog) printResultHumanLog(testname, SKIPPED); \
  } while (0)
#define REPORT_TEST_FAILED() \
  do { \
    if (use_humanlog) printResultHumanLog(testname, FAILED); \
  } while (0)

extern void stop_process_();

extern thread_t spawnNewThread(void *initial_func, void *param);
extern void* joinThread(thread_t tid);
extern int threads_equal(thread_t a, thread_t b);
extern void initThreads();

extern void initLock(testlock_t *newlock);
extern void testLock(testlock_t *lck);
extern void testUnlock(testlock_t *lck);
extern void initBarrier(testbarrier_t *barrier, unsigned int count);
extern void waitTestBarrier(testbarrier_t *barrier);
extern thread_t threadSelf();
extern unsigned long thread_int(thread_t a);
extern void schedYield();

extern void *loadDynamicLibrary(char *name);
extern void *getFuncFromDLL(void *libhandle, const char *func_name);

extern void test_passes(const char *testname);
extern void test_fails(const char *testname);
extern int verifyScalarValue(const char *name, int a, int value,
			     const char *testName, const char *testDesc);

extern void handleAttach();

#if !defined(P_sleep)
#if defined(os_windows_test)
#define P_sleep(sec) Sleep(1000*(sec))
#else
#define P_sleep(sec) sleep(sec)
#endif
#endif

/*
 * A high precision sleep
 *
 * Sleeps for time specified by milliseconds. This function assumes that
 * the parameter is less than 1000.
 *
 * Returns 0 if the sleep fails.
 */
int precisionSleep(int milliseconds);

/* Event source interface
 * Schedule a timer (or some other mechanism) that periodically delivers
 * events to the mutator on behalf of the mutatee
 *
 * The underlying mechanism is most definitely portable so hence the
 * void *.
 */
typedef struct event_source_struct event_source;

/* Starts the event source and returns the opaque handle to the
 * event source. This handle is NULL if the event source
 * cannot be created.
 */
event_source *startEventSource();

/* Returns the number of events that have occurred so far */
uint64_t getEventCounter();

/* Stops the specified event source.
 *
 * Returns non-zero values if successful, zero on failure
 * On success, the specified event source will be free'd.
 */
int stopEventSource(event_source *eventSource);

#ifdef __cplusplus
} /* terminate extern "C" */
#endif

#endif /* _mutatee_util_h_ */
