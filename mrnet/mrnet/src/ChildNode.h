/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(__childnode_h)
#define __childnode_h 1

#include <string>

#include "mrnet/src/Message.h"
#include "mrnet/src/RemoteNode.h"

namespace MRN
{

class ChildNode: public Error{
 private:
    static const RemoteNode * upstream_node;
    std::string hostname;
    Port port;
    bool threaded;

 protected:
    static void set_UpStreamNode( const RemoteNode * );

 public:
    ChildNode( std::string ihostname, Port iport, bool ithreaded );
    virtual ~ChildNode(void) {};

    virtual int proc_PacketsFromUpStream(std::list <Packet> &)const=0;
    virtual int proc_DataFromUpStream(Packet&)const=0;

    int recv_PacketsFromUpStream(std::list <Packet> &packet_list) const;
    int send_PacketUpStream(Packet& packet) const;
    int flush_PacketsUpStream() const;
    int send_Events( ) const;
    bool has_data( ) const;

    std::string get_HostName() const;
    Port get_Port() const;

    int getConnections( int** conns, unsigned int* nConns ) const;
    virtual void error( ErrorCode, const char *, ... ) const;

    static const RemoteNode * get_UpStreamNode( void );
};

inline bool ChildNode::has_data( ) const
{
    return upstream_node->has_data();
}

inline int ChildNode::recv_PacketsFromUpStream(std::list <Packet>
                                               &packet_list) const
{
  return upstream_node->recv(packet_list);
}

inline int ChildNode::send_PacketUpStream(Packet& packet) const
{
  return upstream_node->send(packet);
}

inline int ChildNode::flush_PacketsUpStream() const
{
  return upstream_node->flush();
}

inline std::string ChildNode::get_HostName() const
{
  return hostname;
}

inline Port ChildNode::get_Port() const
{
  return port;
}

inline void ChildNode::set_UpStreamNode( const RemoteNode * iupstream_node)
{
    upstream_node = iupstream_node;
}

inline const RemoteNode * ChildNode::get_UpStreamNode( void )
{
    return upstream_node;
}

} // namespace MRN

#endif /* __childnode_h */
