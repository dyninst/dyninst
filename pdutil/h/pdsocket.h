/*
 * Copyright (c) 1996-2003 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: pdsocket.h,v 1.4 2003/10/28 17:39:48 pcroth Exp $
#ifndef PDSOCKET_H
#define PDSOCKET_H


#if defined(i386_unknown_nt4_0)

#include <winsock2.h>

typedef	SOCKET	PDSOCKET;

#  define	CLOSEPDSOCKET(s)	closesocket(s)
#  define	PDSOCKET_ERRNO		WSAGetLastError()

#  define	INVALID_PDSOCKET	(INVALID_SOCKET)
#  define	PDSOCKET_ERROR		SOCKET_ERROR

#else /* defined(i386_unknown_nt4_0) */

typedef	int	PDSOCKET;

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#  define	CLOSEPDSOCKET(s)	close(s)
#  define	PDSOCKET_ERRNO		errno

#  define	INVALID_PDSOCKET	(-1)
#  define	PDSOCKET_ERROR		(-1)

#endif /* defined(i386_unknown_nt4_0) */

#if defined(__cplusplus)

struct PdSocket
{
    PDSOCKET  s;

    PdSocket( PDSOCKET _s = INVALID_PDSOCKET )
      : s( _s )
    { }

    PdSocket( const PdSocket& origSock )
      : s( origSock.s )
    { }

    bool operator==( const PdSocket& origSock ) { return origSock.s == s; }
    bool operator!=( const PdSocket& origSock ) { return origSock.s != s; }

    bool IsBad( void ) const;
};

#endif // __cplusplus

#endif /* PDSOCKET_H */
