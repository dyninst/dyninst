#include <stdlib.h>
#include "mailbox.h"
#include "thrtab.h"
#include "thrtab_entries.h"
#include "lwp.h"
#include "xplat/h/TLSKey.h"

namespace pdthr
{

XPlat::TLSKey* lwp::tid_key = NULL;
XPlat::TLSKey* lwp::lwp_key = NULL;
XPlat::TLSKey* lwp::name_key = NULL;
XPlat::TLSKey* lwp::mailbox_key = NULL;
#ifdef DO_LIBPDTHREAD_MEASUREMENTS
XPlat::TLSKey* lwp::perf_data_key = NULL;
#endif /* DO_LIBPDTHREAD_MEASUREMENTS */

lwp* lwp::main_thr = NULL;



void
lwp::initialize_tsd_keys( void )
{
    lwp_key = new XPlat::TLSKey;
    tid_key = new XPlat::TLSKey;
    name_key = new XPlat::TLSKey;
    mailbox_key = new XPlat::TLSKey;
#ifdef DO_LIBPDTHREAD_MEASUREMENTS
    perf_data_key = new XPlat::TLSKey;
#endif /* DO_LIBPDTHREAD_MEASUREMENTS */
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

    // ensure we construct our keys exactly once
    // Note: we avoid making keys_once a static class data member
    // because of difficulties in ensuring when it will be constructed
    // if pdthread is built as a shared library.
    static XPlat::Once keys_once;
    keys_once.DoOnce( lwp::initialize_tsd_keys );

    assert( lwp_key != NULL );
    lwp_key->Set( (void*)whom );

    assert( tid_key != NULL );
    tid_key->Set( (void*)(&whom->_self) );

    assert( mailbox_key != NULL );
    mailbox_key->Set( (void*)whom->_mail );

#ifdef DO_LIBPDTHREAD_MEASUREMENTS
    
    assert( perf_data_key != NULL );
    perf_data_key->Set( (void*)(&whom->_perf_data) );

#endif /* DO_LIBPDTHREAD_MEASUREMENTS */

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
    XPlat::Thread::Create( lwp::wrapper_func, this, &_pself );
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
        XPlat::Thread::Join(_pself, return_val);
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
    assert( lwp_key != NULL );
    return (lwp*)lwp_key->Get();
}



lwp* lwp::get_main(thread_t tid) {
    if(lwp::main_thr != NULL) {
        return lwp::main_thr;
    } else {
        lwp::main_thr = new lwp();
        return lwp::main_thr;
    }
}



const char* lwp::name(const char* new_name) {
    assert( name_key != NULL );
    const char* retval = (const char*)name_key->Get();
    if(new_name) {
        name_key->Set( (void*)new_name );
        lwp::get_lwp()->my_name = new_name;
    }
    
    return retval;
}




const char* lwp::get_name() {
    return my_name;
}


    
thread_t lwp::get_self() {
    assert( tid_key != NULL );
    thread_t *self = (thread_t*)tid_key->Get();
    if(!self)
        return 0;
    else
        return *self;
}



mailbox* lwp::get_mailbox() {
    assert( mailbox_key != NULL );
    return (mailbox*)mailbox_key->Get();
}



#if DO_LIBPDTHREAD_MEASUREMENTS == 1
thr_perf_data_t* lwp::get_perf_data() {
    assert( perf_data_key != NULL );
    return (thr_perf_data_t*)perf_data_key->Get();
}
#endif /* DO_LIBPDTHREAD_MEASUREMENTS == 1 */



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

} // namespace pdthr

