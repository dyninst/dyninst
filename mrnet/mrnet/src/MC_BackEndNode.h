#if !defined(__mc_backendnode_h)
#define __mc_backendnode_h 1

#include <string>

#include "mrnet/src/MC_CommunicationNode.h"
#include "mrnet/src/MC_ChildNode.h"
#include "mrnet/src/MC_Message.h"

class MC_BackEndNode: public MC_ChildNode, public MC_CommunicationNode {
 public:
  MC_BackEndNode(string _hostname, unsigned short _port,
                 string _phostname, unsigned short _pport, 
                 unsigned short _pid);
  virtual ~MC_BackEndNode(void);
  virtual int proc_PacketsFromUpStream(std::list <MC_Packet *> &);
  virtual int proc_DataFromUpStream(MC_Packet *);
  int send(MC_Packet *);
  int flush();
  int recv();
};

#endif /* __mc_backendnode_h */
