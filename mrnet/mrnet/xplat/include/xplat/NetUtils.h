/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: NetUtils.h,v 1.4 2004/06/01 18:23:49 pcroth Exp $
#ifndef XPLAT_NETUTILS_H
#define XPLAT_NETUTILS_H

#include <string>
#include "xplat/Types.h"

namespace XPlat
{

class NetUtils
{
public:
    class NetworkAddress
    {
    private:
        std::string str;        // IPv4 address in dotted decimal
        in_addr_t iaddr;        // IPv4 address in host byte order

    public:
        NetworkAddress( in_addr_t inaddr );
        NetworkAddress( const NetworkAddress& obj )
          : str( obj.str ),
            iaddr( obj.iaddr )
        { }

        NetworkAddress&
        operator=( const NetworkAddress& obj )
        {
            if( &obj != this )
            {
                str = obj.str;
                iaddr = obj.iaddr;
            }
            return *this;
        }
            

        std::string GetString( void ) const { return str; }
        in_addr_t GetInAddr( void ) const   { return iaddr; }
    };

private:
    static std::string FindHostName( void );
    static std::string FindNetworkName( void );
    static NetworkAddress FindNetworkAddress( void );

public:
    // get IPv4 info about the local host
    static std::string GetHostName( void );
    static std::string GetNetworkName( void );
    static NetworkAddress GetNetworkAddress( void );

    // get IPv4 info about a host
    static NetworkAddress GetAddressOfHost( std::string host );

    // check whether given host is local 
    static bool IsLocalHost( const std::string& host );
    static bool IsLocalHost( in_addr_t addr );

    static int GetLastError( void );
};

} // namespace XPlat

#endif // XPLAT_NETUTILS_H
