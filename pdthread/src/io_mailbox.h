#ifndef __libthread_io_mailbox_h__
#define __libthread_io_mailbox_h__

#include "mailbox.h"
#include "refarray.C"
#include "dllist.C"
#include "predicate.h"
#include "../h/thread.h"

namespace pdthr
{

class io_entity;

class io_mailbox : public mailbox {
    thread_t owned_by;
    io_entity* underlying_io;

  public:
    io_mailbox(thread_t owner, io_entity* ie);
    ~io_mailbox();

    /* put() delivers message m into this mailbox */
    virtual int put(message* m);

    /* recv() receives mail from this mailbox into buf;
       see the libthread documentation for the full
       meanings of the args */
    virtual int recv(thread_t* sender, tag_t* tagp, void* buf, unsigned* countp);

    /* poll() checks for suitable available messages */
    virtual int poll(thread_t* from, tag_t* tagp, unsigned block, unsigned fd_first=0);

};

} // namespace pdthr

#endif
