#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/utils.h"

/*===========================================================*/
/*  MC_CommunicationNode CLASS METHOD DEFINITIONS            */
/*===========================================================*/
MC_CommunicationNode::MC_CommunicationNode(std::string &_h, unsigned short _p)
  :hostname(getNetworkName(_h)), port(_p), id(0)
{
}

MC_CommunicationNode::MC_CommunicationNode(std::string &_h, unsigned short _p,
                                           unsigned short _id)
  :hostname(getNetworkName(_h)), port(_p), id(_id)
{
}

