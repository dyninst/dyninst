#ifndef __communication_node_h
#define __communication_node_h 1

#include <string>
#include "mrnet/src/Errors.h"

enum ProtocolTags{MRN_NEW_SUBTREE_PROT=200, MRN_DEL_SUBTREE_PROT,
                     MRN_RPT_SUBTREE_PROT,
                     MRN_NEW_APPLICATION_PROT, MRN_DEL_APPLICATION_PROT,
                     MRN_NEW_STREAM_PROT, MRN_DEL_STREAM_PROT,
                     MRN_DATA_PROT,
                     MRN_GET_LEAF_INFO_PROT,
                     MRN_CONNECT_LEAVES_PROT};

class CommunicationNode: public Error{
 protected:
  std::string hostname;
  unsigned short port;      // MRNet-assigned "port"
  unsigned short id;        // id, if back-end

 public:
  CommunicationNode(std::string &_hostname, unsigned short _port);
  CommunicationNode(std::string &_hostname, unsigned short _port,
                       unsigned short _id);
  std::string get_HostName() const  { return hostname; }
  unsigned short get_Port() const   { return port; }
  unsigned short get_Id() const     { return id; }
};

#endif /* __MRN_communicationnode_h */
