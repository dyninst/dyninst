/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/
// $Id: NCIO-unix.C,v 1.3 2004/06/01 16:34:22 pcroth Exp $
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include "xplat/NCIO.h"

namespace XPlat
{

const int NCBlockingRecvFlag = MSG_WAITALL;

int
NCSend( XPSOCKET s, NCBuf* ncbufs, unsigned int nBufs )
{
    int ret = 0;

    unsigned int nBufsLeftToSend = nBufs;
    NCBuf* currBuf = ncbufs;
    while( nBufsLeftToSend > 0 )
    {
        // determine how many bufs we will try to send
        unsigned int nBufsToSend = 
            ((nBufsLeftToSend > IOV_MAX ) ? IOV_MAX : nBufsLeftToSend);
        
        // convert our buffer spec to writev's buffer spec
        unsigned int nBytesToSend = 0;
        struct iovec* currIov = new iovec[nBufsToSend];
        for( unsigned int i = 0; i < nBufsToSend; i++ )
        {
            currIov[i].iov_base = currBuf[i].buf;
            currIov[i].iov_len = currBuf[i].len;
            nBytesToSend += currBuf[i].len;
        }

        // do the send
        int sret = writev( s, currIov, nBufsToSend );
        delete[] currIov;
        if( sret < 0 )
        {
            ret = sret;
            break;
        }
        else
        {
            ret += sret;
        }

        // advance through buffers
        nBufsLeftToSend -= nBufsToSend;
        currBuf += nBufsToSend;
    }

    return ret;
}


int
NCRecv( XPSOCKET s, NCBuf* ncbufs, unsigned int nBufs )
{
    int ret = 0;

    unsigned int nBufsLeftToRecv = nBufs;
    NCBuf* currBuf = ncbufs;
    while( nBufsLeftToRecv > 0 )
    {
        // determine how many bufs we will try to receive
        unsigned int nBufsToRecv = 
            ((nBufsLeftToRecv > IOV_MAX ) ? IOV_MAX : nBufsLeftToRecv);
        
        // convert our buffer spec to recvmsg/readv's buffer spec
        msghdr msg;

        msg.msg_name = NULL;
        msg.msg_iov = new iovec[nBufsToRecv];
        msg.msg_iovlen = nBufsToRecv;
        msg.msg_control = NULL;
        msg.msg_controllen = 0;

        unsigned int nBytesToSend = 0;
        for( unsigned int i = 0; i < nBufsToRecv; i++ )
        {
            msg.msg_iov[i].iov_base = currBuf[i].buf;
            msg.msg_iov[i].iov_len = currBuf[i].len;
            nBytesToSend += currBuf[i].len;
        }

        // do the receive
        int sret = recvmsg( s, &msg, NCBlockingRecvFlag );
        delete[] msg.msg_iov;
        if( sret < 0 )
        {
            ret = sret;
            break;
        }
        else
        {
            ret += sret;
        }

        // advance through buffers
        nBufsLeftToRecv -= nBufsToRecv;
        currBuf += nBufsToRecv;
    }

    return ret;
}

} // namespace XPlat

