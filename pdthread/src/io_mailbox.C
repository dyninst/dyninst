#include "io_mailbox.h"
#include "message.h"
#include <stdio.h>
#include <stdlib.h>

io_mailbox::io_mailbox(thread_t owner, io_entity* ie)
        : mailbox(owner), underlying_io(ie) { }

int io_mailbox::put(message* m) {
    int ret = (int)m->deliver_to(underlying_io);
    delete m;
    return ret;
}


int io_mailbox::recv(thread_t* sender, tag_t* tagp, void* buf, unsigned* countp) {
    fprintf(stderr, "panic: call to io_mailbox::recv(); possible thrtab corruption?\n");
    abort();
    return THR_ERR;
}

int io_mailbox::poll(thread_t* from, tag_t* tagp, unsigned block, unsigned fd_first) {
    fprintf(stderr, "panic: call to io_mailbox::poll(); possible thrtab corruption?\n");
    abort();
    return THR_ERR;
}
