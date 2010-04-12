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

#define slicing_cerr if (debug) cerr

static int debug = 0;

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

Graph::Ptr Slicer::forwardSlice(PredicateFunc &e,
				PredicateFunc &w,
				CallStackFunc &c) {
  // The End functor tells us when we should end the slice
  // The Widen functor tells us when we should give up and say
  // "this could go anywhere", which is represented with a
  // Node::Ptr. 

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
  constructInitialElement(initial);

  Predicates p(e, w, c);

  AssignNode::Ptr aP = createNode(initial);
  //cerr << "Inserting entry node " << aP << "/" << aP->format() << endl;
  ret->insertEntryNode(aP);

  Elements worklist;
  worklist.push(initial);

  std::set<Assignment::Ptr> visited;

  while (!worklist.empty()) {
    Element current = worklist.front(); worklist.pop();

    slicing_cerr << "Slicing from " << current.ptr->format() << endl;

    assert(current.ptr);

    // As a note, anything we see here has already been added to the
    // return graph. We're trying to decide whether to keep searching.

    // Don't get stuck in a loop
    if (visited.find(current.ptr) != visited.end()) {
      slicing_cerr << "\t Already visited, skipping" << endl;
      continue;
    }
    else {
      visited.insert(current.ptr);
    }
    
    // Do we widen out? This should check the defined
    // abstract region...
    if (p.widen(current.ptr)) {
      slicing_cerr << "\t\t... widens slice" << endl;
      widen(ret, current);
      continue;
    }

    // Do we stop here according to the end predicate?
    if (p.end(current.ptr)) {
      slicing_cerr << "\t\t... ends slice" << endl;
      markAsExitNode(ret, current);
      continue;
    }

    Elements found;

    // Find everyone who uses what this ptr defines
    current.reg = current.ptr->out();

    if (!forwardSearch(current, found, p)) {
      slicing_cerr << "\t\t... forward search failed" << endl;
      widen(ret, current);
    }
    while (!found.empty()) {
      Element target = found.front(); found.pop();

      slicing_cerr << "\t Adding edge to " << target.ptr->format() << endl;

      insertPair(ret, current, target);
      worklist.push(target);
    }
  }

  cleanGraph(ret);

  return ret;
}

Graph::Ptr Slicer::backwardSlice(PredicateFunc e, PredicateFunc w) {
    // The End functor tells us when we should end the slice
    // The Widen functor tells us when we should give up and say
    // "this could go anywhere", which is represented with a
    // Node::Ptr. 
    
    assert(e);
    assert(w);

    Graph::Ptr ret = Graph::createGraph();

    // A backward slice is a graph of all instructions that define
    // vars used at the particular point we have identified. 
    // We derive it as follows:

    /* 
       Worklist W;
       Graph G;
       End E;
       Widen Wi;

       W.push(assignmentPtr);

       while(!W.empty)
           working = W.pop;
           if (E(working)) continue;
           if (Wi(working))
               G.insert(working, widenMarker);
        continue;
            // Otherwise, find parent(s) and enque them
            Let C = findDefs(working);
            Foreach c in C:
                G.insert(working, c)
                w.push(c) 
    */
    
    Element initial;
    // Cons up the first element. We need a context, a location,
    // and an abstraction region

    ContextElement context(f_);
    initial.con.push_front(ContextElement(f_));
    initial.loc = Location(f_, b_);
    initial.loc.fwd = false;
    getInsnsBackward(initial.loc);
    fastBackward(initial.loc, a_->addr());
    initial.reg = a_->out();
    initial.ptr = a_;

    AssignNode::Ptr aP = createNode(initial);
    cerr << "Inserting exit node " << aP << "/" << aP->format() << endl;
    ret->insertExitNode(aP);

    Elements worklist;
    worklist.push(initial);

    std::set<Assignment::Ptr> visited;

    while (!worklist.empty()) {
        Element current = worklist.front(); worklist.pop();

        cerr << "Slicing from " << current.ptr->format() << endl;

        assert(current.ptr);

	if (visited.find(current.ptr) != visited.end()) {
	  cerr << "\t Already visited, skipping" << endl;
	  continue;
        } else {
            visited.insert(current.ptr);
	}

        // Do we widen out? This should check the defined abstract
        // region
        if (w(current.ptr)) {
            cerr << "\t\t... widens slice" << endl;
            widenBackward(ret, current);
            continue;
        }   

        // Do we stop here according to the end predicate?
        if (e(current.ptr)) {
            cerr << "\t\t... ends slice" << endl;
            markAsEntryNode(ret, current);
            continue;
        }

        Elements found;

        // Find everyone who defines what this instruction uses
        vector<AbsRegion> inputs = current.ptr->inputs();

	if (inputs.empty()) {
	  // We're ending here, so make sure this is labelled
	  // as an entry point.
	  markAsEntryNode(ret, current);
	}
	else {
	  vector<AbsRegion>::iterator input_iter;
	  for (input_iter = inputs.begin();
	       input_iter != inputs.end();
	       input_iter++) {
	    
            // Do processing on each input
            current.reg = (*input_iter);
	    
            if (!backwardSearch(current, found)) {
	      cerr << "\t\t... backward search failed" << endl;
	      widenBackward(ret, current);
            }
	    if(found.empty()) {
	      cerr << "\t\tWARNING: backward search returned true, but found was empty!" << endl;
	      widenBackward(ret, current);
	    }
            while (!found.empty()) {
	      Element target = found.front(); found.pop();
	      
	      cerr << "\t Adding edge to " << target.ptr->format() << endl;
	      
	      insertPair(ret, target, current);
	      worklist.push(target);
            }
	  }
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

  //slicing_cerr << "Get stack depth at " << std::hex << callAddr
  //<< std::dec << " " << (int) height << endl;

  return true;
}

void Slicer::pushContext(Context &context,
			 image_func *callee,
			 image_basicBlock *callBlock,
			 long stackDepth) {
  //slicing_cerr << "pushContext with " << context.size() << " elements" << endl;
  assert(context.front().block == NULL);
  context.front().block = callBlock;

  //slicing_cerr << "\t" << (context.front().func ? context.front().func->symTabName() : "NULL")
  //<< ", " << context.front().stackDepth << endl;

  context.push_front(ContextElement(callee, stackDepth));
};

void Slicer::popContext(Context &context) {
  context.pop_front();

  context.front().block = NULL;
}

void Slicer::shiftAbsRegion(AbsRegion &callerReg,
			    AbsRegion &calleeReg,
			    long stack_depth,
			    image_func *callee) {
  if (callerReg.absloc() == Absloc()) {
    // Typed, so just keep the same type and call it a day
    calleeReg = callerReg;
    return;
  }
  else { 
    assert(callerReg.type() == Absloc::Unknown);
    
    const Absloc &callerAloc = callerReg.absloc();
    if (callerAloc.type() != Absloc::Stack) {
      calleeReg = AbsRegion(callerAloc);
    }
    else {
      if (stack_depth == -1) {
	// Oops
	calleeReg = AbsRegion(Absloc::Stack);
	return;
      }
      else {
	//slicing_cerr << "*** Shifting caller absloc " << callerAloc.off()
	//<< " by stack depth " << stack_depth 
	//<< " and setting to function " << callee->symTabName() << endl;
	calleeReg = AbsRegion(Absloc(callerAloc.off() - stack_depth,
				     0, // Entry point has region 0 by definition
				     callee->symTabName()));
      }
    }
  }
}

bool Slicer::handleCallDetails(AbsRegion &reg,
			       Context &context,
			       image_basicBlock *callerBlock,
			       image_func *callee) {
  image_func *caller = context.front().func;
  AbsRegion newReg = reg;

  long stack_depth;
  if (!getStackDepth(caller, callerBlock->endOffset(), stack_depth)) {
    return false;
  }

  // Increment the context
  pushContext(context, callee, callerBlock, stack_depth);

  // Translate the AbsRegion from caller to callee
  shiftAbsRegion(reg,
		 newReg,
		 stack_depth,
		 callee);

  //slicing_cerr << "After call, context has " << context.size() << " elements" << endl;
  //slicing_cerr << "\t" << (context.front().func ? context.front().func->symTabName() : "NULL")
  //       << ", " << context.front().stackDepth << endl;

  reg = newReg;
  return true;
}

void Slicer::handleReturnDetails(AbsRegion &reg,
				 Context &context) {
  // We need to add back the appropriate stack depth, basically
  // reversing what we did in handleCall

  //  slicing_cerr << "Return: context has " << context.size() << " elements" << endl;
  //slicing_cerr << "\t" << (context.front().func ? context.front().func->symTabName() : "NULL")
  //<< ", " << context.front().stackDepth << endl;

  long stack_depth = context.front().stackDepth;

  popContext(context);

  assert(!context.empty());

  slicing_cerr << "\t" << (context.front().func ? context.front().func->symTabName() : "NULL")
       << ", " << context.front().stackDepth << endl;


  AbsRegion newRegion;
  shiftAbsRegion(reg, newRegion, 
		 -1*stack_depth,
		 context.front().func);
  reg = newRegion;
}

// Given a <location> this function returns a list of successors.
// If the successor is in a different function the searched-for
// AbsRegion should be updated (along with the context) but this
// doesn't handle that. 

bool Slicer::getSuccessors(Element &current,
			   Elements &succ,
			   Predicates &p) {
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

    slicing_cerr << "\t\t\t\t Adding intra-block successor " << newElement.reg.format() << endl;
    slicing_cerr << "\t\t\t\t\t Current region is " << current.reg.format() << endl;

    return true;
  }

  bool ret = true;
  // At the end of the block: set up the next blocks.
  bool err = false;

  if (current.loc.block->containsCall()) {
    Element newElement;
    slicing_cerr << "\t\t Handling call:";
    if (handleCall(current.loc.block,
		   current,
		   newElement,
		   p, 
		   err)) {
      slicing_cerr << " succeeded, err " << err << endl;
      succ.push(newElement);
    }
    else {
      slicing_cerr << " failed, err " << err << endl;
    }
  }
  else if (current.loc.block->containsRet()) { 
    Element newElement;
    slicing_cerr << "\t\t Handling return:";
    if (handleReturn(current.loc.block,
		     current,
		     newElement,
		     p,
		     err)) {
      slicing_cerr << " succeeded, err " << err << endl;
      succ.push(newElement);
    }
    else {
      slicing_cerr << " failed, err " << err << endl;
    }
  }
  else {
    pdvector<image_edge *> outs;
    current.loc.block->getTargets(outs);

    if (outs.empty()) { 
      if (current.loc.block->canBeRelocated()) {
	// Should be a halt. 
	//cerr << "Found block /w/ no successors: " 
	//<< std::hex << current.loc.block->firstInsnOffset() << std::dec << endl;
      }
      else {
	ret = false;
      }
    }
    else {
      for (unsigned i = 0; i < outs.size(); ++i) {
	Element newElement;
	if (handleDefault(outs[i],
			  current,
			  newElement,
			  p,
			  err)) {
	  succ.push(newElement);
	}
      }
    }
  }
  if (err) {
    ret = false;
  }

  return ret;
}

bool Slicer::getPredecessors(Element &current, Elements &pred)
{
    // Simple case: if we're not at the beginning of the instructions
    // in the block, then add the previous one and return
    InsnVec::reverse_iterator prev = current.loc.rcurrent;
    prev++;

    if (prev != current.loc.rend) {
        Element newElement = current;
        // We're in the same context since we're in the same block
        // Also, AbsRegion
        // But update the Location
        newElement.loc.rcurrent = prev;
        pred.push(newElement);

        cerr << "\t\t\t\t Adding intra-block predecessor " 
            << std::hex << newElement.loc.addr() << " "  
            << newElement.reg.format() << endl;
        cerr << "\t\t\t\t Current region is " << current.reg.format() 
            << endl;

        return true;
    }
    
    bool ret = true;
    // At the beginning of the block, set up the previous blocks.
    std::vector<image_edge *> inEdges;
    current.loc.block->getSources(inEdges);
    for (std::vector<image_edge *>::iterator iter = inEdges.begin();
            iter != inEdges.end(); iter++) {
        Element newElement;

        switch((*iter)->getType()) {
            case ET_NOEDGE:
            case ET_FUNLINK:
                // We skip these because we're going interprocedural...
                continue;
            case ET_CALL:
                // STUB
                // We skip this for now because it's complicated
                // Hopefully, this will be easier with parseAPI
                // Need to handle 
                cout << "Found STUB in getPredecessors, ET_CALL" << endl;
                continue;
            default:
                if (!handleDefaultEdgeBackward((*iter)->getSource(),
                            current,
                            newElement)) {
                    ret = false;
                    continue;
                }
                break;
        }

        cerr << "\t\t\t\t Adding inter-block predecessor " 
            << std::hex << newElement.loc.addr() << " "
            << newElement.reg.format() << endl;

        pred.push(newElement);
    }

    if (current.loc.block->isEntryBlock(current.loc.block->getFirstFunc())) {
        // STUB
        cout << "Found STUB in getPredecessors, block is Entry Block" << endl;
    }
    return ret;
}

bool Slicer::handleDefaultEdge(image_basicBlock *block,
			       Element &current,
			       Element &newElement) {
  // Since we're in the same function we can keep the AbsRegion
  // and Context. Instead we only need to update the Location
  newElement = current;
  newElement.loc.block = e->getTarget();
  
  // Cache the new vector of instruction instances and get iterators into it
  getInsns(newElement.loc);
  
  return true;
}

bool Slicer::handleDefaultEdgeBackward(image_basicBlock *block,
			       Element &current,
			       Element &newElement) {
  // Since we're in the same function we can keep the AbsRegion
  // and Context. Instead we only need to update the Location
  newElement = current;
  newElement.loc.block = block;

  // Cache the new vector of instruction instances and get iterators into it
  getInsnsBackward(newElement.loc);

  return true;
}


bool Slicer::handleCall(image_basicBlock* block,
                        Element& current,
                        Element& newElement,
                        Predicates& p,
                        bool& err)
{
    // Best attempt: find the callee
    pdvector<image_edge *> outs;
    block->getTargets(outs);
    image_basicBlock *callee = NULL;
    image_edge *funlink = NULL;
    for (unsigned i = 0; i < outs.size(); ++i) {
        if (outs[i]->getType() == ET_CALL) {
            callee = outs[i]->getTarget();
        } else if (outs[i]->getType() == ET_FUNLINK) {
            funlink = outs[i];
        }
    }
    if (followCall(callee, forward, current, p)) {
        if (!callee) {
            err = true;
            return false;
        }
        newElement = current;
        // Update location
        newElement.loc.block = callee;
        newElement.loc.func = callee->getEntryFunc();
        getInsns(newElement.loc);
        // HandleCall updates both an AbsRegion and a context...
        if (!handleCallDetails(newElement.reg,
             newElement.con,
             current.loc.block,
             newElement.loc.func)) {
                 err = true;
                 return false;
             }
    }
    else {
        // Use the funlink
        if (!funlink) {
            // ???
            return false;
        }
        if (!handleDefault(funlink,
             current,
             newElement,
             p,
             err)) {
                 err = true;
                 return false;
             }
    }

    return true;
}


bool Slicer::handleReturn(image_basicBlock *,
			  Element &current,
			  Element &newElement, 
			  Predicates &,
			  bool &err) {
  // As handleCallEdge, but now with 50% fewer calls
  newElement = current;

  // Find out the successor block...
  Context callerCon = newElement.con;
  callerCon.pop_front();

  if (callerCon.empty()) {
    return false;
  }

  image_basicBlock *retBlock = NULL;

  std::vector<image_edge *> outEdges;
  callerCon.front().block->getTargets(outEdges);
  for (std::vector<image_edge *>::iterator iter = outEdges.begin();
       iter != outEdges.end(); iter++) {
    if ((*iter)->getType() == ET_FUNLINK) {
      retBlock = (*iter)->getTarget();
      break;
    }
  }
  if (!retBlock) {
    err = true;
    return false;
  }

  // Pops absregion and context
  handleReturnDetails(newElement.reg,
		      newElement.con);
  
  newElement.loc.func = newElement.con.front().func;
  newElement.loc.block = retBlock;
  getInsns(newElement.loc);
  return true;
};

bool Slicer::forwardSearch(Element &initial,
			   Elements &succ,
			   Predicates &p) {
  bool ret = true;
  
  Assignment::Ptr source = initial.ptr;

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

  slicing_cerr << "\t\t Getting forward successors from " << initial.ptr->format() 
       << " - " << initial.reg.format() << endl;

  if (!getSuccessors(initial,
		     worklist,
		     p)) {
    ret = false;
  }

  // Need this so we don't get trapped in a loop (literally) 
  std::set<Address> visited;
  
  
  while (!worklist.empty()) {
    Element current = worklist.front();
    worklist.pop();

    if (visited.find(current.addr()) != visited.end()) {
      slicing_cerr << "Already visited instruction @ " 
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

    slicing_cerr << "\t\t\t Examining successor @ " << std::hex << current.loc.current->second
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

      slicing_cerr << "\t\t\t\t Assignment " << assign->format() << endl;

      // If this assignment uses an absRegion that overlaps with 
      // searchRegion, add it to the return list. 

      for (unsigned k = 0; k < assign->inputs().size(); ++k) {
	const AbsRegion &uReg = assign->inputs()[k];
	slicing_cerr << "\t\t\t\t\t\t" << uReg.format() << endl;
	if (searchRegion.contains(uReg)) {
	  slicing_cerr << "\t\t\t\t\t Overlaps, adding" << endl;
	  // We make a copy of each Element for each Assignment...
	  current.ptr = assign;

	  // EMILY: update this too
	  current.usedIndex = k;
	  // ---

	  succ.push(current);
	}
      }
      // Did we find a definition of the same abstract region?
      // TBD: overlaps isn't quite the right thing here. "contained
      // by" would be better, but we need to get the AND/OR 
      // of abslocs hammered out.
      if (searchRegion.contains(assign->out())) {
	slicing_cerr << "\t\t\t\t\t Kills, stopping" << endl;
	addSucc = false;
      }
    }
    if (addSucc) {
      if (!getSuccessors(current,
			 worklist, 
			 p))  {
	ret = false;
      }
    }
  }
  return ret;
}

bool Slicer::backwardSearch(Element &initial, Elements &pred)
{
    bool ret = true;

    // Might as well use a breadth-first search
    //
    // Worklist W = {}
    // For each pred in start.insn.predcessors
    //      W.push(pred)
    // While W != {}
    //      Let insn I = W.pop
    //      Foreach assignment A in I.assignments
    //          If A.uses(start.defs)
    //              foundList.push_back(A)
    //          If A.defins(start.defs)
    //              defined = true
    //          If (!defined)
    //              Foreach pred in I.predecessors
    //                  If !visited(I.block)
    //                      W.push(pred)


    Elements worklist;

    cerr << "\t\t Getting backward predecesors from " 
        << initial.ptr->format() << " - " 
        << initial.reg.format() << endl;

    if (!getPredecessors(initial, worklist)) {
        ret = false;
    }

    // Need this so we don't get trapped in a loop
    std::set<Address> visited;

    while (!worklist.empty()) {
        Element current = worklist.front(); worklist.pop();

        if (visited.find(current.addr()) != visited.end()) {
            cerr << "Already visited instruction @ "
                << std::hex << current.addr() << std::dec
                << endl;
            continue;
        } else {
            visited.insert(current.addr());
        }
        
        // What we're looking for
        // This gets updated as we move from function to function,
        // so always pull it off current
        const AbsRegion searchRegion = current.reg;

        // After this point we treat current as scratch space
        // to scribble on and return

        cerr << "\t\t\t Examining predecessor @ " << std::hex
            << /*current.loc.rcurrent->second*/ current.loc.addr()
            << std::dec << " with region " << searchRegion.format() << endl;

        // Split the instruction up
        std::vector<Assignment::Ptr> assignments;
        convertInstruction(current.loc.rcurrent->first,
                current.addr(),
                current.loc.func,
                assignments);
        bool addPred = true;

        for (std::vector<Assignment::Ptr>::iterator iter = assignments.begin();
                iter != assignments.end();
                ++iter) {
            Assignment::Ptr &assign = *iter;

            cerr << "\t\t\t\t Assignment " << assign->format() << endl;

            // If this assignment uses an AbsRegion that overlaps
            // with searchRegion, add it to the return list
            
            const AbsRegion &oReg = assign->out();
            cerr << "\t\t\t\t\t\t" << oReg.format() << endl;
            if (searchRegion.contains(oReg)) {
                cerr << "\t\t\t\t\t Overlaps, adding" << endl;
                // We make a copy of each Element for each Assignment...
                current.ptr = assign;
		current.usedIndex = std::distance(assignments.begin(), iter);
                pred.push(current);
                addPred = false;
            }
        }   
        
        if (addPred) {
            if (!getPredecessors(current, worklist))
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
  if (!a_) {
    return "<NULL>";
  }

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
  loc.begin = insnCache_[loc.block].begin();
  loc.end = insnCache_[loc.block].end();
}

void Slicer::getInsnsBackward(Location &loc) {
    InsnCache::iterator iter = insnCache_.find(loc.block);
    if (iter == insnCache_.end()) {
        loc.block->getInsnInstances(insnCache_[loc.block]);
    }

    loc.rcurrent = insnCache_[loc.block].rbegin();
    loc.rbegin = insnCache_[loc.block].rbegin();
    loc.rend = insnCache_[loc.block].rend();
}

void Slicer::insertPair(Graph::Ptr ret,
			Element &source,
			Element &target) {
  AssignNode::Ptr s = createNode(source);
  AssignNode::Ptr t = createNode(target);

  ret->insertPair(s, t);

  // Also record which input to t is defined by s
  t->addAssignment(s, target.usedIndex);
}

void Slicer::widen(Graph::Ptr ret,
		   Element &source) {
  ret->insertPair(createNode(source),
		  widenNode());
  //cerr << "Inserting exit node " << widenNode() << endl;

  ret->insertExitNode(widenNode());
}

void Slicer::widenBackward(Graph::Ptr ret,
        Element &target) {
  ret->insertPair(widenNode(), createNode(target));
  ret->insertEntryNode(widenNode());
}

AssignNode::Ptr Slicer::widenNode() {
  if (widen_) {
    return widen_;
  }

  widen_ = AssignNode::create(Assignment::Ptr(), NULL, NULL);
  return widen_;
}

void Slicer::markAsExitNode(Graph::Ptr ret, Element &e) {
  //cerr << "Inserting exit node for assignment " << e.ptr->format() << endl;
  ret->insertExitNode(createNode(e));
}

void Slicer::markAsEntryNode(Graph::Ptr ret, Element &e) {
    ret->insertEntryNode(createNode(e));
}

void Slicer::fastForward(Location &loc, Address addr) {
  while ((loc.current != loc.end) &&
	 (loc.addr() < addr)) {
    loc.current++;
  }
  assert(loc.current != loc.end);
  assert(loc.addr() == addr);  
}
void Slicer::fastBackward(Location &loc, Address addr) {
    while ((loc.rcurrent != loc.rend) &&
            (loc.addr() > addr)) {
        loc.rcurrent++;
    }
    assert(loc.rcurrent != loc.rend);
    assert(loc.addr() == addr);
}

void Slicer::cleanGraph(Graph::Ptr ret) {
  
  // Clean the graph up
  
  // TODO: make this more efficient by backwards traversing.
  // For now, I know that we're generating graphs with tons of
  // unnecessary flag sets (which are immediately killed) and so
  // we don't have long non-exiting chains, we have "fuzz"
  
  NodeIterator nbegin, nend;
  ret->allNodes(nbegin, nend);
  
  std::list<Node::Ptr> toDelete;
  
  for (; nbegin != nend; ++nbegin) {
    AssignNode::Ptr foozle = dyn_detail::boost::dynamic_pointer_cast<AssignNode>(*nbegin);
    //cerr << "Checking " << foozle << "/" << foozle->format() << endl;
    if ((*nbegin)->hasOutEdges()) {
      //cerr << "\t has out edges, leaving in" << endl;
      continue;
    }
    if (ret->isExitNode(*nbegin)) {
      //cerr << "\t is exit node, leaving in" << endl;
      continue;
    }
    //cerr << "\t deleting" << endl;
    toDelete.push_back(*nbegin);
  }

  for (std::list<Node::Ptr>::iterator tmp = toDelete.begin(); tmp != toDelete.end(); ++tmp) {
    ret->deleteNode(*tmp);
  }
}

bool Slicer::followCall(image_basicBlock *target, Direction dir, Element &current, Predicates &p) {
  // We provide the call stack and the potential callee.
  // It returns whether we follow the call or not.
  
  // A NULL callee indicates an indirect call.
  // TODO on that one...
  
  // Find the callee
  assert(dir == forward);
  image_func *callee = (target ? target->getEntryFunc() : NULL);

  // Create a call stack

  std::stack<std::pair<image_func *, int> > callStack;
  for (Context::reverse_iterator calls = current.con.rbegin();
       calls != current.con.rend(); ++calls) {
    if (calls->func)  {
      //cerr << "Adding " << calls->func->symTabName() << " to call stack" << endl;
      callStack.push(std::make_pair<image_func *, int>(calls->func, calls->stackDepth));
    }
  }
  //cerr << "Calling followCall with stack and " << (callee ? callee->symTabName() : "<NULL>") << endl;
  return p.followCall(callee, callStack, current.reg);
}

image_basicBlock *Slicer::getBlock(image_edge *e, 
				   Direction dir) {
  return ((dir == forward) ? e->getTarget() : e->getSource());
}

bool Slicer::isWidenNode(Node::Ptr n) {
  AssignNode::Ptr foozle = dyn_detail::boost::dynamic_pointer_cast<AssignNode>(n);
  if (!foozle) return false;
  if (!foozle->assign()) return true;
  return false;
}

void Slicer::constructInitialElement(Element &initial) {
  // Cons up the first Element. We need a context, a location, and an
  // abstract region
  ContextElement context(f_);
  initial.con.push_front(ContextElement(f_));
  initial.loc = Location(f_, b_);
  getInsns(initial.loc);
  fastForward(initial.loc, a_->addr());
  initial.reg = a_->out();
  initial.ptr = a_;
}
				    
