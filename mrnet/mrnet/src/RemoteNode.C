#include <stdio.h>
#include <arpa/inet.h>

#include "mrnet/src/RemoteNode.h"
#include "mrnet/src/utils.h"
#include "src/config.h"

namespace MRN
{

/*====================================================*/
/*  RemoteNode CLASS METHOD DEFINITIONS            */
/*====================================================*/

ParentNode * RemoteNode::local_parent_node=NULL;
ChildNode * RemoteNode::local_child_node=NULL;

void * RemoteNode::recv_thread_main(void * args)
{
  std::list <Packet *>packet_list;
  RemoteNode * remote_node = (RemoteNode *)args;
  std::string local_hostname, remote_hostname;

  //TLS: setup thread local storage for recv thread
  // I am localhost:localport_UPRECVFROM_remotehost:remoteport
  std::string name;
  char local_port_str[128];
  char remote_port_str[128];
  if( remote_node->is_upstream() ){
    sprintf(local_port_str, "%d",
            local_child_node->get_Port());
    sprintf(remote_port_str, "%d",
            remote_node->CommunicationNode::id);
    getHostName(local_hostname, local_child_node->get_HostName() );
    getHostName(remote_hostname, remote_node->get_HostName() );
    name = "UPRECV(";
    name += local_hostname;
    name += ":";
    name += local_port_str;
    name += "<==";
    name += remote_hostname;
    name += ":";
    name += remote_port_str;
    name += ")";
  }
  else{
      sprintf(local_port_str, "%d", local_parent_node->config_port);
      sprintf(remote_port_str, "%d",
              remote_node->CommunicationNode::id);

      getHostName(local_hostname, local_parent_node->get_HostName() );
      getHostName(remote_hostname, remote_node->get_HostName() );
      name = "DOWNRECV(";
      name += local_hostname;
      name += ":";
      name += local_port_str;
      name += "<==";
      name += remote_hostname;
      name += ":";
      name += remote_port_str;
      name += ")";
  }

  int status;
  tsd_t * local_data = new tsd_t;
  local_data->thread_id = pthread_self();
  local_data->thread_name = strdup(name.c_str());
  if( (status = pthread_setspecific(tsd_key, local_data)) != 0){
    mrn_printf(1, MCFL, stderr, "pthread_key_create(): %s\n", strerror(status)); 
    exit(-1);
  }

  mrn_printf(3, MCFL, stderr, "In recv_thread_main()\n");
  while(1){

    // TODO this only works if the recv on remote_node is a blocking recv...
    // is it?
    int rret = remote_node->recv( packet_list );
    if( (rret == -1) || ((rret == 0) && (packet_list.size() == 0)) )
    {
        if( rret == -1 )
        {
            mrn_printf(1, MCFL, stderr, 
                "RemoteNode recv failed - recv thread exiting\n");

        }
        pthread_exit(args);
    }

    if( remote_node->is_upstream() ){
      if(local_child_node->proc_PacketsFromUpStream(packet_list) == -1){
        mrn_printf(1, MCFL, stderr, "proc_PacketsFromUpstream() failed\n");
      }
    }
    else{
      if(local_parent_node->proc_PacketsFromDownStream(packet_list) == -1){
        mrn_printf(1, MCFL, stderr, "proc_PacketsFromDownstream() failed\n");
      }
    }
  }

  return NULL;
}

void * RemoteNode::send_thread_main(void * args)
{
  RemoteNode * remote_node = (RemoteNode *)args;

  //TLS: setup thread local storage for recv thread
  // I am localhost:localport_UPRECVFROM_remotehost:remoteport
  std::string name, local_hostname, remote_hostname;
  char local_port_str[128];
  char remote_port_str[128];

  if( remote_node->is_upstream() ){
    sprintf(local_port_str, "%d",
            local_child_node->get_Port());
    sprintf(remote_port_str, "%d",
            remote_node->CommunicationNode::id);
    getHostName(local_hostname, local_child_node->get_HostName() );
    getHostName(remote_hostname, remote_node->get_HostName() );

    name = "UPSEND(";
    name += local_hostname;
    name += ":";
    name += local_port_str;
    name += "==>";
    name += remote_hostname;
    name += ":";
    name += remote_port_str;
    name += ")";
  }
  else{
    sprintf(local_port_str, "%d", local_parent_node->config_port);
    sprintf(remote_port_str, "%d",
            remote_node->CommunicationNode::id);
    getHostName(local_hostname, local_child_node->get_HostName() );
    getHostName(remote_hostname, remote_node->get_HostName() );

    name = "DOWNSEND";
    name += local_hostname;
    name += ":";
    name += local_port_str;
    name += "==>";
    name += remote_hostname;
    name += ":";
    name += remote_port_str;
    name += ")";
  }

  int status;
  tsd_t * local_data = new tsd_t;
  local_data->thread_id = pthread_self();
  local_data->thread_name = strdup(name.c_str());
  if( (status = pthread_setspecific(tsd_key, local_data)) != 0){
    mrn_printf(1, 0, 0, stderr, "pthread_key_create(): %s\n", strerror(status)); 
    exit(-1);
  }

  while(1){
    remote_node->msg_out_sync.lock();

    while(remote_node->msg_out.size_Packets() == 0){
      mrn_printf(3, MCFL, stderr, "send_thread_main() waiting on nonempty ..\n");
      remote_node->msg_out_sync.wait(MRN_MESSAGEOUT_NONEMPTY);
    }

    mrn_printf(3, MCFL, stderr, "send_thread_main() sending packets ...\n");
    if( remote_node->msg_out.send(remote_node->sock_fd) == -1 ){
        mrn_printf(1, MCFL, stderr, "RN: send_thread_main: send failed\n" );
	mrn_printf(1, MCFL, stderr, "msg.send() failed. Thread Exiting\n");
	remote_node->msg_out_sync.unlock();
	pthread_exit(args);
    }

    remote_node->msg_out_sync.unlock();
  }

  return NULL;
}

RemoteNode::RemoteNode(bool _threaded, std::string &_hostname,
                             unsigned short _port)
  :CommunicationNode(_hostname, _port), threaded(_threaded), sock_fd(0),
   _is_internal_node(false), _is_upstream(false)
{
  msg_out_sync.register_cond(MRN_MESSAGEOUT_NONEMPTY);
}

RemoteNode::RemoteNode(bool _threaded, std::string &_hostname,
                             unsigned short _port, unsigned short _id)
  :CommunicationNode(_hostname, _port, _id), threaded(_threaded), sock_fd(0),
   _is_internal_node(false), _is_upstream(false)
{
  msg_out_sync.register_cond(MRN_MESSAGEOUT_NONEMPTY);
}

int RemoteNode::connect()
{
    mrn_printf(3, MCFL, stderr, "In connect(%s:%d) ...\n",
               hostname.c_str(), port);
    if(connectHost(&sock_fd, hostname.c_str(), port) == -1){
        mrn_printf(1, MCFL, stderr, "connect_to_host() failed\n");
        MRN_errno = MRN_ECREATPROCFAILURE;
        return -1;
    }
    
    poll_struct.fd = sock_fd;
    poll_struct.events = POLLIN;
    
    mrn_printf(3, MCFL, stderr,
               "connect_to_host() succeeded. new socket = %d\n", sock_fd);
    return 0;
}



int
RemoteNode::accept_Connection( int lsock_fd, bool do_connect )
{
  int retval = 0;


  if( do_connect )
  {
    if( (sock_fd = getSocketConnection(lsock_fd)) == -1){
        mrn_printf(1, MCFL, stderr, "get_socket_connection() failed\n");
        MRN_errno = MRN_ESOCKETCONNECT;
        return -1;
    }
  }
  else
  {
    // socket is already connected
    sock_fd = lsock_fd;
  }
  assert( sock_fd != -1 );

  if(threaded){
    mrn_printf(3, MCFL, stderr, "Creating Downstream recv thread ...\n");
    retval = pthread_create(&recv_thread_id, NULL,
                            RemoteNode::recv_thread_main, (void *) this);
    if(retval != 0){
      mrn_printf(1, MCFL, stderr, "Downstream recv thread creation failed...\n");
      //thread create error
    }
    mrn_printf(3, MCFL, stderr, "Creating Downstream send thread ...\n");
    retval = pthread_create(&send_thread_id, NULL,
                            RemoteNode::send_thread_main, (void *) this);
    if(retval != 0){
      mrn_printf(1, MCFL, stderr, "Downstream send thread creation failed...\n");
      //thread create error
    }
  }
  else{
    poll_struct.fd = sock_fd;
    poll_struct.events = POLLIN;
  }

  return retval;
}


int RemoteNode::new_InternalNode(int listening_sock_fd, std::string parent_host,
                                    unsigned short parent_port,
                                    unsigned short parent_id,
                                    std::string commnode_cmd)
{
  char parent_port_str[128];
  char parent_id_str[128];
  char port_str[128];
  std::string rsh("");
  std::string username("");
  std::vector <std::string> args;

  mrn_printf(3, MCFL, stderr, "In new_InternalNode(%s:%d) ...\n",
             hostname.c_str(), get_Id());

  _is_internal_node = true;

  args.push_back(hostname);
  sprintf(port_str, "%d", get_Id());
  args.push_back(port_str);
  args.push_back(parent_host);
  sprintf(parent_port_str, "%d", parent_port);
  args.push_back(std::string(parent_port_str));
  sprintf(parent_id_str, "%d", parent_id);
  args.push_back(std::string(parent_id_str));

  if(createProcess(rsh, hostname, username, commnode_cmd, args) == -1){
    mrn_printf(1, MCFL, stderr, "createProcess() failed\n"); 
    MRN_errno = MRN_ECREATPROCFAILURE;
    return -1;
  }

  return accept_Connection( listening_sock_fd );
}


int RemoteNode::new_Application(int listening_sock_fd,
                                   unsigned int backend_id,
                                   std::string parent_host,
                                   unsigned short parent_port,
                                   unsigned short parent_id, std::string &cmd,
                                   std::vector <std::string> &args){
  std::string rsh("");
  std::string username("");

  mrn_printf(3, MCFL, stderr, "In new_Application(%s:%d,\n"
                     "                   cmd: %s)\n",
                     hostname.c_str(), get_Id(), cmd.c_str());
  
  char port_str[128];
  sprintf(port_str, "%d", get_Id());
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

  if(createProcess(rsh, hostname, username, cmd, args) == -1){
    mrn_printf(1, MCFL, stderr, "createProcess() failed\n"); 
    MRN_errno = MRN_ECREATPROCFAILURE;
    return -1;
  }

  char backend_id_str[32];
  sprintf(backend_id_str, "%u", backend_id);

  // accept the connection
  sock_fd = getSocketConnection( listening_sock_fd );
  if( sock_fd == -1 )
  {
    mrn_printf(1, MCFL, stderr, "get_socket_connection() failed\n" );
    MRN_errno = MRN_ESOCKETCONNECT;
    return -1;
  }
  mrn_printf(3, MCFL, stderr, "get_socket_connection() returned %d\n", sock_fd );

  // consume the backend id sent from the backend
  uint32_t idBuf = 0;
  int rret = ::recv( sock_fd, &idBuf, 4, 0 );
  if( rret != 4 )
  {
    mrn_printf(1, MCFL, stderr, "failed to receive id from backend\n" );
    MRN_errno = MRN_ESOCKETCONNECT;
    return -1;
  }
  uint32_t backendId = ntohl(idBuf);
  assert( backendId == get_Id() );

  return accept_Connection( sock_fd, false );
}


int
RemoteNode::accept_Application( int connected_sock_fd )
{
    return accept_Connection( connected_sock_fd, false );    
}


int RemoteNode::send(Packet *packet)
{
  mrn_printf(3, MCFL, stderr, "In remotenode.send(). Calling msg.add_packet()\n");

  if(threaded){
    msg_out_sync.lock();
  }

  msg_out.add_Packet(packet);
  if( msg_out.size_Packets() == IOV_MAX )
  {
    int sret = msg_out.send( sock_fd );
    if( sret == -1 )
    {
        mrn_printf(1, MCFL, stderr, "RemoteNode.send, flush forced, but failed\n" );
    }
  }

  if(threaded){
    msg_out_sync.signal(MRN_MESSAGEOUT_NONEMPTY);
    msg_out_sync.unlock();
  }

  mrn_printf(3, MCFL, stderr, "Leaving remotenode.send()\n");
  return 0;
}

int RemoteNode::recv(std::list <Packet *> &packet_list)
{
  return msg_in.recv(sock_fd, packet_list, this);
}

bool RemoteNode::has_data()
{
  poll_struct.revents = 0;

  mrn_printf(3, MCFL, stderr, "In remotenode.has_data(%d)\n", poll_struct.fd);

  if(poll(&poll_struct, 1, 0) == -1){
    mrn_printf(1, MCFL, stderr, "poll() failed\n");
    return false;
  }

  if(poll_struct.revents & POLLNVAL){
    mrn_printf(1, MCFL, stderr, "poll() says invalid request occured\n");
    perror("poll()");
    return false;
  }
  if(poll_struct.revents & POLLERR){
    mrn_printf(1, MCFL, stderr, "poll() says error occured:");
    perror("poll()");
    return false;
  }
  if(poll_struct.revents & POLLHUP){
    mrn_printf(1, MCFL, stderr, "poll() says hangup occured:");
    perror("poll()");
    return false;
  }
  if(poll_struct.revents & POLLIN){
    mrn_printf(1, MCFL, stderr, "poll() says data to be read.\n");
    return true;
  }

  mrn_printf(3, MCFL, stderr, "Leaving remotenode.has_data(). No data available\n");
  return false;
}

int RemoteNode::flush()
{
  if(threaded){
    return 0;
  }

  mrn_printf(3, MCFL, stderr, "In remotenode.flush(). Calling msg.send()\n");
  if( msg_out.send(sock_fd) == -1){
    mrn_printf(1, MCFL, stderr, "msg.send() failed\n");
    return -1;
  }

  mrn_printf(3, MCFL, stderr, "Leaving remotenode.flush().\n");
  return 0;
}

bool RemoteNode::is_backend(){
  return !_is_internal_node;
}

bool RemoteNode::is_internal(){
  return _is_internal_node;
}

bool RemoteNode::is_upstream(){
  return _is_upstream;
}

} // namespace MRN
