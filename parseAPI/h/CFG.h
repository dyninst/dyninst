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
#ifndef _PARSER_CFG_H_
#define _PARSER_CFG_H_

#include <vector>
#include <set>
#include <map>
#include <string>
#include <functional>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/range.hpp>
#include "dyntypes.h"
#include "IBSTree.h"

#include "InstructionSource.h"
#include "ParseContainers.h"
#include "Annotatable.h"
#include <iostream>

namespace Dyninst {

   namespace InstructionAPI {
      class Instruction;
      typedef boost::shared_ptr<Instruction> InstructionPtr;
   }

namespace ParseAPI {

class LoopAnalyzer;
class dominatorCFG;
class CodeObject;
class CFGModifier;

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

PARSER_EXPORT std::string format(EdgeTypeEnum e);

#define FLIST_BADNEXT ((void*)0x111)
#define FLIST_BADPREV ((void*)0x222)

/*
 * All CFG objects extend allocatable, which
 * allows them to be added and removed from
 * accounting structures in constant time
 */
class PARSER_EXPORT allocatable {
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


class PARSER_EXPORT Edge : public allocatable {
   friend class CFGModifier;
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
    Edge(Block * source,
         Block * target,
         EdgeTypeEnum type);
     virtual ~Edge();

    Block * src() const { return _source; }
    Block * trg() const { return _target; }
    EdgeTypeEnum type() const { 
        return static_cast<EdgeTypeEnum>(_type._type_enum); 
    }
    bool sinkEdge() const { return _type._sink != 0; }
    bool interproc() const { 
       return (_type._interproc != 0 ||
               type() == CALL ||
               type() == RET);
    }

    bool intraproc() const {
       return !interproc();
    }

    void install();

    /* removes from blocks (and if of type CALL, from finalized source functions ) */
    void uninstall();

    static void destroy(Edge *, CodeObject *);

 friend class CFGFactory;
 friend class Parser;
};

/* 
 * Iteration over edges can be controlled by an EdgePredicate.
 * Edges are returned only if pred(edge) evaluates true.
 * 
 * EdgePredicates are composable by AND.
 */
class PARSER_EXPORT EdgePredicate 
{
 public:
  EdgePredicate() { }
    virtual ~EdgePredicate() { }
    virtual bool pred_impl(Edge *) const;
    bool operator()(Edge* e) const 
    {
      return pred_impl(e);
    }
 };

/* may follow branches into the function if there is shared code */
class PARSER_EXPORT Intraproc : public EdgePredicate {
 public:
    Intraproc() { }
    ~Intraproc() { }
    bool pred_impl(Edge *) const;

};

/* follow interprocedural edges */
 class PARSER_EXPORT Interproc : public EdgePredicate {
    public:
        Interproc() {}
        ~Interproc() { }
        bool pred_impl(Edge *) const;
};

/*
 * For proper ostritch-like denial of 
 * unresolved control flow edges
 */
 class PARSER_EXPORT NoSinkPredicate : public ParseAPI::EdgePredicate {
 public:
    NoSinkPredicate() { }

    bool pred_impl(ParseAPI::Edge * e) const {
        return !e->sinkEdge() && EdgePredicate::pred_impl(e);
    }
};

/* doesn't follow branches into the function if there is shared code */
class Function;
 class PARSER_EXPORT SingleContext : public EdgePredicate {
 private:
    Function * _context;
    bool _forward;
    bool _backward;
 public:
    SingleContext(Function * f, bool forward, bool backward) : 
        _context(f),
        _forward(forward),
        _backward(backward) { }
    ~SingleContext() { }
    bool pred_impl(Edge *) const;
};

/* Doesn't follow branches into the function if there is shared code. 
 * Will follow interprocedural call/return edges */
 class PARSER_EXPORT SingleContextOrInterproc : public EdgePredicate {
    private:
        Function * _context;
        bool _forward;
        bool _backward;
    public:
        SingleContextOrInterproc(Function * f, bool forward, bool backward) :
            _context(f),
            _forward(forward),
            _backward(backward) { }
        ~SingleContextOrInterproc() { }
        bool pred_impl(Edge *) const;
	bool operator()(Edge* e) const 
	{
	  return pred_impl(e);
	}
};

class CodeRegion;
class PARSER_EXPORT Block : public Dyninst::SimpleInterval<Address, int>,
              public allocatable {
    friend class CFGModifier;
 public:
    typedef std::map<Offset, InstructionAPI::InstructionPtr> Insns;
    typedef std::vector<Edge*> edgelist;

    Block(CodeObject * o, CodeRegion * r, Address start);
    virtual ~Block();

    Address start() const { return _start; }
    Address end() const { return _end; }
    Address lastInsnAddr() const { return _lastInsn; } 
    Address last() const { return lastInsnAddr(); }
    Address size() const { return _end - _start; }
    bool containsAddr(Address addr) const { return addr >= _start && addr < _end; }

    bool parsed() const { return _parsed; }

    CodeObject * obj() const { return _obj; }
    CodeRegion * region() const { return _region; }

    /* Edge access */
    const edgelist & sources() const { return _srclist; }
    const edgelist & targets() const { return _trglist; }

    bool consistent(Address addr, Address & prev_insn);

    int  containingFuncs() const;
    void getFuncs(std::vector<Function *> & funcs);
    template<class OutputIterator> void getFuncs(OutputIterator result); 

    void getInsns(Insns &insns) const;
    InstructionAPI::InstructionPtr getInsn(Offset o) const;

    bool wasUserAdded() const;

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

    static void destroy(Block *b);

    bool operator==(const Block &rhs) const;

    bool operator!=(const Block &rhs) const;

private:
    void addSource(Edge * e);
    void addTarget(Edge * e);
    void removeTarget(Edge * e);
    void removeSource(Edge * e);
    void removeFunc(Function *);
    void updateEnd(Address addr);

 private:
    CodeObject * _obj;
    CodeRegion * _region;

    Address _start;
    Address _end;
    Address _lastInsn;


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
    _srclist.push_back(e);
}

inline void Block::addTarget(Edge * e)
{
    _trglist.push_back(e);
}

inline void Block::removeTarget(Edge * e)
{
    for(unsigned i=0;i<_trglist.size();++i) {
        if(_trglist[i] == e) {
            _trglist[i] = _trglist.back();
            _trglist.pop_back();    
            break;
        }
    }
}

inline void Block::removeSource(Edge * e) {
    for(unsigned i=0;i<_srclist.size();++i) {
        if(_srclist[i] == e) {
            _srclist[i] = _srclist.back();
            _srclist.pop_back();    
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
    MODIFICATION, // Added via user modification
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
class Loop;
class LoopTreeNode;

class PARSER_EXPORT Function : public allocatable, public AnnotatableSparse {
   friend class CFGModifier;
   friend class LoopAnalyzer;
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
    Function(); 
 public:
    bool _is_leaf_function;
    Address _ret_addr; // return address of a function stored in stack at function entry
    typedef std::map<Address, Block*> blockmap;
    template <typename P>
    struct select2nd
    {
      typedef typename P::second_type result_type;
      
      result_type operator()(const P& p) const
      {
	return p.second;
      }
    };
    typedef select2nd<blockmap::value_type> selector;

    
    typedef boost::transform_iterator<selector, blockmap::iterator> bmap_iterator;
    typedef boost::transform_iterator<selector, blockmap::const_iterator> bmap_const_iterator;
    typedef boost::iterator_range<bmap_iterator> blocklist;
    typedef boost::iterator_range<bmap_const_iterator> const_blocklist;
    typedef std::set<Edge*> edgelist;
    
    Function(Address addr, std::string name, CodeObject * obj, 
        CodeRegion * region, InstructionSource * isource);

    virtual ~Function();

    virtual const std::string & name() const;

    Address addr() const { return _start; }
    CodeRegion * region() const { return _region; }
    InstructionSource * isrc() const { return _isrc; }
    CodeObject * obj() const { return _obj; }
    FuncSource src() const { return _src; }
    FuncReturnStatus retstatus() const { return _rs; }
    Block * entry() const { return _entry; }
    bool parsed() const { return _parsed; }

    /* Basic block and CFG access */
    blocklist blocks();
    const_blocklist blocks() const;
    size_t num_blocks()
    {
      if(!_cache_valid) finalize();
      return _bmap.size();
    }
    
    bool contains(Block *b);
    const edgelist & callEdges();
    const_blocklist returnBlocks() ;
    const_blocklist exitBlocks();
    const_blocklist exitBlocks() const;

    /* Function details */
    bool hasNoStackFrame() const { return _no_stack_frame; }
    bool savesFramePointer() const { return _saves_fp; }
    bool cleansOwnStack() const { return _cleans_stack; }

    /* Loops */    
    LoopTreeNode* getLoopTree() const;
    Loop* findLoop(const char *name) const;
    bool getLoops(std::vector<Loop*> &loops) const;
    bool getOuterLoops(std::vector<Loop*> &loops) const;

    /* Dominator info */

    /* Return true if A dominates B in this function */
    bool dominates(Block* A, Block *B) const;
    Block* getImmediateDominator(Block *A) const;
    void getImmediateDominates(Block *A, std::set<Block*> &) const;
    void getAllDominates(Block *A, std::set<Block*> &) const;

    /* Post-dominator info */

    /* Return true if A post-dominates B in this function */
    bool postDominates(Block* A, Block *B) const;
    Block* getImmediatePostDominator(Block *A) const;
    void getImmediatePostDominates(Block *A, std::set<Block*> &) const;
    void getAllPostDominates(Block *A, std::set<Block*> &) const;


    /* Parse updates and obfuscation */
    void setEntryBlock(Block *new_entry);
    void set_retstatus(FuncReturnStatus rs);
    void removeBlock( Block* );

    StackTamper tampersStack(bool recalculate=false);

    struct less
    {
        bool operator()(const Function * f1, const Function * f2) const
        {
            return f1->addr() < f2->addr();
        }
    };

    /* Contiguous code segments of function */
    std::vector<FuncExtent *> const& extents();

    /* This should not remain here - this is an experimental fix for
       defensive mode CFG inconsistency */
    void invalidateCache() { _cache_valid = false; }

    static void destroy(Function *f);

 private:
    void delayed_link_return(CodeObject * co, Block * retblk);
    void finalize();

    bool _parsed;
    bool _cache_valid;
    //    blocklist _bl;
    std::vector<FuncExtent *> _extents;

    /* rapid lookup for edge predicate tests */
    blocklist blocks_int();
    
    blockmap _bmap;
    bmap_iterator blocks_begin() {
      return bmap_iterator(_bmap.begin());
    }
    bmap_iterator blocks_end() {
      return bmap_iterator(_bmap.end());
    }
    bmap_const_iterator blocks_begin() const 
    {
      return bmap_const_iterator(_bmap.begin());
    }
    
    bmap_const_iterator blocks_end() const 
    {
      return bmap_const_iterator(_bmap.end());
    }
    
    

    /* rapid lookup for interprocedural queries */
    edgelist _call_edge_list;
    blockmap _retBL;
    bmap_const_iterator ret_begin() const 
    {
      return bmap_const_iterator(_retBL.begin());
    }
    bmap_const_iterator ret_end() const 
    {
      return bmap_const_iterator(_retBL.end());
    }
    // Superset of return blocks; this includes all blocks where
    // execution leaves the function without coming back, including
    // returns, calls to non-returning calls, tail calls, etc.
    // Might want to include exceptions...
    blockmap _exitBL;
    bmap_const_iterator exit_begin() const 
    {
      return bmap_const_iterator(_exitBL.begin());
    }
    bmap_const_iterator exit_end() const 
    {
      return bmap_const_iterator(_exitBL.end());
    }

    /* function details */
    bool _no_stack_frame;
    bool _saves_fp;
    bool _cleans_stack;
    StackTamper _tamper;
    Address _tamper_addr;

    /* Loop details*/
    mutable bool _loop_analyzed; // true if loops in the function have been found and stored in _loops
    mutable std::set<Loop*> _loops;
    mutable LoopTreeNode *_loop_root; // NULL if the tree structure has not be calculated
    void getLoopsByNestingLevel(std::vector<Loop*>& lbb, bool outerMostOnly) const;


    /* Dominator and post-dominator info details */
    mutable bool isDominatorInfoReady;
    mutable bool isPostDominatorInfoReady;
    void fillDominatorInfo() const;
    void fillPostDominatorInfo() const;
    /** set of basic blocks that this basicblock dominates immediately*/
    mutable std::map<Block*, std::set<Block*>*> immediateDominates;
    /** basic block which is the immediate dominator of the basic block */
    mutable std::map<Block*, Block*> immediateDominator;
    /** same as previous two fields, but for postdominator tree */
    mutable std::map<Block*, std::set<Block*>*> immediatePostDominates;
    mutable std::map<Block*, Block*> immediatePostDominator;

    /*** Internal parsing methods and state ***/
    void add_block(Block *b);

    friend void Edge::uninstall();
    friend class Parser;
    friend class CFGFactory;
    friend class CodeObject;
    friend class dominatorCFG;
};

/* Describes a contiguous extent of a Function object */
class PARSER_EXPORT FuncExtent : public Dyninst::SimpleInterval<Address, Function* > {
 private:
    Function * _func;
    Address _start;
    Address _end;

 public:
    FuncExtent(Function * f, Address start, Address end) :
        _func(f),
        _start(start),
        _end(end) { }

    virtual ~FuncExtent() {
        _func = NULL;
    }

    Function * func() { return _func; }

    Address start() const { return _start; }
    Address end() const { return _end; }

    /* interval implementation */
    Address low() const { return _start; }
    Address high() const { return _end; }
    Function* id() const { return _func; }

    bool operator==(const FuncExtent &rhs) const {
        return _func == rhs._func &&
               _start == rhs._start &&
               _end == rhs._end;
    }

    bool operator!=(const FuncExtent &rhs) const {
        return !(rhs == *this);
    }
};

/** Natural loops
  */

class PARSER_EXPORT Loop  
{
	friend class LoopAnalyzer;
	friend std::ostream& operator<<(std::ostream&,Loop&);

private:
        std::set<Edge*> backEdges;
	std::set<Block*> entries;

        // the function this loop is part of
        const Function * func;

	/** set of loops that are contained (nested) in this loop. */
        std::set<Loop*> containedLoops;

	/** the basic blocks in the loop */

        std::set<Block*> childBlocks;
        std::set<Block*> exclusiveBlocks;
    Loop* parent;
public:
	/** If loop which directly encloses this loop. NULL if no such loop */
    void insertBlock(Block* b);
    void insertChildBlock(Block* b);

    Loop* parentLoop() { return parent; }
	/** Return true if the given address is within the range of
	    this loop's basicBlocks */

        bool containsAddress(Address addr);
	  
	/** Return true if the given address is within the range of
	    this loop's basicBlocks or its children */
		   
	bool containsAddressInclusive(Address addr);


	/** Loop::getBackEdges */
        /** Sets edges to the set of back edges that define this loop,
            returns the number of back edges that define this loop */
        int getBackEdges(std::vector<Edge*> &edges);

        /* returns the entry blocks of the loop.
	 * A natural loop has a single entry block
	 * and an irreducible loop has mulbile entry blocks
	 * */
	int getLoopEntries(std::vector<Block*>&);

	/** Loop::getContainedLoops    */
	/** returns vector of contained loops */

        bool getContainedLoops(std::vector<Loop*> &loops);

	/** Loop::getOuterLoops    */
	/** returns vector of outer contained loops */

	bool getOuterLoops(std::vector<Loop*> &loops);

	/** Loop::getLoopBasicBlocks    */
	/** returns all basic blocks in the loop */

        bool getLoopBasicBlocks(std::vector<Block*> &blocks);

	/** Loop::getLoopBasicBlocksExclusive    */
	/** returns all basic blocks in this loop, exluding the blocks
	    of its sub loops. */

        bool getLoopBasicBlocksExclusive(std::vector<Block*> &blocks);

        /** does this loop or its subloops contain the given block? */

        bool hasBlock(Block *b);

        /** does this loop contain the given block? */

        bool hasBlockExclusive(Block *b);

	/** Loop::hasAncestor    */
	/** returns true if this loop is a descendant of the given loop */

        bool hasAncestor(Loop *loop);

	/** returns the function this loop is in */

        const Function * getFunction();

	/** Loop::~Loop    */
	/** destructor for the class */

        ~Loop() { }

        std::string format() const;
    void insertLoop(Loop *childLoop);

private:
// internal use only
	/** constructor of class */
	Loop(const Function *);

	/** constructor of the class */
	Loop(Edge *, const Function *);

	/** get either contained or outer loops, determined by outerMostOnly */
	bool getLoops(std::vector<Loop*>&, 
		      bool outerMostOnly) const;

        }; // class Loop

/** A class to represent the tree of nested loops and 
 *  callees (functions) in the control flow graph.
 *  @see BPatch_basicBlockLoop
 *  @see BPatch_flowGraph
 */

class PARSER_EXPORT LoopTreeNode {
    friend class LoopAnalyzer;

 public:
    // A loop node contains a single Loop instance
    Loop *loop;

    // The LoopTreeNode instances nested within this loop.
    std::vector<LoopTreeNode *> children;

    //  LoopTreeNode::LoopTreeNode
    //  Create a loop tree node for Loop with name n 
    LoopTreeNode(Loop *l, const char *n);

    //  Destructor
    ~LoopTreeNode();

    //  LoopTreeNode::name
    //  Return the name of this loop. 
    const char * name(); 

    //  LoopTreeNode::getCalleeName
    //  Return the function name of the ith callee. 
    const char * getCalleeName(unsigned int i);

    //  LoopTreeNode::numCallees
    //  Return the number of callees contained in this loop's body. 
    unsigned int numCallees();

    //Returns a vector of the functions called by this loop.
    bool getCallees(std::vector<Function *> &v);
    

    //  BPatch_loopTreeNode::findLoop
    //  find loop by hierarchical name
    Loop * findLoop(const char *name);

 private:

    /** name which indicates this loop's relative nesting */
    char *hierarchicalName;

    // A vector of functions called within the body of this loop (and
    // not the body of sub loops). 
    std::vector<Function *> callees;

}; // class LoopTreeNode 

} //namespace ParseAPI
} //namespace Dyninst

#endif
