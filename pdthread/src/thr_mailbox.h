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
	friend class ready_socks_populator;
	friend class fd_set_populator;

  private:
    int msg_avail_pipe[2];
    

	// monitor used to enforce mutual exclusion to the bound socket set
	// and the ready-to-read socket set
    dllist<PDSOCKET,dummy_sync> *ready_socks;
    dllist<PDSOCKET,dummy_sync> *bound_socks;

    pthread_sync* sock_monitor;
    
    dllist<message*,dummy_sync> *messages;
    dllist<message*,dummy_sync> *sock_messages;
    
    bool check_for(thread_t* sender, tag_t* type,
                          bool do_block = false, bool do_yank = false,
                          message** m = NULL, unsigned io_first = 1);
	void wait_for_input( void );

	void raise_msg_avail( void );
	void clear_msg_avail( void );
    
	bool is_buffered_special_ready( thread_t* sender, tag_t* type );

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

    void bind_sock(PDSOCKET s, unsigned special, int (*wb)(void*), void* desc, thread_t* ptid);
    void unbind_sock(PDSOCKET s, bool getlock = true);

    bool is_sock_bound(PDSOCKET s);

    void clear_ready_sock( PDSOCKET sock );
    
    virtual void dump_state();
};
#endif


