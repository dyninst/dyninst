/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(__remotenode_h) 
#define __remotenode_h 1

#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/Message.h"
#include "xplat/Thread.h"
#include "xplat/Monitor.h"
#include "xplat/Mutex.h"


namespace MRN
{

class ChildNode;
class ParentNode;
class RemoteNode:public CommunicationNode, public Error {
 private:
    enum {MRN_MESSAGEOUT_NONEMPTY};
    bool threaded;
    mutable Message msg_in;
    mutable Message msg_out;

    int sock_fd;
    bool _is_internal_node;
    Rank rank;              // UnknownRank for all but connections to a BE

    int accept_Connection( int sock_fd, bool doConnect = true ) const;
    static int poll_timeout;
    static XPlat::Mutex poll_timeout_mutex;

 public:
    static ParentNode * local_parent_node;
    static ChildNode * local_child_node;
    static void * recv_thread_main(void * arg);
    static void * send_thread_main(void * arg);
    XPlat::Thread::Id recv_thread_id, send_thread_id;
    bool _is_upstream;
    mutable XPlat::Monitor msg_out_sync;

    RemoteNode(bool threaded, std::string &_hostname, Port _port );
    int connect();
    int new_InternalNode(int listening_sock_fd,
                         std::string parent_hostname, Port parent_port,
                         std::string commnode) const;
    int new_Application(int listening_sock_fd,
                        std::string parent_hostname, Port parent_port,
                        Rank be_rank,
                        std::string &cmd, std::vector <std::string> &args) const;
    int accept_Application( int sock_fd )const;

    static int connect_to_backend( int listening_sock, Rank* rank );
    int connect_to_leaf( Rank r );

    int send(Packet&) const ;
    int flush() const ;
    int recv(std::list <Packet> &) const; //blocking recv
    bool has_data() const;
    bool is_backend() const;
    bool is_internal() const;
    bool is_upstream() const;

    int get_SocketFd( void ) const { return sock_fd; }

    Rank get_Rank( void ) const { return rank; }

    static void set_BlockingTimeOut( int _timeout );
    static int get_BlockingTimeOut( );
};

inline int RemoteNode::accept_Application( int connected_sock_fd ) const
{
    return accept_Connection( connected_sock_fd, false );    
}

inline int RemoteNode::recv(std::list <Packet> &packet_list) const
{
    return msg_in.recv(sock_fd, packet_list, this);
}

inline bool RemoteNode::is_backend() const {
    return !_is_internal_node;
}

inline bool RemoteNode::is_internal() const {
    return _is_internal_node;
}

inline bool RemoteNode::is_upstream() const {
    return _is_upstream;
}

inline void RemoteNode::set_BlockingTimeOut(int itimeout)
{
    poll_timeout_mutex.Lock();
    poll_timeout = itimeout;
    poll_timeout_mutex.Unlock();
}

inline int RemoteNode::get_BlockingTimeOut( )
{
    int ret;

    poll_timeout_mutex.Lock();
    ret = poll_timeout;
    poll_timeout_mutex.Unlock();

    return ret;
}

} // namespace MRN

#endif /* __remotenode_h  */
