#if !defined(__mc_internalnode_h)
#define __mc_internalnode_h 1

#include <string>
#include <list>

#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/ParentNode.h"
#include "mrnet/src/ChildNode.h"
#include "mrnet/src/Message.h"

class MC_InternalNode: public MC_ParentNode, public MC_ChildNode,
		       public MC_CommunicationNode
{
 protected:
  virtual int deliverLeafInfoResponse( MC_Packet* pkt );
  virtual int deliverConnectLeavesResponse( MC_Packet* pkt );

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
