/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#ifndef __communicationnode_h
#define __communicationnode_h 1

#include <string>
#include "mrnet/MRNet.h"
#include "mrnet/src/Errors.h"

namespace MRN
{

enum ProtocolTags {
    PROT_NEW_SUBTREE = FIRST_CTL_TAG,
    PROT_DEL_SUBTREE,
    PROT_RPT_SUBTREE,
    PROT_NEW_APPLICATION,
    PROT_DEL_APPLICATION,
    PROT_NEW_STREAM,
    PROT_DEL_STREAM,
    PROT_NEW_FILTER,
    PROT_DATA,
    PROT_EVENT,
    PROT_GET_LEAF_INFO,
    PROT_CONNECT_LEAVES
};

class CommunicationNode {
 protected:
    std::string hostname;
    Port port;    // MRNet-assigned "port"
#if READY
    Rank rank;    // back-end rank, if a back-end
#endif // READY
    
 public:

    ~CommunicationNode(){}
#if READY
    CommunicationNode( std::string & _hostname, Port _port,
                       Rank _rank = UnknownRank );
#else
    CommunicationNode( std::string & _hostname, Port _port );
#endif // READY
    std::string get_HostName( ) const;
    Port get_Port( ) const;

#if READY
    Rank get_Rank( ) const;
#endif // READY
};

inline Port CommunicationNode::get_Port() const
{
    return port;
}

inline std::string CommunicationNode::get_HostName() const
{
    return hostname;
}

#if READY
inline Rank CommunicationNode::get_Rank() const
{
    return rank;
}
#endif // READY

}                               // namespace MRN

#endif                          /* __communicationnode_h */
