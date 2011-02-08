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

#ifndef SHOWERROR_H
#define SHOWERROR_H

#include <string>
extern int dyn_debug_crash;

extern int patch_debug_relocation;
extern int patch_debug_springboard;

#define relocation_cerr   if (patch_debug_relocation) std::cerr
#define springboard_cerr  if (patch_debug_relocation) std::cerr

extern int relocation_printf_int(const char *format, ...);
extern int springboard_printf_int(const char *format, ...);


#if defined(__GNUC__)

#define relocation_printf(format, args...) do { if (patch_debug_relocation) relocation_printf_int(format, ## args); } while(0)
#define springboard_printf(format, args...) do { if (patch_debug_springboard) springboard_printf_int(format, ## args); } while(0)

#else
// Non-GCC doesn't have the ## macro
#define relocation_printf relocation_printf_int
#define springboard_printf springboard_printf_int

#endif


// And initialization
extern bool init_debug_patchapi();

#endif
