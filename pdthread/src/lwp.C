#include <pthread.h>
#include <stdlib.h>
#include "mailbox.h"
#include "thrtab.h"
#include "thrtab_entries.h"

pthread_key_t lwp::tid_key;
pthread_key_t lwp::lwp_key;
pthread_key_t lwp::name_key;
pthread_key_t lwp::mailbox_key;
pthread_once_t lwp::keys_once;
lwp* lwp::main_thr = NULL;



void lwp::initialize_tsd_keys() {
    pthread_key_create(&lwp::lwp_key, NULL);
    pthread_key_create(&lwp::tid_key, NULL);
    pthread_key_create(&lwp::name_key, NULL);
    pthread_key_create(&lwp::mailbox_key, NULL);
}



void* lwp::wrapper_func(void* arg) {
    lwp* the_thread = (lwp*)arg;
    void* result;
    lwp::task_t task = the_thread->_task;
    lwp::value_t taskarg = the_thread->_arg;

    lwp::set_tsd_keys_for(the_thread);

    thrtab::register_joinable(the_thread->_self);

    the_thread->start_running();

    result = task(taskarg);

    // NB: this is only reached if task returns via "return" or via
    // falling off of the end of the function
    thr_exit(result);    
    
    // not reached; present for compiler placation
    return 0;
}



void lwp::set_tsd_keys_for(lwp* whom) {
    pthread_once(&lwp::keys_once, lwp::initialize_tsd_keys);
    pthread_setspecific(lwp::lwp_key, (void*)whom);
    pthread_setspecific(lwp::tid_key, (void*)(&whom->_self));
    pthread_setspecific(lwp::mailbox_key, (void*)whom->_mail);
}



void lwp::init(mailbox *my_mailbox, lwp::task_t func, lwp::value_t arg) {
    _mail = my_mailbox;
    _task = func;
    _arg = arg;
    if(_self == 1) {
        // main thread
        lwp::set_tsd_keys_for(this);
    }
    
}



void lwp::start() {
    // FIXME:  we really ought to handle non-default stacks 
    pthread_create(&_pself, NULL, lwp::wrapper_func, this);
    // FIXME:  check for error status
}



int lwp::join(thread_t* departed, void** return_val) {
    int retval = THR_OKAY;
    
    _joined_lk.acquire(rwlock::write);
    _reaped_lk.acquire(rwlock::write);
    
    if(_joined || _reaped) {
        retval = THR_ERR;
    } else {
        _joined = 1;
        pthread_join(_pself, return_val);
        // FIXME: check result of this function
        _reaped = 1;
        if(departed)
            *departed = _self;
    }

    _reaped_lk.release(rwlock::write);
    _joined_lk.release(rwlock::write);
    
    return retval;
}


    
lwp* lwp::get_lwp() {
    return (lwp*)pthread_getspecific(lwp::lwp_key);

}



lwp* lwp::get_main(thread_t tid=1) {
    if(lwp::main_thr != NULL) {
        return lwp::main_thr;
    } else {
        lwp::main_thr = new lwp();
        return lwp::main_thr;
    }
}



const char* lwp::name(const char* new_name) {
    const char* retval = (const char*)pthread_getspecific(lwp::name_key);
    if(new_name)
        pthread_setspecific(lwp::name_key, new_name);
    return retval;
}



    
thread_t lwp::get_self() {
    thread_t *self = (thread_t*)pthread_getspecific(lwp::tid_key);
    if(!self)
        return 0;
    else
        return *self;
}



mailbox* lwp::get_mailbox() {
    return (mailbox*)pthread_getspecific(lwp::mailbox_key);
}



/* miscellaneous synchronized accessors */

void lwp::set_returnval(void* result) {
    _returnval = result;
}

unsigned lwp::is_running() {
    unsigned retval;
    
    _running_lk.acquire(rwlock::read);
    retval = _running;
    _running_lk.release(rwlock::read);
    
    return retval;
}

unsigned lwp::is_completed() {
    unsigned retval;
    
    _completed_lk.acquire(rwlock::read);
    retval = _completed;
    _completed_lk.release(rwlock::read);
    
    return retval;
}

void lwp::start_running() {
    _running_lk.acquire(rwlock::write);
    _running = 1;
    _running_lk.release(rwlock::write);
}

void lwp::stop_running() {
    _running_lk.acquire(rwlock::write);
    _running = 0;
    _running_lk.release(rwlock::write);
}

void lwp::complete() {
    _completed_lk.acquire(rwlock::write);
    _completed = 1;
    _completed_lk.release(rwlock::write);
}
