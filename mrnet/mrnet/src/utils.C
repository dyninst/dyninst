/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

#include "mrnet/src/utils.h"
#include "mrnet/src/Types.h"
#include "src/config.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <map>

#if !defined(WIN32)
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif // defined(WIN32)

#if defined(solaris)
#include <sys/sockio.h>         //only for solaris
#endif // defined(solaris)

#include "xplat/NetUtils.h"
#include "xplat/PathUtils.h"
#include "xplat/Error.h"

#include "mrnet/MRNet.h"


namespace MRN
{

std::string LocalHostName="";
Port LocalPort=0;

XPlat::TLSKey tsd_key;


int connectHost( int *sock_in, const std::string & hostname, Port port )
{
    int sock = *sock_in;
    struct sockaddr_in server_addr;
    struct hostent *server_hostent;

    mrn_printf( 3, MCFL, stderr, "In connect_to_host(%s:%d) ...\n",
                hostname.c_str(  ), port );

    if( sock == 0 ) {
        sock = socket( AF_INET, SOCK_STREAM, 0 );

        if( sock == -1 ) {
            mrn_printf( 1, MCFL, stderr, "socket() failed\n" );
            perror( "socket()" );
            return -1;
        }
    }

    server_hostent = gethostbyname( hostname.c_str(  ) );

    if( server_hostent == NULL ) {
        perror( "gethostbyname()" );
        return -1;
    }

    memset( &server_addr, 0, sizeof( server_addr ) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons( port );
    memcpy( &server_addr.sin_addr, server_hostent->h_addr_list[0],
            sizeof( struct in_addr ) );

    unsigned int nConnectTries = 0;
    int cret = -1;
    while( ( cret == -1 ) && ( nConnectTries < 5 ) ) {
        cret =
            connect( sock, (sockaddr *) & server_addr, sizeof( server_addr ) );
        if( cret == -1 ) {
            int err = XPlat::NetUtils::GetLastError();
            if( !XPlat::Error::ETimedOut( err ) ) {
                mrn_printf( 1, MCFL, stderr, "connect() failed: %s\n",
                            XPlat::Error::GetErrorString( err ).c_str() );
                return -1;
            }
            nConnectTries++;
            mrn_printf( 3, MCFL, stderr, "connection timed out %d times\n",
                        nConnectTries );
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
        mrn_printf( 1, MCFL, stderr, "failed to set TCP_NODELAY\n" );
    }
#endif // defined(TCP_NODELAY)

    mrn_printf( 3, MCFL, stderr,
                "Leaving Connect_to_host(). Returning sock: %d\n", sock );
    *sock_in = sock;
    return 0;
}

int bindPort( int *sock_in, Port *port_in )
{
    int sock = *sock_in;
    Port port = *port_in;
    struct sockaddr_in local_addr;

    mrn_printf( 3, MCFL, stderr, "In bind_to_port(sock:%d, port:%d)\n",
                sock, port );

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
                mrn_printf( 1, MCFL, stderr, "bind failed: %s\n",
                    XPlat::Error::GetErrorString( err ).c_str() );
                return -1;
            }
        }
    }

    if( listen( sock, 64 ) == -1 ) {
        mrn_printf( 1, MCFL, stderr, "%s", "" );
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
        mrn_printf( 1, MCFL, stderr, "failed to set TCP_NODELAY\n" );
    }
#endif // defined(TCP_NODELAY)

    // determine which port we were actually assigned to
    if( getPortFromSocket( sock, port_in ) != 0 )
    {
        mrn_printf( 1, MCFL, stderr,
            "failed to obtain port from socket\n" );
        close( sock );
        return -1;
    }
    
    *sock_in = sock;
    mrn_printf( 3, MCFL, stderr,
                "Leaving bind_to_port(). Returning sock:%d, port:%d\n",
                sock, port );
    return 0;
}

int getSocketConnection( int bound_socket )
{
    int connected_socket;

    mrn_printf( 3, MCFL, stderr, "In get_connection(sock:%d).\n",
                bound_socket );

    connected_socket = accept( bound_socket, NULL, NULL );
    if( connected_socket == -1 ) {
        mrn_printf( 1, MCFL, stderr, "%s", "" );
        perror( "accept()" );
        return -1;
    }

#if defined(TCP_NODELAY)
    // turn off Nagle algorithm for coalescing packets
    int optVal = 1;
    int ssoret = setsockopt( connected_socket,
                             IPPROTO_TCP,
                             TCP_NODELAY,
                             (const char*)&optVal,
                             sizeof( optVal ) );
    if( ssoret == -1 ) {
        mrn_printf( 1, MCFL, stderr, "failed to set TCP_NODELAY\n" );
    }
#endif // defined(TCP_NODELAY)

    mrn_printf( 3, MCFL, stderr,
                "Leaving get_connection(). Returning sock:%d\n",
                connected_socket );
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

// get the current (localhost) machine name, e.g. "grilled"
int getHostName( std::string & out_hostname, const std::string & in_name )
{
    static std::string local_hostname = "";
    static bool first_time = true;
    std::string network_name;

    if( ( in_name == "" ) && ( !first_time ) ) {
        out_hostname = local_hostname;
        return 0;
    }

    if( getNetworkName( network_name, in_name ) == -1 ) {
        // cannot determine network name
        return -1;
    }

    //TODO: need to add check for IP addresses returned by getNetworkName()

    int idx = network_name.find( '.' );
    if( idx != -1 ) {
        out_hostname = network_name.substr( 0, idx );
    }
    else {
        out_hostname = network_name;
    }

    if( in_name == "" ) {
        local_hostname = out_hostname;
        LocalHostName = local_hostname;
        first_time = false;
    }
    return 0;
}

// get the network domain name from the given hostname (default=localhost)
// e.g. "grilled.cs.wisc.edu" -> "cs.wisc.edu"
int getDomainName( std::string & domainname,
                   const std::string & in_hostname )
{
    static std::string local_domainname = "";
    static bool first_time = true;
    std::string network_name;

    if( ( in_hostname == "" ) && ( !first_time ) ) {
        domainname = local_domainname;
        return 0;
    }

    if( getNetworkName( network_name, in_hostname ) == -1 ) {
        // cannot determine network name
        return -1;
    }

    //TODO: need to add check for IP addresses returned by getNetworkName()

    int idx = network_name.find( '.' );
    if( idx == -1 ) {
        //no "." found in network_name
        return -1;
    }
    else {
        domainname =
            network_name.substr( idx + 1, network_name.length(  ) );
    }

    if( in_hostname == "" ) {
        local_domainname = domainname;
        first_time = false;
    }

    return 0;
}

// get the fully-qualified network name for given hostname (default=localhost)
// e.g. "grilled" -> "grilled.cs.wisc.edu"
int getNetworkName( std::string & network_name, const std::string & in_hostname )
{
    static std::string local_network_name( "" );
    static bool first_time = true;
    struct in_addr in;
    struct hostent *hp;

    if( ( in_hostname == "" ) && ( !first_time ) ) {
        network_name = local_network_name;
        return 0;
    }

    std::string ip_address;
    if( getNetworkAddr( ip_address, in_hostname ) == -1 ) {
        //cannot get network address
        return -1;
    }

    // use to initialize struct in_addr
    // since inet_pton not available on windows
    hp = gethostbyname( ip_address.c_str(  ) );
    if( hp == NULL ) {
        mrn_printf( 1, MCFL, stderr, "Host information not found for \"%s\"\n",
                    ip_address.c_str(  ) );
        return -1;
    }
    memcpy( ( void * )( &in.s_addr ), ( void * )( hp->h_addr_list[0] ),
            hp->h_length );

#if defined(solaris) || defined(WIN32)
    hp = gethostbyaddr( ( const char * )&in, sizeof( in ), AF_INET );
#else
    hp = gethostbyaddr( ( void * )&in, sizeof( in ), AF_INET );
#endif

    if( hp == NULL ) {
        mrn_printf( 1, MCFL, stderr, "Host information not found for %s;"
                    " Using IP address!\n", ip_address.c_str(  ) );
        network_name = ip_address;
        if( in_hostname == "" ) {
            local_network_name = network_name;
            first_time = false;
        }
        return 0;
    }
    network_name = hp->h_name;

    int idx = network_name.find( '.' );
    if( idx == -1 ) {
        //no "." found in network_name
        mrn_printf( 1, MCFL, stderr,
                    "networkname is not fully qualified (%s);"
                    "); using IP address!\n", network_name.c_str(  ) );
        network_name = ip_address;
        return -1;
    }

    if( in_hostname == "" ) {
        local_network_name = network_name;
        first_time = false;
    }

    return 0;
}

int getNetworkAddr( std::string & ipaddr, const std::string hostname )
{
    static std::string local_ipaddr = "";
    static bool first_time = true;
    struct hostent *hp;

    if( hostname == "" ) {
        // find this machine's hostname
        if( first_time ) {
            local_ipaddr = XPlat::NetUtils::GetNetworkAddress().GetString();
            if( local_ipaddr.length() == 0 ) {
                mrn_printf( 1, MCFL, stderr, "get_local_ip_address() failed\n" );
                return -1;
            }
        }
        first_time = false;
        ipaddr = local_ipaddr;
        return 0;
    }

    hp = gethostbyname( hostname.c_str(  ) );
    if( hp == NULL ) {
        mrn_printf( 1, MCFL, stderr, "Host information not found for \"%s\"\n",
                    hostname.c_str(  ) );
        return -1;
    }

    struct in_addr in;
    memcpy( &in.s_addr, *( hp->h_addr_list ), sizeof( in.s_addr ) );

    // Convert address to a string.
    // NOte that we use the XPlat utility function to convert the address
    // because inet_ntoa is not (necessarily) thread-safe and other
    // alternatives (e.g., inet_ntop) are not yet available on all platforms
    // of interest.
    XPlat::NetUtils::NetworkAddress tmpAddr( ntohl( in.s_addr ) );
    ipaddr = tmpAddr.GetString();
    return 0;
}



int mrn_printf( int level, const char *file, int line, FILE * fp,
                const char *format, ... )
{
    int retval;
    va_list arglist;

    if( level > OUTPUT_LEVEL ) {
        return 0;
    }

    if( file ) {
        // get thread name
        const char *thread_name = NULL;
        tsd_t *tsd = ( tsd_t * )tsd_key.Get();
        if( tsd != NULL ) {
            thread_name = tsd->thread_name;
        }

        fprintf( fp, "%s:%s:%d: ",
                 ( thread_name !=
                   NULL ) ? thread_name : "<noname (tsd NULL)>",
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
