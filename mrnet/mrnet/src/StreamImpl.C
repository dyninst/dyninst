/*===========================================================*/
/*             StreamImpl CLASS METHOD DEFINITIONS            */
/*===========================================================*/

#include <stdarg.h>
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/NetworkImpl.h"
#include "mrnet/src/utils.h"

namespace MRN
{

unsigned int StreamImpl::cur_stream_idx=0;
std::map <unsigned int, StreamImpl *>* StreamImpl::streams = NULL;
bool StreamImpl::force_network_recv=false;

StreamImpl::StreamImpl(Communicator *_comm, int _sync_id,
                        int _ds_filter_id, int _us_filter_id)
  : ds_filter_id(_ds_filter_id),
    us_filter_id(_us_filter_id),
    sync_id(_sync_id)
{
    static unsigned int next_stream_id=1;  //id '0' reserved for broadcast
    communicator = new CommunicatorImpl(*_comm); //copy the comm.

    stream_id = next_stream_id++;

    if( StreamImpl::streams == NULL ) {
        StreamImpl::streams = new std::map<unsigned int, StreamImpl*>;
    }
    (*StreamImpl::streams)[stream_id] = this;

    if ( Network::network ){
        const std::vector <EndPoint*>* endpoints =
            communicator->get_EndPoints();
        int * backends = new int[endpoints->size()];
        unsigned int i;
        
        mrn_printf(4, MCFL, stderr, "Adding backends to stream %d: [ ",
                   stream_id);
        for(i=0; i<endpoints->size(); i++){
            mrn_printf(4, 0,0, stderr, "%d, ", (*endpoints)[i]->get_Id() );
            backends[i] = (*endpoints)[i]->get_Id();
        }
        mrn_printf(4, 0,0, stderr, "]\n");

        Packet packet(0, MRN_NEW_STREAM_PROT, "%d %ad %d %d %d",
                      stream_id, backends, endpoints->size(),
                      sync_id, ds_filter_id, us_filter_id);
        StreamManager * stream_mgr;
        stream_mgr = Network::network->front_end->proc_newStream(packet);
        Network::network->front_end->send_newStream(packet, stream_mgr);
    }
}
    
StreamImpl::StreamImpl(int _stream_id, int * backends, int num_backends,
                     int _sync_id, int _ds_filter_id, int _us_filter_id)
  : ds_filter_id(_ds_filter_id),
    us_filter_id(_us_filter_id),
    sync_id(_sync_id),
    stream_id(_stream_id)
{
    (*StreamImpl::streams)[stream_id] = this;
}

StreamImpl::~StreamImpl()
{
}

int StreamImpl::recv(int *tag, void **ptr, Stream **stream, bool blocking)
{
    bool checked_network = false;   // have we checked sockets for input?
    Packet cur_packet;

    mrn_printf(3, MCFL, stderr, "In StreamImpl::recv().\n");

    if( streams->empty() && NetworkImpl::is_FrontEnd() ){
      //No streams exist -- bad for FE
      mrn_printf(1, MCFL, stderr, "%s recv in FE when no streams "
         "exist\n", (blocking? "Blocking" : "Non-blocking") );
      return -1;
    }

    // check streams for input
get_packet_from_stream_label:
    if( !streams->empty() ) {
        unsigned int start_idx = cur_stream_idx;
        do{
            StreamImpl* cur_stream = (*StreamImpl::streams)[cur_stream_idx];
            if(!cur_stream){
                cur_stream_idx++;
                cur_stream_idx %= streams->size();
                continue;
            }

            if( cur_stream->IncomingPacketBuffer.size() != 0 ){
                mrn_printf( 3, MCFL, stderr,
                    "Checking stream[%d] ...", cur_stream_idx);
                mrn_printf( 3, 0, 0, stderr, "found %d packets\n",
                             (int)cur_stream->IncomingPacketBuffer.size() );

                cur_packet = *( cur_stream->IncomingPacketBuffer.begin() );
                cur_stream->IncomingPacketBuffer.pop_front();
                cur_stream_idx++;
                cur_stream_idx %= streams->size();

                break;
            }
            mrn_printf(3, 0, 0, stderr, "found 0 packets\n");

            cur_stream_idx++;
            cur_stream_idx %= streams->size();
        } while(start_idx != cur_stream_idx);
    }

    if( cur_packet != *Packet::NullPacket ) {
        *tag = cur_packet.get_Tag();
        *stream = (*StreamImpl::streams)[cur_packet.get_StreamId()];
        *ptr = (void *) new Packet(cur_packet);
        mrn_printf(4, MCFL, stderr, "cur_packet tag: %d, fmt: %s\n",
                   cur_packet.get_Tag(), cur_packet.get_FormatString() );
        return 1;
    }
    else if( blocking || !checked_network ) {

        // No packets are already in the stream
        // check whether there is data waiting to be read on our sockets
        if( Network::network->recv( blocking ) == -1 ){
            mrn_printf( 1, MCFL, stderr, "Network::recv() failed.\n" );
            return -1;
        }
        checked_network = true;

        // go back to check whether we found enough to make a complete packet
        goto get_packet_from_stream_label;
    }

    assert( !blocking );
    assert( checked_network );
    return 0;
}

int StreamImpl::send_aux(int tag, char const * fmt, va_list arg_list )
{
    Packet packet(stream_id, tag, fmt, arg_list);
    if(packet.fail()){
        mrn_printf(1, MCFL, stderr, "new packet() fail\n");
        return -1;
    }
    mrn_printf(3, MCFL, stderr, "new packet() succeeded. Calling frontend.send()\n");
    int status = Network::network->send(packet);
    return status;
}


int StreamImpl::send(int tag, char const * fmt, ...)
{
    int status;
    va_list arg_list;

    mrn_printf(3, MCFL, stderr, "In stream[%d].send(). Calling new packet()\n", stream_id);

    va_start(arg_list, fmt);
    status = send_aux( tag, fmt, arg_list );
    va_end(arg_list);

    mrn_printf(3, MCFL, stderr, "network.send() %s",
               (status==-1 ? "failed\n" : "succeeded\n"));
    return status;
}

int StreamImpl::recv(int *tag, void ** ptr, bool blocking)
{
    Packet cur_packet;

    mrn_printf(3, MCFL, stderr, "In StreamImpl::recv().\n");

    if( force_network_recv || IncomingPacketBuffer.size() == 0 ){
        Network::network->recv( false );
    }

    if( IncomingPacketBuffer.size() == 0  && !blocking ){
        //No packets, but non-blocking
        mrn_printf(3, MCFL, stderr, "No packets currently on stream\n");
        return 0;
    }
    else{
        while( IncomingPacketBuffer.size() == 0 ){
            //Loop until we get a packet on this stream's incoming buffer.
            if( Network::network->recv( true ) == -1 ){
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

StreamImpl * StreamImpl::get_Stream(int stream_id)
{
    assert( streams != NULL );
    StreamImpl *stream = (*streams)[stream_id];
    if(stream){
        return stream;
    }
    else{
        streams->erase(stream_id);
        return NULL;
    }
}

} // namespace MRN
