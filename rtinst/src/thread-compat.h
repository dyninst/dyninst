/*
 * Thread compatibility header file
 */

#ifndef _THREAD_COMPAT_
#define _THREAD_COMPAT

#if defined(rs6000_ibm_aix4_1)
#include <pthread.h>
typedef pthread_key_t                     dyninst_key_t;
typedef pthread_cond_t                    dyninst_cond_t;
typedef pthread_mutex_t                   dyninst_mutex_t;
typedef pthread_t                         dyninst_t;
typedef pthread_rwlock_t                  dyninst_rwlock_t;

#define P_thread_getspecific(key)         pthread_getspecific(key)
#define P_thread_setspecific(key, val)    pthread_setspecific(key,val)
#define P_thread_key_create(key,dest)     pthread_key_create(key,dest)
#define P_thread_self()                   pthread_self()
#define P_lwp_self()                      thread_self()
#endif

#if defined(sparc_sun_solaris2_4)
#include <thread.h>
typedef thread_key_t                     dyninst_key_t;
typedef cond_t                           dyninst_cond_t;
typedef mutex_t                          dyninst_mutex_t;
typedef rwlock_t                         dyninst_rwlock_t;
typedef thread_t                         dyninst_t;
typedef sema_t                           dyninst_sema_t;

extern void *P_thread_getspecific(dyninst_key_t);
#define P_thread_setspecific(key, val)    thr_setspecific(key,val)
#define P_thread_key_create(key,dest)     thr_keycreate(key,dest)
#define P_thread_self()                   pthread_self()
#define P_lwp_self()                      _lwp_self()

#endif

#endif
