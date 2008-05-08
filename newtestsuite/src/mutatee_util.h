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

/* Utility functions used by some test program mutatees. */

#ifndef _mutatee_util_h_
#define _mutatee_util_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#if defined(os_windows)

#include <windows.h>

#if !defined(INVALID_HANDLE)
#define INVALID_HANDLE ((HANDLE) -1)
#endif

typedef CRITICAL_SECTION testlock_t;
typedef struct  {
    int threadid;
    HANDLE hndl;
} thread_t;

#else

#include <pthread.h>
#include <unistd.h>
typedef pthread_mutex_t testlock_t;
typedef pthread_t thread_t;

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
static volatile int dummy3__;
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

/* Mutatee cleanup: PID registration */
extern void setPIDFilename(char *pfn);
extern char *getPIDFilename();
extern void registerPID(int pid);


extern int setupFortranOutput();
extern int cleanupFortranOutput();

extern int fastAndLoose;

extern char *executable_name;
extern void setExecutableName(const char *new_name);
extern int use_attach;
extern void setUseAttach(int v);
extern char *humanlog_name;
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
extern void joinThread(thread_t tid);
extern int threads_equal(thread_t a, thread_t b);
extern void initThreads();

extern void initLock(testlock_t *newlock);
extern void testLock(testlock_t *lck);
extern void testUnlock(testlock_t *lck);
extern thread_t threadSelf();
extern unsigned long thread_int(thread_t a);
extern void schedYield();

extern void *loadDynamicLibrary(char *name);
extern void *getFuncFromDLL(void *libhandle, char *func_name);

extern void test_passes(const char *testname);
extern void test_fails(const char *testname);
extern int verifyScalarValue(const char *name, int a, int value,
			     const char *testName, const char *testDesc);

#if !defined(P_sleep)
#if defined(os_windows)
#define P_sleep(sec) Sleep(1000*(sec))
#else
#define P_sleep(sec) sleep(sec)
#endif
#endif

#ifdef __cplusplus
} /* terminate extern "C" */
#endif

#endif /* _mutatee_util_h_ */
