/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: NCIO-win.C,v 1.5 2008/10/09 19:54:02 mjbrim Exp $

#include <winsock2.h>
#include <stdio.h>
#include "xplat/NCIO.h"

namespace XPlat
{

// the Winsock version of recv doesn't take an argument to say to do 
// a blocking receive.
const int NCBlockingRecvFlag = 0;

int
NCSend( XPSOCKET s, NCBuf* ncbufs, unsigned int nBufs )
{
    int ret = 0;
    DWORD nBytesSent = 0;

    // convert buffer specifiers
    WSABUF* wsaBufs = new WSABUF[nBufs];
    for( unsigned int i = 0; i < nBufs; i++ )
    {
        wsaBufs[i].buf = ncbufs[i].buf;
        wsaBufs[i].len = ncbufs[i].len;
        nBytesSent += ncbufs[i].len;
    }

    // do the send
    nBytesSent = 0;
    int sret = WSASend( s, wsaBufs, nBufs, &nBytesSent, 0, NULL, NULL );
    if( sret != SOCKET_ERROR )
    {
        ret = nBytesSent;
    }
    else
    {
        ret = -1;
    }
    return ret;
}


int
NCRecv( XPSOCKET s, NCBuf* ncbufs, unsigned int nBufs )
{
    int ret = 0;
	signed int bytes_remaining = 0;
    // convert buffer specifiers
    WSABUF* wsaBufs = new WSABUF[nBufs];
    for( unsigned int i = 0; i < nBufs; i++ )
    {
        wsaBufs[i].buf = ncbufs[i].buf;
        wsaBufs[i].len = ncbufs[i].len;
		bytes_remaining += ncbufs[i].len;
    }

    // do the receive
    while (1) {
        DWORD nBytesReceived = 0;
        DWORD dwFlags = 0;
        int rret = WSARecv( s, wsaBufs, nBufs, &nBytesReceived, &dwFlags, NULL, NULL );
        if( rret == SOCKET_ERROR || nBytesReceived == 0)
            return -1;

        bytes_remaining -= nBytesReceived;
        ret += nBytesReceived;
        if (bytes_remaining <= 0)
            break;
        if (bytes_remaining > 0) {
            for (unsigned i=0; i<nBufs; i++) {
                if (!wsaBufs[i].len) 
                    continue;
                if (!nBytesReceived)
                    break;
                if (wsaBufs[i].len <= nBytesReceived) {
                    wsaBufs[i].len = 0;
                    nBytesReceived -= wsaBufs[i].len;
                    continue;
                }
                else {
                    wsaBufs[i].len -= nBytesReceived;
                    wsaBufs[i].buf = ((char *) wsaBufs[i].buf) + nBytesReceived;
                    nBytesReceived = 0;
                }
            }
        }
    }

    delete wsaBufs;
    return ret;
}



} // namespace XPlat

