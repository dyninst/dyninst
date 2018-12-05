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

#include "parseAPI/h/InstructionAdapter.h"

#include "parse-cfg.h"

#include "Parsing.h"
#include "debug.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

#if defined(arch_power) || defined(arch_aarch64)
void 
DynParseCallback::instruction_cb(Function*f,Block *,Address,insn_details*det)
{
    parse_func * ifunc = static_cast<parse_func*>(f);
    /* In case we do callback from a place where the leaf function analysis has not been done */ 
    Address ret_addr = 0;
    if (f->_is_leaf_function) { 
    	f->_is_leaf_function = !(det->insn->isReturnAddrSave(ret_addr));
    	if (!f->_is_leaf_function) f->_ret_addr = ret_addr;
    }
    ifunc->saves_return_addr_ = !(f->_is_leaf_function);
}
#endif

