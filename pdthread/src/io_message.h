#ifndef __libthread_io_message_h__
#define __libthread_io_message_h__

#include "message.h"

namespace pdthr
{

/* This class represents a message coming from a file or socket; it
   can be used either for "special" or non-"special" files */

class io_message : public message {
  private:
    io_entity* source;

  public:
    io_message(io_entity* from, thread_t sender,
               tag_t tag, void* buf = NULL, unsigned count = 0)
            : message(sender, tag, buf, count), source(from) {}
    virtual ~io_message() {}
    virtual thread_t deliver(tag_t* tagp, void* buf, unsigned* countp);
    virtual unsigned deliver_to(io_entity* ie);
};

} // namespace pdthr

#endif
