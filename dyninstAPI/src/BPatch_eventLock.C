#include "BPatch_eventLock.h"
//#define DEBUG_MUTEX 1
MUTEX_TYPE global_mutex;
bool mutex_created = false;
#ifdef DEBUG_MUTEX
int lock_depth = 0;
#endif

BPatch_eventLock::BPatch_eventLock() 
{
  if (mutex_created) return;
#if defined(os_windows)
  InitializeCriticalSection(&global_mutex);
#else
  pthread_mutexattr_t mutex_type;
  pthread_mutexattr_init(&mutex_type);
#if defined(arch_ia64)
  try {
    pthread_mutexattr_settype(&mutex_type, PTHREAD_MUTEX_RECURSIVE_NP);
  }catch (...) {
    fprintf(stderr, "%s[%d]: exception %d\n", __FILE__, __LINE__);
  }
  try {
  pthread_mutex_init(&global_mutex, &mutex_type);
  }catch (...) {
    fprintf(stderr, "%s[%d]: exception%d\n", __FILE__, __LINE__);
  }
#else
  pthread_mutexattr_settype(&mutex_type, PTHREAD_MUTEX_TYPE);
  pthread_mutex_init(&global_mutex, &mutex_type);
#endif
#endif // !Windows
  mutex_created = true;
}

#ifdef NOTDEF
bool BPatch_eventLock::init() 
{
  if (mutex_created) return;
#if defined(os_windows)
  InitializeCriticalSection(&global_mutex);
#else
  pthread_mutexattr_t mutex_type;
  pthread_mutexattr_init(&mutex_type);
#if defined(arch_ia64)
  if (0 != pthread_mutexattr_setkind_np(&mutex_type, PTHREAD_MUTEX_TYPE))
    return false;
#else
  if (0 != pthread_mutexattr_settype(&mutex_type, PTHREAD_MUTEX_TYPE))
    return false;
#endif
  if (0 != pthread_mutex_init(&global_mutex, &mutex_type))
    return false;
#endif // !Windows
  mutex_created = true;
  return true;
}
#endif
BPatch_eventLock::~BPatch_eventLock() {};

#if defined(os_windows)

int BPatch_eventLock::_Lock(const char *__file__, unsigned int __line__) const
{
  EnterCriticalSection(&global_mutex);
  return 0;
}

int BPatch_eventLock::_Trylock(const char *__file__, unsigned int __line__) const
{
  TryEnterCriticalSection(&global_mutex);
  return 0;
}

int BPatch_eventLock::_Unlock(const char *__file__, unsigned int __line__) const
{
  LeaveCriticalSection(&global_mutex);
  return 0;
}

#else
int BPatch_eventLock::_Lock(const char *__file__, unsigned int __line__) const
{
  int err = 0;
  if(0 != (err = pthread_mutex_lock(&global_mutex))){
    ERROR_BUFFER;
    fprintf(stderr, "%s[%d]:  failed to lock mutex: %s[%d]\n",
            __file__, __line__, STRERROR(err, buf), err);
  }
#ifdef DEBUG_MUTEX
  lock_depth++;
  fprintf(stderr, "%s[%d]:  lock, depth = %d\n", __file__, __line__, lock_depth);
#endif
  return err;
}

int BPatch_eventLock::_Trylock(const char *__file__, unsigned int __line__) const
{
  int err = 0;
  if(0 != (err = pthread_mutex_trylock(&global_mutex))){
    if (EBUSY != err) {
      ERROR_BUFFER;
      //  trylock returns EBUSY immediately when lock cannot be obtained
      fprintf(stderr, "%s[%d]:  failed to trylock mutex: %s[%d]\n",
              __file__, __line__, STRERROR(err, buf), err);
    }
  }
  return err;
}

int BPatch_eventLock::_Unlock(const char *__file__, unsigned int __line__) const
{
  int err = 0;
#ifdef DEBUG_MUTEX
  lock_depth--;
  fprintf(stderr, "%s[%d]:  unlock, lock depth will be %d \n", __file__, __line__, lock_depth);
#endif
  if(0 != (err = pthread_mutex_unlock(&global_mutex))){
    ERROR_BUFFER;
    fprintf(stderr, "%s[%d]:  failed to unlock mutex: %s[%d]\n",
            __file__, __line__, STRERROR(err, buf), err);
  }
  return err;
}

#endif


