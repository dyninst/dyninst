#ifndef __libthread_file_q_h__
#define __libthread_file_q_h__

#if !defined(__in_thrtabentries__) && !defined(__in_file_q__)
#error You should not include this file directly
#endif

#include "../h/thread.h"
#include "pthread_sync.h"
#include "hashtbl.C"

class file_q : public io_entity {
  private:
    static hashtbl<PDDESC,file_q*,pthread_sync> file_registry;
  public:
    file_q(PDDESC the_fd, thread_t owned_by, int (*will_block_func)(void*), void* desc, bool is_special=false);
    virtual ~file_q();
    virtual item_t gettype() { return item_t_file; }
    static file_q* file_from_desc(PDDESC fd);
    PDDESC fd;

    virtual int do_read(void* buf, unsigned bufsize, unsigned* count);
    virtual int do_write(void* buf, unsigned bufsize, unsigned* count);
}; /* end of class file_q */

#endif
