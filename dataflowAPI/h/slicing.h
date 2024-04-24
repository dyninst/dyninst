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
    ParseAPI::Function *func;

    ParseAPI::Block *block;

     int stackDepth;

  ContextElement(ParseAPI::Function *f) : 
    func(f), block(NULL), stackDepth(-1) {}
  ContextElement(ParseAPI::Function *f, long depth) :
    func(f), block(NULL), stackDepth(depth) {}
  };

  typedef std::deque<ContextElement> Context;


  struct DATAFLOW_EXPORT Location {
    ParseAPI::Function *func;
    ParseAPI::Block *block;

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

    DATAFLOW_EXPORT virtual bool addNodeCallback(AssignmentPtr,
                                                 std::set<ParseAPI::Edge*> &) { return true;}
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

  bool getStackDepth(ParseAPI::Function *func, ParseAPI::Block *block, Address callAddr, long &height);

  void pushContext(Context &context,
		   ParseAPI::Function *callee,
		   ParseAPI::Block *callBlock,
		   long stackDepth);

  void popContext(Context &context);

  typedef std::queue<Location> LocList;
 
  bool ReachableFromBothBranches(ParseAPI::Edge *e, std::vector<Element> &newE);

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

    struct Def {
      Def(Element const& e, AbsRegion const& r) : ele(e), data(r) { } 
      Element ele;
      AbsRegion data;
 
      struct DefHasher {
          size_t operator() (const Def &o) const {
	      return Assignment::AssignmentPtrHasher()(o.ele.ptr);
	  }
      };
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

    class DefCache {
      public:
        DefCache() { }
        ~DefCache() { }

        void merge(DefCache const& o);
   
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

  void shiftAbsRegion(AbsRegion const&callerReg,
		      AbsRegion &calleeReg,
		      long stack_depth,
		      ParseAPI::Function *callee);

    void shiftAllAbsRegions(
        SliceFrame & cur,
        long stack_depth,
        ParseAPI::Function *callee);

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


  std::unordered_map<AssignmentPtr, SliceNode::Ptr, Assignment::AssignmentPtrHasher> created_;

  struct EdgeTupleHasher {
    size_t operator() (const EdgeTuple& et) const {
        size_t seed = (size_t)(et.s.get());
        boost::hash_combine( seed , (size_t)(et.d.get()));
	return seed;
    }
  };
  std::unordered_map<EdgeTuple, int, EdgeTupleHasher> unique_edges_;

  typedef std::map<AbsRegion, std::set<Element> > PrevMap;
  std::map<Address, PrevMap> prev_maps;

  std::set<SliceNode::Ptr> plausibleNodes;

  std::deque<Address> addrStack;
  std::set<Address> addrSet;

  AssignmentConverter* converter;
  bool own_converter;

  SliceNode::Ptr widen_;
 public: 
  std::set<ParseAPI::Edge*> visitedEdges;
};

}

#endif
