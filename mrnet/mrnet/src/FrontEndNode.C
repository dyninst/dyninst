#include <stdio.h>
#include "mrnet/src/FrontEndNode.h"
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/utils.h"
/*======================================================*/
/*  MC_FrontEndNode CLASS METHOD DEFINITIONS            */
/*======================================================*/

MC_FrontEndNode::MC_FrontEndNode(std::string _hostname, unsigned short _port)
 :MC_ParentNode(false, _hostname, _port),
  MC_CommunicationNode(_hostname, _port),
  leafInfoPacket( NULL ),
  leavesConnectedPacket( NULL )
{
  MC_RemoteNode::local_parent_node = this;
}

MC_FrontEndNode::~MC_FrontEndNode()
{
}

int MC_FrontEndNode::proc_DataFromDownStream(MC_Packet *packet)
{
  mc_printf(MCFL, stderr, "In frontend.proc_DataFromUpStream()\n");

  MC_StreamManager * stream_mgr = StreamManagerById[ packet->get_StreamId() ];
  std::list<MC_Packet *> packets;
  std::list<MC_Packet *> ::iterator iter;

  stream_mgr->push_packet(packet, packets);

  if(packets.size() != 0){
    for(iter = packets.begin(); iter != packets.end(); iter++){
      MC_Packet *cur_packet = *iter;
      MC_StreamImpl * stream;
      stream = MC_StreamImpl::get_Stream(cur_packet->get_StreamId());

      if( stream ){
        mc_printf(MCFL, stderr, "Put packet in stream %d\n", cur_packet->get_StreamId());
        stream->add_IncomingPacket(packet);
      }
      else{
        mc_printf(MCFL, stderr, "Packet from unknown stream %d\n", cur_packet->get_StreamId());
        return -1;
      }
    }
  }

  return 0;
}

int MC_FrontEndNode::proc_PacketsFromDownStream(std::list <MC_Packet *> &packet_list)
{
  int retval=0;
  MC_Packet *cur_packet;

  mc_printf(MCFL, stderr, "In procPacketsFromDownStream()\n");

  std::list<MC_Packet *>::iterator iter=packet_list.begin();
  for(; iter != packet_list.end(); iter++){
    cur_packet = (*iter);
    switch(cur_packet->get_Tag()){
    case MC_RPT_SUBTREE_PROT:
      //printf(MCFL, stderr, "Calling proc_newSubTreeReport()\n");
      if(proc_newSubTreeReport(cur_packet) == -1){
	mc_printf(MCFL, stderr, "proc_newSubTreeReport() failed\n");
	retval=-1;
      }
      //printf(MCFL, stderr, "proc_newSubTreeReport() succeeded\n");
      break;

    case MC_GET_LEAF_INFO_PROT:
        if( proc_getLeafInfoResponse( cur_packet ) == -1 )
        {
            mc_printf( MCFL, stderr, "proc_getLeafInfoResponse() failed\n");
            retval = -1;
        }
        break;

    case MC_CONNECT_LEAVES_PROT:
        if( proc_connectLeavesResponse( cur_packet ) == -1 )
        {
            mc_printf( MCFL, stderr, "proc_connectLeavesResponse() failed\n");
            retval = -1;
        }
        break;

    default:
      //Any unrecognized tag is assumed to be data
      //printf(MCFL, stderr, "Calling proc_DataFromDownStream(). Tag: %d\n",
                 //cur_packet->get_Tag());
      if(proc_DataFromDownStream(cur_packet) == -1){
	mc_printf(MCFL, stderr, "proc_DataFromDownStream() failed\n");
	retval=-1;
      }
      //printf(MCFL, stderr, "proc_DataFromDownStream() succeeded\n");
    }
  }

  mc_printf(MCFL, stderr, "proc_PacketsFromDownStream() %s",
             (retval == -1 ? "failed\n" : "succeeded\n"));
  packet_list.clear();
  return retval;
}

int MC_FrontEndNode::send(MC_Packet *packet)
{
  return send_PacketDownStream(packet);
}

int MC_FrontEndNode::flush()
{
  return flush_PacketsDownStream();
}

int MC_FrontEndNode::flush(unsigned int stream_id)
{
  return flush_PacketsDownStream(stream_id);
}

int MC_FrontEndNode::recv()
{
  std::list <MC_Packet *> packet_list;
  mc_printf(MCFL, stderr, "In frontend.recv(). Calling recvfromdownstream()\n");

  if(recv_PacketsFromDownStream(packet_list) == -1){
    mc_printf(MCFL, stderr, "recv_packetsfromdownstream() failed\n");
    return -1;
  }

  if(packet_list.size() == 0){
    mc_printf(MCFL, stderr, "No packets read!\n");
    return 0;
  }

  if(proc_PacketsFromDownStream(packet_list) == -1){
    mc_printf(MCFL, stderr, "proc_packetsfromdownstream() failed\n");
    return -1;
  }
  mc_printf(MCFL, stderr, "Leaving frontend.recv()\n");
  return 1;
}


int
MC_FrontEndNode::deliverConnectLeavesResponse( MC_Packet* pkt )
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
        leavesConnectedPacket = new MC_Packet( pkt->get_BufferLen(),
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
MC_FrontEndNode::deliverLeafInfoResponse( MC_Packet* pkt )
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
        leafInfoPacket = new MC_Packet( pkt->get_BufferLen(),
                                        pkt->get_Buffer() );

        // release the given packet
        // TODO (is this safe?)
        delete pkt;
    }
    else
    {
        // we can use the packet as it is
        leafInfoPacket = pkt;
    }

    return 0;
}

