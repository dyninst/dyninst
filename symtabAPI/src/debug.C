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

#include "debug.h"

int sym_debug_create = 0;
int sym_debug_parsing = 0;
int sym_debug_aggregate = 0;
int sym_debug_object = 0;
int sym_debug_types = 0;
int sym_debug_translate = 0;
int sym_debug_rewrite = 0;

bool init_debug_symtabAPI() {
    static bool initialized = false;
    if (initialized) return true;
    initialized = true;
    if (getenv("SYMTAB_DEBUG_PARSING")) {
        sym_debug_parsing = 1;
    }
    if (getenv("SYMTAB_DEBUG_AGG") ||
        getenv("SYMTAB_DEBUG_AGGREGATE") ||
        getenv("SYMTAB_DEBUG_AGGREGATES")) {
        sym_debug_aggregate = 1;
    }
    if (getenv("SYMTAB_DEBUG_CREATE")) {
        sym_debug_create = 1;
    }
    if (getenv("SYMTAB_DEBUG_OBJECT")) {
        sym_debug_object = 1;
    }
    if (getenv("SYMTAB_DEBUG_TYPES")) {
        sym_debug_types = 1;
    }
    if (getenv("SYMTAB_DEBUG_REWRITE") ||
	getenv("SYMTAB_DEBUG_REWRITER")) {
        sym_debug_rewrite = 1;
    }

    return true;
}

int parsing_printf(const char *format, ...)
{
  if (!sym_debug_parsing) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

int create_printf(const char *format, ...)
{
  if (!sym_debug_create) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

int aggregate_printf(const char *format, ...)
{
  if (!sym_debug_parsing) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

int object_printf(const char *format, ...)
{
  if (!sym_debug_object) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

int types_printf(const char *format, ...)
{
  if (!sym_debug_types) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

int rewrite_printf(const char *format, ...)
{
  init_debug_symtabAPI();
  if (!sym_debug_rewrite) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}
