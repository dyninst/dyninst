/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#ifndef __communicationnode_h
#define __communicationnode_h 1

#include <string>
#include "mrnet/src/Errors.h"
#include "mrnet/h/MRNet.h"

namespace MRN
{

enum ProtocolTags {
    PROT_NEW_SUBTREE = MRN::FIRST_CTL_TAG,
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
    unsigned short port;    // MRNet-assigned "port"
    unsigned short id;      // id, if back-end
    
 public:
    CommunicationNode( std::string & _hostname, unsigned short _port );
    CommunicationNode( std::string & _hostname, unsigned short _port,
                       unsigned short _id );
    std::string get_HostName( ) const;
    unsigned short get_Port( ) const;
    unsigned short get_Id( ) const;
};

inline std::string CommunicationNode::get_HostName() const
{
    return hostname;
}

inline unsigned short CommunicationNode::get_Port() const
{
    return port;
}

inline unsigned short CommunicationNode::get_Id() const
{
    return id;
}

}                               // namespace MRN

#endif                          /* __communicationnode_h */
