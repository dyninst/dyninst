/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#if !defined(__stream_h)
#define __stream_h 1

#include <list>
#include <vector>
#include <set>
#include <map>

#include "xplat/Monitor.h"
#include "xplat/Mutex.h"
#include "mrnet/FilterIds.h"
#include "mrnet/Packet.h"
#include "Filter.h"

namespace MRN
{

class Network;
class FrontEndNode;
class BackEndNode;

class PeerNode;
typedef boost::shared_ptr<PeerNode> PeerNodePtr;

class Stream{
    friend class Network;
    friend class FrontEndNode;
    friend class BackEndNode;
    friend class InternalNode;
    friend class ParentNode;

 public:
    Stream( Network * inetwork, int iid, Rank *ibackends, unsigned int inum_backends,
            int ius_filter_id, int isync_filter_id, int ids_filter_id );

    ~Stream();

    int send( int tag, const char *format_str, ... );
    int send( int tag, const void **data, const char *format_str);
    
    int flush( ) const;

    int recv( int *otag, PacketPtr &opacket, bool iblocking = true );
    int close( void );

    const std::set<Rank> & get_EndPoints( void ) const ;
    unsigned int get_Id( void ) const ;
    unsigned int size( void ) const ;
    bool has_Data( void );

    int set_FilterParameters( bool upstream, const char *format_str, ... ) const;
    int set_FilterParameters( const char *params_fmt, va_list params, bool upstream ) const;

    bool is_PeerClosed( Rank irank ) const;
    unsigned int num_ClosedPeers( void ) const ;
    bool is_Closed( void ) const;
    const std::set<Rank> & get_ClosedPeers( void ) const ;
    std::set < Rank > get_ChildPeers() const;

 private:
    int send_aux( int tag, const char *format_str, PacketPtr &packet );
    void add_IncomingPacket( PacketPtr );
    PacketPtr get_IncomingPacket( void );
    int push_Packet( PacketPtr, std::vector<PacketPtr> &, std::vector<PacketPtr> &, bool going_upstream );

    int send_FilterStateToParent( void ) const;
    PacketPtr get_FilterState( ) const;

    void set_FilterParams( bool, PacketPtr& ) const;


    bool remove_Node( Rank irank );
    bool recompute_ChildrenNodes( void );
    bool close_Peer( Rank irank );
    void signal_BlockedReceivers( void ) const;
    int block_ForIncomingPacket( void ) const;


    //Static Data Members
    Network * _network;
    unsigned short _id;
    int _sync_filter_id;
    Filter * _sync_filter;
    int _us_filter_id;
    Filter * _us_filter;
    int _ds_filter_id;
    Filter * _ds_filter;
    std::set < Rank > _end_points;  //end-points of stream

    //Dynamic Data Members
    bool _us_closed;
    bool _ds_closed;
    std::set< Rank > _peers; //peers in stream
    std::set< Rank > _closed_peers;
    mutable XPlat::Mutex _peers_sync;

    std::list < PacketPtr > _incoming_packet_buffer;
    mutable XPlat::Monitor _incoming_packet_buffer_sync;
    enum {PACKET_BUFFER_NONEMPTY};
};


} // namespace MRN

#endif /* __stream_h */
