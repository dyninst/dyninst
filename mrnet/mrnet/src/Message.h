#if !defined(Message_h)
#define Message_h

#include <list>
#include <vector>

#include <sys/socket.h>

#include "mrnet/src/Packet.h"

namespace MRN
{

class Message
{
    std::list < Packet >packets;

 public:
    int send( int sock_fd );
    int recv( int sock_fd, std::list < Packet >&packets, RemoteNode * );
    void add_Packet( Packet & );
    int size_Packets(  );
    int size_Bytes(  );
};

int read( int fd, void *buf, int size );
int readmsg( int fd, struct msghdr *msg );
int write( int fd, const void *buf, int size );

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
