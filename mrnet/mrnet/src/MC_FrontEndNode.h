#if !defined(__mc_frontendnode_h)
#define __mc_frontendnode_h 1

#include "mrnet/src/MC_CommunicationNode.h"
#include "mrnet/src/MC_ParentNode.h"
#include "mrnet/src/MC_Message.h"

class MC_FrontEndNode: public MC_ParentNode, public MC_CommunicationNode{
 public:
  MC_FrontEndNode(string _hostname, unsigned short _port);
  virtual ~MC_FrontEndNode(void);
  virtual int proc_PacketsFromDownStream(std::list <MC_Packet *> &);
  virtual int proc_DataFromDownStream(MC_Packet *);
  int send(MC_Packet *);
  int flush();
  int flush(unsigned int);
  int recv();
};

#endif /* __mc_frontendnode_h */
