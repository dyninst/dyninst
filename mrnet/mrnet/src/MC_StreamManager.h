#if !defined(__mc_streammanager_h)
#define __mc_streammanager_h

#include <list>
#include "mrnet/src/MC_Filter.h"

class MC_RemoteNode;
class MC_StreamManager{
  friend class MC_ParentNode;
  friend class MC_InternalNode;
 private:
  unsigned short stream_id;
  MC_Filter * aggregator;
  MC_Filter * sync;

  MC_RemoteNode * upstream_node;
  std::list <MC_RemoteNode *> downstream_nodes;

 public:
  MC_StreamManager(int stream_id, int filter_id,
		   std::list <MC_RemoteNode *> &_downstream);
  int push_packet(MC_Packet *);
};

#endif /* __mc_streammanager_h  */
