/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(__parentnode_h)
#define __parentnode_h 1

#include <map>
#include <list>
#include <string>

#include "mrnet/src/Message.h"
#include "mrnet/src/StreamManager.h"
#include "mrnet/src/Filter.h"
#include "mrnet/src/RemoteNode.h"
#include "xplat/Monitor.h"
#include "xplat/Mutex.h"

namespace MRN
{

class ParentNode: public Error {
    friend class Aggregator;
    friend class Synchronizer;
    friend class RemoteNode;
 private:

    std::string hostname;
    Port port;
    Port config_port;
    int listening_sock_fd;

    bool threaded;

    bool isLeaf_;           // am I a leaf in the MRNet tree?
    mutable std::vector < Packet >childLeafInfoResponses;
    mutable XPlat::Mutex childLeafInfoResponsesLock;
    mutable std::vector < Packet >childConnectedLeafResponses;
    mutable XPlat::Mutex childConnectedLeafResponsesLock;

    int wait_for_SubTreeReports( ) const;

 protected:
    enum
        { ALLNODESREPORTED };
    mutable std::list < const RemoteNode * >children_nodes;
    mutable std::list < int >backend_descendant_nodes;
    mutable XPlat::Monitor subtreereport_sync;
    mutable unsigned int num_descendants, num_descendants_reported;

    mutable std::map < unsigned int, const RemoteNode * >ChildNodeByRank;
    mutable XPlat::Mutex childnodebybackendid_sync;

    //std::map < unsigned int, StreamManager * >StreamManagerById;
    //XPlat::Mutex streammanagerbyid_sync;

    virtual int deliverLeafInfoResponse( Packet& pkt )const = 0;
    virtual int deliverConnectLeavesResponse( Packet& pkt )const = 0;

    bool isLeaf( void ) const {
        return isLeaf_;
    }

 public:
    ParentNode( bool _threaded, std::string, Port );
    virtual ~ ParentNode( void );

    virtual int proc_PacketsFromDownStream( std::list < Packet >& ) const = 0;
    virtual int proc_DataFromDownStream( Packet& ) const = 0;

    int recv_PacketsFromDownStream( std::list < Packet >&packet_list,
                                    bool blocking = true ) const;
    int send_PacketDownStream( Packet & packet, bool internal_only =
                               false ) const;
    int flush_PacketsDownStream( unsigned int stream_id ) const;
    int flush_PacketsDownStream( void ) const;

    int proc_newSubTree( Packet & ) const;
    int proc_delSubTree( Packet & ) const;

    int proc_newSubTreeReport( Packet & ) const;
    int proc_Event( Packet & ) const;
    int send_Event( Packet & ) const;

    StreamManager *proc_newStream( Packet & ) const;
    int send_newStream( Packet &, StreamManager * ) const;
    int proc_delStream( Packet & ) const;
    int proc_newApplication( Packet & ) const;
    int proc_delApplication( Packet & ) const;

    int proc_newFilter( Packet & ) const;

    int proc_getLeafInfo( Packet & ) const;
    int proc_getLeafInfoResponse( Packet & ) const;

    int proc_connectLeaves( Packet & ) const;
    int proc_connectLeavesResponse( Packet & ) const;

    std::string get_HostName(  ) const;
    Port get_Port(  ) const;
    int get_SocketFd(int **array, unsigned int *array_size) const;
    int getConnections( int **conns, unsigned int *nConns ) const;
};

bool lt_RemoteNodePtr( RemoteNode * p1, RemoteNode * p2 );
bool equal_RemoteNodePtr( RemoteNode * p1, RemoteNode * p2 );


inline std::string ParentNode::get_HostName(  ) const
{
    return hostname;
}

inline Port ParentNode::get_Port(  ) const
{
    return port;
}

}                               // namespace MRN

#endif                          /* __parentnode_h */
