/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/utils.h"

namespace MRN
{

/*===========================================================*/
/*  CommunicationNode CLASS METHOD DEFINITIONS            */
/*===========================================================*/
#if READY
CommunicationNode::CommunicationNode(std::string &_h, Port _p, Rank _rank)
    : port(_p), rank(_rank)
{
    getNetworkName(hostname, _h);
}
#else
CommunicationNode::CommunicationNode(std::string &_h, Port _p )
    : port(_p)
{
    getNetworkName(hostname, _h);
}
#endif // READY

} // namespace MRN
