/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Error-win.C,v 1.3 2005/03/24 04:59:22 darnold Exp $
#include <windows.h>
#include <winsock2.h>
#include "xplat/Error.h"

namespace XPlat
{

std::string
Error::GetErrorString( int code )
{
    char msgbuf[1024];
    DWORD dwErr = (DWORD)code;

    DWORD dwRet = FormatMessage( 
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwErr,
        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
        (LPTSTR)msgbuf,
        1024 / sizeof(TCHAR),
        NULL );

    return ((dwRet != 0) ? msgbuf : "");
}


bool
Error::ETimedOut( int code )
{
    return (code == WSAETIMEDOUT);
}


bool
Error::EAddrInUse( int code )
{
    return (code == WSAEADDRINUSE);
}

} // namespace XPlat

