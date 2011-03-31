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
#include "CodeObject.h"
#include "CFG.h"
#include "IA_IAPI.h"
using namespace Dyninst::InstructionAPI;
#include "InstructionAdapter.h"

#include "Parser.h"
#include "debug_parse.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

Block::Block(CodeObject * o, CodeRegion *r, Address start) :
    _obj(o),
    _region(r),
    _start(start),
    _end(start),
    _lastInsn(start),
    _srclist(_sources),
    _trglist(_targets),
    _func_cnt(0),
    _parsed(false)
{

}

Block::~Block()
{
    // nothing special
}

bool
Block::consistent(Address addr, Address & prev_insn) 
{
    InstructionSource * isrc;
    if(!_obj->cs()->regionsOverlap())
        isrc = _obj->cs();
    else
        isrc = region();
    const unsigned char * buf =
        (const unsigned char*)(region()->getPtrToInstruction(_start));
    InstructionDecoder dec(buf,size(),isrc->getArch());
    InstructionAdapter_t ah(dec,_start,_obj,region(),isrc, this);

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
    _obj->findFuncs(region(),start(),stab);
    set<Function *>::iterator sit = stab.begin();
    for( ; sit != stab.end() ;++sit) {
        if((*sit)->contains(this))
            funcs.push_back(*sit);
    }
}

bool
EdgePredicate::pred_impl(Edge * e) const
{
    if(_next)
        return (*_next)(e);
    else
        return true;
}

bool
Intraproc::pred_impl(Edge * e) const
{
    bool base = EdgePredicate::pred_impl(e);
    return base && (e->type() != CALL) && (e->type() != RET);
}

bool Interproc::pred_impl(Edge *e) const 
{
    bool base = EdgePredicate::pred_impl(e);

    return !base || ((e->type() == CALL) || (e->type() == RET));
}

bool
SingleContext::pred_impl(Edge * e) const
{
    bool base = EdgePredicate::pred_impl(e);
    return base && 
        (!_forward || _context->contains(e->trg())) &&
        (!_backward || _context->contains(e->src()));
}

bool
SingleContextOrInterproc::pred_impl(Edge * e) const
{
    bool base = EdgePredicate::pred_impl(e);

    bool singleContext = base && 
        (!_forward || _context->contains(e->trg())) &&
        (!_backward || _context->contains(e->src()));

    bool interproc = !base || ((e->type() == CALL) || (e->type() == RET));

    return singleContext || interproc;
}

int Block::containingFuncs() const {
    _obj->finalize();
    return _func_cnt;
}

void Block::removeFunc(Function *) 
{
    if (0 == _func_cnt) {
        _obj->finalize();
    }
    assert(0 != _func_cnt);
    _func_cnt --;
}

void Edge::install()
{
    src()->addTarget(this);
    trg()->addSource(this);
}
