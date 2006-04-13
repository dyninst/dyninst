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
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "mutatee_util.h"

#if defined(os_windows)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

thread_t spawnNewThread(void *initial_func, void *param) {
    thread_t newthr;
    newthr.hndl = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) initial_func, 
                               param, 0, (LPDWORD) &(newthr.threadid));
    return newthr;
}

void joinThread(thread_t threadid) {
    HANDLE tid;
    DWORD result;

    tid = threadid.hndl;
    assert(tid != INVALID_HANDLE);
    for (;;) {
        GetExitCodeThread(tid, &result);
        if (result != STILL_ACTIVE)
            break;
        Sleep(500);
    }
    CloseHandle(tid);
}

thread_t threadSelf() {
    thread_t self;
    self.hndl = INVALID_HANDLE;
    self.threadid = GetCurrentThreadId();
    return self;
}

void initThreads() {
}

void initLock(testlock_t *newlock) {
    CRITICAL_SECTION *cs = (CRITICAL_SECTION *) newlock;
    InitializeCriticalSection(cs);
}

void testLock(testlock_t *lck) {
    CRITICAL_SECTION *cs = (CRITICAL_SECTION *) lck;
    EnterCriticalSection(lck);
}

void testUnlock(testlock_t *lck) {
    CRITICAL_SECTION *cs = (CRITICAL_SECTION *) lck;
    LeaveCriticalSection(lck);
}

int threads_equal(thread_t a, thread_t b) {
    return a.threadid == b.threadid;
}

unsigned long thread_int(thread_t a) {
    return (unsigned long)a.threadid;
}

void schedYield() {
    Sleep(1);
}

#else
#include <pthread.h>

/*Spawn a posix thread with pthread_create*/
thread_t spawnNewThread(void *initial_func, void *param) {
    static int initialized = 0;
    static pthread_attr_t attr;
    pthread_t new_thread;
    int result;

    if (!initialized) {
        initialized = 1;
        pthread_attr_init(&attr);
        pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    }
    
    result = pthread_create(&new_thread, &attr, 
                            (void*(*)(void*)) initial_func, 
                            param);
    if (result != 0) {
        return 0;
    }
    return (thread_t) new_thread;
}

void joinThread(thread_t threadid) {
    pthread_t p = (pthread_t) threadid;
    pthread_join(p, NULL);
}

void initThreads() {
}

void initLock(testlock_t *newlock) {
   pthread_mutex_init((pthread_mutex_t *) newlock, NULL);
}

void testLock(testlock_t *lck) {
   pthread_mutex_lock((pthread_mutex_t *) lck);
}

void testUnlock(testlock_t *lck) {
   pthread_mutex_unlock((pthread_mutex_t *) lck);
}

thread_t threadSelf() {
    return (thread_t) pthread_self();
}

int threads_equal(thread_t a, thread_t b) {
    return a == b;
}

unsigned long thread_int(thread_t a) {
    return (unsigned long)a;
}

void schedYield() {
    sched_yield();
}

#endif
