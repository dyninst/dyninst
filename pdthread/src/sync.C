/* sync.C -- synchronization primitives */

#include "../h/thread.h"
#include "pthread_sync.h"
#include "rwlock.h"

int thr_monitor_create(thread_monitor_t* mon) {
    *mon = new pthread_sync;
    return THR_OKAY;
}

int thr_monitor_destroy(thread_monitor_t* mon) {
    delete *mon;
    return THR_OKAY;
}

int thr_monitor_enter(thread_monitor_t* mon) {
    (*mon)->lock();
    return THR_OKAY;
}

int thr_monitor_leave(thread_monitor_t* mon) {
    (*mon)->unlock();
    return THR_OKAY;
}

int thr_cond_register(thread_monitor_t* mon, unsigned cond_no) {
    (*mon)->register_cond(cond_no);
    return THR_OKAY;
}

int thr_cond_wait(thread_monitor_t* mon, unsigned cond_no) {
    (*mon)->wait(cond_no);
    return THR_OKAY;
}

int thr_cond_signal(thread_monitor_t* mon, unsigned cond_no) {
    (*mon)->signal(cond_no);
    return THR_OKAY;
}

int thr_rwlock_create(thread_rwlock_t* rw, pref_t pref) {
    *rw = new rwlock(pref == rwlock_favor_read ? rwlock::favor_readers : rwlock::favor_writers);
    return THR_OKAY;
}

int thr_rwlock_destroy(thread_rwlock_t* rw) {
    delete *rw;
    return THR_OKAY;
}

int thr_rwlock_acquire(thread_rwlock_t* rw, action_t action) {
    (*rw)->acquire(action == action_read ? rwlock::read : rwlock::write);
    return THR_OKAY;
}

int thr_rwlock_release(thread_rwlock_t* rw, action_t action) {
    (*rw)->release(action == action_read ? rwlock::read : rwlock::write);
    return THR_OKAY;
}
    


