#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/utils.h"

namespace MRN
{

/*===========================================================*/
/*  CommunicationNode CLASS METHOD DEFINITIONS            */
/*===========================================================*/
CommunicationNode::CommunicationNode(std::string &_h, unsigned short _p)
  : port(_p), id(0)
{
    getNetworkName(hostname, _h);
}

CommunicationNode::CommunicationNode(std::string &_h, unsigned short _p,
                                           unsigned short _id)
  : port(_p), id(_id)
{
    getNetworkName(hostname, _h);
}

} // namespace MRN
