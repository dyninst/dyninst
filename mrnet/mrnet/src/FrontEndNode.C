#include <stdio.h>
#include "mrnet/src/FrontEndNode.h"
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/utils.h"
/*======================================================*/
/*  FrontEndNode CLASS METHOD DEFINITIONS            */
/*======================================================*/

FrontEndNode::FrontEndNode(std::string _hostname, unsigned short _port)
 :ParentNode(false, _hostname, _port),
  CommunicationNode(_hostname, _port),
  leafInfoPacket( NULL ),
  leavesConnectedPacket( NULL )
{
  RemoteNode::local_parent_node = this;
}

FrontEndNode::~FrontEndNode()
{
}

int FrontEndNode::proc_DataFromDownStream(Packet *packet)
{
  mrn_printf(3, MCFL, stderr, "In frontend.proc_DataFromUpStream()\n");

  StreamManager * stream_mgr = StreamManagerById[ packet->get_StreamId() ];
  std::list<Packet *> packets;
  std::list<Packet *> ::iterator iter;

  stream_mgr->push_packet(packet, packets);

  if(packets.size() != 0){
    for(iter = packets.begin(); iter != packets.end(); iter++){
      Packet *cur_packet = *iter;
      StreamImpl * stream;
      stream = StreamImpl::get_Stream(cur_packet->get_StreamId());

      if( stream ){
        mrn_printf(3, MCFL, stderr, "Put packet in stream %d\n", cur_packet->get_StreamId());
        stream->add_IncomingPacket(cur_packet);
      }
      else{
        mrn_printf(1, MCFL, stderr, "Packet from unknown stream %d\n", cur_packet->get_StreamId());
        return -1;
      }
    }
  }

  return 0;
}

int FrontEndNode::proc_PacketsFromDownStream(std::list <Packet *> &packet_list)
{
  int retval=0;
  Packet *cur_packet;

  mrn_printf(3, MCFL, stderr, "In procPacketsFromDownStream()\n");

  std::list<Packet *>::iterator iter=packet_list.begin();
  for(; iter != packet_list.end(); iter++){
    cur_packet = (*iter);
    switch(cur_packet->get_Tag()){
    case MRN_RPT_SUBTREE_PROT:
      //printf(3, MCFL, stderr, "Calling proc_newSubTreeReport()\n");
      if(proc_newSubTreeReport(cur_packet) == -1){
	mrn_printf(1, MCFL, stderr, "proc_newSubTreeReport() failed\n");
	retval=-1;
      }
      //printf(3, MCFL, stderr, "proc_newSubTreeReport() succeeded\n");
      break;

    case MRN_GET_LEAF_INFO_PROT:
        if( proc_getLeafInfoResponse( cur_packet ) == -1 )
        {
            mrn_printf(1, MCFL, stderr, "proc_getLeafInfoResponse() failed\n");
            retval = -1;
        }
        break;

    case MRN_CONNECT_LEAVES_PROT:
        if( proc_connectLeavesResponse( cur_packet ) == -1 )
        {
            mrn_printf(1, MCFL, stderr, "proc_connectLeavesResponse() failed\n");
            retval = -1;
        }
        break;

    default:
      //Any unrecognized tag is assumed to be data
      //printf(3, MCFL, stderr, "Calling proc_DataFromDownStream(). Tag: %d\n",
                 //cur_packet->get_Tag());
      if(proc_DataFromDownStream(cur_packet) == -1){
	mrn_printf(1, MCFL, stderr, "proc_DataFromDownStream() failed\n");
	retval=-1;
      }
      //printf(3, MCFL, stderr, "proc_DataFromDownStream() succeeded\n");
    }
  }

  mrn_printf(3, MCFL, stderr, "proc_PacketsFromDownStream() %s",
             (retval == -1 ? "failed\n" : "succeeded\n"));
  packet_list.clear();
  return retval;
}

int FrontEndNode::send(Packet *packet)
{
  return send_PacketDownStream(packet);
}

int FrontEndNode::flush()
{
  return flush_PacketsDownStream();
}

int FrontEndNode::flush(unsigned int stream_id)
{
  return flush_PacketsDownStream(stream_id);
}

int FrontEndNode::recv( bool blocking )
{
  std::list <Packet *> packet_list;
  mrn_printf(3, MCFL, stderr, "In frontend.recv(). Calling recvfromdownstream()\n");


  if(recv_PacketsFromDownStream(packet_list) == -1){
    mrn_printf(1, MCFL, stderr, "recv_packetsfromdownstream() failed\n");
    return -1;
  }

  if(packet_list.size() == 0){
    mrn_printf(3, MCFL, stderr, "No packets read!\n");
    return 0;
  }

  if(proc_PacketsFromDownStream(packet_list) == -1){
    mrn_printf(1, MCFL, stderr, "proc_packetsfromdownstream() failed\n");
    return -1;
  }
  mrn_printf(3, MCFL, stderr, "Leaving frontend.recv()\n");

  return 1;
}


int
FrontEndNode::deliverConnectLeavesResponse( Packet* pkt )
{
    //
    // stash the aggregated response for subsequent retrieval
    // (It is assumed that our NetworkImpl is polling us till
    // we have received and stashed this packet.)
    //
    // Note that, if we are the front end *and* the leaf,
    // the packet we're given is the packet that we constructed ourself.
    // The packets we construct ourself have data elements that
    //
    //
    if( this->isLeaf() )
    {
        // we constructed the packet ourself -
        // the packet's data element array contains pointers
        // to data that may no longer be valid
        // 
        // we have to build a new packet that uses the
        // marshalled data in this packet's buffer
        //
        // or we have to have a way to reset the packet to say that
        // it should use the arg list to decode the packet

        // painful - having to marshal/unmarshal the data within
        // the same process.  But if we're pointing to data on the
        // stack, we don't have the original data anymore anyway.
        //
        leavesConnectedPacket = new Packet( pkt->get_BufferLen(),
                                        pkt->get_Buffer() );

        // release the given packet
        // TODO (is this safe?)
        delete pkt;
    }
    else
    {
        // we can use the packet as it is
        leavesConnectedPacket = pkt;
    }

    return 0;
}

int
FrontEndNode::deliverLeafInfoResponse( Packet* pkt )
{
    //
    // stash the aggregated response for subsequent retrieval
    // (It is assumed that our NetworkImpl is polling us till
    // we have received and stashed this packet.)
    //
    // Note that, if we are the front end *and* the leaf,
    // the packet we're given is the packet that we constructed ourself.
    // The packets we construct ourself have data elements that
    //
    //
    if( this->isLeaf() )
    {
        // we constructed the packet ourself -
        // the packet's data element array contains pointers
        // to data that may no longer be valid
        // 
        // we have to build a new packet that uses the
        // marshalled data in this packet's buffer
        //
        // or we have to have a way to reset the packet to say that
        // it should use the arg list to decode the packet

        // painful - having to marshal/unmarshal the data within
        // the same process.  But if we're pointing to data on the
        // stack, we don't have the original data anymore anyway.
        //
        unsigned int buflen = pkt->get_BufferLen();
        char* buf = pkt->get_Buffer();
        if( (buflen == 0) || (buf == NULL) )
        {
            fprintf( stderr, "FE::ParentNode: deliverleafinfo resp: empty buffer\n" );
        }
        leafInfoPacket = new Packet( buflen, buf );

        // release the given packet
        // TODO (is this safe?)
#if READY
        delete pkt;
#endif // READY
    }
    else
    {
        // we can use the packet as it is
        leafInfoPacket = pkt;
    }

    return 0;
}

