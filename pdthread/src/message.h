/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

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
