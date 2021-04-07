/*
 * Copyright (c) 1996-2021 Barton P. Miller
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
#include <vector>
#include <map>
#include <iostream>
#include <boost/assign/list_of.hpp>

#include "dyntypes.h"

#include "CodeSource.h"
#include "debug_parse.h"
#include "util.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

CodeSource::~CodeSource() {
  for(auto *r : _regions)
    delete r;
}

/** CodeSource **/
void
CodeSource::addRegion(CodeRegion * cr)
{
    _regions.push_back(cr);

    // check for overlapping regions
    if(!_regions_overlap) {
        set<CodeRegion *> exist;
        _region_tree.find(cr,exist);
        if(!exist.empty()) {
            _regions_overlap = true;
        }
    }

    _region_tree.insert(cr);
}

void CodeSource::removeRegion(CodeRegion *cr) {
  auto pos = std::remove(_regions.begin(), _regions.end(), cr);
  if (pos != _regions.end()) {
    // NB: Assume no duplicates
    delete *pos;
    _regions.erase(pos);

    // Also remove from the tree
    _region_tree.remove(*pos);
  }
}


int
CodeSource::findRegions(Address addr, set<CodeRegion *> & ret) const
{
    return _region_tree.find(addr,ret);
}

dyn_hash_map<std::string, bool>
CodeSource::non_returning_funcs =
    boost::assign::map_list_of
        // libc-2.31.so from glibc 2.31
        ("_Exit", true)
        ("__GI___assert_fail", true)
        ("__GI___assert_perror_fail", true)
        ("__GI___chk_fail", true)
        ("__GI___fortify_fail", true)
        ("__GI___libc_dynarray_at_failure", true)
        ("__GI___libc_fatal", true)
        ("__GI__dl_signal_error", true)
        ("__GI__dl_signal_exception", true)
        ("__GI__exit", true)
        ("__GI_abort", true)
        ("__GI_exit", true)
        ("__GI_verr", true)
        ("__GI_verrx", true)
        ("__assert", true)
        ("__assert_fail", true)
        ("__assert_fail_base", true)
        ("__assert_perror_fail", true)
        ("__chk_fail", true)
        ("__fortify_fail", true)
        ("__libc_dynarray_at_failure", true)
        ("__libc_fatal", true)
        ("__libc_main", true)
        ("__libc_siglongjmp", true)
        ("__libc_start_main", true)
        ("__run_exit_handlers", true)
        ("__stack_chk_fail", true)
        ("__stack_chk_fail_local", true)
        ("_dl_signal_error", true)
        ("_dl_signal_exception", true)
        ("_dl_start", true)
        ("_exit", true)
        ("_longjmp", true)
        ("abort", true)
        ("err", true)
        ("errx", true)
        ("exit", true)
        ("fatal_error", true)
        ("longjmp", true)
        ("mabort", true)
        ("malloc_printerr", true)
        ("print_and_abort", true)
        ("siglongjmp", true)
        ("svctcp_rendezvous_abort", true)
        ("svcunix_rendezvous_abort", true)
        ("verr", true)
        ("verrx", true)

        // libpthread-2.31.so from glibc 2.31
        ("__GI___pthread_unwind", true)
        ("__GI___pthread_unwind_next", true)
        ("__nptl_main", true)
        ("__pthread_exit", true)
        ("__pthread_unwind", true)
        ("__pthread_unwind_next", true)
        ("longjmp_alias", true)
        ("longjmp_compat", true)
        ("pthread_exit", true)
        ("siglongjmp_alias", true)
        ("start_thread", true)
        ("thrd_exit", true)

        // libstdc++.so from gcc 9.3.0
        ("_ZN10__cxxabiv111__terminateEPFvvE", true)
        ("_ZN10__cxxabiv112__unexpectedEPFvvE", true)
        ("_ZN9__gnu_cxx26__throw_insufficient_spaceEPKcS1_", true)
        ("_ZNK11__gnu_debug16_Error_formatter8_M_errorEv", true)
        ("_ZSt10unexpectedv", true)
        ("_ZSt16__throw_bad_castv", true)
        ("_ZSt17__throw_bad_allocv", true)
        ("_ZSt17rethrow_exceptionNSt15__exception_ptr13exception_ptrE", true)
        ("_ZSt18__throw_bad_typeidv", true)
        ("_ZSt19__throw_ios_failurePKc", true)
        ("_ZSt19__throw_ios_failurePKci", true)
        ("_ZSt19__throw_logic_errorPKc", true)
        ("_ZSt19__throw_range_errorPKc", true)
        ("_ZSt20__throw_domain_errorPKc", true)
        ("_ZSt20__throw_future_errori", true)
        ("_ZSt20__throw_length_errorPKc", true)
        ("_ZSt20__throw_out_of_rangePKc", true)
        ("_ZSt20__throw_system_errori", true)
        ("_ZSt21__throw_bad_exceptionv", true)
        ("_ZSt21__throw_runtime_errorPKc", true)
        ("_ZSt22__throw_overflow_errorPKc", true)
        ("_ZSt23__throw_underflow_errorPKc", true)
        ("_ZSt24__throw_invalid_argumentPKc", true)
        ("_ZSt24__throw_out_of_range_fmtPKcz", true)
        ("_ZSt25__throw_bad_function_callv", true)
        ("_ZSt9terminatev", true)
        ("__cxa_bad_cast", true)
        ("__cxa_bad_typeid", true)
        ("__cxa_call_unexpected", true)
        ("__cxa_deleted_virtual", true)
        ("__cxa_pure_virtual", true)
        ("__cxa_rethrow", true)
        ("__cxa_throw", true)
        ("__cxa_throw_bad_array_new_length", true)

        // libgfortran.so from gcc 9.3.0
        ("_gfortran_error_stop_numeric", true)
        ("_gfortran_error_stop_string", true)
        ("_gfortran_os_error", true)
        ("_gfortran_runtime_error", true)
        ("_gfortran_runtime_error_at", true)
        ("_gfortran_stop_numeric", true)
        ("_gfortran_stop_string", true)
        ("_gfortrani_exit_error", true)
        ("_gfortrani_internal_error", true)
        ("_gfortrani_os_error", true)
        ("_gfortrani_runtime_error", true)
        ("_gfortrani_runtime_error_at", true)
        ("_gfortrani_sys_abort", true)

        // libgomp.so from gcc 9.3.0
        ("GOMP_PLUGIN_fatal", true)
        ("gomp_fatal", true)
        ("gomp_vfatal", true)

        // old and misc functions
        ("__error_at_line_noreturn", true)
        ("__error_noreturn", true)
        ("__longjmp_chk", true)
        ("quick_exit", true)
        ("__f90_stop", true)
        ("fancy_abort", true)
        ("ExitProcess", true)
        ("_Unwind_Resume", true)
        ("__longjmp", true)
        ("_gfortran_abort", true)
        ("_gfortran_exit_i8", true)
        ("_gfortran_exit_i4", true)
        ("for_stop_core", true)
        ("__sys_exit", true)
        ;

dyn_hash_map<int, bool>
CodeSource::non_returning_syscalls_x86 =
    boost::assign::map_list_of
        (1 /*exit*/,true)
        (119 /*sigreturn*/, true);

dyn_hash_map<int, bool>
CodeSource::non_returning_syscalls_x86_64 =
    boost::assign::map_list_of
        (60 /*exit*/,true)
        (15 /*sigreturn*/, true);

bool
CodeSource::nonReturning(string name)
{
#if defined(os_windows)
	// We see MSVCR<N>.exit
	// Of course, it's often reached via indirect call, but hope never fails.
	if ((name.compare(0, strlen("MSVCR"), "MSVCR") == 0) &&
		(name.find("exit") != name.npos)) return true;
#endif
	parsing_printf("Checking non-returning for %s\n", name.c_str());
    return non_returning_funcs.find(name) != non_returning_funcs.end();
}

bool
InstructionSource::isAligned(const Address addr) const
{
    switch (getArch()) {
        case Arch_aarch64:
	case Arch_ppc32:
	case Arch_ppc64:
	    return !(addr & 0x3);
	case Arch_x86:
	case Arch_x86_64:
	    return true;
	default:
	    assert(!"unimplemented architecture");
    }
}
