/***********************************************************************
 * Copyright © 2003-2004 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.     *
 **********************************************************************/

// $Id: Types.h,v 1.2 2004/03/23 01:12:22 eli Exp $
#ifndef XPLAT_TYPES_H
#define XPLAT_TYPES_H

#if defined(WIN32)
#include "xplat/Types-win.h"
#else
#include "xplat/Types-unix.h"
#endif // defined(WIN32)

#endif // XPLAT_TYPES_H
