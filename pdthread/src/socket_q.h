#ifndef __libthread_socket_q_h__
#define __libthread_socket_q_h__

#ifndef __in_thrtabentries__
#error You should not include this file directly
#endif

#include "../h/thread.h"
#include "pthread_sync.h"
#include "hashtbl.C"

class socket_q : public io_entity {
  private:
    static hashtbl<PDSOCKET,socket_q*,pthread_sync> socket_registry;    
  public:
    socket_q(PDSOCKET the_sock, thread_t owned_by,
             int (*will_block_func)(void*), void* desc);

    virtual ~socket_q();
    
    virtual item_t gettype() { return item_t_socket; }
    static socket_q* socket_from_desc(PDSOCKET fd);
    PDSOCKET sock;

    virtual int do_read(void* buf, unsigned bufsize, unsigned* count);
    virtual int do_write(void* buf, unsigned bufsize, unsigned* count);
}; /* end of class socket_q */

#endif
