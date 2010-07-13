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
#ifndef PARSER_CFG_H__
#define PARSER_CFG_H__

#include <vector>
#include <map>
#include <string>

#include "dyntypes.h"
#include "IBSTree.h"

#include "InstructionSource.h"
#include "ParseContainers.h"

#include "Annotatable.h"
#if !defined(_MSC_VER)
#include <stdint.h>
#endif

namespace Dyninst {
namespace ParseAPI {

enum EdgeTypeEnum {
    CALL = 0,
    COND_TAKEN,
    COND_NOT_TAKEN,
    INDIRECT,
    DIRECT,
    FALLTHROUGH,
    CATCH,
    CALL_FT,        // fallthrough after call instruction
    RET,
    NOEDGE,
    edgetype_end__
};

#define FLIST_BADNEXT ((void*)0x111)
#define FLIST_BADPREV ((void*)0x222)

/*
 * All CFG objects extend allocatable, which
 * allows them to be added and removed from
 * accounting structures in constant time
 */
class allocatable {
 public:
    allocatable() :
       anext_((allocatable*)FLIST_BADNEXT),
       aprev_((allocatable*)FLIST_BADPREV)
    { }
    allocatable * anext_;
    allocatable * aprev_;

    inline allocatable * alloc_next() const { return anext_; }
    inline allocatable * alloc_prev() const { return aprev_; }
    inline void alloc_set_next(allocatable *t) { anext_ = t; }
    inline void alloc_set_prev(allocatable *t) { aprev_ = t; }

    void remove() {
        if(anext_ != (allocatable*)FLIST_BADNEXT)
            anext_->aprev_ = aprev_;
        if(aprev_ != (allocatable*)FLIST_BADPREV)
            aprev_->anext_ = anext_;

        anext_ = (allocatable*)FLIST_BADNEXT;
        aprev_ = (allocatable*)FLIST_BADPREV;
    }

    void append(allocatable & new_elem) {
        if(anext_ == (allocatable*)FLIST_BADNEXT ||
           aprev_ == (allocatable*)FLIST_BADPREV)
        {
            fprintf(stderr,
                "allocatable::append called on element not on list: %p\n",this);
            return;
        }
        anext_->aprev_ = &new_elem;
        new_elem.anext_ = anext_;
        new_elem.aprev_ = this;
        anext_ = &new_elem;
    }

    void prepend(allocatable & new_elem) {
        if(anext_ == (allocatable*)FLIST_BADNEXT ||
           aprev_ == (allocatable*)FLIST_BADPREV)
        {
            fprintf(stderr,
               "allocatable::prepend called on element not on list: %p\n",this);
            return;
        }
        aprev_->anext_ = &new_elem;
        new_elem.aprev_ = aprev_;
        new_elem.anext_ = this;
        aprev_ = &new_elem;
    }
};

class Block;
class Edge : public allocatable {
 protected:
    Block * source_;
    Block * target_;

 private:

#if defined(_MSC_VER)
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int8 uint8_t;
#endif

    struct EdgeType {
        EdgeType(EdgeTypeEnum t, bool s) :
            type_enum_(t), sink_(s), interproc_(false)
        { }
        uint16_t type_enum_;
        uint8_t sink_;
        uint8_t  interproc_;    // modifier for interprocedural branches
                                // (tail calls)
    };
    EdgeType type_;

 public:
    PARSER_EXPORT Edge(Block * source,
         Block * target,
         EdgeTypeEnum type) :
        source_(source),
        target_(target),
        type_(type,false) { }

    PARSER_EXPORT virtual ~Edge() { }

    PARSER_EXPORT virtual Block * src() const { return source_; }
    PARSER_EXPORT virtual Block * trg() const { return target_; }
    PARSER_EXPORT EdgeTypeEnum type() const { 
        return static_cast<EdgeTypeEnum>(type_.type_enum_); 
    }
    bool sinkEdge() const { return type_.sink_ != 0; }
    bool interproc() const { return type_.interproc_ != 0; }

 friend class CFGFactory;
 friend class Parser;
};

/* 
 * Iteration over edges can be controlled by an EdgePredicate.
 * Edges are returned only if pred(edge) evaluates true.
 * 
 * EdgePredicates are composable by AND.
 */
class EdgePredicate 
    : public iterator_predicate<
        EdgePredicate,
        Edge *,
        Edge *
      >
{
 protected:
    EdgePredicate * next_; 
 public:
    PARSER_EXPORT EdgePredicate() : next_(NULL) { }
    PARSER_EXPORT EdgePredicate(EdgePredicate * next) : next_(next) { }
    PARSER_EXPORT virtual ~EdgePredicate() { }
    PARSER_EXPORT virtual bool pred_impl(Edge *) const;
};

class Intraproc : public EdgePredicate {
 public:
    PARSER_EXPORT Intraproc() { }
    PARSER_EXPORT Intraproc(EdgePredicate * next) : EdgePredicate(next) { }
    PARSER_EXPORT ~Intraproc() { }
    PARSER_EXPORT bool pred_impl(Edge *) const;
};

/*
 * For proper ostritch-like denial of 
 * unresolved control flow edges
 */
class NoSinkPredicate : public ParseAPI::EdgePredicate {
 public:
    NoSinkPredicate() { }
    NoSinkPredicate(EdgePredicate * next)
        : EdgePredicate(next) 
    { } 

    bool pred_impl(ParseAPI::Edge * e) const {
        return !e->sinkEdge() && EdgePredicate::pred_impl(e);
    }
};

class Function;
class SingleContext : public EdgePredicate {
 private:
    Function * context_;
    bool forward_;
    bool backward_;
 public:
    PARSER_EXPORT SingleContext(Function * f, bool forward, bool backward) : 
        context_(f),
        forward_(forward),
        backward_(backward) { }
    PARSER_EXPORT ~SingleContext() { }
    PARSER_EXPORT bool pred_impl(Edge *) const;
};

class CodeObject;
class CodeRegion;
class Block : public Dyninst::interval<Address>, 
              public allocatable {
 public:
    typedef ContainerWrapper<
        vector<Edge*>,
        Edge*,
        Edge*,
        EdgePredicate
    > edgelist;

    PARSER_EXPORT Block(CodeObject * o, CodeRegion * r, Address start);
    PARSER_EXPORT virtual ~Block();

    PARSER_EXPORT Address start() const { return start_; }
    PARSER_EXPORT Address end() const { return end_; }
    PARSER_EXPORT Address lastInsnAddr() const { return lastInsn_; } 
    PARSER_EXPORT Address size() const { return end_ - start_; }

    PARSER_EXPORT bool parsed() const { return parsed_; }

    PARSER_EXPORT CodeObject * obj() const { return obj_; }
    PARSER_EXPORT CodeRegion * region() const { return region_; }

    /* Edge access */
    PARSER_EXPORT edgelist & sources() { return srclist_; }
    PARSER_EXPORT edgelist & targets() { return trglist_; }

    PARSER_EXPORT bool consistent(Address addr, Address & prev_insn) const;

    PARSER_EXPORT int  containingFuncs() const;
    PARSER_EXPORT void getFuncs(vector<Function *> & funcs);

    /* interval implementation */
    Address low() const { return start(); }
    Address high() const { return end(); }

    struct compare {
        bool operator()(Block * const & b1, Block * const & b2) const {
            if(b1->start() < b2->start()) return true;
            if(b1->start() > b2->start()) return false;
            
            // XXX debugging
            if(b1 != b2)
                fprintf(stderr,"FATAL: blocks %p [%lx,%lx) and %p [%lx,%lx) "
                            "conflict",b1,b1->start(),b1->end(),
                            b2,b2->start(),b2->end());

            assert(b1 == b2);
            return false;
        }
    };

 private:
    void addSource(Edge * e);
    void addTarget(Edge * e);
    void removeTarget(Edge * e);
    void removeSource(Edge * e);

 private:
    CodeObject * obj_;
    CodeRegion * region_;

    Address start_;
    Address end_;
    Address lastInsn_;

    vector<Edge *> sources_;
    vector<Edge *> targets_;

    edgelist srclist_;
    edgelist trglist_;
    int func_cnt_;
    bool parsed_;

 friend class Function;
 friend class Parser;
 friend class CFGFactory;
};

inline void Block::addSource(Edge * e) 
{
    sources_.push_back(e);
}

inline void Block::addTarget(Edge * e)
{
    targets_.push_back(e);
}

inline void Block::removeTarget(Edge * e)
{
    for(unsigned i=0;i<targets_.size();++i) {
        if(targets_[i] == e) {
            targets_[i] = targets_.back();
            targets_.pop_back();    
            break;
        }
    }
}

inline void Block::removeSource(Edge * e) {
    for(unsigned i=0;i<sources_.size();++i) {
        if(sources_[i] == e) {
            sources_[i] = sources_.back();
            sources_.pop_back();    
            break;
        }
    }
}

enum FuncReturnStatus {
    UNSET,
    NORETURN,
    UNKNOWN,
    RETURN
};

/* Discovery method of functions */
enum FuncSource {
    RT = 0,     // recursive traversal (default)
    HINT,       // specified in code object hints
    GAP,        // gap heuristics
    GAPRT,      // RT from gap-discovered function
    ONDEMAND,   // dynamically discovered
    funcsource_end__
};

class CodeObject;
class CodeRegion;
class FuncExtent;
class Function : public allocatable, public AnnotatableSparse {
 protected:
    Address start_;
    CodeObject * obj_;
    CodeRegion * region_;
    InstructionSource * isrc_;
    
    FuncSource src_;
    FuncReturnStatus rs_;

    std::string name_;
    Block * entry_;
 protected:
    PARSER_EXPORT Function(); 
 public:
    typedef ContainerWrapper<
        vector<Block*>,
        Block*,
        Block*
    > blocklist;
    typedef ContainerWrapper<
        vector<Edge*>,
        Edge*,
        Edge*
    > edgelist;

    PARSER_EXPORT Function(Address addr, string name, CodeObject * obj, 
        CodeRegion * region, InstructionSource * isource);

    PARSER_EXPORT virtual ~Function();
    PARSER_EXPORT virtual const string & name();

    PARSER_EXPORT Address addr() const { return start_; }
    PARSER_EXPORT CodeRegion * region() const { return region_; }
    PARSER_EXPORT InstructionSource * isrc() const { return isrc_; }
    PARSER_EXPORT CodeObject * obj() const { return obj_; }
    PARSER_EXPORT FuncSource src() const { return src_; }
    PARSER_EXPORT FuncReturnStatus retstatus() const { return rs_; }
    PARSER_EXPORT Block * entry() const { return entry_; }
    PARSER_EXPORT bool parsed() const { return parsed_; }

    /* Basic block and CFG access */
    PARSER_EXPORT blocklist & blocks();
    PARSER_EXPORT bool contains(Block *b);
    PARSER_EXPORT edgelist & callEdges();
    PARSER_EXPORT blocklist & returnBlocks();

    /* Function details */
    PARSER_EXPORT bool hasNoStackFrame() const { return no_stack_frame_; }
    PARSER_EXPORT bool savesFramePointer() const { return saves_fp_; }
    PARSER_EXPORT bool cleansOwnStack() const { return cleans_stack_; }

    struct less
    {
        bool operator()(const Function * f1, const Function * f2) const
        {
            return f1->addr() < f2->addr();
        }
    };

    /* Contiguous code segments of function */
    PARSER_EXPORT std::vector<FuncExtent *> const& extents();

 private:
    std::vector<Block *> const& blocks_int();
    void delayed_link_return(CodeObject * co, Block * retblk);
    void finalize();

    bool parsed_;
    bool cache_valid_;
    blocklist bl_;
    std::vector<Block *> blocks_;
    std::vector<FuncExtent *> extents_;

    /* rapid lookup for edge predicate tests */
    //typedef dyn_hash_map<Address, Block*> blockmap;
    typedef std::map<Address, Block*> blockmap;
    blockmap bmap_;

    /* rapid lookup for interprocedural queries */
    std::vector<Edge *> call_edges_;
    edgelist call_edge_list_;
    std::vector<Block *> return_blocks_;
    blocklist retBL_;


    /* function details */
    bool no_stack_frame_;
    bool saves_fp_;
    bool cleans_stack_;

    /*** Internal parsing methods and state ***/
    void add_block(Block *b);
    std::vector<Block *> * dangling_;

    friend class Parser;
    friend class CFGFactory;
    friend class CodeObject;
};

/* Describes a contiguous extent of a Function object */
class FuncExtent : public Dyninst::interval<Address> {
 private:
    Function * func_;
    Address start_;
    Address end_;

 public:
    FuncExtent(Function * f, Address start, Address end) :
        func_(f),
        start_(start),
        end_(end) { }

    ~FuncExtent() {
        func_ = NULL;
    }

    PARSER_EXPORT Function * func() { return func_; }

    PARSER_EXPORT Address start() const { return start_; }
    PARSER_EXPORT Address end() const { return end_; }

    /* interval implementation */
    PARSER_EXPORT Address low() const { return start_; }
    PARSER_EXPORT Address high() const { return end_; } 
};


} //namespace ParseAPI
} //namespace Dyninst

#endif
