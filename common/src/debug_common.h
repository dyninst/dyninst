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

#ifndef COMMON_DEBUG_H
#define COMMON_DEBUG_H

#include <string>
#include "util.h"

COMMON_EXPORT extern int common_debug_dwarf;
extern int common_debug_addrtranslate;

#if defined(__GNUC__)
#define dwarf_printf(format, ...)                                       \
   do {                                                                 \
      dwarf_printf_int("[%s:%u] " format, __FILE__, __LINE__, ## __VA_ARGS__); \
   } while (0)
#else
#define dwarf_printf dwarf_printf_int
#endif

COMMON_EXPORT int dwarf_printf_int(const char *format, ...);

#if defined(__GNUC__)
#define translate_printf(format, ...)                                       \
   do {                                                                 \
      translate_printf_int("[%s:%u] " format, __FILE__, __LINE__, ## __VA_ARGS__); \
   } while (0)
#else
#define translate_printf translate_printf_int
#endif

COMMON_EXPORT int translate_printf_int(const char *format, ...);

// And initialization
COMMON_EXPORT bool init_debug_common();

#endif /* COMMON_DEBUG_H */
