#include "mrnet/src/StreamManager.h"
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/utils.h"

MC_StreamManager::MC_StreamManager(int sid, int fid,
		   std::list <MC_RemoteNode *> &_downstream)
  : stream_id(sid),
    upstream_node( NULL ),
    downstream_nodes(_downstream)
{
  aggregator = (MC_Filter *) new MC_Aggregator(fid);
  sync = (MC_Filter *) new MC_Synchronizer(SYNC_WAITFORALL, downstream_nodes);
}

int
MC_StreamManager::push_packet(MC_Packet *packet,
                              std::list<MC_Packet *> & out_packet_list){
  std::list<MC_Packet *> in_packet_list;

  mc_printf(MCFL, stderr, "In stream_mgr.push_packet()\n");

  in_packet_list.push_back(packet);
  if( sync->push_packets(in_packet_list, out_packet_list) == -1){
    mc_printf(MCFL, stderr, "sync.push_packets() failed\n");
    return -1;
  }

  if( out_packet_list.size() != 0 ){
    in_packet_list = out_packet_list;
    out_packet_list.clear();
    if( aggregator->push_packets(in_packet_list, out_packet_list) == -1){
      mc_printf(MCFL, stderr, "sync.push_packets() failed\n");
      return -1;
    }
  }

  mc_printf(MCFL, stderr, "Leaving stream_mgr.push_packet()\n");
  return 0;
}
