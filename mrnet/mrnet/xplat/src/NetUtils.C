/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: NetUtils.C,v 1.1 2003/11/14 19:27:03 pcroth Exp $
#include "xplat/Types.h"
#include "xplat/NetUtils.h"


namespace XPlat
{

std::string
NetUtils::FindNetworkName( void )
{
    std::string ipaddr = GetNetworkAddress();
    std::string ret;

    // do the lookup
    int err = -1;
    struct hostent* hp = gethostbyname( ipaddr.c_str() );
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


bool
NetUtils::IsLocalHost( const std::string& host )
{
    bool ret = false;

    if( (host == "") ||
        (host == "localhost") ||
        (host == GetNetworkName()) ||
        (host == GetNetworkAddress()) )
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

std::string
NetUtils::GetNetworkAddress( void )
{
    static std::string cachedAddress;

    // check if we've already looked up our network address
    if( cachedAddress.length() > 0 )
    {
        return cachedAddress;
    }

    // we didn't have the network address cached - 
    // look it up
    cachedAddress = FindNetworkAddress();

    return cachedAddress;
}

} // namespace XPlat
