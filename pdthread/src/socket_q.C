#include "thrtab_entries.h"

namespace pdthr
{

hashtbl<PdSocket,socket_q*> socket_q::socket_registry("PdSocket","socket_q*","socket_registry");

socket_q::socket_q( PdSocket the_sock,
                    thread_t owned_by, 
                    int (*will_block_func)(void*),
                    void* desc,
                    bool is_special)
  : io_entity(owned_by, will_block_func, desc, is_special),
    sock(the_sock)
{
    socket_registry.put(the_sock, this);
}

socket_q::~socket_q() {
    socket_registry.put(this->sock, NULL);
}

socket_q* socket_q::socket_from_desc( const PdSocket& the_sock) {
    return socket_registry.get(the_sock);
}

int socket_q::do_read(void* buf, unsigned bufsize, unsigned* count) {
    // this function completely taken from old libthread
    // --willb, 18 Feb 2002
    int ret;
    
    ret = recv( sock.s, (char*)buf, bufsize, 0 );
    if( ret != PDSOCKET_ERROR ) {
        *count = (unsigned)ret;
    }
    return (ret != PDSOCKET_ERROR) ? THR_OKAY : PDSOCKET_ERRNO;
}

int socket_q::do_write(void* buf, unsigned bufsize, unsigned* count) {
    // this function completely taken from old libthread
    // --willb, 18 Feb 2002 
    int ret;

    ret = send( sock.s, (const char*)buf, bufsize, 0 );
    if( ret != PDSOCKET_ERROR ) {
        *count = (unsigned)ret;
    }
    return (ret != PDSOCKET_ERROR) ? THR_OKAY : PDSOCKET_ERRNO;
}

} // namespace pdthr
