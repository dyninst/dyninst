
#ifndef __TEST12_H__
#define __TEST12_H__

#include "test_lib_test9.h"

#define FAIL_MES(x,y) logerror("**Failed %s (%s)\n", x,y);
#define PASS_MES(x,y) logerror("Passed %s (%s)\n", x,y);
#define SKIP(x,y) logerror("Skipped %s (%s)\n", x,y);

#define TEST1_THREADS 10
#define TEST3_THREADS 10
#define TEST4_THREADS 10
#define TEST5_THREADS 10 
#define TEST6_THREADS 10
#define TEST7_THREADS 10
#define TEST8_THREADS 10


#define TEST7_NUMCALLS 10 /* number of callpoint messages we expect in subetst7 */
#define MAX_TEST 8 
#define TIMEOUT 15000 /* ms */
#define SLEEP_INTERVAL 100 /*100 ms*/
#if defined (os_windows)
#error
#else
#define MUTEX_INIT_FUNC "pthread_mutex_init"
#define MUTEX_LOCK_FUNC "pthread_mutex_lock"
#define MUTEX_UNLOCK_FUNC "pthread_mutex_unlock"
#define MUTEX_DESTROY_FUNC "pthread_mutex_destroy"
#endif
typedef enum {
   null_event = 3,
   mutex_init = 4,
   mutex_lock = 5,
   mutex_unlock = 6, 
   mutex_destroy = 7,
   func_entry = 8,
   func_callsite = 9,
   func_exit = 10,
   test3_event1,
   test3_event2,
   test3_event3
} user_event_t;

#if defined(m32_test)
typedef struct {
  unsigned int id;
  user_event_t what; 
  unsigned long long int tid;
} user_msg_t;
#else
typedef struct {
  unsigned int id;
  user_event_t what; 
  unsigned long tid;
} user_msg_t;
#endif

#endif
