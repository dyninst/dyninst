/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

/*===========================================================*/
/*             StreamImpl CLASS METHOD DEFINITIONS            */
/*===========================================================*/

#include <stdarg.h>
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/NetworkImpl.h"
#include "mrnet/src/utils.h"

namespace MRN
{

unsigned int StreamImpl::next_stream_id=1;  //id '0' reserved for broadcast
bool StreamImpl::force_network_recv=false;

StreamImpl::StreamImpl(Network * _network, Communicator *_comm,
                       int _sync_id, int _ds_filter_id, int _us_filter_id)
  : network(_network), ds_filter_id(_ds_filter_id),
    us_filter_id(_us_filter_id),
    sync_id(_sync_id), stream_id( next_stream_id )
{
    next_stream_id++;
    communicator = new Communicator(*_comm); //copy the comm.

    if ( network->is_FrontEnd() ){
        const std::vector <EndPoint*> endpoints = communicator->get_EndPoints();
        int * backends = new int[ endpoints.size() ];
        unsigned int i;
        
        mrn_printf(4, MCFL, stderr, "Adding backends to stream %d: [ ",
                   stream_id);
        for(i=0; i<endpoints.size(); i++){
            mrn_printf(4, 0,0, stderr, "%d, ", endpoints[i]->get_Id() );
            backends[i] = endpoints[i]->get_Id();
        }
        mrn_printf(4, 0,0, stderr, "]\n");

        Packet packet( 0, PROT_NEW_STREAM, "%d %ad %d %d %d",
                       stream_id, backends, endpoints.size(),
                       sync_id, ds_filter_id, us_filter_id );
        StreamManager * stream_mgr;
        stream_mgr = network->get_FrontEndNode()->proc_newStream(packet);
        network->get_FrontEndNode()->send_newStream(packet, stream_mgr);
        delete [] backends;
    }
}
    
StreamImpl::StreamImpl(Network * _network, int _stream_id,
                       int * backends, int num_backends, int _sync_id,
                       int _ds_filter_id, int _us_filter_id)
  : network(_network),
    ds_filter_id(_ds_filter_id),
    us_filter_id(_us_filter_id),
    sync_id(_sync_id),
    stream_id(_stream_id)
{
}

StreamImpl::~StreamImpl()
{
}

int StreamImpl::send_aux(int tag, char const * fmt, va_list arg_list ) const
{
    mrn_printf(3, MCFL, stderr, "DCA: StreamImpl::send_aux() Creating new packet with stream_id: %d", stream_id);
    Packet packet(stream_id, tag, fmt, arg_list);
    if(packet.fail()){
        mrn_printf(1, MCFL, stderr, "new packet() fail\n");
        return -1;
    }
    mrn_printf(3, MCFL, stderr, "new packet() succeeded. Calling frontend.send()\n");
    int status = network->send(packet);
    return status;
}


int StreamImpl::send(int tag, char const * fmt, ...) const
{
    int status;
    va_list arg_list;

    mrn_printf(3, MCFL, stderr, "In stream[%d].send(). Calling new packet()\n", stream_id);

    va_start(arg_list, fmt);
    status = send_aux( tag, fmt, arg_list );
    va_end(arg_list);

    mrn_printf(3, MCFL, stderr, "stream[%d].send() %s", stream_id,
               (status==-1 ? "failed\n" : "succeeded\n"));

    return status;
}

int StreamImpl::recv(int *tag, void ** ptr, bool blocking)
{
    Packet cur_packet;

    mrn_printf(3, MCFL, stderr, "In StreamImpl::recv().\n");

    if( force_network_recv || IncomingPacketBuffer.size() == 0 ){
        network->recv( false );
    }

    if( IncomingPacketBuffer.size() == 0  && !blocking ){
        //No packets, but non-blocking
        mrn_printf(3, MCFL, stderr, "No packets currently on stream\n");
        return 0;
    }
    else{
        while( IncomingPacketBuffer.size() == 0 ){
            //Loop until we get a packet on this stream's incoming buffer.
            if( network->recv( true ) == -1 ){
                mrn_printf(1, MCFL, stderr, "Network::recv failed.");
            }
        }
        cur_packet = *( IncomingPacketBuffer.begin() );
        IncomingPacketBuffer.remove(cur_packet);
        *tag = cur_packet.get_Tag();
        *ptr = (void *) new Packet(cur_packet);
    }

    mrn_printf(4, MCFL, stderr, "packet's tag: %d\n", cur_packet.get_Tag());
    mrn_printf(3, MCFL, stderr, "Leaving StreamImpl::recv().\n");
    return 1;
}

Packet StreamImpl::get_IncomingPacket( )
{
    Packet cur_packet=*Packet::NullPacket;

    if( IncomingPacketBuffer.size() != 0 ){
        cur_packet = *( IncomingPacketBuffer.begin() );
        IncomingPacketBuffer.pop_front();
    }

    return cur_packet;
}

} // namespace MRN
