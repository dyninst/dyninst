/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#if !defined(Message_h)
#define Message_h

#include <list>
#include <vector>

#include "mrnet/src/Packet.h"
#include "mrnet/src/Error.h"

namespace MRN
{

class Message: public Error{
    std::list < Packet >packets;

 public:
    int send( int sock_fd );
    int recv( int sock_fd, std::list < Packet >&packets, RemoteNode * );
    void add_Packet( Packet & );
    int size_Packets(  );
    int size_Bytes(  );
};

#if READY
int read( int fd, void *buf, int size );
int write( int fd, const void *buf, int size );
#endif // READY

inline void Message::add_Packet( Packet& packet )
{
    packets.push_back( packet );
}

inline int Message::size_Packets(  )
{
    return packets.size(  );
}

inline int Message::size_Bytes(  )
{
    assert( 0 );
    return 0;
}

}                               // namespace MRN

#endif                          /* Message_h */
