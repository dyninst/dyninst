#if !defined(__remotenode_h) 
#define __remotenode_h 1

#include <poll.h>

#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/Message.h"
#include "mrnet/src/pthread_sync.h"

namespace MRN
{

class ChildNode;
class ParentNode;
class RemoteNode:public CommunicationNode{
 private:
    enum {MRN_MESSAGEOUT_NONEMPTY};
    bool threaded;
    Message msg_in, msg_out;

    int sock_fd;
    bool _is_internal_node;
    struct pollfd poll_struct;

    int accept_Connection( int sock_fd, bool doConnect = true );

 public:
    static ParentNode * local_parent_node;
    static ChildNode * local_child_node;
    static void * recv_thread_main(void * arg);
    static void * send_thread_main(void * arg);
    pthread_t recv_thread_id, send_thread_id;
    bool _is_upstream;
    pthread_sync msg_out_sync;

    RemoteNode(bool threaded, std::string &_hostname, unsigned short _port);
    RemoteNode(bool threaded, std::string &_hostname, unsigned short _port,
               unsigned short _id);
    int connect();
    int new_InternalNode(int listening_sock_fd, std::string parent_hostname,
                         unsigned short parent_port, unsigned short parent_id,
                         std::string commnode);
    int new_Application(int listening_sock_fd,
                        unsigned int backend_id,
                        std::string parent_hostname,
                        unsigned short parent_port, unsigned short parent_id,
                        std::string &cmd, std::vector <std::string> &args);
    int accept_Application( int sock_fd );

    int send(Packet *);
    int flush();
    int recv(std::list <Packet *> &); //blocking recv
    bool has_data();
    bool is_backend();
    bool is_internal();
    bool is_upstream();

    const pollfd& get_pollfd( void ) const    { return poll_struct; }
    int get_sockfd( void ) const              { return sock_fd; }

    static void set_BlockingTimeOut( int _timeout );
    static int get_BlockingTimeOut( );
    static int poll_timeout;
};

inline int RemoteNode::accept_Application( int connected_sock_fd )
{
    return accept_Connection( connected_sock_fd, false );    
}

inline int RemoteNode::recv(std::list <Packet *> &packet_list)
{
    return msg_in.recv(sock_fd, packet_list, this);
}

inline bool RemoteNode::is_backend(){
    return !_is_internal_node;
}

inline bool RemoteNode::is_internal(){
    return _is_internal_node;
}

inline bool RemoteNode::is_upstream(){
    return _is_upstream;
}

inline void RemoteNode::set_BlockingTimeOut(int _timeout)
{
    poll_timeout = _timeout;
}

inline int RemoteNode::get_BlockingTimeOut( )
{
    return poll_timeout;
}

} // namespace MRN

#endif /* __remotenode_h  */
