#include <stdio.h>

#include "mrnet/src/ChildNode.h"
#include "mrnet/src/utils.h"

/*===================================================*/
/*  ChildNode CLASS METHOD DEFINITIONS            */
/*===================================================*/
ChildNode::ChildNode(bool _threaded, std::string _hostname,
                           unsigned short _port)
  :hostname(_hostname), port(_port), threaded(_threaded)
{}

ChildNode::~ChildNode(void)
{}

int ChildNode::recv_PacketsFromUpStream(std::list <Packet *> &packet_list)
{
  mrn_printf(3, MCFL, stderr, "In recv_PacketsFromUpStream()\n");
  return upstream_node->recv(packet_list);
}

int ChildNode::send_PacketUpStream(Packet *packet)
{
  mrn_printf(3, MCFL, stderr, "In send_PacketUpStream()\n");
  return upstream_node->send(packet);
}

int ChildNode::flush_PacketsUpStream()
{
  mrn_printf(3, MCFL, stderr, "In flush_PacketsUpStream()\n");
  return upstream_node->flush();
}
std::string ChildNode::get_HostName(){
  return hostname;
}

unsigned short ChildNode::get_Port()
{
  return port;
}

int
ChildNode::getConnections( int** conns, unsigned int* nConns )
{
    int ret = 0;

    if( (conns != NULL) && (nConns != NULL) )
    {
        *nConns = 1;
        *conns = new int[*nConns];
        (*conns)[0] = upstream_node->get_sockfd();
    }
    else
    {
        ret = -1;
    }
    return ret;
}

