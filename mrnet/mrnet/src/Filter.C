/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include <vector>

#include "mrnet/src/Filter.h"
#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/ParentNode.h"
#include "mrnet/src/utils.h"
#include "mrnet/h/MRNet.h"
#include "xplat/SharedObject.h"


namespace MRN
{

/*======================================*
 *    Filter Class Definition        *
 *======================================*/
std::map < unsigned short, void *>Filter::FilterFuncById;
std::map < unsigned short, std::string > Filter::FilterFmtById;
int FilterCounter::count=0;
static FilterCounter fc;


Filter::Filter(unsigned short _filter_id)
  :filter_id(_filter_id), local_storage(NULL)
{
}

Filter::~Filter(  )
{
}

int Filter::load_FilterFunc( const char *so_file, const char *func,
                             bool transformation_filter,
                             unsigned short in_fid )
{
    XPlat::SharedObject* so_handle = NULL;
    void *func_ptr;
    const char *fmt_str;
    std::string func_fmt_str = func;
    func_fmt_str += "_format_string";

    so_handle = XPlat::SharedObject::Load( so_file );
    if( so_handle == NULL ) {
        mrn_printf( 1, MCFL, stderr, "XPlat::SharedObject::Load() failed.\n" );
        char buf[1024];
        sprintf( buf, "XPlat::SharedObject::Load(\"%s\"): %s\n",
                 so_file, XPlat::SharedObject::GetErrorString() );
        Event * event = Event::new_Event( ESYSTEM, buf );
        Event::add_Event( *event );
        delete event;
        return -1;
    }

    func_ptr = so_handle->GetSymbol( func );
    if( func_ptr == NULL ) {
        mrn_printf( 1, MCFL, stderr,
                    "XPlat::SharedObject::GetSymbol() failed.\n" );
        char buf[1024];
        sprintf( buf, "XPlat::SharedObject::GetSymbol(\"%s\"): %s\n",
                 so_file, XPlat::SharedObject::GetErrorString() );
        Event *event = Event::new_Event( ESYSTEM, buf );
        Event::add_Event( *event );
        delete event;
        delete so_handle;
        return -1;
    }

    fmt_str = ( const char * )so_handle->GetSymbol( func_fmt_str.c_str() );
    if( fmt_str == NULL ) {
        mrn_printf( 1, MCFL, stderr,
                    "XPlat::SharedObject::GetSymbol() failed.\n" );
        char buf[1024];
        sprintf( buf, "XPlat::SharedObject::GetSymbol(\"%s\"): %s\n",
                 so_file, XPlat::SharedObject::GetErrorString() );
        Event * event = Event::new_Event( ESYSTEM, buf );
        Event::add_Event( *event );
        delete event;
        delete so_handle;
        return -1;
    }

    return (int) register_Filter( (void*)func_ptr, fmt_str );
}

/*==========================================*
 *    Aggregator Class Definition        *
 *==========================================*/
TransFilter::TransFilter( unsigned short _filter_id )
    : Filter( _filter_id )
{
    trans_filter = ( void ( * )( const std::vector < Packet >&,
                                 std::vector < Packet >&,
                                 void ** ) )
        FilterFuncById[_filter_id];
    fmt_str = FilterFmtById[_filter_id];
}

TransFilter::~TransFilter(  )
{
}

int TransFilter::push_packets( std::vector < Packet >&packets_in,
                               std::vector < Packet >&packets_out )
{
    mrn_printf( 3, MCFL, stderr, "In aggr.push_packets()\n" );
    
    if( trans_filter == NULL ) {  //do nothing
        packets_out = packets_in;
        packets_in.clear(  );
        mrn_printf( 3, MCFL, stderr, "NULL FILTER: returning %d packets\n",
                    packets_out.size(  ) );
        return 0;
    }

    //TODO: put exception block to catch user error
    trans_filter( packets_in, packets_out, &local_storage );
    packets_in.clear(  );
    
    mrn_printf( 3, MCFL, stderr, "trans_filter() returned %u packets\n",
                packets_out.size() );
    mrn_printf( 3, MCFL, stderr, "Leaving aggr.push_packets()\n" );
    return 0;
}

/*============================================*
 *    SyncFilter Class Definition        *
 *============================================*/
SyncFilter::SyncFilter( unsigned short _filter_id,
                        const std::list < RemoteNode * >&nodes )
    :    Filter( _filter_id ), downstream_nodes( nodes )
{
    sync_filter = ( void ( * )
             ( std::vector < Packet >&,
               std::vector < Packet >&,
               const std::list < RemoteNode * >&,
               void ** ) )
        FilterFuncById[_filter_id];
}

SyncFilter::~SyncFilter(  )
{
}

int SyncFilter::push_packets( std::vector < Packet >&packets_in,
                              std::vector < Packet >&packets_out )
{
    mrn_printf( 3, MCFL, stderr,
                "In sync.push_packets(). Pushing %d packets\n",
                packets_in.size(  ) );

    if( sync_filter == NULL ) {  //do nothing
        packets_out = packets_in;
        packets_in.clear(  );
        mrn_printf( 3, MCFL, stderr, "NULL FILTER: returning %d packets\n",
                    packets_out.size(  ) );
        return 0;
    }

    fsync.Lock(  );
    //TODO: put exception block to catch user error
    sync_filter( packets_in, packets_out, downstream_nodes, &local_storage );
    fsync.Unlock(  );
    mrn_printf( 3, MCFL, stderr,
                    "Leaving sync.push_packets(). Returning %d packets\n",
                packets_out.size(  ) );
    return 0;
}

} // namespace MRN
