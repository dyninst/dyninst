#include "mrnet/src/Packet.h"
#include "mrnet/src/utils.h"

namespace MRN
{

Packet * Packet::NullPacket=NULL;
int Packet_counter::count=0;

/**************
 * Packet
 **************/
PacketData::PacketData( unsigned short _stream_id, int _tag, const char *fmt,
                va_list arg_list )
    :    stream_id( _stream_id ), tag( _tag ), src(NULL),
         fmt_str( strdup(fmt) ), buf(NULL)
{
    PDR pdrs;
    mrn_printf( 3, MCFL, stderr, "In Packet(%p) constructor\n", this );

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
        error( EPACKING, "pdr_packet() failed\n" );
        return;
    }

    mrn_printf( 3, MCFL, stderr,
                "Packet(%p) constructor succeeded. fmt_str=%s\n",
                this, fmt_str );
    return;
}

PacketData::PacketData( unsigned int _buf_len, char *_buf )
    :    src( NULL ), fmt_str( NULL ), buf( _buf ), buf_len( _buf_len )
{
    PDR pdrs;
    mrn_printf( 3, MCFL, stderr, "In Packet(%p) constructor\n", this );

    if( buf_len == 0 ){  //NullPacket has buf==NULL and buf_len==0
        return;
    }

    pdrmem_create( &pdrs, buf, buf_len, PDR_DECODE );

    if( !PacketData::pdr_packet( &pdrs, this ) ) {
        mrn_printf( 1, MCFL, stderr, "pdr_packet() failed\n" );
        MRN_errno = MRN_EPACKING;
    }

    mrn_printf( 3, MCFL, stderr,
                "Packet(%p) constructor succeeded: src:%s, "
                "tag:%d, fmt:%s\n", this, src, tag, fmt_str );
}

PacketData::PacketData(const PacketData& p)
    : data_elements(p.data_elements), stream_id(p.stream_id), tag(p.tag),
      src(NULL), fmt_str(NULL), buf(NULL), buf_len(p.buf_len)
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
}

PacketData& PacketData::operator=(const PacketData& p)
{
    if( this != &p ){
        if(src) free(src);
        if(buf) free(buf);
        if(fmt_str) free(fmt_str);

        stream_id = p.stream_id;
        tag = p.tag;
        buf_len = p.buf_len;

        if( buf_len != 0 ){
            buf = (char *)malloc( buf_len * sizeof(char) );
            memcpy(buf, p.buf, buf_len);
        }
        else{
            buf = NULL;
        }

        if( p.src != NULL ){
            src = strdup(p.src);
        }
        else{
            src = NULL;
        }

        if( p.fmt_str != NULL ){
            fmt_str = strdup(p.fmt_str);
        }
        else{
            fmt_str = NULL;
        }
        data_elements = p.data_elements;
    }

    return *this;
}

PacketData::~PacketData()
{
    if( src != NULL ){
        free(src);
    }
    if( buf != NULL ){
        free(buf);
    }
    if( fmt_str != NULL ){
        free(fmt_str);
    }
}

bool PacketData::operator==(const PacketData& p) const
{
    return ( this == &p );
}

bool PacketData::operator!=(const PacketData& p) const
{
    return ( this != &p );
}

int PacketData::ExtractVaList( const char *fmt, va_list arg_list )
{
    mrn_printf( 3, MCFL, stderr, "In ExtractVaList(%p)\n", this );

    if( strcmp( fmt_str, fmt ) ) {
        error(EFMTSTR, "Extracted (%s), Packet (%s): Format string mismatch\n",
                fmt, fmt_str);
        return -1;
    }

    //TODO: add exception block here to catch user errors
    DataElementArray2ArgList( arg_list );

    mrn_printf( 3, MCFL, stderr, "ExtractVaList(%p) succeeded\n", this );
    return 0;
}

bool_t PacketData::pdr_packet( PDR * pdrs, PacketData * pkt )
{
    char *cur_fmt, *fmt, *buf_ptr;
    unsigned int i;
    bool_t retval = 0;
    DataElement cur_elem;

    mrn_printf( 3, MCFL, stderr, "In pdr_packet. op: %d\n", pdrs->p_op );

    /* Process Packet Header into/out of the pdr mem */
    /********************************************************************
  Packet Buffer Format:
    ___________________________________________________
    | streamid | tag | srcstr |  fmtstr | packed_data |
    ---------------------------------------------------
    *********************************************************************/
    if( pdr_uint16( pdrs, &( pkt->stream_id ) ) == FALSE ) {
        mrn_printf( 1, MCFL, stderr, "pdr_uint16() failed\n" );
        return FALSE;
    }
    if( pdr_int32( pdrs, &( pkt->tag ) ) == FALSE ) {
        mrn_printf( 1, MCFL, stderr, "pdr_uint32() failed\n" );
        return FALSE;
    }
    if( pdr_wrapstring( pdrs, &( pkt->src ) ) == FALSE ) {
        mrn_printf( 1, MCFL, stderr, "pdr_wrapstring() failed\n" );
        return FALSE;
    }
    if( pdr_wrapstring( pdrs, &( pkt->fmt_str ) ) == FALSE ) {
        mrn_printf( 1, MCFL, stderr, "pdr_wrapstring() failed\n" );
        return FALSE;
    }

    if( !pkt->get_FormatString(  ) ) {
        mrn_printf( 3, MCFL, stderr,
                    "No data in message. just header info\n" );
        return TRUE;
    }

    fmt = strdup( pkt->get_FormatString(  ) );
    cur_fmt = strtok_r( fmt, " \t\n%", &buf_ptr );
    i = 0;

    do {
        if( cur_fmt == NULL ) {
            break;
        }

        if( pdrs->p_op == PDR_ENCODE ) {
            cur_elem = pkt->data_elements[i];
        }
        else if( pdrs->p_op == PDR_DECODE ) {
            cur_elem.type = Fmt2Type( cur_fmt );
        }
        mrn_printf( 3, MCFL, stderr,
                    "Handling packet[%d], cur_fmt: \"%s\", type: %d\n", i,
                    cur_fmt, cur_elem.type );

        switch ( cur_elem.type ) {
        case UNKNOWN_T:
            assert( 0 );
        case CHAR_T:
        case UCHAR_T:
            retval =
                pdr_uchar( pdrs, ( uchar_t * ) ( &( cur_elem.val.c ) ) );
            break;

        case CHAR_ARRAY_T:
        case UCHAR_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem.val.p = NULL;
            }
            retval =
                pdr_array( pdrs, ( char ** )( &cur_elem.val.p ),
                           &( cur_elem.array_len ), INT32_MAX,
                           sizeof( uchar_t ), ( pdrproc_t ) pdr_uchar );
            break;

        case INT16_T:
        case UINT16_T:
            retval =
                pdr_uint16( pdrs, ( uint16_t * ) ( &( cur_elem.val.d ) ) );
            break;
        case INT16_ARRAY_T:
        case UINT16_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem.val.p = NULL;
            }
            retval =
                pdr_array( pdrs, ( char ** )( &cur_elem.val.p ),
                           &( cur_elem.array_len ), INT32_MAX,
                           sizeof( uint16_t ), ( pdrproc_t ) pdr_uint16 );
            break;

        case INT32_T:
        case UINT32_T:
            retval =
                pdr_uint32( pdrs, ( uint32_t * ) ( &( cur_elem.val.d ) ) );
            break;
        case INT32_ARRAY_T:
        case UINT32_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem.val.p = NULL;
            }
            retval =
                pdr_array( pdrs, ( char ** )( &cur_elem.val.p ),
                           &( cur_elem.array_len ), INT32_MAX,
                           sizeof( uint32_t ), ( pdrproc_t ) pdr_uint32 );
            break;

        case INT64_T:
        case UINT64_T:
            retval =
                pdr_uint64( pdrs, ( uint64_t * ) ( &( cur_elem.val.d ) ) );
            break;
        case INT64_ARRAY_T:
        case UINT64_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem.val.p = NULL;
            }
            retval = pdr_array( pdrs, ( char ** )( &cur_elem.val.p ),
                                &( cur_elem.array_len ), INT32_MAX,
                                sizeof( uint64_t ), ( pdrproc_t ) pdr_uint64 );
            break;

        case FLOAT_T:
            retval = pdr_float( pdrs, ( float * )( &( cur_elem.val.f ) ) );
            mrn_printf( 3, MCFL, stderr, "floats value: %p: %f\n",
                       &(cur_elem.val.f), cur_elem.val.f );
            break;
        case DOUBLE_T:
            retval =
                pdr_double( pdrs, ( double * )( &( cur_elem.val.lf ) ) );
            break;
        case FLOAT_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem.val.p = NULL;
            }
            retval =
                pdr_array( pdrs, ( char ** )( &cur_elem.val.p ),
                           &( cur_elem.array_len ), INT32_MAX,
                           sizeof( float ), ( pdrproc_t ) pdr_float );
            break;
        case DOUBLE_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem.val.p = NULL;
            }
            retval =
                pdr_array( pdrs, ( char ** )( &cur_elem.val.p ),
                           &( cur_elem.array_len ), INT32_MAX,
                           sizeof( double ), ( pdrproc_t ) pdr_double );
            break;
        case STRING_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem.val.p = NULL;
            }
            retval =
                pdr_wrapstring( pdrs, ( char ** )&( cur_elem.val.p ) );
            break;
        }
        if( !retval ) {
            mrn_printf( 1, MCFL, stderr,
                        "pdr_xxx() failed for elem[%d] of type %d\n", i,
                        cur_elem.type );
            return retval;
        }
        if( pdrs->p_op == PDR_DECODE ) {
            pkt->data_elements.push_back( cur_elem );
        }
        cur_fmt = strtok_r( NULL, " \t\n%", &buf_ptr );
        i++;
    } while( cur_fmt != NULL );

    mrn_printf( 3, MCFL, stderr, "pdr_packet() succeeded\n" );
    return TRUE;
}

void PacketData::ArgList2DataElementArray( va_list arg_list )
{
    char *cur_fmt, *fmt = strdup( fmt_str ), *buf_ptr;
    DataElement cur_elem;

    mrn_printf( 3, MCFL, stderr,
                "In ArgList2DataElementArray, packet(%p)\n", this );

    cur_fmt = strtok_r( fmt, " \t\n%", &buf_ptr );
    do {
        if( cur_fmt == NULL ) {
            break;
        }

        cur_elem.type = Fmt2Type( cur_fmt );
        mrn_printf( 3, MCFL, stderr,
                    "Handling new packet, cur_fmt: \"%s\", type: %d\n",
                    cur_fmt, cur_elem.type );
        switch ( cur_elem.type ) {
        case UNKNOWN_T:
            assert( 0 );
        case CHAR_T:
            cur_elem.val.c = ( char )va_arg( arg_list, int32_t );
            break;
        case UCHAR_T:
            cur_elem.val.uc = ( char )va_arg( arg_list, uint32_t );
            break;

        case INT16_T:
            cur_elem.val.hd = ( short int )va_arg( arg_list, int32_t );
            break;
        case UINT16_T:
            cur_elem.val.uhd = ( short int )va_arg( arg_list, uint32_t );
            break;

        case INT32_T:
            cur_elem.val.d = ( int )va_arg( arg_list, int32_t );
            break;
        case UINT32_T:
            cur_elem.val.ud = ( int )va_arg( arg_list, uint32_t );
            break;

        case INT64_T:
            cur_elem.val.ld = ( int64_t )va_arg( arg_list, int64_t );
            break;
        case UINT64_T:
            cur_elem.val.uld = ( uint64_t )va_arg( arg_list, uint64_t );
            break;

        case FLOAT_T:
            cur_elem.val.f = ( float )va_arg( arg_list, double );
            mrn_printf( 3, MCFL, stderr, "floats value: %p: %f\n",
                       &(cur_elem.val.f), cur_elem.val.f );
            break;
        case DOUBLE_T:
            cur_elem.val.lf = ( double )va_arg( arg_list, double );
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
            cur_elem.val.p = ( void * )va_arg( arg_list, void * );
            cur_elem.array_len =
                ( uint32_t )va_arg( arg_list, uint32_t );
            break;
        case STRING_T:
            cur_elem.val.p = ( void * )va_arg( arg_list, void * );
            cur_elem.array_len = strlen( ( char * )cur_elem.val.p );
            break;
        default:
            assert( 0 );
            break;
        }
        data_elements.push_back( cur_elem );
        cur_fmt = strtok_r( NULL, " \t\n%", &buf_ptr );
    } while( cur_fmt != NULL );

    mrn_printf( 3, MCFL, stderr,
                "ArgList2DataElementArray succeeded, packet(%p)\n", this );
    free(fmt);
}

void PacketData::DataElementArray2ArgList( va_list arg_list )
{
    char *cur_fmt, *fmt = strdup( fmt_str ), *buf_ptr;
    int i = 0;
    DataElement cur_elem;
    void *tmp_ptr;

    mrn_printf( 3, MCFL, stderr,
                "In DataElementArray2ArgList, packet(%p)\n", this );
    cur_fmt = strtok_r( fmt, " \t\n%", &buf_ptr );
    do {
        if( cur_fmt == NULL ) {
            break;
        }

        cur_elem = data_elements[i];
        assert( cur_elem.type == Fmt2Type( cur_fmt ) );
        switch ( cur_elem.type ) {
        case UNKNOWN_T:
            assert( 0 );
        case CHAR_T:
            tmp_ptr = ( void * )va_arg( arg_list, char * );
            *( ( char * )tmp_ptr ) = cur_elem.val.c;
            break;
        case UCHAR_T:
            tmp_ptr = ( void * )va_arg( arg_list, unsigned char * );
            *( ( unsigned char * )tmp_ptr ) = cur_elem.val.uc;
            break;

        case INT16_T:
            tmp_ptr = ( void * )va_arg( arg_list, short int * );
            *( ( short int * )tmp_ptr ) = cur_elem.val.hd;
            break;
        case UINT16_T:
            tmp_ptr = ( void * )va_arg( arg_list, unsigned short int * );
            *( ( unsigned short int * )tmp_ptr ) = cur_elem.val.uhd;
            break;

        case INT32_T:
            tmp_ptr = ( void * )va_arg( arg_list, int * );
            *( ( int * )tmp_ptr ) = cur_elem.val.d;
            break;
        case UINT32_T:
            tmp_ptr = ( void * )va_arg( arg_list, unsigned int * );
            *( ( unsigned int * )tmp_ptr ) = cur_elem.val.ud;
            break;

        case INT64_T:
            tmp_ptr = ( void * )va_arg( arg_list, int64_t * );
            *( ( int64_t * )tmp_ptr ) = cur_elem.val.ld;
            break;
        case UINT64_T:
            tmp_ptr = ( void * )va_arg( arg_list, uint64_t * );
            *( ( uint64_t * )tmp_ptr ) = cur_elem.val.uld;
            break;

        case FLOAT_T:
            tmp_ptr = ( void * )va_arg( arg_list, float * );
            *( ( float * )tmp_ptr ) = cur_elem.val.f;
            break;
        case DOUBLE_T:
            tmp_ptr = ( void * )va_arg( arg_list, double * );
            *( ( double * )tmp_ptr ) = cur_elem.val.lf;
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
            tmp_ptr = ( void * )va_arg( arg_list, void ** );
            assert( tmp_ptr != NULL );
            *( ( void ** )tmp_ptr ) = cur_elem.val.p;
            tmp_ptr = ( void * )va_arg( arg_list, int * );
            assert( tmp_ptr != NULL );
            *( ( int * )tmp_ptr ) = cur_elem.array_len;
            break;
        case STRING_T:
            tmp_ptr = ( void * )va_arg( arg_list, char ** );
            *( ( char ** )tmp_ptr ) = ( char * )cur_elem.val.p;
            break;
        default:
            assert( 0 );
        }
        i++;
        cur_fmt = strtok_r( NULL, " \t\n%", &buf_ptr );
    } while( cur_fmt != NULL );

    mrn_printf( 3, MCFL, stderr,
                "DataElementArray2ArgList succeeded, packet(%p)\n", this );
    free(fmt);
    return;
}

}                               /* namespace MRN */
