/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: NetUtils-win.C,v 1.1 2003/11/14 19:27:03 pcroth Exp $
#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include "xplat/NetUtils.h"


namespace XPlat
{

std::string
NetUtils::FindNetworkAddress( void )
{
    std::string ret;

    unsigned long num_adapters;
    if( GetNumberOfInterfaces( &num_adapters ) != NO_ERROR ) {
        fprintf( stderr, "GetNumberOfInterfaces() failed\n" );
    }
    num_adapters--;         //exclude loopback interface, always last (?)

    PIP_ADAPTER_INFO pAdapterInfo = new IP_ADAPTER_INFO[num_adapters];
    unsigned long OutBufLen = sizeof( IP_ADAPTER_INFO ) * num_adapters;

    if( GetAdaptersInfo( pAdapterInfo, &OutBufLen ) != ERROR_SUCCESS ) {
        fprintf( stderr, "GetAdaptersInfo() failed\n" );
    }

    PIP_ADAPTER_INFO tmp_adapter_info;
    for( tmp_adapter_info = pAdapterInfo; tmp_adapter_info;
         tmp_adapter_info = tmp_adapter_info->Next ) {
        if( tmp_adapter_info->Type == MIB_IF_TYPE_ETHERNET ) {
            ret = tmp_adapter_info->IpAddressList.IpAddress.String;
            break;
        }
    }

    return ret;
}

int
NetUtils::GetLastError( void )
{
    return WSAGetLastError();
}


} // namespace XPlat
