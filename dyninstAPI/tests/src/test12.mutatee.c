/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>

#include "test12.h"
#include "../../../dyninstAPI_RT/h/dyninstRTExport.h"
#define TRUE 1
#define FALSE 0

#if defined(i386_unknown_nt4_0)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

#if defined(sparc_sun_solaris2_4)  || defined(i386_unknown_solaris2_5) || \
    defined(i386_unknown_linux2_0) || defined(x86_64_unknown_linux2_4) || \
    defined(mips_sgi_irix6_4) || defined(alpha_dec_osf4_0) || \
    defined(rs6000_ibm_aix4_1) || defined(os_linux)
#include <dlfcn.h>
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

int runTest[MAX_TEST+1];
int passedTest[MAX_TEST+1];

#define dprintf if (debugPrint) printf
int debugPrint = 0;
int isAttached = 0;
int mutateeIdle = 0;
int mutateeXLC = 0;


/*
 * Stop the process (in order to wait for the mutator to finish what it's
 * doing and restart us).
 */
void stop_process()
{
#ifdef i386_unknown_nt4_0
    DebugBreak();
#else

#if !defined(bug_irix_broken_sigstop)
    kill(getpid(), SIGSTOP);
#else
    kill(getpid(), SIGEMT);
#endif

#endif
}

#if defined (os_windows)
#error
#else
typedef void *(*ThreadMain_t)(void *);
typedef pthread_t Thread_t;
typedef pthread_mutex_t Lock_t;
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
    fprintf(stderr, "%s[%d]:  could not alloc space for %d thread handles\n",
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

    if (0 != pthread_create(&(threads[i]), &attr, (void *(*)(void*))tmain, NULL)) {
      err = 1;
      fprintf(stderr, "%s[%d]:pthread_create\n", __FILE__, __LINE__);
      goto cleanup;
    }
    dprintf("%s[%d]:  PTHREAD_CREATE: %lu\n", __FILE__, __LINE__, threads[i]); 
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
    fprintf(stderr, "%s[%d]:  creating lock on thread %lu\n", __FILE__, __LINE__, (unsigned long) pthread_self());
  if (0 != pthread_mutex_init(lock, NULL)) {
    perror("pthread_mutex_init");
    return FALSE;
  }
  return TRUE;
}

int destroyLock(Lock_t *lock)
{
  if (debugPrint)
    fprintf(stderr, "%s[%d]:  destroying lock on thread %lu\n", __FILE__, __LINE__, (unsigned long) pthread_self());
  if (0 != pthread_mutex_destroy(lock)) {
    perror("pthread_mutex_destroy");
    return FALSE;
  }
  return TRUE;
}

int lockLock(Lock_t *lock)
{
  if (debugPrint)
    fprintf(stderr, "%s[%d]:  locking lock on thread %lu\n", __FILE__, __LINE__, (unsigned long) pthread_self());
  if (0 != pthread_mutex_lock(lock)) {
    perror("pthread_mutex_lock");
    return FALSE;
  }
  return TRUE;
}

int unlockLock(Lock_t *lock)
{
  if (debugPrint)
    fprintf(stderr, "%s[%d]:  unlocking lock on thread %lu\n", __FILE__, __LINE__, (unsigned long) pthread_self());
  if (0 != pthread_mutex_unlock(lock)) {
    perror("pthread_mutex_unlock");
    return FALSE;
  }
  return TRUE;
}

#endif

/*
 * Check to see if the mutator has attached to us.
 */
int checkIfAttached()
{
    return isAttached;
}

void sleep_ms(int _ms) 
{
  struct timespec ts,rem;
  ts.tv_sec = 0;
  ts.tv_nsec = _ms * 1000 /*us*/ * 1000/*ms*/;
  assert(_ms < 1000);

 sleep:
 if (0 != nanosleep(&ts, &rem)) {
    if (errno == EINTR) {
      dprintf("%s[%d]:  sleep interrupted\n", __FILE__, __LINE__);
      ts.tv_sec = rem.tv_sec;
      ts.tv_nsec = rem.tv_nsec;
      goto sleep;
    }
    assert(0);
  }

}

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
pthread_t test1threads[TEST1_THREADS];
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
    if (pthread_equal(test1threads[i],(pthread_t)id)) {
      found = 1;
      current_locks[i] = (unsigned)val;
      break;
    }
  }
  if (!found)
    fprintf(stderr, "%s[%d]: FIXME\n", __FILE__, __LINE__);
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
   int lockres =    (*DYNINSTlock_thelock)(&test1lock);

   register_my_lock((unsigned long)pthread_self(),1);
   pthread_mutex_lock(&real_lock);

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
void func1_1()
{

  dyntid_t (**DYNINST_pthread_self)(void);
  int lockres;
  int bigTIMEOUT;
  int timeout;

  /*pthread_attr_t attr;*/
  unsigned int i;
  void *RTlib;

  /* zero out lock registry: */
  for (i = 0; i < TEST1_THREADS; ++i) {
    current_locks[i] = 0;
  }

#if !defined (os_windows) && !defined(os_irix)

  RTlib = dlopen("libdyninstAPI_RT.so.1", RTLD_NOW);
  if (!RTlib) {
    fprintf(stderr, "%s[%d]:  could not open dyninst RT lib: %s\n", __FILE__, __LINE__, dlerror());
    exit(1);
  }

  DYNINSTinit_thelock = (void (*)(dyninst_lock_t *))dlsym(RTlib, "dyninst_init_lock");
  DYNINSTlock_thelock = (int (*)(dyninst_lock_t *))dlsym(RTlib, "dyninst_lock");
  DYNINSTunlock_thelock = (void (*)(dyninst_lock_t *))dlsym(RTlib, "dyninst_unlock");
  DYNINST_pthread_self = (dyntid_t (**)(void))dlsym(RTlib, "DYNINST_pthread_self");
  if (!DYNINSTinit_thelock) {
    fprintf(stderr, "%s[%d]:  could not DYNINSTinit_thelock: %s\n", __FILE__, __LINE__, dlerror());
    exit(1);
  }
  if (!DYNINSTlock_thelock) {
    fprintf(stderr, "%s[%d]:  could not DYNINSTlock_thelock: %s\n", __FILE__, __LINE__, dlerror());
    exit(1);
  }
  if (!DYNINSTunlock_thelock) {
    fprintf(stderr, "%s[%d]:  could not DYNINSTunlock_thelock:%s\n", __FILE__, __LINE__, dlerror());
    exit(1);
  }

  pthread_mutex_init(&real_lock, NULL);

  (*DYNINSTunlock_thelock)(&test1lock);
#if !defined(os_solaris)
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
#endif
  lockres = (*DYNINSTlock_thelock)(&test1lock);
  createThreads(TEST1_THREADS, thread_main1, test1threads);
  assert(test1threads);


  sleep_ms(5);

  dprintf("%s[%d]:  doing initial unlock...\n", __FILE__, __LINE__);
#if !defined(os_solaris)
  /* (*DYNINSTunlock_thelock)(&test1lock); */ 

#endif
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
  exit(subtest1err);
}

/********************************************************************/
/********************************************************************/
/***********  Subtest 2:  dynamic callsites */
/********************************************************************/
/********************************************************************/

#define NUM_DYN_CALLS 8 
typedef int (*intFuncArg) (int);
int call2_zero() {return 0;}

int call2_1(int arg) {return arg+1;}
int call2_2(int arg) {return arg+2;}
int call2_3(int arg) {return arg+3;}
int call2_4(int arg) {return arg+4;}

int call2_dispatch(intFuncArg callme, int arg) 
{
  /*fprintf(stderr, "%s[%d]:  inside call2_dispatch\n", __FILE__, __LINE__);*/
  static int callsite_selector = 0;
  int ret = -1;
  intFuncArg tocall = (intFuncArg) callme;

  ret = call2_zero(); /* lets have a non-dynamic call site here too */

  if (!tocall) {
    fprintf(stderr, "%s[%d]:  FIXME!\n", __FILE__, __LINE__);
    return -1;
  }

  /*  3 dynamic call sites */
  switch (callsite_selector) {
  case 0: ret = (tocall)(arg); callsite_selector++; break;
  case 1: ret = (tocall)(arg+1); callsite_selector++; break;
  case 2: ret = (tocall)(arg+2); callsite_selector = 0; break;
  }

  if (ret) 
    ret = call2_zero(); /* lets have a non-dynamic call site here too */

  return ret;

}

void func2_1()
{
#if !defined(arch_ia64)
  /*  want to trigger a lot of dynamic calls, and then stop the process. */
  /*  to make sure we test possible race in event handling. */

  int nextfunc = 1;
  unsigned int i;
  for (i = 0; i < NUM_DYN_CALLS; ++i) {
    switch (nextfunc) {
    case 1: call2_dispatch(call2_1, i); nextfunc++; break;
    case 2: call2_dispatch(call2_2, i); nextfunc++; break;
    case 3: call2_dispatch(call2_3, i); nextfunc++; break;
    case 4: call2_dispatch(call2_4, i); nextfunc = 1; break;
    }; 
  }
  
  mutateeIdle = 1;
  while (mutateeIdle);
  /*  stop the process (mutator will restart us) */
  /*stop_process(); */
#endif
}

Lock_t test3lock;

void *thread_main3(void *arg)
{
  int x, i;
  lockLock(&test3lock);
  x = 0;

  for (i = 0; i < 0xffff; ++i) {
    x = x + i;
  }
  /*fprintf(stderr, "%s[%d]:  PTHREAD_DESTROY\n", __FILE__, __LINE__); */

  unlockLock(&test3lock);
  dprintf("%s[%d]:  %lu exiting...\n", __FILE__, __LINE__, (unsigned long) pthread_self());
  return (void *) x;
}


Thread_t test3_threads[TEST3_THREADS];
void func3_1()
{

  createLock(&test3lock);
  mutateeIdle = 1;

  lockLock(&test3lock);
  assert (NULL != createThreads(TEST3_THREADS, thread_main3, test3_threads));

  sleep_ms(999);
  unlockLock(&test3lock);
  dprintf("%s[%d]:  func3_1\n", __FILE__, __LINE__);
  while (mutateeIdle) {}

  dprintf("%s[%d]:  leaving func3_1\n", __FILE__, __LINE__);
}

Lock_t test4lock;
void *thread_main4(void *arg)
{
  int x, i;
  lockLock(&test4lock); 
  x = 0;

  for (i = 0; i < 0xf; ++i) {
    x = x + i;
  }

  unlockLock(&test4lock); 
  dprintf("%s[%d]:  %lu exiting...\n", __FILE__, __LINE__, (unsigned long) pthread_self());
  return (void *) x;
}

Thread_t test4_threads[TEST3_THREADS];
void func4_1()
{
/*
#if defined(os_linux) && defined(arch_x86)
#else
*/
  dprintf("%s[%d]:  welcome to func4_1\n", __FILE__, __LINE__);
  createLock(&test4lock);


  lockLock(&test4lock); 
   
  assert (NULL != createThreads(TEST3_THREADS, thread_main4, test4_threads));

  unlockLock(&test4lock); 
  mutateeIdle = 1;
  while (mutateeIdle) {}
/*
#endif
*/

}

Lock_t test5lock;
void *thread_main5(void *arg)
{
  int x, i;
  lockLock(&test5lock); 
  x = 0;
  for (i = 0; i < 0xfffffff; ++i) {
    x = x + i;
  }

  unlockLock(&test5lock); 
  dprintf("%s[%d]:  %lu exiting...\n", __FILE__, __LINE__, (unsigned long) pthread_self());
  return (void *) x;
}

Thread_t test5_threads[TEST5_THREADS];

void func5_1()
{
#if defined(os_none)
  createLock(&test5lock);
  lockLock(&test5lock); 
  assert (NULL != createThreads(TEST5_THREADS, thread_main5, test5_threads));

  sleep_ms(999);
  unlockLock(&test5lock); 
  mutateeIdle = 1;
  while (mutateeIdle) {}
#else
#endif
}

Lock_t test6lock;
void *thread_main6(void *arg)
{
  int x, i;
  lockLock(&test6lock); 
  x = 0;
  for (i = 0; i < 0xf; ++i) {
    x = x + i;
  }

  unlockLock(&test6lock); 
  dprintf("%s[%d]:  %lu exiting...\n", __FILE__, __LINE__, (unsigned long) pthread_self());
  return (void *) x;
}

Thread_t test6_threads[TEST5_THREADS];

void func6_1()
{
#if defined(os_none)
  createLock(&test6lock);
  lockLock(&test6lock); 
  assert (NULL != createThreads(TEST6_THREADS, thread_main6, test6_threads));

  unlockLock(&test6lock); 
  mutateeIdle = 1;
  while (mutateeIdle) {}
#else
#endif
}

int call7_2(int x)
{
  int i;
  int y = x;
  for (i = 0; i < 0xfff; ++i) 
    y += i;
  
  return y;
}

int call7_1() 
{
  int x = 0;
  int z = 0;
  int i;
  for (i = 0; i < TEST7_NUMCALLS; ++i) {
    z += call7_2(x); 
  }
  return z;
}

void func7_1()
{
  /*  this is a simple single threaded scenario for user defined callback testing
      the entry, exit and callpoints of call7_1 are instrumented with messaging
      functions
  */
  int x = 0;
  x = call7_1();

  mutateeIdle = 1;
  while (mutateeIdle);

  /*free (threads);*/
}

void *thread_main8(void *arg)
{
  /*  The mutator will patch in messaging primitives to signal events at mutex creation,
      deletion, locking and unlocking.  Thus far, we are only considering reporting of events
      so actual contention is meaningless */
  Lock_t newmutex;
  if (!createLock(&newmutex)) {
     fprintf(stderr, "%s[%d]:  createLock failed\n", __FILE__, __LINE__);
     return NULL;
  }
  sleep_ms(100);
  if (!lockLock(&newmutex)) {
     fprintf(stderr, "%s[%d]:  lockLock failed\n", __FILE__, __LINE__);
     return NULL;
  }
  sleep_ms(100);
  if (!unlockLock(&newmutex)) {
     fprintf(stderr, "%s[%d]:  unlockLock failed\n", __FILE__, __LINE__);
     return NULL;
  }
  sleep_ms(100); 
  if (!destroyLock(&newmutex)) {
     fprintf(stderr, "%s[%d]:  destroyLock failed\n", __FILE__, __LINE__);
     return NULL;
  }

  sleep(1);
  return NULL;
}

Thread_t test8threads[TEST8_THREADS];
void func8_1()
{
  Thread_t *threads = test8threads;

  threads = createThreads(TEST8_THREADS, thread_main8,threads);
  assert (threads);

  mutateeIdle = 1;
  while (mutateeIdle);

  /*free (threads);*/
}


/********************************************************************/
/********************************************************************/
/********************************************************************/
#ifdef i386_unknown_nt4_0
#define USAGE "Usage: test12.mutatee [-attach] [-verbose] -run <num> .."
#else
#define USAGE "Usage: test12.mutatee [-attach <fd>] [-verbose] -run <num> .."
#endif

int main(int iargc, char *argv[])
{                                       /* despite different conventions */
    unsigned argc=(unsigned)iargc;      /* make argc consistently unsigned */
    unsigned int i, j;
#if !defined(i386_unknown_nt4_0)
    int pfd;
#endif
    int useAttach = FALSE;
    int doAttachCheck = TRUE;

    for (j=0; j <= MAX_TEST; j++) runTest[j] = FALSE;

    for (i=1; i < argc; i++) {
        if (!strcmp(argv[i], "-verbose")) {
            debugPrint = 1;
        } else if (!strcmp(argv[i], "-attach")) {
            useAttach = TRUE;
#ifndef i386_unknown_nt4_0
            if (++i >= argc) {
                printf("attach usage\n");
                fprintf(stderr, "%s\n", USAGE);
                exit(-1);
            }
            pfd = atoi(argv[i]);
#endif
        } else if (!strcmp(argv[i], "-attachrun")) {
            doAttachCheck = FALSE;
        } else if (!strcmp(argv[i], "-run")) {
            for (j=i+1; j < argc; j++) {
                unsigned int testId;
                if (argv[j] && isdigit(*argv[j]) && (testId = atoi(argv[j]))) {
                    if ((testId > 0) && (testId <= MAX_TEST)) {
                        dprintf("selecting test %d\n", testId);
                        runTest[testId] = TRUE;
                    } else {
                        printf("%s[%d]: invalid test %d requested\n", __FILE__, __LINE__, testId);
                        exit(-1);
                    }
                } else {
                    /* end of test list */
                    break;
                }
            }
            i = j-1;
       } else if (!strcmp(argv[i], "-runall")) {
          for (j=0; j <= MAX_TEST; j++) runTest[j] = TRUE;
       } else {
            printf("unexpected parameter '%s'\n", argv[i]);
            fprintf(stderr, "%s\n", USAGE);
            exit(-1);
        }
    }

    if ((argc==1) || debugPrint)
        printf("Mutatee %s [%s]:\"%s\"\n", argv[0],
                mutateeCplusplus ? "C++" : "C", Builder_id);
    if (argc==1) exit(0);

     /* set xlc flag if appropriate */
     mutateeXLC = 0;
     if (strstr(argv[0], "xlc") || strstr(argv[0], "xlC"))
        mutateeXLC = 1;

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
        if (doAttachCheck) {
          printf("Waiting for mutator to attach...\n"); fflush(stdout);
          while (!checkIfAttached()) ;
          printf("Mutator attached.  Mutatee continuing.\n");
        }
    }

    /*  test1 operates on a different "mode", ie the mutatee is started
        specially for test1, which executes it apart from the other tests. */
    if (runTest[2] || runTest[3] || 
        runTest[4] || runTest[5] ||
        runTest[6] || runTest[7] ||
        runTest[8]) 
          runTest[1] = FALSE;

    if (runTest[1]) func1_1();
    if (runTest[2]) func2_1();
    if (runTest[3]) func3_1();
    if (runTest[4]) func4_1();
    if (runTest[5]) func5_1();
    if (runTest[6]) func6_1();
    if (runTest[7]) func7_1();
    if (runTest[8]) func8_1();

#if 0
    while(1);
#endif

    return(0);
}

