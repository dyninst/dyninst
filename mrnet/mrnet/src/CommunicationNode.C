#include "mrnet/src/MC_CommunicationNode.h"
#include "mrnet/src/utils.h"

/*===========================================================*/
/*  MC_CommunicationNode CLASS METHOD DEFINITIONS            */
/*===========================================================*/
MC_CommunicationNode::MC_CommunicationNode(std::string &_h, unsigned short _p)
  :hostname(getNetworkName(_h)), port(0), config_port(_p)
{
}

MC_CommunicationNode::MC_CommunicationNode(std::string &_h, unsigned short _p,
                                           unsigned short _id)
  :hostname(getNetworkName(_h)), port(_p), config_port(_id)
{
}

std::string MC_CommunicationNode::get_HostName()
{
  return hostname;
}

unsigned short MC_CommunicationNode::get_Port()
{
  return port;
}
