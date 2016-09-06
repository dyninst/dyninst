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

#include "dyntypes.h"

#include "CFG.h"
#include "InstructionSource.h"

namespace Dyninst {
namespace ParseAPI {

template <class T>
class flist_iter {
    typedef T elem;
 private:
    mutable allocatable * cur_;
 public:
    flist_iter(elem * cur) :
        cur_(cur) { }

    flist_iter(allocatable * cur) :
        cur_(cur) { }

    inline elem &operator*() { return *(elem*)cur_; }
    inline const elem &operator*() const { return *(elem*)cur_; }

    inline flist_iter operator++(int) const {
        flist_iter result = *this;
        cur_ = cur_->alloc_next();
        return result;
    }
    inline flist_iter operator++() const {
        cur_ = cur_->alloc_next();
        return *this;
    }

    inline bool operator==(const flist_iter &iter) const {
        return (cur_ == iter.cur_);
    }
    inline bool operator!=(const flist_iter &iter) const {
        return (cur_ != iter.cur_);
    }
};

template <class T>
class fact_list {
 public:
    typedef flist_iter<T> iterator;
    typedef const flist_iter<T> const_iterator;
    typedef T elem;

    fact_list() {
        head.alloc_set_next(&head);
        head.alloc_set_prev(&head);
    }

    ~fact_list() { }

    void add(elem & new_elem) {
        head.append(new_elem);
    }
    void add_tail(elem & new_elem) {
        head.prepend(new_elem);
    }
    void clear() {
        while(head.alloc_next() != &head)
            head.alloc_next()->remove();
    }
    
    // iterators
    iterator begin() { return iterator(head.alloc_next()); }
    iterator end() { return iterator(&head); }
    const_iterator begin() const { return iterator(head.alloc_next()); }
    const_iterator end() const { return iterator(&head); }
 private:
    allocatable head;
};
/** An implementation of CFGFactory is responsible for allocation and
    deallocation of CFG objects like Blocks, Edges, and Functions.
    Overriding the default methods of this interface allows the parsing
    routines to generate and work with extensions of the base types **/

/** Objects created by a CFGFactory must descend from `allocatable' **/

class PARSER_EXPORT CFGFactory {   
 public:
    CFGFactory() {};
    virtual ~CFGFactory();
    
    /*
     * These methods are called by ParseAPI, and perform bookkeeping
     * around the user-overridden creation/destruction methods.
     */
    Function *_mkfunc(Address addr, FuncSource src,
                                    std::string name, CodeObject *obj,
                                    CodeRegion *region, InstructionSource *isrc);
    Block *_mkblock(Function *f, CodeRegion *r, Address addr);
    Edge *_mkedge(Block *src, Block *trg, EdgeTypeEnum type);
    
    Block *_mksink(CodeObject *obj, CodeRegion *r);

    void destroy_func(Function *f);
    void destroy_block(Block *b);
    void destroy_edge(Edge *e);

    void destroy_all();

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

    fact_list<Edge> edges_;
    fact_list<Block> blocks_;
    fact_list<Function> funcs_;
};



    }
}

#endif
