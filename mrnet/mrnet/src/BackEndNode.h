#if !defined(__mc_backendnode_h)
#define __mc_backendnode_h 1

#include <string>

#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/ChildNode.h"
#include "mrnet/src/Message.h"

class MC_BackEndNode: public MC_ChildNode, public MC_CommunicationNode {
 private:
    unsigned int backend_id;    // id in the backend namespace
                                // TODO does this duplicate "port" in ChildNode?
 public:
  MC_BackEndNode(std::string _hostname, unsigned short _backend_id,
                 std::string _phostname, unsigned short _pport, 
                 unsigned short _pid);
  virtual ~MC_BackEndNode(void);
  virtual int proc_PacketsFromUpStream(std::list <MC_Packet *> &);
  virtual int proc_DataFromUpStream(MC_Packet *);
  int send(MC_Packet *);
  int flush();
  int recv();
};

#endif /* __mc_backendnode_h */
