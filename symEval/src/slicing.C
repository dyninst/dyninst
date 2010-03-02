// Simple search mechanism to assist in short-range slicing.

#include "dyninstAPI/src/image-func.h"
#include <set>
#include <vector>
#include <queue>
#include "symEval/h/Absloc.h"
#include "symEval/h/AbslocInterface.h"
#include "Instruction.h"

#include "dyninstAPI/src/stackanalysis.h"
#include "dyninstAPI/src/symtab.h"

#include "symEval/h/slicing.h"

#include "dynutil/h/Graph.h"
#include "instructionAPI/h/Instruction.h"

using namespace Dyninst;
using namespace InstructionAPI;
using namespace std;


Address AssignNode::addr() const { 
  if (a_)
    return a_->addr(); 
  return 0;
}

// Constructor. Takes the initial point we slice from. 

// TODO: make this function-less interprocedural. That would throw the
// stack analysis for a loop, but is generally doable...
Slicer::Slicer(Assignment::Ptr a,
	       image_basicBlock *block,
	       image_func *func) : 
  a_(a),
  b_(block),
  f_(func),
  converter(true) {};

Graph::Ptr Slicer::forwardSlice(PredicateFunc &e, PredicateFunc &w) {
  // The End functor tells us when we should end the slice
  // The Widen functor tells us when we should give up and say
  // "this could go anywhere", which is represented with a
  // Node::Ptr. 


  assert(e);
  assert(w);

  Graph::Ptr ret = Graph::createGraph();

  // A forward slice is a graph of all instructions that are affected
  // by the particular point we have identified. We derive it as follows:

  /*
    Worklist W;
    Graph G;
    End E;
    Widen Wi;

    W.push(assignmentPtr);

    while (!W.empty) 
      working = W.pop;
      if (E(working)) continue;
      if (Wi(working))
        G.insert(working, widenMarker);
	continue;
      // Otherwise find children of working and enqueue them
      Let C = findReachingDefs(working);
      Foreach c in C:
        G.insert(working, C)
	W.push(c)
  */

  Element initial;
  // Cons up the first Element. We need a context, a location, and an
  // abstract region
  ContextElement context(f_);
  initial.con.push(ContextElement(f_));
  initial.loc = Location(f_, b_);
  getInsns(initial.loc);
  fastForward(initial.loc, a_->addr());
  initial.reg = a_->out();
  initial.ptr = a_;

  AssignNode::Ptr aP = createNode(initial);
  ret->insertEntryNode(aP);

  Elements worklist;
  worklist.push(initial);

  std::set<Assignment::Ptr> visited;

  while (!worklist.empty()) {
    Element current = worklist.front(); worklist.pop();

    cerr << "Slicing from " << current.ptr->format() << endl;

    assert(current.ptr);

    // As a note, anything we see here has already been added to the
    // return graph. We're trying to decide whether to keep searching.

    // Don't get stuck in a loop
    if (visited.find(current.ptr) != visited.end()) {
      cerr << "\t Already visited, skipping" << endl;
      continue;
    }
    else {
      visited.insert(current.ptr);
    }
    
    // Do we widen out? This should check the defined
    // abstract region...
    if (w(current.ptr)) {
      cerr << "\t\t... widens slice" << endl;
      widen(ret, current);
      continue;
    }

    // Do we stop here according to the end predicate?
    if (e(current.ptr)) {
      cerr << "\t\t... ends slice" << endl;
      markAsExitNode(ret, current);
      continue;
    }

    Elements found;

    // Find everyone who uses what this ptr defines
    current.reg = current.ptr->out();

    if (!forwardSearch(current, found)) {
      cerr << "\t\t... forward search failed" << endl;
      widen(ret, current);
    }
    while (!found.empty()) {
      Element target = found.front(); found.pop();

      cerr << "\t Adding edge to " << target.ptr->format() << endl;

      insertPair(ret, current, target);
      worklist.push(target);
    }
  }
  
  return ret;
}

bool Slicer::getStackDepth(image_func *func, Address callAddr, long &height) {
  StackAnalysis sA(func);

  StackAnalysis::Height heightSA = sA.findSP(callAddr);

  // Ensure that analysis has been performed.
  assert(!heightSA.isTop());
  
  if (heightSA.isBottom()) {
    return false;
  }
  
  height = heightSA.height();
  
  // The height will include the effects of the call
  // Should check the region... 

  cerr << "Get stack depth at " << std::hex << callAddr
       << std::dec << " " << (int) height << endl;

  return true;
}

void Slicer::pushContext(Context &context,
			 image_func *callee,
			 image_basicBlock *callBlock,
			 long stackDepth) {
  assert(context.top().block == NULL);
  context.top().block = callBlock;
  context.top().stackDepth = stackDepth;

  context.push(ContextElement(callee));
};

void Slicer::popContext(Context &context) {
  context.pop();

  context.top().block = NULL;
  context.top().stackDepth = 0;
}

void Slicer::shiftAbsRegion(AbsRegion &callerReg,
			    AbsRegion &calleeReg,
			    long stack_depth,
			    image_func *callee) {
  if (callerReg.abslocs().empty()) {
    // Typed, so just keep the same type and call it a day
    calleeReg = callerReg;
    return;
  }
  else { 
    assert(callerReg.type() == Absloc::Unknown);
    calleeReg = AbsRegion(Absloc::Unknown);
    
    const std::set<Absloc> &alocs = callerReg.abslocs();
    for (std::set<Absloc>::const_iterator iter = alocs.begin();
	 iter != alocs.end(); ++iter) {
      const Absloc &callerAloc = *iter;
      if (callerAloc.type() != Absloc::Stack) {
	calleeReg.insert(callerAloc);
      }
      else {
	if (stack_depth == -1) {
	  // Oops
	  calleeReg = AbsRegion(Absloc::Stack);
	  return;
	}
	else {
	  cerr << "*** Shifting caller absloc " 
	       << (int) callerAloc.loc()
	       << " by stack depth " << stack_depth << endl;
	  calleeReg.insert(Absloc(Absloc::Stack, 
				  callerAloc.loc() - stack_depth,
				  0, // Entry point has region 0 by definition
				  callee->symTabName()));
	}
      }
    }
  }
}

void Slicer::handleCall(AbsRegion &reg,
			Context &context,
			image_basicBlock *callerBlock,
			image_func *callee) {
  image_func *caller = context.top().func;
  AbsRegion newReg = reg;

  long stack_depth;
  if (!getStackDepth(caller, callerBlock->endOffset(), stack_depth))
    stack_depth = -1;

  // Increment the context
  pushContext(context, callee, callerBlock, stack_depth);

  // Translate the AbsRegion from caller to callee
  shiftAbsRegion(reg,
		 newReg,
		 stack_depth,
		 callee);
  
  reg = newReg;
}

void Slicer::handleReturn(AbsRegion &reg,
			  Context &context) {
  // We need to add back the appropriate stack depth, basically
  // reversing what we did in handleCall

  long stack_depth = context.top().stackDepth;
  
  popContext(context);

  AbsRegion newRegion;
  shiftAbsRegion(reg, newRegion, 
		 -1*stack_depth,
		 context.top().func);
}

// Given a <location> this function returns a list of successors.
// If the successor is in a different function the searched-for
// AbsRegion should be updated (along with the context) but this
// doesn't handle that. 

bool Slicer::getSuccessors(Element &current,
			   Elements &succ) {
  // Simple case: if we're not at the end of the instructions
  // in this block, then add the next one and return.

  InsnVec::iterator next = current.loc.current;
  next++;

  if (next != current.loc.end) {
    Element newElement = current;
    // We're in the same context since we're in the same block
    // Also, AbsRegion
    // But update the Location
    newElement.loc.current = next;
    succ.push(newElement);

    cerr << "\t\t\t\t Adding intra-block successor " << newElement.reg.format() << endl;
    cerr << "\t\t\t\t\t Current region is " << current.reg.format() << endl;

    return true;
  }

  bool ret = true;
  // At the end of the block: set up the next blocks.
  std::vector<image_edge *> outEdges;
  current.loc.block->getTargets(outEdges);
  for (std::vector<image_edge *>::iterator iter = outEdges.begin();
       iter != outEdges.end(); iter++) {
    Element newElement;
    
    switch ((*iter)->getType()) {
    case ET_NOEDGE:
    case ET_FUNLINK:
      // We skip these because we're going interprocedural...
      continue;
    case ET_CALL:
      if (!handleCallEdge((*iter)->getTarget(), 
			  current, 
			  newElement)) {
	ret = false;
	continue;
      }
      break;
    default:
      if (!handleDefaultEdge((*iter)->getTarget(), 
			     current, 
			     newElement)) {
	ret = false;
	continue;
      }
      break;
    }

    cerr << "\t\t\t\t Adding inter-block successor " << newElement.reg.format() << endl;

    succ.push(newElement);
  }
  if (current.loc.block->isExitBlock()) {
    Element newElement;
    // Returns false if we're out of context...
    if (handleReturnEdge(current, newElement)) {
      succ.push(newElement);
    }
    else {
      // hrm. Just skip for now, I suppose...
    }
  }
  return ret;
}

bool Slicer::handleDefaultEdge(image_basicBlock *block,
			       Element &current,
			       Element &newElement) {
  // Since we're in the same function we can keep the AbsRegion
  // and Context. Instead we only need to update the Location
  newElement = current;
  newElement.loc.block = block;

  // Cache the new vector of instruction instances and get iterators into it
  getInsns(newElement.loc);

  return true;
}

bool Slicer::handleCallEdge(image_basicBlock *block,
			    Element &current,
			    Element &newElement) {
  // Mildly more complicated than the above due to needing
  // to handle context and tick over the AbsRegion.

  image_func *callee = block->getEntryFunc();
  if (!callee) return false;

  newElement = current;
  // Update location
  newElement.loc.block = block;
  newElement.loc.func = callee;
  getInsns(newElement.loc);

  // HandleCall updates both an AbsRegion and a context...
  handleCall(newElement.reg,
	     newElement.con,
	     current.loc.block,
	     newElement.loc.func);

  return true;
}

bool Slicer::handleReturnEdge(Element &current,
			      Element &newElement) {
  // As handleCallEdge, but now with 50% fewer calls
  newElement = current;

  // Find out the successor block...
  Context callerCon = newElement.con;
  callerCon.pop();

  if (callerCon.empty()) return false;

  image_basicBlock *retBlock = NULL;

  std::vector<image_edge *> outEdges;
  callerCon.top().block->getTargets(outEdges);
  for (std::vector<image_edge *>::iterator iter = outEdges.begin();
       iter != outEdges.end(); iter++) {
    if ((*iter)->getType() == ET_FUNLINK) {
      retBlock = (*iter)->getTarget();
      break;
    }
  }
  assert(retBlock);

  // Pops absregion and context
  handleReturn(newElement.reg,
	       newElement.con);
  
  newElement.loc.func = newElement.con.top().func;
  newElement.loc.block = retBlock;
  getInsns(newElement.loc);
  return true;
};

bool Slicer::forwardSearch(Element &initial,
			   Elements &succ) {
  bool ret = true;

  // Might as well use a breadth-first search
  // 
  // Worklist W = {}
  // For each succ in start.insn.successors
  //   W.push(succ)
  // While W != {}
  //   Let insn I = W.pop
  //   Foreach assignment A in I.assignments
  //     If A.uses(start.defs)
  //       foundList.push_back(A)
  //     If A.defines(start.defs)
  //       defined = true
  //   If (!defined)
  //     Foreach succ in I.successors
  //       If !visisted(I.block)
  //         W.push(succ)

  // The worklist is a queue of (block, int) pairs - 
  // where the int is the index of the instruction within
  // the block.

  // I don't believe we need to keep a visited set, as we stop
  // the first definition anyway (hence visited is implicit)

  Elements worklist;

  cerr << "\t\t Getting forward successors from " << initial.ptr->format() 
       << " - " << initial.reg.format() << endl;

  if (!getSuccessors(initial,
		     worklist)) {
    ret = false;
  }

  // Need this so we don't get trapped in a loop (literally) 
  std::set<Address> visited;
  
  
  while (!worklist.empty()) {
    Element current = worklist.front();
    worklist.pop();

    if (visited.find(current.addr()) != visited.end()) {
      cerr << "Already visited instruction @ " 
	   << std::hex << current.addr() << std::dec
	   << endl;
      continue;
    }
    else {
      visited.insert(current.addr());
    }

    // What we're looking for
    // This gets updated as we move from function to function,
    // so always pull it off current
    const AbsRegion searchRegion = current.reg;

    // After this point we treat current as a scratch space to scribble in
    // and return...

    cerr << "\t\t\t Examining successor @ " << std::hex << current.loc.current->second
	 << std::dec << " with region " << searchRegion.format() << endl;



    // Split the instruction up
    std::vector<Assignment::Ptr> assignments;
    convertInstruction(current.loc.current->first,
		       current.addr(),
		       current.loc.func,
		       assignments);
    bool addSucc = true;

    for (std::vector<Assignment::Ptr>::iterator iter = assignments.begin();
	 iter != assignments.end(); ++iter) {
      Assignment::Ptr &assign = *iter;

      cerr << "\t\t\t\t Assignment " << assign->format() << endl;

      // If this assignment uses an absRegion that overlaps with 
      // searchRegion, add it to the return list. 

      for (unsigned k = 0; k < assign->inputs().size(); ++k) {
	const AbsRegion &uReg = assign->inputs()[k];
	cerr << "\t\t\t\t\t\t" << uReg.format() << endl;
	if (searchRegion.overlaps(uReg)) {
	  cerr << "\t\t\t\t\t Overlaps, adding" << endl;
	  // We make a copy of each Element for each Assignment...
	  current.ptr = assign;
	  succ.push(current);
	}
      }
      // Did we find a definition of the same abstract region?
      // TBD: overlaps isn't quite the right thing here. "contained
      // by" would be better, but we need to get the AND/OR 
      // of abslocs hammered out.
      if (searchRegion.overlaps(assign->out())) {
	cerr << "\t\t\t\t\t Kills, stopping" << endl;
	addSucc = false;
      }
    }
    if (addSucc) {
      if (!getSuccessors(current,
			 worklist))
	ret = false;
    }
  }
  return ret;
}

AssignNode::Ptr Slicer::createNode(Element &elem) {
  if (created_.find(elem.ptr) != created_.end()) {
    return created_[elem.ptr];
  }
  AssignNode::Ptr newNode = AssignNode::create(elem.ptr, elem.loc.block, elem.loc.func);
  created_[elem.ptr] = newNode;
  return newNode;
}

std::string AssignNode::format() const {
  stringstream ret;
  ret << "(" << a_->format() << "@" << f_->symTabName() << ")";
  return ret.str();
}

void Slicer::convertInstruction(Instruction::Ptr insn,
				Address addr,
				image_func *func,
				std::vector<Assignment::Ptr> &ret) {
  converter.convert(insn,
		    addr,
		    func,
		    ret);
  return;
}

void Slicer::getInsns(Location &loc) {

  InsnCache::iterator iter = insnCache_.find(loc.block);
  if (iter == insnCache_.end()) {
    loc.block->getInsnInstances(insnCache_[loc.block]);
  }
  
  loc.current = insnCache_[loc.block].begin();
  loc.end = insnCache_[loc.block].end();
}

void Slicer::insertPair(Graph::Ptr ret,
			Element &source,
			Element &target) {
  ret->insertPair(createNode(source),
		 createNode(target));
}

void Slicer::widen(Graph::Ptr ret,
		   Element &source) {
  ret->insertPair(createNode(source),
		  widenNode());
}

AssignNode::Ptr Slicer::widenNode() {
  if (widen_) {
    return widen_;
  }

  widen_ = AssignNode::create(Assignment::Ptr(), NULL, NULL);
  return widen_;
}

void Slicer::markAsExitNode(Graph::Ptr ret, Element &e) {
  ret->insertExitNode(createNode(e));
}

void Slicer::fastForward(Location &loc, Address addr) {
  while ((loc.current != loc.end) &&
	 (loc.addr() < addr)) {
    loc.current++;
  }
  assert(loc.current != loc.end);
  assert(loc.addr() == addr);  
}
