#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/utils.h"

/*===========================================================*/
/*  CommunicationNode CLASS METHOD DEFINITIONS            */
/*===========================================================*/
CommunicationNode::CommunicationNode(std::string &_h, unsigned short _p)
  :hostname(getNetworkName(_h)), port(_p), id(0)
{
}

CommunicationNode::CommunicationNode(std::string &_h, unsigned short _p,
                                           unsigned short _id)
  :hostname(getNetworkName(_h)), port(_p), id(_id)
{
}

