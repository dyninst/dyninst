// A simple forward slice using a search of the control flow graph.
// Templated on a function that says when to stop slicing.

#if !defined(_SLICING_H_)
#define _SLICING_H_

#include "dyn_detail/boost/shared_ptr.hpp"
#include <vector>
#include "dyntypes.h"
#include <queue>
#include <set>
#include <map>
#include <list>
#include <stack>

#include "util.h"
#include "Node.h"
#include "Edge.h"

#include "AbslocInterface.h"


namespace Dyninst {

namespace ParseAPI {
  class Block;
  class Edge;
  class Function;
};

class Assignment;
class AbsRegion;
typedef dyn_detail::boost::shared_ptr<Assignment> AssignmentPtr;

class Graph;
typedef dyn_detail::boost::shared_ptr<Graph> GraphPtr;

class InstructionAPI::Instruction;
typedef dyn_detail::boost::shared_ptr<InstructionAPI::Instruction> InstructionPtr;

// Used in temp slicer; should probably
// replace OperationNodes when we fix up
// the DDG code.
class SliceNode : public Node {
 public:
  typedef dyn_detail::boost::shared_ptr<SliceNode> Ptr;
      
  DATAFLOW_EXPORT static SliceNode::Ptr create(AssignmentPtr ptr,
				ParseAPI::Block *block,
				ParseAPI::Function *func) {
    return Ptr(new SliceNode(ptr, block, func));
  }
      
  DATAFLOW_EXPORT ParseAPI::Block *block() const { return b_; };
  DATAFLOW_EXPORT ParseAPI::Function *func() const { return f_; };
  DATAFLOW_EXPORT Address addr() const;
  DATAFLOW_EXPORT AssignmentPtr assign() const { return a_; }
      
  DATAFLOW_EXPORT Node::Ptr copy() { return Node::Ptr(); }
  DATAFLOW_EXPORT bool isVirtual() const { return false; }
      
  DATAFLOW_EXPORT std::string format() const;
      
  DATAFLOW_EXPORT virtual ~SliceNode() {};
      
 private:
      
 SliceNode(AssignmentPtr ptr,
	    ParseAPI::Block *block,
	    ParseAPI::Function *func) : 
  a_(ptr), b_(block), f_(func) {};
      
  AssignmentPtr a_;
  ParseAPI::Block *b_;
  ParseAPI::Function *f_;
      
  // This is ugly and should be cleaned up once we've figured
  // out how to move forward on edge classes
  std::map<SliceNode::Ptr, unsigned> assignMap_;
};

class SliceEdge : public Edge {
  public:
   typedef dyn_detail::boost::shared_ptr<SliceEdge> Ptr;

   DATAFLOW_EXPORT static SliceEdge::Ptr create(SliceNode::Ptr source,
                                                SliceNode::Ptr target,
                                                AbsRegion &data) {
      return Ptr(new SliceEdge(source, target, data)); 
   }

   const AbsRegion &data() const { return data_; };

  private:
   SliceEdge(const SliceNode::Ptr source, 
             const SliceNode::Ptr target,
             AbsRegion data) 
      : Edge(source, target), data_(data) {};
   AbsRegion data_;
};

class Slicer {
 public:
  typedef std::pair<InstructionPtr, Address> InsnInstance;
  typedef std::vector<InsnInstance> InsnVec;

  DATAFLOW_EXPORT Slicer(AssignmentPtr a,
	 ParseAPI::Block *block,
	 ParseAPI::Function *func);
    
  DATAFLOW_EXPORT static bool isWidenNode(Node::Ptr n);

  class Predicates {
  public:
    typedef std::pair<ParseAPI::Function *, int> StackDepth_t;
    typedef std::stack<StackDepth_t> CallStack_t;

    DATAFLOW_EXPORT virtual bool widenAtPoint(AssignmentPtr) { return false; }
    DATAFLOW_EXPORT virtual bool endAtPoint(AssignmentPtr) { return false; }
    DATAFLOW_EXPORT virtual bool followCall(ParseAPI::Function * /*callee*/,
                                           CallStack_t & /*cs*/,
                                           AbsRegion /*argument*/) { 
       return false; 
    }
    DATAFLOW_EXPORT virtual std::vector<ParseAPI::Function *>
        followCallBackward(ParseAPI::Block * /*callerBlock*/,
                CallStack_t & /*cs*/,
                AbsRegion /* argument */) {
            std::vector<ParseAPI::Function *> vec;
            return vec;
        }
    DATAFLOW_EXPORT virtual bool addPredecessor(AbsRegion reg) {
        return true;
    }
    DATAFLOW_EXPORT virtual bool widenAtAssignment(const AbsRegion & /*in*/,
                                                  const AbsRegion & /*out*/) { 
       return false; 
    }
    DATAFLOW_EXPORT virtual ~Predicates() {};
  };

  DATAFLOW_EXPORT GraphPtr forwardSlice(Predicates &predicates);
  
  DATAFLOW_EXPORT GraphPtr backwardSlice(Predicates &predicates);

 private:

  typedef enum {
    forward,
    backward } Direction;

  typedef std::map<ParseAPI::Block *, InsnVec> InsnCache;


  // Our slicing is context-sensitive; that is, if we enter
  // a function foo from a caller bar, all return edges
  // from foo must enter bar. This makes an assumption that
  // the return address is not modified, but hey. 
  // We represent this as a list of call sites. This is redundant
  // with the image_instPoint data structure, but hopefully that
  // one will be going away. 

  struct ContextElement {
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
    long stackDepth;

  ContextElement(ParseAPI::Function *f) : 
    func(f), block(NULL), stackDepth(-1) {};
  ContextElement(ParseAPI::Function *f, long depth) :
    func(f), block(NULL), stackDepth(depth) {};
  };

  // This should be sufficient...
  typedef std::deque<ContextElement> Context;

  bool getStackDepth(ParseAPI::Function *func, Address callAddr, long &height);

  // Add the newly called function to the given Context.
  void pushContext(Context &context,
		   ParseAPI::Function *callee,
		   ParseAPI::Block *callBlock,
		   long stackDepth);

  // And remove it as appropriate
  void popContext(Context &context);

  // Shift an abs region by a given stack offset
  void shiftAbsRegion(AbsRegion &callerReg,
		      AbsRegion &calleeReg,
		      long stack_depth,
		      ParseAPI::Function *callee);

  // Handling a call does two things:
  // 1) Translates the given AbsRegion into the callee-side
  //	view; this just means adjusting stack locations. 
  // 2) Increases the given context
  // Returns false if we didn't translate the absregion correctly
  bool handleCallDetails(AbsRegion &reg,
			 Context &context,
			 ParseAPI::Block *callerBlock,
			 ParseAPI::Function *callee);

  bool handleCallDetailsBackward(AbsRegion &reg,
                                Context &context,
                                ParseAPI::Block * callBlock,
                                ParseAPI::Function * caller);

  // Where we are in a particular search...
  struct Location {
    // The block we're looking through
    ParseAPI::Function *func;
    ParseAPI::Block *block; // current block

    // Where we are in the block
    InsnVec::iterator current;
    InsnVec::iterator end;

    bool fwd;

    InsnVec::reverse_iterator rcurrent;
    InsnVec::reverse_iterator rend;

    Address addr() const { if(fwd) return current->second; else return rcurrent->second;}

  Location(ParseAPI::Function *f,
	   ParseAPI::Block *b) : func(f), block(b), fwd(true){};
  Location() : func(NULL), block(NULL), fwd(true) {};
  };
    
  typedef std::queue<Location> LocList;
  
  // And the tuple of (context, AbsRegion, Location)
  // that specifies both context and what to search for
  // in that context
  struct Element {
  Element() : valid(true) {};
    Location loc;
    Context con;
    AbsRegion reg;
    // This is for returns, and not for the intermediate
    // steps. OTOH, I'm being a bit lazy...
    Assignment::Ptr ptr;
     AbsRegion inputRegion;
     bool valid;
    Address addr() const { return loc.addr(); }
  };

  typedef std::queue<Element> Elements;

  GraphPtr sliceInternal(Direction dir,
			 Predicates &predicates);
  
  bool getMatchingElements(Element &initial, Elements &worklist,
			   Predicates &p, Direction dir);
  
  bool getNextCandidates(Element &initial, Elements &worklist,
			 Predicates &p, Direction dir);

  void findMatches(Element &current, Assignment::Ptr &assign, 
		   Direction dir,  Elements &succ); 

  bool kills(Element &current, Assignment::Ptr &assign);

  bool followCall(ParseAPI::Block *b,
		  Direction d,
		  Element &current,
		  Predicates &p);
  
  std::vector<ParseAPI::Function *> 
      followCallBackward(ParseAPI::Block * callerBlock,
              Direction d,
              Element &current,
              Predicates &p);

  bool followReturn(ParseAPI::Block *b,
                    Direction d,
                    Element &current,
                    Predicates &p);

  bool handleDefault(ParseAPI::Edge *e,
                     Direction dir,
		     Element &current,
		     Element &newElement,
		     Predicates &p,
		     bool &err);
                
  bool handleCall(ParseAPI::Block *block,
		  Element &current,
		  Element &newElement,
		  Predicates &p,
		  bool &err);

  bool handleCallBackward(ParseAPI::Edge *e,
                          Element &current,
                          Elements &newElements,
                          Predicates &p,
                          bool &err);

  bool handleReturn(ParseAPI::Block *b,
		    Element &current,
		    Element &newElement,
		    Predicates &p,
		    bool &err);

  bool handleReturnBackward(ParseAPI::Edge *e,
                            Element &current,
                            Element &newElement,
                            Predicates &p,
                            bool &err);

  void handleReturnDetails(AbsRegion &reg,
			   Context &context);

  bool handleReturnDetailsBackward(AbsRegion &reg,
                                    Context &context,
                                    ParseAPI::Block *callBlock,
                                    ParseAPI::Function *callee);

  bool getSuccessors(Element &current,
		     Elements &worklist,
		     Predicates &p);

  bool getPredecessors(Element &current,
		       Elements &worklist,
		       Predicates &p);

  bool search(Element &current,
	      Elements &foundList,
	      Predicates &p,
	      Direction dir);

  void widen(GraphPtr graph, Direction dir, Element &source);

  void insertPair(GraphPtr graph,
		  Direction dir,
		  Element &source,
		  Element &target);

  void convertInstruction(InstructionPtr,
			  Address,
			  ParseAPI::Function *,
			  std::vector<AssignmentPtr> &);

  void fastForward(Location &loc, Address addr);

  void fastBackward(Location &loc, Address addr);

  SliceNode::Ptr widenNode();

  void markAsEndNode(GraphPtr ret, Direction dir, Element &current);
  
  void markAsExitNode(GraphPtr ret, Element &current);

  void markAsEntryNode(GraphPtr ret, Element &current);

  void getInsns(Location &loc);

  void getInsnsBackward(Location &loc);

  void setAliases(Assignment::Ptr, Element &);

  SliceNode::Ptr createNode(Element &);

  void cleanGraph(GraphPtr g);

  ParseAPI::Block *getBlock(ParseAPI::Edge *e,
			    Direction dir);
  
  void constructInitialElement(Element &initial, Direction dir);

  void insertInitialNode(GraphPtr ret, Direction dir, SliceNode::Ptr aP);

  InsnCache insnCache_;

  AssignmentPtr a_;
  ParseAPI::Block *b_;
  ParseAPI::Function *f_;

  std::set<SliceNode::Ptr> visited_;
  std::map<AssignmentPtr, SliceNode::Ptr> created_;

  AssignmentConverter converter;

  SliceNode::Ptr widen_;
};
};

#endif
