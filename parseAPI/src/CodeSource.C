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
	    // for(auto i = exist.begin();
	    // 	i != exist.end();
	    // 	++i)
	    // {
	    // 	std::cerr << "Region " << **i << " overlaps " << *cr << std::endl;
	    // }
            _regions_overlap = true;
	}
    }

    _region_tree.insert(cr);
}

int
CodeSource::findRegions(Address addr, set<CodeRegion *> & ret) const
{
    return _region_tree.find(addr,ret);
}

dyn_hash_map<std::string, bool>
CodeSource::non_returning_funcs =
    boost::assign::map_list_of
        ("exit",true)
        ("abort",true)
        ("__f90_stop",true)
        ("fancy_abort",true)
        ("__stack_chk_fail",true)
        ("__assert_fail",true)
        ("ExitProcess",true)
        ("_ZSt17__throw_bad_allocv",true)
        ("_ZSt20__throw_length_errorPKc",true)
        ("_Unwind_Resume",true)
        ("longjmp",true)
	("__longjmp",true)
        ("siglongjmp",true)
        ("_ZSt16__throw_bad_castv",true)
        ("_ZSt19__throw_logic_errorPKc",true)
        ("_ZSt20__throw_out_of_rangePKc",true)
        ("__cxa_rethrow",true)
        ("__cxa_throw",true)
        ("_ZSt21__throw_runtime_errorPKc",true)
	("_ZSt9terminatev",true)
        ("_gfortran_os_error",true)
        ("_gfortran_runtime_error",true)
        ("_gfortran_stop_numeric", true)
        ("_gfortran_runtime_error_at", true)
        ("_gfortran_stop_string", true)
        ("_gfortran_abort", true)
        ("_gfortran_exit_i8", true)
        ("_gfortran_exit_i4", true)
        ("for_stop_core", true)
        ("__sys_exit", true);

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
