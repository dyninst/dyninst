#if !defined(mc_networkimpl_h)
#define mc_networkimpl_h 1

#include <vector>
#include <string>


#include "mrnet/src/MC_Errors.h"
#include "mrnet/src/MC_Message.h"
#include "mrnet/src/MC_NetworkGraph.h"
#include "mrnet/src/MC_BackEndNode.h"
#include "mrnet/src/MC_FrontEndNode.h"

#include "mrnet/h/MC_Network.h"

class MC_NetworkImpl: public MC_Error {
  friend class MC_NetworkGraph;
  friend class MC_CommunicatorImpl;
  friend class MC_StreamImpl;

 private:
  string filename;          /* Name of topology configuration file */
  string application;       /* Name of application to launch */
  std::vector <MC_EndPoint *> * endpoints; //BackEnds addressed by communicator
  static MC_BackEndNode * back_end;
  MC_FrontEndNode *front_end;

  MC_EndPoint * get_EndPoint(const char * _hostname, unsigned short _port);
  int parse_configfile();

 public:
  MC_NetworkGraph * graph;  /* heirarchical DAG of tree nodes */
  static void error_str(const char *);
  static int recv(void);
  static int send(MC_Packet *);

  MC_NetworkImpl(const char * _filename, const char * _application);
  ~MC_NetworkImpl();
};

#endif /* mrnet_H */
