#if !defined(__mc_internalnode_h)
#define __mc_internalnode_h 1

#include <string>
#include <list>

#include "mrnet/src/MC_CommunicationNode.h"
#include "mrnet/src/MC_ParentNode.h"
#include "mrnet/src/MC_ChildNode.h"
#include "mrnet/src/MC_Message.h"

class MC_InternalNode: public MC_ParentNode, public MC_ChildNode,
		       public MC_CommunicationNode
{
 public:
  MC_InternalNode(std::string hostname, unsigned short port,
                  std::string _phostname, unsigned short _pport, unsigned short _pid);
  virtual ~MC_InternalNode(void);
  void waitLoop();
  int send_newSubTreeReport(bool status);
  virtual int proc_PacketsFromUpStream(std::list <MC_Packet *> &);
  virtual int proc_DataFromUpStream(MC_Packet *);
  virtual int proc_PacketsFromDownStream(std::list <MC_Packet *> &);
  virtual int proc_DataFromDownStream(MC_Packet *);
};

#endif /* __mc_internalnode_h */
