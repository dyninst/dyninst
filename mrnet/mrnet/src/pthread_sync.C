#include "pthread_sync.h"
#include<pthread.h>
#include<stdlib.h>
#include <stdio.h>

namespace MRN
{

#define MAX_CONDS 512

pthread_sync::pthread_sync() {
    this->num_registered_conds = 0;
    this->conds = new pthread_cond_t*[MAX_CONDS];
    this->registered_conds = new int[MAX_CONDS]; 
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


} // namespace MRN

