/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: Types.h,v 1.4 2005/03/29 16:01:29 darnold Exp $
#ifndef XPLAT_TYPES_H
#define XPLAT_TYPES_H

#if defined(os_windows)
#include "xplat/Types-win.h"
#else
#include "xplat/Types-unix.h"
#endif // defined(WIN32)

#endif // XPLAT_TYPES_H
