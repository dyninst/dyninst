/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: NetUtils.h,v 1.2 2004/03/23 01:12:22 eli Exp $
#ifndef XPLAT_NETUTILS_H
#define XPLAT_NETUTILS_H

#include <string>

namespace XPlat
{

class NetUtils
{
private:
    static std::string FindHostName( void );
    static std::string FindNetworkName( void );
    static std::string FindNetworkAddress( void );

public:
    static std::string GetHostName( void );
    static std::string GetNetworkName( void );
    static std::string GetNetworkAddress( void );

    static bool IsLocalHost( const std::string& host );

    static int GetLastError( void );
};

} // namespace XPlat

#endif // XPLAT_NETUTILS_H
