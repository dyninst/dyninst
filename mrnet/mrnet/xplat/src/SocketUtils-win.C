/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: SocketUtils-win.C,v 1.4 2007/01/24 19:34:27 darnold Exp $
//#include <windows.h>
#include <winsock2.h>
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
