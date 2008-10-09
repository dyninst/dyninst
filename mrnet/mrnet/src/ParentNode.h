/****************************************************************************
 * Copyright  2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(__parentnode_h)
#define __parentnode_h 1

#include <map>
#include <list>
#include <string>

#include "Error.h"
#include "xplat/Monitor.h"
#include "xplat/Mutex.h"
#include "mrnet/Packet.h"
#include "CommunicationNode.h"
#include "PeerNode.h"

namespace MRN
{

class Network;
class Stream;

class ParentNode: public Error, public CommunicationNode {
    friend class Aggregator;
    friend class Synchronizer;
    friend class PeerNode;
 public:
    ParentNode( Network *, std::string const&, Rank );
    virtual ~ ParentNode( void );

    int proc_PacketsFromChildren( std::list < PacketPtr >& ) const;
    virtual int proc_DataFromChildren( PacketPtr ) const = 0;
    virtual int proc_NewParentReportFromParent( PacketPtr ipacket ) const=0;
    int proc_FailureReport( PacketPtr ipacket ) const;
    int proc_RecoveryReport( PacketPtr ipacket ) const;

    int recv_PacketsFromChildren( std::list < PacketPtr >&packet_list,
                                    bool blocking = true ) const;
    int flush_PacketsToChildren( void ) const;

    int proc_newSubTree( PacketPtr ipacket );
    int proc_DeleteSubTree( PacketPtr ipacket ) const;
    int proc_DeleteSubTreeAck( PacketPtr ipacket ) const;

    int proc_newSubTreeReport( PacketPtr ipacket ) const;
    int proc_Event( PacketPtr ipacket ) const;
    int send_Event( PacketPtr ipacket ) const;

    Stream * proc_newStream( PacketPtr ipacket ) const;
    int proc_deleteStream( PacketPtr ipacket ) const;
    int proc_closeStream( PacketPtr ipacket ) const;

    int proc_newFilter( PacketPtr ipacket ) const;
    int proc_DownstreamFilterParams( PacketPtr &ipacket ) const;
    int proc_UpstreamFilterParams( PacketPtr &ipacket ) const;

    int proc_SubTreeInfoRequest( PacketPtr ipacket ) const;

    int get_ListeningSocket( void ) const { return listening_sock_fd; }
    int getConnections( int **conns, unsigned int *nConns ) const;

    int proc_NewChildDataConnection( PacketPtr ipacket, int sock );
    PeerNodePtr find_ChildNodeByRank( int irank );

    int launch_InternalNode( std::string ihostname, Rank irank,
                             std::string icommnode_exe ) const;
    int launch_Application( std::string ihostname, Rank irank,
                            std::string &ibackend_exe,
                            std::vector <std::string> &ibackend_args) const;

    int waitfor_SubTreeReports( void ) const;
    bool waitfor_DeleteSubTreeAcks( void ) const ;

 protected:
    Network * _network;

    enum { ALLNODESREPORTED };
    mutable XPlat::Monitor subtreereport_sync;
    mutable unsigned int _num_children, _num_children_reported;

 private:
    int listening_sock_fd;
    PacketPtr _initial_subtree_packet;
};

//bool lt_PeerNodePtr( PeerNodePtr p1, PeerNodePtr p2 );
//bool equal_PeerNodePtr( PeerNodePtr p1, PeerNodePtr p2 );

}                               // namespace MRN

#endif                          /* __parentnode_h */
