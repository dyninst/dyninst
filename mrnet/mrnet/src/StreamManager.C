/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include "mrnet/src/StreamManager.h"
#include "mrnet/src/StreamImpl.h"
#include "mrnet/src/utils.h"

namespace MRN
{

std::map<unsigned int, StreamManager*> StreamManager::allStreamManagersById;
XPlat::Mutex StreamManager::all_stream_managers_mutex;


StreamManager::StreamManager(int istream_id,
                             const RemoteNode * iupstream_node,
                             const std::list <const RemoteNode *> &idownstream_nodes,
                             int isync_id, int ids_agg_id, int ius_agg_id)
    : stream_id(istream_id),
      downstream_aggregator( new TransFilter( ids_agg_id ) ),
      upstream_aggregator ( new TransFilter( ius_agg_id ) ),
      sync( new SyncFilter(isync_id, idownstream_nodes )),
      upstream_node( iupstream_node ),
      downstream_nodes( idownstream_nodes )
{
    set_StreamManagerById( stream_id, this);
}

StreamManager::~StreamManager( )
{
    delete_StreamManagerById( stream_id );

    delete sync;
    delete downstream_aggregator;
    delete upstream_aggregator;
}

int StreamManager::push_packet(Packet& packet,
                               std::vector<Packet> & out_packets,
                               bool going_upstream)
const
{
    std::vector<Packet> in_packets;
    
    mrn_dbg(3, mrn_printf(FLF, stderr, "Entering StreamMgr.push_packet()\n"));

    in_packets.push_back(packet);

    // we only allow synchronization filters on packets flowing upstream
    if( going_upstream ){
        if( sync->push_packets(in_packets, out_packets) == -1){
            mrn_dbg(1, mrn_printf(FLF, stderr, "Sync.push_packets() failed\n"));
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
            mrn_dbg(1, mrn_printf(FLF, stderr, "Sync.push_packets() failed\n"));
            return -1;
        }
    }

    mrn_dbg(3, mrn_printf(FLF, stderr, "Leaving stream_mgr.push_packet(),"
               " returning %u packets\n", out_packets.size() ));
    return 0;
}

void StreamManager::set_StreamManagerById( unsigned int iid, StreamManager * istream_mgr )
{
    all_stream_managers_mutex.Lock();
    allStreamManagersById[iid] = istream_mgr;
    all_stream_managers_mutex.Unlock();
}

StreamManager * StreamManager::get_StreamManagerById( unsigned int iid )
{
    StreamManager * ret;
    std::map<unsigned int, StreamManager*>::iterator iter; 

    all_stream_managers_mutex.Lock();

    iter = allStreamManagersById.find( iid );
    if(  iter != allStreamManagersById.end() ){
        ret = iter->second;
    }
    else{
        ret = NULL;
    }
    all_stream_managers_mutex.Unlock();

    return ret;
}

void StreamManager::delete_StreamManagerById( unsigned int iid )
{
    std::map<unsigned int, StreamManager*>::iterator iter; 

    all_stream_managers_mutex.Lock();

    iter = allStreamManagersById.find( iid );
    if(  iter != allStreamManagersById.end() ){
        allStreamManagersById.erase( iter );
    }

    all_stream_managers_mutex.Unlock();
}
} // namespace MRN

