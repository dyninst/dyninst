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

// $Id: debug.C,v 1.6 2008/02/23 02:09:05 jaw Exp $

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string>
#include "dyninstAPI/src/debug.h"
#include "compiler_annotations.h"

int patch_debug_relocation = 0;
int patch_debug_springboard = 0;

int relocation_printf_int(const char *format, ...)  DYNINST_PRINTF_ANNOTATION(1, 2);
int springboard_printf_int(const char *format, ...) DYNINST_PRINTF_ANNOTATION(1, 2);

bool init_debug_patchapi() {
  static bool init = false;
  if (init) return true;
  init = true;

  if ( getenv("DYNINST_DEBUG_RELOCATION") ||
       getenv("PATCHAPI_DEBUG_RELOCATION")) {
     fprintf(stderr, "Enabling DyninstAPI relocation debug\n");
     patch_debug_relocation = 1;
  }

  if ( getenv("DYNINST_DEBUG_SPRINGBOARD") ||
       getenv("PATCHAPI_DEBUG_SPRINGBOARD")) {
     fprintf(stderr, "Enabling DyninstAPI springboard debug\n");
     patch_debug_relocation = 1;
  }

  return true;
}

int relocation_printf_int(const char *format, ...)
{
  if (!patch_debug_relocation) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

int springboard_printf_int(const char *format, ...)
{
  if (!patch_debug_springboard) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

