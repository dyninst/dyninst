#include "mrnet/src/StreamManager.h"
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/utils.h"

namespace MRN
{

StreamManager::StreamManager(int sid, std::list <RemoteNode *> &_downstream,
                             int sync_id, int ds_agg_id, int us_agg_id)
    : stream_id(sid),
      downstream_aggregator( new TransFilter( ds_agg_id ) ),
      upstream_aggregator ( new TransFilter( us_agg_id ) ),
      sync( new SyncFilter(sync_id, _downstream)),
      upstream_node( NULL ),
      downstream_nodes(_downstream)
{
}

StreamManager::~StreamManager( )
{
    delete sync;
    delete downstream_aggregator;
    delete upstream_aggregator;
}

int StreamManager::push_packet(Packet& packet,
                               std::vector<Packet> & out_packets,
                               bool going_upstream){
    std::vector<Packet> in_packets;
    
    mrn_printf(3, MCFL, stderr, "Entering StreamMgr.push_packet()\n");

    in_packets.push_back(packet);

    // we only allow synchronization filters on packets flowing upstream
    if( going_upstream ){
        if( sync->push_packets(in_packets, out_packets) == -1){
            mrn_printf(1, MCFL, stderr, "Sync.push_packets() failed\n");
            return -1;
        }

        if( out_packets.size() != 0 ){
            in_packets = out_packets;
            out_packets.clear();
        }
    }

    if( in_packets.size() > 0 ) {
        Filter* cur_agg = (going_upstream ?
                           upstream_aggregator : downstream_aggregator );
        if( cur_agg->push_packets(in_packets, out_packets) == -1){
            mrn_printf(1, MCFL, stderr, "Sync.push_packets() failed\n");
            return -1;
        }
    }

    mrn_printf(3, MCFL, stderr, "Leaving stream_mgr.push_packet(),"
               " returning %u packets\n", out_packets.size() );
    return 0;
}

} // namespace MRN

