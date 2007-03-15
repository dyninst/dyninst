/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include "utils.h"
#include "Types.h"

#include "xplat/NetUtils.h"
#include "xplat/PathUtils.h"
#include "xplat/SocketUtils.h"
#include "xplat/Mutex.h"
#include "xplat/Error.h"

#include "mrnet/MRNet.h"

#include "config.h"
#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <map>

#if !defined(os_windows)
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif // defined(os_windows)

#if defined(os_solaris)
#include <sys/sockio.h>         //only for solaris
#endif // defined(os_solaris)

static XPlat::Mutex gethostbyname_mutex;

namespace MRN
{

std::string LocalHostName="";
std::string LocalDomainName="";
std::string LocalNetworkName="";
std::string LocalNetworkAddr="";

Port LocalPort=0;

XPlat::TLSKey tsd_key;

static struct hostent * copy_hostent( struct hostent *);
static void delete_hostent( struct hostent *in );
static struct hostent * mrnet_gethostbyname( const char * name );

inline struct hostent * copy_hostent( struct hostent *in)
{
    struct hostent * out = new struct hostent;
    unsigned int i=0;

    //copy h_name, h_addrtype, and h_length
    out->h_name = strdup( in->h_name );
    out->h_addrtype = in->h_addrtype;
    out->h_length = in->h_length;

    //deep copy h_aliases
    unsigned int count=0;
    while( in->h_aliases[count] != NULL )
        count++;

    out->h_aliases = new char * [ count+1 ];
    for(i=0; i<count; i++ ){
        out->h_aliases[i] = strdup( in->h_aliases[i] );
    }
    out->h_aliases[count] = NULL;

    //deep copy h_addr_list
    count=0;
    while( in->h_addr_list[count] != 0 )
        count++;

    out->h_addr_list = new char * [ count+1 ];
    for(i=0; i<count; i++ ){
        out->h_addr_list[i] = new char[4];
        out->h_addr_list[i][0] = in->h_addr_list[i][0];
        out->h_addr_list[i][1] = in->h_addr_list[i][1];
        out->h_addr_list[i][2] = in->h_addr_list[i][2];
        out->h_addr_list[i][3] = in->h_addr_list[i][3];
    }
    out->h_addr_list[count] = NULL;

    return out;
}

inline void delete_hostent( struct hostent *in )
{
    free(in->h_name);

    unsigned int count=0;
    while( in->h_aliases[count] != NULL )
        free( in->h_aliases[count++] );
    delete [] in->h_aliases;

    count=0;
    while( in->h_addr_list[count] != 0 )
        delete [] in->h_addr_list[count++];

    delete [] in->h_addr_list;
    delete in;
}

struct hostent * mrnet_gethostbyname( const char * name )
{

    gethostbyname_mutex.Lock();

    struct hostent * temp_hostent = gethostbyname( name );

    if( temp_hostent == NULL ){
        gethostbyname_mutex.Unlock();
        return NULL;
    }
    struct hostent * ret_hostent = copy_hostent(temp_hostent);
  
    gethostbyname_mutex.Unlock();
    return ret_hostent;
}

int connectHost( int *sock_in, const std::string & hostname, Port port )
{

    int sock = *sock_in;
    struct sockaddr_in server_addr;
    struct hostent *server_hostent;

    mrn_dbg( 3, mrn_printf(FLF, stderr, "In connect_to_host(%s:%d) ...\n",
                hostname.c_str(  ), port ) );

    if( sock == 0 ) {
        sock = socket( AF_INET, SOCK_STREAM, 0 );

        if( sock == -1 ) {
            mrn_dbg( 1, mrn_printf(FLF, stderr, "socket() failed\n" ) );
            perror( "socket()" );
            return -1;
        }
    }

    server_hostent = mrnet_gethostbyname( hostname.c_str( ) );
    if( server_hostent == NULL ) {
        perror( "gethostbyname()" );
        delete_hostent( server_hostent );
        return -1;
    }

    memset( &server_addr, 0, sizeof( server_addr ) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons( port );
    memcpy( &server_addr.sin_addr, server_hostent->h_addr_list[0],
            sizeof( struct in_addr ) );
    delete_hostent( server_hostent );

    unsigned int nConnectTries = 0;
    int cret = -1;
    while( ( cret == -1 ) && ( nConnectTries < 5 ) ) {
        cret =
            connect( sock, (sockaddr *) & server_addr, sizeof( server_addr ) );
        if( cret == -1 ) {
            int err = XPlat::NetUtils::GetLastError();
            if( !XPlat::Error::ETimedOut( err ) ) {
                mrn_dbg( 1, mrn_printf(FLF, stderr, "connect() failed: %s\n",
                            XPlat::Error::GetErrorString( err ).c_str() ) );
                return -1;
            }
            nConnectTries++;
            mrn_dbg( 3, mrn_printf(FLF, stderr, "connection timed out %d times\n",
                        nConnectTries ) );
        }
    }

#if defined(TCP_NODELAY)
    // turn off Nagle algorithm for coalescing packets
    int optVal = 1;
    int ssoret = setsockopt( sock,
                             IPPROTO_TCP,
                             TCP_NODELAY,
                             (const char*)&optVal,
                             sizeof( optVal ) );
    if( ssoret == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "failed to set TCP_NODELAY\n" ) );
    }
#endif // defined(TCP_NODELAY)

    mrn_dbg( 3, mrn_printf(FLF, stderr,
                "Leaving Connect_to_host(). Returning sock: %d\n", sock ) );
    *sock_in = sock;
    return 0;
}

int bindPort( int *sock_in, Port *port_in )
{
    int sock = *sock_in;
    Port port = *port_in;
    struct sockaddr_in local_addr;

    mrn_dbg( 3, mrn_printf(FLF, stderr, "In bind_to_port(sock:%d, port:%d)\n",
                sock, port ) );

    sock = socket( AF_INET, SOCK_STREAM, 0 );
    if( sock == -1 ) {
        perror( "socket()" );
        return -1;
    }

    memset( &local_addr, 0, sizeof( local_addr ) );
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl( INADDR_ANY );

    if( port != 0 ) {
        local_addr.sin_port = htons( port );
        if( bind( sock, (sockaddr*) & local_addr, sizeof( local_addr ) ) ==
            -1 ) {
            perror( "bind()" );
            return -1;
        }
    }
    else {
        // port = 7000;

        // let the system assign a port
        local_addr.sin_port = htons( port );
        while( bind( sock, (sockaddr*) & local_addr, sizeof( local_addr ) ) ==
               -1 ) {

            int err = XPlat::NetUtils::GetLastError();
            if( XPlat::Error::EAddrInUse( err ) ) {
                local_addr.sin_port = htons( ++port );
                continue;
            }
            else {
                mrn_dbg( 1, mrn_printf(FLF, stderr, "bind failed: %s\n",
                    XPlat::Error::GetErrorString( err ).c_str() ) );
                return -1;
            }
        }
    }

    if( listen( sock, 64 ) == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "%s", "" ) );
        perror( "listen()" );
        return -1;
    }

#if defined(TCP_NODELAY)
    // turn off Nagle algorithm for coalescing packets
    int optVal = 1;
    int ssoret = setsockopt( sock,
                             IPPROTO_TCP,
                             TCP_NODELAY,
                             (const char*)&optVal,
                             sizeof( optVal ) );
    if( ssoret == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "failed to set TCP_NODELAY\n" ) );
    }
#endif // defined(TCP_NODELAY)

    // determine which port we were actually assigned to
    if( getPortFromSocket( sock, port_in ) != 0 )
    {
        mrn_dbg( 1, mrn_printf(FLF, stderr,
            "failed to obtain port from socket\n" ) );
        XPlat::SocketUtils::Close( sock );
        return -1;
    }
    
    *sock_in = sock;
    mrn_dbg( 3, mrn_printf(FLF, stderr,
                "Leaving bind_to_port(). Returning sock:%d, port:%d\n",
                sock, port ) );
    return 0;
}

int getSocketConnection( int bound_socket )
{
    int connected_socket;

    mrn_dbg( 3, mrn_printf(FLF, stderr, "In get_connection(sock:%d).\n",
                bound_socket ) );

    do{
        connected_socket = accept( bound_socket, NULL, NULL );
        if( connected_socket == -1 ) {
            mrn_dbg( 1, mrn_printf(FLF, stderr, "%s", "" ) );
            perror( "accept()" );

            if ( errno != EINTR ) {
                return -1;
            }
        }
    } while ( ( connected_socket == -1 ) && ( errno == EINTR ) );

#if defined(TCP_NODELAY)
    // turn off Nagle algorithm for coalescing packets
    int optVal = 1;
    int ssoret = setsockopt( connected_socket,
                             IPPROTO_TCP,
                             TCP_NODELAY,
                             (const char*)&optVal,
                             sizeof( optVal ) );
    if( ssoret == -1 ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr, "failed to set TCP_NODELAY\n" ) );
    }
#endif // defined(TCP_NODELAY)

    mrn_dbg( 3, mrn_printf(FLF, stderr,
                "Leaving get_connection(). Returning sock:%d\n",
                connected_socket ) );
    return connected_socket;
}


int getPortFromSocket( int sock, Port *port )
{
    struct sockaddr_in local_addr;
    socklen_t sockaddr_len = sizeof( local_addr );


    if( getsockname( sock, (sockaddr*) & local_addr, &sockaddr_len ) == -1 ) {
        perror( "getsockname" );
        return -1;
    }

    *port = ntohs( local_addr.sin_port );
    return 0;
}



int mrn_printf( const char *file, int line, const char * /* func */,
                FILE * fp, const char *format, ... )
{
    int retval;
    va_list arglist;

    if( file ) {
        // get thread name
        const char *thread_name = NULL;

        tsd_t *tsd = ( tsd_t * )tsd_key.Get();
        if( tsd != NULL ) {
            thread_name = tsd->thread_name;
        }
            
        fprintf( fp, "%s: %s: %d: ",
                 ( thread_name != NULL ) ? thread_name : "<unknown thread>",
                 XPlat::PathUtils::GetFilename( file ).c_str(),
                 line );
    }

    va_start( arglist, format );
    retval = vfprintf( fp, format, arglist );
    va_end( arglist );
    fflush( fp );

    return retval;
}

}                               // namespace MRN
