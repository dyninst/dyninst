#if !defined(__internalnode_h)
#define __internalnode_h 1

#include <string>
#include <list>

#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/ParentNode.h"
#include "mrnet/src/ChildNode.h"
#include "mrnet/src/Message.h"
#include "mrnet/src/pthread_sync.h"

namespace MRN
{

class InternalNode: public ParentNode, public ChildNode,
		       public CommunicationNode
{
 protected:
  virtual int deliverLeafInfoResponse( Packet* pkt );
  virtual int deliverConnectLeavesResponse( Packet* pkt );

 public:
  InternalNode(std::string hostname, unsigned short port,
                  std::string _phostname, unsigned short _pport, unsigned short _pid);
  virtual ~InternalNode(void);
  void waitLoop();
  int send_newSubTreeReport(bool status);
  virtual int proc_PacketsFromUpStream(std::list <Packet *> &);
  virtual int proc_DataFromUpStream(Packet *);
  virtual int proc_PacketsFromDownStream(std::list <Packet *> &);
  virtual int proc_DataFromDownStream(Packet *);
};

} // namespace MRN
#endif /* __internalnode_h */
