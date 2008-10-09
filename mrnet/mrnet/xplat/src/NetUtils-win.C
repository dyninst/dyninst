/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: NetUtils-win.C,v 1.8 2008/10/09 19:54:03 mjbrim Exp $

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
	DWORD ret = GetAdaptersInfo( pAdapterInfo, &output_buffer_len );
	delete pAdapterInfo;
    if( (ret == ERROR_BUFFER_OVERFLOW) || (ret == ERROR_SUCCESS) ) {
		int nif = output_buffer_len / sizeof(IP_ADAPTER_INFO);
		if( output_buffer_len > (nif * sizeof(IP_ADAPTER_INFO)) )
			nif++;
        return nif;
    }
    fprintf( stderr, "FindNumberOfLocalNetworkInterfaces() failed\n" );
    return -1;
}


int NetUtils::FindLocalNetworkInterfaces( std::vector<NetUtils::NetworkAddress> &local_addresses )
{
	unsigned long num_interfaces = FindNumberOfLocalNetworkInterfaces();
    if( num_interfaces == -1 ){
        fprintf( stderr, "FindLocalNetworkInterfaces() failed\n" );
        return -1;
    }

	PIP_ADAPTER_INFO pAdapterInfo = new IP_ADAPTER_INFO[num_interfaces];
    unsigned long OutBufLen = sizeof( IP_ADAPTER_INFO ) * num_interfaces;
	DWORD ret = GetAdaptersInfo( pAdapterInfo, &OutBufLen );
    if( ret != ERROR_SUCCESS ) {
        fprintf( stderr, "GetAdaptersInfo() failed (rc = %d) ", ret);
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
    delete[] pAdapterInfo;
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
