#ifndef mrnet_H
#define mrnet_H 1


#include "common/h/String.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/list.h"

#include "mrnet/src/MC_Errors.h"
#include "mrnet/src/MC_Message.h"
#include "mrnet/src/MC_NetworkGraph.h"
#include "mrnet/src/MC_CommunicationNode.h"

class MC_EndPoint;
class MC_Communicator;
class MC_FrontEndNode;
class MC_BackEndNode;
class MC_RemoteNode;

class MC_Network: public MC_Error {
  friend class MC_Communicator;
  friend class MC_NetworkGraph;
  friend class MC_Stream;

 private:
  static MC_Network * network;
  static MC_BackEndNode * back_end;
  vector <MC_EndPoint *> * endpoints; //BackEnds addressed by communicator
  string filename;          /* Name of topology configuration file */
  string application;       /* Name of application to launch */
  MC_NetworkGraph * graph;  /* heirarchical DAG of tree nodes */
  bool net_initialized;
  MC_FrontEndNode *front_end;

  MC_EndPoint * get_EndPoint(string _hostname, unsigned short _port);
  int parse_configfile();

 public:
  static int new_Network(const char * _filename, const char * _application);
  static void delete_Network();
  static int init_Backend(const char *hostname, const char *port);
  static int recv();
  static int send(MC_Packet *);
  static void error_str(const char *);

  MC_Network(const char * _filename, const char * _application);
  ~MC_Network();
};

class MC_Stream{
  friend class MC_Network;

 private:
  /* "Registered" streams */
  static dictionary_hash <unsigned int, MC_Stream *> streams;
  static unsigned int cur_stream_idx, next_stream_id;
  List <MC_Packet *> IncomingPacketBuffer;
  unsigned short filter_id;
  unsigned short stream_id;
  MC_Communicator * communicator;

 public:
  MC_Stream(MC_Communicator &, int _filter_id);
  MC_Stream(int stream_id, int * backends=0, int num_backends=-1, int filter_id=-1);
  static int recv(int *tag, void **buf, MC_Stream ** stream);
  static int unpack(char * buf, const char * format_str, ...);
  static MC_Stream * get_Stream(int stream_id);

  int send(int tag, const char * format_str, ...);
  int flush();
  int recv(int *tag, void **buf);
  void add_IncomingPacket(MC_Packet *);
  vector <MC_EndPoint *> * get_EndPoints();
};

class MC_Communicator{
  friend class MC_Stream;
  friend class MC_Network;

 private:
  static MC_Communicator * comm_Broadcast;
  static void create_BroadcastCommunicator(vector <MC_EndPoint *> * endpoints);
  vector <MC_RemoteNode *> downstream_nodes; //BackEnds addressed by communicator
  vector <MC_EndPoint *> endpoints;     //BackEnds addressed by communicator

 public:
  static MC_Communicator * get_BroadcastCommunicator();

  MC_Communicator();

  int add_EndPoint(string hostname, unsigned short port);
  int add_EndPoint(MC_EndPoint *);
  vector <MC_EndPoint *> * get_EndPoints();
  int size();
  string get_HostName(int); 
  unsigned short get_Port(int);
  unsigned int get_Id(int);
};

class MC_NetworkNode;
class MC_EndPoint{
 private:
  unsigned int id;
  string hostname;
  unsigned short port;
  MC_NetworkNode * downstream_node;

 public:
  MC_EndPoint(int _id, string _hostname, unsigned short _port);
  bool compare(string _hostname, unsigned short _port);
  string get_HostName();
  unsigned short get_Port();
  unsigned int get_Id();
};

#endif /* mrnet_H */
