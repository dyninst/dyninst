#if !defined(NetworkGraph_h)
#define NetworkGraph_h

#include <vector>
#include <map>

#include <string>


#include "mrnet/src/Errors.h"
#include "mrnet/src/Message.h"
#include "mrnet/h/MR_Network.h"
using namespace MRN;

class NetworkGraph;

class NetworkNode{
  friend class NetworkGraph;
 private:
  unsigned int id;
  std::string hostname;
  unsigned short port;
  NetworkGraph * network_graph;
  std::vector <NetworkNode *> children;
  bool _visited;

 public:
  NetworkNode(char * _hostname, unsigned short port);
  std::string get_HostName();
  unsigned short get_Port();
  void add_Child(NetworkNode *);
  void visit();
  bool visited();
};

class SerialGraph{
 private:
  std::string byte_array;
  unsigned int buf_idx;
  unsigned int num_nodes;
  unsigned int num_backends;
  void find_NumNodes();
  void find_NumBackends();

 public:
  SerialGraph();
  SerialGraph(const char *);
  SerialGraph(std::string);
  void add_BackEnd(std::string, unsigned short, unsigned short);
  void add_SubTreeRoot(std::string, unsigned short);
  void end_SubTree();
  std::string get_ByteArray();
  void print();

  std::string get_RootName();
  unsigned short get_RootPort();
  void set_ToFirstChild();
  SerialGraph * get_NextChild();
  bool has_children();
  int get_Id();
  unsigned int get_NumNodes();
  unsigned int get_NumBackends();
};

class NetworkGraph{
 private:
  NetworkNode * root;
  std::map<std::string, NetworkNode*>* nodes; 
  bool graph_checked;
  unsigned int visited_nodes;
  bool _has_cycle;

  SerialGraph serial_graph;
  std::vector <EndPoint *> * endpoints;

  void preorder_traversal(NetworkNode *);

 public:
  NetworkGraph();
  void set_Root(NetworkNode * );
  std::vector <EndPoint *> * get_EndPoints();
  NetworkNode * get_Root();
  NetworkNode * find_Node(char *, unsigned short);
  bool has_cycle();
  bool fully_connected();
  int get_Size();
  SerialGraph & get_SerialGraph();
  void add_Node(NetworkNode*);
};

#endif /* NetworkGraph_h */
