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

#include "../h/thread.h"

#define PTHREAD_SYNC_C
#include "pthread_sync.h"
#undef PTHREAD_SYNC_C

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_CONDS 512

#if DO_LIBPDTHREAD_MEASUREMENTS == 1

class sync_visitor {
  public:
    void exec(pthread_sync* p);
};

void sync_visitor::exec(pthread_sync* p) {
    if(p) p->dump_measurements();
}

hashtbl<pthread_sync*, pthread_sync*> sync_registry::registry;

void sync_registry::register_sync(pthread_sync* p) {
    sync_registry::registry.put(p,p);
}

void sync_registry::unregister_sync(pthread_sync* p) {
    sync_registry::registry.put(p,NULL);
}

void sync_registry::dump_all_sync_stats() {
    sync_visitor sv;
    sync_registry::registry.map_vals(&sv);
}

lock_stats_printer::lock_stats_printer(FILE* f, hashtbl<thread_t, long long> *s, const char *c)
        : out_file(f), total_ops(0), stats(s), comment(c) {}

#define LOCK_STATS_PRINTER_BUFSIZE 512

void lock_stats_printer::exec(thread_t key_val) {
    char out_line[LOCK_STATS_PRINTER_BUFSIZE];
    int locks_per_key = 0;
    int line_length = 0;

    memset((void*)out_line, 0, LOCK_STATS_PRINTER_BUFSIZE);
    
    locks_per_key = stats->get(key_val);

    fprintf(out_file, "thread %d acquired monitor \"%s\" %d times\n", key_val, comment, locks_per_key);
    
    total_ops += locks_per_key;
}

#endif /* DO_LIBPDTHREAD_MEASUREMENTS == 1 */

pthread_sync::pthread_sync(const char* c) 
        : comment(c)
{
    this->num_registered_conds = 0;
    this->conds = new (pthread_cond_t*)[MAX_CONDS];
    this->registered_conds = new (int)[MAX_CONDS]; 
    this->mutex = new pthread_mutex_t;
    pthread_mutex_init(mutex, NULL);
}

pthread_sync::~pthread_sync() {
    for (int i = 0; i < num_registered_conds ; i++) {
        pthread_cond_destroy(conds[registered_conds[i]]);
        delete conds[registered_conds[i]];
    }

    pthread_mutex_destroy(mutex);
    delete mutex;
    delete [] conds;
    delete [] registered_conds;

    dump_measurements();

#if DO_LIBPDTHREAD_MEASUREMENTS == 1
    sync_registry::unregister_sync(this);
#endif /* DO_LIBPDTHREAD_MEASUREMENTS == 1 */
}

void pthread_sync::dump_measurements() {
#if DO_LIBPDTHREAD_MEASUREMENTS == 1
    
    char outfile[256], summary[256];
    FILE* out_file;

    snprintf(outfile, 255, "lock-stats-for-%s-%d:%d.txt\0", comment, getpid(), thr_self());
    
    out_file = fopen(outfile, "w+");    
    
    lock_stats_printer lsp(out_file, &lock_stats, comment);
    
    lock_stats.map_keys(&lsp);

    fprintf(out_file, "total ops on this monitor: %d\n\0", lsp.get_total_ops());

    fclose(out_file);
    
#endif /* DO_LIBPDTHREAD_MEASUREMENTS == 1 */
}

void pthread_sync::lock() {
    int status = 0;
    COLLECT_MEASUREMENT(THR_LOCK_ACQ);
    
#if LIBTHREAD_DEBUG == 1
        fprintf(stderr, "acquiring lock %p (by pthread %d)...\n", &mutex, pthread_self());
#endif

    status = pthread_mutex_trylock(mutex);

    if (status == EBUSY) {
        COLLECT_MEASUREMENT(THR_LOCK_BLOCK);
        COLLECT_MEASUREMENT(THR_LOCK_TIMER_START);
        pthread_mutex_lock(mutex);
        COLLECT_MEASUREMENT(THR_LOCK_TIMER_STOP);
    }

#if LIBTHREAD_DEBUG == 1
        fprintf(stderr, "acquired lock %p!\n", &mutex);
#endif
}

void pthread_sync::unlock() {
#if LIBTHREAD_DEBUG == 1
        fprintf(stderr, "releasing lock %p...\n", &mutex);
#endif
        pthread_mutex_unlock(mutex);
#if LIBTHREAD_DEBUG == 1
        fprintf(stderr, "released lock %p!\n", &mutex);
#endif
}

void pthread_sync::register_cond(unsigned cond_num) {
    registered_conds[num_registered_conds++] = cond_num;
    conds[cond_num] = new pthread_cond_t; 
    pthread_cond_init(conds[cond_num], NULL);
}

void pthread_sync::signal(unsigned cond_num) {
    pthread_cond_signal(conds[cond_num]);
}

void pthread_sync::wait(unsigned cond_num) {
    pthread_cond_wait(conds[cond_num], mutex);
}

