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
#ifndef _INSTRUCTIONAPI_DEBUG_
#define _INSTRUCTIONAPI_DEBUG_

#include <stdlib.h>
#include <stdio.h>
#include <util.h>
#include "compiler_annotations.h"

namespace Dyninst {
namespace InstructionAPI {
    extern int INSTRUCTION_EXPORT decoding_printf_int(const char *format, ...)
        DYNINST_PRINTF_ANNOTATION(1, 2);
    extern int dyn_debug_decoding;
    extern int dyn_debug_initialized;

#define decoding_cerr if (dyn_debug_decoding) cerr

#define dyn_debug_printf(debug_sys, ...) do { if(!dyn_debug_initialized || dyn_debug_##debug_sys) debug_sys##_printf_int(__VA_ARGS__); } while(0)

#define decoding_printf(...)              dyn_debug_printf(decoding, __VA_ARGS__)
}
}
#endif
