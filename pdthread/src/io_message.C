#include "io_message.h"
#include "thrtab.h"
#include <assert.h>

namespace pdthr
{

thread_t io_message::deliver(tag_t* tagp, void* buf, unsigned* countp) {
    assert(tagp && (*tagp == MSG_TAG_ANY || *tagp == this->tag));
    unsigned count = countp ? * countp : 0;
    if(count && !(source->is_special())) {
        /* non-"special" descriptor; apropos action is to read() 
           up to count bytes into buf */
        io_entity* ie = (io_entity*)thrtab::get_entry(this->sender);
        ie->do_read(buf, count, countp);
        *tagp = this->tag;
        return this->sender;
    } else {
        /* "special" descriptor; we're only going to notify the callee
           of message availability */
        *tagp = this->tag;
        return this->sender;
    }    
}

unsigned io_message::deliver_to(io_entity* ie) {
    fprintf(stderr, "panic: bogus operation: attempting io_message::deliver_to()\n");
    
    assert(false);
    return 0;
}

} // namespace pdthr
