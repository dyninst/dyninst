/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: NetUtils.C,v 1.9 2007/03/15 20:11:07 darnold Exp $
#include <sstream>
#include "xplat/Types.h"
#include "xplat/NetUtils.h"

#if defined(os_windows)
#include <ws2tcpip.h>
#endif /* os_windows */

namespace XPlat
{

int NetUtils::FindNetworkName( std::string ihostname, std::string & ohostname )
{
    struct addrinfo *addrs, hints;
    int error;

    if( ihostname == "" ){
        return -1;
    }

    // do the lookup
    memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_CANONNAME;
    hints.ai_socktype = SOCK_STREAM;
    if ( error = getaddrinfo(ihostname.c_str(), NULL, NULL, &addrs)) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(error));
        return -1;
    }

    char hostname[256];
    if( error = getnameinfo(addrs->ai_addr, sizeof(struct sockaddr), hostname, sizeof(hostname), NULL,0,0) ){
        fprintf(stderr, "getnameinfo(): %s\n", gai_strerror(error));
        return -1;
    }

    ohostname = hostname;

    return 0;
}

int NetUtils::FindHostName( std::string ihostname, std::string & ohostname)
{
    std::string fqdn;
    if( GetNetworkName( ihostname, fqdn ) == -1 ){
        return -1;
    }

    // extract host name from the fully-qualified domain name
    std::string::size_type firstDotPos = fqdn.find_first_of( '.' );
    if( firstDotPos != std::string::npos ) {
        ohostname = fqdn.substr( 0, firstDotPos );
    }
    else {
        ohostname = fqdn;
    }

    return 0;
}


bool
NetUtils::IsLocalHost( const std::string& ihostname )
{

    std::vector< NetworkAddress > local_addresses;
    GetLocalNetworkInterfaces( local_addresses );

    NetworkAddress iaddress;
    if( GetNetworkAddress( ihostname, iaddress ) == -1 ){
        return false;
    }

    for( unsigned int i=0; i<local_addresses.size(); i++ ) {
        if( local_addresses[i] == iaddress ){
            return true;
        }
    }

    return false;
}


int NetUtils::GetHostName( std::string ihostname, std::string &ohostname )
{
    static std::string cachedName;

    // check if we've already looked up our host name
    if( ihostname == "") {
        if(cachedName.length() == 0 ) {
            if( FindHostName( ihostname, cachedName ) == -1 ){
                return -1;
            }
        }

        ohostname = cachedName;
        return 0;
    }
    else{
        return FindHostName( ihostname, ohostname );
    }
}


int NetUtils::GetNetworkName( std::string ihostname, std::string & ohostname )
{
    static std::string cachedName;

    // check if we've already looked up our fully qualified domain name
    if( ihostname == "" ){
        if ( cachedName.length() == 0 ) {
            if( FindNetworkName( ihostname, cachedName ) == -1 ){
                return -1;
            }
        }

        ohostname = cachedName;
        return 0;
    }
    else{
        return FindNetworkName( ihostname, ohostname );
    }
}


int NetUtils::GetNetworkAddress( std::string ihostname, NetworkAddress & oaddr )
{
    static NetUtils::NetworkAddress cachedLocalAddr( ntohl( INADDR_ANY ) );

    // check if we've already looked up our network address
    if( ihostname == "" ){
        if( cachedLocalAddr.GetInAddr() == ntohl( INADDR_ANY ) ) {
            // we didn't have the network address cached - look it up
            if( FindNetworkAddress( ihostname, cachedLocalAddr  ) == -1 ){
                return -1;
            }
        }
        oaddr = cachedLocalAddr;
        return 0;
    }
    else{
        return FindNetworkAddress( ihostname, oaddr );
    }
}

// Note: does not use inet_ntoa or similar functions because they are not
// necessarily thread safe, or not available on all platforms of interest.
NetUtils::NetworkAddress::NetworkAddress( in_addr_t inaddr )
  : iaddr( inaddr )
{
    // find the dotted decimal form of the address

    // get address in network byte order
    in_addr_t nboaddr = htonl( iaddr );

    // access the address as an array of bytes
    const unsigned char* cp = (const unsigned char*)&nboaddr;

    std::ostringstream astr;
    astr << (unsigned int)cp[0] 
        << '.' << (unsigned int)cp[1] 
        << '.' << (unsigned int)cp[2] 
        << '.' << (unsigned int)cp[3];

    str = astr.str();
}

int NetUtils::GetNumberOfNetworkInterfaces( void )
{
    static int cachedNumberOfInterfaces=0;

    if( cachedNumberOfInterfaces == 0 ) {
        cachedNumberOfInterfaces = FindNumberOfLocalNetworkInterfaces( );
    }

    return cachedNumberOfInterfaces;
}

int NetUtils::GetLocalNetworkInterfaces( std::vector<NetUtils::NetworkAddress> & iaddresses )
{
    static std::vector<NetUtils::NetworkAddress> cachedLocalAddresses;

    if( cachedLocalAddresses.size() == 0 ){
        int ret = FindLocalNetworkInterfaces( cachedLocalAddresses );
        if( ret == -1 )
            return -1;
    } 

    iaddresses = cachedLocalAddresses;
    return 0;
}

int NetUtils::FindNetworkAddress( std::string ihostname, NetUtils::NetworkAddress &oaddr )
{
    struct addrinfo *addrs, hints;
    int error;

    if( ihostname == "" ){
        return -1;
    }

    // do the lookup
    memset( &hints, 0, sizeof(hints) );
    hints.ai_flags = AI_CANONNAME;
    hints.ai_socktype = SOCK_STREAM;
    if ( error = getaddrinfo(ihostname.c_str(), NULL, NULL, &addrs)) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(error));
        return -1;
    }

    struct in_addr in;
    struct sockaddr_in *sinptr = ( struct sockaddr_in * )(addrs->ai_addr);
    memcpy( &in.s_addr, ( void * )&( sinptr->sin_addr ), sizeof( in.s_addr ) );
    oaddr = NetworkAddress( ntohl(in.s_addr) );
    return 0;
}

} // namespace XPlat
