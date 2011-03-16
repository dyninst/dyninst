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
#include <stdio.h>

#include "parseAPI/h/InstructionAdapter.h"

#if defined(arch_sparc)
// XXX needs raw instruction from iterator
#include "parseAPI/src/IA_InstrucIter.h"
#endif

#include "image-func.h"
#include "Parsing.h"
#include "debug.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

#if defined(arch_power)
void 
DynParseCallback::instruction_cb(Function*f,Address,insn_details*det)
{
    image_func * ifunc = static_cast<image_func*>(f);
    /* In case we do callback from a place where the leaf function analysis has not been done */ 
    Address ret_addr = 0;
    if (f->_is_leaf_function) { 
    	f->_is_leaf_function = !(det->insn->isReturnAddrSave(ret_addr));
    	if (!f->_is_leaf_function) f->_ret_addr = ret_addr;
    }
    ifunc->ppc_saves_return_addr_ = !(f->_is_leaf_function);
}
#endif

#if defined(arch_sparc)
void
DynParseCallback::instruction_cb(Function*f,Address,insn_details*det)
{
    image_func * ifunc = static_cast<image_func*>(f);

    // Check whether "07" is live, AKA we can't call safely.
    // Could we just always assume this?
    if (!ifunc->o7_live) {
       
#if 0
	InsnRegister reads[7];
	InsnRegister writes[7];

	ah.getInstruction().get_register_operands(reads, writes);
	int i;
        for(i=0; i<7; i++) {
	  if (reads[i].is_o7()) {
	    o7_live = true;
	    break;
	  }
	}
#endif
        InsnAdapter::IA_InstrucIter& ah = 
            *static_cast<InsnAdapter::IA_InstrucIter*>(det->insn);
        instruction insn = ah.getInstruction();
        insn.get_register_operands();
        std::vector<InsnRegister> *read_regs_p  = NULL;
        extern AnnotationClass<std::vector<InsnRegister> > RegisterReadSetAnno;
        if (!insn.getAnnotation(read_regs_p, RegisterReadSetAnno))
        {
           return;
        }
        assert(read_regs_p);
        std::vector<InsnRegister> &read_regs = *read_regs_p;
#if 0
        Annotatable<InsnRegister, register_read_set_a> &read_regs = insn;
#endif
        assert(read_regs.size() < 9);

        for (unsigned int i = 0; i < read_regs.size(); ++i) {
           if (read_regs[i].is_o7()) {
              ifunc->o7_live = true;
              break;
           }
        }

	if(ifunc->o7_live) {	  
            parsing_printf("Setting o7 to live at 0x%x, func %s\n",
                    ah.getAddr(), ifunc->symTabName().c_str());
        }
    }
}
#endif
