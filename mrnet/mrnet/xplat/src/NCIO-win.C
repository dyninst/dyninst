/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/
// $Id: NCIO-win.C,v 1.1 2003/11/14 19:27:03 pcroth Exp $

#include <winsock2.h>
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

    // convert buffer specifiers
    WSABUF* wsaBufs = new WSABUF[nBufs];
    for( unsigned int i = 0; i < nBufs; i++ )
    {
        wsaBufs[i].buf = ncbufs[i].buf;
        wsaBufs[i].len = ncbufs[i].len;
    }

    // do the send
    DWORD nBytesSent = 0;
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

    // convert buffer specifiers
    WSABUF* wsaBufs = new WSABUF[nBufs];
    for( unsigned int i = 0; i < nBufs; i++ )
    {
        wsaBufs[i].buf = ncbufs[i].buf;
        wsaBufs[i].len = ncbufs[i].len;
    }

    // do the receive
    DWORD nBytesReceived = 0;
    DWORD dwFlags = 0;
    int rret = WSARecv( s, wsaBufs, nBufs, &nBytesReceived, &dwFlags, NULL, NULL );
    if( rret != SOCKET_ERROR )
    {
        ret = nBytesReceived;
    }
    else
    {
        ret = -1;
    }
    return ret;
}



} // namespace XPlat

