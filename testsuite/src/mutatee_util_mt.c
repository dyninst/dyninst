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
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "mutatee_util.h"

#if defined(os_windows_test)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

thread_t spawnNewThread(void *initial_func, void *param) {
    thread_t newthr;
    newthr.hndl = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) initial_func,
                               param, 0, (LPDWORD) &(newthr.threadid));
    return newthr;
}

void* joinThread(thread_t threadid) {
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
    return NULL;
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

void initBarrier(testbarrier_t *barrier, unsigned int count) {
    //TODO
}

void waitTestBarrier(testbarrier_t *barrier) {
    //TODO
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

void *joinThread(thread_t threadid) {
   void *result;
   pthread_t p = (pthread_t) threadid;
   pthread_join(p, &result);
   return result;
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

#if defined(os_aix_test)
void initBarrier(testbarrier_t *barrier, unsigned int count) {
    assert(0); // XXX What to do if missing pthread_barrier_t?
}

void waitTestBarrier(testbarrier_t *barrier) {
    assert(0); // XXX What to do if missing pthread_barrier_t?
}
#else
void initBarrier(testbarrier_t *barrier, unsigned int count) {
    pthread_barrier_init((pthread_barrier_t *)barrier, NULL, count);
}

void waitTestBarrier(testbarrier_t *barrier) {
    pthread_barrier_wait((pthread_barrier_t *)barrier);
}
#endif

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

