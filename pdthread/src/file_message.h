#ifndef __libthread_io_message_h__
#define __libthread_io_message_h__

#include "message.h"

/* This class represents a message coming from a file or socket; it
   can be used either for "special" or non-"special" files */

class io_message : public message {
  private:
    io_entity* source;
    thr_mailbox* dest;

  public:
    io_message(io_entity* from, thr_mailbox* to, thread_t sender,
               tag_t tag, void* buf = NULL, unsigned count = 0)
            : message(sender, tag, buf, count), source(from),
              dest(to) {}
    

#endif
