/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include "mrnet/src/Types.h"
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

#include "mrnet/src/utils.h"
#include "mrnet/src/Message.h"
#include "xplat/NCIO.h"

namespace MRN
{

int read( int fd, void *buf, int size );
int write( int fd, const void *buf, int size );

int Message::recv( int sock_fd, std::list < Packet >&packets_in,
                   RemoteNode * remote_node )
{
    unsigned int i;
    int32_t buf_len;
    uint32_t no_packets = 0, *packet_sizes;
    char *buf = NULL;
    PDR pdrs;
    enum pdr_op op = PDR_DECODE;


    mrn_printf( 3, MCFL, stderr, "Receiving packets to message (%p)\n",
                this );

    //
    // packet count
    //

    /* find out how many packets are coming */
    buf_len = pdr_sizeof( ( pdrproc_t ) ( pdr_uint32 ), &no_packets );
    assert( buf_len );
    buf = ( char * )malloc( buf_len );
    assert( buf );

    mrn_printf( 3, MCFL, stderr, "Calling read(%d, %p, %d)\n", sock_fd, buf,
                buf_len );
    int retval;
    if( ( retval = MRN::read( sock_fd, buf, buf_len ) ) != buf_len ) {
        mrn_printf( 3, MCFL, stderr, "read returned %d\n", retval );
        _perror( "MRN::read()" );
        error( MRN_ESYSTEM, "MRN::read() %d of %d bytes received: %s",
               retval, buf_len, strerror(errno) );
        free( buf );
        return -1;
    }


    //
    // pdrmem initialize
    //

    pdrmem_create( &pdrs, buf, buf_len, op );
    if( !pdr_uint32( &pdrs, &no_packets ) ) {
        error( MRN_EPACKING, "pdr_uint32() failed\n");
        free( buf );
        return -1;
    }
    free( buf );
    mrn_printf( 3, MCFL, stderr,
                "pdr_uint32() succeeded. Receive %d packets\n",
                no_packets );

    if( no_packets == 0 ) {
        mrn_printf( 2, MCFL, stderr, "warning: Receiving %d packets\n",
                    no_packets );
    }


    //
    // packet size vector
    //

    /* recv an vector of packet_sizes */
    //buf_len's value is hardcode, breaking pdr encapsulation barrier :(
    buf_len = (sizeof( uint32_t ) * no_packets) + 1;  // 1 byte pdr overhead
    buf = ( char * )malloc( buf_len );
    assert( buf );


    packet_sizes = ( uint32_t * ) malloc( sizeof( uint32_t ) * no_packets );
    if( packet_sizes == NULL ) {
        mrn_printf( 1, MCFL, stderr,
                    "recv: packet_size malloc is NULL for %d packets\n",
                    no_packets );
    }
    assert( packet_sizes );

    mrn_printf( 3, MCFL, stderr,
                "Calling read(%d, %p, %d) for %d buffer lengths.\n",
                sock_fd, buf, buf_len, no_packets );
    int readRet = MRN::read( sock_fd, buf, buf_len );
    if( readRet != buf_len ) {
        mrn_printf( 1, MCFL, stderr, "MRN::read() failed\n" );
        error( MRN_ESYSTEM, "MRN::read() %d of %d bytes received: %s",
               readRet, buf_len, strerror(errno) );
        free( buf );
        free( packet_sizes );
        return -1;
    }

    //
    // packets
    //

    pdrmem_create( &pdrs, buf, buf_len, op );
    if( !pdr_vector ( &pdrs, ( char * )( packet_sizes ), no_packets,
                      sizeof( uint32_t ), ( pdrproc_t ) pdr_uint32 ) ) {
        mrn_printf( 1, MCFL, stderr, "pdr_vector() failed\n" );
        error( MRN_EPACKING, "pdr_uint32() failed\n");
        free( buf );
        free( packet_sizes );
        return -1;
    }
    free( buf );

    /* recv packet buffers */
    XPlat::NCBuf* ncbufs = new XPlat::NCBuf[no_packets];

    mrn_printf( 3, MCFL, stderr, "Reading %d packets of size: [",
                no_packets );

    int total_bytes = 0;
    for( i = 0; i < no_packets; i++ ) {
        ncbufs[i].buf = (char*)malloc( packet_sizes[i] );
        ncbufs[i].len = packet_sizes[i];
        total_bytes += packet_sizes[i];
        mrn_printf( 3, 0, 0, stderr, "%d, ", packet_sizes[i] );
    }
    mrn_printf( 3, 0, 0, stderr, "]\n" );

    retval = XPlat::NCRecv( sock_fd, ncbufs, no_packets );
    if( retval != total_bytes ) {
        mrn_printf( 1, MCFL, stderr, "%s", "" );
        _perror( "XPlat::NCRecv()" );
        error( MRN_ESYSTEM, "XPlat::NCRecv() %d of %d bytes received: %s",
               retval, buf_len, strerror(errno) );

        for( i = 0; i < no_packets; i++ )
        {
            free( (void*)(ncbufs[i].buf) );
        }
        delete[] ncbufs;

        free( packet_sizes );
        return -1;
    }

    //
    // post-processing
    //

    for( i = 0; i < no_packets; i++ ) {
        Packet new_packet ( ncbufs[i].len, ncbufs[i].buf );
        if( new_packet.fail(  ) ) {
            mrn_printf( 1, MCFL, stderr, "packet creation failed\n" );
            for( i = 0; i < no_packets; i++ )
            {
                free( (void*)(ncbufs[i].buf) );
            }
            delete[] ncbufs;
            free( packet_sizes );
            return -1;
        }
        new_packet.set_InletNode( remote_node );
        packets_in.push_back( new_packet );
    }

    // release dynamically allocated memory
    // Note: don't release the NC buffers; that memory was passed
    // off to the Packet object(s).
    delete[] ncbufs;
    free( packet_sizes );
    mrn_printf( 3, MCFL, stderr, "Msg(%p)::recv() succeeded\n", this );
    return 0;
}

int Message::send( int sock_fd )
{
    /* send an array of packet_sizes */
    unsigned int i;
    uint32_t *packet_sizes=NULL, no_packets;
    char *buf=NULL;
    int buf_len;
    PDR pdrs;
    enum pdr_op op = PDR_ENCODE;


    mrn_printf( 3, MCFL, stderr, "Sending packets from message %p\n",
                this );
    if( packets.size( ) == 0 ) {   //nothing to do
        mrn_printf( 3, MCFL, stderr, "Nothing to send!\n" );
        return 0;
    }
    no_packets = packets.size( );

    /* send packet buffers */
    XPlat::NCBuf* ncbufs = new XPlat::NCBuf[no_packets];

    //Process packets in list to prepare for send()
    packet_sizes = ( uint32_t * ) malloc( sizeof( uint32_t ) * no_packets );
    assert( packet_sizes );
    std::list < Packet >::iterator iter = packets.begin( );
    mrn_printf( 3, MCFL, stderr, "Writing %d packets of size: [ ",
                no_packets );
    int total_bytes = 0;
    for( i = 0; iter != packets.end( ); iter++, i++ ) {

        Packet curPacket = *iter;

        ncbufs[i].buf = curPacket.get_Buffer( );
        ncbufs[i].len = curPacket.get_BufferLen( );
        packet_sizes[i] = curPacket.get_BufferLen( );

        total_bytes += ncbufs[i].len;
        mrn_printf( 3, 0, 0, stderr, "%d, ", ( int )ncbufs[i].len );
    }
    mrn_printf( 3, 0, 0, stderr, "]\n" );

    /* put how many packets are going */

    buf_len = pdr_sizeof( ( pdrproc_t )( pdr_uint32 ), &no_packets );
    assert( buf_len );
    buf = ( char * )malloc( buf_len );
    assert( buf );
    pdrmem_create( &pdrs, buf, buf_len, op );


    if( !pdr_uint32( &pdrs, &no_packets ) ) {
        error( MRN_EPACKING, "pdr_uint32() failed\n" );
        free( buf );
        delete[] ncbufs;
        free( packet_sizes );
        return -1;
    }

    mrn_printf( 3, MCFL, stderr, "Calling write(%d, %p, %d)\n", sock_fd,
                buf, buf_len );
    if( MRN::write( sock_fd, buf, buf_len ) != buf_len ) {
        mrn_printf( 1, MCFL, stderr, "write() failed" );
        _perror( "write()" );
        free( buf );
        delete[] ncbufs;
        free( packet_sizes );
        return -1;
    }
    mrn_printf( 3, MCFL, stderr, "write() succeeded" );
    free( buf );

    /* send a vector of packet_sizes */
    buf_len = (no_packets * sizeof( uint32_t )) + 1;  //1 extra bytes overhead
    buf = ( char * )malloc( buf_len );
    assert( buf );
    pdrmem_create( &pdrs, buf, buf_len, op );

    if( !pdr_vector
        ( &pdrs, ( char * )( packet_sizes ), no_packets, sizeof( uint32_t ),
          ( pdrproc_t ) pdr_uint32 ) ) {
        mrn_printf( 1, MCFL, stderr, "pdr_vector() failed\n" );
        error( MRN_EPACKING, "pdr_vector() failed\n" );
        free( buf );
        delete[] ncbufs;
        free( packet_sizes );
        return -1;
    }

    mrn_printf( 3, MCFL, stderr, "Calling write(%d, %p, %d)\n", sock_fd,
                buf, buf_len );
    int mcwret = MRN::write( sock_fd, buf, buf_len );
    if( mcwret != buf_len ) {
        mrn_printf( 1, MCFL, stderr, "write failed" );
        _perror( "write()" );
        free( buf );
        delete[] ncbufs;
        free( packet_sizes );
        return -1;
    }
    free( packet_sizes );
    free( buf );


    // send the packets
    mrn_printf( 3, MCFL, stderr,
                "Calling XPlat::NCSend(%d buffers, %d total bytes)\n",
                no_packets, total_bytes );

    int sret = XPlat::NCSend( sock_fd, ncbufs, no_packets );
    if( sret != total_bytes ) {
        mrn_printf( 3, MCFL, stderr,
                    "XPlat::NCSend() returned %d of %d bytes, errno = %d, nbuffers = %d\n",
                    sret, total_bytes, errno, no_packets );
        for( i = 0; i < no_packets; i++ ) {
            mrn_printf( 3, MCFL, stderr, "buffer[%d].size = %d\n",
                        i, ncbufs[i].len );
        }
        _perror( "XPlat::NCSend()" );
        error( MRN_ESYSTEM, "XPlat::NCSend() returned %d of %d bytes: %s\n",
                    sret, total_bytes, strerror(errno) );
        delete[] ncbufs;
        return -1;
    }

    packets.clear(  );

    delete[] ncbufs;
    mrn_printf( 3, MCFL, stderr, "msg(%p)::send() succeeded\n", this );
    return 0;
}


/*********************************************************
 *  Functions used to implement sending and recieving of
 *  some basic data types
 *********************************************************/

int write( int fd, const void *buf, int count )
{
    int ret = ::send( fd, (const char*)buf, count, 0 );

    //TODO: recursive call checking for syscall interuption
    return ret;
}

int read( int fd, void *buf, int count )
{
    int bytes_recvd = 0, retval;
    while( bytes_recvd != count ) {
        retval = ::recv( fd, ( ( char * )buf ) + bytes_recvd,
                       count - bytes_recvd,
                       XPlat::NCBlockingRecvFlag );

        if( retval == -1 ) {
            if( errno == EINTR ) {
                continue;
            }
            else {
                mrn_printf( 3, MCFL, stderr,
                            "premature return from read(). Got %d of %d "
                            " bytes. errno: %d ", bytes_recvd, count,
                            errno );
                return -1;
            }
        }
        else if( ( retval == 0 ) && ( errno == EINTR ) ) {
            // this situation has been seen to occur on Linux
            // when the remote endpoint has gone away
            return -1;
        }
        else {
            bytes_recvd += retval;
            if( bytes_recvd < count && errno == EINTR ) {
                continue;
            }
            else {
                //bytes_recvd is either count, or error other than "eintr" occured
                if( bytes_recvd != count ) {
                    mrn_printf( 3, MCFL, stderr,
                                "premature return from read(). %d of %d "
                                " bytes. errno: %d ", bytes_recvd, count,
                                errno );
                }
                return bytes_recvd;
            }
        }
    }
    assert( 0 );
    return -1;
}

}                               // namespace MRN
