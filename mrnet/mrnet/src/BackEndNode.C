/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include <stdio.h>
#include <arpa/inet.h>

#include "mrnet/src/BackEndNode.h"
#include "mrnet/src/RemoteNode.h"
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/utils.h"

namespace MRN
{

/*=====================================================*/
/*  BackEndNode CLASS METHOD DEFINITIONS            */
/*=====================================================*/

BackEndNode::BackEndNode(std::string _hostname,
                         unsigned short _backend_id,
                         std::string _parent_hostname,
                         unsigned short _parent_port,
                         unsigned short _parent_id)
    :ChildNode(false, _hostname, _backend_id), 
     CommunicationNode(_hostname, 0, _backend_id)
{
    RemoteNode::local_child_node = this;
    mrn_printf(3, MCFL, stderr, "In BackEndNode() cnstr.\n");
    mrn_printf(4, MCFL, stderr,
               "host=%s, backendId=%u, parHost=%s, parPort=%u, "
               "parRank=%u\n",
               _hostname.c_str(), _backend_id, _parent_hostname.c_str(),
               _parent_port, _parent_id );
    upstream_node = new RemoteNode(false, _parent_hostname, _parent_port,
                                   _parent_id);

    upstream_node->_is_upstream = true;
    upstream_node->connect();
    if(upstream_node->fail() ){
        mrn_printf(1, MCFL, stderr, "connect() failed\n");
        return;
    }
  
    // do low-level handshake with our id in the backend id namespace
    uint32_t idBuf = htonl(id);
    int sret = ::send( upstream_node->get_sockfd(), &idBuf, 4, 0 );
    if( sret == -1 ) {
        mrn_printf(1, MCFL, stderr, "send of backend id failed\n");
    }

    mrn_printf(3, MCFL, stderr, "Leaving BackEndNode()\n");
}

BackEndNode::~BackEndNode(void)
{
    delete upstream_node;
}

int BackEndNode::proc_PacketsFromUpStream(std::list <Packet> &packets)
{
    int retval=0;
    Packet cur_packet;

    mrn_printf(3, MCFL, stderr, "In proc_PacketsFromUpStream()\n");

    std::list<Packet>::iterator iter=packets.begin();
    for(; iter != packets.end(); iter++){
        cur_packet = (*iter);
        switch(cur_packet.get_Tag()){
        case PROT_DATA:
        case PROT_NEW_SUBTREE:
        case PROT_DEL_SUBTREE:
        case PROT_RPT_SUBTREE:
        case PROT_NEW_APPLICATION:
        case PROT_DEL_APPLICATION:
        case PROT_DEL_STREAM:
        case PROT_GET_LEAF_INFO:
        case PROT_CONNECT_LEAVES:
            //these protocol tags should never reach backend
            mrn_printf(1, MCFL,stderr,"BackEndNode::proc_DataFromUpStream(): "
                       "poison tag: %d\n", cur_packet.get_Tag());
            assert(0);
            break;

        case PROT_NEW_STREAM:
            if(proc_newStream(cur_packet) == -1){
                mrn_printf(1, MCFL, stderr, "proc_newStream() failed\n");
                retval = -1;
            }
            break;

        default:
            //Any Unrecognized tag is assumed to be data
            if(proc_DataFromUpStream(cur_packet) == -1){
                mrn_printf(1, MCFL, stderr, "proc_DataFromUpStream() failed\n");
                retval=-1;
            }
            break;
        }
    }

    packets.clear();
    mrn_printf(3, MCFL, stderr, "proc_PacketsFromUpStream() %s",
               (retval == -1 ? "failed\n" : "succeeded\n"));
    return retval;
}

int BackEndNode::proc_DataFromUpStream(Packet& packet)
{
    StreamImpl * stream;

    mrn_printf(3, MCFL, stderr, "In proc_DataFromUpStream()\n");

    stream = StreamImpl::get_Stream(packet.get_StreamId());

    if( stream ){
        stream->add_IncomingPacket(packet);
    }
    else{
        stream = new StreamImpl(packet.get_StreamId());
        stream->add_IncomingPacket(packet);
    }
    mrn_printf(3, MCFL, stderr, "Leaving proc_DataFromUpStream()\n");
    return 0;
}

int BackEndNode::send(Packet& packet)
{
    mrn_printf(3, MCFL, stderr, "In backend.send(). Calling sendUpStream()\n");
    return send_PacketUpStream(packet);
}

int BackEndNode::flush()
{
    mrn_printf(3, MCFL, stderr, "In backend.flush(). Calling flushUpStream()\n");
    return flush_PacketsUpStream();
}

int BackEndNode::recv( bool blocking )
{
    std::list <Packet> packet_list;
    mrn_printf(3, MCFL, stderr, "In backend.recv(). Calling recvfromUpStream()\n");

    if(recv_PacketsFromUpStream(packet_list) == -1){
        mrn_printf(1, MCFL, stderr, "recv_packetsfromUpStream() failed\n");
        return -1;
    }

    if(packet_list.size() == 0){
        mrn_printf(3, MCFL, stderr, "No packets read!\n");
        return 0;
    }

    if(proc_PacketsFromUpStream(packet_list) == -1){
        mrn_printf(1, MCFL, stderr, "proc_packetsfromUpStream() failed\n");
        return -1;
    }

    mrn_printf(3, MCFL, stderr, "Leaving backend.recv().\n");
    return 1;
}


int BackEndNode::proc_newStream(Packet& pkt)
{
    mrn_printf( 3, MCFL, stderr, "In proc_newStream()\n" );

    // extract the info needed to build the stream
    int stream_id = -1;
    int* backends = NULL;
    unsigned int num_backends = 0;
    int sync_id = -1;
    int ds_filter_id = -1;
    int us_filter_id = -1;
    int uret = pkt.ExtractArgList( "%d %ad %d %d %d",
                                    &stream_id,
                                    &backends, &num_backends,
                                    &sync_id,
                                    &ds_filter_id,
                                    &us_filter_id );
    if( uret == -1 ) {
        mrn_printf( 1, MCFL, stderr, "ExtractArgList() failed\n" );
        return -1;
    }

    // Build a new stream object.
    // (As a side effect, registers the stream.)
    (void)new StreamImpl( stream_id,
                          backends,
                          num_backends,
                          sync_id,
                          ds_filter_id,
                          us_filter_id );

    mrn_printf( 3, MCFL, stderr, "procNewStream() succeeded\n" );
    return 1;
}

} // namespace MRN
