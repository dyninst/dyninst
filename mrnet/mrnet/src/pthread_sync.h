#ifndef SEEN_PTHREAD_SYNC
#define SEEN_PTHREAD_SYNC

#include<pthread.h>
#include<stdlib.h>

class pthread_sync {
  private:
    pthread_mutex_t *mutex;
    pthread_cond_t **conds;
    int *registered_conds;
    int num_registered_conds;
    
  public:
    pthread_sync();
    ~pthread_sync();
    void lock();
    void unlock();
    void register_cond(unsigned cond_num);
    void signal(unsigned cond_num);
    void wait(unsigned cond_num);
};

#endif
