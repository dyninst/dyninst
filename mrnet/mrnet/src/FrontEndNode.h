#if !defined(__frontendnode_h)
#define __frontendnode_h 1

#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/ParentNode.h"
#include "mrnet/src/Message.h"

class FrontEndNode: public ParentNode, public CommunicationNode{
 private:
    std::string commnode;
    Packet* leafInfoPacket;
    Packet* leavesConnectedPacket;

 protected:
  virtual int deliverLeafInfoResponse( Packet* pkt );
  virtual int deliverConnectLeavesResponse( Packet* pkt );

 public:
  FrontEndNode(std::string _hostname, unsigned short _port);
  virtual ~FrontEndNode(void);
  virtual int proc_PacketsFromDownStream(std::list <Packet *> &);
  virtual int proc_DataFromDownStream(Packet *);
  int send(Packet *);
  int flush();
  int flush(unsigned int);
  int recv( bool blocking=true );

  Packet* get_leafInfoPacket( void )     
    {
        Packet* ret = leafInfoPacket;
        leafInfoPacket = NULL;
        return ret;
    }

  Packet* get_leavesConnectedPacket( void )
    {
        Packet* ret = leavesConnectedPacket;
        leavesConnectedPacket = NULL;
        return ret;
    }
};

#endif /* __frontendnode_h */
