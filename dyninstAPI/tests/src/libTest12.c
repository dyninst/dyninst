#include <stdio.h>
#include <stdlib.h>
#include <dyninstRTExport.h>
#include <test12.h>

/*
  A library of functions used in test12.  
*/

/*  reportMutexCreate
    inserted at mutex init points by the mutator to test the async user messaging 
    mode.
*/
unsigned int nextid = 0;

void reportMutexInit()
{
  user_msg_t msg;
  msg.id = nextid++;
  msg.what = mutex_init;
  msg.tid = (unsigned long) pthread_self();
  //fprintf(stderr, "%s[%d]:  reporting init, thread %lu\n", __FILE__, __LINE__, msg.tid);
  if (0 != DYNINSTuserMessage(&msg, sizeof(user_msg_t))) {
    fprintf(stderr, "%s[%d]:  DYNINSTuserMessage failed\n", __FILE__, __LINE__);
  }
}

/*  reportMuteDestroy()x
    inserted at mutex destroy points by the mutator to test the async user messaging 
    mode.
*/
void reportMutexDestroy()
{
  user_msg_t msg;
  msg.id = nextid++;
  msg.what = mutex_destroy;
  msg.tid = (unsigned long) pthread_self();
  //fprintf(stderr, "%s[%d]:  reporting destroy-%d: thread %lu\n", __FILE__, __LINE__, msg.what,msg.tid);
  if (0 != DYNINSTuserMessage(&msg, sizeof(user_msg_t))) {
    fprintf(stderr, "%s[%d]:  DYNINSTuserMessage failed\n", __FILE__, __LINE__);
  }
}

/*  reportMutexLock()
    inserted at mutex lock points by the mutator to test the async user messaging 
    mode.
*/
void reportMutexLock()
{
  user_msg_t msg;
  msg.id = nextid++;
  msg.what = mutex_lock;
  msg.tid = (unsigned long) pthread_self();
  //fprintf(stderr, "%s[%d]:  reporting lock-%d: thread %lu\n", __FILE__, __LINE__, msg.what, msg.tid);
  if (0 != DYNINSTuserMessage(&msg, sizeof(user_msg_t))) {
    fprintf(stderr, "%s[%d]:  DYNINSTuserMessage failed\n", __FILE__, __LINE__);
  }
}

/*  reportMuteUnlock()
    inserted at mutex unlock points by the mutator to test the async user messaging 
    mode.
*/
void reportMutexUnlock()
{
  user_msg_t msg;
  msg.id = nextid++;
  msg.what = mutex_unlock;
  msg.tid = (unsigned long) pthread_self();
  //fprintf(stderr, "%s[%d]:  reporting unlock-%d\n", __FILE__, __LINE__, msg.what);
  if (0 != DYNINSTuserMessage(&msg, sizeof(user_msg_t))) {
    fprintf(stderr, "%s[%d]:  DYNINSTuserMessage failed\n", __FILE__, __LINE__);
  }
}
