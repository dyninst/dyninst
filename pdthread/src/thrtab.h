#ifndef __libthread_thrtab_h__
#define __libthread_thrtab_h__

#include "../h/thread.h"
#include "refarray.h"
#include "dllist.C"
#include "rwlock.h"
#include "thrtab_entries.h"
#include "hashtbl.C"
#include "xplat/h/Monitor.h"

namespace pdthr
{

class thrtab {
  private:
    static refarray<entity> entries;
    static hashtbl<entity*,thread_t> entity_registry;
    static dllist<thread_t> joinables;
    static unsigned initialized;
    static unsigned create_main_thread();
    
  public:
    static thread_t create_thread(lwp::task_t func,
                                  lwp::value_t arg,
                                  bool start);
    static thread_t create_file( PdFile fd, thread_t owner,
                                int (*will_block_func)(void*), void* desc,
                                bool is_special);
    static thread_t create_socket( PdSocket sock, thread_t owner,
                                  int (*will_block_func)(void*), void* desc,
                                  bool is_special);
#if defined(i386_unknown_nt4_0)
    static thread_t create_wmsg( void );
#endif // defined(i386_unknown_nt4_0)
    
    static entity* get_entry(thread_t tid);
    static thread_t get_tid(entity* e);
    static void remove(thread_t tid);
    static bool is_valid(thread_t tid) {
        return entries[tid] != NULL;
    }
    static unsigned size() {
        return entries.capacity();
    }
    static bool is_io_entity(thread_t tid);
    static void register_joinable(thread_t tid);
    static void unregister_joinable(thread_t tid);
    static thread_t get_any_joinable();
};

} // namespace pdthr

#endif /*  __libthread_thrtab_h__ */
