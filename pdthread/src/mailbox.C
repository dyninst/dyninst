#include "mailbox.h"
#include "refarray.h"
#include "predicate.h"
#include "message_predicates.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include "mailbox_defs.h"

refarray<mailbox,1>* mailbox::registry = NULL;

mailbox* mailbox::for_thread(thread_t tid) {
    return (*registry)[tid];
}

void mailbox::register_mbox(thread_t owner, mailbox* which) {
    if (!mailbox::registry)
        mailbox::registry = new refarray<mailbox,1>(64,NULL,NULL);
    mailbox::registry->set_elem_at(owner, which);    
}

mailbox::mailbox(thread_t owner) {
    owned_by = owner;
    mailbox::register_mbox(owner, this); 
    monitor = new pthread_sync();
    monitor->register_cond(RECV_AVAIL);
}

mailbox::~mailbox() {
    delete monitor;
}
