#if !defined(__mc_streammanager_h)
#define __mc_streammanager_h

#include <list>
#include "mrnet/src/MC_Filter.h"

class MC_RemoteNode;
class MC_StreamManager{
  friend class MC_LocalNode;
  friend class MC_InternalNode;
  friend class MC_FrontEndNode;
 private:
  unsigned short stream_id;
  MC_Filter * aggregator;
  MC_Filter * sync;

  MC_RemoteNode * upstream_node;
  std::list <MC_RemoteNode *> downstream_nodes;

 public:
  MC_StreamManager(int stream_id, int filter_id, MC_RemoteNode * _upstream,
		   std::list <MC_RemoteNode *> &_downstream);
};

#endif /* __mc_streammanager_h  */
