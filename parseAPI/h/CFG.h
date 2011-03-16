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
#ifndef _PARSER_CFG_H_
#define _PARSER_CFG_H_

#include <vector>
#include <set>
#include <map>
#include <string>

#include "dyntypes.h"
#include "IBSTree.h"

#include "InstructionSource.h"
#include "ParseContainers.h"

#include "Annotatable.h"

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
    _edgetype_end_
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
    Block * _source;
    Block * _target;

 private:

#if defined(_MSC_VER)
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int8 uint8_t;
#else
	typedef unsigned short uint16_t;
	typedef unsigned char uint8_t;
#endif

    struct EdgeType {
        EdgeType(EdgeTypeEnum t, bool s) :
            _type_enum(t), _sink(s), _interproc(false)
        { }
        uint16_t _type_enum;
        uint8_t _sink;
        uint8_t  _interproc;    // modifier for interprocedural branches
                                // (tail calls)
    };
    EdgeType _type;

 public:
    PARSER_EXPORT Edge(Block * source,
         Block * target,
         EdgeTypeEnum type) :
        _source(source),
        _target(target),
        _type(type,false) { }

    PARSER_EXPORT virtual ~Edge() { }

    PARSER_EXPORT virtual Block * src() const { return _source; }
    PARSER_EXPORT virtual Block * trg() const { return _target; }
    PARSER_EXPORT EdgeTypeEnum type() const { 
        return static_cast<EdgeTypeEnum>(_type._type_enum); 
    }
    bool sinkEdge() const { return _type._sink != 0; }
    bool interproc() const { return _type._interproc != 0; }

    PARSER_EXPORT void install();

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
    EdgePredicate * _next; 
 public:
    PARSER_EXPORT EdgePredicate() : _next(NULL) { }
    PARSER_EXPORT EdgePredicate(EdgePredicate * next) : _next(next) { }
    PARSER_EXPORT virtual ~EdgePredicate() { }
    PARSER_EXPORT virtual bool pred_impl(Edge *) const;
};

/* may follow branches into the function if there is shared code */
class Intraproc : public EdgePredicate {
 public:
    PARSER_EXPORT Intraproc() { }
    PARSER_EXPORT Intraproc(EdgePredicate * next) : EdgePredicate(next) { }
    PARSER_EXPORT ~Intraproc() { }
    PARSER_EXPORT bool pred_impl(Edge *) const;
};

/* follow interprocedural edges */
class Interproc : public EdgePredicate {
    public:
        PARSER_EXPORT Interproc() {}
        PARSER_EXPORT Interproc(EdgePredicate * next) : EdgePredicate(next) { }
        PARSER_EXPORT ~Interproc() { }
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

/* doesn't follow branches into the function if there is shared code */
class Function;
class SingleContext : public EdgePredicate {
 private:
    Function * _context;
    bool _forward;
    bool _backward;
 public:
    PARSER_EXPORT SingleContext(Function * f, bool forward, bool backward) : 
        _context(f),
        _forward(forward),
        _backward(backward) { }
    PARSER_EXPORT ~SingleContext() { }
    PARSER_EXPORT bool pred_impl(Edge *) const;
};

/* Doesn't follow branches into the function if there is shared code. 
 * Will follow interprocedural call/return edges */
class SingleContextOrInterproc : public EdgePredicate {
    private:
        Function * _context;
        bool _forward;
        bool _backward;
    public:
        PARSER_EXPORT SingleContextOrInterproc(Function * f, bool forward, bool backward) :
            _context(f),
            _forward(forward),
            _backward(backward) { }
        PARSER_EXPORT ~SingleContextOrInterproc() { }
        PARSER_EXPORT bool pred_impl(Edge *) const;
};

class CodeObject;
class CodeRegion;
class Block : public Dyninst::interval<Address>, 
              public allocatable {
 public:
    typedef ContainerWrapper<
        std::vector<Edge*>,
        Edge*,
        Edge*,
        EdgePredicate
    > edgelist;

    PARSER_EXPORT Block(CodeObject * o, CodeRegion * r, Address start);
    PARSER_EXPORT virtual ~Block();

    PARSER_EXPORT Address start() const { return _start; }
    PARSER_EXPORT Address end() const { return _end; }
    PARSER_EXPORT Address lastInsnAddr() const { return _lastInsn; } 
    PARSER_EXPORT Address size() const { return _end - _start; }

    PARSER_EXPORT bool parsed() const { return _parsed; }

    PARSER_EXPORT CodeObject * obj() const { return _obj; }
    PARSER_EXPORT CodeRegion * region() const { return _region; }

    /* Edge access */
    PARSER_EXPORT edgelist & sources() { return _srclist; }
    PARSER_EXPORT edgelist & targets() { return _trglist; }

    PARSER_EXPORT bool consistent(Address addr, Address & prev_insn);

    PARSER_EXPORT int  containingFuncs() const;
    PARSER_EXPORT void getFuncs(std::vector<Function *> & funcs);

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
    void removeFunc(Function *);

 private:
    CodeObject * _obj;
    CodeRegion * _region;

    Address _start;
    Address _end;
    Address _lastInsn;

    std::vector<Edge *> _sources;
    std::vector<Edge *> _targets;

    edgelist _srclist;
    edgelist _trglist;
    int _func_cnt;
    bool _parsed;

 friend class Edge;
 friend class Function;
 friend class Parser;
 friend class CFGFactory;
};

inline void Block::addSource(Edge * e) 
{
    _sources.push_back(e);
}

inline void Block::addTarget(Edge * e)
{
    _targets.push_back(e);
}

inline void Block::removeTarget(Edge * e)
{
    for(unsigned i=0;i<_targets.size();++i) {
        if(_targets[i] == e) {
            _targets[i] = _targets.back();
            _targets.pop_back();    
            break;
        }
    }
}

inline void Block::removeSource(Edge * e) {
    for(unsigned i=0;i<_sources.size();++i) {
        if(_sources[i] == e) {
            _sources[i] = _sources.back();
            _sources.pop_back();    
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
    _funcsource_end_
};

enum StackTamper {
    TAMPER_UNSET,
    TAMPER_NONE,
    TAMPER_REL,
    TAMPER_ABS,
    TAMPER_NONZERO
};

class CodeObject;
class CodeRegion;
class FuncExtent;
class Function : public allocatable, public AnnotatableSparse {
 protected:
    Address _start;
    CodeObject * _obj;
    CodeRegion * _region;
    InstructionSource * _isrc;
    
    FuncSource _src;
    FuncReturnStatus _rs;

    std::string _name;
    Block * _entry;
 protected:
    PARSER_EXPORT Function(); 
 public:
    bool _is_leaf_function;
    Address _ret_addr; // return address of a function stored in stack at function entry
    typedef ContainerWrapper<
        std::vector<Block*>,
        Block*,
        Block*
    > blocklist;
    typedef ContainerWrapper<
        std::set<Edge*>,
        Edge*,
        Edge*
    > edgelist;

    PARSER_EXPORT Function(Address addr, string name, CodeObject * obj, 
        CodeRegion * region, InstructionSource * isource);

    PARSER_EXPORT virtual ~Function();
    PARSER_EXPORT virtual const string & name();

    PARSER_EXPORT Address addr() const { return _start; }
    PARSER_EXPORT CodeRegion * region() const { return _region; }
    PARSER_EXPORT InstructionSource * isrc() const { return _isrc; }
    PARSER_EXPORT CodeObject * obj() const { return _obj; }
    PARSER_EXPORT FuncSource src() const { return _src; }
    PARSER_EXPORT FuncReturnStatus retstatus() const { return _rs; }
    PARSER_EXPORT Block * entry() const { return _entry; }
    PARSER_EXPORT bool parsed() const { return _parsed; }

    /* Basic block and CFG access */
    PARSER_EXPORT blocklist & blocks();
    PARSER_EXPORT bool contains(Block *b);
    PARSER_EXPORT edgelist & callEdges();
    PARSER_EXPORT blocklist & returnBlocks();

    /* Function details */
    PARSER_EXPORT bool hasNoStackFrame() const { return _no_stack_frame; }
    PARSER_EXPORT bool savesFramePointer() const { return _saves_fp; }
    PARSER_EXPORT bool cleansOwnStack() const { return _cleans_stack; }

    /* Parse updates and obfuscation */
    PARSER_EXPORT void set_retstatus(FuncReturnStatus rs) { _rs = rs; }
    PARSER_EXPORT void deleteBlocks( vector<Block*> & dead_funcs,
                                     Block * new_entry );
    PARSER_EXPORT StackTamper stackTamper() { return _tamper; }

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

    bool _parsed;
    bool _cache_valid;
    blocklist _bl;
    std::vector<Block *> _blocks;
    std::vector<FuncExtent *> _extents;

    /* rapid lookup for edge predicate tests */
    //typedef dyn_hash_map<Address, Block*> blockmap;
    typedef std::map<Address, Block*> blockmap;
    blockmap _bmap;

    /* rapid lookup for interprocedural queries */
    std::set<Edge *> _call_edges;
    edgelist _call_edge_list;
    std::vector<Block *> _return_blocks;
    blocklist _retBL;


    /* function details */
    bool _no_stack_frame;
    bool _saves_fp;
    bool _cleans_stack;
    StackTamper _tamper;
    Address _tamper_addr;

    /*** Internal parsing methods and state ***/
    void add_block(Block *b);

    friend class Parser;
    friend class CFGFactory;
    friend class CodeObject;
};

/* Describes a contiguous extent of a Function object */
class FuncExtent : public Dyninst::interval<Address> {
 private:
    Function * _func;
    Address _start;
    Address _end;

 public:
    FuncExtent(Function * f, Address start, Address end) :
        _func(f),
        _start(start),
        _end(end) { }

    ~FuncExtent() {
        _func = NULL;
    }

    PARSER_EXPORT Function * func() { return _func; }

    PARSER_EXPORT Address start() const { return _start; }
    PARSER_EXPORT Address end() const { return _end; }

    /* interval implementation */
    PARSER_EXPORT Address low() const { return _start; }
    PARSER_EXPORT Address high() const { return _end; } 
};


} //namespace ParseAPI
} //namespace Dyninst

#endif
