/* -*- mode: c++; c-basic-indent: 4 -*- */
#ifndef __libthread_io_entity_h__
#define __libthread_io_entity_h__

#ifndef __in_thrtabentries__
#error You should not include this file directly
#endif

#include "mailbox.h"
#include "io_mailbox.h"

class io_entity : public entity {
  protected:
    bool boundp;
    thread_t owner;
    int (*will_block_fn)(void*);
    void* desc;
    mailbox* my_mail;
    thread_t my_self;
    int special;
    
  public:
    io_entity(thread_t owned_by, int (*will_block_func)(void*), void* the_desc, int is_desc_special=1) : owner(owned_by), will_block_fn(will_block_func), my_mail(NULL), special(is_desc_special), desc(the_desc) { }
    
    virtual ~io_entity() {
        if (my_mail) delete my_mail;
    }
    
    void init(thread_t tid) {
        my_self = tid;
        my_mail = new io_mailbox(tid, this);
    }
    
    virtual bool is_ready(unsigned* sizep, unsigned do_poll) const;
    virtual thread_t get_owner() { return owner; }
    
    virtual int do_read(void* buf, unsigned bufsize, unsigned* count) = 0;
    virtual int do_write(void* buf, unsigned bufsize, unsigned* count) = 0;
    inline int is_special() { return special; }
    inline int will_block() { return will_block_fn && will_block_fn(desc); }
    inline int self() { return my_self; }
};

#endif
