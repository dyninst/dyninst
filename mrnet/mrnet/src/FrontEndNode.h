/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(__frontendnode_h)
#define __frontendnode_h 1

#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/ParentNode.h"
#include "mrnet/src/Message.h"

namespace MRN
{

class FrontEndNode: public ParentNode, public CommunicationNode{
 private:
    Network * network;
    std::string commnode;
    mutable Packet leafInfoPacket;
    mutable Packet leavesConnectedPacket;

 protected:
    virtual int deliverLeafInfoResponse( Packet& pkt ) const;
    virtual int deliverConnectLeavesResponse( Packet& pkt ) const;

 public:
    FrontEndNode(Network *, std::string _hostname, Port _port);
    virtual ~FrontEndNode(void);
    virtual int proc_PacketsFromDownStream(std::list <Packet> &) const;
    virtual int proc_DataFromDownStream(Packet&) const;
    int send(Packet&) const;
    int flush() const;
    int flush(unsigned int) const;
    int recv( bool blocking=true ) const;

    Packet get_leafInfoPacket( void ) const;
    Packet get_leavesConnectedPacket( void ) const;
};

inline Packet FrontEndNode::get_leafInfoPacket( void ) const
{
    Packet ret = leafInfoPacket;
    leafInfoPacket = *Packet::NullPacket;
    return ret;
}

inline Packet FrontEndNode::get_leavesConnectedPacket( void ) const
{
    Packet ret = leavesConnectedPacket;
    leavesConnectedPacket = *Packet::NullPacket;
    return ret;
}

inline int FrontEndNode::send( Packet & packet ) const
{
    return send_PacketDownStream( packet );
}

inline int FrontEndNode::flush(  ) const
{
    return flush_PacketsDownStream(  );
}

inline int FrontEndNode::flush( unsigned int stream_id ) const
{
    return flush_PacketsDownStream( stream_id );
}

} // namespace MRN
#endif /* __frontendnode_h */
