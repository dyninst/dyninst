#if !defined(__backendnode_h)
#define __backendnode_h 1

#include <string>

#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/ChildNode.h"
#include "mrnet/src/Message.h"

class BackEndNode: public ChildNode, public CommunicationNode {
 private:
    unsigned int backend_id;    // id in the backend namespace
                                // TODO does this duplicate "port" in ChildNode?
 public:
  BackEndNode(std::string _hostname, unsigned short _backend_id,
                 std::string _phostname, unsigned short _pport, 
                 unsigned short _pid);
  virtual ~BackEndNode(void);
  virtual int proc_PacketsFromUpStream(std::list <Packet *> &);
  virtual int proc_DataFromUpStream(Packet *);
  int send(Packet *);
  int flush();
  int recv( bool blocking=true );
};

#endif /* __backendnode_h */
