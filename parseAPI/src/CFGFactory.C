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
#include "ParseData.h"

#include "LoopAnalyzer.h"
#include <limits>

#include "CFGFactory.h"
#include "CFG.h"
#include <iostream>

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

std::string ParseAPI::format(EdgeTypeEnum e) {
   switch(e) {
      case CALL:
         return "call";
      case COND_TAKEN:
         return "cond_taken";
      case COND_NOT_TAKEN:
         return "cond_not_taken";
      case INDIRECT:
         return "indirect";
      case DIRECT:
         return "direct";
      case FALLTHROUGH:
         return "fallthrough";
      case CATCH:
         return "catch";
      case CALL_FT:
         return "call_ft";
      case RET:
         return "ret";
      case NOEDGE:
         return "noedge";
      case SYS_CALL:
         return "syscall";
      default:
         return "<unknown>";
   }
}

/** The default CFG object factory. Alternative
    implementations might alllocate objects that
    override the default CFG interfaces, or might
    use pooled allocators, etc.
 */

Edge::EdgeType::EdgeType(EdgeTypeEnum t, bool s)
    : _kind(K_NOEDGE), _sink(s), _conditional(false), _indirect(false), _tailCall(false) {
    switch (t) {
        case CALL: _kind = K_CALL; break;
        case SYS_CALL: _kind = K_SYSCALL; break;
        case COND_TAKEN: _kind = K_BRANCH; _conditional = true; break;
        case COND_NOT_TAKEN: _kind = K_FALLTHROUGH; _conditional = true; break;
        case INDIRECT: _kind = K_BRANCH; _indirect = true; break;
        case DIRECT: _kind = K_BRANCH; break;
        case FALLTHROUGH: _kind = K_FALLTHROUGH; break;
        case CATCH: _kind = K_CATCH; break;
        case CALL_FT: _kind = K_CALL_FT; break;
        case RET: _kind = K_RET; _indirect = true; break;
        default: break;
    }
}

EdgeTypeEnum Edge::type() const {
    bool cond = isConditional();
    switch (_type._kind) {
        case K_CALL: return CALL;
        case K_SYSCALL: return SYS_CALL;
        case K_RET: return cond ? COND_TAKEN : RET;
        case K_BRANCH:
            if (cond) return COND_TAKEN;
            return isIndirect() ? INDIRECT : DIRECT;
        case K_FALLTHROUGH: return cond ? COND_NOT_TAKEN : FALLTHROUGH;
        case K_CALL_FT: return CALL_FT;
        case K_CATCH: return CATCH;
        default: return NOEDGE;
    }
}

Edge::Edge(Block *source, Block *target, EdgeTypeEnum type)
: _source(source),
  _target(target),
  index(source->obj()->parse_data()),
  _target_off(target->low()),
  _from_index(true),
  _type(type,false) {
      
    }

CFGFactory::~CFGFactory()
{
  for(Edge *e : edges_) {
    destroy_edge(e, destroyed_all);
  }
  for(Block *b : blocks_) {
    destroy_block(b);
  }
  for(Function *f : funcs_) {
    destroy_func(f);
  }
}

// ParseAPI call...
Function *
CFGFactory::_mkfunc(Address addr, FuncSource src, string name, 
    CodeObject * obj, CodeRegion * reg, Dyninst::InstructionSource * isrc)
{
   Function * ret = mkfunc(addr,src,name,obj,reg,isrc);

   funcs_.add(ret);
   ret->_src =  src;
   return ret;
}

// And user-overriden create
Function *
CFGFactory::mkfunc(Address addr, FuncSource, string name, 
    CodeObject * obj, CodeRegion * reg, Dyninst::InstructionSource * isrc)
{
    Function * ret = new Function(addr,name,obj,reg,isrc);

    return ret;
}

Block *
CFGFactory::_mkblock(Function *  f , CodeRegion *r, Address addr)
{
   Block * ret = mkblock(f, r, addr);
   blocks_.add(ret);
   return ret;
}

Block *
CFGFactory::_mkblock(CodeObject* co, CodeRegion *r, Address addr)
{
   Block* ret = new Block(co, r, addr);
   blocks_.add(ret);
   return ret;
}

Block *
CFGFactory::mkblock(Function *  f , CodeRegion *r, Address addr) {

    Block * ret = new Block(f->obj(),r,addr, f);
    return ret;
}


Block *
CFGFactory::_mksink(CodeObject * obj, CodeRegion *r) {
   Block * ret = mksink(obj,r);
   blocks_.add(ret);
   return ret;
}

Block *
CFGFactory::mksink(CodeObject * obj, CodeRegion *r) {
    Block * ret = new Block(obj,r,numeric_limits<Address>::max());
    return ret;
}

Edge *
CFGFactory::_mkedge(Block * src, Block * trg, EdgeTypeEnum type) {
    Edge * ret = mkedge(src,trg,type);
    edges_.add(ret);
    return ret;
}

Edge *
CFGFactory::mkedge(Block * src, Block * trg, EdgeTypeEnum type) {
    Edge * ret = new Edge(src,trg,type);
    return ret;
}

void CFGFactory::destroy_func(Function *f) {
   CFGFactory::free_func(f);
}

void
CFGFactory::free_func(Function *f) {
    delete f;
}

void
CFGFactory::destroy_block(Block *b) {
    CFGFactory::free_block(b);
}

void
CFGFactory::free_block(Block *b) {
    delete b;
}

std::string to_str(EdgeState e)
{
    switch(e)
    {
        case created: return "ok";
        case destroyed_noreturn: return "destroyed fallthrough from non-returning call/syscall";
        case destroyed_cb: return "destroyed from callback";
        case destroyed_all: return "destroyed during global cleanup";
        default: return "ERROR: unknown state";
    }
}

void
CFGFactory::destroy_edge(Edge *e, Dyninst::ParseAPI::EdgeState reason) {
    if(reason == destroyed_all) {
        CFGFactory::free_edge(e);
    }
}

void
CFGFactory::free_edge(Edge *e) {
   delete e;
}
