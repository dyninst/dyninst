/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include "mrnet/MRNet.h"

#include "DataElement.h"
#include "PeerNode.h"
#include "ParentNode.h"
#include "ChildNode.h"
#include "utils.h"
#include "mrnet/MRNet.h"
#include "xplat/Tokenizer.h"
#include "xplat/NetUtils.h"

namespace MRN
{

PacketPtr Packet::NullPacket;

Packet::Packet( bool, // to disambiguate function prototype from public constructor 
                unsigned short _stream_id, int _tag, const char *fmt,
                va_list arg_list )
    : stream_id( _stream_id ), tag( _tag ),
      fmt_str( strdup(fmt) ), buf(NULL), inlet_rank(UnknownRank),
      destroy_data( false )
{
    PDR pdrs;
    mrn_dbg( 3, mrn_printf(FLF, stderr, "stream_id:%d, tag:%d, fmt:\"%s\"\n",
                           stream_id, tag, fmt_str ));

    src_rank = UnknownRank;

    //TODO: add exception block to catch user arg errors
    ArgList2DataElementArray( arg_list ); 

    buf_len = pdr_sizeof( ( pdrproc_t ) ( Packet::pdr_packet ), this );
    assert( buf_len );
    buf = ( char * )malloc( buf_len );
    assert( buf );

    pdrmem_create( &pdrs, buf, buf_len, PDR_ENCODE );

    if( !Packet::pdr_packet( &pdrs, this ) ) {
        error( MRN_EPACKING, "pdr_packet() failed\n" );
        return;
    }

    mrn_dbg( 3, mrn_printf(FLF, stderr,
                "Packet(%p) constructor succeeded: src:%u, stream_id:%d "
                "tag:%d, fmt:%s\n", this, src_rank, stream_id, tag, fmt_str ));
    return;
}

Packet::Packet( unsigned short istream_id, int itag, const char *ifmt_str, ... )
    : stream_id( istream_id ), tag( itag ),
      fmt_str( strdup(ifmt_str) ), buf(NULL), inlet_rank(UnknownRank),
      destroy_data( false )
{
    va_list arg_list;
    mrn_dbg(2, mrn_printf(FLF, stderr, "stream_id: %d, tag: %d, fmt:\"%s\"\n",
                          istream_id, itag, ifmt_str));
    src_rank=UnknownRank;

    va_start( arg_list, ifmt_str );
    ArgList2DataElementArray( arg_list ); 
    va_end( arg_list );

    buf_len = pdr_sizeof( ( pdrproc_t ) ( Packet::pdr_packet ), this );
    assert( buf_len );
    buf = ( char * )malloc( buf_len );
    assert( buf );

    PDR pdrs;
    pdrmem_create( &pdrs, buf, buf_len, PDR_ENCODE );

    if( !Packet::pdr_packet( &pdrs, this ) ) {
        error( MRN_EPACKING, "pdr_packet() failed\n" );
        return;
    }

    mrn_dbg( 3, mrn_printf(FLF, stderr,
                "Packet(%p) constructor succeeded: stream_id:%d "
                "tag:%d, fmt:%s\n", this, stream_id, tag, fmt_str ));
    return;
}

Packet::Packet( unsigned short istream_id, int itag, 
		const void **idata, const char *ifmt_str ) 
    : stream_id( istream_id ), tag( itag ),
      fmt_str( strdup(ifmt_str) ), buf(NULL), inlet_rank(UnknownRank),
      destroy_data( false )
{
    mrn_dbg(2, mrn_printf(FLF, stderr, "stream_id: %d, tag: %d, fmt:\"%s\"\n",
                          istream_id, itag, ifmt_str));
    src_rank=UnknownRank;

    ArgVec2DataElementArray( idata ); 

    buf_len = pdr_sizeof( ( pdrproc_t ) ( Packet::pdr_packet ), this );
    assert( buf_len );
    buf = ( char * )malloc( buf_len );
    assert( buf );

    PDR pdrs;
    pdrmem_create( &pdrs, buf, buf_len, PDR_ENCODE );

    if( !Packet::pdr_packet( &pdrs, this ) ) {
        error( MRN_EPACKING, "pdr_packet() failed\n" );
        return;
    }

    mrn_dbg( 3, mrn_printf(FLF, stderr,
                "Packet(%p) constructor succeeded: stream_id:%d "
                "tag:%d, fmt:%s\n", this, stream_id, tag, fmt_str ));
    return;
}
    

Packet::Packet( unsigned int ibuf_len, char * ibuf, Rank iinlet_rank )
    : stream_id( 0 ), fmt_str( NULL ), buf( ibuf ),
      buf_len( ibuf_len ), inlet_rank( iinlet_rank ), destroy_data( true )
{
    PDR pdrs;
    mrn_dbg_func_begin();

    if( buf_len == 0 ){  //NullPacket has buf==NULL and buf_len==0
        return;
    }

    pdrmem_create( &pdrs, buf, buf_len, PDR_DECODE );

    if( !Packet::pdr_packet( &pdrs, this ) ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "pdr_packet() failed\n" ));
        MRN_errno = MRN_EPACKING;
    }

    mrn_dbg( 3, mrn_printf(FLF, stderr,
                           "Packet(%p): src:%u, stream_id:%d tag:%d, fmt:%s\n",
                           this, src_rank, stream_id, tag, fmt_str ));
}

Packet::~Packet()
{
    data_sync.Lock();
    if( fmt_str != NULL ){
        free(fmt_str);
        fmt_str = NULL;
    }
    if( buf != NULL ){
        free(buf);
        buf = NULL;
    }

    for( unsigned int i=0; i < data_elements.size(); i++ ){
        if( destroy_data ){
            (const_cast< DataElement * >( data_elements[i] ))->set_DestroyData( true );
        }
        delete data_elements[i];
    }
    data_sync.Unlock();
}

int Packet::unpack( char const *ifmt_str, ... )
{
    va_list arg_list;

    va_start( arg_list, ifmt_str );
    int ret = ExtractVaList( ifmt_str, arg_list );
    va_end( arg_list );

    return ret;
}

const DataElement * Packet::operator[] ( unsigned int i ) const
{
    data_sync.Lock();
    const DataElement * ret = data_elements[i];
    data_sync.Unlock();
    return ret;
}

bool Packet::operator==(const Packet& p) const
{
    data_sync.Lock();
    bool ret = ( this == &p );
    data_sync.Unlock();
    return ret;
}

bool Packet::operator!=(const Packet& p) const
{
    data_sync.Lock();
    bool ret = ( this != &p );
    data_sync.Unlock();
    return ret;
}

void Packet::set_DestroyData( bool b )
{
    data_sync.Lock();
    destroy_data = b;
    data_sync.Unlock();
}

int Packet::get_Tag( void ) const
{
    data_sync.Lock();
    int ret = tag;
    data_sync.Unlock();
    return ret;
}

unsigned short Packet::get_StreamId(  ) const
{
    data_sync.Lock();
    unsigned short ret = stream_id;
    data_sync.Unlock();
    return ret;
}

const char *Packet::get_FormatString(  ) const
{
    data_sync.Lock();
    const char * ret = fmt_str;
    data_sync.Unlock();
    return ret;
}

const char *Packet::get_Buffer(  ) const
{
    data_sync.Lock();
    const char * ret = buf;
    data_sync.Unlock();
    return ret;
}

unsigned int Packet::get_BufferLen(  ) const
{
    data_sync.Lock();
    unsigned int ret = buf_len;
    data_sync.Unlock();
    return ret;
}

Rank Packet::get_InletNodeRank(  ) const
{
    data_sync.Lock();
    Rank ret = inlet_rank;
    data_sync.Unlock();
    return ret;
}

unsigned int Packet::get_NumDataElements(  ) const
{
    data_sync.Lock();
    unsigned int ret = data_elements.size(  );
    data_sync.Unlock();
    return ret;
}

const DataElement * Packet::get_DataElement( unsigned int i ) const
{
    data_sync.Lock();
    const DataElement * ret = data_elements[i];
    data_sync.Unlock();
    return ret;
}

int Packet::ExtractVaList( const char * /*fmt*/, va_list arg_list ) const
{

    mrn_dbg( 3, mrn_printf(FLF, stderr, "In ExtractVaList(%p)\n", this ));

    //TODO: proper fmt string comparison

    //TODO: add exception block here to catch user errors
    data_sync.Lock();

    DataElementArray2ArgList( arg_list );

    data_sync.Unlock();


    mrn_dbg( 3, mrn_printf(FLF, stderr, "ExtractVaList(%p) succeeded\n", this ));

    return 0;
}

int Packet::ExtractArgList( const char *ifmt_str, ... ) const
{
    va_list arg_list;

    mrn_dbg(2, mrn_printf(FLF, stderr, "fmt:\"%s\"\n", ifmt_str));

    va_start( arg_list, ifmt_str );
    DataElementArray2ArgList( arg_list );
    va_end( arg_list );

    return 0;
}




bool_t Packet::pdr_packet( PDR * pdrs, Packet * pkt )
{
    unsigned int i;
    bool_t retval = 0;
    DataElement * cur_elem=NULL;

    std::string op_str;
    if( pdrs->p_op == PDR_ENCODE )
        op_str="ENCODING";
    else if( pdrs->p_op == PDR_DECODE )
        op_str="DECODING";
    else
        op_str="FREEING";
    mrn_dbg( 3, mrn_printf(FLF, stderr, "op: %s\n", op_str.c_str() ));

    /* Process Packet Header into/out of the pdr mem */
    /********************************************************************
  Packet Buffer Format:
    _____________________________________________________
    | streamid | tag | src_rank |  fmtstr | packed_data |
    -----------------------------------------------------
    *********************************************************************/

    if( pdr_uint16( pdrs, &( pkt->stream_id ) ) == FALSE ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "pdr_uint16() failed\n" ));
        return FALSE;
    }

    if( pdr_int32( pdrs, &( pkt->tag ) ) == FALSE ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "pdr_int32() failed\n" ));
        return FALSE;
    }
    if( pdr_uint32( pdrs, &( pkt->src_rank ) ) == FALSE ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "pdr_uint32() failed\n" ));
        return FALSE;
    }
    if( pdr_wrapstring( pdrs, &( pkt->fmt_str ) ) == FALSE ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "pdr_wrapstring() failed\n" ));
        return FALSE;
    }

    if( !pkt->get_FormatString(  ) ) {
        mrn_dbg( 3, mrn_printf(FLF, stderr,
                    "No data in message. just header info\n" ));
        return TRUE;
    }

    std::string fmt = pkt->get_FormatString();
    XPlat::Tokenizer tok( fmt );
    std::string::size_type curLen;
    const char* delim = " \t\n%";

    std::string::size_type curPos = tok.GetNextToken( curLen, delim );
    i = 0;

    while( curPos != std::string::npos ) {

        assert( curLen != 0 );
        std::string cur_fmt = fmt.substr( curPos, curLen );

        if( pdrs->p_op == PDR_ENCODE ) {
            cur_elem = const_cast< DataElement *>( pkt->data_elements[i] );
        }
        else if( pdrs->p_op == PDR_DECODE ) {
            cur_elem = new DataElement;
            cur_elem->type = Fmt2Type( cur_fmt.c_str() );
        }

        switch ( cur_elem->type ) {
        case UNKNOWN_T:
            assert( 0 );
        case CHAR_T:
        case UCHAR_T:
            retval =
                pdr_uchar( pdrs, ( uchar_t * ) ( &( cur_elem->val.c ) ) );
            break;

        case CHAR_ARRAY_T:
        case UCHAR_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem->val.p = NULL;
            }
            retval =
                pdr_array( pdrs, &cur_elem->val.p,
                           &( cur_elem->array_len ), INT32_MAX,
                           sizeof( uchar_t ), ( pdrproc_t ) pdr_uchar );
            break;

        case INT16_T:
        case UINT16_T:
            retval =
                pdr_uint16( pdrs, ( uint16_t * ) ( &( cur_elem->val.hd ) ) );
            break;
        case INT16_ARRAY_T:
        case UINT16_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem->val.p = NULL;
            }
            retval =
                pdr_array( pdrs, &cur_elem->val.p,
                           &( cur_elem->array_len ), INT32_MAX,
                           sizeof( uint16_t ), ( pdrproc_t ) pdr_uint16 );
            break;

        case INT32_T:
        case UINT32_T:
            retval =
                pdr_uint32( pdrs, ( uint32_t * ) ( &( cur_elem->val.d ) ) );
            break;
        case INT32_ARRAY_T:
        case UINT32_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem->val.p = NULL;
            }
            retval =
                pdr_array( pdrs, &cur_elem->val.p,
                           &( cur_elem->array_len ), INT32_MAX,
                           sizeof( uint32_t ), ( pdrproc_t ) pdr_uint32 );
            break;

        case INT64_T:
        case UINT64_T:
            retval =
                pdr_uint64( pdrs, ( uint64_t * ) ( &( cur_elem->val.ld ) ) );
            break;
        case INT64_ARRAY_T:
        case UINT64_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem->val.p = NULL;
            }
            retval = pdr_array( pdrs, &cur_elem->val.p,
                                &( cur_elem->array_len ), INT32_MAX,
                                sizeof( uint64_t ), ( pdrproc_t ) pdr_uint64 );
            break;

        case FLOAT_T:
            retval = pdr_float( pdrs, ( float * )( &( cur_elem->val.f ) ) );
            break;
        case DOUBLE_T:
            retval =
                pdr_double( pdrs, ( double * )( &( cur_elem->val.lf ) ) );
            break;
        case FLOAT_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem->val.p = NULL;
            }
            retval =
                pdr_array( pdrs, &cur_elem->val.p,
                           &( cur_elem->array_len ), INT32_MAX,
                           sizeof( float ), ( pdrproc_t ) pdr_float );
            break;
        case DOUBLE_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem->val.p = NULL;
            }
            retval =
                pdr_array( pdrs, &cur_elem->val.p,
                           &( cur_elem->array_len ), INT32_MAX,
                           sizeof( double ), ( pdrproc_t ) pdr_double );
            break;
        case STRING_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem->val.p = NULL;
            }
            retval = pdr_array( pdrs, &cur_elem->val.p,
                            &(cur_elem->array_len), INT32_MAX,
                            sizeof(char*),
                            (pdrproc_t)pdr_wrapstring );
            break;
        case STRING_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem->val.p = NULL;
            }
            retval =
                pdr_wrapstring( pdrs, ( char ** )&( cur_elem->val.p ) );

            mrn_dbg( 3, mrn_printf(FLF, stderr,
                                   "string (%p): \"%s\"\n", cur_elem->val.p, cur_elem->val.p));
            break;
        }
        if( !retval ) {
            mrn_dbg( 1, mrn_printf(FLF, stderr,
                        "pdr_xxx() failed for elem[%d] of type %d\n", i,
                        cur_elem->type ));
            return retval;
        }
        if( pdrs->p_op == PDR_DECODE ) {
            pkt->data_elements.push_back( cur_elem );
        }

        curPos = tok.GetNextToken( curLen, delim );
        i++;
    }

    mrn_dbg_func_end();
    return TRUE;
}

void Packet::ArgList2DataElementArray( va_list arg_list )
{
    mrn_dbg_func_begin();

    DataElement * cur_elem=NULL;

    std::string fmt = fmt_str;
    XPlat::Tokenizer tok( fmt );
    std::string::size_type curLen;
    const char* delim = " \t\n%";

    std::string::size_type curPos = tok.GetNextToken( curLen, delim );
    while( curPos != std::string::npos ) {

        assert( curLen != 0 );
        std::string cur_fmt = fmt.substr( curPos, curLen );

        cur_elem = new DataElement;
        cur_elem->type = Fmt2Type( cur_fmt.c_str() );
        switch ( cur_elem->type ) {
        case UNKNOWN_T:
            assert( 0 );
        case CHAR_T:
            cur_elem->val.c = ( char )va_arg( arg_list, int32_t );
            break;
        case UCHAR_T:
            cur_elem->val.uc = ( char )va_arg( arg_list, uint32_t );
            break;

        case INT16_T:
            cur_elem->val.hd = ( short int )va_arg( arg_list, int32_t );
            break;
        case UINT16_T:
            cur_elem->val.uhd = ( short int )va_arg( arg_list, uint32_t );
            break;

        case INT32_T:
            cur_elem->val.d = ( int )va_arg( arg_list, int32_t );
            break;
        case UINT32_T:
            cur_elem->val.ud = ( int )va_arg( arg_list, uint32_t );
            break;

        case INT64_T:
            cur_elem->val.ld = ( int64_t )va_arg( arg_list, int64_t );
            break;
        case UINT64_T:
            cur_elem->val.uld = ( uint64_t )va_arg( arg_list, uint64_t );
            break;

        case FLOAT_T:
            cur_elem->val.f = ( float )va_arg( arg_list, double );
            break;
        case DOUBLE_T:
            cur_elem->val.lf = ( double )va_arg( arg_list, double );
            break;

        case CHAR_ARRAY_T:
        case UCHAR_ARRAY_T:
        case INT32_ARRAY_T:
        case UINT32_ARRAY_T:
        case INT16_ARRAY_T:
        case UINT16_ARRAY_T:
        case INT64_ARRAY_T:
        case UINT64_ARRAY_T:
        case FLOAT_ARRAY_T:
        case DOUBLE_ARRAY_T:
        case STRING_ARRAY_T:
            cur_elem->val.p = va_arg( arg_list, char * );
            cur_elem->array_len =
                ( uint32_t )va_arg( arg_list, uint32_t );
            break;
        case STRING_T:
            cur_elem->val.p = va_arg( arg_list, char * );
            if( cur_elem->val.p != NULL )
               cur_elem->array_len = strlen( ( const char * )cur_elem->val.p );
            else
               cur_elem->array_len = 0;
            break;
        default:
            assert( 0 );
            break;
        }
        data_elements.push_back( cur_elem );

        curPos = tok.GetNextToken( curLen, delim );
    }

    mrn_dbg_func_end();
}

void Packet::ArgVec2DataElementArray( const void **idata )
{
    mrn_dbg_func_begin();

    DataElement * cur_elem=NULL;
    unsigned data_ndx = 0;

    std::string fmt = fmt_str;
    XPlat::Tokenizer tok( fmt );
    std::string::size_type curLen;
    const char* delim = " \t\n%";

    std::string::size_type curPos = tok.GetNextToken( curLen, delim );
    while( curPos != std::string::npos ) {

        assert( curLen != 0 );
        std::string cur_fmt = fmt.substr( curPos, curLen );

        cur_elem = new DataElement;
        cur_elem->type = Fmt2Type( cur_fmt.c_str() );
        switch ( cur_elem->type ) {
        case UNKNOWN_T:
            assert( 0 );
        case CHAR_T:
            cur_elem->val.c = *( char* )idata[data_ndx];
            break;
        case UCHAR_T:
            cur_elem->val.uc = *( char* )idata[data_ndx];
            break;

        case INT16_T:
            cur_elem->val.hd = *( short int* )idata[data_ndx];
            break;
        case UINT16_T:
            cur_elem->val.uhd = *( short int* )idata[data_ndx];
            break;

        case INT32_T:
            cur_elem->val.d = *( int* )idata[data_ndx];
            break;
        case UINT32_T:
            cur_elem->val.ud = *( int* )idata[data_ndx];
            break;

        case INT64_T:
            cur_elem->val.ld = *( int64_t* )idata[data_ndx];
            break;
        case UINT64_T:
            cur_elem->val.uld = *( uint64_t* )idata[data_ndx];
            break;

        case FLOAT_T:
            cur_elem->val.f = *( float* )idata[data_ndx];
            break;
        case DOUBLE_T:
	    cur_elem->val.lf = *( double* )idata[data_ndx];
            break;

        case CHAR_ARRAY_T:
        case UCHAR_ARRAY_T:
        case INT32_ARRAY_T:
        case UINT32_ARRAY_T:
        case INT16_ARRAY_T:
        case UINT16_ARRAY_T:
        case INT64_ARRAY_T:
        case UINT64_ARRAY_T:
        case FLOAT_ARRAY_T:
        case DOUBLE_ARRAY_T:
        case STRING_ARRAY_T:
            cur_elem->val.p = const_cast<void*>( idata[data_ndx++] );
            cur_elem->array_len = *( uint32_t* )idata[data_ndx];
            break;
        case STRING_T:
            cur_elem->val.p = const_cast<void*>( idata[data_ndx] );
            if( cur_elem->val.p != NULL )
               cur_elem->array_len = strlen( ( const char* )cur_elem->val.p );
            else
               cur_elem->array_len = 0;
            break;
        default:
            assert( 0 );
            break;
        }
        data_elements.push_back( cur_elem );

        curPos = tok.GetNextToken( curLen, delim );
	data_ndx++;
    }

    mrn_dbg_func_end();
}

void Packet::DataElementArray2ArgList( va_list arg_list ) const
{
    mrn_dbg_func_begin();
    int i = 0, array_len = 0;
    const DataElement * cur_elem=NULL;
    void *tmp_ptr, *tmp_array;


    std::string fmt = fmt_str;
    XPlat::Tokenizer tok( fmt );
    std::string::size_type curLen;
    const char* delim = " \t\n%";

    std::string::size_type curPos = tok.GetNextToken( curLen, delim );
    std::string cur_fmt;
    while( curPos != std::string::npos ) {
        assert( curLen != 0 );
        cur_fmt = fmt.substr( curPos, curLen );

        cur_elem = data_elements[i];
        assert( cur_elem->type == Fmt2Type( cur_fmt.c_str() ) );
        switch ( cur_elem->type ) {
        case UNKNOWN_T:
            assert( 0 );

        case CHAR_T: {
            char* cp = va_arg( arg_list, char * );
            *cp = cur_elem->val.c;
            break;
        }
        case UCHAR_T: {
            unsigned char* ucp = va_arg( arg_list, unsigned char * );
            *ucp = cur_elem->val.uc;
            break;
        }

        case INT16_T: {
            short int* hdp = va_arg( arg_list, short int * );
            *hdp = cur_elem->val.hd;
            break;
        }
        case UINT16_T: {
            unsigned short int* uhdp = va_arg( arg_list, unsigned short int * );
            *uhdp = cur_elem->val.uhd;
            break;
        }

        case INT32_T: {
            int* dp = va_arg( arg_list, int * );
            *dp = cur_elem->val.d;
            break;
        }
        case UINT32_T: {
            unsigned int* udp = va_arg( arg_list, unsigned int * );
            *udp = cur_elem->val.ud;
            break;
        }

        case INT64_T: {
            int64_t* ldp = va_arg( arg_list, int64_t * );
            *ldp = cur_elem->val.ld;
            break;
        }
        case UINT64_T: {
            uint64_t* uldp = va_arg( arg_list, uint64_t * );
            *uldp = cur_elem->val.uld;
            break;
        }

        case FLOAT_T: {
            float* fp = va_arg( arg_list, float * );
            *fp = cur_elem->val.f;
            break;
        }
        case DOUBLE_T: {
            double* lfp = va_arg( arg_list, double * );
            *lfp = cur_elem->val.lf;
            break;
        }

        case STRING_T: {
            const char** cpp = ( const char ** ) va_arg( arg_list, const char ** );
            *cpp = strdup( (const char *) cur_elem->val.p );
            assert( *cpp != NULL );
            break;
        }

        case CHAR_ARRAY_T:
        case UCHAR_ARRAY_T: {
            tmp_ptr = ( void * )va_arg( arg_list, void ** );
            assert( tmp_ptr != NULL );
            array_len = cur_elem->array_len * sizeof(char);
            if( array_len > 0 ) {
               tmp_array = malloc(array_len);
               assert( tmp_array != NULL );
               memcpy( tmp_array, cur_elem->val.p, array_len );
            }
            else
               tmp_array = NULL;
            *( ( const void ** )tmp_ptr ) = tmp_array;
            tmp_ptr = ( void * )va_arg( arg_list, int * );
            assert( tmp_ptr != NULL );
            *( ( int * )tmp_ptr ) = cur_elem->array_len;
            break;
        }

        case INT32_ARRAY_T:
        case UINT32_ARRAY_T: {
            tmp_ptr = ( void * )va_arg( arg_list, void ** );
            assert( tmp_ptr != NULL );
            array_len = cur_elem->array_len * sizeof(int);
            if( array_len > 0 ) {
               tmp_array = malloc(array_len);
               assert( tmp_array != NULL );
               memcpy( tmp_array, cur_elem->val.p, array_len );
            }
            else
               tmp_array = NULL;
            *( ( const void ** )tmp_ptr ) = tmp_array;
            tmp_ptr = ( void * )va_arg( arg_list, int * );
            assert( tmp_ptr != NULL );
            *( ( int * )tmp_ptr ) = cur_elem->array_len;
            break;
        }

        case INT16_ARRAY_T:
        case UINT16_ARRAY_T: {
            tmp_ptr = ( void * )va_arg( arg_list, void ** );
            assert( tmp_ptr != NULL );
            array_len = cur_elem->array_len * sizeof(short int);
            if( array_len > 0 ) {
               tmp_array = malloc(array_len);
               assert( tmp_array != NULL );
               memcpy( tmp_array, cur_elem->val.p, array_len );
            }
            else
               tmp_array = NULL;
            *( ( const void ** )tmp_ptr ) = tmp_array;
            tmp_ptr = ( void * )va_arg( arg_list, int * );
            assert( tmp_ptr != NULL );
            *( ( int * )tmp_ptr ) = cur_elem->array_len;
            break;
        }

        case INT64_ARRAY_T:
        case UINT64_ARRAY_T: {
            tmp_ptr = ( void * )va_arg( arg_list, void ** );
            assert( tmp_ptr != NULL );
            array_len = cur_elem->array_len * sizeof(int64_t);
            if( array_len > 0 ) {
               tmp_array = malloc(array_len);
               assert( tmp_array != NULL );
               memcpy( tmp_array, cur_elem->val.p, array_len );
            }
            else
               tmp_array = NULL;
            *( ( const void ** )tmp_ptr ) = tmp_array;
            tmp_ptr = ( void * )va_arg( arg_list, int * );
            assert( tmp_ptr != NULL );
            *( ( int * )tmp_ptr ) = cur_elem->array_len;
            break;
        }

        case FLOAT_ARRAY_T: {
            tmp_ptr = ( void * )va_arg( arg_list, void ** );
            assert( tmp_ptr != NULL );
            array_len = cur_elem->array_len * sizeof(float);
            if( array_len > 0 ) {
               tmp_array = malloc(array_len);
               assert( tmp_array != NULL );
               memcpy( tmp_array, cur_elem->val.p, array_len );
            }
            else
               tmp_array = NULL;
            *( ( const void ** )tmp_ptr ) = tmp_array;
            tmp_ptr = ( void * )va_arg( arg_list, int * );
            assert( tmp_ptr != NULL );
            *( ( int * )tmp_ptr ) = cur_elem->array_len;
            break;
        }

        case DOUBLE_ARRAY_T: {
            tmp_ptr = ( void * )va_arg( arg_list, void ** );
            assert( tmp_ptr != NULL );
            array_len = cur_elem->array_len * sizeof(double);
            if( array_len > 0 ) {
               tmp_array = malloc(array_len);
               assert( tmp_array != NULL );
               memcpy( tmp_array, cur_elem->val.p, array_len );
            }
            else
               tmp_array = NULL;
            *( ( const void ** )tmp_ptr ) = tmp_array;
            tmp_ptr = ( void * )va_arg( arg_list, int * );
            assert( tmp_ptr != NULL );
            *( ( int * )tmp_ptr ) = cur_elem->array_len;
            break;
        }

        case STRING_ARRAY_T: {
            tmp_ptr = ( void * )va_arg( arg_list, void ** );
            assert( tmp_ptr != NULL );
            array_len = cur_elem->array_len * sizeof(char*);
            if( array_len > 0 ) {
               tmp_array = malloc(array_len);
               assert( tmp_array != NULL );
               for( unsigned j = 0; j < cur_elem->array_len; j++ ) {
                  (( char ** ) tmp_array)[j] = strdup( ((const char **)(cur_elem->val.p))[j] );
                  assert( (( char ** ) tmp_array)[j] != NULL );
               }
            }
            else
               tmp_array = NULL;
            *( ( const void ** )tmp_ptr ) = tmp_array;
            tmp_ptr = ( void * )va_arg( arg_list, int * );
            assert( tmp_ptr != NULL );
            *( ( int * )tmp_ptr ) = cur_elem->array_len;
            break;
        }

        default:
            assert( 0 );
        }
        i++;

        curPos = tok.GetNextToken( curLen, delim );
    }

    mrn_dbg_func_end();
    return;
}

void Packet::DataElementArray2ArgVec( void **odata ) const
{
    mrn_dbg_func_begin();
    int i = 0, array_len = 0;
    unsigned data_ndx = 0;
    const DataElement * cur_elem=NULL;
    void *tmp_ptr, *tmp_array;

    std::string fmt = fmt_str;
    XPlat::Tokenizer tok( fmt );
    std::string::size_type curLen;
    const char* delim = " \t\n%";

    std::string::size_type curPos = tok.GetNextToken( curLen, delim );
    std::string cur_fmt;
    while( curPos != std::string::npos ) {
        assert( curLen != 0 );
        cur_fmt = fmt.substr( curPos, curLen );

        cur_elem = data_elements[i];
        assert( cur_elem->type == Fmt2Type( cur_fmt.c_str() ) );
        switch ( cur_elem->type ) {
        case UNKNOWN_T:
            assert( 0 );

        case CHAR_T: {
            char* cp = ( char* )odata[data_ndx];
            *cp = cur_elem->val.c;
            break;
        }
        case UCHAR_T: {
            unsigned char* ucp = ( unsigned char* )odata[data_ndx];
            *ucp = cur_elem->val.uc;
            break;
        }

        case INT16_T: {
            short int* hdp = ( short int* )odata[data_ndx];
            *hdp = cur_elem->val.hd;
            break;
        }
        case UINT16_T: {
	    unsigned short int* uhdp = ( unsigned short int* )odata[data_ndx];
            *uhdp = cur_elem->val.uhd;
            break;
        }

        case INT32_T: {
            int* dp = ( int* )odata[data_ndx];
            *dp = cur_elem->val.d;
            break;
        }
        case UINT32_T: {
	    unsigned int* udp = ( unsigned int* )odata[data_ndx];
            *udp = cur_elem->val.ud;
            break;
        }

        case INT64_T: {
	    int64_t* ldp = ( int64_t* )odata[data_ndx];
            *ldp = cur_elem->val.ld;
            break;
        }
        case UINT64_T: {
	    uint64_t* uldp = ( uint64_t* )odata[data_ndx];
            *uldp = cur_elem->val.uld;
            break;
        }

        case FLOAT_T: {
            float* fp = ( float* )odata[data_ndx];
            *fp = cur_elem->val.f;
            break;
        }
        case DOUBLE_T: {
            double* lfp = ( double* )odata[data_ndx];
            *lfp = cur_elem->val.lf;
            break;
        }

        case STRING_T: {
            const char** cpp = ( const char** ) odata[data_ndx];
            *cpp = strdup( (const char*) cur_elem->val.p );
            assert( *cpp != NULL );
            break;
        }

        case CHAR_ARRAY_T:
        case UCHAR_ARRAY_T: {
            tmp_ptr = odata[data_ndx++];
            assert( tmp_ptr != NULL );
            array_len = cur_elem->array_len * sizeof(char);
            if( array_len > 0 ) {
               tmp_array = malloc(array_len);
               assert( tmp_array != NULL );
               memcpy( tmp_array, cur_elem->val.p, array_len );
            }
            else
               tmp_array = NULL;
            *( (const void**)tmp_ptr ) = tmp_array;
            tmp_ptr = odata[data_ndx];
            assert( tmp_ptr != NULL );
            *( (int*)tmp_ptr ) = cur_elem->array_len;
            break;
        }

        case INT32_ARRAY_T:
        case UINT32_ARRAY_T: {
            tmp_ptr = odata[data_ndx++];
            assert( tmp_ptr != NULL );
            array_len = cur_elem->array_len * sizeof(int);
            if( array_len > 0 ) {
               tmp_array = malloc(array_len);
               assert( tmp_array != NULL );
               memcpy( tmp_array, cur_elem->val.p, array_len );
            }
            else
               tmp_array = NULL;
            *( (const void**)tmp_ptr ) = tmp_array;
            tmp_ptr = odata[data_ndx];
            assert( tmp_ptr != NULL );
            *( (int*)tmp_ptr ) = cur_elem->array_len;
            break;
        }

        case INT16_ARRAY_T:
        case UINT16_ARRAY_T: {
            tmp_ptr = odata[data_ndx++];
            assert( tmp_ptr != NULL );
            array_len = cur_elem->array_len * sizeof(short int);
            if( array_len > 0 ) {
               tmp_array = malloc(array_len);
               assert( tmp_array != NULL );
               memcpy( tmp_array, cur_elem->val.p, array_len );
            }
            else
               tmp_array = NULL;
            *( (const void**)tmp_ptr ) = tmp_array;
            tmp_ptr = odata[data_ndx];
            assert( tmp_ptr != NULL );
            *( (int*)tmp_ptr ) = cur_elem->array_len;
            break;
        }

        case INT64_ARRAY_T:
        case UINT64_ARRAY_T: {
	    tmp_ptr = odata[data_ndx++];
            assert( tmp_ptr != NULL );
            array_len = cur_elem->array_len * sizeof(int64_t);
            if( array_len > 0 ) {
               tmp_array = malloc(array_len);
               assert( tmp_array != NULL );
               memcpy( tmp_array, cur_elem->val.p, array_len );
            }
            else
               tmp_array = NULL;
            *( (const void**)tmp_ptr ) = tmp_array;
            tmp_ptr = odata[data_ndx];
            assert( tmp_ptr != NULL );
            *( (int*)tmp_ptr ) = cur_elem->array_len;
            break;
        }

        case FLOAT_ARRAY_T: {
	    tmp_ptr = odata[data_ndx++];
            assert( tmp_ptr != NULL );
            array_len = cur_elem->array_len * sizeof(float);
            if( array_len > 0 ) {
               tmp_array = malloc(array_len);
               assert( tmp_array != NULL );
               memcpy( tmp_array, cur_elem->val.p, array_len );
            }
            else
               tmp_array = NULL;
            *( (const void**)tmp_ptr ) = tmp_array;
            tmp_ptr = odata[data_ndx];
            assert( tmp_ptr != NULL );
            *( (int*)tmp_ptr ) = cur_elem->array_len;
            break;
        }

        case DOUBLE_ARRAY_T: {
            tmp_ptr = odata[data_ndx++];
            assert( tmp_ptr != NULL );
            array_len = cur_elem->array_len * sizeof(double);
            if( array_len > 0 ) {
               tmp_array = malloc(array_len);
               assert( tmp_array != NULL );
               memcpy( tmp_array, cur_elem->val.p, array_len );
            }
            else
               tmp_array = NULL;
            *( (const void**)tmp_ptr ) = tmp_array;
            tmp_ptr = odata[data_ndx];
            assert( tmp_ptr != NULL );
            *( (int*)tmp_ptr ) = cur_elem->array_len;
            break;
        }

        case STRING_ARRAY_T: {
            tmp_ptr = odata[data_ndx++];
            assert( tmp_ptr != NULL );
            array_len = cur_elem->array_len * sizeof(char*);
            if( array_len > 0 ) {
               tmp_array = malloc(array_len);
               assert( tmp_array != NULL );
               for( unsigned j = 0; j < cur_elem->array_len; j++ ) {
                  ((char**)tmp_array)[j] = strdup( ((const char **)(cur_elem->val.p))[j] );
                  assert( ((char**)tmp_array)[j] != NULL );
               }
            }
            else
               tmp_array = NULL;
            *( (const void**)tmp_ptr ) = tmp_array;
            tmp_ptr = odata[data_ndx++];
            assert( tmp_ptr != NULL );
            *( (int*)tmp_ptr ) = cur_elem->array_len;
            break;
        }

        default:
            assert( 0 );
        }
        i++;
	data_ndx++;

        curPos = tok.GetNextToken( curLen, delim );
    }

    mrn_dbg_func_end();
    return;
}

}                               /* namespace MRN */
