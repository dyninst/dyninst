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
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "mutatee_util.h"
#include "test_thread.h"

void sleep_ms(int ms) 
{
  struct timespec ts,rem;
  ts.tv_sec = 0;
  ts.tv_nsec = ms * 1000 /*us*/ * 1000/*ms*/;
  assert(ms < 1000);

 sleep:
 if (0 != nanosleep(&ts, &rem)) {
    if (errno == EINTR) {
      dprintf("%s[%d]:  sleep interrupted\n", __FILE__, __LINE__);
      ts.tv_sec = rem.tv_sec;
      ts.tv_nsec = rem.tv_nsec;
      goto sleep;
    }
    sleep(1);
 }

}

#if defined (os_windows_test)
#error
#else
  /*
    createThreads()
    createThreads creates specified number of threads and returns
    a pointer to an allocated buffer that contains their handles
    caller is responsible for free'ing the result
  */

pthread_attr_t attr;

Thread_t *createThreads(unsigned int num, ThreadMain_t tmain, Thread_t *tbuf)
{
  unsigned int i;
  int err = 0;
  Thread_t *threads;
  if (tbuf == NULL)
    threads = (Thread_t *)malloc(num * sizeof(Thread_t));
  else 
    threads = tbuf;
    
  if (!threads) {
    logerror("%s[%d]:  could not alloc space for %d thread handles\n",
            __FILE__, __LINE__, num);
    return NULL;
  }

  if (0 != pthread_attr_init(&attr)) {
    err = 1;
    perror("pthread_attr_init");
    goto cleanup;
  }

  if (0 != pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED)) {
    err = 1;
    perror("pthread_attr_setdetachstate");
    goto cleanup;
  }

  if (0 != pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM)) {
    err = 1;
    perror("pthread_attr_setscope");
    goto cleanup;
  }

  /* start a bunch of threads */
  for (i = 0; i < num; ++i) {

    if (0 != pthread_create((pthread_t *) &threads[i], 
	                    &attr, (void *(*)(void*))tmain, NULL))    
    {
      err = 1;
      logerror("%s[%d]:pthread_create - %s\n", __FILE__, __LINE__, strerror(errno));
      goto cleanup;
    }
    dprintf("%s[%d]:  PTHREAD_CREATE: %lu\n", __FILE__, __LINE__, 
	    (unsigned long)threads[i]); 
  }

  cleanup:

  if (err) {
    if (!tbuf) free (threads);
    return NULL;
  }

  return threads;
}

int createLock(Lock_t *lock)
{
  if (debugPrint)
    dprintf("%s[%d]:  creating lock on thread %lu\n", __FILE__, __LINE__, (unsigned long) pthread_self());
  if (0 != pthread_mutex_init(lock, NULL)) {
    perror("pthread_mutex_init");
    return FALSE;
  }
  return TRUE;
}

int destroyLock(Lock_t *lock)
{
  if (debugPrint)
    dprintf("%s[%d]:  destroying lock on thread %lu\n", __FILE__, __LINE__, (unsigned long) pthread_self());
  if (0 != pthread_mutex_destroy(lock)) {
    perror("pthread_mutex_destroy");
    return FALSE;
  }
  return TRUE;
}

int lockLock(Lock_t *lock)
{
  if (debugPrint)
    dprintf("%s[%d]:  locking lock on thread %lu\n", __FILE__, __LINE__, (unsigned long) pthread_self());
  if (0 != pthread_mutex_lock(lock)) {
    perror("pthread_mutex_lock");
    return FALSE;
  }
  return TRUE;
}

int unlockLock(Lock_t *lock)
{
  if (debugPrint)
    dprintf("%s[%d]:  unlocking lock on thread %lu\n", __FILE__, __LINE__, (unsigned long) pthread_self());
  if (0 != pthread_mutex_unlock(lock)) {
    perror("pthread_mutex_unlock");
    return FALSE;
  }
  return TRUE;
}

#endif
