/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

/*
 * inst-x86.C - x86 dependent functions and code generator
 */

#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/Vector.h"
#include "image-func.h"
#include "instPoint.h"
#include "symtab.h"
#include "dyninstAPI/h/BPatch_Set.h"
#include "debug.h"
#include <deque>
#include <set>
#include <algorithm>
//#include "arch.h"

#include "instructionAPI/h/Instruction.h"
#include "instructionAPI/h/InstructionDecoder.h"

using namespace Dyninst::ParseAPI;

bool image_func::writesFPRs(unsigned level) {
    using namespace Dyninst::InstructionAPI;
    // Oh, we should be parsed by now...
    if (!parsed()) image_->analyzeIfNeeded();

    if (containsFPRWrites_ == unknown) {
        // Iterate down and find out...
        // We know if we have callees because we can
        // check the instPoints; no reason to iterate over.
        // We also cache callee values here for speed.

        if (level >= 3) {
            return true; // Arbitrarily decided level 3 iteration.
        }        
        Function::edgelist & calls = callEdges();
        Function::edgelist::iterator cit = calls.begin();
        for( ; cit != calls.end(); ++cit) {
            image_edge * ce = static_cast<image_edge*>(*cit);
            image_func * ct = static_cast<image_func*>(
                obj()->findFuncByEntry(region(),ce->trg()->start()));
            if(ct && ct != this) {
                if (ct->writesFPRs(level+1)) {
                    // One of our kids does... if we're top-level, cache it; in 
                    // any case, return
                    if (level == 0)
                        containsFPRWrites_ = used;
                    return true;
                }
            }
            else if(!ct){
                // Indirect call... oh, yeah. 
                if (level == 0)
                    containsFPRWrites_ = used;
                return true;
            }
        }

        // No kids contain writes. See if our code does.
        static RegisterAST::Ptr st0(new RegisterAST(x86::st0));
        static RegisterAST::Ptr st1(new RegisterAST(x86::st1));
        static RegisterAST::Ptr st2(new RegisterAST(x86::st2));
        static RegisterAST::Ptr st3(new RegisterAST(x86::st3));
        static RegisterAST::Ptr st4(new RegisterAST(x86::st4));
        static RegisterAST::Ptr st5(new RegisterAST(x86::st5));
        static RegisterAST::Ptr st6(new RegisterAST(x86::st6));
        static RegisterAST::Ptr st7(new RegisterAST(x86::st7));

        vector<FuncExtent *>::const_iterator eit = extents().begin();
        for( ; eit != extents().end(); ++eit) {
            FuncExtent * fe = *eit;
        
            const unsigned char* buf = (const unsigned char*)
                isrc()->getPtrToInstruction(fe->start());
            if(!buf) {
                parsing_printf("%s[%d]: failed to get insn ptr at %lx\n",
                    FILE__, __LINE__,fe->start());
                // if the function cannot be parsed, it is only safe to 
                // assume that the FPRs are written -- mcnulty
                return true; 
            }
            InstructionDecoder d(buf,fe->end()-fe->start(),isrc()->getArch());
            Instruction::Ptr i;

            while(i = d.decode()) {
                if(i->isWritten(st0) ||
                    i->isWritten(st1) ||
                    i->isWritten(st2) ||
                    i->isWritten(st3) ||
                    i->isWritten(st4) ||
                    i->isWritten(st5) ||
                    i->isWritten(st6) ||
                    i->isWritten(st7)
                   )
                {
                    containsFPRWrites_ = used;
                    return true;
                }
            }
        }
        // No kids do, and we don't. Impressive.
        containsFPRWrites_ = unused;
        return false;
    }
    else if (containsFPRWrites_ == used) {
        return true;
    }
    else if (containsFPRWrites_ == unused) {
        return false;
    }

    fprintf(stderr, "ERROR: function %s, containsFPRWrites_ is %d (illegal value!)\n", 
	    symTabName().c_str(), containsFPRWrites_);
    
    assert(0);
    return false;
}
