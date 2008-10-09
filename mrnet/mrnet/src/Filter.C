/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <vector>

#include "mrnet/MRNet.h"

#include "CommunicationNode.h"
#include "Filter.h"
#include "ParentNode.h"
#include "PeerNode.h"
#include "utils.h"

#include "xplat/SharedObject.h"

using namespace std;

namespace MRN
{

/*======================================*
 *    Filter Class Definition        *
 *======================================*/
map< unsigned short, void (*)() > Filter::FilterFuncs;
map< unsigned short, void (*)() > Filter::GetStateFuncs;
map< unsigned short, string > Filter::FilterFmts;
int FilterCounter::count=0;
static FilterCounter fc;

Filter::Filter(unsigned short iid)
    : _id(iid), _filter_state(NULL), _params(Packet::NullPacket)
{
    _filter_func =
        (void (*)(const vector<PacketPtr>&, vector<PacketPtr>&, vector<PacketPtr>&, void **, PacketPtr& ))
        FilterFuncs[iid];

    _get_state_func = ( PacketPtr (*)( void **, int ) ) GetStateFuncs[iid];

    _fmt_str = FilterFmts[iid];
}

Filter::~Filter(  )
{
}

int Filter::push_Packets( vector< PacketPtr >& ipackets,
                          vector< PacketPtr >& opackets,
                          vector< PacketPtr >& opackets_reverse )
{
    mrn_dbg_func_begin();

    _mutex.Lock();
    
    if( _filter_func == NULL ) {  //do nothing
        opackets = ipackets;
        ipackets.clear( );
        mrn_dbg( 3, mrn_printf(FLF, stderr, "NULL FILTER: returning %d packets\n",
                               opackets.size( ) ));
        _mutex.Unlock();
        return 0;
    }

    //TODO: put exception block to catch user error
    _filter_func( ipackets, opackets, opackets_reverse, &_filter_state, _params );
    ipackets.clear( );
    
    _mutex.Unlock();

    mrn_dbg_func_end();
    return 0;
}

PacketPtr Filter::get_FilterState( int istream_id )
{
    mrn_dbg_func_begin();

    if( _get_state_func == NULL ){
        return Packet::NullPacket;
    }

    PacketPtr packet( _get_state_func( &_filter_state, istream_id ) );

    mrn_dbg_func_end();
    return packet;
}

void Filter::set_FilterParams( PacketPtr iparams )
{
   mrn_dbg_func_begin();
   _params = iparams;
   mrn_dbg_func_end();
}

int Filter::load_FilterFunc( const char *iso_file, const char *ifunc_name )
{
    XPlat::SharedObject* so_handle = NULL;
    void (*filter_func_ptr)()=NULL;
    void (*state_func_ptr)()=NULL;
    const char **fmt_str_ptr=NULL;
    string func_fmt_str = ifunc_name;
    func_fmt_str += "_format_string";
    string state_func_name = ifunc_name;
    state_func_name += "_get_state";

    mrn_dbg_func_begin();

    so_handle = XPlat::SharedObject::Load( iso_file );
    if( so_handle == NULL ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "XPlat::SharedObject::Load(\"%s\"): %s\n",
                 iso_file, XPlat::SharedObject::GetErrorString() ));
        return -1;
    }

    //find where the filter function is loaded
    filter_func_ptr = (void(*)())so_handle->GetSymbol( ifunc_name );
    if( filter_func_ptr == NULL ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr,
                               "XPlat::SharedObject::GetSymbol(\"%s\"): %s\n",
                               ifunc_name, XPlat::SharedObject::GetErrorString() ));

        delete so_handle;
        return -1;
    }

    //find where the filter state function is loaded
    //we don't test filter state function ptr because it doesn't have to exist
    state_func_ptr = (void(*)())so_handle->GetSymbol( state_func_name.c_str() );

    //find where the filter state format string is loaded
    fmt_str_ptr = ( const char ** )so_handle->GetSymbol( func_fmt_str.c_str() );
    if( fmt_str_ptr == NULL ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr,
                               "XPlat::SharedObject::GetSymbol(\"%s\"): %s\n",
                               func_fmt_str.c_str(),
                               XPlat::SharedObject::GetErrorString()));
        delete so_handle;
        return -1;
    }

    mrn_dbg_func_end();
    return  (int)register_Filter( filter_func_ptr, state_func_ptr, *fmt_str_ptr );
}

unsigned short Filter::register_Filter( void (*ifilter_func)( ),
                                        void (*istate_func)( ),
                                        const char *ifmt )
{
    static unsigned short next_filter_id=0; 
    unsigned short cur_filter_id=next_filter_id;
    next_filter_id++;

    //FilterFuncs[cur_filter_id] = (void(*)())ifilter_func;
    //StateFuncs[cur_filter_id] = (void(*)())istate_func;

    FilterFuncs[cur_filter_id] = ifilter_func;
    GetStateFuncs[cur_filter_id] = istate_func;
    FilterFmts[cur_filter_id] = ifmt;

    return cur_filter_id;
}

} // namespace MRN
