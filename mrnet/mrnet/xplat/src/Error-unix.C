/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Error-unix.C,v 1.2 2004/03/23 01:12:23 eli Exp $
#include <errno.h>
#include "xplat/Error.h"

namespace XPlat
{

std::string
Error::GetErrorString( int code )
{
    return strerror( code );
}

bool
Error::ETimedOut( int code )
{
    return (code == ETIMEDOUT);
}


bool
Error::EAddrInUse( int code )
{
    return (code == EADDRINUSE);
}

} // namespace XPlat
