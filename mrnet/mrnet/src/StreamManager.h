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
  Filter * aggregator;
  Filter * sync;

  RemoteNode * upstream_node;
  std::list <RemoteNode *> downstream_nodes;

 public:
  StreamManager(int stream_id, int filter_id, int sync_id,
		   std::list <RemoteNode *> &_downstream);
  int push_packet(Packet *, std::list<Packet *> &);
};

} // namespace MRN

#endif /* __streammanager_h  */
