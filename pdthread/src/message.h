#ifndef __libthread_message_h__
#define __libthread_message_h__

#include <stdlib.h>
#include <string.h>

#include "../h/thread.h"
#include "thrtab_entries.h"

class io_entity;

class message {
  protected:
    thread_t sender;
    tag_t tag;
    unsigned size;
    void* buf;
  public:
    
    message(thread_t sender, tag_t tag, void* buf, unsigned count) {
        this->sender = sender;
        this->tag = tag;
        if(count != 0) {
            // NB: assuming that char is 8 bits
            this->buf = (void*)new char[count];
            this->buf = memcpy(this->buf, buf, count);
        }
        this->size = count;
    }
    
    virtual ~message() {
        if(size != 0)
            delete [] (char*)buf;
    }
    
    /**
       deliver is the method that is invoked when this
       message is recv'd.  It has some of the overloaded
       interface delight of msg_recv itself (see the 
       libthread documentation)

       Briefly:
         if *tagp == MSG_TAG_ANY, *tagp will be set 
         to this message's tag type.
         otherwise, *tagp must equal this->tag.

         *countp holds the number of bytes the receiver
         expects (this number must be lte the size of *buf).
         At most *countp bytes are copied into *buf, and
         *countp is set to this->size if this->size is less
         than *countp

         the return value is this->sender
    */
    virtual thread_t deliver(tag_t* tagp, void* buf, unsigned* countp);

    virtual unsigned deliver_to(io_entity* ie);
    
    inline tag_t type() {
        return tag;
    }
    
    inline thread_t from() {
        return sender;
    }
};

#endif
