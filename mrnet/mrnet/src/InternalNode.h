/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(__internalnode_h)
#define __internalnode_h 1

#include <string>
#include <list>

#include "CommunicationNode.h"
#include "ParentNode.h"
#include "ChildNode.h"
#include "Message.h"

namespace MRN
{

class Network;

class InternalNode: public ParentNode, public ChildNode
{
 public:
    InternalNode( Network * inetwork,
                  std::string const& ihostname, Rank irank,
                  std::string const& iphostname, Port ipport, Rank iprank );
    virtual ~InternalNode(void);

    void waitLoop() const;
    virtual int proc_DataFromParent( PacketPtr ipacket ) const;
    virtual int proc_DataFromChildren( PacketPtr ipacket ) const;
    virtual int proc_FailureReportFromParent( PacketPtr ipacket ) const;
    virtual int proc_NewParentReportFromParent( PacketPtr ipacket ) const;
    void signal_NetworkTermination( );
    void waitfor_NetworkTermination( );

 private:
    XPlat::Monitor _sync;
    enum {NETWORK_TERMINATION};
};

} // namespace MRN
#endif /* __internalnode_h */
