/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(__internalnode_h)
#define __internalnode_h 1

#include <string>
#include <list>

#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/ParentNode.h"
#include "mrnet/src/ChildNode.h"
#include "mrnet/src/Message.h"

namespace MRN
{

class InternalNode: public ParentNode, public ChildNode,
    public CommunicationNode
{
 protected:
    virtual int deliverLeafInfoResponse( Packet& pkt ) const;
    virtual int deliverConnectLeavesResponse( Packet& pkt ) const;

 public:
    InternalNode(std::string hostname, Port port,
                 std::string _phostname, Port _pport);
    virtual ~InternalNode(void);

    void waitLoop() const;
    int send_newSubTreeReport(bool status) const;
    virtual int proc_PacketsFromUpStream(std::list <Packet> &) const;
    virtual int proc_DataFromUpStream(Packet&) const;
    virtual int proc_PacketsFromDownStream(std::list <Packet> &) const;
    virtual int proc_DataFromDownStream(Packet&) const;
};

} // namespace MRN
#endif /* __internalnode_h */
