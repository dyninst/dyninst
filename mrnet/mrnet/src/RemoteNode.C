/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include <stdio.h>


#include "mrnet/src/RemoteNode.h"
#include "mrnet/src/ChildNode.h"
#include "mrnet/src/ParentNode.h"
#include "mrnet/src/utils.h"
#include "src/config.h"
#include "xplat/Process.h"
#include "xplat/Error.h"

namespace MRN
{

/*====================================================*/
/*  RemoteNode CLASS METHOD DEFINITIONS            */
/*====================================================*/

ParentNode * RemoteNode::local_parent_node=NULL;
ChildNode * RemoteNode::local_child_node=NULL;

void * RemoteNode::recv_thread_main(void * args)
{
    std::list <Packet>packet_list;
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
    local_data->thread_id = XPlat::Thread::GetId();
    local_data->thread_name = strdup(name.c_str());
    if( (status = tsd_key.Set( local_data)) != 0){
        mrn_printf(1, MCFL, stderr, "XPlat::TLSKey::Set(): %s\n",
                   strerror(status)); 
        XPlat::Thread::Exit(args);
    }

    mrn_printf(3, MCFL, stderr, "In recv_thread_main()\n");
    while(1){
        // TODO this only works if the recv on remote_node is blocking recv...
        // is it?
        int rret = remote_node->recv( packet_list );
        if( (rret == -1) || ((rret == 0) && (packet_list.size() == 0)) ) {
            if( rret == -1 ) {
                mrn_printf(1, MCFL, stderr, 
                           "RemoteNode recv failed - recv thread exiting\n");
            }
            XPlat::Thread::Exit(args);
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
    local_data->thread_id = XPlat::Thread::GetId();
    local_data->thread_name = strdup(name.c_str());
    if( (status = tsd_key.Set( local_data)) != 0){
        mrn_printf(1, 0, 0, stderr, "XPlat::TLSKey::Set(): %s\n",
                   strerror(status)); 
        XPlat::Thread::Exit(args);
    }

    while(1){
        remote_node->msg_out_sync.Lock();

        while(remote_node->msg_out.size_Packets() == 0){
            mrn_printf(3, MCFL, stderr, "send_thread_main() waiting on nonempty ..\n");
            remote_node->msg_out_sync.WaitOnCondition(MRN_MESSAGEOUT_NONEMPTY);
        }

        mrn_printf(3, MCFL, stderr, "send_thread_main() sending packets ...\n");
        if( remote_node->msg_out.send(remote_node->sock_fd) == -1 ){
            mrn_printf(1, MCFL, stderr, "RN: send_thread_main: send failed\n" );
            mrn_printf(1, MCFL, stderr, "msg.send() failed. Thread Exiting\n");
            remote_node->msg_out_sync.Unlock();
            XPlat::Thread::Exit(args);
        }

        remote_node->msg_out_sync.Unlock();
    }

    return NULL;
}

RemoteNode::RemoteNode(bool _threaded, std::string &_hostname,
                       unsigned short _port)
    :CommunicationNode(_hostname, _port), threaded(_threaded), sock_fd(0),
     _is_internal_node(false), _is_upstream(false)
{
    msg_out_sync.RegisterCondition(MRN_MESSAGEOUT_NONEMPTY);
}

RemoteNode::RemoteNode(bool _threaded, std::string &_hostname,
                       unsigned short _port, unsigned short _id)
    :CommunicationNode(_hostname, _port, _id), threaded(_threaded), sock_fd(0),
     _is_internal_node(false), _is_upstream(false)
{
    msg_out_sync.RegisterCondition(MRN_MESSAGEOUT_NONEMPTY);
}

int RemoteNode::connect()
{
    mrn_printf(3, MCFL, stderr, "In connect(%s:%d) ...\n",
               hostname.c_str(), port);
    if(connectHost(&sock_fd, hostname.c_str(), port) == -1){
        mrn_printf(1, MCFL, stderr, "connect_to_host() failed\n");
        error( ESYSTEM, "connect(): %s\n", strerror(errno) );
        return -1;
    }
    
    mrn_printf(3, MCFL, stderr,
               "connect_to_host() succeeded. new socket = %d\n", sock_fd);
    return 0;
}

int RemoteNode::accept_Connection( int lsock_fd, bool do_connect )
{
    int retval = 0;

    if( do_connect ) {
        if( (sock_fd = getSocketConnection(lsock_fd)) == -1){
            mrn_printf(1, MCFL, stderr, "get_socket_connection() failed\n");
            error( ESYSTEM, "getSocketConnection(): %s\n",
                   strerror(errno) );
            return -1;
        }
    }
    else {
        // socket is already connected
        sock_fd = lsock_fd;
    }
    assert( sock_fd != -1 );

    if(threaded){
        mrn_printf(3, MCFL, stderr, "Creating Downstream recv thread ...\n");
        retval = XPlat::Thread::Create( RemoteNode::recv_thread_main,
                                        (void *) this,
                                        &recv_thread_id );
        if(retval != 0){
            error( ESYSTEM, "XPlat::Thread::Create() failed: %s\n",
                    strerror(errno) );
            mrn_printf(1, MCFL, stderr, "Downstream recv thread creation failed...\n");
        }
        mrn_printf(3, MCFL, stderr, "Creating Downstream send thread ...\n");
        retval = XPlat::Thread::Create( RemoteNode::send_thread_main,
                                        (void *) this,
                                        &send_thread_id );
        if(retval != 0){
            error( ESYSTEM, "XPlat::Thread::Create() failed: %s\n",
                    strerror(errno) );
            mrn_printf(1, MCFL, stderr, "Downstream send thread creation failed...\n");
        }
    }

    return retval;
}


int RemoteNode::new_InternalNode(int listening_sock_fd,
                                 std::string parent_host,
                                 unsigned short parent_port,
                                 unsigned short parent_id,
                                 std::string commnode_cmd)
{
    char parent_port_str[128];
    sprintf(parent_port_str, "%d", parent_port);
    char parent_id_str[128];
    sprintf(parent_id_str, "%d", parent_id);
    char port_str[128];
    sprintf(port_str, "%d", port );

    mrn_printf(3, MCFL, stderr, "In new_InternalNode(%s:%d) ...\n",
               hostname.c_str(), port );

    _is_internal_node = true;

    // set up arguments for the new process
    std::vector <std::string> args;
    args.push_back(commnode_cmd);
    args.push_back(port_str);
    args.push_back(parent_host);
    args.push_back(std::string(parent_port_str));
    args.push_back(std::string(parent_id_str));

    if( XPlat::Process::Create( hostname, commnode_cmd, args ) != 0 ){
        int err = XPlat::Process::GetLastError();

        error( ESYSTEM, "XPlat::Process::Create(%s %s): %s\n",
               hostname.c_str(), commnode_cmd.c_str(),
               XPlat::Error::GetErrorString( err ).c_str() );
        return -1;
    }

    if( accept_Connection( listening_sock_fd ) == -1 ){
        error( ESYSTEM, "accept_Connection(): %s\n", strerror(errno) );
        return -1;
    }
    return 0;
}


int RemoteNode::new_Application(int listening_sock_fd,
                                std::string parent_host,
                                unsigned short parent_port,
                                unsigned short parent_id, std::string &cmd,
                                std::vector <std::string> &args){

    mrn_printf(3, MCFL, stderr, "In new_Application(%s:%d,\n"
               "                   cmd: %s)\n",
               hostname.c_str(), port, cmd.c_str());
  
    char port_str[128];
    sprintf(port_str, "%d", port );
    char parent_port_str[128];
    sprintf(parent_port_str, "%d", parent_port);
    char parent_id_str[128];
    sprintf(parent_id_str, "%d", parent_id);

    // set up arguments for new process
    args.push_back(cmd);
    args.push_back(std::string(port_str));
    args.push_back(parent_host);
    args.push_back(std::string(parent_port_str));
    args.push_back(std::string(parent_id_str));

    if( XPlat::Process::Create( hostname, cmd, args ) != 0 ){
        mrn_printf(1, MCFL, stderr, "XPlat::Process::Create() failed\n"); 
        int err = XPlat::Process::GetLastError();
        error( ESYSTEM, "XPlat::Process::Create(%s %s): %s\n",
               hostname.c_str(), cmd.c_str(),
               XPlat::Error::GetErrorString( err ).c_str() );
        return -1;
    }

    // accept the connection
    sock_fd = getSocketConnection( listening_sock_fd );
    if( sock_fd == -1 ) {
        mrn_printf(1, MCFL, stderr, "get_socket_connection() failed\n" );
        error( ESYSTEM, "accept_Connection(): %s\n", strerror(errno) );
        return -1;
    }

    // consume the backend id sent from the backend
    uint32_t idBuf = 0;
    int rret = ::recv( sock_fd, (char*)&idBuf, 4, 0 );
    if( rret != 4 ) {
        mrn_printf(1, MCFL, stderr, "failed to receive id from backend\n" );
        error( ESYSTEM, "recv(): %s\n", strerror(errno) );
        return -1;
    }
    uint32_t backend_port = ntohl(idBuf);
    assert( backend_port == port );

    if( accept_Connection( sock_fd, false ) == -1 ){
        error( ESYSTEM, "accept_Connection(): %s\n", strerror(errno) );
        return -1;
    }
    return 0;
}

int RemoteNode::send(Packet& packet)
{
    mrn_printf(3, MCFL, stderr, "In remotenode.send(). Calling msg.add_packet()\n");

    if(threaded){
        msg_out_sync.Lock();
    }

    msg_out.add_Packet(packet);

    //TODO: implement flush based on timeout/threshold
    // for right now, always flush
    if( msg_out.size_Packets() > 1 ) {
        int sret = msg_out.send( sock_fd );
        if( sret == -1 ) {
            mrn_printf(1, MCFL, stderr, "Message.send failed\n" );
        }
    }

    if(threaded){
        msg_out_sync.SignalCondition(MRN_MESSAGEOUT_NONEMPTY);
        msg_out_sync.Unlock();
    }

    mrn_printf(3, MCFL, stderr, "Leaving remotenode.send()\n");
    return 0;
}

bool RemoteNode::has_data() const
{
    struct timeval zeroTimeout;
    zeroTimeout.tv_sec = 0;
    zeroTimeout.tv_usec = 0;

    // set up file descriptor set for the poll
    fd_set rfds;
    FD_ZERO( &rfds );
    FD_SET( sock_fd, &rfds );

    // check if data is available
    int sret = select( sock_fd + 1, &rfds, NULL, NULL, &zeroTimeout );
    if( sret == -1 ){
        mrn_printf(1, MCFL, stderr, "select() failed\n");
        return false;
    }

    // We only put one descriptor in the read set.  Therefore, if the return 
    // value from select() is 1, that descriptor has data available.
    if( sret == 1 ){
        mrn_printf(1, MCFL, stderr, "select() says data to be read.\n");
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

int RemoteNode::poll_timeout=-1;

} // namespace MRN
