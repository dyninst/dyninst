#include "mrnet/src/MC_StreamManager.h"

MC_StreamManager::MC_StreamManager(int sid, int fid, MC_RemoteNode * _upstream,
		   std::list <MC_RemoteNode *> &_downstream)
  :stream_id(sid), upstream_node(_upstream), downstream_nodes(_downstream)
{
  aggregator = (MC_Filter *) new MC_Aggregator(fid);
  sync = (MC_Filter *) new MC_Synchronizer(SYNC_WAITFORALL, downstream_nodes);
}
