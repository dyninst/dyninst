/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include "CommunicationNode.h"
#include "xplat/NetUtils.h"

namespace MRN
{

/*===========================================================*/
/*  CommunicationNode CLASS METHOD DEFINITIONS            */
/*===========================================================*/
#if READY
CommunicationNode::CommunicationNode(std::string &_h, Port _p, Rank _rank)
    : port(_p), rank(_rank)
{
    XPlat::NetUtils::GetNetworkName(_h, hostname );
}
#else
CommunicationNode::CommunicationNode(std::string &_h, Port _p )
    : port(_p)
{
    XPlat::NetUtils::GetNetworkName(_h, hostname );
}
#endif // READY

} // namespace MRN
