#if !defined(__streammanager_h)
#define __streammanager_h

#include <vector>
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
    Filter * downstream_aggregator;
    Filter * upstream_aggregator;
    Filter * sync;

    RemoteNode * upstream_node;
    std::list <RemoteNode *> downstream_nodes;

 public:
    static std::map<unsigned int, StreamManager*> allStreamManagers;

    StreamManager(int stream_id,
                  const std::list <RemoteNode *> &_downstream,
                  int sync_id, int ds_agg_id, int us_agg_id );
    ~StreamManager();
    int push_packet(Packet&, std::vector<Packet> &,
                    bool going_upstream=true);

    const std::list<RemoteNode*>&
        get_downstreamNodes( void ) const   { return downstream_nodes; }
    RemoteNode* get_upstreamNode( void )    { return upstream_node; }
};

} // namespace MRN

#endif /* __streammanager_h  */
