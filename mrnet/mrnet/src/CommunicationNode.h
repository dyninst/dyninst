#ifndef __communication_node_h
#define __communication_node_h 1

#include <string>
#include "mrnet/src/Errors.h"

enum MC_ProtocolTags{MC_NEW_SUBTREE_PROT=200, MC_DEL_SUBTREE_PROT,
                     MC_RPT_SUBTREE_PROT,
                     MC_NEW_APPLICATION_PROT, MC_DEL_APPLICATION_PROT,
                     MC_NEW_STREAM_PROT, MC_DEL_STREAM_PROT,
                     MC_DATA_PROT,
                     MC_GET_LEAF_INFO_PROT,
                     MC_CONNECT_LEAVES_PROT};

class MC_CommunicationNode: public MC_Error{
 protected:
  std::string hostname;
  unsigned short port;      // MRNet-assigned "port"
  unsigned short id;        // id, if back-end

 public:
  MC_CommunicationNode(std::string &_hostname, unsigned short _port);
  MC_CommunicationNode(std::string &_hostname, unsigned short _port,
                       unsigned short _id);
  std::string get_HostName() const  { return hostname; }
  unsigned short get_Port() const   { return port; }
  unsigned short get_Id() const     { return id; }
};

#endif /* __mc_communicationnode_h */
