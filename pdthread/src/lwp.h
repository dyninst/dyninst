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

#ifndef __libthread_lwp_h__
#define __libthread_lwp_h__

#ifndef __in_thrtabentries__
#error You should not include this file directly
#endif

#include "xplat/Thread.h"
#include "xplat/TLSKey.h"
#include "xplat/Mutex.h"
#include "xplat/Once.h"

namespace pdthr
{

class mailbox;

class lwp : public entity {
  public:

    /* a task_t is a pointer to a function that takes a void* as an
       argument and returns a void* -- basically, a pointer to a
       function runnable by a libthread thread */
    typedef void* (*task_t)(void*);

    /* a value_t is the type of the argument to a libthread thread
       function as well as the type of the return value of a libthread
       thread */
    typedef void* value_t;

    typedef struct warg {
        task_t func;
        void* arg;
        thread_t tid;
        lwp* the_thread;
    };
    
    typedef struct warg* wrapper_arg_t;
    
  private:

    /* tid_key points to the libthread tid.  This way, a pthreads
       thread can look up its libthread tid using pthreads TSD. */
    static XPlat::TLSKey* tid_key;

    /* For any thread, lwp_key points to the lwp object corresponding
       to its thread table entry */
    static XPlat::TLSKey* lwp_key;

    /* For any thread, mailbox_key points to its incoming thread
       mailbox */
    static XPlat::TLSKey* mailbox_key;

    /* For any thread, name_key points to its thread name */
    static XPlat::TLSKey* name_key;

    /* For any thread, errno_key points to its "errno" */
    static XPlat::TLSKey* errno_key;

#ifdef DO_LIBPDTHREAD_MEASUREMENTS

    static XPlat::TLSKey* perf_data_key;

#endif /* DO_LIBPDTHREAD_MEASUREMENTS */

    /* initialize_tsd_keys creates TSD keys for each of the
       preceding TSD items */
    static void initialize_tsd_keys();

    /* wrapper_func is a function which sets the TSD values and then
       executes a specified task_t -- it casts its void* argument to
       a lwp* to extract the proper TSD values */
    static void* wrapper_func(void* arg);

    static void set_tsd_keys_for(lwp* whom);

    static lwp* main_thr;
    
    thread_t _self;
    XPlat::Thread::Id _pself;

    task_t _task;
    value_t _arg;
    value_t _returnval;

    unsigned _running;
    unsigned _completed;
    unsigned _joined;
    unsigned _reaped;

    XPlat::Mutex _running_lk;
    XPlat::Mutex _completed_lk;
    XPlat::Mutex _joined_lk;
    XPlat::Mutex _reaped_lk;
    
#ifdef DO_LIBPDTHREAD_MEASUREMENTS

    thr_perf_data_t _perf_data;
    
#endif /* DO_LIBPDTHREAD_MEASUREMENTS */

    wrapper_arg_t _wrapped_args;
    mailbox *_mail;

    const char* my_name;
    
  public:
    lwp() : _running(0), _completed(0),
        _joined(0), _reaped(0)
        {
#ifdef DO_LIBPDTHREAD_MEASUREMENTS
            _perf_data.num_lock_acquires = 0;
            _perf_data.num_lock_blocks = 0;
            _perf_data.lock_contention_time = 0;
            _perf_data.lock_timer_start = 0;

            _perf_data.num_msg_ops = 0;
            _perf_data.msg_time = 0;
            _perf_data.msg_timer_start = 0;
#endif /* DO_LIBPDTHREAD_MEASUREMENTS */
        }
    
    virtual item_t gettype() { return item_t_thread; }

    thread_t self() { return _self; }
    void set_self(thread_t self) { _self = self; }
    void init(mailbox *my_mailbox, lwp::task_t func, lwp::value_t arg);
    void start();
    int join(thread_t* departed, void** return_val);

    unsigned is_running();
    unsigned is_completed();
    void start_running();
    void stop_running();
    void complete();
    void set_returnval(void* result);
    
    static mailbox *get_mailbox();
    static thread_t get_self();
    static lwp *get_lwp();
    static lwp *get_main(thread_t tid=1);
    const char* get_name();
    static const char* name(const char* new_name = NULL);

#if DO_LIBPDTHREAD_MEASUREMENTS == 1
    static thr_perf_data_t* get_perf_data();
#endif /* DO_LIBPDTHREAD_MEASUREMENTS == 1 */
    
}; /* end of class lwp */

} // namespace pdthr

#endif
