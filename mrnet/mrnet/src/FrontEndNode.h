#if !defined(__mc_frontendnode_h)
#define __mc_frontendnode_h 1

#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/ParentNode.h"
#include "mrnet/src/Message.h"

class MC_FrontEndNode: public MC_ParentNode, public MC_CommunicationNode{
 private:
    std::string commnode;
    MC_Packet* leafInfoPacket;
    MC_Packet* leavesConnectedPacket;

 protected:
  virtual int deliverLeafInfoResponse( MC_Packet* pkt );
  virtual int deliverConnectLeavesResponse( MC_Packet* pkt );

 public:
  MC_FrontEndNode(std::string _hostname, unsigned short _port);
  virtual ~MC_FrontEndNode(void);
  virtual int proc_PacketsFromDownStream(std::list <MC_Packet *> &);
  virtual int proc_DataFromDownStream(MC_Packet *);
  int send(MC_Packet *);
  int flush();
  int flush(unsigned int);
  int recv();

  MC_Packet* get_leafInfoPacket( void )     
    {
        MC_Packet* ret = leafInfoPacket;
        leafInfoPacket = NULL;
        return ret;
    }

  MC_Packet* get_leavesConnectedPacket( void )
    {
        MC_Packet* ret = leavesConnectedPacket;
        leavesConnectedPacket = NULL;
        return ret;
    }
};

#endif /* __mc_frontendnode_h */
