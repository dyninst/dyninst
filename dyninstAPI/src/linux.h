/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

// $Id: linux.h,v 1.37 2007/12/04 18:05:24 legendre Exp $

#if !defined(os_linux)
#error "invalid architecture-os inclusion"
#endif

#ifndef LINUX_PD_HDR
#define LINUX_PD_HDR
class PCProcess;

#include "common/src/linuxKludges.h"
#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Archive.h"

#define EXIT_NAME "_exit"

#if !defined(arch_x86_64)
#define SIGNAL_HANDLER	 "__restore"
#else
#define SIGNAL_HANDLER   "__restore_rt"
#endif

#if defined(i386_unknown_linux2_0) \
   || defined(x86_64_unknown_linux2_4)
#include "linux-x86.h"
#elif defined(os_linux) && defined(arch_power)
#include "linux-power.h"
#elif defined(os_linux) && defined(arch_aarch64)
#include "linux-aarch64.h"
#else
#error Invalid or unknown architecture-os inclusion
#endif

#include "unix.h"

#ifndef WNOWAIT
#define WNOWAIT WNOHANG
#endif

bool get_linux_version(int &major, int &minor, int &subvers);
bool get_linux_version(int &major, int &minor, int &subvers, int &subsubvers);

#endif
