#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/utils.h"

namespace MRN
{

/*===========================================================*/
/*  CommunicationNode CLASS METHOD DEFINITIONS            */
/*===========================================================*/
int CommunicationNode::poll_timeout=-1;

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

std::string CommunicationNode::get_HostName() const
{
    return hostname;
}

unsigned short CommunicationNode::get_Port() const
{
    return port;
}

unsigned short CommunicationNode::get_Id() const
{
    return id;
}

void CommunicationNode::set_BlockingTimeOut(int _timeout)
{
    poll_timeout = _timeout;
}

int CommunicationNode::get_BlockingTimeOut( )
{
    return poll_timeout;
}

} // namespace MRN
