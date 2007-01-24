/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: Error-unix.C,v 1.3 2007/01/24 19:33:58 darnold Exp $
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
