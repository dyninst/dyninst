#include <stdlib.h>
#include <stdio.h>
#include "../h/thread.h"
#include "thrtab_entries.h"
#include "thrtab.h"
#include <assert.h>

#include "pthread_sync.h"

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

void msg_dump_stats();

#include "hashtbl.C"

class thr_stats_registry {
  private:
    static hashtbl<thread_t, const char*, pthread_sync> thread_names;
    static hashtbl<thread_t, thr_perf_data_t*, pthread_sync> thread_stats;
  public:
    static void register_thread(thread_t tid, const char* thr_name, thr_perf_data_t* perf_data);
    static void dump_all_stats();
};

hashtbl<thread_t, const char*, pthread_sync> thr_stats_registry::thread_names;
hashtbl<thread_t, thr_perf_data_t*, pthread_sync> thr_stats_registry::thread_stats;

void thr_stats_registry::register_thread(thread_t tid, const char* thr_name, thr_perf_data_t* perf_data) {
    thread_names.put(tid, thr_name);
    thread_stats.put(tid, perf_data);
}

void thr_stats_registry::dump_all_stats() {
    const char* current_name;
    thr_perf_data_t* current_perf;
    int i, max = thrtab::size();

    char out_file_name[256];
    FILE* out_FILE;

    snprintf(out_file_name, 255, "thread-stats-for-%d.txt\0", getpid());

    out_FILE = fopen(out_file_name, "w+");
    
    fprintf(out_FILE, "id,lck acq,lck blk,blk tm,msg ops,msg tm,name\n");

    for (i = 0; i < max; i++) {
        current_name = thread_names.get(i);
        current_perf = thread_stats.get(i);
        if(current_name && current_perf) {
            fprintf(out_FILE, "%d,", i);
            fprintf(out_FILE, "%llu,", current_perf->num_lock_acquires);
            fprintf(out_FILE, "%llu,", current_perf->num_lock_blocks);
            fprintf(out_FILE, "%llu,", current_perf->lock_contention_time);
            fprintf(out_FILE, "%llu,", current_perf->num_msg_ops);
            fprintf(out_FILE, "%llu,", current_perf->msg_time);
            fprintf(out_FILE, "%s\n", current_name);
            fflush(out_FILE);            
        }
    }
    
    fclose(out_FILE);

    msg_dump_stats();
}
    
long long thr_get_vtime() {
    long long retval;
#if defined (i386_unknown_linux2_0)
#define THR_GET_VTIME_DEFINED 1
#warning libthread: no thr_get_vtime for linux yet; timer numbers will be imprecise
    // FIXME:  do rdtsc or otherwise more precise implementation
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    retval = (tv.tv_sec * 1000000) + tv.tv_usec;
    
#endif

#if defined (sparc_sun_solaris2_4)
#define THR_GET_VTIME_DEFINED 1
    retval = gethrvtime();
#endif 

#if defined (rs6000_ibm_aix4_1)
#define THR_GET_VTIME_DEFINED 1
#warning libthread: no thr_get_vtime for aix yet; timer numbers will be imprecise
    // FIXME: do big perfctr implementation
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    retval = (tv.tv_sec * 1000000) + tv.tv_usec;

#endif 

#ifndef THR_GET_VTIME_DEFINED
#warning libthread: no thr_get_vtime for your platform, please edit pdthread/src/thread.C and add one
#warning libthread: using lame gettimeofday() based timer; horribly imprecise

    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    retval = (tv.tv_sec * 1000000) + tv.tv_usec;
    
#endif

    return retval;
}

void thr_collect_measurements(int op) {
    thr_perf_data_t* perf_data;
    unsigned long long current_time;

    current_time = thr_get_vtime();

    perf_data = lwp::get_perf_data();

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

void thr_dump_stats_for_thread(const char* thr_name, thr_perf_data_t* perf_data);

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
    return ((file_q*)thrtab_entry)->fd.fd;
}

PDSOCKET thr_socket(thread_t tid) {
    thr_debug_msg(CURRENT_FUNCTION, "tid = %d\n", (unsigned)tid);
    entity* thrtab_entry = thrtab::get_entry(tid);
    if (!thrtab_entry || (thrtab_entry && thrtab_entry->gettype() != item_t_socket))
        return INVALID_PDSOCKET;
    return ((socket_q*)thrtab_entry)->sock.s;
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

#if DO_LIBPDTHREAD_MEASUREMENTS == 1

    thr_stats_registry::register_thread(thr_self(), lwp::name(), lwp::get_perf_data());

#if 0
    char outfile[256], summary[256];
    thr_perf_data_t *perf_data = lwp::get_perf_data();
    FILE* out_file;

    
    out_file = fopen(outfile, "w+");
    
    fprintf(out_file, "number of lock acquires:  %d\n", perf_data->num_lock_acquires);
    fprintf(out_file, "number of lock blocks:  %d\n", perf_data->num_lock_blocks);
    fprintf(out_file, "lock contention time:  %d\n", perf_data->lock_contention_time);
    fprintf(out_file, "number of message ops:  %d\n", perf_data->num_msg_ops);
    fprintf(out_file, "message op time:  %d\n", perf_data->msg_time);

    fclose(out_file);
#endif /* 0 */
#endif /* DO_LIBPDTHREAD_MEASUREMENTS == 1 */

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

void thr_library_cleanup(void) {

#if DO_LIBPDTHREAD_MEASUREMENTS == 1

    // currently, only cleanup task is dumping stats for monitors and threads
    sync_registry::dump_all_sync_stats();
    thr_stats_registry::dump_all_stats();

#endif

}
