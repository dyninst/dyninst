#if !defined(__streammanager_h)
#define __streammanager_h

#include <list>
#include "mrnet/src/Filter.h"

namespace MRN
{

class RemoteNode;
class StreamManager{
  friend class ParentNode;
  friend class InternalNode;
 private:
  unsigned short stream_id;
  Filter * sync;
  Filter * ds_agg;
  Filter * us_agg;

  RemoteNode * upstream_node;
  std::list <RemoteNode *> downstream_nodes;

 public:
  StreamManager(int stream_id, std::list <RemoteNode *> &_downstream,
                int sync_id, int ds_agg_id, int us_agg_id );
  int push_packet(Packet *, std::list<Packet *> &, bool going_upstream);
};

} // namespace MRN

#endif /* __streammanager_h  */
