#ifndef __libthread_thr_mailbox_h__
#define __libthread_thr_mailbox_h__

#include <pthread.h>
#include "message.h"
#include "mailbox.h"
#include "refarray.C"
#include "pthread_sync.h"
#include "dummy_sync.h"
#include "dllist.C"
#include "predicate.h"
#include "../h/thread.h"

extern int SOCK_FD;

class thr_mailbox : public mailbox {
  private:
    static void* fds_select_loop(void* which_mailbox);    
    static void* sock_select_loop(void* which_mailbox);
    
  protected:
    pthread_t fd_selector_thread;
    pthread_t sock_selector_thread;

    volatile unsigned kill_fd_selector;
    volatile unsigned kill_sock_selector;
    int* fds_selector_pipe;
    int* sock_selector_pipe;

    void signal_fd_selector();
    void signal_sock_selector();
    
    pthread_sync* fds_monitor;
    pthread_sync* sock_monitor;
    
    dllist<PDDESC,dummy_sync> *ready_fds;
    dllist<PDDESC,dummy_sync> *bound_fds;

    dllist<PDSOCKET,dummy_sync> *ready_socks;
    dllist<PDSOCKET,dummy_sync> *bound_socks;

    dllist<message*,dummy_sync> *messages;
    dllist<message*,dummy_sync> *sock_messages;
    
    inline bool check_for(thread_t* sender, tag_t* type,
                          bool do_block = false, bool do_yank = false,
                          message** m = NULL, unsigned io_first = 1);
    
  public:
    thr_mailbox(thread_t owner);
    virtual ~thr_mailbox();

    /* put() delivers message m into this mailbox */
    virtual int put(message* m);

    /* put_sock() delivers socket message m into this mailbox */
    virtual int put_sock(message* m);

    /* recv() receives mail from this mailbox into buf;
       see the libthread documentation for the full
       meanings of the args */
    virtual int recv(thread_t* sender, tag_t* tagp, void* buf, unsigned* countp);

    /* poll() checks for suitable available messages */
    virtual int poll(thread_t* from, tag_t* tagp, unsigned block, unsigned fd_first=0);

    void bind_fd(PDDESC fd, unsigned special, int (*wb)(void*), void* desc, thread_t* ptid);
    void bind_sock(PDSOCKET s, unsigned special, int (*wb)(void*), void* desc, thread_t* ptid);

    bool is_sock_bound(PDSOCKET s);
};
#endif


