#include "mrnet/src/utils.h"
#include "src/config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <netdb.h>
#include <stdio.h>
#include <stdarg.h>
#include <libgen.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <string>
#include <map>
#include <dlfcn.h>

#define SA struct sockaddr

#if !defined(i386_unknown_nt4_0)
#include <sys/ioctl.h>
#include <net/if.h>
#else
#include <Iphlpapi.h>
#include <Iptypes.h>
#endif // defined(i386_unknown_nt4_0)

//LOOPBACK_IP used for hack to detect loopback interface
//            must pay attention to endian-ness
#if defined(sparc_sun_solaris2_4)
#include <sys/sockio.h>         //only for solaris
#define LOOPBACK_IP 2130706433
#else
#define LOOPBACK_IP 16777343
#endif

namespace MRN
{

std::string LocalHostName="";
unsigned short LocalPort=0;

static int get_local_ip_address( std::string & ip_address );

pthread_key_t tsd_key;

int createProcess( const std::string & remote_shell,
                   const std::string & hostName,
                   const std::string & userName,
                   const std::string & command,
                   const std::vector < std::string > &arg_list )
{
    std::string in_nname, local_nname, in_naddr, local_naddr;
    getNetworkName( local_nname );
    getNetworkName( in_nname, hostName );
    getNetworkAddr( local_naddr );
    getNetworkAddr( in_naddr, hostName );

    if( ( hostName == "" ) ||
        ( hostName == "localhost" ) ||
        ( in_nname == local_nname ) || ( in_naddr == local_naddr ) ) {
      return execCmd( command, arg_list );
    }
    else if( remote_shell.length(  ) > 0 ) {
      return remoteCommand( remote_shell, hostName, userName,
			    command, arg_list );
    }
    else {
        return rshCommand( hostName, userName, command, arg_list );
    }
}

// directly exec the command (local).
int execCmd( const std::string command,
             const std::vector < std::string > &args )
{
    int ret, i;
    int arglist_len = args.size(  );
    char **arglist = new char *[arglist_len + 2];
    char *cmd = strdup( command.c_str(  ) );

    mrn_printf( 3, MCFL, stderr, "In execCmd(%s) with %d args\n",
                cmd, arglist_len );

    arglist[0] = strdup( basename( cmd ) );
    free( cmd );            //basename may modify!
    arglist[arglist_len + 1] = NULL;
    for( i = 0; i < arglist_len; ++i ) {
        arglist[i + 1] = strdup( args[i].c_str(  ) );
    }

    ret = fork(  );
    if( ret == 0 ) {
        mrn_printf( 3, MCFL, stderr, "Forked child calling execvp:" );
        for( i = 0; arglist[i] != NULL; i++ ) {
            mrn_printf( 3, 0, 0, stderr, "%s ", arglist[i] );
        }
        mrn_printf( 3, 0, 0, stderr, "\n" );

        execvp( command.c_str(  ), arglist );
        perror( "exec()" );
        delete [] arglist;
        exit( -1 );
    }

    for( i = 0; i < arglist_len+1; ++i ) {
        free(arglist[i]);
    }
    delete [] arglist;
    return ( ret == -1 ? -1 : 0 );
}

// Execute 'command' on a remote machine using 'remote_shell' (which can 
// include arguments) passing an argument list of 'arg_list'

int remoteCommand( const std::string remoteExecCmd,
                   const std::string hostName,
                   const std::string userName,
                   const std::string command,
                   const std::vector < std::string > &arg_list )
{
    unsigned int i;
    std::vector < std::string > remoteExecArgList;
    std::vector < std::string > tmp;
    std::string cmd;

    mrn_printf( 3, MCFL, stderr, "In remoteCommand()\n" );
#if defined(DEFAULT_RUNAUTH_COMMAND)
    //might be necessary to call runauth to pass credentials
    cmd = DEFAULT_RUNAUTH_COMMAND;
    remoteExecArgList.push_back( remoteExecCmd );
#else
    cmd = remoteExecCmd;
#endif

    // add the hostname and username to arglist
    remoteExecArgList.push_back( hostName );
    if( userName.length(  ) > 0 ) {
        remoteExecArgList.push_back( std::string( "-l" ) );
        remoteExecArgList.push_back( userName );
    }

    // add remote command and its arguments
    remoteExecArgList.push_back( command );
    for( i = 0; i < arg_list.size(  ); i++ ) {
        remoteExecArgList.push_back( arg_list[i] );
    }
    //remoteExecArgList.push_back("-l0");

    // execute the command
    mrn_printf( 3, MCFL, stderr, "Calling execCmd: %s ", cmd.c_str(  ) );
    for( i = 0; i < remoteExecArgList.size(  ); i++ ) {
        mrn_printf( 3, 0, 0, stderr, "%s ",
                    remoteExecArgList[i].c_str(  ) );
    }
    mrn_printf( 3, 0, 0, stderr, "\n" );

    if( execCmd( cmd, remoteExecArgList ) == -1 ) {
        mrn_printf( 1, MCFL, stderr, "execCmd() failed\n" );
        return -1;
    }

    mrn_printf( 3, MCFL, stderr, "Leaving remoteCommand()\n" );
    return 0;
}

// use rsh to get a remote process started.
int rshCommand( const std::string & hostName,
                const std::string & userName,
                const std::string & command,
                const std::vector < std::string > &arg_list )
{
    // ensure we know the user's desired rsh command
    std::string rshCmd;
    const char *rsh = getenv( RSH_COMMAND_ENV );
    if( rsh == NULL ) {
        rshCmd = DEFAULT_RSH_COMMAND;
    }
    else {
        rshCmd = rsh;
    }

    mrn_printf( 3, MCFL, stderr,
                "In rshCmd(). Calling remoteCmd(%s, %s, %s)\n",
                rshCmd.c_str(  ), hostName.c_str(  ), command.c_str(  ) );
    return remoteCommand( rshCmd, hostName, userName, command, arg_list );
}


int connectHost( int *sock_in, const std::string & hostname,
                 unsigned short port )
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
            connect( sock, ( SA * ) & server_addr, sizeof( server_addr ) );
        if( cret == -1 ) {
            if( errno != ETIMEDOUT ) {
                perror( "connect()" );
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
                             &optVal,
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

int bindPort( int *sock_in, unsigned short *port_in )
{
    int sock = *sock_in;
    unsigned short port = *port_in;
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
        if( bind( sock, ( SA * ) & local_addr, sizeof( local_addr ) ) ==
            -1 ) {
            perror( "bind()" );
            return -1;
        }
    }
    else {
        port = 7000;
        local_addr.sin_port = htons( port );
        while( bind( sock, ( SA * ) & local_addr, sizeof( local_addr ) ) ==
               -1 ) {
            if( errno == EADDRINUSE ) {
                local_addr.sin_port = htons( ++port );
                continue;
            }
            else {
                perror( "bind()" );
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
                             &optVal,
                             sizeof( optVal ) );
    if( ssoret == -1 ) {
        mrn_printf( 1, MCFL, stderr, "failed to set TCP_NODELAY\n" );
    }
#endif // defined(TCP_NODELAY)

    *sock_in = sock;
    *port_in = port;
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
                             &optVal,
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

int getSocketPeer( int connected_socket, std::string & hostname,
                   unsigned short *port )
{
    struct sockaddr_in peer_addr;
    socklen_t peer_addrlen = sizeof( peer_addr );
    char buf[256];

    mrn_printf( 3, MCFL, stderr, "In get_socket_peer()\n" );

    if( getpeername( connected_socket, ( struct sockaddr * )( &peer_addr ),
                     &peer_addrlen ) == -1 ) {
        mrn_printf( 1, MCFL, stderr, "%s", "" );
        perror( "getpeername()" );
        return -1;
    }

    if( inet_ntop( AF_INET, &peer_addr.sin_addr, buf, sizeof( buf ) ) ==
        NULL ) {
        mrn_printf( 1, MCFL, stderr, "%s", "" );
        perror( "inet_ntop()" );
        return -1;
    }

    *port = ntohs( peer_addr.sin_port );
    hostname = buf;
    mrn_printf( 3, MCFL, stderr,
                "Leaving get_socket_peer(). Returning %s:%d\n",
                hostname.c_str(  ), *port );
    return 0;
}

int getPortFromSocket( int sock, unsigned short *port )
{
    struct sockaddr_in local_addr;
    socklen_t sockaddr_len = sizeof( local_addr );


    if( getsockname( sock, ( SA * ) & local_addr, &sockaddr_len ) == -1 ) {
        perror( "getsockname" );
        return -1;
    }

    *port = local_addr.sin_port;
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

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_nt4_0)
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
            if( get_local_ip_address( local_ipaddr ) == -1 ) {
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

    ipaddr = inet_ntoa( in );
    return 0;
}

static int get_local_ip_address( std::string & ip_address )
{
    static std::string local_ip_address( "" );
    static bool first_call = true;
    char ip_address_buf[256];

    if( !first_call ) {
        ip_address = local_ip_address;
        return 0;
    }
    first_call = false;

#if !defined(i386_unknown_nt4_0)
    int sockfd, lastlen, len, firsttime, flags;
    char *buf, *cptr, *ptr, lastname[IFNAMSIZ];
    struct ifreq *ifr, ifrcopy;
    struct ifconf ifc;

    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    if( sockfd < 0 ) {
        perror( "Failed socket()" );
        return -1;
    }

    lastlen = 0;
    firsttime = 1;
    len = 3 * sizeof( struct ifreq );   // initial size guess
    while( 1 ) {
        buf = ( char * )malloc( len );
        assert( buf );
        ifc.ifc_len = len;
        ifc.ifc_buf = buf;
        //printf("\tCalling ioctl w/ len: %d ...\n", len);
#if !defined(aix)
        if( ioctl( sockfd, SIOCGIFCONF, &ifc ) < 0 )
#else
            if( ioctl( sockfd, CSIOCGIFCONF, &ifc ) < 0 )   //use on aix
#endif /* aix */
            {
                perror( "Failed ioctl()" );
                free( buf );
                return -1;
            }
            else {
                //printf("\tComparing %d and lastlen:%d ... \n", ifc.ifc_len, lastlen);
                if( ifc.ifc_len == lastlen ) {
                    //printf("ioctl success\n");
                    break;      //success, len has not changed
                }
                lastlen = ifc.ifc_len;
            }
        if( !firsttime ) {
            firsttime = 0;
            len += 5 * sizeof( struct ifreq );  /* increment size guess */
        }
        free( buf );
    }

    lastname[0] = 0;
    int i;
    for( ptr = buf, i = 0; ptr < buf + ifc.ifc_len;
         i++, ptr += sizeof( ifr->ifr_name ) + len ) {
        //printf("Processing interface %d\n", i);
        ifr = ( struct ifreq * )ptr;

        len = sizeof( struct sockaddr );

        if( ifr->ifr_addr.sa_family != AF_INET ) {
            //printf("\tIgnoring %s (wrong family)!\n", ifr->ifr_name );
            continue;       //ignore other address families
        }

        if( ( cptr = strchr( ifr->ifr_name, ':' ) ) != NULL ) {
            *cptr = 0;      // replace colon with null 
        }
        if( strncmp( lastname, ifr->ifr_name, IFNAMSIZ ) == 0 ) {
            //printf("\tIgnoring %s (alias)!\n", ifr->ifr_name );
            continue;
        }

        ifrcopy = *ifr;
        if( ioctl( sockfd, SIOCGIFFLAGS, &ifrcopy ) < 0 ) {
            perror( "Failed ioctl()" );
            free( buf );
            return -1;
        }
        flags = ifrcopy.ifr_flags;
        if( ( flags & IFF_UP ) == 0 ) {
            //printf("\tIgnoring %s (Not Up!)\n", ifr->ifr_name);
            continue;
        }

        struct in_addr in;
        struct sockaddr_in *sinptr = ( struct sockaddr_in * )&ifr->ifr_addr;
        memcpy( &in.s_addr, ( void * )&( sinptr->sin_addr ),
                sizeof( in.s_addr ) );
        if( in.s_addr == LOOPBACK_IP ) {
            //printf("\tIgnoring %s (loopback!)\n", ifr->ifr_name);
            continue;
        }

        if( inet_ntop( AF_INET, ( const void * )&in, ip_address_buf,
                       sizeof( ip_address_buf ) ) == NULL ) {
            perror( "Failed inet_ntop()" );
            free( buf );
            return -1;
        }
        ip_address = ip_address_buf;
        free( buf );
        return 0;
    }
#else /* i386_unknown_nt4_0 */
    unsigned long num_adapters;

    if( GetNumberOfInterfaces( &num_adapters ) != NO_ERROR ) {
        cerr << "Failed GetNumberOfInterfaces()" << endl;
    }
    num_adapters--;         //exclude loopback interface

    PIP_ADAPTER_INFO pAdapterInfo = new IP_ADAPTER_INFO[num_adapters];
    unsigned long OutBufLen = sizeof( IP_ADAPTER_INFO ) * num_adapters;

    if( GetAdaptersInfo( pAdapterInfo, &OutBufLen ) != ERROR_SUCCESS ) {
        cerr << "Failed GetAdaptersInfo()" << endl;
    }

    PIP_ADAPTER_INFO tmp_adapter_info;
    for( tmp_adapter_info = pAdapterInfo; tmp_adapter_info;
         tmp_adapter_info = tmp_adapter_info->Next ) {
        if( tmp_adapter_info->Type == MIB_IF_TYPE_ETHERNET ) {
            ip_address = tmp_adapter_info->IpAddressList.IpAddress.String;
            return 0;
        }
    }
#endif

    fprintf( stderr, "No network interface seems to be enabled. IP unknown!\n" );
    free( buf );
    return -1;
}

int get_IP_from_socket( int sock )
{
    struct sockaddr_in local_addr;
    socklen_t sockaddr_len = sizeof( local_addr );


    if( getsockname( sock, ( SA * ) & local_addr, &sockaddr_len ) == -1 ) {
        perror( "getsockname" );
        return -1;
    }

    return local_addr.sin_addr.s_addr;
}

int get_IP_from_name( char *name )
{
    struct hostent *_hostent;

    _hostent = gethostbyname( name );

    if( _hostent == NULL ) {
        perror( "gethostbyname()" );
        return -1;
    }

    return ( ( struct in_addr * )( _hostent->h_addr_list[0] ) )->s_addr;
}

int get_local_IP(  )
{
    struct hostent *hptr;
    struct utsname myname;

    if( uname( &myname ) < 0 ) {
        perror( "uname()" );
        return -1;
    }

    if( ( hptr = gethostbyname( myname.nodename ) ) == NULL ) {
        perror( "gethostbyname()" );
        return -1;
    }

    return ( ( struct in_addr * )( hptr->h_addr_list[0] ) )->s_addr;
}

int connect_socket_by_IP( int IP, short port )
{
    struct sockaddr_in server_addr;
    int sock;

    sock = socket( AF_INET, SOCK_STREAM, 0 );

    if( sock == -1 ) {
        perror( "socket()" );
        return -1;
    }

    memset( &server_addr, 0, sizeof( server_addr ) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons( port );
    server_addr.sin_addr.s_addr = IP;

    unsigned int nConnectTries = 0;
    int cret = -1;
    while( ( cret == -1 ) && ( nConnectTries < 5 ) ) {
        cret =
            connect( sock, ( SA * ) & server_addr, sizeof( server_addr ) );
        if( cret == -1 ) {
            if( errno != ETIMEDOUT ) {
                perror( "connect()" );
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
                             &optVal,
                             sizeof( optVal ) );
    if( ssoret == -1 ) {
        mrn_printf( 1, MCFL, stderr, "failed to set TCP_NODELAY\n" );
    }
#endif // defined(TCP_NODELAY)

    return sock;
}

void *getSharedObjectHandle( const char *so_file )
{
    static std::map < std::string, void *>SoHandleByFileName;
    void *so_handle;
    std::string so_key = so_file;
    std::map < std::string, void *>::iterator iter
        = SoHandleByFileName.find( so_key );

    if( iter == SoHandleByFileName.end(  ) ) {
        //shared object not yet loaded ...
        mrn_printf( 3, MCFL, stderr, "Loading so:%s\n", so_file );
        so_handle = dlopen( so_file, RTLD_LAZY | RTLD_GLOBAL );
        if( so_handle == NULL ) {
            mrn_printf( 1, MCFL, stderr, "%s\n", dlerror( ) );
            return NULL;
        }
        else {
            SoHandleByFileName[so_key] = so_handle;
        }
    }
    else {
        //shared object previously loaded, get stored handle from map ...
        so_handle = ( *iter ).second;
    }

    return so_handle;
}

void *getSymbolFromSharedObjectHandle( const char *sym, void *so_handle )
{
    char *error;
    void *sym_ptr = dlsym( so_handle, sym );

    if( ( error = dlerror(  ) ) != NULL ) {
        mrn_printf( 1, MCFL, stderr, "%s\n", error );
        return NULL;
    }
    return sym_ptr;
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
        // basename modifies 1st arg, so copy
        char tmp_filename[256];
        strncpy( tmp_filename, file, sizeof( tmp_filename ) );

        // get thread name
        const char *thread_name = NULL;
        tsd_t *tsd = ( tsd_t * ) pthread_getspecific( tsd_key );
        if( tsd != NULL ) {
            thread_name = tsd->thread_name;
        }

        fprintf( fp, "%s:%s:%d: ",
                 ( thread_name !=
                   NULL ) ? thread_name : "<noname (tsd NULL)>",
                 basename( tmp_filename ), line );
    }

    va_start( arglist, format );
    retval = vfprintf( fp, format, arglist );
    va_end( arglist );

    return retval;
}

}                               // namespace MRN
