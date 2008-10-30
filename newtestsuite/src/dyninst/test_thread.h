#ifndef TEST_THREAD_H
#define TEST_THREAD_H

#include <pthread.h>

void sleep_ms(int ms);

#if defined (os_windows)
#error
#else
typedef void *(*ThreadMain_t)(void *);
#if defined(m32_test)
typedef unsigned long long Thread_t;
#else
typedef pthread_t Thread_t;
#endif
typedef pthread_mutex_t Lock_t;

extern pthread_attr_t attr;

Thread_t *createThreads(unsigned int num, ThreadMain_t tmain, Thread_t *tbuf);
int createLock(Lock_t *lock);
int destroyLock(Lock_t *lock);
int lockLock(Lock_t *lock);
int unlockLock(Lock_t *lock);

#endif /* !defined(os_windows) */


#endif /* !defined(TEST_THREAD_H) */
