#include "thrtab.h"
#include "thrtab_entries.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "thr_mailbox.h"

#if DO_DEBUG_LIBPDTHREAD_THRTAB == 1
#define DO_DEBUG_LIBPDTHREAD 1
#else
#define DO_DEBUG_LIBPDTHREAD 0
#endif

#include "thr_debug.h"

#undef DO_DEBUG_LIBPDTHREAD

refarray<entity,1> thrtab::entries;
dllist<thread_t,pthread_sync> thrtab::joinables(list_types::fifo);

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

    main_thr->init(new thr_mailbox(main_tid), NULL, NULL);

    main_thr = lwp::get_main();    
}

thread_t thrtab::create_thread(lwp::task_t func, lwp::value_t arg, bool start) {
    lwp* thread = new lwp();
    thread_t new_tid = entries.push_back((entity*)thread);
    thr_debug_msg(CURRENT_FUNCTION, "creating new thread with tid %d\n", new_tid);
    thread->set_self(new_tid);
    thread->init(new thr_mailbox(new_tid), func, arg);
    if(start)
        thread->start();
    thr_debug_msg(CURRENT_FUNCTION, "done creating thread with tid %d\n", new_tid);
    return new_tid;
}

thread_t thrtab::create_file(PDDESC fd, thread_t owner, int (*will_block_func)(void*), void* desc) {
    file_q* new_file = new file_q(fd, owner, will_block_func, desc);
    thread_t new_tid = entries.push_back((entity*)new_file);
    new_file->init(new_tid);
    thr_debug_msg(CURRENT_FUNCTION, "creating new file with tid %d\n", new_tid);
    return new_tid;
}

thread_t thrtab::create_socket(PDSOCKET sock, thread_t owner, int 
                               (*will_block_func)(void*), void* desc) {
    socket_q* new_socket = new socket_q(sock, owner, will_block_func, desc);
    thread_t new_tid = entries.push_back((entity*)new_socket);
    new_socket->init(new_tid);
    thr_debug_msg(CURRENT_FUNCTION, "creating new socket with tid %d\n", new_tid);
    return new_tid;
}


entity* thrtab::get_entry(thread_t tid) {
    entity* retval = thrtab::entries[tid];
    thr_debug_msg(CURRENT_FUNCTION, "seeking entity at %d; it is %p\n", tid, retval);
    return retval;
}

bool thrtab::is_io_entity(thread_t tid) {
    entity* entry = thrtab::entries[tid];
    
    thr_debug_msg(CURRENT_FUNCTION, "requesting is_io_entity at %d, of %p\n", tid, entry);
    assert(entry);
    item_t entity_type = entry->gettype();
    return entity_type == item_t_file 
#if defined(i386_unknown_nt4_0)
        || entity_type == item_t_wmsg
#endif
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

