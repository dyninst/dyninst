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

#include "boost/function.hpp"

#include "dynutil/h/Node.h"

#include "AbslocInterface.h"

class image_basicBlock;
class image_func;
class image_edge;

namespace Dyninst {

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
 class AssignNode : public Node {
 public:
    typedef dyn_detail::boost::shared_ptr<AssignNode> Ptr;

    static AssignNode::Ptr create(AssignmentPtr ptr,
				  image_basicBlock *block,
				  image_func *func) {
      return Ptr(new AssignNode(ptr, block, func)); 
    }

    image_basicBlock *block() const { return b_; };
    image_func *func() const { return f_; };
    Address addr() const;
    AssignmentPtr assign() const { return a_; }

    Node::Ptr copy() { return Node::Ptr(); }
    bool isVirtual() const { return false; }

    std::string format() const;
    
    virtual ~AssignNode() {};

    void addAssignment(AssignNode::Ptr p, unsigned u) {
      assignMap_[p] = u;
    }

    unsigned getAssignmentIndex(AssignNode::Ptr p) {
      return assignMap_[p];
    }

 private:

    AssignNode(AssignmentPtr ptr,
	       image_basicBlock *block,
	       image_func *func) : 
    a_(ptr), b_(block), f_(func) {};

    AssignmentPtr a_;
    image_basicBlock *b_;
    image_func *f_;

    // This is ugly and should be cleaned up once we've figured
    // out how to move forward on edge classes
    std::map<AssignNode::Ptr, unsigned> assignMap_;
};



class Slicer {
 public:
  Slicer(AssignmentPtr a,
	 image_basicBlock *block,
	 image_func *func);

  typedef boost::function<bool (AssignmentPtr a)> PredicateFunc;
  typedef boost::function<bool (image_func *c, std::stack<std::pair<image_func *, int> > &cs, bool plt, AbsRegion a)> CallStackFunc;

  GraphPtr forwardSlice(PredicateFunc &e, PredicateFunc &w, CallStackFunc &c);
  
  GraphPtr backwardSlice(PredicateFunc e, PredicateFunc w);
  
  static bool isWidenNode(Node::Ptr n);

 private:
  typedef std::pair<InstructionPtr, Address> InsnInstance;
  typedef std::vector<InsnInstance> InsnVec;

  typedef enum {
    forward,
    backward } Direction;
  
  struct Predicates {
    // It's safe for these to be references because they will only exist
    // under a forward/backwardSlice call.
    PredicateFunc &end;
    PredicateFunc &widen;
    CallStackFunc &followCall;
    
  Predicates(PredicateFunc &e, PredicateFunc &w, CallStackFunc &c) : end(e), widen(w), followCall(c) {};

  };

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

    image_func *func;

    // If non-NULL this must be an internal context
    // element, since we have an active call site.
    image_basicBlock *block;

    // To enter or leave a function we must be able to
    // map corresponding abstract regions. 
    // In particular, we need to know the depth of the 
    // stack in the caller.
    long stackDepth;

  ContextElement(image_func *f) : 
    func(f), block(NULL), stackDepth(-1) {};
  ContextElement(image_func *f, long depth) :
    func(f), block(NULL), stackDepth(depth) {};
  };

  // This should be sufficient...
  typedef std::deque<ContextElement> Context;

  bool getStackDepth(image_func *func, Address callAddr, long &height);

  // Add the newly called function to the given Context.
  void pushContext(Context &context, 
		   image_func *callee,
		   image_basicBlock *callBlock,
		   long stackDepth);

  // And remove it as appropriate
  void popContext(Context &context);

  // Shift an abs region by a given stack offset
  void shiftAbsRegion(AbsRegion &callerReg,
		      AbsRegion &calleeReg,
		      long stack_depth,
		      image_func *callee);

  // Handling a call does two things:
  // 1) Translates the given AbsRegion into the callee-side
  //    view; this just means adjusting stack locations. 
  // 2) Increases the given context
  // Returns false if we didn't translate the absregion correctly

  void handleCallBackward(AbsRegion &reg,
          Context &context,
          image_basicBlock *calleeBlock,
          image_func *caller);


  // Where we are in a particular search...
  struct Location {
    // The block we're looking through
      image_func *func;
      image_basicBlock *block; // current block

    // Where we are in the block
      InsnVec::iterator current;
      InsnVec::iterator begin;
      InsnVec::iterator end;

      bool fwd;

      InsnVec::reverse_iterator rcurrent;
      InsnVec::reverse_iterator rbegin;
      InsnVec::reverse_iterator rend;

      Address addr() const { if(fwd) return current->second; else return rcurrent->second;}

      Location(image_func *f,
               image_basicBlock *b) : func(f), block(b), fwd(true){};
      Location() : func(NULL), block(NULL), fwd(true) {};
  };
    
  typedef std::queue<Location> LocList;
  

  // And the tuple of (context, AbsRegion, Location)
  // that specifies both context and what to search for
  // in that context
  struct Element {
      Location loc;
      Context con;
      AbsRegion reg;
    // This is for returns, and not for the intermediate
    // steps. OTOH, I'm being a bit lazy...
      Assignment::Ptr ptr;
      unsigned usedIndex;

      Address addr() const { return loc.addr(); }
  };
  typedef std::queue<Element> Elements;

  // Handling a call does two things:
  // 1) Translates the given AbsRegion into the callee-side
  //    view; this just means adjusting stack locations. 
  // 2) Increases the given context
  // Returns false if we didn't translate the absregion correctly
  bool handleCall(image_basicBlock* block,
                    Element& current,
                    Element& newElement,
                    Predicates& p,
                    bool& err);

  // And the corresponding...
  // Returns false if we ran out of context (and thus assume widening for now)
  void handleReturn(AbsRegion &reg,
		    Context &context);
  bool handleCallDetails(AbsRegion &reg,
			 Context &context,
			 image_basicBlock *callerBlock,
			 image_func *callee);

  

  bool followCall(image_basicBlock *b, 
		  Direction d,
		  Element &current,
		  Predicates &p);

  bool handleDefault(image_edge *e,
		     Element &current,
		     Element &newElement,
		     Predicates &p,
		     bool &err);


  bool handleReturn(image_basicBlock *b,
		    Element &current,
		    Element &newElement,
		    Predicates &p,
		    bool &err);
  bool handleDefaultEdgeBackward(image_basicBlock *block,
			 Element &current,
			 Element &newElement); 

  bool handleCallEdge(image_basicBlock *block,
		      Element &current,
		      Element &newElement);

  void handleReturnDetails(AbsRegion &reg,
			   Context &context);

  bool getSuccessors(Element &current,
		     Elements &worklist,
		     Predicates &p);

  bool getPredecessors(Element &current,
          Elements &worklist);

  bool forwardSearch(Element &current,
		     Elements &foundList,
		     Predicates &p);

  bool backwardSearch(Element &current,
          Elements &foundList);

  void widen(GraphPtr graph, Element &source);
  
  void widenBackward(GraphPtr graph, Element &target);

  void insertPair(GraphPtr graph, 
		  Element &source, 
		  Element &target);

  void convertInstruction(InstructionPtr,
			  Address,
			  image_func *,
			  std::vector<AssignmentPtr> &);

  void fastForward(Location &loc, Address addr);
  
  void fastBackward(Location &loc, Address addr);

  AssignNode::Ptr widenNode();

  void markAsExitNode(GraphPtr ret, Element &current);
  
  void markAsEntryNode(GraphPtr ret, Element &current);

  void getInsns(Location &loc);
  void getInsnsBackward(Location &loc);

  void setAliases(Assignment::Ptr, Element &);

  AssignNode::Ptr createNode(Element &);

  void cleanGraph(GraphPtr g);

  image_basicBlock *getBlock(image_edge *e, 
			     Direction dir);
  
  void constructInitialElement(Element &initial);

  typedef std::vector<std::pair<Dyninst::InstructionAPI::Instruction::Ptr, Address> > InsnCache;
  InsnCache insnCache_;

  AssignmentPtr a_;
  image_basicBlock *b_;
  image_func *f_;

  std::set<AssignNode::Ptr> visited_;
  std::map<AssignmentPtr, AssignNode::Ptr> created_;

  AssignmentConverter converter;

  AssignNode::Ptr widen_;
};
};

#endif
