#ifndef __communication_node_h
#define __communication_node_h 1

#include <poll.h>
#include "common/h/String.h"
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/list.h"
#include "mrnet/src/MC_Errors.h"
#include "mrnet/src/MC_Message.h"
#include "mrnet/src/MC_NetworkGraph.h"

enum MC_ProtocolTags{MC_NEW_SUBTREE_PROT=200,
                     MC_DEL_SUBTREE_PROT, MC_RPT_SUBTREE_PROT,
                     MC_NEW_APPLICATION_PROT, MC_DEL_APPLICATION_PROT,
                     MC_NEW_STREAM_PROT, MC_DEL_STREAM_PROT,
                     MC_DATA_PROT};

class MC_RemoteNode;

class MC_CommunicationNode: public MC_Error{
 protected:
  string hostname;
  unsigned short port;

 public:
  MC_CommunicationNode();
  MC_CommunicationNode(string &_hostname, unsigned short _port=0);
  string get_HostName();
};

class MC_SerialGraph;
class MC_EndPoint;
class MC_ParentNode{
 protected:
  vector<MC_RemoteNode *> children_nodes;
  vector <int> backend_descendant_nodes;
  unsigned int num_backend_descendant_nodes_to_report;
  dictionary_hash<unsigned int, MC_RemoteNode *> ChildNodeByBackendId;
  dictionary_hash<unsigned int, vector <MC_RemoteNode *> *> ChildrenNodesByStreamId;

 public:
  MC_ParentNode();
  virtual ~MC_ParentNode(void){};
  int recv_PacketsFromDownStream(List <MC_Packet *> &);
  int proc_PacketsFromDownStream(List <MC_Packet *> &);
  int send_PacketDownStream(MC_Packet *);
  int flush_PacketsDownStream();
  int flush_PacketsDownStream(unsigned int stream_id);

  virtual int send_newSubTree(MC_SerialGraph &)=0;
  virtual int send_delSubTree()=0;
  virtual int send_newStream(int, int)=0;
  virtual int send_delStream(int)=0;
  virtual int send_newApplication(string, vector<string> args)=0;
  virtual int send_delApplication()=0;
  virtual int send_DataDownStream(MC_Packet *)=0;
  virtual int proc_newSubTreeReport(MC_Packet *)=0;
  virtual int proc_DataFromDownStream(MC_Packet *)=0;
};

class MC_ChildNode{
 protected:
  MC_RemoteNode * upstream_node;

 public:
  MC_ChildNode();
  virtual ~MC_ChildNode(void){};
  int recv_PacketsFromUpStream(List <MC_Packet *> &);
  int proc_PacketsFromUpStream(List <MC_Packet *> &);
  int send_PacketUpStream(MC_Packet *);
  int flush_PacketsUpStream();

  virtual int proc_newSubTree(MC_Packet *)=0;
  virtual int proc_delSubTree(MC_Packet *)=0;
  virtual int proc_newStream(MC_Packet *)=0;
  virtual int proc_delStream(MC_Packet *)=0;
  virtual int proc_newApplication(MC_Packet *)=0;
  virtual int proc_delApplication(MC_Packet *)=0;
  virtual int proc_DataFromUpStream(MC_Packet *)=0;
  virtual int send_DataUpStream(MC_Packet *)=0;
  virtual int send_newSubTreeReport(bool)=0;
};

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
                      string &cmd, vector <string> &args);
  int send(MC_Packet *);
  int flush();
  int recv(List <MC_Packet *> &);
  bool is_backend();
  bool is_internal();
};

class MC_InternalNode: public MC_ChildNode, public MC_ParentNode,
                       public MC_CommunicationNode
{
 private:
  int listening_sock_fd;
 public:
  MC_InternalNode(string _parent_hostname, unsigned short _parent_port);
  virtual ~MC_InternalNode(void){};

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
  virtual int send_newSubTree(MC_SerialGraph &);
  virtual int send_delSubTree();
  virtual int send_newStream(int, int);
  virtual int send_delStream(int);
  virtual int send_newApplication(string, vector<string> args);
  virtual int send_delApplication();
  virtual int send_DataDownStream(MC_Packet *);
  virtual int proc_newSubTreeReport(MC_Packet *);
  virtual int proc_DataFromDownStream(MC_Packet *);
};

class MC_FrontEndNode: public MC_ParentNode,
		       public MC_CommunicationNode{
 private:
  int listening_sock_fd;

 public:
  MC_FrontEndNode();
  virtual ~MC_FrontEndNode(void);
  int send(MC_Packet *);
  int flush();
  int flush(unsigned int);
  int recv();

  virtual int send_newSubTree(MC_SerialGraph &);
  virtual int send_delSubTree();
  virtual int send_newStream(int, int);
  virtual int send_delStream(int);
  virtual int send_newApplication(string, vector<string> args);
  virtual int send_delApplication();
  virtual int send_DataDownStream(MC_Packet *);
  virtual int proc_newSubTreeReport(MC_Packet *);
  virtual int proc_DataFromDownStream(MC_Packet *);
};

class MC_BackEndNode: public MC_ChildNode,
		      public MC_CommunicationNode{
 public:
  MC_BackEndNode(string _parent_hostname, unsigned short _parent_port);
  virtual ~MC_BackEndNode(void){};
  int send(MC_Packet *);
  int flush();
  int recv();

  //from MC_ChildNode
  virtual int proc_newSubTree(MC_Packet *);
  virtual int proc_delSubTree(MC_Packet *);
  virtual int proc_newStream(MC_Packet *);
  virtual int proc_delStream(MC_Packet *);
  virtual int proc_newApplication(MC_Packet *);
  virtual int proc_delApplication(MC_Packet *);
  virtual int proc_DataFromUpStream(MC_Packet *);
  virtual int send_DataUpStream(MC_Packet *);
  virtual int send_newSubTreeReport(bool);
};

int sort_remote_node_ptr(const void *p1, const void *p2);
#endif /* __mc_communicationnode_h */
