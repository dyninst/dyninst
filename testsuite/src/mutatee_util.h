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

extern FILE *outlog;
extern FILE *errlog;
extern int logstatus(const char *fmt, ...);
extern int logerror(const char *fmt, ...);
extern void flushOutputLog();
extern void flushErrorLog();

extern int fastAndLoose;

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

#if !defined(P_sleep)
#if defined(os_windows)
#define P_sleep(sec) Sleep(1000*(sec))
#else
#define P_sleep(sec) sleep(sec)
#endif
#endif

#endif /* _mutatee_util_h_ */
