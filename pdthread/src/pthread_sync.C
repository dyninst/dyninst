#include "pthread_sync.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "../h/thread.h"

#define MAX_CONDS 512

pthread_sync::pthread_sync() {
    this->num_registered_conds = 0;
    this->conds = new (pthread_cond_t*)[MAX_CONDS];
    this->registered_conds = new (int)[MAX_CONDS]; 
    this->mutex = new pthread_mutex_t;
    pthread_mutex_init(mutex, NULL);
}

pthread_sync::~pthread_sync() {
    for (int i = 0; i < num_registered_conds ; i++) {
        pthread_cond_destroy(conds[registered_conds[i]]);
        delete conds[registered_conds[i]];
    }
    pthread_mutex_destroy(mutex);
    delete mutex;
    delete [] conds;
    delete [] registered_conds;
}

void pthread_sync::lock() {
    int status = 0;
    COLLECT_MEASUREMENT(THR_LOCK_ACQ);
    
#if LIBTHREAD_DEBUG == 1
        fprintf(stderr, "acquiring lock %p (by pthread %d)...\n", &mutex, pthread_self());
#endif
    status = pthread_mutex_trylock(mutex);

    if (status && errno == EBUSY) {
        COLLECT_MEASUREMENT(THR_LOCK_BLOCK);
        COLLECT_MEASUREMENT(THR_LOCK_TIMER_START);
        pthread_mutex_lock(mutex);
        COLLECT_MEASUREMENT(THR_LOCK_TIMER_STOP);
    }

#if LIBTHREAD_DEBUG == 1
        fprintf(stderr, "acquired lock %p!\n", &mutex);
#endif
}

void pthread_sync::unlock() {
#if LIBTHREAD_DEBUG == 1
        fprintf(stderr, "releasing lock %p...\n", &mutex);
#endif
        pthread_mutex_unlock(mutex);
#if LIBTHREAD_DEBUG == 1
        fprintf(stderr, "released lock %p!\n", &mutex);
#endif
}

void pthread_sync::register_cond(unsigned cond_num) {
    registered_conds[num_registered_conds++] = cond_num;
    conds[cond_num] = new pthread_cond_t; 
    pthread_cond_init(conds[cond_num], NULL);
}

void pthread_sync::signal(unsigned cond_num) {
    pthread_cond_signal(conds[cond_num]);
}

void pthread_sync::wait(unsigned cond_num) {
    pthread_cond_wait(conds[cond_num], mutex);
}

