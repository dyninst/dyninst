#ifndef __communication_node_h
#define __communication_node_h 1

#include <poll.h>
#include <list>
#include <vector>

#include <string>
#include <pthread.h>

#include "mrnet/src/MC_Errors.h"
#include "mrnet/src/MC_Message.h"
#include "mrnet/src/MC_NetworkGraph.h"
#include "mrnet/src/MC_StreamManager.h"

enum MC_ProtocolTags{MC_NEW_SUBTREE_PROT=200, MC_DEL_SUBTREE_PROT,
                     MC_RPT_SUBTREE_PROT,
                     MC_NEW_APPLICATION_PROT, MC_DEL_APPLICATION_PROT,
                     MC_NEW_STREAM_PROT, MC_DEL_STREAM_PROT,
                     MC_DATA_PROT};

class MC_CommunicationNode: public MC_Error{
 protected:
  string hostname;
  unsigned short port;

 public:
  MC_CommunicationNode();
  MC_CommunicationNode(string &_hostname, unsigned short _port=0);
  string get_HostName();
  unsigned short get_Port();
};

class MC_LocalNode;
class MC_RemoteNode:public MC_CommunicationNode{
 private:
  MC_Message msg_in;
  MC_Message msg_out;
  int sock_fd;
  bool _is_internal_node;
  bool _is_upstream;
  struct pollfd poll_struct;

  bool threaded;
  pthread_t recv_thread_id, send_thread_id;
  pthread_mutex_t msg_out_mutex;


 public:
  static MC_LocalNode * local_node;
  static void * recv_thread_main(void * arg);
  static void * send_thread_main(void * arg);

  MC_RemoteNode(string &_hostname, unsigned short _port, bool threaded);
  int connect();
  int new_InternalNode(int listening_sock_fd, string parent_hostname,
                       unsigned short parent_port);
  int new_Application(int listening_sock_fd,
                      string parent_hostname, unsigned short parent_port,
                      string &cmd, std::vector <string> &args);
  int send(MC_Packet *);
  int flush();
  int recv(std::list <MC_Packet *> &); //blocking recv
  bool has_data();
  bool is_backend();
  bool is_internal();
  bool is_upstream();
};

class MC_LocalNode: public MC_CommunicationNode{
  friend class MC_Aggregator;
  friend class MC_Synchronizer;
 protected:
  static std::map<unsigned int, MC_Aggregator::AggregatorSpec *> AggrSpecById;
  static std::map<unsigned int,
           void(*)(std::list<MC_Packet*>&, std::list<MC_Packet*>&,
                   std::list<MC_RemoteNode *> &) > SyncById;
  MC_RemoteNode * upstream_node;
  std::list<MC_RemoteNode *> children_nodes;
  int listening_sock_fd;
  std::list<int> backend_descendant_nodes;
  int num_descendants_reported;

  std::map<unsigned int, MC_RemoteNode *> ChildNodeByBackendId;
  std::map<unsigned int, MC_StreamManager *> StreamManagerById;

 public:
  MC_LocalNode(void);
  virtual ~MC_LocalNode(void);

  //for MC_ChildNode
  int recv_PacketsFromUpStream(std::list <MC_Packet *> &packet_list);
  int proc_PacketsFromUpStream(std::list <MC_Packet *> &);
  int send_PacketUpStream(MC_Packet *packet);
  int flush_PacketsUpStream();

  virtual int proc_newSubTree(MC_Packet *){ assert(0); }
  virtual int proc_delSubTree(MC_Packet *) { assert(0); }
  virtual int proc_newStream(MC_Packet *) { assert(0); }
  virtual int proc_delStream(MC_Packet *) { assert(0); }
  virtual int proc_newApplication(MC_Packet *) { assert(0); }
  virtual int proc_delApplication(MC_Packet *) { assert(0); }
  virtual int proc_DataFromUpStream(MC_Packet *) { assert(0); }
  virtual int send_newSubTreeReport(bool status) { assert(0); }
  virtual int send_DataUpStream(MC_Packet *) { assert(0); }

  //for MC_ParentNode
  int recv_PacketsFromDownStream(std::list <MC_Packet *> &packet_list);
  int proc_PacketsFromDownStream(std::list <MC_Packet *> &);
  int send_PacketDownStream(MC_Packet *packet);
  int flush_PacketsDownStream(unsigned int stream_id);
  int flush_PacketsDownStream();

  virtual int send_newSubTree(MC_SerialGraph &) { assert(0); }
  virtual int send_delSubTree() { assert(0); }
  virtual int send_newStream(int, int) { assert(0); }
  virtual int send_delStream(int) { assert(0); }
  virtual int send_newApplication(string, std::vector<string> args) {assert(0);}
  virtual int send_delApplication() { assert(0); }
  virtual int send_DataDownStream(MC_Packet *) { assert(0); }
  virtual int proc_newSubTreeReport(MC_Packet *) { assert(0); }
  virtual int proc_DataFromDownStream(MC_Packet *) { assert(0); }
};

class MC_InternalNode: public MC_LocalNode
{
 public:
  MC_InternalNode(string _parent_hostname, unsigned short _parent_port);
  virtual ~MC_InternalNode(void);

  //from MC_ChildNode
  virtual int proc_newSubTree(MC_Packet *);
  virtual int proc_delSubTree(MC_Packet *);
  virtual int proc_newStream(MC_Packet *);
  virtual int proc_delStream(MC_Packet *);
  virtual int proc_newApplication(MC_Packet *);
  virtual int proc_delApplication(MC_Packet *);
  virtual int proc_DataFromUpStream(MC_Packet *);
  virtual int send_newSubTreeReport(bool status);
  virtual int send_DataUpStream(MC_Packet *);

  //from MC_ParentNode
  //virtual int send_newSubTree(MC_SerialGraph &);
  //virtual int send_delSubTree();
  //virtual int send_newStream(int, int);
  //virtual int send_delStream(int);
  //virtual int send_newApplication(string, std::vector<string> args);
  //virtual int send_delApplication();
  //virtual int send_DataDownStream(MC_Packet *);
  virtual int proc_newSubTreeReport(MC_Packet *);
  virtual int proc_DataFromDownStream(MC_Packet *);
};

class MC_FrontEndNode: public MC_LocalNode{
 private:
  int listening_sock_fd;

 public:
  MC_FrontEndNode();
  virtual ~MC_FrontEndNode(void);
  int send(MC_Packet *);
  int flush();
  int flush(unsigned int);
  int recv();

  //for parent nodes
  virtual int send_newSubTree(MC_SerialGraph &);
  virtual int send_delSubTree();
  virtual int send_newStream(int, int);
  virtual int send_delStream(int);
  virtual int send_newApplication(string, std::vector<string> args);
  virtual int send_delApplication();
  virtual int send_DataDownStream(MC_Packet *);
  virtual int proc_newSubTreeReport(MC_Packet *);
  virtual int proc_DataFromDownStream(MC_Packet *);

  //for child nodes
  //virtual int proc_newSubTree(MC_Packet *);
  //virtual int proc_delSubTree(MC_Packet *);
  //virtual int proc_newStream(MC_Packet *);
  //virtual int proc_delStream(MC_Packet *);
  //virtual int proc_newApplication(MC_Packet *);
  //virtual int proc_delApplication(MC_Packet *);
  //virtual int proc_DataFromUpStream(MC_Packet *);
  //virtual int send_newSubTreeReport(bool status);
  //virtual int send_DataUpStream(MC_Packet *);
};

class MC_BackEndNode: public MC_LocalNode{
 public:
  MC_BackEndNode(string _parent_hostname, unsigned short _parent_port);
  virtual ~MC_BackEndNode(void){};
  int send(MC_Packet *);
  int flush();
  int recv();


  //for parent nodes
  //virtual int send_newSubTree(MC_SerialGraph &);
  //virtual int send_delSubTree();
  //virtual int send_newStream(int, int);
  //virtual int send_delStream(int);
  //virtual int send_newApplication(string, std::vector<string> args);
  //virtual int send_delApplication();
  //virtual int send_DataDownStream(MC_Packet *);
  //virtual int proc_newSubTreeReport(MC_Packet *);
  //virtual int proc_DataFromDownStream(MC_Packet *);

  //for child nodes
  //virtual int proc_newSubTree(MC_Packet *);
  //virtual int proc_delSubTree(MC_Packet *);
  //virtual int proc_newStream(MC_Packet *);
  //virtual int proc_delStream(MC_Packet *);
  //virtual int proc_newApplication(MC_Packet *);
  virtual int proc_delApplication(MC_Packet *);
  virtual int proc_DataFromUpStream(MC_Packet *);
  virtual int send_DataUpStream(MC_Packet *);
  virtual int send_newSubTreeReport(bool);
};














#if defined(UNCUT)


class MC_RemoteNode:public MC_CommunicationNode{
 private:
  MC_Message msg_in;
  MC_Message msg_out;
  int sock_fd;
  struct pollfd poll_struct;
  bool _is_internal_node;

 public:
  MC_RemoteNode(string &_hostname, unsigned short _port);
  int connect();
  int new_InternalNode(int listening_sock_fd, string parent_hostname,
                       unsigned short parent_port);
  int new_Application(int listening_sock_fd,
                      string parent_hostname, unsigned short parent_port,
                      string &cmd, std::vector <string> &args);
  int send(MC_Packet *);
  int flush();
  int recv(std::list <MC_Packet *> &);
  bool is_backend();
  bool is_internal();
};
#endif

bool lt_RemoteNodePtr(MC_RemoteNode *p1, MC_RemoteNode *p2);
bool equal_RemoteNodePtr(MC_RemoteNode *p1, MC_RemoteNode *p2);

#endif /* __mc_communicationnode_h */
