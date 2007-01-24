/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: SocketUtils.h,v 1.2 2007/01/24 19:33:51 darnold Exp $
#ifndef XPLAT_SOCKETUTILS_H
#define XPLAT_SOCKETUTILS_H

#include "xplat/Types.h"

namespace XPlat
{

class SocketUtils
{
public:
    static int Close( int sock );
};

} // namespace XPlat

#endif // XPLAT_SOCKETUTILS_H
