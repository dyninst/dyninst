/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

/*=======================================================*/
/*            Stream CLASS METHOD DEFINITIONS            */
/*=======================================================*/

#include <stdarg.h>
#include <set>

#include "mrnet/MRNet.h"
#include "CommunicationNode.h"
#include "PeerNode.h"
#include "mrnet/Stream.h"
#include "FrontEndNode.h"
#include "BackEndNode.h"
#include "Filter.h"
#include "Router.h"

#include "utils.h"

using namespace std;

namespace MRN
{

Stream::Stream( Network * inetwork,
                int iid,
                Rank *ibackends,
                unsigned int inum_backends,
                int ius_filter_id,
                int isync_filter_id,
                int ids_filter_id )
  : _network(inetwork),
    _id( iid ),
    _sync_filter_id(isync_filter_id),
    _sync_filter( new Filter( isync_filter_id ) ),
    _us_filter_id(ius_filter_id),
    _us_filter ( new Filter(ius_filter_id) ),
    _ds_filter_id(ids_filter_id),
    _ds_filter( new Filter(ids_filter_id) ),
    _us_closed(false),
    _ds_closed(false)
{
    set< PeerNodePtr > node_set;
    mrn_dbg( 3, mrn_printf(FLF, stderr,
                           "id:%d, us_filter:%d, sync_id:%d, ds_filter:%d\n",
                           _id, _us_filter_id , _sync_filter_id, _ds_filter_id));

    _incoming_packet_buffer_sync.RegisterCondition( PACKET_BUFFER_NONEMPTY );

    //parent nodes set up relevant downstream nodes 
    if( _network->is_LocalNodeParent() ) {
        //for each backend in the stream, we add the proper forwarding node
        for( unsigned int i = 0; i < inum_backends; i++ ) {
            mrn_dbg( 3, mrn_printf(FLF, stderr, "getting outlet for backend[%d] ... ",
                                   ibackends[i] ));
            _end_points.insert( ibackends[i] );
            PeerNodePtr outlet = _network->get_OutletNode( ibackends[i] );
            if( outlet != NULL ) {
                mrn_dbg( 3, mrn_printf(FLF, stderr,
                                       "Adding Endpoint %d to stream %d\n",
                                       ibackends[i], _id ));
                _peers.insert( outlet->get_Rank() );
            }
        }
    }

    mrn_dbg_func_end();
}
    
Stream::~Stream()
{
    if( _network->is_LocalNodeFrontEnd() ) {
        PacketPtr packet( new Packet( 0, PROT_DEL_STREAM, "%d", _id ) );
        if( _network->get_LocalFrontEndNode()->proc_deleteStream( packet ) == -1 ) {
            mrn_dbg(2, mrn_printf(FLF, stderr, "proc_deleteStream() failed\n"));
        }
    }

    _network->delete_Stream( _id );
}

int Stream::send(int itag, const char *iformat_str, ...)
{
    mrn_dbg_func_begin();

    int status;
    va_list arg_list;
    va_start(arg_list, iformat_str);

    PacketPtr packet( new Packet(true, _id, itag, iformat_str, arg_list) );
    if( packet->has_Error() ){
        mrn_dbg(1, mrn_printf(FLF, stderr, "new packet() fail\n"));
        return -1;
    }
    mrn_dbg(3, mrn_printf(FLF, stderr, "packet() succeeded. Calling send_aux()\n"));

    va_end(arg_list);

    status = send_aux( itag, iformat_str, packet );

    mrn_dbg_func_end();
    return status;
}

int Stream::send(int itag, const void **idata, const char *iformat_str)
{
    mrn_dbg_func_begin();

    int status;

    PacketPtr packet( new Packet(_id, itag, idata, iformat_str) );
    if( packet->has_Error() ){
        mrn_dbg(1, mrn_printf(FLF, stderr, "new packet() fail\n"));
        return -1;
    }
    mrn_dbg(3, mrn_printf(FLF, stderr, "packet() succeeded. Calling send_aux()\n"));

    status = send_aux( itag, iformat_str, packet );

    mrn_dbg_func_end();
    return status;
}

int Stream::send_aux(int itag, char const *ifmt, PacketPtr &ipacket )
{
    Timer tagg, tsend;

    mrn_dbg(3, mrn_printf(FLF, stderr,
                          "stream_id: %d, tag:%d, fmt=\"%s\"\n", _id, itag, ifmt));

    vector<PacketPtr> ipackets, opackets, opackets_reverse;
    
    mrn_dbg_func_begin();

    ipackets.push_back(ipacket);

    Filter* cur_agg = ( _network->is_LocalNodeBackEnd() ? _us_filter : _ds_filter );

    tagg.start();
    if( cur_agg->push_Packets( ipackets, opackets, opackets_reverse ) == -1){
        mrn_dbg(1, mrn_printf(FLF, stderr, "aggr.push_packets() failed\n"));
        return -1;
    }
    tagg.stop();

    ipackets.clear();

    tsend.start();
    if( !opackets.empty() ) {
        if( _network->is_LocalNodeFrontEnd() ) {
            if( _network->send_PacketsToChildren( opackets ) == -1 ) {
                mrn_dbg(1, mrn_printf(FLF, stderr, "send_PacketsToChidlren() failed\n"));
                return -1;
            }
        }
        else {
            assert( _network->is_LocalNodeBackEnd() ) ;
            if( _network->send_PacketsToParent( opackets ) == -1 ) {
                mrn_dbg(1, mrn_printf(FLF, stderr, "send_PacketsToParent() failed\n"));
                return -1;
            }
        }
        opackets.clear();
    }
    if( !opackets_reverse.empty() ) {
        for( unsigned int i = 0; i < opackets_reverse.size( ); i++ ) {
            PacketPtr cur_packet( opackets_reverse[i] );

            mrn_dbg( 3, mrn_printf(FLF, stderr, "Put packet in stream %d\n",
                                   cur_packet->get_StreamId(  ) ));
            add_IncomingPacket( cur_packet );
        }
    }
    tsend.stop();

    mrn_dbg(5, mrn_printf(FLF, stderr, "agg_lat: %.5lf send_lat: %.5lf\n",
                          tagg.get_latency_msecs(), tsend.get_latency_msecs() ));
    mrn_dbg_func_end();
    return 0;
}

int Stream::flush() const
{
    int retval = 0;
    mrn_dbg_func_begin();

    if( _network->is_LocalNodeFrontEnd() ){
        set < Rank >::const_iterator iter;

        _peers_sync.Lock();
        for( iter=_peers.begin(); iter!=_peers.end(); iter++ ){
            mrn_dbg( 5, mrn_printf(FLF, stderr,
                                   "Calling children_nodes[%d].flush() ...\n",
                                   (*iter) ));
            PeerNodePtr cur_peer = _network->get_PeerNode( *iter );
            if( cur_peer != PeerNode::NullPeerNode ) {
                if( cur_peer->flush( ) == -1 ) {
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "flush() failed\n" ));
                    retval = -1;
                }
                mrn_dbg( 5, mrn_printf(FLF, stderr, "flush() succeeded\n" ));
            }
        }
        _peers_sync.Unlock();
    }
    else if( _network->is_LocalNodeBackEnd() ){
        mrn_dbg( 5, mrn_printf(FLF, stderr, "calling backend flush()\n" ));
        retval = _network->get_ParentNode()->flush();
    }

    mrn_dbg_func_end();
    return retval;
}

int Stream::recv(int *otag, PacketPtr & opacket, bool iblocking)
{
    mrn_dbg_func_begin();

    PacketPtr cur_packet = get_IncomingPacket();

    if( cur_packet != Packet::NullPacket ) {
        *otag = cur_packet->get_Tag();
        opacket = cur_packet;
        return 1;
    }

    //At this point, no packets already available for this stream

    if( !iblocking ){
        return 0;
    }

    //We're gonna block till this stream gets a packet
    if( _network->is_LocalNodeThreaded() ) {
        if( block_ForIncomingPacket() == -1 ) {
            mrn_dbg( 1, mrn_printf(FLF, stderr,
                                   "Stream[%u] block_ForIncomingPacket() failed.\n",
                                   _id ));
            return -1;
        }
    }
    else {
        // Not threaded, keep receiving on network till stream has packet
        while( _incoming_packet_buffer.empty() ) {
            if( _network->recv( iblocking ) == -1 ){
                mrn_dbg( 1, mrn_printf(FLF, stderr, "FrontEnd::recv() failed\n" ));
                return -1;
            }
        }
    }

    //we should have a packet now
    cur_packet = get_IncomingPacket();

    assert( cur_packet != Packet::NullPacket );
    *otag = cur_packet->get_Tag();
    opacket = cur_packet;

    mrn_dbg(5, mrn_printf(FLF, stderr, "Received packet tag: %d\n", *otag ));
    mrn_dbg_func_end();
    return 1;
}

void Stream::signal_BlockedReceivers( void ) const
{
    //wait for non-empty buffer condition
    _incoming_packet_buffer_sync.Lock();
    _incoming_packet_buffer_sync.SignalCondition( PACKET_BUFFER_NONEMPTY );
    _incoming_packet_buffer_sync.Unlock();
}

int Stream::block_ForIncomingPacket( void ) const
{
    //wait for non-empty buffer condition
    _incoming_packet_buffer_sync.Lock();
    while( _incoming_packet_buffer.empty() && !is_Closed() ){
        mrn_dbg( 5, mrn_printf(FLF, stderr, "Stream[%u](%p) blocking for a packet ...\n",
                               _id, this ));
        _incoming_packet_buffer_sync.WaitOnCondition( PACKET_BUFFER_NONEMPTY );
        mrn_dbg( 5, mrn_printf(FLF, stderr, "Stream[%u] signaled for a packet.\n",
                               _id ));
    }
    _incoming_packet_buffer_sync.Unlock();

    if( is_Closed() ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "Stream[%u] is closed.\n", _id));
        return -1;
    }

    return 0;
}

bool Stream::close_Peer( Rank irank )
{
    mrn_dbg_func_begin();

    _peers_sync.Lock();
    _closed_peers.insert( irank );
    _peers_sync.Unlock();

    //invoke sync filter to see if packets will now go thru
    vector<PacketPtr> opackets, opackets_reverse;
    bool going_toparent;

    if( _network->is_LocalNodeChild() &&
        _network->get_ParentNode()->get_Rank() == irank ) {
        going_toparent=true;
    }
    else{ 
        going_toparent=false;
    }

    if( push_Packet( Packet::NullPacket, opackets, opackets_reverse, going_toparent ) == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "push_Packet() failed\n" ));
        return false;
    }

    if( !opackets.empty() ) {
        if( _network->is_LocalNodeInternal() ) {
            //internal nodes must send packets up/down as appropriate
            if( going_toparent ) {
                if( _network->send_PacketsToParent( opackets ) == -1 ) {
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "send_PacketToChildren() failed\n" ));
                    return false;
                }
            }
            else{
                if( _network->send_PacketsToChildren( opackets ) == -1 ) {
                    mrn_dbg( 1, mrn_printf(FLF, stderr, "send_PacketToChildren() failed\n" ));
                    return false;
                }
            }
        }
        else {
            //Root/leaf nodes simply add packets to streams for application to get
            for( unsigned int i = 0; i < opackets.size( ); i++ ) {
                mrn_dbg( 3, mrn_printf(FLF, stderr, "Put packet in stream %d\n", _id ));
                add_IncomingPacket( opackets[i] );
            }
        }
    }

    mrn_dbg_func_end();
    return true;
}

const std::set<Rank> & Stream::get_ClosedPeers( void ) const
{
    return _closed_peers;
}

bool Stream::is_Closed( void ) const
{
    bool retval = false;
    _peers_sync.Lock();

    if( _network->is_LocalNodeParent() ) {
        if( _peers.size() )
            retval = ( _closed_peers.size() == _peers.size() );
    }

    _peers_sync.Unlock();

    return retval;
}

bool Stream::is_PeerClosed( Rank irank ) const
{
    bool retval = false;

    _peers_sync.Lock();
    if( _closed_peers.find( irank ) != _closed_peers.end() ) {
        retval = true;
    }
    _peers_sync.Unlock();

    return retval;
}

unsigned int Stream::num_ClosedPeers( void ) const 
{
    unsigned int retval;

    _peers_sync.Lock();
    retval = _closed_peers.size();
    _peers_sync.Unlock();

    return retval;
}

int Stream::close( )
{
    flush();

    assert(!"Who the hell is calling this?");
    PacketPtr packet( new Packet( 0, PROT_CLOSE_STREAM, "%d", _id ) );

    int retval;
    if( _network->is_LocalNodeFrontEnd() ) {
        _ds_closed = true;
        retval = _network->send_PacketToChildren( packet );
    }
    else {
        _us_closed = true;
        retval = _network->send_PacketToParent( packet );
    }
 
    return retval;
}

int Stream::push_Packet( PacketPtr ipacket,
                         vector<PacketPtr>& opackets,
                         vector<PacketPtr>& opackets_reverse,
                         bool igoing_upstream )
{
    vector<PacketPtr> ipackets;
    
    mrn_dbg_func_begin();

    if( ipacket != Packet::NullPacket ) {
        ipackets.push_back(ipacket);
    }

    // if not back-end and going upstream, sync first
    if( !_network->is_LocalNodeBackEnd() && igoing_upstream ){
        if( _sync_filter->push_Packets(ipackets, opackets, opackets_reverse ) == -1){
            mrn_dbg(1, mrn_printf(FLF, stderr, "Sync.push_packets() failed\n"));
            return -1;
        }

        if( opackets.size() != 0 ){
            ipackets = opackets;
            opackets.clear();
        }
    }

    if( ipackets.size() > 0 ) {
        Filter* cur_agg = (igoing_upstream ? _us_filter : _ds_filter );
        if( cur_agg->push_Packets(ipackets, opackets, opackets_reverse ) == -1){
            mrn_dbg(1, mrn_printf(FLF, stderr, "aggr.push_packets() failed\n"));
            return -1;
        }
    }

    mrn_dbg_func_end();
    return 0;
}

PacketPtr Stream::get_IncomingPacket( )
{
    PacketPtr cur_packet( Packet::NullPacket );

    _incoming_packet_buffer_sync.Lock();
    if( !_incoming_packet_buffer.empty() ){
        cur_packet = *( _incoming_packet_buffer.begin() );
        _incoming_packet_buffer.pop_front();
    }
    _incoming_packet_buffer_sync.Unlock();

    return cur_packet;
}

void Stream::add_IncomingPacket( PacketPtr ipacket )
{
    _incoming_packet_buffer_sync.Lock();
    _incoming_packet_buffer.push_back( ipacket );
    mrn_dbg(5, mrn_printf(FLF, stderr, "Stream[%d](%p): signaling \"got packet\"\n",
                          _id, this));
    _incoming_packet_buffer_sync.SignalCondition( PACKET_BUFFER_NONEMPTY );
    _network->signal_NonEmptyStream();
    _incoming_packet_buffer_sync.Unlock();
}

unsigned int Stream::size( void ) const
{
    return _end_points.size();
}

unsigned int Stream::get_Id( void ) const 
{
    return _id;
}

const set<Rank> & Stream::get_EndPoints( void ) const
{
    return _end_points;
}

bool Stream::has_Data( void )
{
    _incoming_packet_buffer_sync.Unlock();
    bool empty = _incoming_packet_buffer.empty();
    _incoming_packet_buffer_sync.Unlock();

    return !empty;
}

int Stream::set_FilterParameters( const char *iparams_fmt, va_list iparams, bool iupstream ) const
{
    int tag = PROT_SET_FILTERPARAMS_DOWNSTREAM;
    if( iupstream )
       tag = PROT_SET_FILTERPARAMS_UPSTREAM;
    
    PacketPtr packet( new Packet(true, _id, tag, iparams_fmt, iparams) );
    if( packet->has_Error() ) {
        mrn_dbg(1, mrn_printf(FLF, stderr, "new packet() fail\n"));
        return -1;
    }

    if( _network->is_LocalNodeFrontEnd() ) {
        ParentNode *parent = _network->get_LocalParentNode();
        if( parent == NULL ) {
            mrn_dbg(1, mrn_printf(FLF, stderr, "local parent is NULL\n"));
            return -1;
        }
        if( iupstream ) {
            if( parent->proc_UpstreamFilterParams( packet ) == -1 ) {
                mrn_dbg( 1, mrn_printf(FLF, stderr, "FrontEnd::recv() failed\n" ));
                return -1;
            }
        }
        else {
            if( parent->proc_DownstreamFilterParams( packet ) == -1 ) {
                mrn_dbg( 1, mrn_printf(FLF, stderr, "FrontEnd::recv() failed\n" ));
                return -1;
            }
        }
    }
    return 0;
}

int Stream::set_FilterParameters( bool iupstream, const char *iformat_str, ... ) const
{
    int rc;

    mrn_dbg_func_begin();
    
    va_list arg_list;
    va_start(arg_list, iformat_str);
    
    rc = set_FilterParameters( iformat_str, arg_list, iupstream );

    va_end(arg_list);

    mrn_dbg_func_end();

    return rc;
}

void Stream::set_FilterParams( bool upstream, PacketPtr &iparams ) const
{
    if( upstream ) {
        _us_filter->set_FilterParams(iparams);
    }
    else {
        _ds_filter->set_FilterParams(iparams);
    }
}

PacketPtr Stream::get_FilterState( void ) const
{
    mrn_dbg_func_begin();
    PacketPtr packet ( _us_filter->get_FilterState( _id ) );
    mrn_dbg_func_end();
    return packet;
}

int Stream::send_FilterStateToParent( void ) const
{
    mrn_dbg_func_begin();
    PacketPtr packet = get_FilterState( );

    if( packet != Packet::NullPacket ){
        _network->get_ParentNode()->send( packet );
    }

    mrn_dbg_func_end();
    return 0;
}

bool Stream::remove_Node( Rank irank )
{
    _peers_sync.Lock();
    _peers.erase( irank );
    _peers_sync.Unlock();

    return true;;
}

bool Stream::recompute_ChildrenNodes( void )
{
    set < Rank >::const_iterator iter;

    _peers_sync.Lock();
    _peers.clear();
    for( iter=_end_points.begin(); iter!=_end_points.end(); iter++ ){
        Rank cur_rank=*iter;

        mrn_dbg( 3, mrn_printf(FLF, stderr, "Resetting outlet for backend[%d] ...\n",
                               cur_rank ));

        PeerNodePtr outlet = _network->get_OutletNode( cur_rank );
        if( outlet != NULL ) {
            mrn_dbg( 3, mrn_printf(FLF, stderr,
                                   "Adding outlet[%d] for node[%d] to stream[%d].\n",
                                   outlet->get_Rank(), cur_rank, _id ));
            _peers.insert( outlet->get_Rank() );
        }
    }
    _peers_sync.Unlock();

    return true;
}

set < Rank > Stream::get_ChildPeers() const
{
    return _peers;
}

} // namespace MRN
