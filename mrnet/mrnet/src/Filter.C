#include "mrnet/src/Filter.h"
#include "mrnet/src/CommunicationNode.h"
#include "mrnet/src/ParentNode.h"
#include "mrnet/src/utils.h"

#include <vector>

namespace MRN
{

/*======================================*
 *    Filter Class Definition        *
 *======================================*/
std::map < unsigned int, void *>Filter::FilterFuncById;
std::map < unsigned int, std::string > Filter::FilterFmtById;

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
    void *so_handle;
    void *func_ptr;
    const char *fmt_str;
    std::string func_fmt_str = func;
    func_fmt_str += "_format_string";

    so_handle = getSharedObjectHandle( so_file );
    if( so_handle == NULL ) {
        mrn_printf( 1, MCFL, stderr, "getSharedObjectHandle() failed.\n" );
        return -1;
    }

    func_ptr = getSymbolFromSharedObjectHandle( func, so_handle );
    if( func_ptr == NULL ) {
        mrn_printf( 1, MCFL, stderr,
                    "getSymbolFromSharedObjectHandle() failed.\n" );
        return -1;
    }

    fmt_str = ( const char * )
        getSymbolFromSharedObjectHandle( func_fmt_str.c_str(  ),
                                             so_handle );
    if( fmt_str == NULL ) {
        mrn_printf( 1, MCFL, stderr,
                    "getSymbolFromSharedObjectHandle() failed.\n" );
        return -1;
    }

    unsigned short fid;
    if( in_fid == 0 ) {
        fid = get_NextFilterFuncId(  );
    }
    else {
        fid = in_fid;
    }

    FilterFuncById[fid] = func_ptr;
    FilterFmtById[fid] = fmt_str;
    
    return ( int )fid;
}

unsigned short Filter::get_NextFilterFuncId(  )
{
    static unsigned short next_filter_func_id = 100;

    next_filter_func_id++;
    return next_filter_func_id - 1;
}

/*==========================================*
 *    Aggregator Class Definition        *
 *==========================================*/
TransFilter::TransFilter( unsigned short _filter_id )
    : Filter( _filter_id )
{
    trans_filter = ( void ( * )( std::vector < Packet >&,
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

    trans_filter( packets_in, packets_out, &local_storage );
    packets_in.clear(  );
    
    mrn_printf( 3, MCFL, stderr, "Leaving aggr.push_packets()\n" );
    return 0;
}

/*============================================*
 *    SyncFilter Class Definition        *
 *============================================*/
SyncFilter::SyncFilter( unsigned short _filter_id,
                        std::list < RemoteNode * >&nodes )
    :    Filter( _filter_id ), downstream_nodes( nodes )
{
    sync_filter = ( void ( * )
             ( std::vector < Packet >&,
               std::vector < Packet >&,
               std::list < RemoteNode * >&,
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

    fsync.lock(  );
    sync_filter( packets_in, packets_out, downstream_nodes, &local_storage );
    fsync.unlock(  );
    mrn_printf( 3, MCFL, stderr,
                    "Leaving sync.push_packets(). Returning %d packets\n",
                packets_out.size(  ) );
    return 0;
}

} // namespace MRN
