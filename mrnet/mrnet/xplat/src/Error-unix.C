/***********************************************************************
 * Copyright © 2003 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Error-unix.C,v 1.1 2003/11/14 19:27:02 pcroth Exp $
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
