/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include "mrnet/src/Types.h"
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <poll.h>

#include "mrnet/src/Message.h"
#include "mrnet/src/utils.h"

namespace MRN
{

int Message::recv( int sock_fd, std::list < Packet >&packets_in,
                   RemoteNode * remote_node )
{
    int i;
    int32_t buf_len;
    uint32_t no_packets = 0, *packet_sizes;
    struct msghdr msg;
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
    if( ( retval = read( sock_fd, buf, buf_len ) ) != buf_len ) {
        mrn_printf( 3, MCFL, stderr, "read returned %d\n", retval );
        _perror( "read()" );
        error( ESYSTEM, "read() %d of %d bytes received: %s",
               retval, buf_len, strerror(errno) );
        free( buf );
        return -1;
    }


    //
    // pdrmem initialize
    //

    pdrmem_create( &pdrs, buf, buf_len, op );
    if( !pdr_uint32( &pdrs, &no_packets ) ) {
        error( EPACKING, "pdr_uint32() failed\n");
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
    int readRet = read( sock_fd, buf, buf_len );
    if( readRet != buf_len ) {
        mrn_printf( 1, MCFL, stderr, "read() failed\n" );
        error( ESYSTEM, "read() %d of %d bytes received: %s",
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
        error( EPACKING, "pdr_uint32() failed\n");
        free( buf );
        free( packet_sizes );
        return -1;
    }
    free( buf );

    /* recv packet buffers */
    msg.msg_name = NULL;
    msg.msg_iov =
        ( struct iovec * )malloc( sizeof( struct iovec ) * no_packets );
    assert( msg.msg_iov );
    msg.msg_iovlen = no_packets;
    msg.msg_control = NULL; /* ancillary data, see below */
    msg.msg_controllen = 0;

    mrn_printf( 3, MCFL, stderr, "Reading %d packets of size: [",
                no_packets );
    int total_bytes = 0;
    for( i = 0; i < msg.msg_iovlen; i++ ) {
        msg.msg_iov[i].iov_base = malloc( packet_sizes[i] );
        assert( msg.msg_iov[i].iov_base );
        msg.msg_iov[i].iov_len = packet_sizes[i];
        total_bytes += packet_sizes[i];
        mrn_printf( 3, 0, 0, stderr, "%d, ", packet_sizes[i] );
    }
    mrn_printf( 3, 0, 0, stderr, "]\n" );

    retval = readmsg( sock_fd, &msg );
    if( retval != total_bytes ) {
        mrn_printf( 1, MCFL, stderr, "%s", "" );
        _perror( "readmsg()" );
        error( ESYSTEM, "readmsg() %d of %d bytes received: %s",
               retval, buf_len, strerror(errno) );
        free(msg.msg_iov);
        free( packet_sizes );
        return -1;
    }

    //
    // post-processing
    //

    for( i = 0; i < msg.msg_iovlen; i++ ) {
        Packet new_packet ( msg.msg_iov[i].iov_len,
                            ( char * )msg.msg_iov[i].
                            iov_base );
        if( new_packet.fail(  ) ) {
            mrn_printf( 1, MCFL, stderr, "packet creation failed\n" );
            free(msg.msg_iov);
            free( packet_sizes );
            return -1;
        }
        new_packet.set_InletNode( remote_node );
        packets_in.push_back( new_packet );
    }

    free(msg.msg_iov);
    free( packet_sizes );
    mrn_printf( 3, MCFL, stderr, "Msg(%p)::recv() succeeded\n", this );
    return 0;
}

int Message::send( int sock_fd )
{
    /* send an array of packet_sizes */
    unsigned int i;
    uint32_t *packet_sizes=NULL, no_packets;
    struct iovec *iov;
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
    iov = ( struct iovec * )malloc( sizeof( struct iovec ) * no_packets );
    assert( iov );

    //Process packets in list to prepare for send()
    packet_sizes = ( uint32_t * ) malloc( sizeof( uint32_t ) * no_packets );
    assert( packet_sizes );
    std::list < Packet >::iterator iter = packets.begin( );
    mrn_printf( 3, MCFL, stderr, "Writing %d packets of size: [ ",
                no_packets );
    int total_bytes = 0;
    for( i = 0; iter != packets.end( ); iter++, i++ ) {

        Packet curPacket = *iter;

        iov[i].iov_base = curPacket.get_Buffer( );
        iov[i].iov_len = curPacket.get_BufferLen( );
        packet_sizes[i] = curPacket.get_BufferLen( );

        total_bytes += iov[i].iov_len;
        mrn_printf( 3, 0, 0, stderr, "%d, ", ( int )iov[i].iov_len );
    }
    mrn_printf( 3, 0, 0, stderr, "]\n" );

    /* put how many packets are going */

    buf_len = pdr_sizeof( ( pdrproc_t )( pdr_uint32 ), &no_packets );
    assert( buf_len );
    buf = ( char * )malloc( buf_len );
    assert( buf );
    pdrmem_create( &pdrs, buf, buf_len, op );


    if( !pdr_uint32( &pdrs, &no_packets ) ) {
        error( EPACKING, "pdr_uint32() failed\n" );
        free( buf );
        free( iov );
        free( packet_sizes );
        return -1;
    }

    mrn_printf( 3, MCFL, stderr, "Calling write(%d, %p, %d)\n", sock_fd,
                buf, buf_len );
    if( MRN::write( sock_fd, buf, buf_len ) != buf_len ) {
        mrn_printf( 1, MCFL, stderr, "write() failed" );
        _perror( "write()" );
        free( buf );
        free( iov );
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
        error( EPACKING, "pdr_vector() failed\n" );
        free( buf );
        free( iov );
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
        free( iov );
        free( packet_sizes );
        return -1;
    }
    free( packet_sizes );
    free( buf );


    if( no_packets > IOV_MAX ) {
        mrn_printf( 3, MCFL, stderr, "splitting writev\n" );
    }

    uint32_t nPacketsLeftToSend = no_packets;
    struct iovec *currIov = iov;
    while( nPacketsLeftToSend > 0 ) {
        uint32_t iovlen =
            ( nPacketsLeftToSend > IOV_MAX ) ? IOV_MAX : nPacketsLeftToSend;

        // count the bytes in the packets to be sent
        int nBytesToSend = 0;
        for( i = 0; i < iovlen; i++ ) {
            nBytesToSend += currIov[i].iov_len;
        }

        mrn_printf( 3, MCFL, stderr,
                    "Calling writev(%d vectors, %d total bytes)\n", iovlen,
                    nBytesToSend );

        int ret = writev( sock_fd, currIov, iovlen );
        if( ret != nBytesToSend ) {
            mrn_printf( 3, MCFL, stderr,
                        "writev() returned %d of %d bytes, errno = %d, iovlen = %d\n",
                        ret, nBytesToSend, errno, iovlen );
            for( i = 0; i < iovlen; i++ ) {
                mrn_printf( 3, MCFL, stderr, "vector[%d].size = %d\n",
                            i, currIov[i].iov_len );
            }
            _perror( "writev()" );
            error( ESYSTEM, "writev() returned %d of %d bytes: %s\n",
                        ret, nBytesToSend, strerror(errno) );
            free(iov);
            return -1;
        }

        // advance
        nPacketsLeftToSend -= iovlen;
        currIov += iovlen;
    }

    packets.clear(  );


    free(iov);
    mrn_printf( 3, MCFL, stderr, "msg(%p)::send() succeeded\n", this );
    return 0;
}


/*********************************************************
 *  Functions used to implement sending and recieving of
 *  some basic data types
 *********************************************************/

int write( int fd, const void *buf, int count )
{
    int ret = send( fd, buf, count, 0 );

    //TODO: recursive call checking for syscall interuption
    return ret;
}

int read( int fd, void *buf, int count )
{
    int bytes_recvd = 0, retval;
    while( bytes_recvd != count ) {
        retval = recv( fd, ( ( char * )buf ) + bytes_recvd,
                       count - bytes_recvd, MSG_WAITALL );

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

int readmsg( int fd, struct msghdr *msg )
{
    //TODO: recursive call checking for syscall interuption
    return recvmsg( fd, msg, MSG_WAITALL );
}

}                               // namespace MRN
