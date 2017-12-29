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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include <fstream>
#include <set>
#include <string>
#include <vector>

// parseAPI
#include "CodeSource.h"
#include "CodeObject.h"
#include "CFG.h"

// symtabAPI
#include "Function.h"

// instructionAPI
#include "InstructionDecoder.h"
#include "Register.h"
#include "Dereference.h"
#include "Immediate.h"

// dataflowAPI
#include "SymEval.h"
#include "slicing.h"

// local
#include "util.h"
#include "types.h"
#include "semanticDescriptor.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::SymtabAPI;
using namespace Dyninst::InstructionAPI;
using namespace Dyninst::DataflowAPI;

/*
 * Retrieve instructions
 */
void getInsnInstances(ParseAPI::Block *block,
		      Slicer::InsnVec &insns) {
  Offset off = block->start();
  const unsigned char *ptr = (const unsigned char *)block->region()->getPtrToInstruction(off);
  if (ptr == NULL) return;
  InstructionDecoder d(ptr, block->size(), block->obj()->cs()->getArch());
  while (off < block->end()) {
    insns.push_back(std::make_pair(d.decode(), off));
    off += insns.back().first->size();
  }
}

/* 
 * Determine if insn is a system call
 */
bool isSyscall(Instruction::Ptr & insn, Address & syscallTrampStore)
{
    static RegisterAST::Ptr gs(new RegisterAST(x86::gs));

    Immediate::Ptr syscallTrampPtr = Immediate::makeImmediate(Result(s32,
                syscallTrampStore));
    Dereference syscallTrampCall(syscallTrampPtr, u32);

    return (((insn->getOperation().getID() == e_call) &&
                (insn->isRead(gs))) ||
            ((insn->getOperation().getID() == e_call) &&
             (*(insn->getControlFlowTarget()) == syscallTrampCall)) ||
            (insn->getOperation().getID() == e_syscall) ||
            (insn->getOperation().getID() == e_int) ||
            (insn->getOperation().getID() == power_op_sc));
}


/* 
 * Check if a given instruction is an indirect call through the _dl_sysinfo symbol.
 * If so, store the symbol address.
 */
bool isCallToSyscallTrampStore(Instruction::Ptr & insn, Address & _syscallTramp) 
{
    if (insn->getOperation().getID() == e_call) {
        Expression::Ptr cft = insn->getControlFlowTarget();
        if (typeid(cft) == typeid(Dereference::Ptr)) {
            vector<InstructionAST::Ptr> children;
            cft->getChildren(children);
            if (children.size() == 1) {
                InstructionAST::Ptr child = children.front();
                Expression::Ptr immed = boost::dynamic_pointer_cast<Expression>(child);
                Result res = immed->eval();
                Address syscallTramp = res.convert<Address>();
                if (syscallTramp) {
                    _syscallTramp = syscallTramp;
                    return true;
                }
            }
        }
    }
    
    return false;
}

/* 
 * Locate the address of the _dl_sysinfo symbol.
 *
 * This version will only work in cases where we have some amount of symbol 
 * information. In the more likely case, we'll take more extreme measures to 
 * locate this value.
 */
Address getSyscallTrampStore(SymtabAPI::Symtab * symtab)
{
    vector<Symbol *> symVec;
    if (!symtab->findSymbol(symVec, 
                "_dl_sysinfo", 
                Symbol::ST_UNKNOWN,
                anyName,
                false,
                false)) {
        return 0;
    } else {
        Symbol * sym = *(symVec.begin());
        Offset symAddr = sym->getOffset();
        return symAddr;
    }   
}



/* 
 * If we were unable to locate the _dl_sysinfo symbol directly, use heuristics
 * to locate it
 */
Address searchForSyscallTrampStore(CodeObject::funclist & procedures) 
{ 
    Address syscallTramp; 
    CodeObject::funclist::iterator pIter; 

    for (pIter = procedures.begin(); pIter != procedures.end(); ++pIter) {
        ParseAPI::Function::blocklist blocks = (*pIter)->blocks();

        ParseAPI::Function::blocklist::iterator bIter;
        for (bIter = blocks.begin(); bIter != blocks.end(); ++bIter) {
            std::vector<std::pair<Instruction::Ptr, Address> > insns;
            getInsnInstances(*bIter, insns);
            std::vector<std::pair<Instruction::Ptr, Address> >::iterator insnIter;
            for (insnIter= insns.begin();
                    insnIter != insns.end();
                    ++insnIter) {
                std::pair<Instruction::Ptr, Address> insn = *insnIter;
                if (isCallToSyscallTrampStore(insn.first, syscallTramp)) {
                    return syscallTramp;
                }   
            }
        }
    }

    return 0;
}
