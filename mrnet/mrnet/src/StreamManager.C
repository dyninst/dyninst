#include "mrnet/src/StreamManager.h"
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/utils.h"

namespace MRN
{

StreamManager::StreamManager(int sid, std::list <RemoteNode *> &_downstream,
                                int sync_id, int ds_agg_id, int us_agg_id)
  : stream_id(sid),
    sync( new Synchronizer(sync_id, _downstream)),
    ds_agg( new Aggregator( ds_agg_id ) ),
    us_agg( new Aggregator( us_agg_id ) ),
    upstream_node( NULL ),
    downstream_nodes(_downstream)
{
}

int StreamManager::push_packet(Packet *packet,
                              std::list<Packet *> & out_packet_list,
                              bool going_upstream){
  std::list<Packet *> in_packet_list;

  mrn_printf(3, MCFL, stderr, "Entering StreamMgr.push_packet()\n");

  in_packet_list.push_back(packet);

  // we only allow synchronization filters on packets flowing upstream
  if( going_upstream ){
      if( sync->push_packets(in_packet_list, out_packet_list) == -1){
        mrn_printf(1, MCFL, stderr, "Sync.push_packets() failed\n");
        return -1;
      }

      if( out_packet_list.size() != 0 ){
        in_packet_list = out_packet_list;
        out_packet_list.clear();
      }
  }

  if( in_packet_list.size() > 0 )
  {
    Filter* cur_agg = (going_upstream ? us_agg : ds_agg);
    if( cur_agg->push_packets(in_packet_list, out_packet_list) == -1){
      mrn_printf(1, MCFL, stderr, "Sync.push_packets() failed\n");
      return -1;
    }
  }

  mrn_printf(3, MCFL, stderr, "Leaving stream_mgr.push_packet()\n");
  return 0;
}

} // namespace MRN

