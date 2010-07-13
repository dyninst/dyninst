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
#include "CodeObject.h"
#include "CFG.h"
#if defined(cap_instruction_api)
#include "IA_IAPI.h"
using namespace Dyninst::InstructionAPI;
#else
#include "IA_InstrucIter.h"
#endif
#include "InstructionAdapter.h"

#include "Parser.h"
#include "debug.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

Block::Block(CodeObject * o, CodeRegion *r, Address start) :
    obj_(o),
    region_(r),
    start_(start),
    end_(start),
    lastInsn_(start),
    srclist_(sources_),
    trglist_(targets_),
    func_cnt_(0),
    parsed_(false)
{

}

Block::~Block()
{
    // nothing special
}

bool
Block::consistent(Address addr, Address & prev_insn) const
{
    InstructionSource * isrc;
    if(!obj_->cs()->regionsOverlap())
        isrc = obj_->cs();
    else
        isrc = region();
#if defined(cap_instruction_api)
    const unsigned char * buf = 
        (const unsigned char*)(region()->getPtrToInstruction(start_));
    InstructionDecoder dec(buf,size(),isrc->getArch());
    InstructionAdapter_t ah(dec,start_,obj_,region(),isrc);
#else
    InstrucIter iter(start_,size(),isrc);
    InstructionAdapter_t ah(iter,obj_,region(),isrc);
#endif

    Address cur = ah.getAddr();
    //parsing_printf("consistency check for [%lx,%lx), start: %lx addr: %lx\n",
        //start(),end(),cur,addr);
    while(cur < addr) {
        ah.advance();
        prev_insn = cur;
        cur = ah.getAddr();
        //parsing_printf(" cur: %lx\n",cur);
    }
    return cur == addr;
}

void
Block::getFuncs(vector<Function *> & funcs)
{
    set<Function *> stab;
    obj_->findFuncs(region(),start(),stab);
    set<Function *>::iterator sit = stab.begin();
    for( ; sit != stab.end() ;++sit) {
        if((*sit)->contains(this))
            funcs.push_back(*sit);
    }
}

bool
EdgePredicate::pred_impl(Edge * e) const
{
    if(next_)
        return (*next_)(e);
    else
        return true;
}

bool
Intraproc::pred_impl(Edge * e) const
{
    bool base = EdgePredicate::pred_impl(e);
    return base && (e->type() != CALL) && (e->type() != RET);
}

bool
SingleContext::pred_impl(Edge * e) const
{
    bool base = EdgePredicate::pred_impl(e);
    return base && 
        (!forward_ || context_->contains(e->trg())) &&
        (!backward_ || context_->contains(e->src()));
}

int Block::containingFuncs() const {
    obj_->finalize();
    return func_cnt_;
}

