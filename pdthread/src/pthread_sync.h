#ifndef SEEN_PTHREAD_SYNC
#define SEEN_PTHREAD_SYNC

#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>

#if DO_LIBPDTHREAD_MEASUREMENTS == 1
#include "../h/thread.h"
#include "dummy_sync.h"
#include "hashtbl.C"
#ifdef PTHREAD_SYNC_C
#undef COLLECT_MEASUREMENT
#define COLLECT_MEASUREMENT(x) do { \
    thr_collect_measurements(x); \
    lock_stats.put(thr_self(), lock_stats.get(thr_self()) + 1); \
 } while (0)
#endif /* PTHREAD_SYNC_C */
#endif /* DO_LIBPDTHREAD_MEASUREMENTS == 1 */

class pthread_sync;

#if DO_LIBPDTHREAD_MEASUREMENTS == 1
class sync_registry {
  private:
    static hashtbl<pthread_sync*, pthread_sync*> registry;
  public:
    static void register_sync(pthread_sync*);
    static void unregister_sync(pthread_sync*);
    static void dump_all_sync_stats();
};
#endif /* DO_LIBPDTHREAD_MEASUREMENTS */

class pthread_sync {
  private:
    pthread_mutex_t *mutex;
    pthread_cond_t **conds;
    int *registered_conds;
    int num_registered_conds;
    const char* comment;
    
#if DO_LIBPDTHREAD_MEASUREMENTS == 1
    hashtbl<thread_t, long long> lock_stats;
#endif
    
  public:
    pthread_sync(const char *c = "unspecified");
    ~pthread_sync();
    void lock();
    void unlock();
    void register_cond(unsigned cond_num);
    void signal(unsigned cond_num);
    void wait(unsigned cond_num);
    void dump_measurements(void);
    const char* get_name() {
        return comment;
    }
};

#if DO_LIBPDTHREAD_MEASUREMENTS == 1

class lock_stats_printer {
  private:
    int total_ops;
    FILE* out_file;
    const char *comment;
    hashtbl<thread_t, long long> *stats;
    
  public:
    lock_stats_printer(FILE* f, hashtbl<thread_t, long long> *s, const char *comment);
    void exec(thread_t key_val);

    int get_total_ops() { return total_ops; }
};

#endif

#endif
