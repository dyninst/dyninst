/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

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

    static std::map<unsigned int, StreamManager*> allStreamManagersById;

 public:
    StreamManager(int stream_id,
                  const std::list <RemoteNode *> &_downstream,
                  int sync_id, int ds_agg_id, int us_agg_id );
    ~StreamManager();
    int push_packet(Packet&, std::vector<Packet> &,
                    bool going_upstream=true);

    const std::list<RemoteNode*>&
        get_downstreamNodes( void ) const   { return downstream_nodes; }
    RemoteNode* get_upstreamNode( void )    { return upstream_node; }

    static void set_StreamManagerById( unsigned int, StreamManager * );
    static StreamManager * get_StreamManagerById( unsigned int );
    static void delete_StreamManagerById( unsigned int );
    static XPlat::Mutex all_stream_managers_mutex;
};

} // namespace MRN

#endif /* __streammanager_h  */
