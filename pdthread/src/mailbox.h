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

#ifndef __libthread_mailbox_h__
#define __libthread_mailbox_h__

#include "refarray.C"
#include "dllist.C"
#include "predicate.h"
#include "../h/thread.h"
#include "xplat/Mutex.h"


namespace pdthr
{

class message;

class mailbox {
  private:
    static refarray<mailbox>* registry;
    static void register_mbox(thread_t owner, mailbox* which);
    
  protected:
    thread_t owned_by;

	// lock for mutual exclusion to the mailbox message queue(s)
    XPlat::Mutex qmutex;
    
  public:
    mailbox(thread_t owner);
    virtual ~mailbox();
    
    static void dump_mailboxes();
    void dump();
    void dump_meta();
    virtual void dump_state();
    
    /* mailbox::for_thread() returns the appropriate mailbox for tid */
    static mailbox* for_thread(thread_t tid);

    /* put() delivers message m into this mailbox */
    virtual int put(message* m) = 0;

    /* recv() receives mail from this mailbox into buf;
       see the libthread documentation for the full
       meanings of the args */
    virtual int recv(thread_t* sender, tag_t* tagp, void* buf, unsigned* countp) = 0;

    // same as the above recv, but transfers ownership of
    // the buffer to the receiver
    virtual int recv(thread_t* sender, tag_t* tagp, void** buf) = 0;

    /* poll() checks for suitable available messages */
    virtual int poll(thread_t* from, tag_t* tagp, unsigned block, unsigned fd_first=0) = 0;
};

} // namespace pdthr

#endif
