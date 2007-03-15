/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: NetUtils-win.C,v 1.7 2007/03/15 20:11:06 darnold Exp $

#include <winsock2.h>
#include <Iphlpapi.h>
#include "xplat/NetUtils.h"


namespace XPlat
{

int NetUtils::FindNumberOfLocalNetworkInterfaces( void )
{
    PIP_ADAPTER_INFO pAdapterInfo = new IP_ADAPTER_INFO;
    unsigned long output_buffer_len = sizeof(IP_ADAPTER_INFO);

    // Make call to GetAdaptersInfo to get number of addapters
    if( GetAdaptersInfo( pAdapterInfo, &output_buffer_len ) == ERROR_BUFFER_OVERFLOW) {
        delete pAdapterInfo;
        //GetAdaptersInfo doesn't return loopback -- we add 1 for this interface
        return 1 + (output_buffer_len / sizeof(IP_ADAPTER_INFO)) ;
    }

    fprintf( stderr, "FindNumberOfNetworkInterfaces() failed\n" );
    return -1;
}


int NetUtils::FindLocalNetworkInterfaces( std::vector<NetUtils::NetworkAddress> &local_addresses )
{
	unsigned long num_interfaces = FindNumberOfLocalNetworkInterfaces();
    if( num_interfaces == -1 ){
        fprintf( stderr, "GetLocalNetworkInterfaces() failed\n" );
        return -1;
    }
    --num_interfaces; //subtract 1 since GetAdaptersInfo() doesn't return loopback

    PIP_ADAPTER_INFO pAdapterInfo = new IP_ADAPTER_INFO[num_interfaces];
    unsigned long OutBufLen = sizeof( IP_ADAPTER_INFO ) * num_interfaces;

    if( GetAdaptersInfo( pAdapterInfo, &OutBufLen ) != ERROR_SUCCESS ) {
        fprintf( stderr, "GetAdaptersInfo() failed\n" );
        return -1;
    }

    PIP_ADAPTER_INFO tmp_adapter_info;
    NetworkAddress addr;
    for( tmp_adapter_info = pAdapterInfo; tmp_adapter_info;
         tmp_adapter_info = tmp_adapter_info->Next ) {
        if( tmp_adapter_info->Type == MIB_IF_TYPE_ETHERNET ) {
            FindNetworkAddress( tmp_adapter_info->IpAddressList.IpAddress.String, addr ); 
            local_addresses.push_back( addr );
        }
    }

    //loopback not returned by GetAdaptersInfo() -- we add
    FindNetworkAddress( "127.0.0.1", addr ); 
    local_addresses.push_back( addr );

    return 0;
}

int
NetUtils::GetLastError( void )
{
    return WSAGetLastError();
}


} // namespace XPlat
