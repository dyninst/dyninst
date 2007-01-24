/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: SocketUtils-unix.C,v 1.2 2007/01/24 19:34:26 darnold Exp $
#include <unistd.h>
#include "xplat/Types.h"
#include "xplat/SocketUtils.h"

namespace XPlat
{

int
SocketUtils::Close( int sock )
{
    return close( sock );
}

} // namespace XPlat

