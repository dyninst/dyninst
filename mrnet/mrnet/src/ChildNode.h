/****************************************************************************
 * Copyright © 2003-2008 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(__childnode_h)
#define __childnode_h 1

#include <string>

#include "Message.h"
#include "PeerNode.h"

namespace MRN
{
class Network;

class ChildNode: public Error, public CommunicationNode {
  public:
    ChildNode( Network *inetwork,
               std::string const& ihostname, Rank irank,
               std::string const& iphostname, Port ipport, Rank iprank );
    virtual ~ChildNode(void) {};

    int proc_PacketsFromParent(std::list <PacketPtr> &)const;
    virtual int proc_DataFromParent(PacketPtr)const=0;
    virtual int proc_FailureReportFromParent( PacketPtr ipacket ) const=0;
    virtual int proc_NewParentReportFromParent( PacketPtr ipacket ) const=0;
    int proc_CollectPerfData( PacketPtr ipacket ) const;
    int proc_TopologyReport( PacketPtr ipacket ) const;
    int proc_RecoveryReport( PacketPtr ipacket ) const;
    int send_NewSubTreeReport( void )const;
    bool ack_DeleteSubTree( void ) const ;


    int recv_PacketsFromParent( std::list <PacketPtr> &packet_list ) const;
    int send_EventsToParent( ) const;
    bool has_PacketsFromParent( ) const;

    virtual void error( ErrorCode, const char *, ... ) const;

    int init_newChildDataConnection( PeerNodePtr iparent,
                                     Rank ifailed_rank = UnknownRank );
    int request_SubTreeInfo( void ) const ;

 protected:
    Network * _network;

 private:
    uint16_t _incarnation; //incremented each time child connects to new parent
};

} // namespace MRN

#endif /* __childnode_h */
