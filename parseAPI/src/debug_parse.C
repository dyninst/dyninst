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
#include <stdarg.h>
#include <stdlib.h>
#include <string>

#include "debug_parse.h"

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace Dyninst::ParseAPI;

int Dyninst::ParseAPI::dyn_debug_parsing = 0;
int Dyninst::ParseAPI::dyn_debug_malware = 0;
int Dyninst::ParseAPI::dyn_debug_indirect_collect = 0;
int Dyninst::ParseAPI::dyn_debug_initialized = 0;

int Dyninst::ParseAPI::parsing_printf_int(const char *format, ...)
{
    if(!dyn_debug_initialized) {
        if(getenv("DYNINST_DEBUG_PARSING"))
            dyn_debug_parsing = 1;
        if(getenv("DYNINST_DEBUG_MALWARE"))
            dyn_debug_malware = 1;
        dyn_debug_initialized = 1;
    }

    if(!dyn_debug_parsing) return 0;
    if(NULL == format) return -1;

    auto const id = []() -> int {
#ifdef _OPENMP
      return omp_get_thread_num();
#endif
      return 0;
    }();

    va_list va;
    va_start(va,format);

    auto ret = 0;

    #pragma omp critical
    {
      ret = fprintf(stderr, "[thread %d] ", id);
      if(ret >= 0) {
        ret = vfprintf(stderr, format, va);
      }
      fflush(stderr);
    }

    va_end(va);
    return ret;
}

int Dyninst::ParseAPI::malware_printf_int(const char *format, ...)
{
    if(!dyn_debug_initialized) {
        if(getenv("DYNINST_DEBUG_PARSING"))
            dyn_debug_parsing = 1;
        if(getenv("DYNINST_DEBUG_MALWARE"))
            dyn_debug_malware = 1;
        dyn_debug_initialized = 1;
    }

    if(!dyn_debug_malware) return 0;
    if(NULL == format) return -1;

    va_list va;
    va_start(va,format);
    int ret = vfprintf(stderr, format, va);
    va_end(va);

    return ret;
}

int Dyninst::ParseAPI::indirect_collect_printf_int(const char *format, ...)
{
    if(!dyn_debug_initialized) {
        if(getenv("DYNINST_DEBUG_INDIRECT_COLLECT"))
            dyn_debug_indirect_collect = 1;
        dyn_debug_initialized = 1;
    }

    if(!dyn_debug_parsing) return 0;
    if(NULL == format) return -1;

    va_list va;
    va_start(va,format);
    int ret = vfprintf(stderr, format, va);
    va_end(va);

    return ret;
}
const std::string PARSE_BLOCK_COUNT("parseBlockCount");
const std::string PARSE_FUNCTION_COUNT("parseFunctionCount");
const std::string PARSE_BLOCK_SIZE("parseBlockSize");

const std::string PARSE_NORETURN_COUNT("parseNoReturnCount");
const std::string PARSE_RETURN_COUNT("parseReturnCount");
const std::string PARSE_UNKNOWN_COUNT("parseUnknownCount");

const std::string PARSE_NORETURN_HEURISTIC("parseNoReturnHeuristicCount");

const std::string PARSE_JUMPTABLE_COUNT("parseJumptableCount");
const std::string PARSE_JUMPTABLE_FAIL("parseJumptableFail");
const std::string PARSE_TAILCALL_COUNT("isTailcallCount");
const std::string PARSE_TAILCALL_FAIL("isTailcallFail");

const std::string PARSE_TOTAL_TIME("parseTotalTime");
const std::string PARSE_JUMPTABLE_TIME("parseJumpTableTime");
