#if !defined(MC_NetworkGraph_h)
#define MC_NetworkGraph_h

#include <vector>
#include <map>

#include <string>


#include "mrnet/src/Errors.h"
#include "mrnet/src/Message.h"
#include "mrnet/h/MR_Network.h"

class MC_NetworkGraph;
class MC_EndPoint;

class MC_NetworkNode{
  friend class MC_NetworkGraph;
 private:
  unsigned int id;
  std::string hostname;
  unsigned short port;
  MC_NetworkGraph * network_graph;
  std::vector <MC_NetworkNode *> children;
  bool _visited;

 public:
  MC_NetworkNode(char * _hostname, unsigned short port);
  std::string get_HostName();
  unsigned short get_Port();
  void add_Child(MC_NetworkNode *);
  void visit();
  bool visited();
};

class MC_SerialGraph{
 private:
  std::string byte_array;
  unsigned int buf_idx;
  unsigned int num_nodes;
  unsigned int num_backends;
  void find_NumNodes();
  void find_NumBackends();

 public:
  MC_SerialGraph();
  MC_SerialGraph(const char *);
  MC_SerialGraph(std::string);
  void add_BackEnd(std::string, unsigned short, unsigned short);
  void add_SubTreeRoot(std::string, unsigned short);
  void end_SubTree();
  std::string get_ByteArray();
  void print();

  std::string get_RootName();
  unsigned short get_RootPort();
  void set_ToFirstChild();
  MC_SerialGraph * get_NextChild();
  bool has_children();
  int get_Id();
  unsigned int get_NumNodes();
  unsigned int get_NumBackends();
};

class MC_NetworkGraph{
 private:
  MC_NetworkNode * root;
  std::map<std::string, MC_NetworkNode*>* nodes; 
  bool graph_checked;
  unsigned int visited_nodes;
  bool _has_cycle;

  MC_SerialGraph serial_graph;
  std::vector <MC_EndPoint *> * endpoints;

  void preorder_traversal(MC_NetworkNode *);

 public:
  MC_NetworkGraph();
  void set_Root(MC_NetworkNode * );
  std::vector <MC_EndPoint *> * get_EndPoints();
  MC_NetworkNode * get_Root();
  MC_NetworkNode * find_Node(char *, unsigned short);
  bool has_cycle();
  bool fully_connected();
  int get_Size();
  MC_SerialGraph & get_SerialGraph();
  void add_Node(MC_NetworkNode*);
};

#endif /* MC_NetworkGraph_h */
