/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: Types.h,v 1.5 2007/01/24 19:33:57 darnold Exp $
#ifndef XPLAT_TYPES_H
#define XPLAT_TYPES_H

#if defined(os_windows)
#include "xplat/Types-win.h"
#else
#include "xplat/Types-unix.h"
#endif // defined(WIN32)

#endif // XPLAT_TYPES_H
