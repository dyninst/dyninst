#include "mrnet/src/Types.h"

#include <fstream>
#include <sys/types.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include "mrnet/src/MC_CommunicationNode.h"
#include "mrnet/src/utils.h"
#include "mrnet/src/config.h"
#include "mrnet/src/pdr.h"

#include "mrnet/h/MC_Network.h"
#include "mrnet/src/MC_StreamImpl.h"

/*===========================================================*/
/*  MC_CommunicationNode CLASS METHOD DEFINITIONS            */
/*===========================================================*/
MC_CommunicationNode::MC_CommunicationNode()
  :hostname(getNetworkName()), port(0)
{
  mc_printf((stderr, "In MC_CommunicationNode(): %s:%d\n", hostname.c_str(), port));
}

MC_CommunicationNode::MC_CommunicationNode(string &_h, unsigned short _p)
  :hostname(getNetworkName(_h)), port(_p)
{
  mc_printf((stderr, "In MC_CommunicationNode(): %s:%d\n", hostname.c_str(), port));
}

string MC_CommunicationNode::get_HostName()
{
  return hostname;
}

unsigned short MC_CommunicationNode::get_Port()
{
  return port;
}

/*====================================================*/
/*  MC_RemoteNode CLASS METHOD DEFINITIONS            */
/*====================================================*/
MC_LocalNode * MC_RemoteNode::local_node;
void * MC_RemoteNode::recv_thread_main(void * args)
{
  list <MC_Packet *>packet_list;
  MC_RemoteNode * remote_node = (MC_RemoteNode *)args;

  while(1){
    if( remote_node->recv(packet_list) == 0 ){
      //error
    }

    if( remote_node->is_upstream() ){
      if(local_node->proc_PacketsFromUpStream(packet_list) == -1){
        mc_printf((stderr, "proc_PacketsFromUpstream() failed\n"));
      }
    }
    else{
      if(local_node->proc_PacketsFromDownStream(packet_list) == -1){
        mc_printf((stderr, "proc_PacketsFromUpstream() failed\n"));
      }
    }
  }

  return NULL;
}

void * MC_RemoteNode::send_thread_main(void * args)
{
  MC_RemoteNode * remote_node = (MC_RemoteNode *)args;

  while(1){
    if( remote_node->msg_out.size_Packets() != 0 ){
      if( remote_node->msg_out.send(remote_node->sock_fd) == -1 ){
        mc_printf((stderr, "proc_PacketsFromUpstream() failed\n"));
      }
    }
  }

  return NULL;
}

MC_RemoteNode::MC_RemoteNode(string &_hostname, unsigned short _port,
                             bool _threaded)
  :MC_CommunicationNode(_hostname, _port), sock_fd(0), _is_internal_node(false),
   threaded(_threaded)
{
}

int MC_RemoteNode::connect()
{
  int retval;
  mc_printf((stderr, "In connect(%s:%d) ... ", hostname.c_str(), port));
  if(connect_to_host(&sock_fd, hostname.c_str(), port) == -1){
    mc_printf((stderr, "connect_to_host() failed\n"));
    mc_errno = MC_ECREATPROCFAILURE;
    return -1;
  }

  if(threaded){
    retval = pthread_create(&recv_thread_id, NULL,
                            MC_RemoteNode::recv_thread_main, (void *) this);
    if(retval != 0){
      //thread create error
    }
    retval = pthread_create(&send_thread_id, NULL,
                            MC_RemoteNode::send_thread_main, (void *) this);
    if(retval != 0){
      //thread create error
    }
  }
  else{
    poll_struct.fd = sock_fd;
    poll_struct.events = POLLIN;
  }

  mc_printf((stderr, "connect_to_host() succeeded\n"));
  return 0;
}

int MC_RemoteNode::new_InternalNode(int listening_sock_fd, string parent_host,
                                    unsigned short parent_port)
{
  int retval;
  string rsh("");
  string username("");
  string cmd(COMMNODE_EXE);
  vector <string> args;

  _is_internal_node = true;

  args.push_back(parent_host);

  char parent_port_str[128];
  sprintf(parent_port_str, "%d", parent_port);
  args.push_back(string(parent_port_str));

  mc_printf((stderr, "Create InternalNode(%s:%d) ...", hostname.c_str(), port));
  if(create_Process(rsh, hostname, username, cmd, args) == -1){
    mc_printf((stderr, "createProcess() failed\n")); 
    mc_errno = MC_ECREATPROCFAILURE;
    return -1;
  }
  mc_printf((stderr, "createProcess() succeeded.\nWait for a connection...")); 

  if( (sock_fd = get_socket_connection(listening_sock_fd)) == -1){
    mc_printf((stderr, "get_socket_connection() failed\n"));
    mc_errno = MC_ESOCKETCONNECT;
    return -1;
  }
  mc_printf((stderr, "get_socket_connection() succeeded\n"));

  if(threaded){
    retval = pthread_create(&recv_thread_id, NULL,
                            MC_RemoteNode::recv_thread_main, (void *) this);
    if(retval != 0){
      //thread create error
    }
    retval = pthread_create(&send_thread_id, NULL,
                            MC_RemoteNode::send_thread_main, (void *) this);
    if(retval != 0){
      //thread create error
    }
  }
  else{
    poll_struct.fd = sock_fd;
    poll_struct.events = POLLIN;
  }

  return 0;
}

int MC_RemoteNode::new_Application(int listening_sock_fd, string parent_host,
                                   unsigned short parent_port, string &cmd,
                                   vector <string> &args){
  int retval;
  string rsh("");
  string username("");

  mc_printf((stderr, "In new_Application(%s:%d,\n"
                     "                   command: %s)\n",
                     hostname.c_str(), port, cmd.c_str()));
  
  //add current nodes hostname and port as very last args
  args.push_back(parent_host);
  char parent_port_str[128];
  sprintf(parent_port_str, "%d", parent_port);
  args.push_back(string(parent_port_str));

  if(create_Process(rsh, hostname, username, cmd, args) == -1){
    mc_printf((stderr, "createProcess() failed\n")); 
    mc_errno = MC_ECREATPROCFAILURE;
    return -1;
  }
  mc_printf((stderr, "createProcess() succeeded\n")); 
  if( (sock_fd = get_socket_connection(listening_sock_fd)) == -1){
    mc_printf((stderr, "get_socket_connection() failed\n"));
    mc_errno = MC_ESOCKETCONNECT;
    return -1;
  }
  mc_printf((stderr, "get_socket_connection() succeeded\n"));

  if(threaded){
    retval = pthread_create(&recv_thread_id, NULL,
                            MC_RemoteNode::recv_thread_main, (void *) this);
    if(retval != 0){
      //thread create error
    }
    retval = pthread_create(&send_thread_id, NULL,
                            MC_RemoteNode::send_thread_main, (void *) this);
    if(retval != 0){
      //thread create error
    }
  }
  else{
    poll_struct.fd = sock_fd;
    poll_struct.events = POLLIN;
  }

  return 0;
}

int MC_RemoteNode::send(MC_Packet *packet)
{
  mc_printf((stderr, "In remotenode.send(). Calling msg.add_packet()\n"));
  if(threaded){
    pthread_mutex_lock(&msg_out_mutex);
  }
  msg_out.add_Packet(packet);
  if(threaded){
    pthread_mutex_unlock(&msg_out_mutex);
  }
  mc_printf((stderr, "Leaving remotenode.send()\n"));
  return 0;
}

int MC_RemoteNode::recv(list <MC_Packet *> &packet_list)
{
  int retval;

  retval = msg_in.recv(sock_fd, packet_list, this);

  return retval;
}

bool MC_RemoteNode::has_data()
{
  poll_struct.revents = 0;

  mc_printf((stderr, "In remotenode.poll(%d)\n", poll_struct.fd));

  if(poll(&poll_struct, 1, 0) == -1){
    mc_printf((stderr, "poll() failed\n"));
    return false;
  }

  if(poll_struct.revents & POLLNVAL){
    mc_printf((stderr, "poll() says invalid request occured\n"));
    perror("poll()");
    return false;
  }
  if(poll_struct.revents & POLLERR){
    mc_printf((stderr, "poll() says error occured:"));
    perror("poll()");
    return false;
  }
  if(poll_struct.revents & POLLHUP){
    mc_printf((stderr, "poll() says hangup occured:"));
    perror("poll()");
    return false;
  }
  if(poll_struct.revents & POLLIN){
    mc_printf((stderr, "poll() says data to be read. calling recv()\n"));
    return true;
  }

  mc_printf((stderr, "No data currently available\n"));
  return false;
}

int MC_RemoteNode::flush()
{
  mc_printf((stderr, "In remotenode.flush(). Calling msg.send()\n"));
  if( msg_out.send(sock_fd) == -1){
    mc_printf((stderr, "msg.send() failed\n"));
    return -1;
  }
  else{
    mc_printf((stderr, "msg.send() succeeded\n"));
    return 0;
  }
}

bool MC_RemoteNode::is_backend(){
  return !_is_internal_node;
}

bool MC_RemoteNode::is_internal(){
  return _is_internal_node;
}

bool MC_RemoteNode::is_upstream(){
  return _is_upstream;
}

/*===================================================*/
/*  MC_LocalNode CLASS METHOD DEFINITIONS            */
/*===================================================*/
std::map<unsigned int, MC_Aggregator::AggregatorSpec*>MC_LocalNode::AggrSpecById;
std::map<unsigned int, void(*)(std::list <MC_Packet*>&, std::list <MC_Packet*>&,
                               std::list<MC_RemoteNode *> &)>
   MC_LocalNode::SyncById;

MC_LocalNode::MC_LocalNode()
  :listening_sock_fd(0), num_descendants_reported(0)
{
  //hardcoding some aggregator definitions
  MC_Aggregator::AggregatorSpec *tmp = new MC_Aggregator::AggregatorSpec;
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
}

MC_LocalNode::~MC_LocalNode()
{
}

int MC_LocalNode::recv_PacketsFromUpStream(std::list <MC_Packet *> &packet_list)
{
  mc_printf((stderr, "In recv_PacketsFromUpStream()\n"));
  return upstream_node->recv(packet_list);
}

int MC_LocalNode::proc_PacketsFromUpStream(std::list <MC_Packet *> &packets)
{
  int retval=0;
  MC_Packet *cur_packet;

  mc_printf((stderr, "In proc_PacketsFromUpStream()\n"));

  std::list<MC_Packet *>::iterator iter=packets.begin();
  for(; iter != packets.end(); iter++){
    cur_packet = (*iter);
    switch(cur_packet->get_Tag()){
    case MC_NEW_SUBTREE_PROT:
      mc_printf((stderr, "Calling proc_newSubTree()\n"));
      if(proc_newSubTree(cur_packet) == -1){
	mc_printf((stderr, "proc_newSubTree() failed\n"));
	retval=-1;
      }
      mc_printf((stderr, "proc_newsubtree() succeded\n"));
      break;
    case MC_DEL_SUBTREE_PROT:
      mc_printf((stderr, "Calling proc_delSubTree()\n"));
      if(proc_delSubTree(cur_packet) == -1){
	mc_printf((stderr, "proc_delSubTree() failed\n"));
	retval=-1;
      }
      mc_printf((stderr, "proc_delSubTree() succeded\n"));
      break;
    case MC_NEW_APPLICATION_PROT:
      mc_printf((stderr, "Calling proc_newApplication()\n"));
      if(proc_newApplication(cur_packet) == -1){
	mc_printf((stderr, "proc_newApplication() failed\n"));
	retval=-1;
      }
      mc_printf((stderr, "proc_newApplication() succeded\n"));
      break;
    case MC_DEL_APPLICATION_PROT:
      mc_printf((stderr, "Calling proc_delApplication()\n"));
      if(proc_delApplication(cur_packet) == -1){
	mc_printf((stderr, "proc_delApplication() failed\n"));
	retval=-1;
      }
      mc_printf((stderr, "proc_delApplication() succeded\n"));
      break;
    case MC_NEW_STREAM_PROT:
      mc_printf((stderr, "Calling proc_newStream()\n"));
      if(proc_newStream(cur_packet) == -1){
	mc_printf((stderr, "proc_newStream() failed\n"));
	retval=-1;
      }
      mc_printf((stderr, "proc_newStream() succeded\n"));
      break;
    case MC_DEL_STREAM_PROT:
      mc_printf((stderr, "Calling proc_delStream()\n"));
      if(proc_delStream(cur_packet) == -1){
	mc_printf((stderr, "proc_delStream() failed\n"));
	retval=-1;
      }
      mc_printf((stderr, "proc_delStream() succeded\n"));
      break;
    default:
      //Any Unrecognized tag is assumed to be data
      mc_printf((stderr, "Calling proc_DataFromUpStream(). Tag: %d\n",cur_packet->get_Tag()));
      if(proc_DataFromUpStream(cur_packet) == -1){
	mc_printf((stderr, "proc_DataFromUpStream() failed\n"));
	retval=-1;
      }
      mc_printf((stderr, "proc_DataFromUpStream() succeded\n"));
      break;
    }
  }

  packets.clear();
  mc_printf((stderr, "proc_PacketsFromUpStream() %s",
             (retval == -1 ? "failed\n" : "succeeded\n")));
  return retval;
}

int MC_LocalNode::send_PacketUpStream(MC_Packet *packet)
{
  mc_printf((stderr, "In send_PacketUpStream()\n"));
  return upstream_node->send(packet);
}

int MC_LocalNode::flush_PacketsUpStream()
{
  mc_printf((stderr, "In flush_PacketsUpStream()\n"));
  return upstream_node->flush();
}

int MC_LocalNode::recv_PacketsFromDownStream(std::list <MC_Packet *> &packet_list)
{
  unsigned int i;
  int retval=0;

  mc_printf((stderr, "In recv_PacketsFromDownStream()\n"));

  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = children_nodes.begin();
      iter != children_nodes.end(); iter++, i++){
    mc_printf((stderr, "Calling downstream[%d].recv() ...\n", i));
    if( (*iter)->recv(packet_list) == -1){
      mc_printf((stderr, "downstream.recv() failed\n"));
      retval = -1;
      continue;
    }
    mc_printf((stderr, "downstream.recv() succeeded\n"));
  }

  mc_printf((stderr, "recv_PacketsFromDownStream %s",
	     retval == 0 ? "succeeded\n" : "failed\n"));
  return retval;
}

int MC_LocalNode::proc_PacketsFromDownStream(std::list <MC_Packet *> &packet_list)
{
  int retval=0;
  MC_Packet *cur_packet;

  mc_printf((stderr, "In procfromdownstream()\n"));

  std::list<MC_Packet *>::iterator iter=packet_list.begin();
  for(; iter != packet_list.end(); iter++){
    cur_packet = (*iter);
    switch(cur_packet->get_Tag()){
    case MC_RPT_SUBTREE_PROT:
      mc_printf((stderr, "Calling proc_newSubTreeReport()\n"));
      if(proc_newSubTreeReport(cur_packet) == -1){
	mc_printf((stderr, "proc_newSubTreeReport() failed\n"));
	retval=-1;
      }
      mc_printf((stderr, "proc_newSubTreeReport() succeeded\n"));
      break;
    default:
      //Any unrecognized tag is assumed to be data
      mc_printf((stderr, "Calling proc_DataFromDownStream(). Tag: %d", cur_packet->get_Tag()));
      if(proc_DataFromDownStream(cur_packet) == -1){
	mc_printf((stderr, "proc_DataFromDownStream() failed\n"));
	retval=-1;
      }
      mc_printf((stderr, "proc_DataFromDownStream() succeeded\n"));
    }
  }

  mc_printf((stderr, "proc_PacketsFromDownStream() %s",
             (retval == -1 ? "failed\n" : "succeeded\n")));
  packet_list.clear();
  return retval;
}


int MC_LocalNode::send_PacketDownStream(MC_Packet *packet)
{
  unsigned int i;
  int retval=0;
  MC_StreamManager * stream_mgr;

  mc_printf((stderr, "In send_PacketDownStream()\n"));
  mc_printf((stderr, "StreamMangerById[%d] = %p\n",
             packet->get_StreamId(),
	     StreamManagerById[packet->get_StreamId()]));
  stream_mgr = StreamManagerById[packet->get_StreamId()];

  std::list <MC_RemoteNode *>::iterator iter;

  for(i=0,iter = stream_mgr->downstream_nodes.begin();
      iter != stream_mgr->downstream_nodes.end();
      iter++, i++){
    mc_printf((stderr, "Calling downstream[%d].send() ...\n", i));
    if( (*iter)->send(packet) == -1){
      mc_printf((stderr, "downstream.send() failed\n"));
      retval = -1;
    }
    mc_printf((stderr, "downstream.send() succeeded\n"));
  }

  mc_printf((stderr, "send_PacketDownStream %s",
	     retval == 0 ? "succeeded\n" : "failed\n"));
  return retval;
}

int MC_LocalNode::flush_PacketsDownStream()
{
  unsigned int i;
  int retval=0;

  mc_printf((stderr, "In flush_PacketsDownStream()\n"));

  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = children_nodes.begin();
      iter != children_nodes.end(); iter++, i++){
    mc_printf((stderr, "Calling downstream[%d].flush() ...\n", i));
    if( (*iter)->flush() == -1){
      mc_printf((stderr, "downstream.flush() failed\n"));
      retval = -1;
    }
    mc_printf((stderr, "downstream.flush() succeeded\n"));
  }

  mc_printf((stderr, "flush_PacketsFromDownStream %s",
	     retval == 0 ? "succeeded\n" : "failed\n"));
  return retval;
}

int MC_LocalNode::flush_PacketsDownStream(unsigned int stream_id)
{
  unsigned int i;
  int retval=0;
  MC_StreamManager * stream_mgr;

  mc_printf((stderr, "In flush_PacketsDownStream(%d)\n", stream_id));
  stream_mgr = StreamManagerById[stream_id];

  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = stream_mgr->downstream_nodes.begin();
      iter != stream_mgr->downstream_nodes.end();
      iter++, i++){
    mc_printf((stderr, "Calling downstream[%d].flush() ...\n", i));
    if( (*iter)->flush() == -1){
      mc_printf((stderr, "downstream.flush() failed\n"));
      retval = -1;
    }
    mc_printf((stderr, "downstream.flush() succeeded\n"));
  }

  mc_printf((stderr, "flush_PacketsFromDownStream %s",
	     retval == 0 ? "succeeded\n" : "failed\n"));
  return retval;
}

/*======================================================*/
/*  MC_InternalNode CLASS METHOD DEFINITIONS            */
/*======================================================*/
MC_InternalNode::MC_InternalNode(string _parent_hostname, unsigned short _parent_port)
  :MC_LocalNode()
{
  upstream_node = new MC_RemoteNode(_parent_hostname, _parent_port, true);

  mc_printf((stderr, "Calling connect() ..."));
  upstream_node->connect();
  if(upstream_node->fail() ){
    mc_printf((stderr, "connect() failed\n"));
    mc_errno = MC_ECANNOTBINDPORT;
    return;
  }

  mc_printf((stderr, "connect() succeeded.\nCalling bind_to_port(%d)...", port));
  if( bind_to_port(&listening_sock_fd, &port) == -1){
    mc_printf((stderr, "bind_to_port() failed\n"));
    mc_errno = MC_ECANNOTBINDPORT;
    return;
  }
  mc_printf((stderr, "bind_to_port() succeeded\n"));
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

int MC_InternalNode::proc_newSubTree(MC_Packet *packet)
{
  std::list <MC_Packet *> packet_list;
  char *byte_array;
  string cmd_commnode(COMMNODE_EXE);
  vector <string> arglist_commnode;
  arglist_commnode.push_back(getHostName());
  int num_descendants_to_report=0;

  char port_str[128];
  sprintf(port_str, "%d", port);
  arglist_commnode.push_back(string(port_str));
  string rootname;

  mc_printf((stderr, "In proc_newSubTree()\n"));

  mc_printf((stderr, "Calling ExtractArgList()\n"));
  if( packet->ExtractArgList("%s", &byte_array) == -1){
    mc_printf((stderr, "ExtractArgList() failed\n"));
    return -1;
  }
  mc_printf((stderr, "ExtractArgList() succeeded\n"));

  string s(byte_array);
  MC_SerialGraph sg( s );
  mc_printf((stderr, "sg.root:%s, local:%s\n", sg.get_RootName().c_str(),
	     hostname.c_str()));
  assert(getNetworkName(sg.get_RootName()) == hostname);

  MC_SerialGraph *cur_sg;

  sg.set_ToFirstChild();
  for(cur_sg = sg.get_NextChild(); cur_sg; cur_sg = sg.get_NextChild()){
    mc_printf((stderr, "cur_child: %s\n", cur_sg->get_RootName().c_str()));

    if(cur_sg->has_children()){
      num_descendants_to_report++;
      mc_printf((stderr, "cur_child has children() (internal node)\n"));
      //If the cur_child has children launch him
      rootname = cur_sg->get_RootName();
      MC_RemoteNode *cur_node = new MC_RemoteNode(rootname, 0, true);

      mc_printf((stderr, "Calling new_InternalNode()\n"));
      cur_node->new_InternalNode(listening_sock_fd, hostname, port);
      mc_printf((stderr, "new_InternalNode() completed\n"));

      if(cur_node->good() ){
	mc_printf((stderr, "Calling new Packet()\n"));
        packet = new MC_Packet(MC_NEW_SUBTREE_PROT, "%s", cur_sg->get_ByteArray().c_str());
      mc_printf((stderr, "new Packet() completed. Calling send/flush\n"));
        if( cur_node->send(packet) == -1 ||
            cur_node->flush() == -1){
          mc_printf((stderr, "send/flush failed\n"));
          return -1;
        }
        mc_printf((stderr, "send/flush succeeded\n"));
        children_nodes.push_back(cur_node);
      }
    }
    else{
      string rootname = cur_sg->get_RootName();
      string dummy_cmd;
      vector <string> dummy_args;
      mc_printf((stderr, "cur_child is backend[%d]\n", cur_sg->get_Id()));
      mc_printf((stderr, "Calling new RemoteNode(%s)\n", cur_sg->get_RootName().c_str()));
      MC_RemoteNode *cur_node = new MC_RemoteNode(rootname, 0, true);
      mc_printf((stderr, "new RemoteNode() completed\n"));
      //If the cur_child is a backend, just "record" his Id
      ChildNodeByBackendId[cur_sg->get_Id()] = cur_node;
      backend_descendant_nodes.push_back(cur_sg->get_Id());
      children_nodes.push_back(cur_node);
    }
  }

  while( num_descendants_to_report > num_descendants_reported){
    mc_printf((stderr, "Calling recv/proc packets()\n"));
    if( recv_PacketsFromDownStream(packet_list) == -1 ||
	proc_PacketsFromDownStream(packet_list) == -1){
      mc_printf((stderr, "recv/proc failed\n"));
      return -1;
    }
    mc_printf((stderr, "recv/proc succeeded\n"));
  }

  mc_printf((stderr, "Calling send_newsubtreereport\n"));
  if( send_newSubTreeReport(true) == -1){
    mc_printf((stderr, "send_newsubtreereport() failed\n"));
    return -1;
  }
  mc_printf((stderr, "send_newsubtreereport() succeeded\n"));

  mc_printf((stderr, "proc_newSubTree() succeeded\n"));
  return 0;
}

int MC_InternalNode::proc_delSubTree(MC_Packet *packet)
{
  unsigned int i;
  int retval=0;
  mc_printf((stderr, "In proc_delSubTree()\n"));
  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = children_nodes.begin();
      iter != children_nodes.end(); iter++, i++){
    if( (*iter)->send(packet) == -1){
      mc_printf((stderr, "downstream.send() failed\n"));
      retval = -1;
      continue;
    }
    mc_printf((stderr, "downstream.send() succeeded\n"));
  }

  if(flush_PacketsDownStream() == -1){
    mc_printf((stderr, "flush() failed. Now Exiting since delSubtree()\n"));
    exit(-1);
  }

  mc_printf((stderr, "internal.sendDelSubTree() succeeded. Now Exiting\n"));
  exit(0);
  return 0;
}

int MC_InternalNode::proc_newStream(MC_Packet *packet)
{
  unsigned int i, num_backends;
  int * backends, filter_id, stream_id, retval;
  std::list <MC_RemoteNode *> node_set;

  mc_printf((stderr, "In proc_newSTream()\n"));

  mc_printf((stderr, "Calling ExtractArgList()\n"));
  if( packet->ExtractArgList("%d %ad %d", &stream_id, &backends, &num_backends,
                             &filter_id) == -1){
    mc_printf((stderr, "ExtractArgList() failed\n"));
    return -1;
  }
  mc_printf((stderr, "ExtractArgList() succeeded\n"));

  //assert( !MC_StreamImpl::get_Stream(stream_id));
  //register new stream, though not yet needed.
  //new MC_Stream(stream_id, backends, num_backends, filter_id);

  for(i=0; i<num_backends; i++){
    mc_printf((stderr, "Adding Endpoint %d to stream %d\n", backends[i], stream_id));
    node_set.push_back(ChildNodeByBackendId[backends[i]] );
  }
  
  node_set.sort(lt_RemoteNodePtr);      //sort the set of nodes (by ptr value)
  node_set.unique(equal_RemoteNodePtr); //remove duplicates

  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = node_set.begin(); iter != node_set.end(); iter++, i++){
    if( (*iter) == NULL){ //temporary fix for adding unreachable backends
      node_set.erase(iter);
    }
  }

  MC_StreamManager * stream_mgr = new MC_StreamManager(stream_id, filter_id,
						       upstream_node, node_set);
  StreamManagerById[stream_id] = stream_mgr;

  for(i=0,iter = node_set.begin(); iter != node_set.end(); iter++, i++){
    if( (*iter)->is_internal() ){ //only pass on to internal nodes
      mc_printf((stderr, "Calling node_set[%d].send() ...\n", i));
      if( (*iter)->send(packet) == -1){
        mc_printf((stderr, "node_set.send() failed\n"));
        retval = -1;
        continue;
      }
    }
    mc_printf((stderr, "node_set.send() succeeded\n"));
  }

  mc_printf((stderr, "internal.procNewStream() succeeded\n"));
  return 0;
}

int MC_InternalNode::proc_delStream(MC_Packet *packet)
{
  int retval;
  unsigned int i;
  int stream_id;

  mc_printf((stderr, "In proc_delStream()\n"));

  MC_StreamManager * stream_mgr = StreamManagerById[packet->get_StreamId()];

  mc_printf((stderr, "Calling ExtractArgList()\n"));
  if( packet->ExtractArgList("%d", &stream_id) == -1){
    mc_printf((stderr, "ExtractArgList() failed\n"));
    return -1;
  }
  mc_printf((stderr, "ExtractArgList() succeeded\n"));
  assert(stream_id == packet->get_StreamId());

  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = stream_mgr->downstream_nodes.begin(); iter != stream_mgr->downstream_nodes.end(); iter++, i++){
    mc_printf((stderr, "Calling node_set[%d].send() ...\n", i));
    if( (*iter)->send(packet) == -1){
      mc_printf((stderr, "node_set.send() failed\n"));
      retval = -1;
      continue;
    }
    mc_printf((stderr, "node_set.send() succeeded\n"));
  }

  StreamManagerById.erase(stream_id);
  mc_printf((stderr, "internal.procDelStream() succeeded\n"));
  return 0;
}

int MC_InternalNode::proc_newApplication(MC_Packet *packet)
{
  unsigned int i;
  int retval =0;
  char *cmd;

  mc_printf((stderr, "In proc_newApplication()\n"));

  mc_printf((stderr, "Calling ExtractArgList()\n"));
  if( packet->ExtractArgList("%s", &cmd) == -1){
    mc_printf((stderr, "ExtractArgList() failed\n"));
    return -1;
  }
  mc_printf((stderr, "ExtractArgList() succeeded\n"));

  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = children_nodes.begin();
      iter != children_nodes.end(); iter++, i++){
    mc_printf((stderr, "processing %s child: %s\n",
	       ( (*iter)->is_internal() ? "internal" : "backend"),
	        (*iter)->get_HostName().c_str()));
    if( (*iter)->is_internal()){
      mc_printf((stderr, "Calling child_node.send() ...\n"));
      if( (*iter)->send(packet) == -1){
        mc_printf((stderr, "child_node.send() failed\n"));
        retval = -1;
        continue;
      }
      mc_printf((stderr, "child_node.send() succeeded\n"));
    }
    else{
      string cmd_str(cmd);
      vector <string> dummy_args;
      mc_printf((stderr, "Calling child_node.new_application(%s) ...\n", cmd));
      if( (*iter)->new_Application(listening_sock_fd, hostname, port, cmd_str, dummy_args) == -1){
	mc_printf((stderr, "child_node.new_application() failed\n"));
        retval = -1;
      }
      mc_printf((stderr, "child_node.new_application() succeeded\n"));
    }
  }
  return retval;
}

int MC_InternalNode::proc_delApplication(MC_Packet *packet)
{
  unsigned int i;
  int retval=0;
  mc_printf((stderr, "In proc_delApplication()\n"));

  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = children_nodes.begin();
      iter != children_nodes.end(); iter++, i++){
    mc_printf((stderr, "Calling downstream[%d].send() ...\n", i));
    if( (*iter)->send(packet) == -1){
      mc_printf((stderr, "downstream.send() failed\n"));
      retval = -1;
      continue;
    }
    mc_printf((stderr, "downstream.send() succeeded\n"));
  }

  if(flush_PacketsDownStream() == -1){
    mc_printf((stderr, "flush() failed.\n"));
    return -1;
  }

  mc_printf((stderr, "internal.sendDelApplication() succeeded.\n"));
  return 0;
}

int MC_InternalNode::proc_DataFromUpStream(MC_Packet *packet)
{
  int retval;
  unsigned int i;

  mc_printf((stderr, "In proc_DataFromUpStream()\n"));

  MC_StreamManager *stream_mgr = StreamManagerById[packet->get_StreamId()];

  std::list <MC_RemoteNode *>::iterator iter;
  for(i=0,iter = stream_mgr->downstream_nodes.begin();
      iter != stream_mgr->downstream_nodes.end();
      iter++, i++){
    mc_printf((stderr, "Calling node_set[%d].send() ...\n", i));
    if( (*iter)->send(packet) == -1){
      mc_printf((stderr, "node_set.send() failed\n"));
      retval = -1;
      continue;
    }
    mc_printf((stderr, "node_set.send() succeeded\n"));
  }

  mc_printf((stderr, "internal.procDataFromUpStream() succeeded\n"));
  return 0;
}

int MC_InternalNode::send_DataUpStream(MC_Packet *)
{
  return 0;
}

int MC_InternalNode::send_newSubTreeReport(bool status)
{
  unsigned int *backends, i;
  mc_printf((stderr, "In send_newSubTreeReport()\n"));

  backends = (unsigned int*)malloc
             (sizeof(unsigned int)*backend_descendant_nodes.size());
  assert(backends);

  std::list <int>::iterator iter;
  for(i=0, iter=backend_descendant_nodes.begin();
      iter != backend_descendant_nodes.end();
      i++, iter++){
    backends[i] = *iter;
    mc_printf((stderr, "Adding backend %d to my report\n", backends[i]));
  }

  mc_printf((stderr, "Calling new packet()\n"));
  MC_Packet *packet = new MC_Packet(MC_RPT_SUBTREE_PROT, "%d %ad", status,
				    backends, backend_descendant_nodes.size());
  if(packet->good()){
    mc_printf((stderr, "new packet() succeeded\n"));
    mc_printf((stderr, "Calling upstream.send/flush()\n"));
    if( upstream_node->send(packet) == -1 ||
        upstream_node->flush() == -1){
      mc_printf((stderr, "send/flush failed\n"));
      return -1;
    }
    mc_printf((stderr, "send/flush succeeded\n"));
  }
  else{
    mc_printf((stderr, "new packet() failed\n"));
    return -1;
  }

  mc_printf((stderr, "send_newSubTreeReport() succeeded\n"));
  return 0;
}

/*
int MC_InternalNode::send_newSubTree(MC_SerialGraph &)
{
  return 0;
}

int MC_InternalNode::send_delSubTree()
{
  return 0;
}

int MC_InternalNode::send_newStream(int, int)
{
  return 0;
}

int MC_InternalNode::send_delStream(int)
{
  return 0;
}

int MC_InternalNode::send_newApplication(string, vector<string> args)
{
  return 0;
}

int MC_InternalNode::send_delApplication()
{
  return 0;
}

int MC_InternalNode::send_DataDownStream(MC_Packet *)
{
  return 0;
}
*/

int MC_InternalNode::proc_newSubTreeReport(MC_Packet *packet)
{
  int status;
  int *backends;
  int i, no_backends;

  mc_printf((stderr, "In internal.proc_newSubTreeReport()\n"));
  if( packet->ExtractArgList("%d %ad", &status, &backends, &no_backends) == -1){
    mc_printf((stderr, "ExtractArgList failed\n"));
    return -1;
  }
  mc_printf((stderr, "ExtractArgList succeeded\n"));

  mc_printf((stderr, "report status: %d\n", status));
  num_descendants_reported++;
  for(i=0; i<no_backends; i++){
    mc_printf((stderr, "Adding backend %d to my list\n", backends[i]));
    backend_descendant_nodes.push_back(backends[i]);
    ChildNodeByBackendId[backends[i]] = packet->inlet_node;
  }

  mc_printf((stderr, "Leaving internal.proc_newSubTreeReport()\n"));
  return status;
}

int MC_InternalNode::proc_DataFromDownStream(MC_Packet *packet)
{
  mc_printf((stderr, "In proc_DataFromDownStream()\n"));

  mc_printf((stderr, "Calling upstream.send() ...\n"));
  if(upstream_node->send(packet) == -1){
    mc_printf((stderr, "upstream.send() failed\n"));
    return -1;
  }
  mc_printf((stderr, "upstream.send() succeeded\n"));

  return 0;
}

/*======================================================*/
/*  MC_FrontEndNode CLASS METHOD DEFINITIONS            */
/*======================================================*/

MC_FrontEndNode::MC_FrontEndNode()
 :listening_sock_fd(0)
{
  mc_printf((stderr, "MC_FrontEndNode(): Calling bind_to_port(%d)\n", port));
  if( bind_to_port(&listening_sock_fd, &port) == -1){
    mc_printf((stderr, "bind_to_port() failed\n"));
    mc_errno = MC_ECANNOTBINDPORT;
    return;
  }
  mc_printf((stderr, "bind_to_port() succeeded\n"));
}

MC_FrontEndNode::~MC_FrontEndNode()
{
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
  mc_printf((stderr, "In frontend.recv(). Calling recvfromdownstream()\n"));

  if(recv_PacketsFromDownStream(packet_list) == -1){
    mc_printf((stderr, "recv_packetsfromdownstream() failed\n"));
    return -1;
  }
  mc_printf((stderr, "recv_packetsfromdownstream() succeeded. call proc()\n"));

  if(packet_list.size() == 0){
    mc_printf((stderr, "No packets read!\n"));
    return 0;
  }

  if(proc_PacketsFromDownStream(packet_list) == -1){
    mc_printf((stderr, "proc_packetsfromdownstream() failed\n"));
    return -1;
  }
  mc_printf((stderr, "proc_packetsfromdownstream() succeeded\n"));
  mc_printf((stderr, "Leaving frontend.recv()\n"));
  return 1;
}

/* int MC_FrontEndNode::proc_newSubTree(MC_Packet *)
{
  assert(0);
  return(0);
} 

int MC_FrontEndNode::proc_delSubTree(MC_Packet *)
{
  assert(0);
  return(0);
}

int MC_FrontEndNode::proc_newStream(MC_Packet *)
{
  assert(0);
  return(0);
}

int MC_FrontEndNode::proc_delStream(MC_Packet *)
{
  assert(0);
  return(0);
}

int MC_FrontEndNode::proc_newApplication(MC_Packet *)
{
  assert(0);
  return(0);
}

int MC_FrontEndNode::proc_delApplication(MC_Packet *)
{
  assert(0);
  return(0);
}

int MC_FrontEndNode::proc_DataFromUpStream(MC_Packet *)
{
  assert(0);
  return(0);
}

int MC_FrontEndNode::send_newSubTreeReport(bool status)
{
  assert(0);
  return(0);
}

int MC_FrontEndNode::send_DataUpStream(MC_Packet *)
{
  assert(0);
  return(0);
}
*/

int MC_FrontEndNode::send_newSubTree(MC_SerialGraph &sg)
{
  int num_descendants_to_report=0;
  std::list <MC_Packet *> packet_list;
  MC_Packet * packet;
  string cmd_commnode(COMMNODE_EXE);
  vector <string> arglist_commnode;
  string rootname(sg.get_RootName());

  mc_printf((stderr, "In frontend.sendnewsubtree()\n"));

  mc_printf((stderr, "Calling new remotenode()\n"));
  MC_RemoteNode *cur_node = new MC_RemoteNode(rootname, 0, false);

  mc_printf((stderr, "Calling new_InternalNode()\n"));
  cur_node->new_InternalNode(listening_sock_fd, hostname, port);
  mc_printf((stderr, "new_InternalNode() completed\n"));

  if(cur_node->good() ){
    mc_printf((stderr, "new remotenode() succeeded\n"));
    mc_printf((stderr, "Calling new packet\n"));
    packet = new MC_Packet(MC_NEW_SUBTREE_PROT, "%s", sg.get_ByteArray().c_str());
    mc_printf((stderr, "new packet() returned\n"));

    if( packet->good() ){
      mc_printf((stderr, "new packet() succeeded\n"));
      if( cur_node->send(packet) == -1 ||
          cur_node->flush() == -1){
        mc_printf((stderr, "send_newsubtree():send/flush failed\n"));
        return -1;
      }
    }
    else{
      mc_printf((stderr, "new packet() failed\n"));
      return -1;
    }
    num_descendants_to_report++;
    children_nodes.push_back(cur_node);
  }
  else{
    mc_printf((stderr, "new remotenode() failed\n"));
    return -1;
  }

  mc_printf((stderr, "Calling recv/proc fromdownstream()\n"));
  while( num_descendants_to_report > num_descendants_reported){
    mc_printf((stderr, "%d of %d Descendants have checked in.\n",
	       num_descendants_to_report, num_descendants_reported));
    if( recv() == -1 ){
      mc_printf((stderr, "recv() failed\n"));
      return -1;
    }
    mc_printf((stderr, "recv() succeeded\n"));
  }

  mc_printf((stderr, "send_newsubtree() completed\n"));
  return 0;
}

int MC_FrontEndNode::send_delSubTree()
{
  unsigned int i;
  int retval;
  MC_Packet *packet;

  mc_printf((stderr, "In frontend.sendDelSubTree()\n"));

  packet = new MC_Packet(MC_DEL_SUBTREE_PROT, "");
  if(packet->good()){
    mc_printf((stderr, "new packet() succeeded\n"));

    std::list <MC_RemoteNode *>::iterator iter;
    for(i=0,iter = children_nodes.begin();
        iter != children_nodes.end(); iter++, i++){
      mc_printf((stderr, "Calling downstream[%d].send() ...\n", i));
      if( (*iter)->send(packet) == -1){
        mc_printf((stderr, "downstream.send() failed\n"));
        retval = -1;
	continue;
      }

      mc_printf((stderr, "downstream.flush() succeeded\n"));
      if( (*iter)->flush() == -1){
        mc_printf((stderr, "downstream.flush() failed\n"));
        retval = -1;
	continue;
      }
      mc_printf((stderr, "downstream.flush() succeeded\n"));
    }
  }
  else{
    mc_printf((stderr, "new packet() failed\n"));
    return -1;
  }

  mc_printf((stderr, "Frontend.sendDelSubTree() succeeded\n"));
  return 0;
}

int MC_FrontEndNode::send_newStream(int stream_id, int filter_id)
{
  unsigned int i;
  int retval;
  vector <MC_EndPoint *> * endpoints = MC_StreamImpl::get_Stream(stream_id)->
                                       get_EndPoints();
  MC_Packet *packet;
  int * backends = (int *) malloc (sizeof(int) * endpoints->size());
  assert(backends);

  for(i=0; i<endpoints->size(); i++){
    mc_printf((stderr, "Adding backend:%d to stream %d", (*endpoints)[i]->get_Id(), stream_id));
    backends[i] = (*endpoints)[i]->get_Id();
  }

  mc_printf((stderr, "In frontend.sendnewStream(%d)\n", stream_id));

  MC_StreamManager * stream_mgr = new MC_StreamManager(stream_id, filter_id,
					       upstream_node, children_nodes);
  StreamManagerById[stream_id] = stream_mgr;
  mc_printf((stderr, "StreamManagerById[%d] = %p\n", stream_id,
	     StreamManagerById[stream_id]));

  packet = new MC_Packet(MC_NEW_STREAM_PROT, "%d %ad %d", stream_id, backends,
                         endpoints->size(), filter_id);
  if(packet->good()){
    mc_printf((stderr, "new packet() succeeded\n"));

    std::list <MC_RemoteNode *>::iterator iter;
    for(i=0,iter = children_nodes.begin();
        iter != children_nodes.end(); iter++, i++){
      mc_printf((stderr, "Calling children_nodes[%d].send() ...\n", i));
      if( (*iter)->send(packet) == -1){
        mc_printf((stderr, "children_nodes.send() failed\n"));
        retval = -1;
	continue;
      }
      mc_printf((stderr, "children_nodes.send() succeeded\n"));
    }
  }
  else{
    mc_printf((stderr, "new packet() failed\n"));
    return -1;
  }

  mc_printf((stderr, "Frontend.sendNewStream() succeeded\n"));
  return 0;
}

int MC_FrontEndNode::send_delStream(int stream_id)
{
  MC_Packet *packet;
  int retval;
  unsigned int i;

  mc_printf((stderr, "In frontend.sendDelStream()\n"));

  MC_StreamManager * stream_mgr = StreamManagerById[stream_id];

  packet = new MC_Packet(MC_DEL_STREAM_PROT, "%d", stream_id);
  if(packet->good()){
    mc_printf((stderr, "new packet() succeeded\n"));

    std::list <MC_RemoteNode *>::iterator iter;
    for(i=0,iter = stream_mgr->downstream_nodes.begin();
        iter != stream_mgr->downstream_nodes.end();
        iter++, i++){
      mc_printf((stderr, "Calling node_set[%d].send() ...\n", i));
      if( (*iter)->send(packet) == -1){
        mc_printf((stderr, "node_set.send() failed\n"));
        retval = -1;
	continue;
      }
      mc_printf((stderr, "node_set.send() succeeded\n"));
    }
  }
  else{
    mc_printf((stderr, "new packet() failed\n"));
    return -1;
  }

  mc_printf((stderr, "Frontend.sendDelStream() succeeded\n"));
  return 0;
}

int MC_FrontEndNode::send_newApplication(string cmd, vector<string> args)
{
  MC_Packet *packet;
  unsigned int i;
  int retval;

  mc_printf((stderr, "In frontend.sendnewApplication()\n"));

  packet = new MC_Packet(MC_NEW_APPLICATION_PROT, "%s", cmd.c_str());
  if(packet->good()){
    mc_printf((stderr, "new packet() succeeded\n"));

    std::list <MC_RemoteNode *>::iterator iter;
    for(i=0,iter = children_nodes.begin();
        iter != children_nodes.end(); iter++, i++){
      mc_printf((stderr, "Calling downstream[%d].send() ...\n", i));
      if( (*iter)->send(packet) == -1){
        mc_printf((stderr, "downstream.send() failed\n"));
        retval = -1;
	continue;
      }
      if( (*iter)->flush() == -1){
        mc_printf((stderr, "downstream.flush() failed\n"));
        retval = -1;
	continue;
      }
      mc_printf((stderr, "downstream.send()/flush() succeeded\n"));
    }
  }
  else{
    mc_printf((stderr, "new packet() failed\n"));
    return -1;
  }

  mc_printf((stderr, "Frontend.sendnewApplication() succeeded\n"));
  return 0;
}

int MC_FrontEndNode::send_delApplication()
{
  MC_Packet *packet;
  unsigned int i;
  int retval;

  mc_printf((stderr, "In frontend.sendDelApplication()\n"));

  packet = new MC_Packet(MC_DEL_APPLICATION_PROT, "");
  if(packet->good()){
    mc_printf((stderr, "new packet() succeeded\n"));

    std::list <MC_RemoteNode *>::iterator iter;
    for(i=0,iter = children_nodes.begin();
        iter != children_nodes.end(); iter++, i++){
      mc_printf((stderr, "Calling downstream[%d].send() ...\n", i));
      if( (*iter)->send(packet) == -1){
        mc_printf((stderr, "downstream.send() failed\n"));
        retval = -1;
	continue;
      }
      mc_printf((stderr, "downstream.send() succeeded\n"));
    }
  }
  else{
    mc_printf((stderr, "new packet() failed\n"));
    return -1;
  }

  mc_printf((stderr, "Frontend.sendDelApplication() succeeded\n"));
  return 0;
}

int MC_FrontEndNode::send_DataDownStream(MC_Packet *)
{
  return 0;
}

int MC_FrontEndNode::proc_newSubTreeReport(MC_Packet *packet)
{
  int status;
  int *backends;
  int i, no_backends;

  mc_printf((stderr, "In frontend.proc_newSubTreeReport()\n"));
  if( packet->ExtractArgList("%d %ad", &status, &backends, &no_backends) == -1){
    mc_printf((stderr, "ExtractArgList failed\n"));
    return -1;
  }
  mc_printf((stderr, "ExtractArgList succeeded\n"));

  num_descendants_reported++;
  mc_printf((stderr, "report status: %d\n", status));
  for(i=0; i<no_backends; i++){
    mc_printf((stderr, "Adding backend %d to my list\n", backends[i]));
    backend_descendant_nodes.push_back(backends[i]);
  }

  mc_printf((stderr, "Leaving frontend.proc_newSubTreeReport()\n"));
  return status;
}

int MC_FrontEndNode::proc_DataFromDownStream(MC_Packet *packet)
{
  MC_StreamImpl * stream;

  mc_printf((stderr, "In proc_DataFromUpStream()\n"));

  stream = MC_StreamImpl::get_Stream(packet->get_StreamId());

  if( stream ){
    mc_printf((stderr, "Inserting packet into stream %d\n", packet->get_StreamId()));
    stream->add_IncomingPacket(packet);
  }
  else{
    mc_printf((stderr, "Packet from unknown stream %d\n", packet->get_StreamId()));
    return -1;
  }
  mc_printf((stderr, "Leaving proc_DataFromUpStream()\n"));
  return 0;
}

/*=====================================================*/
/*  MC_BackEndNode CLASS METHOD DEFINITIONS            */
/*=====================================================*/

MC_BackEndNode::MC_BackEndNode(string _parent_hostname,
                               unsigned short _parent_port)
{
  mc_printf((stderr, "In backendnode()\n"));
  upstream_node = new MC_RemoteNode(_parent_hostname, _parent_port, false);

  mc_printf((stderr, "Calling connect()\n"));
  upstream_node->connect();
  if(upstream_node->fail() ){
    mc_printf((stderr, "connect() failed\n"));
    mc_errno = MC_ECANNOTBINDPORT;
    return;
  }
  mc_printf((stderr, "connect() succeeded\n"));
}

int MC_BackEndNode::send(MC_Packet *packet)
{
  mc_printf((stderr, "In backend.send(). Calling sendupstream()\n"));
  return send_PacketUpStream(packet);
}

int MC_BackEndNode::flush()
{
  mc_printf((stderr, "In backend.flush(). Calling flushupstream()\n"));
  return flush_PacketsUpStream();
}

int MC_BackEndNode::recv()
{
  std::list <MC_Packet *> packet_list;
  mc_printf((stderr, "In backend.recv(). Calling recvfromupstream()\n"));

  if(recv_PacketsFromUpStream(packet_list) == -1){
    mc_printf((stderr, "recv_packetsfromupstream() failed\n"));
    return -1;
  }
  mc_printf((stderr, "recv_packetsfromupstream() succeeded. calling proc()\n"));

  if(packet_list.size() == 0){
    mc_printf((stderr, "No packets read!\n"));
    return 0;
  }

  if(proc_PacketsFromUpStream(packet_list) == -1){
    mc_printf((stderr, "proc_packetsfromupstream() failed\n"));
    return -1;
  }
  mc_printf((stderr, "proc_packetsfromupstream() succeeded\n"));

  return 1;
}

/*
int MC_BackEndNode::send_newSubTree(MC_SerialGraph &)
{
  assert(0);
  return 0;
}

int MC_BackEndNode::send_delSubTree()
{
  assert(0);
  return 0;
}

int MC_BackEndNode::send_newStream(int, int)
{
  assert(0);
  return 0;
}

int MC_BackEndNode::send_delStream(int)
{
  assert(0);
  return 0;
}

int MC_BackEndNode::send_newApplication(string, std::vector<string> args)
{
  assert(0);
  return 0;
}

int MC_BackEndNode::send_delApplication()
{
  assert(0);
  return 0;
}

int MC_BackEndNode::send_DataDownStream(MC_Packet *)
{
  assert(0);
  return 0;
}

int MC_BackEndNode::proc_newSubTreeReport(MC_Packet *)
{
  assert(0);
  return 0;
}

int MC_BackEndNode::proc_DataFromDownStream(MC_Packet *)
{
  assert(0);
  return 0;
}


int MC_BackEndNode::proc_newSubTree(MC_Packet *)
{
  assert(0);
  return 0;
}

int MC_BackEndNode::proc_delSubTree(MC_Packet *)
{
  //for now this means exit!
  mc_printf((stderr, "backend.sendDelSubTree() succeeded. Now Exiting\n"));
  exit(0);
}

int MC_BackEndNode::proc_newStream(MC_Packet *)
{
  assert(0);
  return 0;
}

int MC_BackEndNode::proc_delStream(MC_Packet *)
{
  assert(0);
  return 0;
}

int MC_BackEndNode::proc_newApplication(MC_Packet *)
{
  assert(0);
  return 0;
}

*/

int MC_BackEndNode::proc_delApplication(MC_Packet *packet)
{
  mc_printf((stderr, "In proc_delApplication()\n"));

  mc_printf((stderr, "Calling exit()\n"));
  exit(0);
  return 0;
}

int MC_BackEndNode::proc_DataFromUpStream(MC_Packet *packet)
{
  MC_StreamImpl * stream;

  mc_printf((stderr, "In proc_DataFromUpStream()\n"));

  stream = MC_StreamImpl::get_Stream(packet->get_StreamId());

  if( stream ){
    mc_printf((stderr, "Inserting packet into stream %d\n", packet->get_StreamId()));
    stream->add_IncomingPacket(packet);
  }
  else{
    mc_printf((stderr, "Inserting packet into NEW stream %d\n", packet->get_StreamId()));
    stream = new MC_StreamImpl(packet->get_StreamId());
    stream->add_IncomingPacket(packet);
  }
  mc_printf((stderr, "Leaving proc_DataFromUpStream()\n"));
  return 0;
}

int MC_BackEndNode::send_DataUpStream(MC_Packet *)
{
  return 0;
}

int MC_BackEndNode::send_newSubTreeReport(bool)
{
  assert(0);
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
  if(p1->get_HostName() != p2->get_HostName()){
    return false;
  }
  else{
    return (p1->get_Port() == p2->get_Port());
  }
}

