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
#ifndef _CFG_FACTORY_H_
#define _CFG_FACTORY_H_

#include <string>

#include "dyntypes.h"

#include "LockFreeQueue.h"
#include "CFG.h"
#include "InstructionSource.h"

namespace Dyninst {
namespace ParseAPI {

template <class T>
class fact_list {
public:
  typedef typename LockFreeQueue<T>::iterator iterator;
  typedef std::forward_iterator_tag iterator_category;
  typedef T elem;
  typedef T &reference;

  fact_list() {
  }

  ~fact_list() { }

  void add(elem new_elem) {
    queue.insert(new_elem);
  }
    
  // iterators
  iterator begin() { return queue.begin(); }
  iterator end() { return queue.end(); }
private:
  LockFreeQueue<T> queue;
};


/** An implementation of CFGFactory is responsible for allocation and
    deallocation of CFG objects like Blocks, Edges, and Functions.
    Overriding the default methods of this interface allows the parsing
    routines to generate and work with extensions of the base types **/

enum EdgeState {
    created,
    destroyed_cb,
    destroyed_noreturn,
    destroyed_all
};

class PARSER_EXPORT CFGFactory  {
 public:
    CFGFactory() {}
    virtual ~CFGFactory();
    
    /*
     * These methods are called by ParseAPI, and perform bookkeeping
     * around the user-overridden creation/destruction methods.
     */
    Function *_mkfunc(Address addr, FuncSource src,
                                    std::string name, CodeObject *obj,
                                    CodeRegion *region, InstructionSource *isrc);
    Block *_mkblock(Function *f, CodeRegion *r, Address addr);
    Block *_mkblock(CodeObject *co, CodeRegion *r, Address addr);
    Edge *_mkedge(Block *src, Block *trg, EdgeTypeEnum type);
    
    Block *_mksink(CodeObject *obj, CodeRegion *r);

    void destroy_func(Function *f);
    void destroy_block(Block *b);
    void destroy_edge(Edge *e, EdgeState reason);

 protected:
    virtual Function * mkfunc(Address addr, FuncSource src, 
            std::string name, CodeObject * obj, CodeRegion * region, 
            Dyninst::InstructionSource * isrc);
    virtual Block * mkblock(Function * f, CodeRegion * r, 
            Address addr);
    virtual Edge * mkedge(Block * src, Block * trg, 
            EdgeTypeEnum type);
    /*
     * A `sink' block is the target of all unresolvable control
     * flow in a parsing unit. Implementors may return a unique
     * sink per CodeObject or a single global sink.
     */
    virtual Block * mksink(CodeObject *obj, CodeRegion *r);

    virtual void free_func(Function * f);
    virtual void free_block(Block * b);
    virtual void free_edge(Edge * e);

    fact_list<Edge *> edges_;
    fact_list<Block *> blocks_;
    fact_list<Function *> funcs_;
};



    }
}

#endif
