#ifndef __communicationnode_h
#define __communicationnode_h 1

#include <string>
#include "mrnet/src/Errors.h"
#include "mrnet/h/MRNet.h"

namespace MRN
{

enum ProtocolTags {
    MRN_NEW_SUBTREE_PROT = MRN::FIRST_CTL_TAG,
    MRN_DEL_SUBTREE_PROT,
    MRN_RPT_SUBTREE_PROT,
    MRN_NEW_APPLICATION_PROT,
    MRN_DEL_APPLICATION_PROT,
    MRN_NEW_STREAM_PROT,
    MRN_DEL_STREAM_PROT,
    MRN_NEW_FILTER_PROT,
    MRN_DATA_PROT,
    MRN_GET_LEAF_INFO_PROT,
    MRN_CONNECT_LEAVES_PROT
};

class CommunicationNode: public Error {
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
