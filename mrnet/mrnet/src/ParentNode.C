#include <stdio.h>

#include "mrnet/src/ParentNode.h"
#include "mrnet/src/NetworkGraph.h"
#include "mrnet/src/utils.h"

/*====================================================*/
/*  MC_ParentNode CLASS METHOD DEFINITIONS            */
/*====================================================*/
std::map<unsigned int, MC_Aggregator::AggregatorSpec*>*
  MC_ParentNode::AggrSpecById = NULL;
std::map<unsigned int, void(*)(std::list <MC_Packet*>&, std::list <MC_Packet*>&,
                               std::list<MC_RemoteNode *> &, void **)>*
   MC_ParentNode::SyncById = NULL;

MC_ParentNode::MC_ParentNode(bool _threaded, std::string _hostname,
                             unsigned short _port)
  :hostname(_hostname), port(0), config_port(_port), listening_sock_fd(0),
   threaded(_threaded),
   isLeaf_(false),
   num_descendants(0), num_descendants_reported(0)
{
  mc_printf(MCFL, stderr, "In MC_ParentNode(): Calling bind_to_port(%d)\n", 
             port);
  if( bind_to_port(&listening_sock_fd, &port) == -1){
    mc_printf(MCFL, stderr, "bind_to_port() failed\n");
    //errno = MC_ECANNOTBINDPORT;
    return;
  }
  subtreereport_sync.register_cond(MC_ALLNODESREPORTED);

  //hardcoding some aggregator definitions
  if( AggrSpecById == NULL )
  {
        AggrSpecById = 
        new std::map<unsigned int, MC_Aggregator::AggregatorSpec*>;
  }
  MC_Aggregator::AggregatorSpec *tmp = new MC_Aggregator::AggregatorSpec;
  tmp->format_str = AGGR_NULL_FORMATSTR;
  tmp->filter = NULL;
  (*AggrSpecById)[AGGR_NULL] =tmp;

  tmp = new MC_Aggregator::AggregatorSpec;
  tmp->format_str = AGGR_INT_SUM_FORMATSTR;
  tmp->filter = aggr_Int_Sum;
  (*AggrSpecById)[AGGR_INT_SUM_ID] =tmp;

  tmp = new MC_Aggregator::AggregatorSpec;
  tmp->format_str = AGGR_FLOAT_AVG_FORMATSTR;
  tmp->filter = aggr_Float_Avg;
  (*AggrSpecById)[AGGR_FLOAT_AVG_ID] =tmp;

  tmp = new MC_Aggregator::AggregatorSpec;
  tmp->format_str = AGGR_FLOAT_MAX_FORMATSTR;
  tmp->filter = aggr_Float_Max;
  (*AggrSpecById)[AGGR_FLOAT_MAX_ID] =tmp;

  tmp = new MC_Aggregator::AggregatorSpec;
  tmp->format_str = AGGR_CHARARRAY_CONCAT_FORMATSTR;
  tmp->filter = aggr_CharArray_Concat;
  (*AggrSpecById)[AGGR_CHARARRAY_CONCAT_ID] =tmp;

  tmp = new MC_Aggregator::AggregatorSpec;
  tmp->format_str = AGGR_INT_EQ_CLASS_FORMATSTR;
  tmp->filter = aggr_IntEqClass;
  (*AggrSpecById)[AGGR_INT_EQ_CLASS_ID] = tmp;

  //hardcoding some synchonizer definitions
  if( SyncById == NULL )
  {
        SyncById = 
        new std::map<unsigned int, void(*)(std::list <MC_Packet*>&, std::list <MC_Packet*>&,
                               std::list<MC_RemoteNode *> &, void **)>;
  }
  (*SyncById)[SYNC_WAITFORALL] = sync_WaitForAll;
  (*SyncById)[SYNC_DONTWAIT] = sync_DontWait;
  (*SyncById)[SYNC_TIMEOUT] = sync_TimeOut;

  mc_printf(MCFL, stderr, "Leaving MC_ParentNode()\n");
}


MC_ParentNode::~MC_ParentNode(void)
{}

int MC_ParentNode::recv_PacketsFromDownStream(std::list<MC_Packet*>& pkt_list )
{
    int ret = 0;

    mc_printf( MCFL, stderr, "In recv_PacketsFromDownStream()\n" );


    // add all downstream remote nodes to the set to poll
    pollfd* pfds = new pollfd[ children_nodes.size() ];
    unsigned int npfds = children_nodes.size();
    unsigned int i = 0;
    for( std::vector<MC_RemoteNode*>::iterator iter = children_nodes.begin();
                iter != children_nodes.end();
                iter++, i++ )
    {
        MC_RemoteNode* currRemNode = *iter;
        assert( currRemNode != NULL );
        pfds[i].fd = currRemNode->get_sockfd();
        pfds[i].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI;
        pfds[i].revents = 0;
    }


    // check for input on our downstream connections
    int pollret = poll( pfds, npfds, 0 );

    if( pollret > 0 )
    {
        // there is input on some connection
        // determine the connection on which input exists
        MC_RemoteNode* readyNode = NULL;
        for( i = 0; i < children_nodes.size(); i++ )
        {
            if( pfds[i].revents & POLLIN )
            {
                readyNode = children_nodes[i];
                break;
            }
        }
        assert( readyNode != NULL );

        // receive data from the ready connection
        int rret = readyNode->recv( pkt_list );
        if( rret == -1 )
        {
            ret = -1;
            mc_printf(MCFL, stderr,
                 "recv_PacketsFromDownStream recv() from ready node failed\n" );
        }
    }
    else if( pollret < 0 )
    {
        // an error occurred
        ret = -1;
        mc_printf(MCFL, stderr, "recv_PacketsFromDownStream poll failed\n" );
    }
    delete[] pfds;

    mc_printf(MCFL, stderr, "recv_PacketsFromDownStream %s\n",
	     (ret == 0 ? "succeeded" : "failed") );

    return ret;
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

  std::vector<MC_RemoteNode *>::iterator iter;
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
  std::string application((appl == NULL) ? "" : appl);

  //printf(MCFL, stderr, "sg.root:%s:%d, local:%s:%d\n", sg.get_RootName().c_str(),
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

      mc_printf(MCFL, stderr, "cur_child(%s:%d) is backend[%d]\n",
                 rootname.c_str(), rootport, rootid);

      // since this child is a backend, I'm a leaf
      isLeaf_ = true;

      MC_RemoteNode *cur_node = new MC_RemoteNode(threaded,
                                                    rootname,
                                                    rootport,
                                                    cur_sg->get_Id() );

      if( application.length() > 0 )
      {
          std::vector <std::string> dummy_args;
          if( cur_node->new_Application(listening_sock_fd, hostname,
                                        port, config_port,
                                        application, dummy_args)== -1){
            mc_printf(MCFL, stderr, "child_node.new_application() failed\n");
            return -1;
          }
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

  std::vector<MC_RemoteNode *>::iterator iter;
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

  mc_printf(MCFL, stderr, "In parentnode.proc_newSubTreeReport()\n");
  if( packet->ExtractArgList("%d %ad", &status, &backends, &no_backends) == -1){
    mc_printf(MCFL, stderr, "ExtractArgList failed\n");
    return -1;
  }

  if( threaded ){ subtreereport_sync.lock( ); }
  num_descendants_reported++;
  mc_printf(MCFL, stderr, "%d of %d descendants_reported\n",
            num_descendants_reported, num_descendants);
  mc_printf(MCFL, stderr, "Adding %d backends [ ", no_backends);
  childnodebybackendid_sync.lock( );
  for(i=0; i<no_backends; i++){
   _fprintf((stderr, "%d(%p), ", backends[i], ChildNodeByBackendId[backends[i]]));
    ChildNodeByBackendId[backends[i]] = packet->inlet_node;
    backend_descendant_nodes.push_back(backends[i]);
  }
  _fprintf((stderr, "]\n"));
  mc_printf(MCFL, stderr, "map[%d] at %p = %p\n", 0, &ChildNodeByBackendId,
	     ChildNodeByBackendId[0]);
  childnodebybackendid_sync.unlock( );
  if( threaded ){ 
      if( num_descendants == num_descendants ){
          subtreereport_sync.signal(MC_ALLNODESREPORTED);
      }
      subtreereport_sync.unlock( );
  }

  mc_printf(MCFL, stderr, "Leaving parentnode.proc_newSubTreeReport()\n");
  return status;
}

MC_StreamManager *
MC_ParentNode::proc_newStream(MC_Packet * packet)
{
  unsigned int i, num_backends;
  int stream_id, filter_id, *backends;

  std::list <MC_RemoteNode *> node_set;

  mc_printf(MCFL, stderr, "In proc_newSTream()\n");

  //assert( !MC_StreamImpl::get_Stream(stream_id));
  //register new stream, though not yet needed.
  //new MC_Stream(stream_id, backends, num_backends, filter_id);
  if( packet->ExtractArgList("%d %ad %d", &stream_id, &backends, &num_backends,
                             &filter_id) == -1){
    mc_printf(MCFL, stderr, "ExtractArgList() failed\n");
    return NULL;
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
  
  //printf(MCFL, stderr, "nodeset_size:%d\n", node_set.size());
  //  node_set.sort(lt_RemoteNodePtr);      //sort the set of nodes (by ptr value)
  //printf(MCFL, stderr, "nodeset_size:%d\n", node_set.size());
  //  node_set.unique(equal_RemoteNodePtr); //remove duplicates
  //printf(MCFL, stderr, "nodeset_size:%d\n", node_set.size());
  node_set.sort();
  node_set.unique();

  std::list <MC_RemoteNode *>::iterator iter, del_iter;
  for(i=0,iter = node_set.begin(); iter != node_set.end(); i++){
    if( (*iter) == NULL){ //temporary fix for adding unreachable backends
        del_iter = iter;
        iter++;
        node_set.erase(del_iter);
    }
    else{
        iter++;
        mc_printf(MCFL, stderr, "node[%d] in stream %d is %p\n", i, stream_id, *iter);
    }
  }

  MC_StreamManager * stream_mgr = new MC_StreamManager(stream_id, filter_id,
						       node_set);
  if(threaded){ streammanagerbyid_sync.lock(); }
  mc_printf(MCFL, stderr, "DCA: adding stream_mgr(%p) to strmgr[%d]\n",
            stream_mgr, stream_id);
  StreamManagerById[stream_id] = stream_mgr;
  if(threaded){ streammanagerbyid_sync.unlock(); }

  mc_printf(MCFL, stderr, "internal.procNewStream() succeeded\n");
  return stream_mgr;
}

int
MC_ParentNode::send_newStream(MC_Packet * packet, MC_StreamManager * stream_mgr)
{
  int i, retval;
  std::list <MC_RemoteNode *> node_set = stream_mgr->downstream_nodes;
  std::list<MC_RemoteNode *>::iterator iter;

  for(i=0,iter = node_set.begin(); iter != node_set.end(); iter++, i++){
    if( (*iter)->is_internal() ){ //only pass on to internal nodes
      //printf(MCFL, stderr, "Calling node_set[%d].send() ...\n", i);
      if( (*iter)->send(packet) == -1){
        mc_printf(MCFL, stderr, "node_set.send() failed\n");
        retval = -1;
        continue;
      }
    }
    //printf(MCFL, stderr, "node_set.send() succeeded\n");
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

  std::vector<MC_RemoteNode *>::iterator iter;
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

  std::vector<MC_RemoteNode *>::iterator iter;
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
    assert(p1 && p2);

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




int
MC_ParentNode::proc_getLeafInfo( MC_Packet* pkt )
{
    int ret = 0;

    // request packet is empty -
    // no need to extract anything

    // determine if I am a leaf in the MRNet tree.
    //
    // NOTE: though the terminology is counter-intuitive, a ParentNode 
    // can be a leaf in the MRNet tree.  A ParentNode is a leaf in the 
    // tree if all of its children are backends.
    //
    // handle the request
    if( isLeaf() )
    {
        mc_printf(MCFL, stderr, "leaf at %s:%u\n", hostname.c_str(), port);

        // build our leaf connection info arrays
        unsigned int nBEs = children_nodes.size();
        int* beIds = new int[nBEs];
        const char** beHosts = new const char*[nBEs];
        int* beRanks = new int[nBEs];
        const char** myHosts = new const char*[nBEs];
        int* myPorts = new int[nBEs];
        int* myRanks = new int[nBEs];

        assert( ChildNodeByBackendId.size() == nBEs );
        unsigned int i = 0;
        for( std::map<unsigned int, MC_RemoteNode*>::iterator iter = 
                ChildNodeByBackendId.begin();
                iter != ChildNodeByBackendId.end();
                iter++ )
        {
            unsigned int curId = iter->first;
            MC_RemoteNode* curChild = iter->second;

            beIds[i] = curId;
            beHosts[i] = strdup( curChild->get_HostName().c_str() );
            beRanks[i] = curChild->get_Port();

            // my info doesn't change, regardless of the child's info
            myHosts[i] = strdup( hostname.c_str() );
            myPorts[i] = port;
            myRanks[i] = config_port;

            i++;
        }

        MC_Packet* resp = new MC_Packet( MC_GET_LEAF_INFO_PROT,
                                            "%ad %as %ad %as %ad %ad",
                                            beIds, nBEs,
                                            beHosts, nBEs,
                                            beRanks, nBEs,
                                            myHosts, nBEs,
                                            myPorts, nBEs,
                                            myRanks, nBEs );

        // deliver the response 
        // (how it is delivered depends on what type of 
        // ParentNode we actually are)
        if( deliverLeafInfoResponse( resp ) == -1 )
        {
            mc_printf( MCFL, stderr, "failed to deliver getLeafInfo response\n" );
            ret = -1;
        }
    }
    else
    {
        // forward the request packet to each of my children
        // TODO am I safe to reuse the same packet here
        for( std::vector<MC_RemoteNode*>::iterator childIter = 
                    children_nodes.begin();
                childIter != children_nodes.end();
                childIter++ )
        {
            MC_RemoteNode* child = *childIter;
            assert( child != NULL );

            if( (child->send( pkt ) == -1) ||
                (child->flush() == -1) )
            {
                mc_printf( MCFL, stderr, "failed to deliver getLeafInfo request to all children\n" );
                ret = -1;
                break;
            }
        }
    }
    return ret;
}



int
MC_ParentNode::proc_getLeafInfoResponse( MC_Packet* pkt )
{
    int ret = 0;

    // add the response to our set of child responses
    childLeafInfoResponses.push_back( pkt );

    // check if we've received responses from all of our children
    if( childLeafInfoResponses.size() == children_nodes.size() )
    {
        // we have received responses from all of our children -
        // aggregate the responses
        //
        // TODO use some sort of encoding to store leaf info?  is any possible?
        //
        // We just concatenate the responses
        // TODO can we just build a response packet by referring to
        // the data elements of all these other packets?  Doesn't look 
        // like it.
        //
        std::vector<unsigned int> allIds;
        std::vector<char*> allHosts;
        std::vector<unsigned int> allRanks;
        std::vector<char*> allParHosts;
        std::vector<unsigned int> allParPorts;
        std::vector<unsigned int> allParRanks;
        for( std::vector<MC_Packet*>::iterator piter = 
                            childLeafInfoResponses.begin();
                piter != childLeafInfoResponses.end();
                piter++ )
        {
            MC_Packet* currPkt = *piter;
            assert( currPkt != NULL );

            // get the data out of the current packet
            int* currIds = NULL;
            unsigned int currNumIds = 0;
            char** currHosts = NULL;
            unsigned int currNumHosts = 0;
            int* currRanks = NULL;
            unsigned int currNumRanks = 0;
            char** currParHosts = NULL;
            unsigned int currNumParHosts = 0;
            int* currParPorts = NULL;
            unsigned int currNumParPorts = 0;
            int* currParRanks = NULL;
            unsigned int currNumParRanks = 0;
            int eret = currPkt->ExtractArgList( "%ad %as %ad %as %ad %ad", 
                                        &currIds, &currNumIds,
                                        &currHosts, &currNumHosts,
                                        &currRanks, &currNumRanks,
                                        &currParHosts, &currNumParHosts,
                                        &currParPorts, &currNumParPorts,
                                        &currParRanks, &currNumParRanks );
            if( eret == -1 )
            {
                mc_printf( MCFL, stderr,
                    "failed to extract data from leaf info packet\n" );
                ret = -1;
                break;
            }
            assert( currNumHosts == currNumRanks );

            // add the new set of hosts and ports to our cumulative set
            for( unsigned int i = 0; i < currNumHosts; i++ )
            {
                fprintf( stderr, "MRN: ParentNode: got LeafInfo: %u %s %u %s %u %u\n",

                currIds[i], currHosts[i], currRanks[i], currParHosts[i], currParPorts[i], currParRanks[i] );
                allIds.push_back( currIds[i] );
                allHosts.push_back( currHosts[i] );
                allRanks.push_back( currRanks[i] );
                allParHosts.push_back( currParHosts[i] );
                allParPorts.push_back( currParPorts[i] );
                allParRanks.push_back( currParRanks[i] );
            }
        }

        // build our aggregate response packet
        // (sadly, we have to convert from STL vectors to parallel arrays
        // to build our packet - yet another copy operation)
        // TODO looks to me like there's no way to get at the underlying array
        //
        unsigned int nIds = allIds.size();
        unsigned int nHosts = allHosts.size();
        unsigned int nRanks = allRanks.size();
        unsigned int nParHosts = allParHosts.size();
        unsigned int nParPorts = allParPorts.size();
        unsigned int nParRanks = allParRanks.size();
        assert( nIds == nHosts );
        assert( nHosts == nRanks );
        assert( nHosts == nParHosts );
        assert( nHosts == nParPorts );
        assert( nHosts == nParRanks );
        int* idsArray = new int[nIds];
        char** hostsArray = new char*[nHosts];
        int* ranksArray = new int[nRanks];
        char** parHostsArray = new char*[nParHosts];
        int* parPortsArray = new int[nParPorts];
        int* parRanksArray = new int[nParRanks];
        for( unsigned int i = 0; i < nHosts; i++ )
        {
            idsArray[i] = allIds[i];
            hostsArray[i] = allHosts[i];
            ranksArray[i] = allRanks[i];
            parHostsArray[i] = allParHosts[i];
            parPortsArray[i] = allParPorts[i];
            parRanksArray[i] = allParRanks[i];

            fprintf( stderr, "ParNode: creating new leafinfo packet with %u %s %u %s %u %u\n", 
            idsArray[i], hostsArray[i], ranksArray[i],
            parHostsArray[i], parPortsArray[i], parRanksArray[i] );
        }

        MC_Packet* resp = new MC_Packet( MC_GET_LEAF_INFO_PROT,
                                            "%ad %as %ad %as %ad %ad",
                                            idsArray, nIds,
                                            hostsArray, nHosts,
                                            ranksArray, nRanks,
                                            parHostsArray, nParHosts,
                                            parPortsArray, nParPorts,
                                            parRanksArray, nParRanks );

        // handle the aggregated response
        if( deliverLeafInfoResponse( resp ) == -1 )
        {
            ret = -1;
        }

#if READY
        // release our children's responses
        for( std::vector<MC_Packet*>::iterator iter = childLeafInfoResponses.begin();
            iter != childLeafInfoResponses.end();
            iter++ )
        {
            delete *iter;
        }
#endif // READY
        childLeafInfoResponses.clear();
    }

    return ret;
}



int
MC_ParentNode::proc_connectLeaves( MC_Packet* pkt )
{
    int ret = 0;

    // request packet is empty -
    // no need to extract anything

    // determine if I am a leaf in the MRNet tree.
    //
    // NOTE: though the terminology is counter-intuitive, a ParentNode 
    // can be a leaf in the MRNet tree.  A ParentNode is a leaf in the 
    // tree if all of its children are backends.
    //
    // handle the request
    if( isLeaf() )
    {
        mc_printf(MCFL, stderr, "leaf waiting for connections at %s:%u\n", hostname.c_str(), port);

        // accept backend connections for all of our children nodes
        for( std::vector<MC_RemoteNode*>::iterator childIter = 
                        children_nodes.begin();
                    childIter != children_nodes.end();
                    childIter++ )
        {
            MC_RemoteNode* child = *childIter;
            assert( child != NULL );

            if( child->accept_Application( listening_sock_fd ) != 0 )
            {
                mc_printf( MCFL, stderr, "failed to accept backend connection\n" );
                ret = -1;
                break;
            }
        }

        if( ret == 0 )
        {
            // build a response packet 
            MC_Packet* resp = new MC_Packet( MC_CONNECT_LEAVES_PROT,
                                                "%ud",
                                                children_nodes.size() );

            // deliver the response 
            // (how it is delivered depends on what type of 
            // ParentNode we actually are)
            if( deliverConnectLeavesResponse( resp ) == -1 )
            {
                mc_printf( MCFL, stderr, "failed to deliver connected leaves response\n" );
                ret = -1;
            }
        }
    }
    else
    {
        // forward the request packet to each of my children
        // TODO am I safe to reuse the same packet here
        for( std::vector<MC_RemoteNode*>::iterator childIter = 
                    children_nodes.begin();
                childIter != children_nodes.end();
                childIter++ )
        {
            MC_RemoteNode* child = *childIter;
            assert( child != NULL );

            if( (child->send( pkt ) == -1) ||
                (child->flush() == -1) )
            {
                mc_printf( MCFL, stderr, "failed to deliver connectLeaves request to all children\n" );
                ret = -1;
                break;
            }
        }
    }
    return ret;
}



int
MC_ParentNode::proc_connectLeavesResponse( MC_Packet* pkt )
{
    int ret = 0;

    // add the response to our set of child responses
    childConnectedLeafResponses.push_back( pkt );

    // check if we've received responses from all of our children
    if( childConnectedLeafResponses.size() == children_nodes.size() )
    {
        // we have received responses from all of our children -
        // aggregate the responses
        unsigned int nLeavesConnected = 0;
        for( std::vector<MC_Packet*>::iterator iter = 
                    childConnectedLeafResponses.begin();
                iter != childConnectedLeafResponses.end();
                iter++ )
        {
            MC_Packet* currPkt = *iter;

            unsigned int currNumLeaves = 0;
            int eret = currPkt->ExtractArgList( "%ud", &currNumLeaves );
            if( eret == -1 )
            {
                mc_printf( MCFL, stderr,
                    "failed to extract data from connected leaves response packet\n" );
                ret = -1;
                break;
            }
            nLeavesConnected += currNumLeaves;
        }

        // build the response packet
        MC_Packet* resp = new MC_Packet( MC_CONNECT_LEAVES_PROT, "%ud",
                                            nLeavesConnected );


        // handle the aggregated response
        if( deliverConnectLeavesResponse( resp ) == -1 )
        {
            ret = -1;
        }

#if READY
        // release our children's responses
        for( std::vector<MC_Packet*>::iterator iter = childConnectedLeafResponses.begin();
            iter != childConnectedLeafResponses.end();
            iter++ )
        {
            delete *iter;
        }
#endif // READY
        childConnectedLeafResponses.clear();
    }

    return ret;
}



