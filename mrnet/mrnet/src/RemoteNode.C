/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <stdio.h>


#include "mrnet/src/RemoteNode.h"
#include "mrnet/src/ChildNode.h"
#include "mrnet/src/ParentNode.h"
#include "mrnet/src/utils.h"
#include "src/config.h"
#include "xplat/Process.h"
#include "xplat/SocketUtils.h"
#include "xplat/Error.h"

namespace MRN
{

/*====================================================*/
/*  RemoteNode CLASS METHOD DEFINITIONS            */
/*====================================================*/

ParentNode * RemoteNode::local_parent_node=NULL;
ChildNode * RemoteNode::local_child_node=NULL;
XPlat::Mutex RemoteNode::poll_timeout_mutex;
int RemoteNode::poll_timeout=0;

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
        sprintf(local_port_str, "%d", local_child_node->get_Port());
        sprintf(remote_port_str, "%d", remote_node->get_Port());
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
        sprintf(remote_port_str, "%d", remote_node->get_Port() );
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
        mrn_dbg(1, mrn_printf(FLF, stderr, "XPlat::TLSKey::Set(): %s\n",
                   strerror(status)));
        XPlat::Thread::Exit(args);
    }

    mrn_dbg(3, mrn_printf(FLF, stderr, "In recv_thread_main()\n"));
    while(true){
        // TODO this only works if the recv on remote_node is blocking recv...
        // is it?
        int rret = remote_node->recv( packet_list );
        if( (rret == -1) || ((rret == 0) && (packet_list.size() == 0)) ) {
            if( rret == -1 ) {
                mrn_dbg(1, mrn_printf(FLF, stderr, 
                           "RemoteNode recv failed - recv thread exiting\n"));
            }
            XPlat::Thread::Exit(args);
        }

        if( remote_node->is_upstream() ){
            if(local_child_node->proc_PacketsFromUpStream(packet_list) == -1){
                mrn_dbg(1, mrn_printf(FLF, stderr, "proc_PacketsFromUpstream() failed\n"));
            }
        }
        else{
            if(local_parent_node->proc_PacketsFromDownStream(packet_list) == -1){
                mrn_dbg(1, mrn_printf(FLF, stderr, "proc_PacketsFromDownstream() failed\n"));
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
        sprintf(local_port_str, "%d", local_child_node->get_Port());
        sprintf(remote_port_str, "%d", remote_node->get_Port() );
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
        sprintf(remote_port_str, "%d", remote_node->get_Port() );
        getHostName(local_hostname, local_child_node->get_HostName() );
        getHostName(remote_hostname, remote_node->get_HostName() );

        name = "DOWNSEND(";
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
        mrn_dbg(1, mrn_printf(0,0,0, stderr, "XPlat::TLSKey::Set(): %s\n",
                   strerror(status))); 
        XPlat::Thread::Exit(args);
    }

    while(true){
        remote_node->msg_out_sync.Lock();

        while(remote_node->msg_out.size_Packets() == 0){
            mrn_dbg(3, mrn_printf(FLF, stderr, "send_thread_main() waiting on nonempty ..\n"));
            remote_node->msg_out_sync.WaitOnCondition(MRN_MESSAGEOUT_NONEMPTY);
        }

        mrn_dbg(3, mrn_printf(FLF, stderr, "send_thread_main() sending packets ...\n"));
        if( remote_node->msg_out.send(remote_node->sock_fd) == -1 ){
            mrn_dbg(1, mrn_printf(FLF, stderr, "RN: send_thread_main: send failed\n" ));
            mrn_dbg(1, mrn_printf(FLF, stderr, "msg.send() failed. Thread Exiting\n"));
            remote_node->msg_out_sync.Unlock();
            XPlat::Thread::Exit(args);
        }

        remote_node->msg_out_sync.Unlock();
    }

    return NULL;
}


RemoteNode::RemoteNode(bool _threaded, std::string &_hostname, Port _port)
    :CommunicationNode(_hostname, _port), threaded(_threaded), sock_fd(0),
     _is_internal_node(false), rank( UnknownRank ), _is_upstream(false)
{
    msg_out_sync.RegisterCondition(MRN_MESSAGEOUT_NONEMPTY);
}

int RemoteNode::connect()
{
    mrn_dbg(3, mrn_printf(FLF, stderr, "In connect(%s:%d) ...\n",
               hostname.c_str(), port));
    if(connectHost(&sock_fd, hostname.c_str(), port) == -1){
        mrn_dbg(1, mrn_printf(FLF, stderr, "connect_to_host() failed\n"));
        error( MRN_ESYSTEM, "connect(): %s\n", strerror(errno) );
        return -1;
    }
    
    mrn_dbg(3, mrn_printf(FLF, stderr,
               "connect_to_host() succeeded. new socket = %d\n", sock_fd));
    return 0;
}

int RemoteNode::accept_Connection( int lsock_fd, bool do_connect ) const
{
    int retval = 0;

    if( do_connect ) {
        if( ( *(const_cast< int * >( & sock_fd )) = getSocketConnection(lsock_fd)) == -1){
            mrn_dbg(1, mrn_printf(FLF, stderr, "get_socket_connection() failed\n"));
            error( MRN_ESYSTEM, "getSocketConnection(): %s\n",
                   strerror(errno) );
            return -1;
        }
    }
    else {
        // socket is already connected
        *(const_cast< int * >( & sock_fd )) = lsock_fd;
    }
    assert( sock_fd != -1 );

    if(threaded){
        mrn_dbg(3, mrn_printf(FLF, stderr, "Creating Downstream recv thread ...\n"));
        retval = XPlat::Thread::Create( RemoteNode::recv_thread_main,
                                        const_cast< void * >( (const void *) this ),
                                        const_cast< long int * >( (const long int *) & recv_thread_id ) );
        if(retval != 0){
            error( MRN_ESYSTEM, "XPlat::Thread::Create() failed: %s\n",
                    strerror(errno) );
            mrn_dbg(1, mrn_printf(FLF, stderr, "Downstream recv thread creation failed...\n"));
        }
        mrn_dbg(3, mrn_printf(FLF, stderr, "Creating Downstream send thread ...\n"));
        retval = XPlat::Thread::Create( RemoteNode::send_thread_main,
                                        const_cast< void * >( (const void *) this ),
                                        const_cast< long int * >( (const long int *) & send_thread_id ) );
        if(retval != 0){
            error( MRN_ESYSTEM, "XPlat::Thread::Create() failed: %s\n",
                    strerror(errno) );
            mrn_dbg(1, mrn_printf(FLF, stderr, "Downstream send thread creation failed...\n"));
        }
    }

    return retval;
}


int RemoteNode::new_InternalNode(int listening_sock_fd,
                                 std::string parent_host, Port parent_port,
                                 std::string commnode_cmd)
const
{
    char parent_port_str[128];
    sprintf(parent_port_str, "%d", parent_port);
    char port_str[128];
    sprintf(port_str, "%d", port );

    mrn_dbg(3, mrn_printf(FLF, stderr, "In new_InternalNode(%s:%d) ...\n",
               hostname.c_str(), port ));

    *(const_cast< bool * >( (const bool*)&_is_internal_node )) = true;

    // set up arguments for the new process
    std::vector <std::string> args;
    args.push_back(commnode_cmd);
    args.push_back(port_str);
    args.push_back(parent_host);
    args.push_back(std::string(parent_port_str));

    if( XPlat::Process::Create( hostname, commnode_cmd, args ) != 0 ){
        int err = XPlat::Process::GetLastError();

        error( MRN_ESYSTEM, "XPlat::Process::Create(%s %s): %s\n",
               hostname.c_str(), commnode_cmd.c_str(),
               XPlat::Error::GetErrorString( err ).c_str() );
        mrn_dbg(1, mrn_printf(FLF, stderr,
                   "XPlat::Process::Create(%s %s): %s\n",
                   hostname.c_str(), commnode_cmd.c_str(),
                   XPlat::Error::GetErrorString( err ).c_str() ));
        return -1;
    }

    if( accept_Connection( listening_sock_fd ) == -1 ){
        error( MRN_ESYSTEM, "accept_Connection(): %s\n", strerror(errno) );
        mrn_dbg(1, mrn_printf(FLF, stderr,"accept_Connection(): %s\n",
                   strerror(errno) ));
        return -1;
    }
    return 0;
}


int RemoteNode::new_Application(int listening_sock_fd,
                                std::string parent_host, Port parent_port,
                                Rank be_rank,
                                std::string &cmd, std::vector <std::string> &args)
const
{

    mrn_dbg(3, mrn_printf(FLF, stderr, "In new_Application(\n"
               "\thost: %s:%d,\n"
               "\tcmd: %s)\n",
               hostname.c_str(), port, cmd.c_str()));
  
    char parent_port_str[128];
    sprintf(parent_port_str, "%d", parent_port);
    char rank_str[128];
    sprintf(rank_str, "%d", be_rank );

    std::string parent_nethost;
    getNetworkName( parent_nethost, parent_host );

    // set up arguments for new process
    // TODO: make more elegant. for now we do a copy to get the cmd in front (darnol)

    std::vector<std::string> new_args;

    new_args.push_back(cmd);
    for(unsigned int i=0; i<args.size(); i++){
        new_args.push_back(args[i]);
    }
    new_args.push_back(parent_nethost);
    new_args.push_back(std::string(parent_port_str));
    new_args.push_back(std::string(rank_str));

    mrn_dbg(5, mrn_printf(FLF, stderr, "new_Application() calling create ...\n"
               "\thost: %s:%d,\n"
               "\tcmd: %s)\n",
               hostname.c_str(), cmd.c_str()));
  
    if( XPlat::Process::Create( hostname, cmd, new_args ) != 0 ){
        mrn_dbg(1, mrn_printf(FLF, stderr, "XPlat::Process::Create() failed\n");) 
        int err = XPlat::Process::GetLastError();
        error( MRN_ESYSTEM, "XPlat::Process::Create(%s %s): %s\n",
               hostname.c_str(), cmd.c_str(),
               XPlat::Error::GetErrorString( err ).c_str() );
        return -1;
    }
    mrn_dbg(5, mrn_printf(FLF, stderr, "Success\n"));

    mrn_dbg(5, mrn_printf(FLF, stderr, "new_Application() calling connect ...\n"));
    // establish connection with the backend
    // because we created the process and delivered its rank on 
    // the command line, the back-end should know its own rank.
    *(const_cast< int * >( (const int *)&sock_fd) ) = connect_to_backend( listening_sock_fd, &be_rank );
    if( sock_fd == -1 )
    {
        // callee handled the error
        return -1;
    }
    *(const_cast< int * >( (const int*)&rank) ) = be_rank;

    mrn_dbg(5, mrn_printf(FLF, stderr, "success!.\nnew_Application() calling accept()...\n"));
    // finalize the connection
    if( accept_Connection( sock_fd, false ) != 0 ) {
        error( MRN_ESYSTEM, "accept_Connection(): %s\n", strerror(errno) );
        return -1;
    }
    mrn_dbg(5, mrn_printf(FLF, stderr, "success!\n"));
    return 0;
}


int
RemoteNode::connect_to_backend( int listening_sock_fd, Rank* rank )
{
    assert( rank != NULL );

    // accept the socket connection
    int sock_fd = getSocketConnection( listening_sock_fd );
    if( sock_fd == -1 ) {
        mrn_dbg(1, mrn_printf(FLF, stderr, "get_socket_connection() failed\n" ));
        return -1;
    }

    // do our side of the handshake
    // receive the back-end's idea of its rank
    uint32_t netrank = UnknownRank;
    int rret = ::recv( sock_fd, (char*)&netrank, sizeof(netrank), 0 );
    if( rret != sizeof(Rank) )
    {
        mrn_dbg(1, mrn_printf(FLF, stderr, "failed to receive rank from back-end\n" ));
        XPlat::SocketUtils::Close( sock_fd );
        sock_fd = -1;
        return -1;
    }

    // validate the rank
    Rank hostrank = ntohl( netrank );
    if( hostrank == UnknownRank )
    {
        // the backend should've known its own rank
        mrn_dbg( 1, mrn_printf(FLF, stderr, "backend handshake failed: unknown rank\n" ));
        XPlat::SocketUtils::Close( sock_fd );
        sock_fd = -1;
        return -1;
    }
    else if( *rank != UnknownRank )
    {
        if( *rank != hostrank )
        {
            // the back-end told us a rank that was different
            // from what we were expecting
            mrn_dbg( 1, mrn_printf(FLF, stderr, 
                "backend handshake failed: unexpected rank\n" ));
            XPlat::SocketUtils::Close( sock_fd );
            sock_fd = -1;
            return -1;
        }
    }
    else
    {
        // just set the rank to what the back-end gave us
        *rank = hostrank;
    }
    assert( *rank == hostrank );

    return sock_fd;
}

int RemoteNode::connect_to_leaf( Rank myRank )
{
    connect();
    if( fail() ){
        mrn_dbg(1, mrn_printf(FLF, stderr, "connect() failed\n"));
        XPlat::SocketUtils::Close( sock_fd );
        sock_fd = -1;
        return -1;
    }
  
    // do low-level handshake with our rank
    uint32_t netrank = htonl( myRank );
    int sret = ::send( sock_fd, (const char*)&netrank, sizeof(netrank), 0 );
    if( sret == -1 ) 
    {
        mrn_dbg(1, mrn_printf(FLF, stderr, 
            "leaf handshake failed: send failed: %d: %s \n", 
            errno, strerror(errno) ));
        error( MRN_ESYSTEM, "send(): %s\n", strerror( errno ) );
        XPlat::SocketUtils::Close( sock_fd );
        sock_fd = -1;
        return -1;
    }

    return sock_fd;
}





int RemoteNode::send(Packet& packet) const
{
    mrn_dbg(3, mrn_printf(FLF, stderr, "In remotenode.send(). Calling msg.add_packet()\n"));

    if(threaded){
        msg_out_sync.Lock();
    }

    msg_out.add_Packet(packet);

    //TODO: implement flush based on timeout/threshold
    // for right now, always flush
    if( msg_out.size_Packets() > 1 ) {
        int sret = msg_out.send( sock_fd );
        if( sret == -1 ) {
            mrn_dbg(1, mrn_printf(FLF, stderr, "Message.send failed\n" ));
        }
    }

    if(threaded){
        msg_out_sync.SignalCondition(MRN_MESSAGEOUT_NONEMPTY);
        msg_out_sync.Unlock();
    }

    mrn_dbg(3, mrn_printf(FLF, stderr, "Leaving remotenode.send()\n"));
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
        mrn_dbg(1, mrn_printf(FLF, stderr, "select() failed\n"));
        return false;
    }

    // We only put one descriptor in the read set.  Therefore, if the return 
    // value from select() is 1, that descriptor has data available.
    if( sret == 1 ){
        mrn_dbg(1, mrn_printf(FLF, stderr, "select() says data to be read.\n"));
        return true;
    }

    mrn_dbg(3, mrn_printf(FLF, stderr, "Leaving remotenode.has_data(). No data available\n"));
    return false;
}


int RemoteNode::flush() const
{
    if(threaded){
        return 0;
    }

    mrn_dbg(3, mrn_printf(FLF, stderr, "In remotenode.flush(). Calling msg.send()\n"));
    if( msg_out.send(sock_fd) == -1){
        mrn_dbg(1, mrn_printf(FLF, stderr, "msg.send() failed\n"));
        return -1;
    }

    mrn_dbg(3, mrn_printf(FLF, stderr, "Leaving remotenode.flush().\n"));
    return 0;
}

} // namespace MRN
