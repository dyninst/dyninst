#ifndef __libthread_thrtab_h__
#define __libthread_thrtab_h__

#include "../h/thread.h"
#include "refarray.h"
#include "dllist.C"
#include "rwlock.h"
#include "thrtab_entries.h"
#include "pthread_sync.h"
#include <pthread.h>

class thrtab {
  private:
    static refarray<entity,1> entries;
    static dllist<thread_t,pthread_sync> joinables;
    static unsigned initialized;
    static unsigned create_main_thread();
    
  public:
    static thread_t create_thread(lwp::task_t func,
                                  lwp::value_t arg,
                                  bool start);
    static thread_t create_file(PDDESC fd, thread_t owner,
                                int (*will_block_func)(void*), void* desc,
								bool is_special);
    static thread_t create_socket(PDSOCKET sock, thread_t owner,
                                  int (*will_block_func)(void*), void* desc,
								  bool is_special);
    
    static entity* get_entry(thread_t tid);
    static bool is_valid(thread_t tid) {
        return entries[tid] != NULL;
    }
    static bool is_io_entity(thread_t tid);
    static void register_joinable(thread_t tid);
    static void unregister_joinable(thread_t tid);
    static thread_t get_any_joinable();
};

#endif /*  __libthread_thrtab_h__ */
