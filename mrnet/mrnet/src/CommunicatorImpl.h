#if !defined(__mc_communicator_h)
#define __mc_communicator_h 1

#include <vector>

#include "mrnet/h/MR_Network.h"

class MC_RemoteNode;
class MC_CommunicatorImpl: public MC_Communicator{
  friend class MC_Stream;
  friend class MC_Network;

 private:
  static MC_CommunicatorImpl * comm_Broadcast;
  std::vector <MC_RemoteNode *> downstream_nodes; 
  std::vector <MC_EndPoint *> * endpoints;   //BackEnds addressed by communicator

 public:

  MC_CommunicatorImpl(void);
  MC_CommunicatorImpl(MC_Communicator &);
  virtual ~MC_CommunicatorImpl(void);
  static MC_CommunicatorImpl * get_BroadcastCommunicator(void);
  static void create_BroadcastCommunicator(std::vector <MC_EndPoint *> *);

  std::vector <MC_EndPoint *> * get_EndPoints();
  virtual int add_EndPoint(const char * hostname, unsigned short port);
  virtual int add_EndPoint(MC_EndPoint *);
  virtual int size();
  virtual const char * get_HostName(int); 
  virtual unsigned short get_Port(int);
  virtual unsigned int get_Id(int);
};

#endif /* __mc_communicator_h */
