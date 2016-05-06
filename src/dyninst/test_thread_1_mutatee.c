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
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"
#include "test_thread.h"
#include "test12.h"
#include "dyninstRTExport.h"

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

/********************************************************************/
/********************************************************************/
/***********  Subtest 1:  rtlib spinlocks */
/***********  use dlopen/dlsym to get access to rt lib lock */
/***********  then start up a bunch of threads to contend for it */
/***********  monitor contention for deadlock/broken lock */
/********************************************************************/
/********************************************************************/

unsigned long current_locks[TEST1_THREADS];
/*Thread_t  *test2threads; */
Thread_t test1threads[TEST1_THREADS];
pthread_mutex_t real_lock;

int subtest1counter = 0;
int subtest1err = 0;

void register_my_lock(unsigned long id, unsigned int val)
{
  unsigned int i;
  int found = 0;
  dprintf("%s[%d]:  %sregister lock for thread %lu\n", __FILE__, __LINE__,
           val ? "" : "un", id);
  for (i = 0; i < TEST1_THREADS; ++i) {
    if (pthread_equal((pthread_t)test1threads[i],(pthread_t)id)) {
      found = 1;
      current_locks[i] = (unsigned)val;
      break;
    }
  }
  if (!found)
    logerror("%s[%d]: FIXME\n", __FILE__, __LINE__);
}

int done_threads = 0;

int all_threads_done()
{
  return done_threads == TEST1_THREADS;
}

int is_only_one() {
  unsigned int i;
  int foundone = 0;
  for (i = 0; i < TEST1_THREADS; ++i) {
    if (0 != current_locks[i]) {
      if (foundone) return 0; /*false*/
      foundone++;
    }
  }
  return 1; /*true */
}

void (*DYNINSTinit_thelock)(dyninst_lock_t *);
int (*DYNINSTlock_thelock)(dyninst_lock_t *);
void (*DYNINSTunlock_thelock)(dyninst_lock_t *);
/*dyninst_lock_t test1lock; */
static DECLARE_DYNINST_LOCK(test1lock);

void *thread_main1 (void *arg)
{
   (*DYNINSTlock_thelock)(&test1lock);
   register_my_lock((unsigned long)pthread_self(),1);
   pthread_mutex_lock(&real_lock);
   arg = NULL; /*Silence warnings*/

  /*sleep_ms(1); */

   if (!is_only_one()) {
     subtest1err = 1;
   }
   pthread_mutex_unlock(&real_lock);
   register_my_lock((unsigned long)pthread_self(),0);
   subtest1counter++;

   (*DYNINSTunlock_thelock)(&test1lock); 

   pthread_mutex_lock(&real_lock);
    done_threads++;
   pthread_mutex_unlock(&real_lock);
   return NULL;
}

unsigned long local_pthread_self() {
  return (unsigned long) pthread_self();
}

int func1_1()
{

  dyntid_t (**DYNINST_pthread_self)(void);
  int lockres;
  int bigTIMEOUT;
  int timeout;
  const char *libname;
  /*pthread_attr_t attr;*/
  unsigned int i;
  void *RTlib;

  /* zero out lock registry: */
  for (i = 0; i < TEST1_THREADS; ++i) {
    current_locks[i] = 0;
  }

#if !defined (os_windows_test) && !defined(os_irix)

#if defined(m32_test)
  libname = "libdyninstAPI_RT_m32.so";
#else
  libname = "libdyninstAPI_RT.so";
#endif
  RTlib = dlopen(libname, RTLD_NOW);
  if (!RTlib) {
    logerror("%s[%d]:  could not open dyninst RT lib: %s\n", __FILE__, __LINE__, dlerror());
    char *ld = getenv("LD_LIBRARY_PATH");
    logerror("%s[%d]:  with LD_LIBRARY_PATH of: %s\n", __FILE__, __LINE__, (ld ? ld : "<NULL>"));
    return -1;
  }

  DYNINSTinit_thelock = (void (*)(dyninst_lock_t *))dlsym(RTlib, "dyninst_init_lock");
  DYNINSTlock_thelock = (int (*)(dyninst_lock_t *))dlsym(RTlib, "dyninst_lock");
  DYNINSTunlock_thelock = (void (*)(dyninst_lock_t *))dlsym(RTlib, "dyninst_unlock");
  DYNINST_pthread_self = (dyntid_t (**)(void))dlsym(RTlib, "DYNINST_pthread_self");
  if (!DYNINSTinit_thelock) {
    logerror("%s[%d]:  could not DYNINSTinit_thelock: %s\n", __FILE__, __LINE__, dlerror());
    /* FIXME Don't exit()! */
    /* exit(1); */
    return -1;
  }
  if (!DYNINSTlock_thelock) {
    logerror("%s[%d]:  could not DYNINSTlock_thelock: %s\n", __FILE__, __LINE__, dlerror());
    /* FIXME Don't exit()! */
    /* exit(1); */
    return -1;
  }
  if (!DYNINSTunlock_thelock) {
    logerror("%s[%d]:  could not DYNINSTunlock_thelock:%s\n", __FILE__, __LINE__, dlerror());
    /* FIXME Don't exit()! */
    /* exit(1); */
    return -1;
  }

  pthread_mutex_init(&real_lock, NULL);

  (*DYNINSTunlock_thelock)(&test1lock);
   /*  XXX this is nasty */
   /*  The way this is supposed to work is that we get a lock, then start a bunch of
       threads, which all try to get the same lock, pretty much as soon as they start.
       Then, after starting all the threads, we release the lock and let the threads
       compete for it, checking to make sure that all threads get the lock at some point
       and that no two threads have it at the same time.  
       The problem is that solaris is having problems with this system when the lock is 
       obtained before the threads are spawned (pthread_create hangs) -- it is still ok
       to just start all the threads and have the system run, its just not quite as clean.
       This might be bad asm programming on my behalf, or it might be some idiosyncracy
       with solaris libpthreads.  This worked, incidentally, when this stuff was all in
       the mutator, but that might've been because the asm that was imported to implement
       the locks was the gnu asm, not the solaris-cc asm, which is the stuff that gets
       compiled, by default into the runtime lib*/
/*
   int lockres = (*DYNINSTlock_thelock)(&test1lock); 
*/
  lockres = (*DYNINSTlock_thelock)(&test1lock);
  createThreads(TEST1_THREADS, thread_main1, test1threads);

  sleep_ms(5);

  dprintf("%s[%d]:  doing initial unlock...\n", __FILE__, __LINE__);
  /* (*DYNINSTunlock_thelock)(&test1lock); */

   (*DYNINSTunlock_thelock)(&test1lock);
  /*pthread_mutex_unlock(&real_lock); */

#endif

  bigTIMEOUT = 5000;
  timeout = 0;

  /*   wait for all threads to exit */
  while (timeout < bigTIMEOUT && ! all_threads_done()) {
    timeout += 100;
    sleep_ms(100);
  }

  dlclose(RTlib);
  return subtest1err;
}

/* skeleton test doesn't do anything besides say that it passed */
int test_thread_1_mutatee() {
  int status;

  status = func1_1();
  /* TODO Make sure this is correct */
  if (status != 0) {
    return -1; /* Error of some kind */
  } else {
    test_passes(testname);
    return 0;
  }
}
