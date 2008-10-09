/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(Message_h)
#define Message_h

#include <list>
#include <vector>

#include "Error.h"
#include "mrnet/Types.h"
#include "mrnet/Packet.h"
#include "xplat/Monitor.h"

namespace MRN
{

uint64_t get_TotalBytesSend(void);
uint64_t get_TotalBytesRecv(void);

class Message: public Error{
  public:
   Message( void );
   int send( int isock_fd );
   int recv( int isock_fd, std::list < PacketPtr >&opackets, Rank iinlet_rank );
   void add_Packet( const PacketPtr );
   int size_Packets( void );
   int size_Bytes( void );
   void waitfor_MessagesToSend( void );

 private:
   enum {MRN_QUEUE_NONEMPTY};
   std::list < PacketPtr > _packets;
   XPlat::Monitor _packet_sync;
};

#if READY
int read( int fd, void *buf, int size );
int write( int fd, const void *buf, int size );
#endif // READY

}                               // namespace MRN

#endif                          /* Message_h */
