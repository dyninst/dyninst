/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: SocketUtils.h,v 1.1 2004/06/01 18:23:49 pcroth Exp $
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
