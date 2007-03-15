/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: NetUtils.h,v 1.6 2007/03/15 20:11:03 darnold Exp $
#ifndef XPLAT_NETUTILS_H
#define XPLAT_NETUTILS_H

#include <string>
#include <vector>
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
        NetworkAddress( ): str(""), iaddr( ntohl(INADDR_ANY) ){}
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
            
        bool operator==( const NetworkAddress & in )
            { return iaddr == in.iaddr; }

        std::string GetString( void ) const { return str; }
        in_addr_t GetInAddr( void ) const   { return iaddr; }
    };
private:
    static int FindNetworkName( std::string ihostname, std::string & );
    static int FindHostName( std::string ihostname, std::string & );
    static int FindNetworkAddress( std::string ihostname, NetworkAddress & );
    static int FindNumberOfLocalNetworkInterfaces( void ); 
    static int FindLocalNetworkInterfaces( std::vector<NetworkAddress> & );

public:
    static int GetHostName( std::string ihostname, std::string & );
    static int GetNetworkName( std::string ihostname, std::string & );
    static int GetNetworkAddress( std::string ihostname, NetworkAddress & );
    static int GetNumberOfNetworkInterfaces( void );
    static int GetLocalNetworkInterfaces( std::vector<NetworkAddress> & );

    // check whether given host is local 
    static bool IsLocalHost( const std::string& host );

    static int GetLastError( void );
};

} // namespace XPlat

#endif // XPLAT_NETUTILS_H
