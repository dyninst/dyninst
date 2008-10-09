/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(__backendnode_h)
#define __backendnode_h 1

#include <string>

#include "CommunicationNode.h"
#include "ChildNode.h"
#include "Message.h"

namespace MRN
{
class Network;

class BackEndNode: public ChildNode{
 public:
    BackEndNode(Network * inetwork, 
                std::string imy_hostname, Rank imy_rank,
                std::string iphostname, Port ipport, Rank iprank );
    virtual ~BackEndNode(void);

    virtual int proc_DataFromParent( PacketPtr ) const;
    virtual int proc_FailureReportFromParent( PacketPtr ) const;
    virtual int proc_NewParentReportFromParent( PacketPtr  ) const;

    //int send( PacketPtr ) const;
    //int flush() const;
    //int recv( bool blocking=true ) const;

    int proc_newStream( PacketPtr ) const;
    int proc_DeleteSubTree( PacketPtr ) const;
    int proc_newFilter( PacketPtr ) const;
    int proc_DownstreamFilterParams( PacketPtr &ipacket ) const;
    int proc_UpstreamFilterParams( PacketPtr &ipacket ) const;
};

} // namespace MRN

#endif /* __backendnode_h */
