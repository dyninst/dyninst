/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(__backendnode_h)
#define __backendnode_h 1

#include <string>

#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/ChildNode.h"
#include "mrnet/src/Message.h"

namespace MRN
{

class BackEndNode: public ChildNode, public CommunicationNode{
 private:
    Network * network;
    Rank rank;

    int proc_newStream( Packet & pkt ) const;

 public:
    BackEndNode(Network * _network, 
                    std::string _my_hostname, Port _my_port, Rank _my_rank,
                    std::string _parent_hostname, Port _parent_port);
    virtual ~BackEndNode(void);

    virtual int proc_PacketsFromUpStream(std::list <Packet> &) const;
    virtual int proc_DataFromUpStream(Packet &) const;
    int send(Packet &) const;
    int flush() const;
    int recv( bool blocking=true ) const;

    int get_SocketFd() const ;
};

inline int BackEndNode::get_SocketFd() const
{
    assert( ChildNode::get_UpStreamNode() );
    return ChildNode::get_UpStreamNode()->get_SocketFd();
}
} // namespace MRN

#endif /* __backendnode_h */
