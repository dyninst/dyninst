#if !defined(__communicator_h)
#define __communicator_h 1

#include <vector>

#include "mrnet/h/MR_Network.h"
using namespace MRN;

class RemoteNode;
class CommunicatorImpl: public Communicator{
  friend class Stream;
  friend class Network;

 private:
  static CommunicatorImpl * comm_Broadcast;
  std::vector <RemoteNode *> downstream_nodes; 
  std::vector <EndPoint *> * endpoints;   //BackEnds addressed by communicator

  // used to construct broadcast communicator
  CommunicatorImpl( const std::vector<EndPoint*>& eps );

 public:

  CommunicatorImpl(void);
  CommunicatorImpl(Communicator &);
  virtual ~CommunicatorImpl(void);
  static CommunicatorImpl * get_BroadcastCommunicator(void);
  static void create_BroadcastCommunicator(std::vector <EndPoint *> *);

  const std::vector <EndPoint *> * get_EndPoints() const;
  virtual int add_EndPoint(const char * hostname, unsigned short port);
  virtual int add_EndPoint(EndPoint *);
  virtual unsigned int size() const;
  virtual const char * get_HostName(int) const; 
  virtual unsigned short get_Port(int) const;
  virtual unsigned int get_Id(int) const;
};

#endif /* __communicator_h */
