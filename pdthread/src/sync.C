/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/* sync.C -- synchronization primitives */

#include "../h/thread.h"
#include "rwlock.h"
#include "xplat/Monitor.h"

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
    


