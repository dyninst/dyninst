#if !defined(__mc_parentnode_h)
#define __mc_parentnode_h 1

#include <map>
#include <list>
#include <string>

#include "mrnet/src/MC_Message.h"
#include "mrnet/src/MC_StreamManager.h"
#include "mrnet/src/MC_Filter.h"
#include "mrnet/src/MC_RemoteNode.h"
#include "mrnet/src/pthread_sync.h"

class MC_ParentNode{
  friend class MC_Aggregator;
  friend class MC_Synchronizer;
 private:
  static std::map<unsigned int, MC_Aggregator::AggregatorSpec *> AggrSpecById;
  static std::map<unsigned int,
           void(*)(std::list<MC_Packet*>&, std::list<MC_Packet*>&,
                   std::list<MC_RemoteNode *> &, void **) > SyncById;

  string hostname;
  unsigned short port;
  unsigned short config_port;
  int listening_sock_fd;

  bool threaded;

  int wait_for_SubTreeReports();

 protected:
  enum{MC_ALLNODESREPORTED};
  std::list<MC_RemoteNode *> children_nodes;
  std::list<int> backend_descendant_nodes;
  pthread_sync subtreereport_sync;
  unsigned int num_descendants, num_descendants_reported;

  std::map<unsigned int, MC_RemoteNode *> ChildNodeByBackendId;
  pthread_sync childnodebybackendid_sync;

  std::map<unsigned int, MC_StreamManager *> StreamManagerById;
  pthread_sync streammanagerbyid_sync;

 public:
  MC_ParentNode(bool _threaded, string, unsigned short);
  virtual ~MC_ParentNode(void);

  virtual int proc_PacketsFromDownStream(std::list <MC_Packet *> &)=0;
  virtual int proc_DataFromDownStream(MC_Packet *)=0;

  int recv_PacketsFromDownStream(std::list <MC_Packet *> &packet_list);
  int send_PacketDownStream(MC_Packet *packet);
  int flush_PacketsDownStream(unsigned int stream_id);
  int flush_PacketsDownStream(void);

  int proc_newSubTree(MC_Packet *);
  int proc_delSubTree(MC_Packet *);
  int proc_newSubTreeReport(MC_Packet *);
  int proc_newStream(MC_Packet *);
  int proc_delStream(MC_Packet *);
  int proc_newApplication(MC_Packet *);
  int proc_delApplication(MC_Packet *);

  string get_HostName();
  unsigned short get_Port();
};

bool lt_RemoteNodePtr(MC_RemoteNode *p1, MC_RemoteNode *p2);
bool equal_RemoteNodePtr(MC_RemoteNode *p1, MC_RemoteNode *p2);

#endif /* __mc_parentnode_h */
