#include "message.h"
#include "../h/thread.h"
#include <assert.h>
#include <string.h>

namespace pdthr
{

thread_t message::deliver(tag_t* tagp, void* buf, unsigned* countp) {
    assert(*tagp == MSG_TAG_ANY || *tagp == this->tag);

    unsigned count = countp ? *countp : 0;
     
    if (count > this->size)
        count = this->size;

    if(*tagp == MSG_TAG_ANY)
        *tagp = this->tag;
    
    memcpy(buf, this->buf, count);
    
    return this->sender;
}

thread_t message::deliver(tag_t* tagp, void** buf) {
    assert(*tagp == MSG_TAG_ANY || *tagp == this->tag);

    if(*tagp == MSG_TAG_ANY)
        *tagp = this->tag;
    
    assert( buf != NULL );
    *buf = this->buf;
    this->buf = NULL;
    
    return this->sender;
}

unsigned message::deliver_to(io_entity* ie) {
    unsigned ret;
    ie->do_write(this->buf, this->size, &ret);
    return ret;
}

void message::dump(const char* prefix) {
    fprintf(stderr, "%stid = %d, tag = %d, size = %d\n", prefix, sender, tag, size);
}

} // namespace pdthr

