#include <stdio.h>

#include "mrnet/src/MC_RemoteNode.h"
#include "mrnet/src/utils.h"
#include "mrnet/src/config.h"

/*====================================================*/
/*  MC_RemoteNode CLASS METHOD DEFINITIONS            */
/*====================================================*/

MC_ParentNode * MC_RemoteNode::local_parent_node=NULL;
MC_ChildNode * MC_RemoteNode::local_child_node=NULL;

void * MC_RemoteNode::recv_thread_main(void * args)
{
  std::list <MC_Packet *>packet_list;
  MC_RemoteNode * remote_node = (MC_RemoteNode *)args;

  //TLS: setup thread local storage for recv thread
  // I am localhost:localport_UPRECVFROM_remotehost:remoteport
  std::string name;
  char local_port_str[128];
  char remote_port_str[128];
  if( remote_node->is_upstream() ){
    sprintf(local_port_str, "%d",
            local_child_node->get_Port());
    sprintf(remote_port_str, "%d",
            remote_node->MC_CommunicationNode::config_port);

    name = local_child_node->get_HostName();
    name += ":";
    name += local_port_str;
    name += "_UPRECVFROM_";
    name += remote_node->get_HostName();
    name += ":";
    name += remote_port_str;
  }
  else{
    sprintf(local_port_str, "%d", local_parent_node->get_Port());
    sprintf(remote_port_str, "%d",
            remote_node->MC_CommunicationNode::config_port);

    name = local_parent_node->get_HostName();
    name += ":";
    name += local_port_str;
    name += "_DOWNRECVFROM_";
    name += remote_node->get_HostName();
    name += ":";
    name += remote_port_str;
  }

  int status;
  if( (status = pthread_key_create(&tsd_key, NULL)) != 0){
    fprintf(stderr, "pthread_key_create(): %s\n", strerror(status)); 
    exit(-1);
  }
  tsd_t * local_data = new tsd_t;
  local_data->thread_id = pthread_self();
  local_data->thread_name = strdup(name.c_str());
  if( (status = pthread_setspecific(tsd_key, local_data)) != 0){
    fprintf(stderr, "pthread_key_create(): %s\n", strerror(status)); 
    exit(-1);
  }

  mc_printf(MCFL, stderr, "In recv_thread_main()\n");
  while(1){
    if( remote_node->recv(packet_list) == -1 ){
      mc_printf(MCFL, stderr, "remote_node->recv() failed. Thread Exiting\n");
      pthread_exit(args);
    }

    if( remote_node->is_upstream() ){
      if(local_child_node->proc_PacketsFromUpStream(packet_list) == -1){
        mc_printf(MCFL, stderr, "proc_PacketsFromUpstream() failed\n");
      }
    }
    else{
      if(local_parent_node->proc_PacketsFromDownStream(packet_list) == -1){
        mc_printf(MCFL, stderr, "proc_PacketsFromDownstream() failed\n");
      }
    }
  }

  return NULL;
}

void * MC_RemoteNode::send_thread_main(void * args)
{
  MC_RemoteNode * remote_node = (MC_RemoteNode *)args;
  mc_printf(MCFL, stderr, "In send_thread_main()\n");

  //TLS: setup thread local storage for recv thread
  // I am localhost:localport_UPRECVFROM_remotehost:remoteport
  std::string name;
  char local_port_str[128];
  char remote_port_str[128];
  if( remote_node->is_upstream() ){
    sprintf(local_port_str, "%d",
            local_child_node->get_Port());
    sprintf(remote_port_str, "%d",
            remote_node->MC_CommunicationNode::config_port);

    name = local_child_node->get_HostName();
    name += ":";
    name += local_port_str;
    name += "_UPSENDTO_";
    name += remote_node->get_HostName();
    name += ":";
    name += remote_port_str;
  }
  else{
    sprintf(local_port_str, "%d", local_parent_node->get_Port());
    sprintf(remote_port_str, "%d",
            remote_node->MC_CommunicationNode::config_port);

    name = local_parent_node->get_HostName();
    name += ":";
    name += local_port_str;
    name += "_DOWNSENDTO_";
    name += remote_node->get_HostName();
    name += ":";
    name += remote_port_str;
  }

  int status;
  if( (status = pthread_key_create(&tsd_key, NULL)) != 0){
    fprintf(stderr, "pthread_key_create(): %s\n", strerror(status)); 
    exit(-1);
  }
  tsd_t * local_data = new tsd_t;
  local_data->thread_id = pthread_self();
  local_data->thread_name = strdup(name.c_str());
  while(1){
    remote_node->msg_out_sync.lock();

    while(remote_node->msg_out.size_Packets() == 0){
      mc_printf(MCFL, stderr, "send_thread_main() waiting on nonempty ..\n");
      remote_node->msg_out_sync.wait(MC_MESSAGEOUT_NONEMPTY);
    }

    mc_printf(MCFL, stderr, "send_thread_main() sending packets ...\n");
    if( remote_node->msg_out.send(remote_node->sock_fd) == -1 ){
      mc_printf(MCFL, stderr, "msg.send() failed. Thread Exiting\n");
      remote_node->msg_out_sync.unlock();
      pthread_exit(args);
    }

    remote_node->msg_out_sync.unlock();
  }

  return NULL;
}

MC_RemoteNode::MC_RemoteNode(bool _threaded, std::string &_hostname,
                             unsigned short _port)
  :MC_CommunicationNode(_hostname, _port), threaded(_threaded), sock_fd(0),
   _is_internal_node(false), _is_upstream(false)
{
  msg_out_sync.register_cond(MC_MESSAGEOUT_NONEMPTY);
}

MC_RemoteNode::MC_RemoteNode(bool _threaded, std::string &_hostname,
                             unsigned short _port, unsigned short _id)
  :MC_CommunicationNode(_hostname, _port, _id), threaded(_threaded), sock_fd(0),
   _is_internal_node(false), _is_upstream(false)
{
  msg_out_sync.register_cond(MC_MESSAGEOUT_NONEMPTY);
}

int MC_RemoteNode::connect()
{
  mc_printf(MCFL, stderr, "In connect(%s:%d) ...\n", hostname.c_str(), port);
  if(connect_to_host(&sock_fd, hostname.c_str(), port) == -1){
    mc_printf(MCFL, stderr, "connect_to_host() failed\n");
    mc_errno = MC_ECREATPROCFAILURE;
    return -1;
  }

  poll_struct.fd = sock_fd;
  poll_struct.events = POLLIN;

  mc_printf(MCFL, stderr, "connect_to_host() succeeded. new socket = %d\n", sock_fd);
  return 0;
}

int MC_RemoteNode::new_InternalNode(int listening_sock_fd, std::string parent_host,
                                    unsigned short parent_port,
                                    unsigned short parent_id,
                                    std::string commnode_cmd)
{
  int retval;
  char parent_port_str[128];
  char parent_id_str[128];
  std::string rsh("");
  std::string username("");
  std::vector <std::string> args;

  mc_printf(MCFL, stderr, "In new_InternalNode(%s:%d) ...\n",
             hostname.c_str(), port);

  _is_internal_node = true;

  args.push_back(parent_host);
  sprintf(parent_port_str, "%d", parent_port);
  args.push_back(std::string(parent_port_str));
  sprintf(parent_id_str, "%d", parent_id);
  args.push_back(std::string(parent_id_str));

  if(create_Process(rsh, hostname, username, commnode_cmd, args) == -1){
    mc_printf(MCFL, stderr, "createProcess() failed\n"); 
    mc_errno = MC_ECREATPROCFAILURE;
    return -1;
  }

  if( (sock_fd = get_socket_connection(listening_sock_fd)) == -1){
    mc_printf(MCFL, stderr, "get_socket_connection() failed\n");
    mc_errno = MC_ESOCKETCONNECT;
    return -1;
  }

  if(threaded){
    mc_printf(MCFL, stderr, "Creating Downstream recv thread ...\n");
    retval = pthread_create(&recv_thread_id, NULL,
                            MC_RemoteNode::recv_thread_main, (void *) this);
    if(retval != 0){
      mc_printf(MCFL, stderr, "Downstream recv thread creation failed...\n");
      //thread create error
    }
    mc_printf(MCFL, stderr, "Creating Downstream send thread ...\n");
    retval = pthread_create(&send_thread_id, NULL,
                            MC_RemoteNode::send_thread_main, (void *) this);
    if(retval != 0){
      mc_printf(MCFL, stderr, "Downstream send thread creation failed...\n");
      //thread create error
    }
  }
  else{
    poll_struct.fd = sock_fd;
    poll_struct.events = POLLIN;
  }

  return 0;
}

int MC_RemoteNode::new_Application(int listening_sock_fd, std::string parent_host,
                                   unsigned short parent_port,
                                   unsigned short parent_id, std::string &cmd,
                                   std::vector <std::string> &args){
  int retval;
  std::string rsh("");
  std::string username("");

  mc_printf(MCFL, stderr, "In new_Application(%s:%d,\n"
                     "                   cmd: %s)\n",
                     hostname.c_str(), config_port, cmd.c_str());
  
  char port_str[128];
  sprintf(port_str, "%d", config_port);
  char parent_port_str[128];
  sprintf(parent_port_str, "%d", parent_port);
  char parent_id_str[128];
  sprintf(parent_id_str, "%d", parent_id);

  //append args: hostname, port, parent_hostname, parent_port, parent_id
  args.push_back(hostname);
  args.push_back(std::string(port_str));
  args.push_back(parent_host);
  args.push_back(std::string(parent_port_str));
  args.push_back(std::string(parent_id_str));

  if(create_Process(rsh, hostname, username, cmd, args) == -1){
    mc_printf(MCFL, stderr, "createProcess() failed\n"); 
    mc_errno = MC_ECREATPROCFAILURE;
    return -1;
  }
  if( (sock_fd = get_socket_connection(listening_sock_fd)) == -1){
    mc_printf(MCFL, stderr, "get_socket_connection() failed\n");
    mc_errno = MC_ESOCKETCONNECT;
    return -1;
  }

  if(threaded){
    mc_printf(MCFL, stderr, "Creating Downstream recv thread ...\n");
    retval = pthread_create(&recv_thread_id, NULL,
                            MC_RemoteNode::recv_thread_main, (void *) this);
    if(retval != 0){
      mc_printf(MCFL, stderr, "Downstream recv thread creation failed...\n");
      //thread create error
    }

    mc_printf(MCFL, stderr, "Creating Downstream send thread ...\n");
    retval = pthread_create(&send_thread_id, NULL,
                            MC_RemoteNode::send_thread_main, (void *) this);
    if(retval != 0){
      mc_printf(MCFL, stderr, "Downstream send thread creation failed...\n");
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
  mc_printf(MCFL, stderr, "In remotenode.send(). Calling msg.add_packet()\n");

  if(threaded){
    msg_out_sync.lock();
  }

  msg_out.add_Packet(packet);

  if(threaded){
    msg_out_sync.signal(MC_MESSAGEOUT_NONEMPTY);
    msg_out_sync.unlock();
  }

  mc_printf(MCFL, stderr, "Leaving remotenode.send()\n");
  return 0;
}

int MC_RemoteNode::recv(std::list <MC_Packet *> &packet_list)
{
  return msg_in.recv(sock_fd, packet_list, this);
}

bool MC_RemoteNode::has_data()
{
  poll_struct.revents = 0;

  mc_printf(MCFL, stderr, "In remotenode.has_data(%d)\n", poll_struct.fd);

  if(poll(&poll_struct, 1, 0) == -1){
    mc_printf(MCFL, stderr, "poll() failed\n");
    return false;
  }

  if(poll_struct.revents & POLLNVAL){
    mc_printf(MCFL, stderr, "poll() says invalid request occured\n");
    perror("poll()");
    return false;
  }
  if(poll_struct.revents & POLLERR){
    mc_printf(MCFL, stderr, "poll() says error occured:");
    perror("poll()");
    return false;
  }
  if(poll_struct.revents & POLLHUP){
    mc_printf(MCFL, stderr, "poll() says hangup occured:");
    perror("poll()");
    return false;
  }
  if(poll_struct.revents & POLLIN){
    mc_printf(MCFL, stderr, "poll() says data to be read.\n");
    return true;
  }

  mc_printf(MCFL, stderr, "Leaving remotenode.has_data(). No data available\n");
  return false;
}

int MC_RemoteNode::flush()
{
  if(threaded){
    return 0;
  }

  mc_printf(MCFL, stderr, "In remotenode.flush(). Calling msg.send()\n");
  if( msg_out.send(sock_fd) == -1){
    mc_printf(MCFL, stderr, "msg.send() failed\n");
    return -1;
  }

  mc_printf(MCFL, stderr, "Leaving remotenode.flush().\n");
  return 0;
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

