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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "common/src/debug_common.h"

int common_debug_dwarf = 0;
int common_debug_addrtranslate = 0;
int common_debug_lineinfo = 0;
int common_debug_parsing = 0;
int common_debug_initialized = 0;

bool init_debug_common() {
    if (common_debug_initialized) return true;
    common_debug_initialized = 1;

    if (getenv("COMMON_DEBUG_DWARF") ||
        getenv("DYNINST_DEBUG_DWARF")) {
       common_debug_dwarf = 1;
    }

    if (getenv("DYNINST_DEBUG_ADDRTRANSLATE") ||
        getenv("DYNINST_DEBUG_TRANSLATE")) {
       common_debug_addrtranslate = 1;
    }
    if (getenv("DYNINST_DEBUG_LINEINFO")) {
           common_debug_lineinfo = 1;
    }
    
    if(getenv("COMMON_DEBUG_PARSING")){
        common_debug_parsing = 1;  
    }

    return true;
}


int dwarf_printf_int(const char *format, ...)
{
   init_debug_common();
  if (!common_debug_dwarf) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

int translate_printf_int(const char *format, ...)
{
   init_debug_common();
  if (!common_debug_addrtranslate) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

int lineinfo_printf_int(const char *format, ...)
{
   init_debug_common();
  if (!common_debug_lineinfo) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

int common_parsing_printf_int(const char *format, ...)
{
   init_debug_common();
  if (!common_debug_parsing) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}
