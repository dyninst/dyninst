/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: os.h,v 1.38 2008/06/19 19:53:31 legendre Exp $

#ifndef _OS_HDR
#define _OS_HDR

/*
 * This is an initial attempt at providing an OS abstraction for paradynd
 * I am doing this so I can compile paradynd on solaris
 *
 * This should enforce the abstract OS operations
 */ 

//  JAW: 1-09-07 removed switches for platforms:
//  alpha_dec_osf4_0
//  hppa1_1_hp_hpux
//  mips_unknown_ce2_11
//  mips_sgi_irix6_4

#if defined(sparc_sun_sunos4_1_3)
#include "dyninstAPI/src/sunos.h"

#elif defined(sparc_sun_solaris2_4) \
   || defined(i386_unknown_solaris2_5)
#include "dyninstAPI/src/solaris.h"

#elif defined(os_aix) 
#include "dyninstAPI/src/aix.h"

#elif defined(i386_unknown_nt4_0) 
#include "dyninstAPI/src/pdwinnt.h"

#elif defined(os_linux)
#include "dyninstAPI/src/linux.h"

#elif defined(os_freebsd)
#include "dyninstAPI/src/freebsd.h"

#elif defined(os_vxworks)
#include "dyninstAPI/src/vxworks.h"
#endif

#include <string>
#include "common/h/Types.h"

typedef enum { neonatal, running, stopped, detached, exited, deleted, unknown_ps } processState;
const char *processStateAsString(processState state); // Defined in process.C

class OS {
public:
  static void osTraceMe(void);
  static void osDisconnect(void);
  static bool osKill(int);
  static void make_tempfile(char *);
  static bool execute_file(char *);
  static void unlink(char *);
  static bool executableExists(const std::string &file);
};

#endif
