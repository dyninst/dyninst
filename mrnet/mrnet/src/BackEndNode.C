#include <stdio.h>
#include <arpa/inet.h>

#include "mrnet/src/BackEndNode.h"
#include "mrnet/src/RemoteNode.h"
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/utils.h"

/*=====================================================*/
/*  BackEndNode CLASS METHOD DEFINITIONS            */
/*=====================================================*/

BackEndNode::BackEndNode(std::string _hostname,
                               unsigned short _backend_id,
                               std::string _parent_hostname,
                               unsigned short _parent_port,
                               unsigned short _parent_id)
  :ChildNode(false, _hostname, _backend_id), 
   CommunicationNode(_hostname, _backend_id),
   backend_id(_backend_id)
{
  RemoteNode::local_child_node = this;
  mrn_printf(3, MCFL, stderr,
    "creating BackEndNode, host=%s, backendId=%u, parHost=%s, parPort=%u, parRank=%u\n",
    _hostname.c_str(), _backend_id, _parent_hostname.c_str(), _parent_port, _parent_id );
  upstream_node = new RemoteNode(false, _parent_hostname, _parent_port,
                                    _parent_id);

  upstream_node->connect();
  upstream_node->_is_upstream = true;
  if(upstream_node->fail() ){
    mrn_printf(1, MCFL, stderr, "connect() failed\n");
    MRN_errno = MRN_ECANNOTBINDPORT;
    return;
  }
  
  // do low-level handshake with our id in the backend id namespace
  uint32_t idBuf = htonl(backend_id);
  int sret = ::send( upstream_node->get_sockfd(), &idBuf, 4, 0 );
  if( sret == -1 )
  {
    mrn_printf(1, MCFL, stderr, "send of backend id failed\n");
  }

  mrn_printf(3, MCFL, stderr, "Leaving BackEndNode()\n");
}

BackEndNode::~BackEndNode(void){};

int BackEndNode::proc_PacketsFromUpStream(std::list <Packet *> &packets)
{
  int retval=0;
  Packet *cur_packet;

  mrn_printf(3, MCFL, stderr, "In proc_PacketsFromUpStream()\n");

  std::list<Packet *>::iterator iter=packets.begin();
  for(; iter != packets.end(); iter++){
    cur_packet = (*iter);
    switch(cur_packet->get_Tag()){
    case MRN_DATA_PROT:
    case MRN_NEW_SUBTREE_PROT:
    case MRN_DEL_SUBTREE_PROT:
    case MRN_RPT_SUBTREE_PROT:
    case MRN_NEW_APPLICATION_PROT:
    case MRN_DEL_APPLICATION_PROT:
    case MRN_NEW_STREAM_PROT:
    case MRN_DEL_STREAM_PROT:
    case MRN_GET_LEAF_INFO_PROT:
    case MRN_CONNECT_LEAVES_PROT:
      //these protocol tags should never reach backend
      mrn_printf(1, MCFL,stderr,"BackEndNode::proc_DataFromUpStream saw poison tag: %d\n",
        cur_packet->get_Tag());
      assert(0);
      break;

        break;

    default:
      //Any Unrecognized tag is assumed to be data
      //mrn_printf(3, MCFL, stderr, "Calling proc_DataFromUpStream(). Tag: %d\n",
      //cur_packet->get_Tag());
      if(proc_DataFromUpStream(cur_packet) == -1){
	mrn_printf(1, MCFL, stderr, "proc_DataFromUpStream() failed\n");
	retval=-1;
      }
      //mrn_printf(3, MCFL, stderr, "proc_DataFromUpStream() succeded\n");
      break;
    }
  }

  packets.clear();
  mrn_printf(3, MCFL, stderr, "proc_PacketsFromUpStream() %s",
             (retval == -1 ? "failed\n" : "succeeded\n"));
  return retval;
}

int BackEndNode::proc_DataFromUpStream(Packet *packet)
{
  StreamImpl * stream;

  mrn_printf(3, MCFL, stderr, "In proc_DataFromUpStream()\n");

  stream = StreamImpl::get_Stream(packet->get_StreamId());

  if( stream ){
    //mrn_printf(3, MCFL, stderr, "Inserting packet into stream %d\n", packet->get_StreamId());
    stream->add_IncomingPacket(packet);
  }
  else{
    //mrn_printf(3, MCFL, stderr, "Inserting packet into NEW stream %d\n", packet->get_StreamId());
    stream = new StreamImpl(packet->get_StreamId());
    stream->add_IncomingPacket(packet);
  }
  mrn_printf(3, MCFL, stderr, "Leaving proc_DataFromUpStream()\n");
  return 0;
}

int BackEndNode::send(Packet *packet)
{
  mrn_printf(3, MCFL, stderr, "In backend.send(). Calling sendupstream()\n");
  return send_PacketUpStream(packet);
}

int BackEndNode::flush()
{
  mrn_printf(3, MCFL, stderr, "In backend.flush(). Calling flushupstream()\n");
  return flush_PacketsUpStream();
}

int BackEndNode::recv()
{
  std::list <Packet *> packet_list;
  mrn_printf(3, MCFL, stderr, "In backend.recv(). Calling recvfromupstream()\n");

  if(recv_PacketsFromUpStream(packet_list) == -1){
    mrn_printf(1, MCFL, stderr, "recv_packetsfromupstream() failed\n");
    return -1;
  }

  if(packet_list.size() == 0){
    mrn_printf(3, MCFL, stderr, "No packets read!\n");
    return 0;
  }

  if(proc_PacketsFromUpStream(packet_list) == -1){
    mrn_printf(1, MCFL, stderr, "proc_packetsfromupstream() failed\n");
    return -1;
  }

  mrn_printf(3, MCFL, stderr, "Leaving backend.recv().\n");
  return 1;
}
