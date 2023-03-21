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

#ifndef _DATAFLOW_DEBUG_H_
#define _DATAFLOW_DEBUG_H_

#include <string>
#include "compiler_annotations.h"

extern int df_debug_slicing_on();
extern int df_debug_stackanalysis_on();
extern int df_debug_convert_on();
extern int df_debug_expand_on();
extern int df_debug_liveness_on();

#define slicing_cerr       if (df_debug_slicing_on()) cerr
#define stackanalysis_cerr if (df_debug_stackanalysis_on()) cerr
#define convert_cerr       if (df_debug_convert_on()) cerr
#define expand_cerr        if (df_debug_expand_on()) cerr
#define liveness_cerr      if (df_debug_liveness_on()) cerr

extern int slicing_printf_int(const char *format, ...)
        DYNINST_PRINTF_ANNOTATION(1, 2);
extern int stackanalysis_printf_int(const char *format, ...)
        DYNINST_PRINTF_ANNOTATION(1, 2);
extern int convert_printf_int(const char *format, ...)
        DYNINST_PRINTF_ANNOTATION(1, 2);
extern int expand_printf_int(const char *format, ...)
        DYNINST_PRINTF_ANNOTATION(1, 2);
extern int liveness_printf_int(const char *format, ...)
        DYNINST_PRINTF_ANNOTATION(1, 2);


#define dataflow_debug_printf(debug_sys, ...) do {if (df_debug_##debug_sys##_on()) debug_sys##_printf_int(__VA_ARGS__); } while(0)

#define slicing_printf(...)       dataflow_debug_printf(slicing, __VA_ARGS__)
#define stackanalysis_printf(...) dataflow_debug_printf(stackanalysis, __VA_ARGS__)
#define convert_printf(...)       dataflow_debug_printf(convert, __VA_ARGS__)
#define expand_printf(...)        dataflow_debug_printf(expand, __VA_ARGS__)
#define liveness_printf(...)      dataflow_debug_printf(liveness, __VA_ARGS__)

// And initialization

extern bool df_init_debug();

// Conditional based on debug
extern bool slicing_debug_on();

#endif /* SHOWERROR_H */
