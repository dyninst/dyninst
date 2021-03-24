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
#include "Parser.h"

#include "CodeObject.h"
#include "CFG.h"
#include "IA_IAPI.h"
using namespace Dyninst::InstructionAPI;
#include "InstructionAdapter.h"

#include "debug_parse.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;

int HACKCOUNT = 0;

Block::Block(CodeObject * o, CodeRegion *r, Address start, Function *f) :
    SimpleInterval(start, start, 0),
    _obj(o),
    _region(r),
    _start(start),
    _end(start),
    _lastInsn(start),
    _func_cnt(0),
    _parsed(false),
    _createdByFunc(f)
{
    if (_obj && _obj->cs()) {
        _obj->cs()->incrementCounter(PARSE_BLOCK_COUNT);
        _obj->cs()->addCounter(PARSE_BLOCK_SIZE, size());
    }
}

Block::Block(
    CodeObject * o,
    CodeRegion *r,
    Address start,
    Address end,
    Address last,
    Function *f) :
    SimpleInterval(start, end, 0),
    _obj(o),
    _region(r),
    _start(start),
    _end(end),
    _lastInsn(last),
    _func_cnt(0),
    _parsed(false),
    _createdByFunc(f)
{
}


Block::~Block()
{
    // nothing special
    if (_obj && _obj->cs()) {
        _obj->cs()->decrementCounter(PARSE_BLOCK_COUNT);
        _obj->cs()->addCounter(PARSE_BLOCK_SIZE, -1*size());
    }
}

bool
Block::consistent(Address addr, Address & prev_insn) 
{
    if (addr >= end() || addr < start()) return false;
    InstructionSource * isrc;
    if(_obj && !_obj->cs()->regionsOverlap())
        isrc = _obj->cs();
    else
        isrc = region();
    const unsigned char * buf =
        (const unsigned char*)(region()->getPtrToInstruction(_start));
    InstructionDecoder dec(buf,size(),isrc->getArch());
    InstructionAdapter_t* ah = InstructionAdapter_t::makePlatformIA_IAPI(_obj->cs()->getArch(), dec,_start,_obj,region(),isrc, this);

    Address cur = ah->getAddr();
    //parsing_printf("consistency check for [%lx,%lx), start: %lx addr: %lx\n",
        //start(),end(),cur,addr);
    while(cur < addr) {
        ah->advance();
        prev_insn = cur;
        cur = ah->getAddr();
        //parsing_printf(" cur: %lx\n",cur);
    }
    delete ah;
    return cur == addr;
}

void
Block::getFuncs(vector<Function *> & funcs)
{
    if(!_obj) return; // universal sink
    set<Function *> stab;
    _obj->findFuncsByBlock(region(),this,stab);
    set<Function *>::iterator sit = stab.begin();
    for( ; sit != stab.end() ;++sit) {
        if(((const Function*)(*sit))->contains(this))
            funcs.push_back(*sit);
    }
}

bool
EdgePredicate::pred_impl(Edge *) const
{
  return true;
}

bool
Intraproc::pred_impl(Edge * e) const
{
    bool base = EdgePredicate::pred_impl(e);
    return base && (e->type() != CALL) && (e->type() != RET) && (!e->interproc());
}

bool Interproc::pred_impl(Edge *e) const 
{
    bool base = EdgePredicate::pred_impl(e);

    return !base || ((e->type() == CALL) || (e->type() == RET)) || (e->interproc());
}

bool
SingleContext::pred_impl(Edge * e) const
{
    bool base = EdgePredicate::pred_impl(e);
    return base && !e->interproc() && 
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
    if(_obj) _obj->finalize();
    return _func_cnt;
}

void Block::removeFunc(Function *) 
{
    if ((0 == _func_cnt) && _obj) {
        _obj->finalize();
    }
    assert(0 != _func_cnt);
    _func_cnt.fetch_add(-1);
}

void Block::updateEnd(Address addr)
{
    if(!_obj) return;
    _obj->cs()->addCounter(PARSE_BLOCK_SIZE, -1*size());   
    _end = addr;
    high_ = addr;
    _obj->cs()->addCounter(PARSE_BLOCK_SIZE, size());
}

void Block::destroy(Block *b) {
   b->obj()->destroy(b);
}

void Edge::install()
{
    src()->addTarget(this);
    trg()->addSource(this);
}

void Edge::uninstall()
{
    mal_printf("Uninstalling edge [%lx]->[%lx]\n", 
               src()->lastInsnAddr(), _target_off);
    // if it's a call edge, it's cached in the function object, remove it
    if (CALL == type()) {
        vector<Function*> srcFs;
        src()->getFuncs(srcFs);
        for (vector<Function*>::iterator fit = srcFs.begin(); 
             fit != srcFs.end(); fit++) 
        {
            if ( ! (*fit)->_cache_valid ) {
                continue;
            }
            for (set<Edge*>::iterator eit = (*fit)->_call_edge_list.begin();
                 eit != (*fit)->_call_edge_list.end(); eit++) 
            {
                if (this == (*eit)) {
                    (*fit)->_call_edge_list.erase(*eit);
                    break;
                }
            }
        }
    }
    // remove from source and target blocks
    src()->removeTarget(this);
    trg()->removeSource(this);
}

void Edge::destroy(Edge *e, CodeObject *o) {
   o->destroy(e);
}


Block *Edge::trg() const {
   return _target;
}

std::string format(EdgeTypeEnum e) {
	switch(e) {
		case CALL: return "call";
		case COND_TAKEN: return "cond_taken";
		case COND_NOT_TAKEN: return "cond_not_taken";
		case INDIRECT: return "indirect";
		case DIRECT: return "direct";
		case FALLTHROUGH: return "fallthrough";
		case CATCH: return "catch";
		case CALL_FT: return "call_ft";
		case RET: return "ret";
		case NOEDGE: return "noedge";
		default: return "<unknown>";
	}
}

bool ParseAPI::Block::wasUserAdded() const {
   return region()->wasUserAdded(); 
}

void
Block::getInsns(Insns &insns) const {
 Offset off = start();
  const unsigned char *ptr =
    (const unsigned char *)region()->getPtrToInstruction(off);
  if (ptr == NULL) return;
  InstructionDecoder d(ptr, size(), obj()->cs()->getArch());
  while (off < end()) {
    Instruction insn = d.decode();
    insns[off] = insn;
    off += insn.size();
  }
}

InstructionAPI::Instruction
Block::getInsn(Offset a) const {
   Insns insns;
   getInsns(insns);
   return insns[a];
}


bool Block::operator==(const Block &rhs) const {
    boost::lock_guard<const Block> g1(*this);
    boost::lock_guard<const Block> g2(rhs);
    // All sinks are equal
    if(_start == std::numeric_limits<Address>::max()) {
        return rhs._start == _start;
    }
    return _obj == rhs._obj &&
           _region == rhs._region &&
           _start == rhs._start &&
           _end == rhs._end &&
           _lastInsn == rhs._lastInsn &&
           _srclist == rhs._srclist &&
           _trglist == rhs._trglist &&
           _func_cnt == rhs._func_cnt &&
           _parsed == rhs._parsed;
}

bool Block::operator!=(const Block &rhs) const {
    return !(rhs == *this);
}

void Block::addSource(Edge * e) 
{
    boost::lock_guard<Block> g(*this);
    _srclist.insert(e);
}

void Block::addTarget(Edge * e)
{
    boost::lock_guard<Block> g(*this);
    if(e->type() == FALLTHROUGH ||
            e->type() == COND_NOT_TAKEN)
    {
        assert(e->_target_off == end());
    }
    _trglist.insert(e);

}

void Block::removeTarget(Edge * e)
{
    if (e == NULL) return;
    boost::lock_guard<Block> g(*this);
    _trglist.erase(e);
}

void Block::removeSource(Edge * e) {
    if (e == NULL) return;
    boost::lock_guard<Block> g(*this);
    _srclist.erase(e);
}

void Block::moveTargetEdges(Block* B) {
    if (this == B) return;
    boost::lock_guard<Block> g(*this);
    Block* A = this;
    /* We move outgoing edges from this block to block B, which is 
     * necessary when spliting blocks.
     * The start of block B should be consistent with block A.
     *
     */
    Block::edgelist &trgs = _trglist;
    Block::edgelist::iterator tit = trgs.begin();
	for (; tit != trgs.end(); ++tit) {
        ParseAPI::Edge *e = *tit;
        // Helgrind gets confused, we use a cmp&swap to hide the write.
        assert(e->_source.compare_exchange_strong(A, B));
        B->addTarget(e);
	}
    trgs.clear();
}
