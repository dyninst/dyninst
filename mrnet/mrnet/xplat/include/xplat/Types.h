/****************************************************************************
 * Copyright © 2003-2005 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

// $Id: Types.h,v 1.3 2005/03/24 04:59:21 darnold Exp $
#ifndef XPLAT_TYPES_H
#define XPLAT_TYPES_H

#if defined(WIN32)
#include "xplat/Types-win.h"
#else
#include "xplat/Types-unix.h"
#endif // defined(WIN32)

#endif // XPLAT_TYPES_H
