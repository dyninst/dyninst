#ifndef __communication_node_h
#define __communication_node_h 1

#include <string>
#include "mrnet/src/MC_Errors.h"

enum MC_ProtocolTags{MC_NEW_SUBTREE_PROT=200, MC_DEL_SUBTREE_PROT,
                     MC_RPT_SUBTREE_PROT,
                     MC_NEW_APPLICATION_PROT, MC_DEL_APPLICATION_PROT,
                     MC_NEW_STREAM_PROT, MC_DEL_STREAM_PROT,
                     MC_DATA_PROT};

class MC_CommunicationNode: public MC_Error{
 protected:
  std::string hostname;
  unsigned short port;
  unsigned short config_port;

 public:
  MC_CommunicationNode(std::string &_hostname, unsigned short _port);
  MC_CommunicationNode(std::string &_hostname, unsigned short _port,
                       unsigned short _id);
  std::string get_HostName();
  unsigned short get_Port();
};

#endif /* __mc_communicationnode_h */
