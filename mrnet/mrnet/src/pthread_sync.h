#ifndef SEEN_PTHREAD_SYNC
#define SEEN_PTHREAD_SYNC

#include<pthread.h>
#include<stdlib.h>

namespace MRN
{

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

inline void pthread_sync::lock() {
    pthread_mutex_lock(mutex);
}

inline void pthread_sync::unlock() {
    pthread_mutex_unlock(mutex);
}

inline void pthread_sync::register_cond(unsigned cond_num) {
    registered_conds[num_registered_conds++] = cond_num;
    conds[cond_num] = new pthread_cond_t; 
    pthread_cond_init(conds[cond_num], NULL);
}

inline void pthread_sync::signal(unsigned cond_num) {
    pthread_cond_signal(conds[cond_num]);
}

inline void pthread_sync::wait(unsigned cond_num) {
    pthread_cond_wait(conds[cond_num], mutex);
}

} // namespace MRN

#endif
