/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: NetUtils.C,v 1.4 2004/06/01 16:57:21 pcroth Exp $
#include <sstream>
#include "xplat/Types.h"
#include "xplat/NetUtils.h"


namespace XPlat
{

std::string
NetUtils::FindNetworkName( void )
{
    std::string ipaddr = GetNetworkAddress().GetString();
    std::string ret;

    // do the lookup
    struct in_addr sin;
    struct hostent* hp = gethostbyname( ipaddr.c_str() );   // just copies addr
    memcpy( &sin.s_addr, hp->h_addr_list[0], hp->h_length );

    hp = gethostbyaddr( &sin, sizeof(sin), AF_INET );
    if( hp != NULL )
    {
        // we found our network name
        ret = hp->h_name;
    }

    return ret;
}

std::string
NetUtils::FindHostName( void )
{
    std::string fqdn = GetNetworkName();
    std::string ret;

    // extract host name from the fully-qualified domain name
    std::string::size_type firstDotPos = fqdn.find_first_of( '.' );
    if( firstDotPos != std::string::npos )
    {
        ret = fqdn.substr( 0, firstDotPos - 1 );
    }
    else
    {
        ret = fqdn;
    }
    return ret;
}


// check whether given IPv4 address (in host byte order) is local host
bool
NetUtils::IsLocalHost( in_addr_t addr )
{
    // do the lookup
    std::string ipaddr = GetNetworkAddress().GetString();
    struct hostent* hp = gethostbyname( ipaddr.c_str() );   // just copies addr

    in_addr sin;
    memcpy( &sin.s_addr, hp->h_addr_list[0], hp->h_length );

    return (htonl(addr) == sin.s_addr);
}


bool
NetUtils::IsLocalHost( const std::string& host )
{
    bool ret = false;

    if( (host == "") ||
        (host == "localhost") ||
        (host == GetNetworkName()) ||
        (host == GetNetworkAddress().GetString()) )
    {
        ret = true;
    }
    return ret;
}


std::string
NetUtils::GetHostName( void )
{
    static std::string cachedName;

    // check if we've already looked up our host name
    if( cachedName.length() > 0 )
    {
        return cachedName;
    }

    cachedName = FindHostName();

    return cachedName;
}


std::string
NetUtils::GetNetworkName( void )
{
    static std::string cachedName;

    // check if we've already looked up our fully qualified domain name
    if( cachedName.length() > 0 )
    {
        return cachedName;
    }

    // we didn't have the network name cached - 
    cachedName = FindNetworkName();

    return cachedName;
}


NetUtils::NetworkAddress
NetUtils::GetNetworkAddress( void )
{
    static NetUtils::NetworkAddress cachedLocalAddr( ntohl( INADDR_NONE ) );

    // check if we've already looked up our network address
    if( cachedLocalAddr.GetInAddr() == ntohl( INADDR_NONE ) ) 
    {
        // we didn't have the network address cached - 
        // look it up
        cachedLocalAddr = FindNetworkAddress();
    }
    return cachedLocalAddr;
}


NetUtils::NetworkAddress
NetUtils::GetAddressOfHost( std::string host )
{
    NetworkAddress ret( INADDR_NONE );


    hostent* hp = gethostbyname( host.c_str() );
    if( hp != NULL )
    {
        // we found the address
        ret = NetworkAddress( ntohl( *(in_addr_t*)(hp->h_addr_list[0]) ) );
    }

    return ret;
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


} // namespace XPlat
