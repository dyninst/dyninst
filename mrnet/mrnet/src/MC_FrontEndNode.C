#include <stdio.h>
#include "mrnet/src/MC_FrontEndNode.h"
#include "mrnet/src/MC_StreamImpl.h"
#include "mrnet/src/utils.h"
/*======================================================*/
/*  MC_FrontEndNode CLASS METHOD DEFINITIONS            */
/*======================================================*/

MC_FrontEndNode::MC_FrontEndNode(std::string _hostname, unsigned short _port)
 :MC_ParentNode(false, _hostname, _port),
  MC_CommunicationNode(_hostname, _port)
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
        mc_printf(MCFL, stderr, "Put packet in stream %d\n", packet->get_StreamId());
        stream->add_IncomingPacket(packet);
      }
      else{
        mc_printf(MCFL, stderr, "Packet from unknown stream %d\n", packet->get_StreamId());
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
      //mc_printf(MCFL, stderr, "Calling proc_newSubTreeReport()\n");
      if(proc_newSubTreeReport(cur_packet) == -1){
	mc_printf(MCFL, stderr, "proc_newSubTreeReport() failed\n");
	retval=-1;
      }
      //mc_printf(MCFL, stderr, "proc_newSubTreeReport() succeeded\n");
      break;
    default:
      //Any unrecognized tag is assumed to be data
      //mc_printf(MCFL, stderr, "Calling proc_DataFromDownStream(). Tag: %d\n",
                 //cur_packet->get_Tag());
      if(proc_DataFromDownStream(cur_packet) == -1){
	mc_printf(MCFL, stderr, "proc_DataFromDownStream() failed\n");
	retval=-1;
      }
      //mc_printf(MCFL, stderr, "proc_DataFromDownStream() succeeded\n");
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

/*
int MC_FrontEndNode::send_newSubTree(MC_SerialGraph &sg)
{
  int num_descendants_to_report=0;
  std::list <MC_Packet *> packet_list;
  MC_Packet * packet;
  std::vector <std::string> arglist_commnode;
  std::string rootname(sg.get_RootName());
  unsigned short rootport(sg.get_RootPort());

  mc_printf(MCFL, stderr, "In frontend.sendnewsubtree()\n");

  MC_RemoteNode *cur_node = new MC_RemoteNode(rootname, rootport, false);

  cur_node->new_InternalNode(listening_sock_fd, hostname, port, commnode);

  if(cur_node->good() ){
    packet = new MC_Packet(MC_NEW_SUBTREE_PROT, "%s", sg.get_ByteArray().c_str());

    if( packet->good() ){
      if( cur_node->send(packet) == -1 ||
          cur_node->flush() == -1){
        mc_printf(MCFL, stderr, "send_newsubtree():send/flush failed\n");
        return -1;
      }
    }
    else{
      mc_printf(MCFL, stderr, "new packet() failed\n");
      return -1;
    }
    num_descendants_to_report++;
    children_nodes.push_back(cur_node);
  }
  else{
    mc_printf(MCFL, stderr, "new remotenode() failed\n");
    return -1;
  }

  while( num_descendants_to_report > num_descendants_reported){
    mc_printf(MCFL, stderr, "%d of %d Descendants have checked in.\n",
	       num_descendants_reported, num_descendants_to_report);
    if( recv() == -1 ){
      mc_printf(MCFL, stderr, "recv() failed\n");
      return -1;
    }
  }

  mc_printf(MCFL, stderr, "send_newsubtree() completed\n");
  return 0;
}

int MC_FrontEndNode::send_delSubTree()
{
  unsigned int i;
  int retval;
  MC_Packet *packet;

  mc_printf(MCFL, stderr, "In frontend.sendDelSubTree()\n");

  packet = new MC_Packet(MC_DEL_SUBTREE_PROT, "");
  if(packet->good()){

    std::list <MC_RemoteNode *>::iterator iter;
    for(i=0,iter = children_nodes.begin();
        iter != children_nodes.end(); iter++, i++){
      mc_printf(MCFL, stderr, "Calling downstream[%d].send() ...\n", i);
      if( (*iter)->send(packet) == -1){
        mc_printf(MCFL, stderr, "downstream.send() failed\n");
        retval = -1;
	continue;
      }

      mc_printf(MCFL, stderr, "downstream.send() succeeded\n");
      if( (*iter)->flush() == -1){
        mc_printf(MCFL, stderr, "downstream.flush() failed\n");
        retval = -1;
	continue;
      }
    }
  }
  else{
    mc_printf(MCFL, stderr, "new packet() failed\n");
    return -1;
  }

  mc_printf(MCFL, stderr, "Frontend.sendDelSubTree() succeeded\n");
  return 0;
}

int MC_FrontEndNode::send_newStream(int stream_id, int filter_id)
{
  unsigned int i;
  int retval;
  std::vector <MC_EndPoint *> * endpoints = MC_StreamImpl::get_Stream(stream_id)->
                                       get_EndPoints();
  MC_Packet *packet;

  mc_printf(MCFL, stderr, "In frontend.sendnewStream(%d)\n", stream_id);
  int * backends = (int *) malloc (sizeof(int) * endpoints->size();
  assert(backends);

  mc_printf(MCFL, stderr, "Adding backends to stream %d: [ \n", stream_id);
  for(i=0; i<endpoints->size(); i++){
    _fprintf((stderr, "%d, ", (*endpoints)[i]->get_Id()));
    backends[i] = (*endpoints)[i]->get_Id();
  }
  _fprintf((stderr, "\n"));


  MC_StreamManager * stream_mgr = new MC_StreamManager(stream_id, filter_id,
					       upstream_node, children_nodes);
  StreamManagerById[stream_id] = stream_mgr;
  mc_printf(MCFL, stderr, "StreamManagerById[%d] = %p\n", stream_id,
	     StreamManagerById[stream_id]);

  packet = new MC_Packet(MC_NEW_STREAM_PROT, "%d %ad %d", stream_id, backends,
                         endpoints->size(), filter_id);
  if(packet->good()){

    std::list <MC_RemoteNode *>::iterator iter;
    for(i=0,iter = children_nodes.begin();
        iter != children_nodes.end(); iter++, i++){
      mc_printf(MCFL, stderr, "Calling children_nodes[%d].send() ...\n", i);
      if( (*iter)->send(packet) == -1){
        mc_printf(MCFL, stderr, "children_nodes.send() failed\n");
        retval = -1;
	continue;
      }
      mc_printf(MCFL, stderr, "children_nodes.send() succeeded\n");
    }
  }
  else{
    mc_printf(MCFL, stderr, "new packet() failed\n");
    return -1;
  }

  mc_printf(MCFL, stderr, "Frontend.sendNewStream() succeeded\n");
  return 0;
}

int MC_FrontEndNode::send_delStream(int stream_id)
{
  MC_Packet *packet;
  int retval;
  unsigned int i;

  mc_printf(MCFL, stderr, "In frontend.sendDelStream()\n");

  MC_StreamManager * stream_mgr = StreamManagerById[stream_id];

  packet = new MC_Packet(MC_DEL_STREAM_PROT, "%d", stream_id);
  if(packet->good()){

    std::list <MC_RemoteNode *>::iterator iter;
    for(i=0,iter = stream_mgr->downstream_nodes.begin();
        iter != stream_mgr->downstream_nodes.end();
        iter++, i++){
      mc_printf(MCFL, stderr, "Calling node_set[%d].send() ...\n", i);
      if( (*iter)->send(packet) == -1){
        mc_printf(MCFL, stderr, "node_set.send() failed\n");
        retval = -1;
	continue;
      }
      mc_printf(MCFL, stderr, "node_set.send() succeeded\n");
    }
  }
  else{
    mc_printf(MCFL, stderr, "new packet() failed\n");
    return -1;
  }

  mc_printf(MCFL, stderr, "Frontend.sendDelStream() succeeded\n");
  return 0;
}

int MC_FrontEndNode::send_newApplication(std::string cmd, std::vector<std::string> args)
{
  MC_Packet *packet;
  unsigned int i;
  int retval;

  mc_printf(MCFL, stderr, "In frontend.sendnewApplication()\n");

  packet = new MC_Packet(MC_NEW_APPLICATION_PROT, "%s", cmd.c_str());
  if(packet->good()){

    std::list <MC_RemoteNode *>::iterator iter;
    for(i=0,iter = children_nodes.begin();
        iter != children_nodes.end(); iter++, i++){
      mc_printf(MCFL, stderr, "Calling downstream[%d].send() ...\n", i);
      if( (*iter)->send(packet) == -1){
        mc_printf(MCFL, stderr, "downstream.send() failed\n");
        retval = -1;
	continue;
      }
      if( (*iter)->flush() == -1){
        mc_printf(MCFL, stderr, "downstream.flush() failed\n");
        retval = -1;
	continue;
      }
      mc_printf(MCFL, stderr, "downstream.send()/flush() succeeded\n");
    }
  }
  else{
    mc_printf(MCFL, stderr, "new packet() failed\n");
    return -1;
  }

  mc_printf(MCFL, stderr, "Frontend.sendnewApplication() succeeded\n");
  return 0;
}

int MC_FrontEndNode::send_delApplication()
{
  MC_Packet *packet;
  unsigned int i;
  int retval;

  mc_printf(MCFL, stderr, "In frontend.sendDelApplication()\n");

  packet = new MC_Packet(MC_DEL_APPLICATION_PROT, "");
  if(packet->good()){

    std::list <MC_RemoteNode *>::iterator iter;
    for(i=0,iter = children_nodes.begin();
        iter != children_nodes.end(); iter++, i++){
      mc_printf(MCFL, stderr, "Calling downstream[%d].send() ...\n", i);
      if( (*iter)->send(packet) == -1){
        mc_printf(MCFL, stderr, "downstream.send() failed\n");
        retval = -1;
	continue;
      }
      mc_printf(MCFL, stderr, "downstream.send() succeeded\n");
    }
  }
  else{
    mc_printf(MCFL, stderr, "new packet() failed\n");
    return -1;
  }

  mc_printf(MCFL, stderr, "Frontend.sendDelApplication() succeeded\n");
  return 0;
}

int MC_FrontEndNode::send_DataDownStream(MC_Packet *)
{
  return 0;
}


*/
