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
// A simple forward slice using a search of the control flow graph.
// Templated on a function that says when to stop slicing.

#if !defined(_SLICING_H_)
#define _SLICING_H_

#include <vector>
#include "dyntypes.h"
#include <queue>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <list>
#include <stack>
#include <deque>
#include <stddef.h>
#include <string>
#include <utility>

#include "util.h"
#include "Node.h"
#include "Edge.h"

#include "AbslocInterface.h"

#include <boost/functional/hash.hpp>

namespace Dyninst {

namespace ParseAPI {
  class Block;
  class Edge;
  class Function;
}

class Assignment;
class AbsRegion;
typedef boost::shared_ptr<Assignment> AssignmentPtr;

class Graph;
typedef boost::shared_ptr<Graph> GraphPtr;

 namespace InstructionAPI {
   class Instruction;
 }
 typedef boost::shared_ptr<InstructionAPI::Instruction> InstructionPtr;

 class Slicer;

// Used in temp slicer; should probably
// replace OperationNodes when we fix up
// the DDG code.
class DATAFLOW_EXPORT SliceNode : public Node {
 public:
  typedef boost::shared_ptr<SliceNode> Ptr;
      
  static SliceNode::Ptr create(AssignmentPtr ptr,
				ParseAPI::Block *block,
				ParseAPI::Function *func) {
    return Ptr(new SliceNode(ptr, block, func));
  }
      
  ParseAPI::Block *block() const { return b_; }
  ParseAPI::Function *func() const { return f_; }
  Address addr() const;
  AssignmentPtr assign() const { return a_; }
      
  Node::Ptr copy() { return Node::Ptr(); }
  bool isVirtual() const { return false; }
      
  std::string format() const;
      
  virtual ~SliceNode() {}
      
 private:
      
 SliceNode(AssignmentPtr ptr,
	    ParseAPI::Block *block,
	    ParseAPI::Function *func) : 
  a_(ptr), b_(block), f_(func) {}
      
  AssignmentPtr a_;
  ParseAPI::Block *b_;
  ParseAPI::Function *f_;

  friend class Slicer;
};

class SliceEdge : public Edge {
  public:
   typedef boost::shared_ptr<SliceEdge> Ptr;

   DATAFLOW_EXPORT static SliceEdge::Ptr create(SliceNode::Ptr source,
                                                SliceNode::Ptr target,
                                                AbsRegion const&data) {
      return Ptr(new SliceEdge(source, target, data)); 
   }

   const AbsRegion &data() const { return data_; }

  private:
   SliceEdge(const SliceNode::Ptr source, 
             const SliceNode::Ptr target,
             AbsRegion const& data) 
      : Edge(source, target), data_(data) {}
   AbsRegion data_;
};

class Slicer {
 public:
  typedef std::pair<InstructionAPI::Instruction, Address> InsnInstance;
  typedef std::vector<InsnInstance> InsnVec;

  // An instruction cache to avoid redundant instruction decoding.
  // A user can optionaly provide a cache shared by multiple slicers.
  // The cache is keyed with basic block starting address.
  typedef dyn_hash_map<Address, InsnVec> InsnCache;

  DATAFLOW_EXPORT Slicer(AssignmentPtr a,
	 ParseAPI::Block *block,
	 ParseAPI::Function *func,
	 bool cache = true,
	 bool stackAnalysis = true);

  DATAFLOW_EXPORT Slicer(AssignmentPtr a,
          ParseAPI::Block *block,
          ParseAPI::Function *func,
          AssignmentConverter *ac);

  DATAFLOW_EXPORT Slicer(AssignmentPtr a,
          ParseAPI::Block *block,
          ParseAPI::Function *func,
          AssignmentConverter *ac,
          InsnCache *c);


  DATAFLOW_EXPORT ~Slicer();
    
  DATAFLOW_EXPORT static bool isWidenNode(Node::Ptr n);

  struct DATAFLOW_EXPORT ContextElement {
    // We can implicitly find the callsite given a block,
    // since calls end blocks. It's easier to look up 
    // the successor this way than with an address.

    ParseAPI::Function *func;

    // If non-NULL this must be an internal context
    // element, since we have an active call site.
    ParseAPI::Block *block;

    // To enter or leave a function we must be able to
    // map corresponding abstract regions. 
    // In particular, we need to know the depth of the 
    // stack in the caller.
     int stackDepth;

  ContextElement(ParseAPI::Function *f) : 
    func(f), block(NULL), stackDepth(-1) {}
  ContextElement(ParseAPI::Function *f, long depth) :
    func(f), block(NULL), stackDepth(depth) {}
  };

  // This should be sufficient...
  typedef std::deque<ContextElement> Context;


  // Where we are in a particular search...
  struct DATAFLOW_EXPORT Location {
    // The block we're looking through
    ParseAPI::Function *func;
    ParseAPI::Block *block; // current block

    // Where we are in the block
    InsnVec::iterator current;
    InsnVec::iterator end;

    bool fwd;

    InsnVec::reverse_iterator rcurrent;
    InsnVec::reverse_iterator rend;

    Address addr() const { if(fwd) return (*current).second; else return (*rcurrent).second;}

  Location(ParseAPI::Function *f,
	   ParseAPI::Block *b) : func(f), block(b), fwd(true){}
  Location() : func(NULL), block(NULL), fwd(true) {}
  };
    
  // Describes an abstract region, a minimal context
  // (block and function), and the assignment that
  // relates to that region (uses or defines it, 
  // depending on slice direction)
  //
  // A slice is composed of Elements; SliceFrames 
  // keep a list of the currently active elements
  // that are at the `leading edge' of the 
  // under-construction slice
  struct DATAFLOW_EXPORT Element {
    Element(ParseAPI::Block * b,
        ParseAPI::Function * f,
        AbsRegion const& r,
        Assignment::Ptr p)
      : block(b),
        func(f),
        reg(r),
        ptr(p)
    { }

    // basic comparator for ordering
    bool operator<(const Element& el) const { 
        if (ptr->addr() < el.ptr->addr()) { return true; }
        if (el.ptr->addr() < ptr->addr()) { return false; }
        if (ptr->out() < el.ptr->out()) { return true; }
        return false;
    }

    ParseAPI::Block * block;
    ParseAPI::Function * func;

    AbsRegion reg;
    Assignment::Ptr ptr;
  };

  // State for recursive slicing is a context, location pair
  // and a list of AbsRegions that are being searched for.
  struct DATAFLOW_EXPORT SliceFrame {
    SliceFrame(
        Location const& l,
        Context const& c)
      : loc(l),
        con(c),
        valid(true)
    { }
    SliceFrame() : valid(true) { }
    SliceFrame(bool v) : valid(v) { }

    // Active slice nodes -- describe regions
    // that are currently under scrutiny
    std::map<AbsRegion, std::vector<Element> > active;
    typedef std::map<AbsRegion, std::vector<Element> > ActiveMap;

    Location loc;
    Context con;
    bool valid;

    Address addr() const { return loc.addr(); }
  };


  class Predicates {
    bool clearCache, controlFlowDep;

  public:
    typedef std::pair<ParseAPI::Function *, int> StackDepth_t;
    typedef std::stack<StackDepth_t> CallStack_t;

    DATAFLOW_EXPORT bool performCacheClear() { if (clearCache) {clearCache = false; return true;} else return false; }
    DATAFLOW_EXPORT void setClearCache(bool b) { clearCache = b; }
    DATAFLOW_EXPORT bool searchForControlFlowDep() { return controlFlowDep; }
    DATAFLOW_EXPORT void setSearchForControlFlowDep(bool cfd) { controlFlowDep = cfd; }

    // A negative number means that we do not bound slicing size.
    DATAFLOW_EXPORT virtual int slicingSizeLimitFactor() { return -1; }

    DATAFLOW_EXPORT virtual bool allowImprecision() { return false; }
    DATAFLOW_EXPORT virtual bool widenAtPoint(AssignmentPtr) { return false; }
    DATAFLOW_EXPORT virtual bool endAtPoint(AssignmentPtr) { return false; }
    DATAFLOW_EXPORT virtual bool followCall(ParseAPI::Function * /*callee*/,
                                           CallStack_t & /*cs*/,
                                           AbsRegion /*argument*/) { 
       return false; 
    }
    DATAFLOW_EXPORT virtual std::vector<ParseAPI::Function *> 
        followCallBackward(ParseAPI::Block * /*callerB*/,
            CallStack_t & /*cs*/,
            AbsRegion /*argument*/) {
            std::vector<ParseAPI::Function *> vec;
            return vec;
        }
    DATAFLOW_EXPORT virtual bool addPredecessor(AbsRegion /*reg*/) {
        return true;
    }
    DATAFLOW_EXPORT virtual bool widenAtAssignment(const AbsRegion & /*in*/,
                                                  const AbsRegion & /*out*/) { 
       return false; 
    }
    DATAFLOW_EXPORT virtual ~Predicates() {}

    // Callback function when adding a new node to the slice.
    // Return true if we want to continue slicing
    DATAFLOW_EXPORT virtual bool addNodeCallback(AssignmentPtr,
                                                 std::set<ParseAPI::Edge*> &) { return true;}
    // Callback function after we have added new a node and corresponding new edges to the slice.
    // This function allows users to inspect the current slice graph and determine which abslocs
    // need further slicing and which abslocs are no longer interesting, by modifying the current
    // SliceFrame.
    DATAFLOW_EXPORT virtual bool modifyCurrentFrame(SliceFrame &, GraphPtr, Slicer*) {return true;} 						
    DATAFLOW_EXPORT virtual bool ignoreEdge(ParseAPI::Edge*) { return false;}
    DATAFLOW_EXPORT Predicates() : clearCache(false), controlFlowDep(false) {}						

  };

  DATAFLOW_EXPORT GraphPtr forwardSlice(Predicates &predicates);
  
  DATAFLOW_EXPORT GraphPtr backwardSlice(Predicates &predicates);

 private:

  typedef enum {
    forward,
    backward } Direction;


  // Our slicing is context-sensitive; that is, if we enter
  // a function foo from a caller bar, all return edges
  // from foo must enter bar. This makes an assumption that
  // the return address is not modified, but hey. 
  // We represent this as a list of call sites. This is redundant
  // with the image_instPoint data structure, but hopefully that
  // one will be going away. 

  bool getStackDepth(ParseAPI::Function *func, ParseAPI::Block *block, Address callAddr, long &height);

  // Add the newly called function to the given Context.
  void pushContext(Context &context,
		   ParseAPI::Function *callee,
		   ParseAPI::Block *callBlock,
		   long stackDepth);

  // And remove it as appropriate
  void popContext(Context &context);

  typedef std::queue<Location> LocList;
 
  bool ReachableFromBothBranches(ParseAPI::Edge *e, std::vector<Element> &newE);

  // Used for keeping track of visited edges in the
  // slicing search
  struct CacheEdge {
    CacheEdge(Address src, Address trg) : s(src), t(trg) { }
    Address s;
    Address t;

    bool operator<(CacheEdge const& o) const {
        if(s < o.s)
            return true;
        else if(o.s < s)
            return false;
        else if(t < o.t)
            return true;
        else
            return false;
    }
  };

    /* 
     * An element that is a slicing `def' (where
     * def means `definition' in the backward case
     * and `use' in the forward case, along with
     * the associated AbsRegion that labels the slicing
     * edge.
     *
     * These two pieces of information, along with an 
     * element describing the other end of the dependency,
     * are what you need to create a slice edge.
     */
    struct Def {
      Def(Element const& e, AbsRegion const& r) : ele(e), data(r) { } 
      Element ele;
      AbsRegion data;
 
      struct DefHasher {
          size_t operator() (const Def &o) const {
	      return Assignment::AssignmentPtrHasher()(o.ele.ptr);
	  }
      };
      // only the Assignment::Ptr of an Element matters
      // for comparison
      bool operator<(Def const& o) const {
          if(ele.ptr < o.ele.ptr)
              return true;
          else if(o.ele.ptr < ele.ptr)
              return false;
          else if(data < o.data)
              return true;
          else 
              return false;
      }

      bool operator==(Def const &o) const {
          if (ele.ptr != o.ele.ptr) return false;
	  return data == o.data;
      }
    };

    /*
     * A cache from AbsRegions -> Defs.
     *
     * Each node that has been visited in the search
     * has a DefCache that reflects the resolution of
     * any AbsRegions down-slice. If the node is visited
     * again through a different search path (if the graph
     * has fork-join structure), this caching prevents
     * expensive recursion
     */
    class DefCache {
      public:
        DefCache() { }
        ~DefCache() { }

        // add the values from another defcache
        void merge(DefCache const& o);
   
        // replace mappings in this cache with those
        // from another 
        void replace(DefCache const& o);

        std::set<Def> & get(AbsRegion const& r) { 
            return defmap[r];
        }
        bool defines(AbsRegion const& r) const {
            return defmap.find(r) != defmap.end();
        }

        void print() const;

      private:
        std::map< AbsRegion, std::set<Def> > defmap;
    
    };

    // For preventing insertion of duplicate edges
    // into the slice graph
    struct EdgeTuple {
        EdgeTuple(SliceNode::Ptr src, SliceNode::Ptr dst, AbsRegion const& reg)
          : s(src), d(dst), r(reg) { }
        bool operator<(EdgeTuple const& o) const {
            if(s < o.s)
                return true;
            else if(o.s < s)
                return false;
            else if(d < o.d)
                return true;
            else if(o.d < d)
                return false;
            else if(r < o.r)
                return true;
            else
                return false;
        }
	bool operator == (EdgeTuple const &o) const {
	    if (s != o.s) return false;
	    if (d != o.d) return false;
	    return r == o.r;
	}
        SliceNode::Ptr s;
        SliceNode::Ptr d;
        AbsRegion r;
    };

  // Shift an abs region by a given stack offset
  void shiftAbsRegion(AbsRegion const&callerReg,
		      AbsRegion &calleeReg,
		      long stack_depth,
		      ParseAPI::Function *callee);

    // Shift all of the abstract regions active in the current frame
    void shiftAllAbsRegions(
        SliceFrame & cur,
        long stack_depth,
        ParseAPI::Function *callee);

  /*
   * Internal slicing support routines follow. See the
   * implementation file for descriptions of what these
   * routines actually do to implement slicing.
   */
    GraphPtr sliceInternal(Direction dir,
            Predicates &predicates);
    void sliceInternalAux(
            GraphPtr g,
            Direction dir,
            Predicates &p,
            SliceFrame &cand,
            bool skip,
            std::map<CacheEdge, std::set<AbsRegion> > & visited,
            std::unordered_map<Address,DefCache> & single,
            std::unordered_map<Address, DefCache>& cache);

    bool updateAndLink(
            GraphPtr g,
            Direction dir,
            SliceFrame & cand,
            DefCache & cache,
            Predicates &p);

    void updateAndLinkFromCache(
            GraphPtr g,
            Direction dir,
            SliceFrame & f,
            DefCache & cache);

    void removeBlocked(
            SliceFrame & f,
            std::set<AbsRegion> const& block);

    bool stopSlicing(SliceFrame::ActiveMap& active, 
                     GraphPtr g,
                     Address addr,
                     Direction dir);


    void markVisited(
            std::map<CacheEdge, std::set<AbsRegion> > & visited,
            CacheEdge const& e,
            SliceFrame::ActiveMap const& active);

    void cachePotential(
            Direction dir,
            Assignment::Ptr assn,
            DefCache & cache);

    bool findMatch(
            GraphPtr g,
            Direction dir,
            SliceFrame const& cand,
            AbsRegion const& cur,
            Assignment::Ptr assn,
            std::vector<Element> & matches,
            DefCache & cache);

    bool getNextCandidates(
            Direction dir,
            Predicates & p,
            SliceFrame const& cand,
            std::vector<SliceFrame> & newCands);

    /* forward slicing */

    bool getSuccessors(
            Predicates &p,
            SliceFrame const& cand,
            std::vector<SliceFrame> & newCands);


    bool handleCall(
            Predicates & p,
            SliceFrame & cur,
            bool & err);

    bool followCall(
            Predicates & p,
            ParseAPI::Block * target,
            SliceFrame & cur);

    bool handleCallDetails(
            SliceFrame & cur,
            ParseAPI::Block * caller_block);

    bool handleReturn(
            Predicates & p,
            SliceFrame & cur,
            bool & err);

    void handleReturnDetails(
            SliceFrame & cur);

    bool handleDefault(
            Direction dir,
            Predicates & p,
            ParseAPI::Edge * e,
            SliceFrame & cur,
            bool & err);

    /* backwards slicing */

    bool getPredecessors(
            Predicates &p,
            SliceFrame const& cand,
            std::vector<SliceFrame> & newCands);

    bool handleCallBackward(
            Predicates & p,
            SliceFrame const& cand,
            std::vector<SliceFrame> & newCands,
            ParseAPI::Edge * e,
            bool & err);

    std::vector<ParseAPI::Function *> followCallBackward(
            Predicates & p,
            SliceFrame const& cand,
            AbsRegion const& reg,
            ParseAPI::Block * caller_block);

    bool handleCallDetailsBackward(
            SliceFrame & cur);

    bool handleReturnBackward(
            Predicates & p,
            SliceFrame const& cand,
            SliceFrame & newCand,
            ParseAPI::Edge * e,
            bool & err);

    bool handleReturnDetailsBackward(
            SliceFrame & cur,
            ParseAPI::Block * caller_block);

    bool followReturn(
            Predicates & p,
            SliceFrame const& cand,
            ParseAPI::Block * source);
    void handlePredecessorEdge(ParseAPI::Edge* e,
			       Predicates& p,
			       SliceFrame const& cand,
			       std::vector<SliceFrame> & newCands,
			       bool& err,
			       SliceFrame& nf);
  

    /* general slicing support */
  
    void constructInitialFrame(
            Direction dir, 
            SliceFrame & initFrame);
    
    void widenAll(GraphPtr graph, Direction dir, SliceFrame const& frame);
  
  bool kills(AbsRegion const& reg, Assignment::Ptr &assign);

  void widen(GraphPtr graph, Direction dir, Element const&source);

  void insertPair(GraphPtr graph,
		  Direction dir,
		  Element const&source,
		  Element const&target,
          AbsRegion const& data);

  void insertPair(GraphPtr graph,
		  Direction dir,
		  SliceNode::Ptr& source,
		  SliceNode::Ptr& target,
          AbsRegion const& data);

  void convertInstruction(const InstructionAPI::Instruction &,
                          Address,
                          ParseAPI::Function *,
                          ParseAPI::Block *,
                          std::vector<AssignmentPtr> &);

  void fastForward(Location &loc, Address addr);

  void fastBackward(Location &loc, Address addr);

  SliceNode::Ptr widenNode();

  void markAsEndNode(GraphPtr ret, Direction dir, Element &current);
  
  void markAsExitNode(GraphPtr ret, Element &current);

  void markAsEntryNode(GraphPtr ret, Element &current);

  void getInsns(Location &loc);

public:
  void getInsnsBackward(Location &loc);

private:  

  void setAliases(Assignment::Ptr, Element &);

  SliceNode::Ptr createNode(Element const&);

  void cleanGraph(GraphPtr g);

  void promotePlausibleNodes(GraphPtr g, Direction d);

  ParseAPI::Block *getBlock(ParseAPI::Edge *e,
			    Direction dir);
  

  void insertInitialNode(GraphPtr ret, Direction dir, SliceNode::Ptr aP);

  void mergeRecursiveCaches(std::unordered_map<Address, DefCache>& sc, std::unordered_map<Address, DefCache>& c, Address a);

  InsnCache* insnCache_;
  bool own_insnCache;

  AssignmentPtr a_;
  ParseAPI::Block *b_;
  ParseAPI::Function *f_;


  // Assignments map to unique slice nodes
  std::unordered_map<AssignmentPtr, SliceNode::Ptr, Assignment::AssignmentPtrHasher> created_;

  // cache to prevent edge duplication
  struct EdgeTupleHasher {
    size_t operator() (const EdgeTuple& et) const {
        size_t seed = (size_t)(et.s.get());
        boost::hash_combine( seed , (size_t)(et.d.get()));
	return seed;
    }
  };
  std::unordered_map<EdgeTuple, int, EdgeTupleHasher> unique_edges_;

  // map of previous active maps. these are used to end recursion.
  typedef std::map<AbsRegion, std::set<Element> > PrevMap;
  std::map<Address, PrevMap> prev_maps;

  // set of plausible entry/exit nodes.
  std::set<SliceNode::Ptr> plausibleNodes;

  // a stack and set of addresses that mirror our recursion.
  // these are used to detect loops and properly merge cache.
  std::deque<Address> addrStack;
  std::set<Address> addrSet;

  AssignmentConverter* converter;
  bool own_converter;

  SliceNode::Ptr widen_;
 public: 
  // A set of edges that have been visited during slicing,
  // which can be used for external users to figure out
  // which part of code has been analyzed
  std::set<ParseAPI::Edge*> visitedEdges;
};

}

#endif
