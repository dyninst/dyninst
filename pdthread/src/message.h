#ifndef __libthread_message_h__
#define __libthread_message_h__

#include <stdlib.h>
#include <string.h>

#include "../h/thread.h"
#include "thrtab_entries.h"

namespace pdthr
{

class io_entity;

class message {
  protected:
    thread_t sender;
    tag_t tag;
    unsigned size;
    void* buf;
  public:
    
    message(thread_t sender, tag_t tag, void* buf, unsigned count = 0) {
        this->sender = sender;
        this->tag = tag;
        if(count != 0) {
            // NB: assuming that char is 8 bits
            this->buf = (void*)new char[count];
            this->buf = memcpy(this->buf, buf, count);
        }
        else if( buf != NULL )
        {
            // we take ownership of the buffer but we leave size 
            // at 0 so that we don't try to deallocate it when we
            // are destroyed
            this->buf = buf;
        }
        this->size = count;
    }
    
    virtual ~message() {
        // only release the buffer if we allocated it
        // (indicated by a non-zero size)
        if(size != 0)
        {
            delete [] (char*)buf;
        }
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

    // same as the above but transfers ownership of the buffer
    virtual thread_t deliver(tag_t* tagp, void** buf);

    virtual unsigned deliver_to(io_entity* ie);
    
    inline tag_t type() {
        return tag;
    }
    
    inline thread_t from() {
        return sender;
    }

    void dump(const char* prefix);
};

} // namespace pdthr

#endif
