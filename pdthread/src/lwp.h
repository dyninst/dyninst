#ifndef __libthread_lwp_h__
#define __libthread_lwp_h__

#ifndef __in_thrtabentries__
#error You should not include this file directly
#endif

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
        lwp::task_t func;
        void* arg;
        thread_t tid;
        lwp* the_thread;
    };
    
    typedef struct warg* wrapper_arg_t;
    
  private:

    /* tid_key points to the libthread tid.  This way, a pthreads
       thread can look up its libthread tid using pthreads TSD. */
    static pthread_key_t tid_key;

    /* For any thread, lwp_key points to the lwp object corresponding
       to its thread table entry */
    static pthread_key_t lwp_key;

    /* For any thread, mailbox_key points to its incoming thread
       mailbox */
    static pthread_key_t mailbox_key;

    /* For any thread, name_key points to its thread name */
    static pthread_key_t name_key;

    /* For any thread, errno_key points to its "errno" */
    static pthread_key_t errno_key;

#ifdef DO_LIBPDTHREAD_MEASUREMENTS

    static pthread_key_t perf_data_key;

#endif /* DO_LIBPDTHREAD_MEASUREMENTS */
    
    /* keys_once is a latch used to ensure that the static TSD keys
       are only initialized once */
    static pthread_once_t keys_once;

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
    pthread_t _pself;

    task_t _task;
    value_t _arg;
    value_t _returnval;

    unsigned _running;
    unsigned _completed;
    unsigned _joined;
    unsigned _reaped;

    rwlock_t _running_lk;
    rwlock_t _completed_lk;
    rwlock_t _joined_lk;
    rwlock_t _reaped_lk;
    
#ifdef DO_LIBPDTHREAD_MEASUREMENTS

    thr_perf_data_t _perf_data;
    
#endif /* DO_LIBPDTHREAD_MEASUREMENTS */

    wrapper_arg_t _wrapped_args;
    mailbox *_mail;

    const char* my_name;
    
  public:
    lwp() : _running(0), _completed(0),
        _joined(0), _reaped(0),
        _running_lk(rwlock::favor_writers),
        _completed_lk(rwlock::favor_writers),
        _joined_lk(rwlock::favor_writers),
        _reaped_lk(rwlock::favor_writers)
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

#endif
