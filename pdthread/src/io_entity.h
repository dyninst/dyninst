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
    bool special;
    
  public:
    io_entity(thread_t owned_by,
		int (*will_block_func)(void*),
		void* the_desc,
		bool is_desc_special=false);
    
    virtual ~io_entity() {
        if (my_mail) delete my_mail;
    }
    
    void init(thread_t tid) {
        my_self = tid;
        my_mail = new io_mailbox(tid, this);
    }
    
    virtual thread_t get_owner() { return owner; }
    
    virtual int do_read(void* buf, unsigned bufsize, unsigned* count) = 0;
    virtual int do_write(void* buf, unsigned bufsize, unsigned* count) = 0;
    bool is_special() { return special; }

	bool is_buffer_ready( void );

    int self() { return my_self; }
};

#endif
