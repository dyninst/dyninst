/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: NCIO.h,v 1.2 2004/03/23 01:12:22 eli Exp $
#ifndef XPLAT_NCIO_H
#define XPLAT_NCIO_H

namespace XPlat
{

#if defined(WIN32)
typedef SOCKET XPSOCKET;
#else
typedef int XPSOCKET;
#endif // defined(WIN32)

struct NCBuf
{
    unsigned int len;
    char* buf;
};


int NCSend( XPSOCKET s, NCBuf* bufs, unsigned int nBufs );
int NCRecv( XPSOCKET s, NCBuf* bufs, unsigned int nBufs );

// flag to specify with send/recv for blocking recv, if available.
extern const int NCBlockingRecvFlag;

} // namespace XPlat

#endif // XPLAT_NCIO_H
