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
 
#include "function.h"

#include "process.h"
#include "instPoint.h"

#include "mapped_object.h"
#include "mapped_module.h"
#include "InstructionDecoder.h"
#include <set>
#include <sstream>

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

int_block::int_block(image_basicBlock *ib, int_function *func) 
  : highlevel_block(NULL),
    func_(func),
    ib_(ib) {};

// Fork constructor
int_block::int_block(const int_block *parent, int_function *func) :
    highlevel_block(NULL),
    func_(func),
    ib_(parent->ib_)
{}

int_block::~int_block() {}

Address int_block::start() const {
    return ib_->start() + func_->baseAddr();
}

Address int_block::end() const {
    return ib_->end() + func_->baseAddr();
}

Address int_block::last() const {
    return ib_->lastInsnAddr() + func_->baseAddr();
}

unsigned int_block::size() const {
    return ib_->size();
}

int_function *int_block::func() const { 
    return func_;
}

AddressSpace *int_block::addrSpace() const { 
    return func()->proc(); 
}

bool int_block::isEntryBlock() const { 
    return ib_->isEntryBlock(func_->ifunc());
}

std::string int_block::format() const {
    stringstream ret;
    ret << "BB(" 
        << hex << start()
        << ".." 
        << end() << dec << endl;
    return ret.str();
}

#if defined(cap_instruction_api) 
void int_block::getInsnInstances(InsnInstances &instances) const {
  instances.clear();
  llb()->getInsnInstances(instances);
  for (unsigned i = 0; i < instances.size(); ++i) {
      instances[i].second += func()->baseAddr();
  }
}

std::string int_block::disassemble() const {
    stringstream ret;
    InsnInstances instances;
    getInsnInstances(instances);
    for (unsigned i = 0; i < instances.size(); ++i) {
        ret << "\t" << hex << instances[i].second << ": " << instances[i].first->format() << dec << endl;
    }
    return ret.str();
}
#endif

void *int_block::getPtrToInstruction(Address addr) const {
    if (addr < start()) return NULL;
    if (addr > end()) return NULL;
    return func()->obj()->getPtrToInstruction(addr);
}

// Note that code sharing is masked at this level. That is, edges
// to and from a block that do not originate from the low-level function
// that this block's int_function represents will not be included in
// the returned block collection
void int_block::getSources(std::vector<int_block *> &ins) const {

    /* Only allow edges that are within this current function; hide sharing */
    /* Also avoid CALL and RET edges */

    SingleContext epred(func()->ifunc(),true,true);
    Intraproc epred2(&epred);

    Block::edgelist & ib_ins = ib_->sources();
    Block::edgelist::iterator eit = ib_ins.begin(&epred2);

    for( ; eit != ib_ins.end(); ++eit) {
        // FIXME debugging assert
        assert((*eit)->type() != CALL && (*eit)->type() != RET);

        image_basicBlock * sb = (image_basicBlock*)(*eit)->src();
        int_block *sblock = func()->findBlock(sb);
        if (!sblock) {
            fprintf(stderr,"ERROR: no corresponding intblock for "
                    "imgblock #%d at 0x%lx %s[%d]\n", ib_->id(),
                    ib_->firstInsnOffset(),FILE__,__LINE__); 
            assert(0);
        }
        ins.push_back( sblock );
    }
}

void int_block::getTargets(pdvector<int_block *> &outs) const {
    SingleContext epred(func()->ifunc(),true,true);
    Intraproc epred2(&epred);
    NoSinkPredicate epred3(&epred2);

    Block::edgelist & ib_outs = ib_->targets();
    Block::edgelist::iterator eit = ib_outs.begin(&epred3);

    for( ; eit != ib_outs.end(); ++eit) {
        // FIXME debugging assert
        assert((*eit)->type() != CALL && (*eit)->type() != RET);
        image_basicBlock * tb = (image_basicBlock*)(*eit)->trg();
        int_block* tblock = func()->findBlock(tb);
        if (!tblock) {
            fprintf(stderr,"ERROR: no corresponding intblock for "
                    "imgblock #%d at 0x%lx %s[%d]\n", ib_->id(),
                    ib_->firstInsnOffset(),FILE__,__LINE__);                    
            assert(0);
        }
        outs.push_back(tblock);
    }
}

EdgeTypeEnum int_block::getTargetEdgeType(int_block * target) const {
    SingleContext epred(func()->ifunc(),true,true);
    Block::edgelist & ib_outs = ib_->targets();
    Block::edgelist::iterator eit = ib_outs.begin(&epred);
    for( ; eit != ib_outs.end(); ++eit)
        if((*eit)->trg() == target->ib_)
            return (*eit)->type();
    return NOEDGE;
}

EdgeTypeEnum int_block::getSourceEdgeType(int_block *source) const {
    SingleContext epred(func()->ifunc(),true,true);
    Block::edgelist & ib_ins = ib_->sources();
    Block::edgelist::iterator eit = ib_ins.begin(&epred);
    for( ; eit != ib_ins.end(); ++eit)
        if((*eit)->src() == source->ib_)
            return (*eit)->type();
    return NOEDGE;
}

int_block *int_block::getFallthrough() const {
    SingleContext epred(func()->ifunc(),true,true);
    NoSinkPredicate epred2(&epred);
    Block::edgelist & ib_outs = ib_->targets();
    Block::edgelist::iterator eit = ib_outs.begin(&epred2);
    for( ; eit != ib_outs.end(); ++eit) {
        Edge * e = *eit;
        if(e->type() == FALLTHROUGH ||
           e->type() == CALL_FT ||
           e->type() == COND_NOT_TAKEN)
        {
            return func()->findBlock(e->trg());
        }
    }
    return NULL;
}

int_block *int_block::getTarget() const {
    SingleContext epred(func()->ifunc(),true,true);
    NoSinkPredicate epred2(&epred);
    Block::edgelist & ib_outs = ib_->targets();
    Block::edgelist::iterator eit = ib_outs.begin(&epred2);
    for( ; eit != ib_outs.end(); ++eit) {
        Edge * e = *eit;
        if(e->type() == DIRECT||
           e->type() == COND_TAKEN)
        {
            return func()->findBlock(e->trg());
        }
    }
    return NULL;
}
void int_block::setHighLevelBlock(BPatch_basicBlock *newb)
{
   highlevel_block = newb;
}

BPatch_basicBlock *int_block::getHighLevelBlock() const {
   return highlevel_block;
}

bool int_block::containsCall()
{
    Block::edgelist & out_edges = llb()->targets();
    Block::edgelist::iterator eit = out_edges.begin();
    for( ; eit != out_edges.end(); ++eit) {
        if ( CALL == (*eit)->type() ) {
            return true;
        }
    }
    return false;
}

int int_block::id() const {
    return llb()->id();
}
