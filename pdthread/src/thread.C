#include <stdlib.h>
#include <stdio.h>
#include "../h/thread.h"
#include "thrtab_entries.h"
#include "thrtab.h"
#include <assert.h>

#include <sys/time.h>

#if DO_DEBUG_LIBPDTHREAD_THREAD == 1
#define DO_DEBUG_LIBPDTHREAD 1
#else
#define DO_DEBUG_LIBPDTHREAD 0
#endif

#define CURRENT_FILE thread_C
#include "thr_debug.h"

#undef DO_DEBUG_LIBPDTHREAD

#if DO_LIBPDTHREAD_MEASUREMENTS

long long thr_get_vtime() {
    long long retval;
#if defined (i386_unknown_linux2_0)
#define THR_GET_VTIME_DEFINED 1
#warning libthread: no thr_get_vtime for linux yet; timer numbers will be nonsense
    // FIXME:  do rdtsc implementation
    static long long blah = 0;
    retval = blah++;
    
#endif

#if defined (sparc_sun_solaris2_4)
#define THR_GET_VTIME_DEFINED 1
    retval = gethrvtime();
#endif 

#if defined (rs6000_ibm_aix4_1)
#define THR_GET_VTIME_DEFINED 1
#warning libthread: no thr_get_vtime for aix yet; timer numbers will be nonsense
    // FIXME: do big perfctr implementation
    static long long blah = 0;
    retval = blah++;
#endif 

#ifndef THR_GET_VTIME_DEFINED
#warning libthread: no thr_get_vtime for your platform, please edit pdthread/src/thread.C and add one
#warning libthread: using lame gettimeofday() based timer; horribly inaccurate

    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    retval = (tv.tv_sec * 1000000) + tv.usec;
    
#endif

    return retval;
}

void thr_collect_measurement(int op) {
    thr_perf_data_t* perf_data;
    unsigned long long current_time;

    current_time = thr_get_vtime();

    perf_data = pthread_getspecific(lwp::perf_data_key);

    switch (op) {
        case THR_LOCK_ACQ:
            perf_data->num_lock_acquires++;
            break;
        case THR_LOCK_BLOCK:
            perf_data->num_lock_blocks++;
            break;
        case THR_LOCK_TIMER_START:
            perf_data->lock_timer_start = current_time;
            break;
        case THR_LOCK_TIMER_STOP:
            perf_data->lock_contention_time += (current_time - perf_data->lock_timer_start);
            break;
        case THR_MSG_SEND:
        case THR_MSG_RECV:            
        case THR_MSG_POLL:
            perf_data->num_msg_ops++;
            break;
        case THR_MSG_TIMER_START:
            perf_data->msg_timer_start = current_time;
            break;
        case THR_MSG_TIMER_STOP:
            perf_data->msg_time += (current_time - perf_data->msg_timer_start);
            break;
    }
}

#endif /* DO_LIBPDTHREAD_MEASUREMENTS */

item_t thr_type(thread_t tid) {
    thr_debug_msg(CURRENT_FUNCTION, "tid = %d\n", (unsigned)tid);
    if(!(thrtab::get_entry(tid))) 
        return item_t_unknown;
    return thrtab::get_entry(tid)->gettype();
}

PDDESC thr_file(thread_t tid) {
    thr_debug_msg(CURRENT_FUNCTION, "tid = %d\n", (unsigned)tid);
    entity* thrtab_entry = thrtab::get_entry(tid);
    if(!thrtab_entry || (thrtab_entry && thrtab_entry->gettype() != item_t_file))
        return INVALID_PDDESC;
    return ((file_q*)thrtab_entry)->fd;
}

PDSOCKET thr_socket(thread_t tid) {
    thr_debug_msg(CURRENT_FUNCTION, "tid = %d\n", (unsigned)tid);
    entity* thrtab_entry = thrtab::get_entry(tid);
    if (!thrtab_entry || (thrtab_entry && thrtab_entry->gettype() != item_t_socket))
        return INVALID_PDSOCKET;
    return ((socket_q*)thrtab_entry)->sock;
}

int thr_create(void* stack, unsigned stack_size, void* (*func)(void*), 
               void* arg, unsigned thr_flags, thread_t* tid) {
    // FIXME:  no error checking, no custom stack sizes, no starting suspended
    thr_debug_msg(CURRENT_FUNCTION, "stack = %p, stack_size = %d, func = %p, arg = %p, thr_flags = %d, tid = %p", stack, stack_size, func, arg, thr_flags, tid);
    *tid = thrtab::create_thread(func, arg, true);
    return THR_OKAY;
}

const char*	thr_name(const char* name) {
    const char* retval;
    thr_debug_msg(CURRENT_FUNCTION, "name (new) = \"%s\n", name);
    retval = lwp::name(name);
    thr_debug_msg(CURRENT_FUNCTION, "name (was) = \"%s\n", retval);
    return retval;
}


thread_t thr_self(void) {
    thread_t retval;
    retval = lwp::get_self();
    thr_debug_msg(CURRENT_FUNCTION, "returning %d", retval);
    return retval;
}

int thr_join(thread_t tid, thread_t* departed, void** result) {
    lwp* thread;
    thr_debug_msg(CURRENT_FUNCTION, "tid = %d", tid);
    if (tid == 0) {
        thread=(lwp*)thrtab::get_entry(thrtab::get_any_joinable());
    } else {
        thrtab::unregister_joinable(tid);
        thread=(lwp*)thrtab::get_entry(tid);
    }
    
    if(thread && thread->gettype() == item_t_thread)
        return thread->join(departed, result);
    else {
        return THR_ERR;
    }
    
};

void thr_yield(void) {
    // FIXME: currently a no-op; we can put a POSIX sched_yield in here and/or
    // use this function for suspend/resume functionality
    thr_debug_msg(CURRENT_FUNCTION, "");
}

static void thr_really_do_exit(lwp* the_thread, void* result) {
    thr_debug_msg(CURRENT_FUNCTION, "");    

    the_thread->stop_running();
    the_thread->complete();
    the_thread->set_returnval(result);
    
    thrtab::unregister_joinable(the_thread->self());

    pthread_exit(result);
}

void thr_exit(void* result) {
    thr_debug_msg(CURRENT_FUNCTION, "result = %p", result);
    thr_really_do_exit(lwp::get_lwp(), result);
}

int thr_keycreate(thread_key_t* keyp, void (*dtor)(void*)){
    thr_debug_msg(CURRENT_FUNCTION, "keyp = %p, dtor = %p", keyp, dtor);
    pthread_key_create(keyp, dtor);
    // FIXME: do error checking
    return THR_OKAY;
}

int thr_setspecific(thread_key_t key, void* data) {
    thr_debug_msg(CURRENT_FUNCTION, "key = %p, data = %p", key, data);
    pthread_setspecific(key,data);
    // FIXME: do error checking
    return THR_OKAY;
}

int thr_getspecific(thread_key_t key, void** datap) {
    thr_debug_msg(CURRENT_FUNCTION, "keyp = %d", key);
    *datap = pthread_getspecific(key);
    // FIXME: do error checking
    return THR_OKAY;
}

