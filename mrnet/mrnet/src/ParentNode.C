#include <stdio.h>

#include "mrnet/src/MC_ParentNode.h"
#include "mrnet/src/MC_NetworkGraph.h"
#include "mrnet/src/utils.h"

/*====================================================*/
/*  MC_ParentNode CLASS METHOD DEFINITIONS            */
/*====================================================*/
std::map<unsigned int, MC_Aggregator::AggregatorSpec*>
  MC_ParentNode::AggrSpecById;
std::map<unsigned int, void(*)(std::list <MC_Packet*>&, std::list <MC_Packet*>&,
                               std::list<MC_RemoteNode *> &, void **)>
   MC_ParentNode::SyncById;

MC_ParentNode::MC_ParentNode(bool _threaded, std::string _hostname,
                             unsigned short _port)
  :hostname(_hostname), port(_port), config_port(_port), listening_sock_fd(0),
   threaded(_threaded), num_descendants(0), num_descendants_reported(0)
{
  mc_printf(MCFL, stderr, "In MC_ParentNode(): Calling bind_to_port(%d)\n", 
             port);
  if( bind_to_port(&listening_sock_fd, &port) == -1){
    mc_printf(MCFL, stderr, "bind_to_port() failed\n");
    //mc_errno = MC_ECANNOTBINDPORT;
    return;
  }
  subtreereport_sync.register_cond(MC_ALLNODESREPORTED);

  //hardcoding some aggregator definitions
  MC_Aggregator::AggregatorSpec *tmp = new MC_Aggregator::AggregatorSpec;
  tmp->format_str = AGGR_NULL_FORMATSTR;
  tmp->filter = NULL;
  AggrSpecById[AGGR_NULL] =tmp;

  tmp = new MC_Aggregator::AggregatorSpec;
  tmp->format_str = AGGR_INT_SUM_FORMATSTR;
  tmp->filter = aggr_Int_Sum;
  AggrSpecById[AGGR_INT_SUM_ID] =tmp;

  tmp = new MC_Aggregator::AggregatorSpec;
  tmp->format_str = AGGR_FLOAT_AVG_FORMATSTR;
  tmp->filter = aggr_Float_Avg;
  AggrSpecById[AGGR_FLOAT_AVG_ID] =tmp;

  tmp = new MC_Aggregator::AggregatorSpec;
  tmp->format_str = AGGR_CHARARRAY_CONCAT_FORMATSTR;
  tmp->filter = aggr_CharArray_Concat;
  AggrSpecById[AGGR_CHARARRAY_CONCAT_ID] =tmp;

  //hardcoding some synchonizer definitions
  SyncById[SYNC_WAITFORALL] = sync_WaitForAll;
  SyncById[SYNC_DONTWAIT] = sync_DontWait;
  SyncById[SYNC_TIMEOUT] = sync_TimeOut;

  mc_printf(MCFL, stderr, "Leaving MC_ParentNode()\n");
}


MC_ParentNode::~MC_ParentNode(void)
{}

int MC_ParentNode::recv_PacketsFromDownStream(std::list<MC_Packet *>&packet_list)
{
  unsigned int i;
  int retval=0;

  mc_printf(MCFL, stderr, "In recv_PacketsFromDownStream()\n");

  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = children_nodes.begin();
      iter != children_nodes.end(); iter++, i++){
    mc_printf(MCFL, stderr, "Calling downstream[%d].recv() ...\n", i);
    if( (*iter)->recv(packet_list) == -1){
      mc_printf(MCFL, stderr, "downstream.recv() failed\n");
      retval = -1;
      continue;
    }
    mc_printf(MCFL, stderr, "downstream.recv() succeeded\n");
  }

  mc_printf(MCFL, stderr, "recv_PacketsFromDownStream %s",
	     retval == 0 ? "succeeded\n" : "failed\n");
  return retval;
}

int MC_ParentNode::send_PacketDownStream(MC_Packet *packet)
{
  unsigned int i;
  int retval=0;
  MC_StreamManager * stream_mgr;

  mc_printf(MCFL, stderr, "In send_PacketDownStream()\n");
  mc_printf(MCFL, stderr, "StreamMangerById[%d] = %p\n",
             packet->get_StreamId(),
	     StreamManagerById[packet->get_StreamId()]);
  if(threaded){ streammanagerbyid_sync.lock(); }
  stream_mgr = StreamManagerById[packet->get_StreamId()];
  if(threaded){ streammanagerbyid_sync.unlock(); }

  std::list <MC_RemoteNode *>::iterator iter;

  for(i=0,iter = stream_mgr->downstream_nodes.begin();
      iter != stream_mgr->downstream_nodes.end();
      iter++, i++){
    mc_printf(MCFL, stderr, "Calling downstream[%d].send() ...\n", i);
    if( (*iter)->send(packet) == -1){
      mc_printf(MCFL, stderr, "downstream.send() failed\n");
      retval = -1;
    }
    mc_printf(MCFL, stderr, "downstream.send() succeeded\n");
  }

  mc_printf(MCFL, stderr, "send_PacketDownStream %s",
	     retval == 0 ? "succeeded\n" : "failed\n");
  return retval;
}

int MC_ParentNode::flush_PacketsDownStream()
{
  unsigned int i;
  int retval=0;

  mc_printf(MCFL, stderr, "In flush_PacketsDownStream()\n");

  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = children_nodes.begin();
      iter != children_nodes.end(); iter++, i++){
    mc_printf(MCFL, stderr, "Calling downstream[%d].flush() ...\n", i);
    if( (*iter)->flush() == -1){
      mc_printf(MCFL, stderr, "downstream.flush() failed\n");
      retval = -1;
    }
    mc_printf(MCFL, stderr, "downstream.flush() succeeded\n");
  }

  mc_printf(MCFL, stderr, "flush_PacketsFromDownStream %s",
	     retval == 0 ? "succeeded\n" : "failed\n");
  return retval;
}

int MC_ParentNode::flush_PacketsDownStream(unsigned int stream_id)
{
  unsigned int i;
  int retval=0;
  MC_StreamManager * stream_mgr;

  mc_printf(MCFL, stderr, "In flush_PacketsDownStream(%d)\n", stream_id);
  if(threaded){ streammanagerbyid_sync.lock(); }
  stream_mgr = StreamManagerById[stream_id];
  if(threaded){ streammanagerbyid_sync.unlock(); }

  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = stream_mgr->downstream_nodes.begin();
      iter != stream_mgr->downstream_nodes.end();
      iter++, i++){
    mc_printf(MCFL, stderr, "Calling downstream[%d].flush() ...\n", i);
    if( (*iter)->flush() == -1){
      mc_printf(MCFL, stderr, "downstream.flush() failed\n");
      retval = -1;
    }
    mc_printf(MCFL, stderr, "downstream.flush() succeeded\n");
  }

  mc_printf(MCFL, stderr, "flush_PacketsFromDownStream %s",
	     retval == 0 ? "succeeded\n" : "failed\n");
  return retval;
}

int MC_ParentNode::proc_newSubTree(MC_Packet * packet)
{
  char *byte_array=NULL;
  char *appl=NULL;
  char *commnode=NULL;
  mc_printf(MCFL, stderr, "In proc_newSubTree()\n");

  if( packet->ExtractArgList("%s%s%s", &byte_array, &appl, &commnode) == -1){
    mc_printf(MCFL, stderr, "ExtractArgList() failed\n");
    return -1;
  }

  MC_SerialGraph sg(byte_array);
  std::string application(appl);

  //mc_printf(MCFL, stderr, "sg.root:%s:%d, local:%s:%d\n", sg.get_RootName().c_str(),
	     //sg.get_RootPort(), hostname.c_str(), port);
  //assert(getNetworkName(sg.get_RootName()) == hostname);

  MC_SerialGraph *cur_sg;
  sg.set_ToFirstChild();
  for(cur_sg = sg.get_NextChild(); cur_sg; cur_sg = sg.get_NextChild()){

    if(cur_sg->has_children()){
      if(threaded){ subtreereport_sync.lock(); }
      num_descendants++;
      if(threaded){ subtreereport_sync.unlock(); }

      std::string rootname = cur_sg->get_RootName();
      unsigned short rootport = cur_sg->get_RootPort();
      mc_printf(MCFL, stderr, "cur_child(%s:%d) has children (internal node)\n",
                 rootname.c_str(), rootport);
      //If the cur_child has children launch him
      MC_RemoteNode *cur_node = new MC_RemoteNode(threaded, rootname, rootport);

      cur_node->new_InternalNode(listening_sock_fd,
                                    hostname, port, config_port, commnode);

      if(cur_node->good() ){
        packet = new MC_Packet(MC_NEW_SUBTREE_PROT, "%s%s%s",
                               cur_sg->get_ByteArray().c_str(),
			       application.c_str(),
                   commnode);
        if( cur_node->send(packet) == -1 ||
            cur_node->flush() == -1){
          mc_printf(MCFL, stderr, "send/flush failed\n");
          return -1;
        }
        children_nodes.push_back(cur_node);
      }
    }
    else{
      std::string rootname = cur_sg->get_RootName();
      unsigned short rootport = cur_sg->get_RootPort();
      unsigned short rootid = cur_sg->get_Id();

      std::vector <std::string> dummy_args;
      mc_printf(MCFL, stderr, "cur_child(%s:%d) is backend[%d]\n",
                 rootname.c_str(), rootport, rootid);
      MC_RemoteNode *cur_node = new MC_RemoteNode(threaded, rootname, rootport);

      if( cur_node->new_Application(listening_sock_fd, hostname, port,
                                   config_port, application, dummy_args)== -1){
	mc_printf(MCFL, stderr, "child_node.new_application() failed\n");
	return -1;
      }

      if(threaded){ childnodebybackendid_sync.lock(); }
      ChildNodeByBackendId[rootid] = cur_node;
      backend_descendant_nodes.push_back(rootid);
      if(threaded){ childnodebybackendid_sync.unlock(); }

      children_nodes.push_back(cur_node);
    }
  }

  if( num_descendants > 0){
    if( wait_for_SubTreeReports() == -1){
      mc_printf(MCFL, stderr, "wait_for_SubTreeReports() failed!\n");
      return -1;
    }
  }
  else{
    mc_printf(MCFL, stderr, "All my %d children must be BEs\n", 
	       children_nodes.size());
  }

  mc_printf(MCFL, stderr, "proc_newSubTree() succeeded\n");
  return 0;
}

int MC_ParentNode::wait_for_SubTreeReports(void){
  std::list <MC_Packet *> packet_list;
  if(threaded){
    subtreereport_sync.lock();
    while( num_descendants > num_descendants_reported){
      mc_printf(MCFL, stderr, "Waiting for downstream nodes_reported signal ...\n");
      subtreereport_sync.wait(MC_ALLNODESREPORTED);
      mc_printf(MCFL, stderr, "%d of %d Descendants have checked in.\n",
	         num_descendants_reported, num_descendants);
    }
    subtreereport_sync.unlock();
  }
  else{
    while( num_descendants > num_descendants_reported){
      mc_printf(MCFL, stderr, "%d of %d Descendants have checked in.\n",
	         num_descendants_reported, num_descendants);
      if(recv_PacketsFromDownStream(packet_list) == -1){
        mc_printf(MCFL, stderr, "recv_PacketsFromDownStream() failed\n");
	return -1;
      }
      if(proc_PacketsFromDownStream(packet_list) == -1){
        mc_printf(MCFL, stderr, "recv_PacketsFromDownStream() failed\n");
	return -1;
      }
    }
  }
  mc_printf(MCFL, stderr, "All %d downstream nodes have reported\n",
               num_descendants);
  return 0;
}

int MC_ParentNode::proc_delSubTree(MC_Packet * packet)
{
  unsigned int i;
  int retval=0;
  mc_printf(MCFL, stderr, "In proc_delSubTree()\n");

  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = children_nodes.begin();
      iter != children_nodes.end(); iter++, i++){
    if( (*iter)->send(packet) == -1){
      mc_printf(MCFL, stderr, "downstream.send() failed\n");
      retval = -1;
      continue;
    }
  }

  if(flush_PacketsDownStream() == -1){
    mc_printf(MCFL, stderr, "flush() failed. Now Exiting since delSubtree()\n");
    exit(-1);
  }

  mc_printf(MCFL, stderr, "internal.sendDelSubTree() succeeded. Now Exiting\n");
  exit(0);
  return 0;
}

int MC_ParentNode::proc_newSubTreeReport(MC_Packet *packet)
{
  int status;
  int *backends;
  int i, no_backends;

  mc_printf(MCFL, stderr, "In frontend.proc_newSubTreeReport()\n");
  if( packet->ExtractArgList("%d %ad", &status, &backends, &no_backends) == -1){
    mc_printf(MCFL, stderr, "ExtractArgList failed\n");
    return -1;
  }

  num_descendants_reported++;
  for(i=0; i<no_backends; i++){
    mc_printf(MCFL, stderr, "Adding backend %d to my list\n", backends[i]);
    backend_descendant_nodes.push_back(backends[i]);
  }

  mc_printf(MCFL, stderr, "Leaving frontend.proc_newSubTreeReport()\n");
  return status;
}
/* int MC_InternalNode::proc_newSubTreeReport(MC_Packet *packet)
{
  int status;
  int *backends;
  int i, no_backends;

  mc_printf(MCFL, stderr, "In internal.proc_newSubTreeReport()\n");
  if( packet->ExtractArgList("%d %ad", &status, &backends, &no_backends) == -1){
    mc_printf(MCFL, stderr, "ExtractArgList failed\n");
    return -1;
  }

  subtreereport_sync.lock();
  num_descendants_reported++;
  mc_printf(MCFL, stderr, "%d of %d descendants_reported\n",
	     num_descendants_reported,
             num_descendants);
  mc_printf(MCFL, stderr, "Adding %d backends [ ", no_backends);
  childnodebybackendid_sync.lock();
  for(i=0; i<no_backends; i++){
    backend_descendant_nodes.push_back(backends[i]);
    ChildNodeByBackendId[backends[i]] = packet->inlet_node;
    _fprintf((stderr, "%d(%p), ", backends[i],
              ChildNodeByBackendId[backends[i]]);
  }
  _fprintf((stderr, "]\n");
  mc_printf(MCFL, stderr, "map[%d] at %p = %p\n", 0, &ChildNodeByBackendId,
	     ChildNodeByBackendId[0]);
  childnodebybackendid_sync.unlock();

  if( num_descendants_reported == num_descendants){
    subtreereport_sync.signal(MC_ALLNODESREPORTED);
  }
  subtreereport_sync.unlock();

  mc_printf(MCFL, stderr, "Leaving internal.proc_newSubTreeReport()\n");
  return status;
}
*/
int MC_ParentNode::proc_newStream(MC_Packet * packet)
{
  unsigned int i, num_backends;
  int stream_id, filter_id, *backends, retval;

  std::list <MC_RemoteNode *> node_set;

  mc_printf(MCFL, stderr, "In proc_newSTream()\n");

  //assert( !MC_StreamImpl::get_Stream(stream_id));
  //register new stream, though not yet needed.
  //new MC_Stream(stream_id, backends, num_backends, filter_id);
  if( packet->ExtractArgList("%d %ad %d", &stream_id, &backends, &num_backends,
                             &filter_id) == -1){
    mc_printf(MCFL, stderr, "ExtractArgList() failed\n");
    return -1;
  }

  childnodebybackendid_sync.lock();
  mc_printf(MCFL, stderr, "map size: %d\n", ChildNodeByBackendId.size());
  for(i=0; i<num_backends; i++){
    mc_printf(MCFL, stderr, "map[%d] at %p = %p\n", backends[i], &ChildNodeByBackendId, ChildNodeByBackendId[backends[i]]);
    mc_printf(MCFL, stderr, "Adding Endpoint %d(%p) to stream %d\n", backends[i], 
               ChildNodeByBackendId[backends[i]], stream_id);
    node_set.push_back(ChildNodeByBackendId[backends[i]] );
  }
  childnodebybackendid_sync.unlock();
  
  //mc_printf(MCFL, stderr, "nodeset_size:%d\n", node_set.size());
  node_set.sort(lt_RemoteNodePtr);      //sort the set of nodes (by ptr value)
  //mc_printf(MCFL, stderr, "nodeset_size:%d\n", node_set.size());
  node_set.unique(equal_RemoteNodePtr); //remove duplicates
  //mc_printf(MCFL, stderr, "nodeset_size:%d\n", node_set.size());

  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = node_set.begin(); iter != node_set.end(); iter++, i++){
    if( (*iter) == NULL){ //temporary fix for adding unreachable backends
      node_set.erase(iter);
    }
    else{
      mc_printf(MCFL, stderr, "node[%d] in stream %d is %p\n", i, stream_id, *iter);
    }
  }

  MC_StreamManager * stream_mgr = new MC_StreamManager(stream_id, filter_id,
						       node_set);
  if(threaded){ streammanagerbyid_sync.lock(); }
  StreamManagerById[stream_id] = stream_mgr;
  if(threaded){ streammanagerbyid_sync.unlock(); }

  for(i=0,iter = node_set.begin(); iter != node_set.end(); iter++, i++){
    if( (*iter)->is_internal() ){ //only pass on to internal nodes
      //mc_printf(MCFL, stderr, "Calling node_set[%d].send() ...\n", i);
      if( (*iter)->send(packet) == -1){
        mc_printf(MCFL, stderr, "node_set.send() failed\n");
        retval = -1;
        continue;
      }
    }
    //mc_printf(MCFL, stderr, "node_set.send() succeeded\n");
  }

  mc_printf(MCFL, stderr, "internal.procNewStream() succeeded\n");
  return 0;
}

int MC_ParentNode::proc_delStream(MC_Packet * packet)
{
  int retval;
  unsigned int i;
  int stream_id;

  mc_printf(MCFL, stderr, "In proc_delStream()\n");

  streammanagerbyid_sync.lock();
  MC_StreamManager * stream_mgr = StreamManagerById[packet->get_StreamId()];
  streammanagerbyid_sync.unlock();

  if( packet->ExtractArgList("%d", &stream_id) == -1){
    mc_printf(MCFL, stderr, "ExtractArgList() failed\n");
    return -1;
  }
  assert(stream_id == packet->get_StreamId());

  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = stream_mgr->downstream_nodes.begin(); iter != stream_mgr->downstream_nodes.end(); iter++, i++){
    mc_printf(MCFL, stderr, "Calling node_set[%d].send() ...\n", i);
    if( (*iter)->send(packet) == -1){
      mc_printf(MCFL, stderr, "node_set.send() failed\n");
      retval = -1;
      continue;
    }
    mc_printf(MCFL, stderr, "node_set.send() succeeded\n");
  }

  streammanagerbyid_sync.lock();
  StreamManagerById.erase(stream_id);
  streammanagerbyid_sync.unlock();
  mc_printf(MCFL, stderr, "internal.procDelStream() succeeded\n");
  return 0;
}

int MC_ParentNode::proc_newApplication(MC_Packet * packet)
{
  unsigned int i;
  int retval =0;
  char *cmd;

  mc_printf(MCFL, stderr, "In proc_newApplication()\n");

  if( packet->ExtractArgList("%s", &cmd) == -1){
    mc_printf(MCFL, stderr, "ExtractArgList() failed\n");
    return -1;
  }

  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = children_nodes.begin();
      iter != children_nodes.end(); iter++, i++){
    mc_printf(MCFL, stderr, "processing %s child: %s\n",
	       ( (*iter)->is_internal() ? "internal" : "backend"),
	        (*iter)->get_HostName().c_str());
    if( (*iter)->is_internal()){
      mc_printf(MCFL, stderr, "Calling child_node.send() ...\n");
      if( (*iter)->send(packet) == -1){
        mc_printf(MCFL, stderr, "child_node.send() failed\n");
        retval = -1;
        continue;
      }
      mc_printf(MCFL, stderr, "child_node.send() succeeded\n");
    }
    else{
      std::string cmd_str(cmd);
      std::vector <std::string> dummy_args;
      mc_printf(MCFL, stderr, "Calling child_node.new_application(%s) ...\n", cmd);
      if( (*iter)->new_Application(listening_sock_fd, hostname, port,
                                   config_port, cmd_str, dummy_args) == -1){
	mc_printf(MCFL, stderr, "child_node.new_application() failed\n");
        retval = -1;
      }
      mc_printf(MCFL, stderr, "child_node.new_application() succeeded\n");
    }
  }

  mc_printf(MCFL, stderr, "Leaving proc_newApplication()\n");
  return retval;
}

int MC_ParentNode::proc_delApplication(MC_Packet * packet)
{
  unsigned int i;
  int retval=0;
  mc_printf(MCFL, stderr, "In proc_delApplication()\n");

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

  if(flush_PacketsDownStream() == -1){
    mc_printf(MCFL, stderr, "flush() failed.\n");
    return -1;
  }

  mc_printf(MCFL, stderr, "internal.sendDelApplication() succeeded.\n");
  return 0;
}

bool lt_RemoteNodePtr(MC_RemoteNode *p1, MC_RemoteNode *p2){
  if(p1->get_HostName() < p2->get_HostName()){
    return true;
  }
  else if(p1->get_HostName() == p2->get_HostName()){
    return (p1->get_Port() < p2->get_Port());
  }
  else{
    return false;
  }
}

bool equal_RemoteNodePtr(MC_RemoteNode *p1, MC_RemoteNode *p2){
  return p1 == p2; //tmp hack, should be something like below

  if(p1->get_HostName() != p2->get_HostName()){
    return false;
  }
  else{
    return (p1->get_Port() == p2->get_Port());
  }
}

std::string MC_ParentNode::get_HostName(){
  return hostname;
}

unsigned short MC_ParentNode::get_Port()
{
  return port;
}
