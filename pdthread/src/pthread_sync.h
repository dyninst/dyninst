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
