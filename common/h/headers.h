/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


// $Id: headers.h,v 1.25 2007/05/30 19:19:50 legendre Exp $

#ifndef KLUDGES_H
#define KLUDGES_H

#include <sys/types.h>

/*
 * Kludges to handle broken system includes and such...
 */

extern "C" {
typedef int (*xdr_rd_func)(void *, char *, int);
typedef int (*xdr_wr_func)(void *, char *, int);
}

#include "common/h/Types.h"

#if defined(os_solaris)
#include "common/h/solarisHeaders.h"

#elif defined(os_linux)
#include "common/h/linuxHeaders.h"

#elif defined(os_aix)
#include "common/h/aixv41Headers.h"

#elif defined(os_osf)
#include "common/h/osfHeaders.h"

#elif defined(os_windows)
#include "common/h/ntHeaders.h"

#elif defined(os_irix)
#include "common/h/irixHeaders.h"

#endif  /* architecture specific */

#endif /* KLUDGES_H */
