#ifndef __libthread_mailbox_h__
#define __libthread_mailbox_h__

#include <pthread.h>
#include "refarray.C"
#include "pthread_sync.h"
#include "dummy_sync.h"
#include "dllist.C"
#include "predicate.h"
#include "../h/thread.h"

class message;

class mailbox {
  private:
    static refarray<mailbox,1>* registry;
    static void register_mbox(thread_t owner, mailbox* which);
    
  protected:
    thread_t owned_by;

	// monitor for mutual exclusion to the mailbox message queue(s)
	// and to signal when a message is available
    pthread_sync* monitor;
    
  public:
    mailbox(thread_t owner);
    virtual ~mailbox();
    
    static void dump_mailboxes();
    void dump();
    void dump_meta();
    virtual void dump_state();
    
    /* mailbox::for_thread() returns the appropriate mailbox for tid */
    static mailbox* for_thread(thread_t tid);

    /* put() delivers message m into this mailbox */
    virtual int put(message* m) = 0;

    /* recv() receives mail from this mailbox into buf;
       see the libthread documentation for the full
       meanings of the args */
    virtual int recv(thread_t* sender, tag_t* tagp, void* buf, unsigned* countp) = 0;

    /* poll() checks for suitable available messages */
    virtual int poll(thread_t* from, tag_t* tagp, unsigned block, unsigned fd_first=0) = 0;
};
#endif
