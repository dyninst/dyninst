/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(__PeerNode_h) 
#define __PeerNode_h 1

#include <map>
#include <boost/shared_ptr.hpp>

#include "CommunicationNode.h"
#include "Message.h"
#include "Error.h"
#include "mrnet/Types.h"
#include "xplat/Thread.h"
#include "xplat/Monitor.h"
#include "xplat/Mutex.h"

namespace MRN
{

class ChildNode;
class ParentNode;
class Network;

class Packet;

class PeerNode;
typedef boost::shared_ptr<PeerNode> PeerNodePtr;

class PeerNode:public CommunicationNode, public Error {
    friend class Network;
 public:
    static PeerNodePtr NullPeerNode;
    static void * recv_thread_main(void * arg);
    static void * send_thread_main(void * arg);

    int connect_DataSocket( void );
    int connect_EventSocket( void );

    int new_InternalNode(int listening_sock_fd,
                         std::string parent_hostname, Port parent_port,
                         std::string commnode) const;
    int new_Application(int listening_sock_fd,
                        std::string parent_hostname, Port parent_port,
                        Rank be_rank,
                        std::string &cmd, std::vector <std::string> &args) const;

    static int connect_to_backend( int listening_sock, Rank* rank );
    int connect_to_leaf( Rank r );

    int send(const PacketPtr ) const ;
    int sendDirectly( const PacketPtr ipacket ) const;
    int flush( bool ignore_threads = false ) const ;
    int recv(std::list <PacketPtr> &) const; //blocking recv
    bool has_data() const;
    bool is_backend() const;
    bool is_internal() const;
    bool is_parent() const;
    bool is_child() const;

    void set_DataSocketFd( int isock ) { _data_sock_fd = isock; }
    int get_DataSocketFd( void ) const { return _data_sock_fd; }
    int get_EventSocketFd( void ) const { return _event_sock_fd; }
    int start_CommunicationThreads( void );

    int waitfor_FlushCompletion( void ) const ;
    void signal_FlushComplete( void ) const ;
    void mark_Failed( void );

    static void set_BlockingTimeOut( int _timeout );
    static int get_BlockingTimeOut( void );

    static void cancel_ChildrenSendThreads( void );

    static PeerNodePtr new_PeerNode( std::string const& ihostname,
                                     Port iport,
                                     Rank irank,
                                     bool iis_parent,
                                     bool iis_internal );
    static void delete_PeerNode( Rank irank );
    static PeerNodePtr get_PeerNode( Rank );
    static const std::map < Rank, PeerNodePtr > & get_PeerNodes();

 private:
    PeerNode( Network *, std::string const& ihostname, Port iport, Rank irank,
              bool iis_parent, bool iis_internal );

    //Static data members
    Network * _network;
    int _data_sock_fd;
    int _event_sock_fd;
    bool _is_internal_node;
    bool _is_parent;
    XPlat::Thread::Id recv_thread_id, send_thread_id;

    //Dynamic data members
    mutable Message _msg_out;
    mutable Message _msg_in;
    bool _available;
    mutable XPlat::Monitor _flush_sync;
    enum{ MRN_FLUSH_COMPLETE };

    static int poll_timeout;
    static XPlat::Mutex poll_timeout_mutex;
};

} // namespace MRN

#endif /* __PeerNode_h  */
