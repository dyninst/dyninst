#include <stdlib.h>
#include <stdio.h>
#include "../h/thread.h"
#include "thrtab_entries.h"
#include "thrtab.h"
#include <assert.h>

item_t thr_type(thread_t tid) {
    assert(thrtab::get_entry(tid));    
    return thrtab::get_entry(tid)->gettype();
}

PDDESC thr_file(thread_t tid) {
    entity* thrtab_entry = thrtab::get_entry(tid);
    assert(thrtab_entry && thrtab_entry->gettype() == item_t_file);
    return ((file_q*)thrtab_entry)->fd;
}

PDSOCKET thr_socket(thread_t tid) {
    entity* thrtab_entry = thrtab::get_entry(tid);
    assert(thrtab_entry && thrtab_entry->gettype() == item_t_socket);
    return ((socket_q*)thrtab_entry)->sock;
}

int thr_create(void* stack, unsigned stack_size, void* (*func)(void*), 
               void* arg, unsigned thr_flags, thread_t* tid) {
    // FIXME:  no error checking, no custom stack sizes, no starting suspended
    *tid = thrtab::create_thread(func, arg, true);
    return THR_OKAY;
}

const char*	thr_name(const char* name) {
    return (const char*)0;
}


thread_t thr_self(void) {
    return lwp::get_self();
}

int thr_join(thread_t tid, thread_t* departed, void** result) {
    lwp* thread;
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
}

static void thr_really_do_exit(lwp* the_thread, void* result) {
    
    the_thread->stop_running();
    the_thread->complete();
    the_thread->set_returnval(result);
    
    thrtab::unregister_joinable(the_thread->self());

    pthread_exit(result);
}

void thr_exit(void* result) {
    thr_really_do_exit(lwp::get_lwp(), result);
}

int thr_keycreate(thread_key_t* keyp, void (*dtor)(void*)) {
    pthread_key_create(keyp, dtor);
    // FIXME: do error checking
    return THR_OKAY;
}

int thr_setspecific(thread_key_t key, void* data) {
    pthread_setspecific(key,data);
    // FIXME: do error checking
    return THR_OKAY;
}

int thr_getspecific(thread_key_t key, void** datap) {
    *datap = pthread_getspecific(key);
    // FIXME: do error checking
    return THR_OKAY;
}

