#include <stdio.h>

#include "mrnet/src/ChildNode.h"
#include "mrnet/src/utils.h"

/*===================================================*/
/*  MC_ChildNode CLASS METHOD DEFINITIONS            */
/*===================================================*/
MC_ChildNode::MC_ChildNode(bool _threaded, std::string _hostname,
                           unsigned short _port)
  :hostname(_hostname), port(_port), threaded(_threaded)
{}

MC_ChildNode::~MC_ChildNode(void)
{}

int MC_ChildNode::recv_PacketsFromUpStream(std::list <MC_Packet *> &packet_list)
{
  mc_printf(MCFL, stderr, "In recv_PacketsFromUpStream()\n");
  return upstream_node->recv(packet_list);
}

int MC_ChildNode::send_PacketUpStream(MC_Packet *packet)
{
  mc_printf(MCFL, stderr, "In send_PacketUpStream()\n");
  return upstream_node->send(packet);
}

int MC_ChildNode::flush_PacketsUpStream()
{
  mc_printf(MCFL, stderr, "In flush_PacketsUpStream()\n");
  return upstream_node->flush();
}
std::string MC_ChildNode::get_HostName(){
  return hostname;
}

unsigned short MC_ChildNode::get_Port()
{
  return port;
}
