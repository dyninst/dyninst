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

/* -*- mode: c++; c-basic-indent: 4 -*- */
#ifndef __libthread_io_entity_h__
#define __libthread_io_entity_h__

#include "mailbox.h"
#include "io_mailbox.h"
#include "entity.h"

namespace pdthr
{

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
    
    virtual thread_t get_owner( void )  const   { return owner; }
    
    virtual int do_read(void* buf, unsigned bufsize, unsigned* count) = 0;
    virtual int do_write(void* buf, unsigned bufsize, unsigned* count) = 0;
    bool is_special( void ) const   { return special; }

	bool is_buffer_ready( void );

    int self( void ) const          { return my_self; }
};

} // namespace pdthr

#endif
