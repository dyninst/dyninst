#include <stdio.h>

#include "mrnet/src/MC_InternalNode.h"
#include "mrnet/src/utils.h"

/*======================================================*/
/*  MC_InternalNode CLASS METHOD DEFINITIONS            */
/*======================================================*/
MC_InternalNode::MC_InternalNode(std::string _hostname, unsigned short _port,
                                 std::string _phostname, unsigned short _pport,
                                 unsigned short _pid)
  :MC_ParentNode(true, _hostname, _port),
   MC_ChildNode(true, _hostname, _port),
   MC_CommunicationNode(_hostname, _port)
{
  int retval;
  mc_printf(MCFL, stderr, "In MC_InternalNode: parent_host: %s, parent_port: %d"
            " parent_id: %d\n", _phostname.c_str(), _pport, _pid);

  upstream_node = new MC_RemoteNode(true, _phostname, _pport, _pid);
  MC_RemoteNode::local_child_node = this;
  MC_RemoteNode::local_parent_node = this;

  //mc_printf(MCFL, stderr, "Calling connect() ...\n");
  upstream_node->connect();
  upstream_node->_is_upstream = true;
  if(upstream_node->fail() ){
    mc_printf(MCFL, stderr, "connect() failed\n");
    mc_errno = MC_ECANNOTBINDPORT;
    return;
  }

  //mc_printf(MCFL, stderr, "connect() succeeded. Call bind_to_port(%d)...\n", port);
  //if( bind_to_port(&listening_sock_fd, &(this->MC_CommunicationNode::port) ) == -1){
    //mc_printf(MCFL, stderr, "bind_to_port() failed\n");
    //mc_errno = MC_ECANNOTBINDPORT;
    //return;
  //}
  //mc_printf(MCFL, stderr, "Bound to port %d, via socket: %d\n",
             //this->MC_CommunicationNode::port, listening_sock_fd);

  mc_printf(MCFL, stderr, "Creating Upstream recv thread ...\n");
  retval = pthread_create(&(upstream_node->recv_thread_id), NULL,
                          MC_RemoteNode::recv_thread_main,
                          (void *) upstream_node);
  if(retval != 0){
    mc_printf(MCFL, stderr, "Upstream recv thread creation failed\n");
    //thread create error
  }

  mc_printf(MCFL, stderr, "Creating Upstream send thread ...\n");
  retval = pthread_create(&(upstream_node->send_thread_id), NULL,
                          MC_RemoteNode::send_thread_main,
                          (void *) upstream_node);
  if(retval != 0){
    mc_printf(MCFL, stderr, "Upstream send thread creation failed\n");
    //thread create error
  }

  mc_printf(MCFL, stderr, "Leaving MC_InternalNode()\n");
}

MC_InternalNode::~MC_InternalNode(void)
{
  delete upstream_node;
  std::list<MC_RemoteNode *>::iterator iter;

  for(iter = children_nodes.begin(); iter != children_nodes.end(); iter++){
    delete (MC_RemoteNode *)(*iter);
    children_nodes.erase(iter);
  }
}

void MC_InternalNode::waitLoop()
{
    // TODO what should we base our termination decision on?
    // * whether we have *any* connections remaining?
    // * whether we have an upstream connection?
    // * whether we have any downstream connections?
    //

    // for now, we base our termination on when we lose our upstream connection
    int iret = pthread_join( upstream_node->recv_thread_id, NULL );
    if( iret != 0 )
    {
        mc_printf( MCFL, stderr, "comm_node failed to join with upstream internal receive thread: %d\n", iret );
    }
}

int MC_InternalNode::send_newSubTreeReport(bool status)
{
  unsigned int *backends, i;
  mc_printf(MCFL, stderr, "In send_newSubTreeReport()\n");

  backends = new unsigned int[backend_descendant_nodes.size()];
  assert(backends);

  std::list <int>::iterator iter;
  mc_printf(MCFL, stderr, "Creating subtree report from %p: [ ", &backend_descendant_nodes);
  for(i=0, iter=backend_descendant_nodes.begin();
      iter != backend_descendant_nodes.end();
      i++, iter++){
    backends[i] = *iter;
    _fprintf((stderr, "%d, ", backends[i]));
  }
  _fprintf((stderr, "]\n"));

  MC_Packet *packet = new MC_Packet(MC_RPT_SUBTREE_PROT, "%d %ad", status,
				    backends, backend_descendant_nodes.size());
  if(packet->good()){
    if( upstream_node->send(packet) == -1 ||
        upstream_node->flush() == -1){
      mc_printf(MCFL, stderr, "send/flush failed\n");
      return -1;
    }
  }
  else{
    mc_printf(MCFL, stderr, "new packet() failed\n");
    return -1;
  }

  mc_printf(MCFL, stderr, "send_newSubTreeReport() succeeded\n");
  return 0;
}


int MC_InternalNode::proc_DataFromUpStream(MC_Packet *packet)
{
  int retval;
  unsigned int i;

  mc_printf(MCFL, stderr, "In proc_DataFromUpStream()\n");

  streammanagerbyid_sync.lock();
  MC_StreamManager *stream_mgr = StreamManagerById[packet->get_StreamId()];
  streammanagerbyid_sync.unlock();
  mc_printf(MCFL, stderr, "DCA: extracted stream_mgr(%p) to strmgr[%d]\n",
            stream_mgr, packet->get_StreamId());
  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = stream_mgr->downstream_nodes.begin();
      iter != stream_mgr->downstream_nodes.end();
      iter++, i++){
    mc_printf(MCFL, stderr, "Calling node_set[%d(%p)].send() ...\n", i,
              *iter);
    if( (*iter)->send(packet) == -1){
      mc_printf(MCFL, stderr, "node_set.send() failed\n");
      retval = -1;
      continue;
    }
    mc_printf(MCFL, stderr, "node_set.send() succeeded\n");
  }

  mc_printf(MCFL, stderr, "internal.procDataFromUpStream() succeeded\n");
  return 0;
}

int MC_InternalNode::proc_DataFromDownStream(MC_Packet *packet)
{
  mc_printf(MCFL, stderr, "In internal.proc_DataFromUpStream()\n");

  MC_StreamManager * stream_mgr = StreamManagerById[ packet->get_StreamId() ];
  std::list<MC_Packet *> packets;
  std::list<MC_Packet *> ::iterator iter;

  stream_mgr->push_packet(packet, packets);
  if(packets.size() != 0){
      for(iter = packets.begin(); iter != packets.end() ; iter++ ){
          if( upstream_node->send( *iter ) == -1){
              mc_printf(MCFL, stderr, "upstream.send() failed()\n");
              return -1;
          }
      }
  }

  mc_printf(MCFL, stderr, "Leaving internal.proc_DataFromUpStream()\n");
  return 0;
}


/*===================================================*/
/*  MC_LocalNode CLASS METHOD DEFINITIONS            */
/*===================================================*/
int MC_InternalNode::proc_PacketsFromUpStream(std::list <MC_Packet *> &packets)
{
  int retval=0;
  MC_Packet *cur_packet;
  MC_StreamManager * stream_mgr;

  mc_printf(MCFL, stderr, "In proc_PacketsFromUpStream()\n");

  std::list<MC_Packet *>::iterator iter=packets.begin();
  for(; iter != packets.end(); iter++){
    cur_packet = (*iter);
    switch(cur_packet->get_Tag()){
    case MC_NEW_SUBTREE_PROT:
      //mc_printf(MCFL, stderr, "Calling proc_newSubTree()\n");
      if(proc_newSubTree(cur_packet) == -1){
	mc_printf(MCFL, stderr, "proc_newSubTree() failed\n");
	retval=-1;
      }
      //AT this point, we have created subteee and collected all reports
      //must send reports upwards
      if(send_newSubTreeReport( true ) == -1 ){
          mc_printf(MCFL, stderr, "send_newSubTreeReport() failed\n");
          retval=-1;
      }
      //mc_printf(MCFL, stderr, "proc_newsubtree() succeded\n");
      break;
    case MC_DEL_SUBTREE_PROT:
      //mc_printf(MCFL, stderr, "Calling proc_delSubTree()\n");
      if(proc_delSubTree(cur_packet) == -1){
	mc_printf(MCFL, stderr, "proc_delSubTree() failed\n");
	retval=-1;
      }
      //mc_printf(MCFL, stderr, "proc_delSubTree() succeded\n");
      break;
    case MC_NEW_APPLICATION_PROT:
      //mc_printf(MCFL, stderr, "Calling proc_newApplication()\n");
      if(proc_newApplication(cur_packet) == -1){
	mc_printf(MCFL, stderr, "proc_newApplication() failed\n");
	retval=-1;
      }
      //mc_printf(MCFL, stderr, "proc_newApplication() succeded\n");
      break;
    case MC_DEL_APPLICATION_PROT:
      //mc_printf(MCFL, stderr, "Calling proc_delApplication()\n");
      if(proc_delApplication(cur_packet) == -1){
	mc_printf(MCFL, stderr, "proc_delApplication() failed\n");
	retval=-1;
      }
      //mc_printf(MCFL, stderr, "proc_delApplication() succeded\n");
      break;
    case MC_NEW_STREAM_PROT:
      //mc_printf(MCFL, stderr, "Calling proc_newStream()\n");
        stream_mgr = proc_newStream(cur_packet);
        if(stream_mgr == NULL){
            mc_printf(MCFL, stderr, "proc_newStream() failed\n");
            retval=-1;
            break;
        }
        stream_mgr->upstream_node = upstream_node;
        if(send_newStream(cur_packet, stream_mgr) == -1){
            mc_printf(MCFL, stderr, "send_newStream() failed\n");
            retval=-1;
            break;
        }
        break;
    case MC_DEL_STREAM_PROT:
      //mc_printf(MCFL, stderr, "Calling proc_delStream()\n");
      if(proc_delStream(cur_packet) == -1){
	mc_printf(MCFL, stderr, "proc_delStream() failed\n");
	retval=-1;
      }
      //mc_printf(MCFL, stderr, "proc_delStream() succeded\n");
      break;

    case MC_GET_LEAF_INFO_PROT:
        //mc_printf(MCFL, stderr, "Calling proc_getLeafInfo()\n");
        if( proc_getLeafInfo(cur_packet) == -1)
        {
            mc_printf(MCFL, stderr, "proc_getLeafInfo() failed\n");
            retval=-1;
        }
        //mc_printf(MCFL, stderr, "proc_getLeafInfo() succeded\n");
        break;

    case MC_CONNECT_LEAVES_PROT:
        if( proc_connectLeaves( cur_packet ) == -1 )
        {
            mc_printf( MCFL, stderr, "proc_connectLeaves() failed\n" );
            retval = -1;
        }
        break;

    default:
      //Any Unrecognized tag is assumed to be data
      //mc_printf(MCFL, stderr, "Calling proc_DataFromUpStream(). Tag: %d\n",cur_packet->get_Tag());
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

int MC_InternalNode::proc_PacketsFromDownStream(std::list <MC_Packet *> &packet_list)
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

    case MC_GET_LEAF_INFO_PROT:
        if( proc_getLeafInfoResponse( cur_packet ) == -1 )
        {
            mc_printf( MCFL, stderr, "proc_getLeafInfoResponse() failed\n" );
            retval = -1;
        }
        break;

    case MC_CONNECT_LEAVES_PROT:
        if( proc_connectLeavesResponse( cur_packet ) == -1 )
        {
            mc_printf( MCFL, stderr, "proc_connectLeavesResponse() failed\n" );
            retval = -1;
        }
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


int
MC_InternalNode::deliverLeafInfoResponse( MC_Packet* pkt )
{
    int ret = 0;

    // deliver the aggregated response to our parent
    if( (upstream_node->send( pkt ) == -1) ||
        (upstream_node->flush() == -1) )
    {
        mc_printf( MCFL, stderr, "failed to deliver response to parent\n" );
    }
    return ret;
}


int
MC_InternalNode::deliverConnectLeavesResponse( MC_Packet* pkt )
{
    int ret = 0;

    // deliver the aggregated response to our parent
    if( (upstream_node->send( pkt ) == -1) ||
        (upstream_node->flush() == -1) )
    {
        mc_printf( MCFL, stderr, "failed to deliver response to parent\n" );
    }
    return ret;
}


