/* sync.C -- synchronization primitives */

#include "../h/thread.h"
#include "rwlock.h"
#include "xplat/h/Monitor.h"

using namespace pdthr;

thread_monitor_t thr_monitor_create() {
    thread_monitor_t ret = NULL;
    
    XPlat::Monitor* newmon = new XPlat::Monitor;
    ret = (thread_monitor_t)newmon;
    return ret;
}

int thr_monitor_destroy(thread_monitor_t mon) {
    delete (XPlat::Monitor*)mon;
    return THR_OKAY;
}

int thr_monitor_enter(thread_monitor_t mon) {
    int lret = ((XPlat::Monitor*)mon)->Lock();
    return ((lret == 0) ? THR_OKAY : THR_ERR);
}

int thr_monitor_leave(thread_monitor_t mon) {
    int uret = ((XPlat::Monitor*)mon)->Unlock();
    return ((uret == 0) ? THR_OKAY : THR_ERR);
}

int thr_cond_register(thread_monitor_t mon, unsigned cond_no) {
    int rret = ((XPlat::Monitor*)mon)->RegisterCondition(cond_no);
    return ((rret == 0) ? THR_OKAY : THR_ERR);
}

int thr_cond_wait(thread_monitor_t mon, unsigned cond_no) {
    int wret = ((XPlat::Monitor*)mon)->WaitOnCondition(cond_no);
    return ((wret == 0) ? THR_OKAY : THR_ERR);
}

int thr_cond_signal(thread_monitor_t mon, unsigned cond_no) {
    int sret = ((XPlat::Monitor*)mon)->SignalCondition(cond_no);
    return ((sret == 0) ? THR_OKAY : THR_ERR);
}

int thr_cond_broadcast(thread_monitor_t mon, unsigned cond_no) {
    int bret = ((XPlat::Monitor*)mon)->BroadcastCondition(cond_no);
    return ((bret == 0) ? THR_OKAY : THR_ERR);
}


// rwlock-related functions
thread_rwlock_t thr_rwlock_create(pref_t pref) {
    return new rwlock(pref == rwlock_favor_read ? rwlock::favor_readers : rwlock::favor_writers);
}

int thr_rwlock_destroy(thread_rwlock_t rw) {
    rwlock* realrw = (rwlock*)rw;
    delete realrw;
    return THR_OKAY;
}

int thr_rwlock_acquire(thread_rwlock_t rw, action_t action) {
    rwlock* realrw = (rwlock*)rw;
    realrw->acquire(action == action_read ? rwlock::read : rwlock::write);
    return THR_OKAY;
}

int thr_rwlock_release(thread_rwlock_t rw, action_t action) {
    rwlock* realrw = (rwlock*)rw;
    realrw->release(action == action_read ? rwlock::read : rwlock::write);
    return THR_OKAY;
}
    


