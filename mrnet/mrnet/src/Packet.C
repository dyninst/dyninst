#include "mrnet/src/Packet.h"
#include "mrnet/src/utils.h"

namespace MRN
{

/**************
 * Packet
 **************/
Packet::Packet( unsigned short _stream_id, int _tag, const char *fmt, ... )
    : stream_id( _stream_id ), tag( _tag )
{
    va_list arg_list;
    PDR pdrs;

    mrn_printf( 3, MCFL, stderr, "In Packet(%p) constructor\n", this );
    fmt_str = strdup( fmt );
    assert( fmt_str );
    std::string network_name;
    getNetworkName( network_name );
    src = strdup( network_name.c_str(  ) );
    assert( src );

    va_start( arg_list, fmt );
    if( ArgList2DataElementArray( arg_list ) == -1 )
        {
            mrn_printf( 1, MCFL, stderr,
                        "ArgList2DataElementArray() failed\n" );
            va_end( arg_list );
            MRN_errno = MRN_EPACKING;
            return;
        }
    va_end( arg_list );

    buf_len = pdr_sizeof( ( pdrproc_t ) ( Packet::pdr_packet ), this );
    assert( buf_len );
    buf = ( char * )malloc( buf_len );
    assert( buf );

    pdrmem_create( &pdrs, buf, buf_len, PDR_ENCODE );
    if( !Packet::pdr_packet( &pdrs, this ) ) {
        mrn_printf( 1, MCFL, stderr, "pdr_packet() failed\n" );
        MRN_errno = MRN_EPACKING;
        return;
    }

    mrn_printf( 3, MCFL, stderr, "Packet(%p) constructor succeeded\n",
                this );
    return;
}

Packet::Packet( unsigned short _stream_id, int _tag, const char *fmt,
                va_list arg_list )
    :    stream_id( _stream_id ), tag( _tag )
{
    PDR pdrs;
    mrn_printf( 3, MCFL, stderr, "In Packet(%p) constructor\n", this );

    fmt_str = strdup( fmt );

    std::string tmp;
    getNetworkName( tmp );
    src = strdup( tmp.c_str(  ) );

    if( ArgList2DataElementArray( arg_list ) == -1 ) {
        mrn_printf( 1, MCFL, stderr,
                    "ArgList2DataElementArray() failed\n" );
        MRN_errno = MRN_EPACKING;
        return;
    }

    buf_len = pdr_sizeof( ( pdrproc_t ) ( Packet::pdr_packet ), this );
    assert( buf_len );
    buf = ( char * )malloc( buf_len );
    assert( buf );

    pdrmem_create( &pdrs, buf, buf_len, PDR_ENCODE );

    if( !Packet::pdr_packet( &pdrs, this ) ) {
        mrn_printf( 1, MCFL, stderr, "pdr_packet() failed\n" );
        MRN_errno = MRN_EPACKING;
        return;
    }

    mrn_printf( 3, MCFL, stderr, "Packet(%p) constructor succeeded\n",
                this );
    return;
}

Packet::Packet( unsigned int _buf_len, char *_buf )
    :    src( NULL ), fmt_str( NULL ), buf( _buf ), buf_len( _buf_len )
{
    PDR pdrs;
    mrn_printf( 3, MCFL, stderr, "In Packet(%p) constructor\n", this );

    pdrmem_create( &pdrs, buf, buf_len, PDR_DECODE );


    if( !Packet::pdr_packet( &pdrs, this ) ) {
        mrn_printf( 1, MCFL, stderr, "pdr_packet() failed\n" );
        MRN_errno = MRN_EPACKING;
    }

    mrn_printf( 3, MCFL, stderr,
                "Packet(%p) constructor succeeded: src:%s, "
                "tag:%d, fmt:%s\n", this, src, tag, fmt_str );
}

Packet::Packet( unsigned short _sid, int _tag,
                DataElement * _data_elements, const char *_fmt_str )
    : stream_id( _sid ), tag( _tag ),
      src( strdup( "<agg>" ) ), fmt_str( strdup( _fmt_str ) )
{
    char *cur_fmt, *fmt = strdup( _fmt_str ), *buf_ptr;
    int i = 0;
    DataElement *cur_elem;

    mrn_printf( 3, MCFL, stderr, "In Packet, packet(%p)\n", this );
    cur_fmt = strtok_r( fmt, " \t\n%", &buf_ptr );
    do {
        if( cur_fmt == NULL ) {
            break;
        }

        cur_elem = &( _data_elements[i++] );
        assert( cur_elem->type == Fmt2Type( cur_fmt ) );
        data_elements.push_back( cur_elem );

        cur_fmt = strtok_r( NULL, " \t\n%", &buf_ptr );
    } while( cur_fmt != NULL );

    // data_elements copied, now pack the message
    PDR pdrs;
    buf_len = pdr_sizeof( ( pdrproc_t ) ( Packet::pdr_packet ), this );
    assert( buf_len );
    buf = ( char * )malloc( buf_len );
    assert( buf );

    pdrmem_create( &pdrs, buf, buf_len, PDR_ENCODE );

    if( !Packet::pdr_packet( &pdrs, this ) ) {
        mrn_printf( 1, MCFL, stderr, "pdr_packet() failed\n" );
        MRN_errno = MRN_EPACKING;
        return;
    }

    mrn_printf( 3, MCFL, stderr, "Packet succeeded, packet(%p)\n", this );
    return;
}

int Packet::ExtractArgList( const char *fmt, ... )
{
    va_list arg_list;

    mrn_printf( 3, MCFL, stderr, "In ExtractArgList(%p)\n", this );

    if( strcmp( fmt_str, fmt ) ) {
        mrn_printf( 1, MCFL, stderr, "Format string mismatch: %s, %s\n",
                    fmt_str, fmt );
        MRN_errno = MRN_EFMTSTR_MISMATCH;
        return -1;
    }

    va_start( arg_list, fmt );
    DataElementArray2ArgList( arg_list );
    va_end( arg_list );

    mrn_printf( 3, MCFL, stderr, "ExtractArgList(%p) succeeded\n", this );
    return 0;
}

int Packet::ExtractVaList( const char *fmt, va_list arg_list )
{
    mrn_printf( 3, MCFL, stderr, "In ExtractVaList(%p)\n", this );

    if( strcmp( fmt_str, fmt ) ) {
        mrn_printf( 1, MCFL, stderr, "Format string mismatch: %s, %s\n",
                    fmt_str, fmt );
        MRN_errno = MRN_EFMTSTR_MISMATCH;
        return -1;
    }

    DataElementArray2ArgList( arg_list );

    mrn_printf( 3, MCFL, stderr, "ExtractVaList(%p) succeeded\n", this );
    return 0;
}

bool_t Packet::pdr_packet( PDR * pdrs, Packet * pkt )
{
    char *cur_fmt, *fmt, *buf_ptr;
    unsigned int i;
    bool_t retval = 0;
    DataElement *cur_elem;

    mrn_printf( 3, MCFL, stderr, "In pdr_packet. op: %d\n", pdrs->p_op );

    /* Process Packet Header into/out of the pdr mem */
    /*******************************************************************************
  Packet Buffer Format:
    ___________________________________________________
    | streamid | tag | srcstr |  fmtstr | packed_data |
    ---------------------------------------------------
    *******************************************************************************/
    //printf(3, MCFL, stderr, "pdrs->space: %d\n", pdrs->space);
    //printf(3, MCFL, stderr, "Calling pdr_uint16()\n");
    if( pdr_uint16( pdrs, &( pkt->stream_id ) ) == FALSE ) {
        mrn_printf( 1, MCFL, stderr, "pdr_uint16() failed\n" );
        return FALSE;
    }
    //printf(1, MCFL, stderr, "Calling pdr_uint32()\n");
    //printf(3, MCFL, stderr, "pdrs->space: %d\n", pdrs->space);
    if( pdr_int32( pdrs, &( pkt->tag ) ) == FALSE ) {
        mrn_printf( 1, MCFL, stderr, "pdr_uint32() failed\n" );
        return FALSE;
    }
    //printf(3, MCFL, stderr, "Calling pdr_wrapstring(%s)\n", pkt->src);
    //printf(3, MCFL, stderr, "pdrs->space: %d\n", pdrs->space);
    if( pdr_wrapstring( pdrs, &( pkt->src ) ) == FALSE ) {
        mrn_printf( 1, MCFL, stderr, "pdr_wrapstring() failed\n" );
        return FALSE;
    }
    //printf(3, MCFL, stderr, "Calling pdr_wrapstring(%s)\n", pkt->fmt_str);
    //printf(3, MCFL, stderr, "pdrs->space: %d\n", pdrs->space);
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
            cur_elem = new DataElement;
            cur_elem->type = Fmt2Type( cur_fmt );
            pkt->data_elements.push_back( cur_elem );
        }
        mrn_printf( 3, MCFL, stderr,
                    "Handling packet[%d], cur_fmt: \"%s\", type: %d\n", i,
                    cur_fmt, cur_elem->type );

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
            retval =
                pdr_array( pdrs, ( char ** )( &cur_elem->val.p ),
                           &( cur_elem->array_len ), INT32_MAX,
                           sizeof( uint64_t ), ( pdrproc_t ) pdr_uint64 );
            break;

        case FLOAT_T:
            retval = pdr_float( pdrs, ( float * )( &( cur_elem->val.f ) ) );
            mrn_printf( 3, MCFL, stderr, "floats value: %f\n",
                        cur_elem->val.f );
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
        case STRING_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem->val.p = NULL;
            }
            else {
                //printf(3, MCFL, stderr, "ENCODING string %s (%p)\n", (char*)cur_elem->val.p, cur_elem->val.p);
                //printf(3, MCFL, stderr, "pdrs->space: %d\n", pdrs->space);
            }
            retval =
                pdr_wrapstring( pdrs, ( char ** )&( cur_elem->val.p ) );
            break;

        case STRING_ARRAY_T:
            if( pdrs->p_op == PDR_DECODE ) {
                cur_elem->val.p = NULL;
            }
            retval = pdr_array( pdrs, ( char ** )( &cur_elem->val.p ),
                                &( cur_elem->array_len ),
                                INT32_MAX,
                                sizeof( char * ),
                                ( pdrproc_t ) pdr_wrapstring );
            break;

        }
        if( !retval ) {
            mrn_printf( 1, MCFL, stderr,
                        "pdr_xxx() failed for elem[%d] of type %d\n", i,
                        cur_elem->type );
            return retval;
        }
        cur_fmt = strtok_r( NULL, " \t\n%", &buf_ptr );
        i++;
    } while( cur_fmt != NULL );

    mrn_printf( 3, MCFL, stderr, "pdr_packet() succeeded\n" );
    return TRUE;
}

int Packet::ArgList2DataElementArray( va_list arg_list )
{
    char *cur_fmt, *fmt = strdup( fmt_str ), *buf_ptr;
    DataElement *cur_elem;

    mrn_printf( 3, MCFL, stderr,
                "In ArgList2DataElementArray, packet(%p)\n", this );

    cur_fmt = strtok_r( fmt, " \t\n%", &buf_ptr );
    do {
        if( cur_fmt == NULL ) {
            break;
        }

        cur_elem = new DataElement;
        cur_elem->type = Fmt2Type( cur_fmt );
        mrn_printf( 3, MCFL, stderr,
                    "Handling new packet, cur_fmt: \"%s\", type: %d\n",
                    cur_fmt, cur_elem->type );
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
        cur_fmt = strtok_r( NULL, " \t\n%", &buf_ptr );
    } while( cur_fmt != NULL );

    mrn_printf( 3, MCFL, stderr,
                "ArgList2DataElementArray succeeded, packet(%p)\n", this );
    return 0;
}

void Packet::DataElementArray2ArgList( va_list arg_list )
{
    char *cur_fmt, *fmt = strdup( fmt_str ), *buf_ptr;
    int i = 0;
    DataElement *cur_elem;
    void *tmp_ptr;

    mrn_printf( 3, MCFL, stderr,
                "In DataElementArray2ArgList, packet(%p)\n", this );
    cur_fmt = strtok_r( fmt, " \t\n%", &buf_ptr );
    do {
        if( cur_fmt == NULL ) {
            break;
        }

        cur_elem = data_elements[i];
        assert( cur_elem->type == Fmt2Type( cur_fmt ) );
        //printf(3, MCFL, stderr, "packet[%d], cur_fmt: \"%s\", cur_type: %d\n",
        //i, cur_fmt, cur_elem->type);
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
            tmp_ptr = ( void * )va_arg( arg_list, void ** );
            assert( tmp_ptr != NULL );
            *( ( void ** )tmp_ptr ) = cur_elem->val.p;
            tmp_ptr = ( void * )va_arg( arg_list, int * );
            assert( tmp_ptr != NULL );
            *( ( int * )tmp_ptr ) = cur_elem->array_len;
            break;
        case STRING_T:
            //printf(3, MCFL, stderr, "Extracting %s\n", (char *)cur_elem->val.p);
            tmp_ptr = ( void * )va_arg( arg_list, char ** );
            *( ( char ** )tmp_ptr ) = ( char * )cur_elem->val.p;
            break;
        default:
            assert( 0 );
        }
        i++;
        cur_fmt = strtok_r( NULL, " \t\n%", &buf_ptr );
    } while( cur_fmt != NULL );

    mrn_printf( 3, MCFL, stderr,
                "DataElementArray2ArgList succeeded, packet(%p)\n", this );
    return;
}

}                               /* namespace MRN */
