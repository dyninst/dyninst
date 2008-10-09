/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#ifndef __communicationnode_h
#define __communicationnode_h 1

#include <string>
#include "mrnet/Types.h"
#include "Error.h"

namespace MRN
{

enum ProtocolTags {
    PROT_SUBTREE_INFO_REQ = FirstSystemTag,
    PROT_NEW_SUBTREE,
    PROT_NEW_SUBTREE_RPT,
    PROT_DEL_SUBTREE,
    PROT_DEL_SUBTREE_ACK,
    PROT_NEW_STREAM,
    PROT_DEL_STREAM,
    PROT_CLOSE_STREAM,
    PROT_NEW_FILTER,
    PROT_SET_FILTERPARAMS_UPSTREAM,
    PROT_SET_FILTERPARAMS_DOWNSTREAM,
    PROT_EVENT,
    PROT_GET_LEAF_INFO,
    PROT_CONNECT_LEAVES,
    PROT_KILL_SELF,
    PROT_NEW_CHILD_FD_CONNECTION,
    PROT_NEW_CHILD_DATA_CONNECTION,
    PROT_FAILURE_RPT,
    PROT_RECOVERY_RPT,
    PROT_NEW_PARENT_RPT,
    PROT_TOPOLOGY_RPT,
    PROT_COLLECT_PERFDATA,

    // aliang
    PROT_GUI_INIT,
    PROT_GUI_KILL_NODE,
    PROT_GUI_CPUPERCENT
    // aliang
};

class CommunicationNode {
 protected:
    std::string _hostname;
    Port _port;
    Rank _rank;
    
 public:

    ~CommunicationNode(){}
    CommunicationNode( std::string const& ihostname, Port iport, Rank irank );

    std::string get_HostName( ) const;
    Port get_Port( ) const;
    Rank get_Rank( ) const;

    struct ltnode {
        bool operator()(const CommunicationNode* n1, const CommunicationNode* n2) const
        {
            return n1->get_Rank() < n2->get_Rank();
        }
    };

};

}                               // namespace MRN

#endif                          /* __communicationnode_h */
