/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include "mrnet/src/Packet.h"
#include "mrnet/src/DataElement.h"
#include "mrnet/src/utils.h"
#include "xplat/Tokenizer.h"

namespace MRN
{

Packet * Packet::NullPacket=NULL;
int Packet_counter::count=0;

/**************
 * Packet
 **************/
PacketData::PacketData( unsigned short _stream_id, int _tag, const char *fmt,
                        va_list arg_list )
    : stream_id( _stream_id ), tag( _tag ), src(NULL),
      fmt_str( strdup(fmt) ), buf(NULL), inlet_node(NULL), destroy_data( false )
{
    PDR pdrs;
    mrn_dbg( 3, mrn_printf(FLF, stderr, "In Packet(%p) constructor\n", this ));

    std::string tmp;
    getNetworkName( tmp );
    src = strdup( tmp.c_str(  ) );

    //TODO: add exception block to catch user arg errors
    ArgList2DataElementArray( arg_list ); 

    buf_len = pdr_sizeof( ( pdrproc_t ) ( PacketData::pdr_packet ), this );
    assert( buf_len );
    buf = ( char * )malloc( buf_len );
    assert( buf );

    pdrmem_create( &pdrs, buf, buf_len, PDR_ENCODE );

    if( !PacketData::pdr_packet( &pdrs, this ) ) {
        error( MRN_EPACKING, "pdr_packet() failed\n" );
        return;
    }

    mrn_dbg( 3, mrn_printf(FLF, stderr,
                "Packet(%p) constructor succeeded: src:%s, stream_id:%d "
                "tag:%d, fmt:%s\n", this, src, stream_id, tag, fmt_str ));
    return;
}

PacketData::PacketData( unsigned int ibuf_len, char * ibuf,
                        const RemoteNode *iremote_node )
    : stream_id( 0 ), src( NULL ), fmt_str( NULL ), buf( ibuf ),
      buf_len( ibuf_len ), inlet_node( iremote_node ), destroy_data( false )
{
    PDR pdrs;
    mrn_dbg( 3, mrn_printf(FLF, stderr, "In Packet(%p) constructor\n", this ));

    if( buf_len == 0 ){  //NullPacket has buf==NULL and buf_len==0
        return;
    }

    pdrmem_create( &pdrs, buf, buf_len, PDR_DECODE );

    if( !PacketData::pdr_packet( &pdrs, this ) ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "pdr_packet() failed\n" ));
        MRN_errno = MRN_EPACKING;
    }

    mrn_dbg( 3, mrn_printf(FLF, stderr,
                "Packet(%p) constructor succeeded: src:%s, stream_id:%d "
                "tag:%d, fmt:%s\n", this, src, stream_id, tag, fmt_str ));
}

PacketData::PacketData(const PacketData& p)
    : Error( ), stream_id(p.stream_id), tag(p.tag),
      src(NULL), fmt_str(NULL), buf(NULL), buf_len(p.buf_len),
      inlet_node( p.inlet_node ), destroy_data(p.destroy_data)
{
    if( buf_len != 0 ){
        buf = (char *)malloc( buf_len * sizeof(char) );
        memcpy(buf, p.buf, buf_len);
    }
    if( p.src != NULL ){
        src = strdup(p.src);
    }
    if( p.fmt_str != NULL ){
        fmt_str = strdup(p.fmt_str);
    }

    //rather than just copying pointers, create a new pointer and copy element
    for( unsigned int i=0; i<p.data_elements.size(); i++){
        data_elements.push_back( new DataElement( *(p.data_elements[i]) ) );
    }
}

PacketData& PacketData::operator=(const PacketData& p)
{
    if( this != &p ){
        if(src) {
            free(src);
            src = NULL;
        }
        if(fmt_str) {
            free(fmt_str);
            fmt_str = NULL;
        }
        if(buf) {
            free(buf);
            buf = NULL;
        }

        stream_id = p.stream_id;
        tag = p.tag;
        if( p.src != NULL ){
            src = strdup(p.src);
        }
        if( p.fmt_str != NULL ){
            fmt_str = strdup(p.fmt_str);
        }
        if( buf_len != 0 ){
            buf = (char *)malloc( buf_len * sizeof(char) );
            memcpy(buf, p.buf, buf_len);
        }
        buf_len = p.buf_len;
        inlet_node = p.inlet_node;
        destroy_data = p.destroy_data;

        //rather than just copying pointers,
        //create a new pointer and copy element
        for( unsigned int i=0; i<p.data_elements.size(); i++){
            data_elements.push_back( new DataElement( *(p.data_elements[i]) ) );
        }
    }

    return *this;
}

PacketData::~PacketData()
{
    data_sync.Lock();
    //fprintf(stderr, "ZZZ: Destructing packet: %p\n", this);
    if( src != NULL ){
        //fprintf(stderr, "ZZZ: freeing src: %p\n", src);
        free(src);
        src = NULL;
    }
    if( fmt_str != NULL ){
        //fprintf(stderr, "ZZZ: freeing fmt_str: %p\n", fmt_str);
        free(fmt_str);
        fmt_str = NULL;
    }
    if( buf != NULL ){
        //fprintf(stderr, "ZZZ: freeing buf: %p\n", buf);
        free(buf);
        buf = NULL;
    }

    for( unsigned int i=0; i < data_elements.size(); i++ ){
        if( destroy_data ){
            ((DataElement *)data_elements[i])->set_DestroyData( true );
        }
        //fprintf( stderr, "ZZZ: deleting data_elem[%d]: %p\n",
                 //i, data_elements[i] );
        delete data_elements[i];
    }
    data_sync.Unlock();
}

bool PacketData::operator==(const PacketData& p) const
{
    data_sync.Lock();
    bool ret = ( this == &p );
    data_sync.Unlock();
    return ret;
}

bool PacketData::operator!=(const PacketData& p) const
{
    data_sync.Lock();
    bool ret = ( this != &p );
    data_sync.Unlock();
    return ret;
}

int PacketData::ExtractVaList( const char * /*fmt*/, va_list arg_list ) const
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

bool_t PacketData::pdr_packet( PDR * pdrs, PacketData * pkt )
{
    unsigned int i;
    bool_t retval = 0;
    DataElement * cur_elem=NULL;

    mrn_dbg( 3, mrn_printf(FLF, stderr, "In pdr_packet. op: %d\n", pdrs->p_op ));

    /* Process Packet Header into/out of the pdr mem */
    /********************************************************************
  Packet Buffer Format:
    ___________________________________________________
    | streamid | tag | srcstr |  fmtstr | packed_data |
    ---------------------------------------------------
    *********************************************************************/
    if( pdr_uint16( pdrs, &( pkt->stream_id ) ) == FALSE ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "pdr_uint16() failed\n" ));
        return FALSE;
    }
    if( pdr_int32( pdrs, &( pkt->tag ) ) == FALSE ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "pdr_uint32() failed\n" ));
        return FALSE;
    }
    if( pdr_wrapstring( pdrs, &( pkt->src ) ) == FALSE ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "pdr_wrapstring() failed\n" ));
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
            cur_elem = (DataElement *)pkt->data_elements[i];
        }
        else if( pdrs->p_op == PDR_DECODE ) {
            cur_elem = new DataElement;
            cur_elem->type = Fmt2Type( cur_fmt.c_str() );
        }
        mrn_dbg( 3, mrn_printf(FLF, stderr,
                    "Handling packet[%d], cur_fmt: \"%s\", type: %d\n", i,
                    cur_fmt.c_str(), cur_elem->type ));

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
                pdr_array( pdrs, ( char ** )( &cur_elem->val.p ),
                           &( cur_elem->array_len ), INT32_MAX,
                           sizeof( uchar_t ), ( pdrproc_t ) pdr_uchar );
            break;

        case INT16_T:
        case UINT16_T:
            retval =
                pdr_uint16( pdrs, ( uint16_t * ) ( &( cur_elem->val.d ) ) );
            break;
        case INT16_ARRAY_T:
        case UINT16_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem->val.p = NULL;
            }
            retval =
                pdr_array( pdrs, ( char ** )( &cur_elem->val.p ),
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
                pdr_array( pdrs, ( char ** )( &cur_elem->val.p ),
                           &( cur_elem->array_len ), INT32_MAX,
                           sizeof( uint32_t ), ( pdrproc_t ) pdr_uint32 );
            break;

        case INT64_T:
        case UINT64_T:
            retval =
                pdr_uint64( pdrs, ( uint64_t * ) ( &( cur_elem->val.d ) ) );
            break;
        case INT64_ARRAY_T:
        case UINT64_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem->val.p = NULL;
            }
            retval = pdr_array( pdrs, ( char ** )( &cur_elem->val.p ),
                                &( cur_elem->array_len ), INT32_MAX,
                                sizeof( uint64_t ), ( pdrproc_t ) pdr_uint64 );
            break;

        case FLOAT_T:
            retval = pdr_float( pdrs, ( float * )( &( cur_elem->val.f ) ) );
            mrn_dbg( 3, mrn_printf(FLF, stderr, "floats value: %p: %f\n",
                       &(cur_elem->val.f), cur_elem->val.f ));
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
                pdr_array( pdrs, ( char ** )( &cur_elem->val.p ),
                           &( cur_elem->array_len ), INT32_MAX,
                           sizeof( float ), ( pdrproc_t ) pdr_float );
            break;
        case DOUBLE_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem->val.p = NULL;
            }
            retval =
                pdr_array( pdrs, ( char ** )( &cur_elem->val.p ),
                           &( cur_elem->array_len ), INT32_MAX,
                           sizeof( double ), ( pdrproc_t ) pdr_double );
            break;
        case STRING_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem->val.p = NULL;
            }
            retval = pdr_array( pdrs, (char**)(&cur_elem->val.p),
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

    mrn_dbg( 3, mrn_printf(FLF, stderr, "pdr_packet() succeeded\n" ));
    return TRUE;
}

void PacketData::ArgList2DataElementArray( va_list arg_list )
{
    DataElement * cur_elem=NULL;

    mrn_dbg( 3, mrn_printf(FLF, stderr,
                "In ArgList2DataElementArray, packet(%p)\n", this ));


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
        mrn_dbg( 3, mrn_printf(FLF, stderr,
                    "Handling new packet, cur_fmt: \"%s\", type: %d\n",
                    cur_fmt.c_str(), cur_elem->type ));
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
            mrn_dbg( 3, mrn_printf(FLF, stderr, "floats value: %p: %f\n",
                       &(cur_elem->val.f), cur_elem->val.f ));
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
            cur_elem->val.p = ( void * )va_arg( arg_list, void * );
            cur_elem->array_len =
                ( uint32_t )va_arg( arg_list, uint32_t );
            break;
        case STRING_T:
            cur_elem->val.p = ( void * )va_arg( arg_list, void * );
            cur_elem->array_len = strlen( ( char * )cur_elem->val.p );
            break;
        default:
            assert( 0 );
            break;
        }
        data_elements.push_back( cur_elem );

        curPos = tok.GetNextToken( curLen, delim );
    }

    mrn_dbg( 3, mrn_printf(FLF, stderr,
                "ArgList2DataElementArray succeeded, packet(%p)\n", this ));
}

void PacketData::DataElementArray2ArgList( va_list arg_list ) const
{
    int i = 0;
    const DataElement * cur_elem=NULL;
    const void *tmp_ptr;

    mrn_dbg( 3, mrn_printf(FLF, stderr,
                "In DataElementArray2ArgList, packet(%p)\n", this ));

    std::string fmt = fmt_str;
    XPlat::Tokenizer tok( fmt );
    std::string::size_type curLen;
    const char* delim = " \t\n%";

    std::string::size_type curPos = tok.GetNextToken( curLen, delim );
    while( curPos != std::string::npos ) {

        assert( curLen != 0 );
        std::string cur_fmt = fmt.substr( curPos, curLen );

        cur_elem = data_elements[i];
        assert( cur_elem->type == Fmt2Type( cur_fmt.c_str() ) );
        switch ( cur_elem->type ) {
        case UNKNOWN_T:
            assert( 0 );
        case CHAR_T:
            tmp_ptr = ( void * )va_arg( arg_list, char * );
            *( ( char * )tmp_ptr ) = cur_elem->val.c;
            break;
        case UCHAR_T:
            tmp_ptr = ( void * )va_arg( arg_list, unsigned char * );
            *( ( unsigned char * )tmp_ptr ) = cur_elem->val.uc;
            break;

        case INT16_T:
            tmp_ptr = ( void * )va_arg( arg_list, short int * );
            *( ( short int * )tmp_ptr ) = cur_elem->val.hd;
            break;
        case UINT16_T:
            tmp_ptr = ( void * )va_arg( arg_list, unsigned short int * );
            *( ( unsigned short int * )tmp_ptr ) = cur_elem->val.uhd;
            break;

        case INT32_T:
            tmp_ptr = ( void * )va_arg( arg_list, int * );
            *( ( int * )tmp_ptr ) = cur_elem->val.d;
            break;
        case UINT32_T:
            tmp_ptr = ( void * )va_arg( arg_list, unsigned int * );
            *( ( unsigned int * )tmp_ptr ) = cur_elem->val.ud;
            break;

        case INT64_T:
            tmp_ptr = ( void * )va_arg( arg_list, int64_t * );
            *( ( int64_t * )tmp_ptr ) = cur_elem->val.ld;
            break;
        case UINT64_T:
            tmp_ptr = ( void * )va_arg( arg_list, uint64_t * );
            *( ( uint64_t * )tmp_ptr ) = cur_elem->val.uld;
            break;

        case FLOAT_T:
            tmp_ptr = ( void * )va_arg( arg_list, float * );
            *( ( float * )tmp_ptr ) = cur_elem->val.f;
            break;
        case DOUBLE_T:
            tmp_ptr = ( void * )va_arg( arg_list, double * );
            *( ( double * )tmp_ptr ) = cur_elem->val.lf;
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
            tmp_ptr = ( const void * )va_arg( arg_list, void ** );
            assert( tmp_ptr != NULL );
            *( ( const void ** )tmp_ptr ) = cur_elem->val.p;
            tmp_ptr = ( void * )va_arg( arg_list, int * );
            assert( tmp_ptr != NULL );
            *( ( int * )tmp_ptr ) = cur_elem->array_len;
            break;
        case STRING_T:
            tmp_ptr = ( void * )va_arg( arg_list, char ** );
            *( ( char ** )tmp_ptr ) = ( char * )cur_elem->val.p;
            break;
        default:
            assert( 0 );
        }
        i++;

        curPos = tok.GetNextToken( curLen, delim );
    }

    mrn_dbg( 3, mrn_printf(FLF, stderr,
                "DataElementArray2ArgList succeeded, packet(%p)\n", this ));
    return;
}

}                               /* namespace MRN */
