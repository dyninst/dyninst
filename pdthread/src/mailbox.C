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

#include "mailbox.h"
#include "refarray.h"
#include "predicate.h"
#include "message_predicates.h"
#include <stdio.h>
#include <sys/types.h>
#include "mailbox_defs.h"
#include "thrtab.h"
#include "thrtab_entries.h"
#include "xplat/Mutex.h"

#if !defined(i386_unknown_nt4_0)
#include <unistd.h>
#else

#endif // !defined(i386_unknown_nt4_0)


namespace pdthr
{

refarray<mailbox>* mailbox::registry = NULL;


void mailbox::dump_mailboxes() {
    mailbox *cur = NULL;
    for (int i = 0; i < 256; i++) {
        (cur = mailbox::for_thread(i));
      if(cur) cur->dump();
  }  
}

void mailbox::dump() {
    this->dump_meta();
    this->dump_state();
}

void mailbox::dump_meta() {
    entity* e = thrtab::get_entry(owned_by);
    static const char* noname = "no name";
    const char* name;
    if (e && e->gettype() == item_t_thread)
        name = ((lwp*)e)->get_name();
    else
        name = noname;
        
    fprintf(stderr, "Mailbox state for thread %d (%s):\n", owned_by, name);
}

void mailbox::dump_state() {
    fprintf(stderr, "   base class (mailbox.[Ch]); no state\n");
}

mailbox* mailbox::for_thread(thread_t tid) {
    return (*registry)[tid];
}

void mailbox::register_mbox(thread_t owner, mailbox* which) {
    if (!mailbox::registry)
        mailbox::registry = new refarray<mailbox>(2048,NULL);
    mailbox::registry->set_elem_at(owner, which);    
}

mailbox::mailbox(thread_t owner) {
    owned_by = owner;
    mailbox::register_mbox(owner, this); 
}

mailbox::~mailbox() {
}

} // namespace pdthr

