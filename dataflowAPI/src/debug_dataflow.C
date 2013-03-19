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
#include <assert.h>
#include <string>
#include "debug_dataflow.h"
#include <stdlib.h>

#include <iostream>

// Internal debugging

int df_debug_slicing= 0;
int df_debug_stackanalysis = 0;
int df_debug_convert = 0;
int df_debug_expand = 0;
int df_debug_liveness = 0;

bool df_init_debug() {

  static bool init = false;
  if (init) return true;
  init = true;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4996) 
#endif

  if ((getenv("DATAFLOW_DEBUG_STACKANALYSIS"))) {
    fprintf(stderr, "Enabling DataflowAPI stack analysis debugging\n");
    df_debug_stackanalysis = 1;
  }

  if ((getenv("DATAFLOW_DEBUG_SLICING"))) {
    fprintf(stderr, "Enabling DataflowAPI slicing debugging\n");
    df_debug_slicing = 1;
  }

  if ((getenv("DATAFLOW_DEBUG_CONVERT"))) {
    fprintf(stderr, "Enabling DataflowAPI->ROSE conversion debugging\n");
    df_debug_convert = 1;
  }

  if ((getenv("DATAFLOW_DEBUG_EXPAND"))) {
    fprintf(stderr, "Enabling DataflowAPI symbolic expansion debugging\n");
    df_debug_expand = 1;
  }

  if ((getenv("DATAFLOW_DEBUG_LIVENESS"))) {
    fprintf(stderr, "Enabling DataflowAPI liveness debugging\n");
    df_debug_liveness = 1;
  }

#if defined(_MSC_VER)
#pragma warning(pop)    
#endif

  return true;
}

bool slicing_debug_on() {
    return df_debug_slicing != 0;
}

int stackanalysis_printf_int(const char *format, ...)
{
  if (!df_debug_stackanalysis) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

int slicing_printf_int(const char *format, ...)
{
  if (!df_debug_slicing) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

int convert_printf_int(const char *format, ...)
{
  if (!df_debug_convert) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

int expand_printf_int(const char *format, ...)
{
  if (!df_debug_expand) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}

int liveness_printf_int(const char *format, ...)
{
  if (!df_debug_liveness) return 0;
  if (NULL == format) return -1;

  va_list va;
  va_start(va, format);
  int ret = vfprintf(stderr, format, va);
  va_end(va);

  return ret;
}
