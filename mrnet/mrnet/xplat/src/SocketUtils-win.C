/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: SocketUtils-win.C,v 1.1 2004/06/01 18:23:52 pcroth Exp $
#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include "xplat/SocketUtils.h"


namespace XPlat
{

int
SocketUtils::Close( int sock )
{
    return closesocket( (SOCKET)sock );
}

} // namespace XPlat
