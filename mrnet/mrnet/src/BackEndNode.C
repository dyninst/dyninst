#include <stdio.h>
#include <arpa/inet.h>

#include "mrnet/src/BackEndNode.h"
#include "mrnet/src/RemoteNode.h"
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/utils.h"

/*=====================================================*/
/*  MC_BackEndNode CLASS METHOD DEFINITIONS            */
/*=====================================================*/

MC_BackEndNode::MC_BackEndNode(std::string _hostname,
                               unsigned short _backend_id,
                               std::string _parent_hostname,
                               unsigned short _parent_port,
                               unsigned short _parent_id)
  :MC_ChildNode(false, _hostname, _backend_id), 
   MC_CommunicationNode(_hostname, _backend_id),
   backend_id(_backend_id)
{
  MC_RemoteNode::local_child_node = this;
  mc_printf(MCFL, stderr, "In BackEndNode()\n");
  upstream_node = new MC_RemoteNode(false, _parent_hostname, _parent_port,
                                    _parent_id);

  upstream_node->connect();
  upstream_node->_is_upstream = true;
  if(upstream_node->fail() ){
    mc_printf(MCFL, stderr, "connect() failed\n");
    mc_errno = MC_ECANNOTBINDPORT;
    return;
  }
  
  // do low-level handshake with our id in the backend id namespace
  uint32_t idBuf = htonl(backend_id);
  int sret = ::send( upstream_node->get_sockfd(), &idBuf, 4, 0 );
  if( sret == -1 )
  {
    mc_printf(MCFL, stderr, "send of backend id failed\n");
  }

  mc_printf(MCFL, stderr, "Leaving BackEndNode()\n");
}

MC_BackEndNode::~MC_BackEndNode(void){};

int MC_BackEndNode::proc_PacketsFromUpStream(std::list <MC_Packet *> &packets)
{
  int retval=0;
  MC_Packet *cur_packet;

  mc_printf(MCFL, stderr, "In proc_PacketsFromUpStream()\n");

  std::list<MC_Packet *>::iterator iter=packets.begin();
  for(; iter != packets.end(); iter++){
    cur_packet = (*iter);
    switch(cur_packet->get_Tag()){
    case MC_DATA_PROT:
    case MC_NEW_SUBTREE_PROT:
    case MC_DEL_SUBTREE_PROT:
    case MC_RPT_SUBTREE_PROT:
    case MC_NEW_APPLICATION_PROT:
    case MC_DEL_APPLICATION_PROT:
    case MC_NEW_STREAM_PROT:
    case MC_DEL_STREAM_PROT:
      //these protocol tags should never reach backend
      assert(0);
    default:
      //Any Unrecognized tag is assumed to be data
      //mc_printf(MCFL, stderr, "Calling proc_DataFromUpStream(). Tag: %d\n",
      //cur_packet->get_Tag());
      if(proc_DataFromUpStream(cur_packet) == -1){
	mc_printf(MCFL, stderr, "proc_DataFromUpStream() failed\n");
	retval=-1;
      }
      //mc_printf(MCFL, stderr, "proc_DataFromUpStream() succeded\n");
      break;
    }
  }

  packets.clear();
  mc_printf(MCFL, stderr, "proc_PacketsFromUpStream() %s",
             (retval == -1 ? "failed\n" : "succeeded\n"));
  return retval;
}

int MC_BackEndNode::proc_DataFromUpStream(MC_Packet *packet)
{
  MC_StreamImpl * stream;

  mc_printf(MCFL, stderr, "In proc_DataFromUpStream()\n");

  stream = MC_StreamImpl::get_Stream(packet->get_StreamId());

  if( stream ){
    //mc_printf(MCFL, stderr, "Inserting packet into stream %d\n", packet->get_StreamId());
    stream->add_IncomingPacket(packet);
  }
  else{
    //mc_printf(MCFL, stderr, "Inserting packet into NEW stream %d\n", packet->get_StreamId());
    stream = new MC_StreamImpl(packet->get_StreamId());
    stream->add_IncomingPacket(packet);
  }
  mc_printf(MCFL, stderr, "Leaving proc_DataFromUpStream()\n");
  return 0;
}

int MC_BackEndNode::send(MC_Packet *packet)
{
  mc_printf(MCFL, stderr, "In backend.send(). Calling sendupstream()\n");
  return send_PacketUpStream(packet);
}

int MC_BackEndNode::flush()
{
  mc_printf(MCFL, stderr, "In backend.flush(). Calling flushupstream()\n");
  return flush_PacketsUpStream();
}

int MC_BackEndNode::recv()
{
  std::list <MC_Packet *> packet_list;
  mc_printf(MCFL, stderr, "In backend.recv(). Calling recvfromupstream()\n");

  if(recv_PacketsFromUpStream(packet_list) == -1){
    mc_printf(MCFL, stderr, "recv_packetsfromupstream() failed\n");
    return -1;
  }

  if(packet_list.size() == 0){
    mc_printf(MCFL, stderr, "No packets read!\n");
    return 0;
  }

  if(proc_PacketsFromUpStream(packet_list) == -1){
    mc_printf(MCFL, stderr, "proc_packetsfromupstream() failed\n");
    return -1;
  }

  mc_printf(MCFL, stderr, "Leaving backend.recv().\n");
  return 1;
}
