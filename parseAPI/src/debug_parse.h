/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
#ifndef _PARSEAPI_DEBUG_
#define _PARSEAPI_DEBUG_

#include <stdlib.h>
#include <stdio.h>
#include <util.h>

namespace Dyninst {
namespace ParseAPI {

    extern int PARSER_EXPORT parsing_printf_int(const char *format, ...);
    extern int PARSER_EXPORT malware_printf_int(const char *format, ...);
    extern int dyn_debug_parsing;
    extern int dyn_debug_malware;
    extern int dyn_debug_initialized;

#define parsing_cerr if (dyn_debug_parsing) cerr
#define malware_cerr if (dyn_debug_malware) cerr

#if defined(__GNUC__)
#define parsing_printf(format, args...) do { if(!dyn_debug_initialized || dyn_debug_parsing) parsing_printf_int(format, ## args); } while(0)
#define mal_printf(format, args...) do { if(!dyn_debug_initialized || dyn_debug_malware) malware_printf_int(format, ## args); } while(0)
#else
#define parsing_printf parsing_printf_int
#define mal_printf malware_printf_int
#endif
}
}

#define PARSE_BLOCK_COUNT "parseBlockCount"
#define PARSE_FUNCTION_COUNT "parseFunctionCount"
#define PARSE_BLOCK_SIZE "parseBlockSize"

#define PARSE_NORETURN_COUNT "parseNoReturnCount"
#define PARSE_RETURN_COUNT "parseReturnCount"
#define PARSE_UNKNOWN_COUNT "parseUnknownCount"

#define PARSE_NORETURN_HEURISTIC "parseNoReturnHeuristicCount"

#define PARSE_JUMPTABLE_COUNT "parseJumptableCount"
#define PARSE_JUMPTABLE_FAIL "parseJumptableFail"
#define PARSE_TAILCALL_COUNT "isTailcallCount"
#define PARSE_TAILCALL_FAIL "isTailcallFail"

#endif
