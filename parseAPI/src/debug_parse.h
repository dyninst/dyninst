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
#ifndef _PARSEAPI_DEBUG_
#define _PARSEAPI_DEBUG_

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <common/h/util.h>
#include "compiler_annotations.h"

namespace Dyninst {
namespace ParseAPI {

    extern int PARSER_EXPORT parsing_printf_int(const char *format, ...)
            DYNINST_PRINTF_ANNOTATION(1, 2);
    extern int PARSER_EXPORT malware_printf_int(const char *format, ...)
            DYNINST_PRINTF_ANNOTATION(1, 2);
    extern int PARSER_EXPORT indirect_collect_printf_int(const char *format, ...)
            DYNINST_PRINTF_ANNOTATION(1, 2);
    extern int dyn_debug_parsing;
    extern int dyn_debug_malware;
    extern int dyn_debug_indirect_collect;
    extern int dyn_debug_initialized;

#define parsing_cerr if (dyn_debug_parsing) cerr
#define malware_cerr if (dyn_debug_malware) cerr

#define dyn_debug_printf(debug_sys, ...) do { if(!dyn_debug_initialized || dyn_debug_##debug_sys) debug_sys##_printf_int(__VA_ARGS__); } while(0)

#define parsing_printf(...)              dyn_debug_printf(parsing, __VA_ARGS__)
#define mal_printf(...)                  dyn_debug_printf(malware, __VA_ARGS__)
#define indirect_collect_printf(...)     dyn_debug_printf(indirect_collect, __VA_ARGS__)
}
}

extern const std::string PARSE_BLOCK_COUNT;
extern const std::string PARSE_FUNCTION_COUNT;
extern const std::string PARSE_BLOCK_SIZE;

extern const std::string PARSE_NORETURN_COUNT;
extern const std::string PARSE_RETURN_COUNT;
extern const std::string PARSE_UNKNOWN_COUNT;

extern const std::string PARSE_NORETURN_HEURISTIC;

extern const std::string PARSE_JUMPTABLE_COUNT;
extern const std::string PARSE_JUMPTABLE_FAIL;
extern const std::string PARSE_TAILCALL_COUNT;
extern const std::string PARSE_TAILCALL_FAIL;

extern const std::string PARSE_TOTAL_TIME;
extern const std::string PARSE_JUMPTABLE_TIME;

#endif
