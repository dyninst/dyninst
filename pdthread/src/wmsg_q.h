#ifndef __libthread_wmsg_q_h__
#define __libthread_wmsg_q_h__

#if !defined(i386_unknown_nt4_0)
#error fatal: compiling windows message queue support on non-windows
#endif

#include "pdthread/src/io_entity.h"

namespace pdthr
{

class wmsg_q : public io_entity
{
public:
    wmsg_q( thread_t owned_by )
      : io_entity( owned_by, NULL, NULL, true ) // special, so we dequeue items
    { }
    virtual ~wmsg_q( void ) { }

    virtual item_t gettype() { return item_t_wmsg; }

    virtual int do_read( void* buf, unsigned int bufsize, unsigned int* count );
    virtual int do_write( void* buf, unsigned int bufsize, unsigned int* count );
}; /* end of class tid_wmsg */

} // namespace pdthr

#endif 
