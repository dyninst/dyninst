/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#if !defined(__streamimpl_h)
#define __streamimpl_h 1

#include <list>
#include <vector>
#include <map>

#include "mrnet/MRNet.h"
#include "mrnet/src/Message.h"
#include "mrnet/src/CommunicatorImpl.h"
#include "mrnet/src/FrontEndNode.h"
#include "mrnet/src/BackEndNode.h"
#include "mrnet/src/NetworkImpl.h"

namespace MRN
{

class StreamImpl{
    friend class Network;

 private:
    Network * network;
    static bool force_network_recv;
    static unsigned int next_stream_id;

    std::list < Packet >IncomingPacketBuffer;
    int ds_filter_id;
    int us_filter_id;
    int sync_id;
    unsigned short stream_id;
    Communicator *communicator;

 public:
    StreamImpl( Network *, Communicator *,
                int us_filter_id=TFILTER_NULL,
                int sync_id=SFILTER_WAITFORALL,
                int ds_filter_id=TFILTER_NULL );
    StreamImpl( Network *, int stream_id, int *backends = 0,
                int num_backends = -1,
                int us_filter_id=TFILTER_NULL,
                int sync_id = SFILTER_DONTWAIT,
                int ds_filter_id=TFILTER_NULL );
    ~StreamImpl( );
    static int recv( int *tag, void **buf, Stream ** stream, bool blocking =
                     true );
    static void set_ForceNetworkRecv( bool force = true );

    void add_IncomingPacket( Packet& );
    Packet get_IncomingPacket( );
    const std::vector < EndPoint * >&get_EndPoints(  ) const;


    int send_aux( int tag, const char *format_str, va_list arg_list ) const;
    static int unpack( void *buf, const char *fmt, va_list arg_list );

//    int send( int tag, const char *format_str, ... ) const;
    int flush(  ) const;
    int recv( int *tag, void **buf, bool blocking = true );
    unsigned int get_NumEndPoints(  ) const;
    Communicator *get_Communicator( void ) const;
    unsigned int get_Id( void ) const ;
};

inline int StreamImpl::flush() const
{
    if( network->is_FrontEnd() ){
        mrn_printf( 3, MCFL, stderr, "calling frontend flush()" );
        return network->get_FrontEndNode()->flush(stream_id);
    }
    else if( network->is_BackEnd() ){
        mrn_printf( 3, MCFL, stderr, "calling backend flush()" );
        return network->get_BackEndNode()->flush();
    }
    else{
        assert( 0 && "Cannot call flush without front/backend init'd");
    }
    return 0;
}


inline unsigned int StreamImpl::get_NumEndPoints() const
{
    return communicator->size();
}

inline void StreamImpl::set_ForceNetworkRecv( bool force )
{
    force_network_recv = force;
}

inline void StreamImpl::add_IncomingPacket(Packet& packet)
{
    IncomingPacketBuffer.push_back(packet);
}

inline const std::vector <EndPoint *> & StreamImpl::get_EndPoints( ) const
{
    return communicator->get_EndPoints( );
}

inline int StreamImpl::unpack( void* buf, const char* fmt_str,
                               va_list arg_list )
{
    Packet * packet = (Packet *)buf;
    int ret = packet->ExtractVaList(fmt_str, arg_list); 
    delete packet;

    return ret;
}

inline Communicator * StreamImpl::get_Communicator( void ) const
{
    return communicator;
}

inline unsigned int StreamImpl::get_Id( void ) const 
{
    return stream_id;
}
} // namespace MRN

#endif /* __streamimpl_h */
