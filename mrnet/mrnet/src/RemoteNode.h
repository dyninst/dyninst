#if !defined(__mc_remotenode_h) 
#define __mc_remotenode_h 1

#include <poll.h>

#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/ChildNode.h"
#include "mrnet/src/ParentNode.h"
#include "mrnet/src/Message.h"
#include "mrnet/src/pthread_sync.h"

class MC_ChildNode;
class MC_ParentNode;
class MC_RemoteNode:public MC_CommunicationNode{
 private:
  enum {MC_MESSAGEOUT_NONEMPTY};
  bool threaded;
  MC_Message msg_in, msg_out;

  int sock_fd;
  bool _is_internal_node;
  struct pollfd poll_struct;

  int accept_Connection( int listening_sock_fd );

 public:
  static MC_ParentNode * local_parent_node;
  static MC_ChildNode * local_child_node;
  static void * recv_thread_main(void * arg);
  static void * send_thread_main(void * arg);
  pthread_t recv_thread_id, send_thread_id;
  bool _is_upstream;
  pthread_sync msg_out_sync;

  MC_RemoteNode(bool threaded, std::string &_hostname, unsigned short _port);
  MC_RemoteNode(bool threaded, std::string &_hostname, unsigned short _port,
                unsigned short _id);
  int connect();
  int new_InternalNode(int listening_sock_fd, std::string parent_hostname,
                       unsigned short parent_port, unsigned short parent_id,
                       std::string commnode);
  int new_Application(int listening_sock_fd, std::string parent_hostname,
                      unsigned short parent_port, unsigned short parent_id,
                      std::string &cmd, std::vector <std::string> &args);
  int accept_Application( int listening_sock_fd );

  int send(MC_Packet *);
  int flush();
  int recv(std::list <MC_Packet *> &); //blocking recv
  bool has_data();
  bool is_backend();
  bool is_internal();
  bool is_upstream();
};

#endif /* __mc_remotenode_h  */
