#include "../h/thread.h"
#include "thrtab.h"
#include "thrtab_entries.h"
#include "message.h"
#include "mailbox.h"

#include <assert.h>
#include <stdio.h>

int msg_send(thread_t tid, tag_t tag, void* buf, unsigned size) {
    thread_t sender = lwp::get_self();
    unsigned ret;
    
    if(!thrtab::is_valid(tid))
        return THR_ERR;

    message* m = new message(sender, tag, buf, size);
    mailbox* recipient = mailbox::for_thread(tid);
    
    assert(recipient);
    
    ret = recipient->put(m);
    
    return ret;
}

int msg_poll(thread_t* tid, tag_t* tag, unsigned block) {
    mailbox* mbox = lwp::get_mailbox();
    return mbox->poll(tid, tag, block);
}

int msg_poll_preference(thread_t* tid, tag_t* tag, unsigned block, unsigned fd_first) {
    mailbox* mbox = lwp::get_mailbox();
    return mbox->poll(tid, tag, block, fd_first);
}


int msg_recv(thread_t* tid, tag_t* tag, void* buf, unsigned* bufsize) {
    mailbox* mbox = lwp::get_mailbox();
    if(!mbox) {
        fprintf(stderr, "panic:  can't get mailbox for lwp %d\n", thr_self());
        assert(mbox);
    }
    
    return mbox->recv(tid,tag,buf,bufsize);
}

int msg_bind(PDDESC fd, unsigned special, int (*will_block)(void*), void* arg, thread_t* tid) {
    ;
}


int msg_bind_sig(int sig, thread_t* tid) {
    fprintf(stderr,"binding signals to message queues not implemented; aborting...\n");
    assert(false);
}


int msg_bind_socket(PDSOCKET s, unsigned special, int (*will_block)(void*), void* arg, thread_t* tid);

#if defined(i386_unknown_nt4_0)
int msg_bind_wmsg(thread_t* tid);
#endif /* defined(i386_unknown_nt4_0) */

int msg_unbind(thread_t tid);

