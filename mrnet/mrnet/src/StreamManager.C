#include "mrnet/src/StreamManager.h"
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/utils.h"

StreamManager::StreamManager(int sid, int fid, int sync_id,
		   std::list <RemoteNode *> &_downstream)
  : stream_id(sid),
    upstream_node( NULL ),
    downstream_nodes(_downstream)
{
  aggregator = (Filter *) new Aggregator(fid);
  sync = (Filter *) new Synchronizer(sync_id, downstream_nodes);
}

int
StreamManager::push_packet(Packet *packet,
                              std::list<Packet *> & out_packet_list){
  std::list<Packet *> in_packet_list;

  mrn_printf(3, MCFL, stderr, "Entering StreamMgr.push_packet()\n");

  in_packet_list.push_back(packet);
  if( sync->push_packets(in_packet_list, out_packet_list) == -1){
    mrn_printf(1, MCFL, stderr, "Sync.push_packets() failed\n");
    return -1;
  }

  if( out_packet_list.size() != 0 ){
    in_packet_list = out_packet_list;
    out_packet_list.clear();
    if( aggregator->push_packets(in_packet_list, out_packet_list) == -1){
      mrn_printf(1, MCFL, stderr, "Sync.push_packets() failed\n");
      return -1;
    }
  }

  mrn_printf(3, MCFL, stderr, "Leaving stream_mgr.push_packet()\n");
  return 0;
}
