#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

#if defined(i386_unknown_nt4_0)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#if defined(sparc_sun_solaris2_4)  || defined(i386_unknown_solaris2_5) || \
    defined(i386_unknown_linux2_0) || defined(mips_sgi_irix6_4) || \
    defined(alpha_dec_osf4_0) || defined(rs6000_ibm_aix4_1) || \
    defined(ia64_unknown_linux2_4)
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

#define MAX_TEST 5
int runTest[MAX_TEST+1];
int passedTest[MAX_TEST+1];

#define dprintf if (debugPrint) printf
int debugPrint = 1;
int isAttached = 0;
int mutateeIdle = 0;
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


/*
 * Check to see if the mutator has attached to us.
 */
int checkIfAttached()
{
    return isAttached;
}
/********************************************************************/
/********************************************************************/
/********************************************************************/
#define NUM_DYN_CALLS 8 
typedef int (*intFuncArg) (int);
int call1_zero() {return 0;}

int call1_1(int arg) {return arg+1;}
int call1_2(int arg) {return arg+2;}
int call1_3(int arg) {return arg+3;}
int call1_4(int arg) {return arg+4;}

int call1_dispatch(intFuncArg callme, int arg) 
{
  /*fprintf(stderr, "%s[%d]:  inside call1_dispatch\n", __FILE__, __LINE__);*/
  static int callsite_selector = 0;
  int ret = -1;
  intFuncArg tocall = (intFuncArg) callme;

  ret = call1_zero(); /* lets have a non-dynamic call site here too */

  if (!tocall) {
    fprintf(stderr, "%s[%d]:  FIXME!\n", __FILE__, __LINE__);
    return ret;
  }

  /*  3 dynamic call sites */
  switch (callsite_selector) {
  case 0: ret = (tocall)(arg); callsite_selector++; break;
  case 1: ret = (tocall)(arg+1); callsite_selector++; break;
  case 2: ret = (tocall)(arg+2); callsite_selector = 0; break;
  }

  if (ret) 
    ret = call1_zero(); /* lets have a non-dynamic call site here too */

  return ret;

}

void func1_1()
{
  /*  want to trigger a lot of dynamic calls, and then stop the process. */
  /*  to make sure we test possible race in event handling. */

  int nextfunc = 1;
  unsigned int i;
  for (i = 0; i < NUM_DYN_CALLS; ++i) {
    switch (nextfunc) {
    case 1: call1_dispatch(call1_1, i); nextfunc++; break;
    case 2: call1_dispatch(call1_2, i); nextfunc++; break;
    case 3: call1_dispatch(call1_3, i); nextfunc++; break;
    case 4: call1_dispatch(call1_4, i); nextfunc = 1; break;
    }; 
  }
  
  mutateeIdle = 1;
  while (mutateeIdle);
  /*  stop the process (mutator will restart us) */
  /*stop_process(); */

}

#define TEST2_THREADS 10
void *thread_main(void *arg)
{
  int x, i;
  x = 0;
  for (i = 0; i < 0xffffff; ++i) {
    x = x + i;
  }
  /*fprintf(stderr, "%s[%d]:  PTHREAD_DESTROY\n", __FILE__, __LINE__); */
  return (void *) x;
}

void func3_1()
{
  pthread_attr_t attr;
  pthread_t threads[TEST2_THREADS];
  unsigned int i;
  int tmp;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /*mutateeIdle = 1; */
  /*while (mutateeIdle); */


  /* start a bunch of threads */
  for (i = 0; i < TEST2_THREADS; ++i) {
    /*fprintf(stderr, "%s[%d]:  PTHREAD_CREATE\n", __FILE__, __LINE__); */
    pthread_create(&(threads[i]), &attr, thread_main, NULL);
  }

  mutateeIdle = 1;
  tmp = 1;
  while (tmp) {
    tmp = mutateeIdle;
  }


}

void func2_1()
{

}

void func4_1()
{
  pthread_attr_t attr;
  pthread_t threads[TEST2_THREADS];
  unsigned int i;
  int tmp;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /*mutateeIdle = 1; */
  /*while (mutateeIdle); */


  /* start a bunch of threads */
  for (i = 0; i < TEST2_THREADS; ++i) {
    /*fprintf(stderr, "%s[%d]:  PTHREAD_CREATE\n", __FILE__, __LINE__); */
    pthread_create(&(threads[i]), &attr, thread_main, NULL);
  }

  mutateeIdle = 1;
  tmp = 1;
  while (tmp) {
    tmp = mutateeIdle;
  }


}

void func5_1()
{

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
        printf("Waiting for mutator to attach...\n"); fflush(stdout);
        while (!checkIfAttached()) ;
        printf("Mutator attached.  Mutatee continuing.\n");
    }

    if (runTest[1]) func1_1();
    if (runTest[2]) func2_1();
    if (runTest[3]) func3_1();
    if (runTest[4]) func4_1();
    if (runTest[5]) func5_1();

    while(1);

    return(0);
}

