
#ifndef __TEST12_H__
#define __TEST12_H__
#define TEST2_THREADS 10
#define TEST3_THREADS 10
#define TEST4_THREADS 10
#define TEST5_THREADS 10 
#define TEST6_THREADS 10
#define TEST7_THREADS 10

#define MAX_TEST 4
#define TIMEOUT 12000 /* ms */
#if defined (os_windows)
#error
#else
#define MUTEX_INIT_FUNC "pthread_mutex_init"
#define MUTEX_LOCK_FUNC "pthread_mutex_lock"
#define MUTEX_UNLOCK_FUNC "pthread_mutex_unlock"
#define MUTEX_DESTROY_FUNC "pthread_mutex_destroy"
#define TEST12_LIBNAME "./libTest12.so"
#endif
typedef enum {
   null_event = 3,
   mutex_init = 4,
   mutex_lock = 5,
   mutex_unlock = 6, 
   mutex_destroy = 7
} user_event_t;

typedef struct {
  unsigned int id;
  user_event_t what; 
  unsigned long tid;
} user_msg_t;

#endif
