#if !defined(__childnode_h)
#define __childnode_h 1

#include <string>

#include "mrnet/src/Message.h"
#include "mrnet/src/RemoteNode.h"

class ChildNode{
 protected:
  RemoteNode * upstream_node;

 private:
  std::string hostname;
  unsigned short port;
  bool threaded;

 public:
  ChildNode(bool, std::string, unsigned short);
  virtual ~ChildNode(void);
  virtual int proc_PacketsFromUpStream(std::list <Packet *> &)=0;
  virtual int proc_DataFromUpStream(Packet *)=0;

  int recv_PacketsFromUpStream(std::list <Packet *> &packet_list);
  int send_PacketUpStream(Packet *packet);
  int flush_PacketsUpStream();

  std::string get_HostName();
  unsigned short get_Port();

  int getConnections( int** conns, unsigned int* nConns );
};

#endif /* __childnode_h */
