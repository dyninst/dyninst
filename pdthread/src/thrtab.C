#include <stdio.h>
#include <stdlib.h>
#include "thrtab.h"
#include "thrtab_entries.h"
#include "thr_mailbox.h"

#if !defined(i386_unknown_nt4_0)
#include <unistd.h>
#endif // !defined(i386_unknown_nt4_0)

#if defined(i386_unknown_nt4_0)
#include "win_thr_mailbox.h"
#include "wmsg_q.h"
#endif // defined(i386_unknown_nt4_0)


#if DO_DEBUG_LIBPDTHREAD_THRTAB == 1
#define DO_DEBUG_LIBPDTHREAD 1
#else
#define DO_DEBUG_LIBPDTHREAD 0
#endif

#define CURRENT_FILE thrtab_C
#include "thr_debug.h"

#undef DO_DEBUG_LIBPDTHREAD

namespace pdthr
{

refarray<entity> thrtab::entries;
hashtbl<entity*,thread_t> thrtab::entity_registry("entity*","thread_t","entity_registry");

dllist<thread_t> thrtab::joinables(list_types::fifo);

unsigned thrtab::initialized = thrtab::create_main_thread();

unsigned thrtab::create_main_thread() {
    thr_debug_msg(CURRENT_FUNCTION, "creating main thread\n");
    unsigned main_tid;
    lwp* main_thr;

    entries.set_destroy_constituents();

    // Because of the semantics of thr_join, there is no thread 0
    thrtab::entries.push_back((entity*)NULL);

    // the "main" thread should be thread 1
    main_thr = lwp::get_main();
    main_tid = thrtab::entries.push_back((entity*)main_thr);

    main_thr->set_self(main_tid);

    thr_debug_msg(CURRENT_FUNCTION, "main_tid == %d\n", main_tid);

#if defined(i386_unknown_nt4_0)
    main_thr->init(new win_thr_mailbox(main_tid), NULL, NULL);
#else // defined(i386_unknown_nt4_0)
    main_thr->init(new thr_mailbox(main_tid), NULL, NULL);
#endif // defined(i386_unknown_nt4_0)

    main_thr = lwp::get_main();    

    entity_registry.put((entity*)main_thr, main_tid);

    return 1;
}

thread_t thrtab::create_thread(lwp::task_t func, lwp::value_t arg, bool start) {
    lwp* thread = new lwp();
    thread_t new_tid = entries.push_back((entity*)thread);
    thr_debug_msg(CURRENT_FUNCTION, "creating new thread with tid %d\n", new_tid);
    thread->set_self(new_tid);
#if defined(i386_unknown_nt4_0)
    thread->init(new win_thr_mailbox(new_tid), func, arg);
#else // defined(i386_unknown_nt4_0)
    thread->init(new thr_mailbox(new_tid), func, arg);
#endif // defined(i386_unknown_nt4_0)
    if(start)
        thread->start();
    thr_debug_msg(CURRENT_FUNCTION, "done creating thread with tid %d\n", new_tid);
    entity_registry.put((entity*)thread, new_tid);

    return new_tid;
}

thread_t thrtab::create_file( PdFile fd,
                                thread_t owner,
                                int (*will_block_func)(void*),
                                void* desc,
                                bool is_special)
{
    file_q* new_file = new file_q(fd, owner, will_block_func, desc, is_special);
    thread_t new_tid = entries.push_back((entity*)new_file);
    new_file->init(new_tid);
    thr_debug_msg(CURRENT_FUNCTION, "creating new file with tid %d\n", new_tid);
    entity_registry.put((entity*)new_file, new_tid);

    return new_tid;
}

thread_t thrtab::create_socket( PdSocket sock,
                                thread_t owner,
                                int (*will_block_func)(void*),
                                void* desc,
                                bool is_special)
{
    socket_q* new_socket = new socket_q(sock, owner, 
										will_block_func, desc, is_special);
    thread_t new_tid = entries.push_back((entity*)new_socket);
    new_socket->init(new_tid);
    thr_debug_msg(CURRENT_FUNCTION, "creating new socket with tid %d\n", new_tid);
    entity_registry.put((entity*)new_socket, new_tid);

    return new_tid;
}

#if defined(i386_unknown_nt4_0)
thread_t
thrtab::create_wmsg( thread_t owner )
{
    wmsg_q* new_q = new wmsg_q( owner );

    thread_t new_tid = entries.push_back( new_q );
    thr_debug_msg( CURRENT_FUNCTION,
                    "created new wmsg_q with tid %d\n", new_tid );
    entity_registry.put( new_q, new_tid );

    return new_tid;
}
#endif // defined(i386_unknown_nt4_0)

entity* thrtab::get_entry(thread_t tid) {
    entity* retval = thrtab::entries[tid];
    thr_debug_msg(CURRENT_FUNCTION, "seeking entity at %d; it is %p\n", tid, retval);
    return retval;
}

thread_t thrtab::get_tid(entity* e) {
    thread_t tid = entity_registry.get(e);
    thr_debug_msg(CURRENT_FUNCTION, "seeking tid for entity %p; it is %d\n", e, tid);
    return tid;
}

void thrtab::remove(thread_t t) {
    entity* e = thrtab::entries[t];
    thrtab::entries.set_elem_at(t,NULL);
    delete e;
}

bool thrtab::is_io_entity(thread_t tid) {
    entity* entry = thrtab::entries[tid];
    
    thr_debug_msg(CURRENT_FUNCTION, "requesting is_io_entity at %d, of %p\n", tid, entry);
    if(!entry) return false;
    item_t entity_type = entry->gettype();
    return entity_type == item_t_file 
#if defined(i386_unknown_nt4_0)
        || entity_type == item_t_wmsg
#endif // defined(i386_unknown_nt4_0)
        || entity_type == item_t_socket;
}

void thrtab::register_joinable(thread_t tid) {
    thr_debug_msg(CURRENT_FUNCTION, "registering %d as joinable\n", tid);

    if(joinables.contains(tid))
        joinables.put(tid);
    thr_debug_msg(CURRENT_FUNCTION, "DONE registering %d as joinable\n", tid);
}

void thrtab::unregister_joinable(thread_t tid) {
    thr_debug_msg(CURRENT_FUNCTION, "unregistering %d as joinable\n", tid);
    if(joinables.contains(tid))
       joinables.yank_nb(tid, NULL);
    thr_debug_msg(CURRENT_FUNCTION, "DONE unregistering %d as joinable\n", tid);
}

thread_t thrtab::get_any_joinable() {
    return joinables.take();
}

} // namespace pdthr

