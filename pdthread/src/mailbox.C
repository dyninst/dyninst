#include "mailbox.h"
#include "refarray.h"
#include "predicate.h"
#include "message_predicates.h"
#include <stdio.h>
#include <sys/types.h>
#include "mailbox_defs.h"
#include "thrtab.h"
#include "thrtab_entries.h"
#include "xplat/h/Mutex.h"

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

