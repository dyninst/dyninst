// Simple search mechanism to assist in short-range slicing.

#include <set>
#include <vector>
#include <queue>
#include "dataflowAPI/h/Absloc.h"
#include "dataflowAPI/h/AbslocInterface.h"
#include "Instruction.h"

#include "dataflowAPI/h/stackanalysis.h"

#include "dataflowAPI/h/slicing.h"

#include "dynutil/h/Graph.h"
#include "instructionAPI/h/Instruction.h"
#include "instructionAPI/h/InstructionDecoder.h"

#include "debug_dataflow.h"

#include "parseAPI/h/CFG.h"
#include "parseAPI/h/CodeSource.h"
#include "parseAPI/h/CodeObject.h"

using namespace Dyninst;
using namespace InstructionAPI;
using namespace std;
using namespace ParseAPI;

Address AssignNode::addr() const { 
  if (a_)
    return a_->addr();
  return 0;
}

bool containsCall(ParseAPI::Block *block) {
  // We contain a call if the out-edges include
  // either a CALL or a CALL_FT edge
  const Block::edgelist &targets = block->targets();
  Block::edgelist::iterator eit = targets.begin();
  for (; eit != targets.end(); ++eit) {
    ParseAPI::Edge *edge = *eit;
    if (edge->type() == CALL) return true;
  }
  return false;
}

bool containsRet(ParseAPI::Block *block) {
  // We contain a call if the out-edges include
  // either a CALL or a CALL_FT edge
  const Block::edgelist &targets = block->targets();
  Block::edgelist::iterator eit = targets.begin();
  for (; eit != targets.end(); ++eit) {
    ParseAPI::Edge *edge = *eit;
    if (edge->type() == RET) return true;
  }
  return false;
}

static void getInsnInstances(ParseAPI::Block *block,
		      Slicer::InsnVec &insns) {
  Offset off = block->start();
  const unsigned char *ptr = (const unsigned char *)block->region()->getPtrToInstruction(off);
  if (ptr == NULL) return;
  InstructionDecoder d(ptr, block->size(), block->obj()->cs()->getArch());
  while (off < block->end()) {
    insns.push_back(std::make_pair(d.decode(), off));
    off += insns.back().first->size();
  }
}

ParseAPI::Function *getEntryFunc(ParseAPI::Block *block) {
  return block->obj()->findFuncByEntry(block->region(), block->start());
}

// Constructor. Takes the initial point we slice from. 

// TODO: make this function-less interprocedural. That would throw the
// stack analysis for a loop, but is generally doable...
Slicer::Slicer(Assignment::Ptr a,
               ParseAPI::Block *block,
               ParseAPI::Function *func) : 
  a_(a),
  b_(block),
  f_(func),
  converter(true) {
  df_init_debug();
};

Graph::Ptr Slicer::forwardSlice(Predicates &predicates) {
  return sliceInternal(forward, predicates);
}

Graph::Ptr Slicer::backwardSlice(Predicates &predicates) {
  return sliceInternal(backward, predicates);
}

Graph::Ptr Slicer::sliceInternal(Direction dir,
				 Predicates &p) {
  Graph::Ptr ret = Graph::createGraph();

  // This does the work of forward or backwards slicing;
  // the few different operations are flagged by the
  // direction.

  // e tells us when we should end (naturally)
  // w tells us when we should end and widen
  // c determines whether a call should be followed or skipped
  // a does ???
  
  Element initial;
  constructInitialElement(initial, dir);
  //     constructInitialElementBackward(initial);

  AssignNode::Ptr aP = createNode(initial);
  if (dir == forward)
      slicing_cerr << "Inserting entry node " << aP << "/" << aP->format() << endl;
  else
      slicing_cerr << "Inserting exit node " << aP << "/" << aP->format() << endl;

  insertInitialNode(ret, dir, aP);

  Elements worklist;
  worklist.push(initial);

  std::set<Assignment::Ptr> visited;

  while (!worklist.empty()) {
    Element current = worklist.front(); worklist.pop();

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

    slicing_cerr << "\tSlicing from " << current.ptr->format() << endl;
    
    // Do we widen out? This should check the defined
    // abstract region...
    if (p.widenAtPoint(current.ptr)) {
      slicing_cerr << "\t\t... widens slice" << endl;
      widen(ret, dir, current);
      continue;
    }

    // Do we stop here according to the end predicate?
    if (p.endAtPoint(current.ptr)) {
      slicing_cerr << "\t\t... ends slice" << endl;
      markAsEndNode(ret, dir, current);
      continue;
    }

    Elements found;
    
    if (!getMatchingElements(current, found, p, dir)) {
      widen(ret, dir, current);
    }
    // We actually want to fall through; it's possible to have
    // a partially successful search.

    while (!found.empty()) {
      Element target = found.front(); found.pop();
      insertPair(ret, dir, current, target);
      worklist.push(target);
    }
  }

  cleanGraph(ret);
  slicing_cerr << "... done" << endl;
  return ret;
}
  
bool Slicer::getMatchingElements(Element &current,
				 Elements &found,
				 Predicates &p,
				 Direction dir) {
  bool ret = true;
  if (dir == forward) {
    // Find everyone who uses what this ptr defines
    current.reg = current.ptr->out();
    
    if (!search(current, found, p, 0, // Index doesn't matter as
		// it's set when we find a match
		forward)) {
      ret = false;
    }
  }
  else {
    assert(dir == backward);

    // Find everyone who defines what this instruction uses
    vector<AbsRegion> inputs = current.ptr->inputs();

    for (unsigned int k = 0; k < inputs.size(); ++k) {
      // Do processing on each input
      current.reg = inputs[k];

      if (!search(current, found, p, k, backward)) {
	slicing_cerr << "\t\t... backward search failed" << endl;
	ret = false;
      }
    }
  }
  return ret;
}

bool Slicer::getStackDepth(ParseAPI::Function *func, Address callAddr, long &height) {
  StackAnalysis sA(func);

  StackAnalysis::Height heightSA = sA.findSP(callAddr);

  // Ensure that analysis has been performed.

  assert(!heightSA.isTop());
  
  if (heightSA.isBottom()) {
    return false;
  }
  
  height = heightSA.height();

  //slicing_cerr << "Get stack depth at " << std::hex << callAddr
  //<< std::dec << " " << (int) height << endl;

  return true;
}

void Slicer::pushContext(Context &context,
			 ParseAPI::Function *callee,
			 ParseAPI::Block *callBlock,
			 long stackDepth) {
  slicing_cerr << "pushContext with " << context.size() << " elements" << endl;
  assert(context.front().block == NULL);
  context.front().block = callBlock;

  //cerr << "Saving block @ " << hex << callBlock->end() << dec << " as call block" << endl;

  slicing_cerr << "\t" 
	       << (context.front().func ? context.front().func->name() : "NULL")
	       << ", " 
	       << context.front().stackDepth 
	       << endl;

    context.push_front(ContextElement(callee, stackDepth));
};

void Slicer::popContext(Context &context) {
  context.pop_front();

  context.front().block = NULL;
}

void Slicer::shiftAbsRegion(AbsRegion &callerReg,
			    AbsRegion &calleeReg,
			    long stack_depth,
			    ParseAPI::Function *callee) {
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
        //<< " and setting to function " << callee->name() << endl;
	calleeReg = AbsRegion(Absloc(callerAloc.off() - stack_depth,
				     0, // Entry point has region 0 by definition
				     callee->name()));
      }
    }
  }
}

bool Slicer::handleCallDetails(AbsRegion &reg,
			Context &context,
			ParseAPI::Block *callerBlock,
			ParseAPI::Function *callee) {
  ParseAPI::Function *caller = context.front().func;
  AbsRegion newReg = reg;

  long stack_depth;
  if (!getStackDepth(caller, callerBlock->lastInsnAddr(), stack_depth)) {
    return false;
  }

  // By definition, the stack of the callee starts _before_ the call instruction,
  // so we take the pre-call stack height and run with it. 

  // Increment the context
  pushContext(context, callee, callerBlock, stack_depth);

  // Translate the AbsRegion from caller to callee
  shiftAbsRegion(reg,
		 newReg,
		 stack_depth,
		 callee);

  //slicing_cerr << "After call, context has " << context.size() << " elements" << endl;
  //slicing_cerr << "\t" << (context.front().func ? context.front().func->name() : "NULL")
  //       << ", " << context.front().stackDepth << endl;

  reg = newReg;
  return true;
}

void Slicer::handleReturnDetails(AbsRegion &reg,
				 Context &context) {
  // We need to add back the appropriate stack depth, basically
  // reversing what we did in handleCall

  //  slicing_cerr << "Return: context has " << context.size() << " elements" << endl;
  //slicing_cerr << "\t" << (context.front().func ? context.front().func->name() : "NULL")
  //<< ", " << context.front().stackDepth << endl;

  long stack_depth = context.front().stackDepth;

  popContext(context);

  assert(!context.empty());

  slicing_cerr << "\t" << (context.front().func ?
			   context.front().func->name() : "NULL")
	       << ", " << context.front().stackDepth << endl;


  AbsRegion newRegion;
  shiftAbsRegion(reg, newRegion,
		 -1*stack_depth,
		 context.front().func);
  reg = newRegion;
}

bool Slicer::handleReturnDetailsBackward(AbsRegion &reg,
        Context &context,
        ParseAPI::Block *callerBlock,
        ParseAPI::Function *callee)
{
    ParseAPI::Function * caller = context.front().func;
    AbsRegion newReg = reg;

    long stack_depth;
    if (!getStackDepth(caller, callerBlock->end(), stack_depth)) {
        return false;
    }

    // Increment the context
    pushContext(context, callee, callerBlock, stack_depth);

    // Translate the AbsRegion from caller to callee
    shiftAbsRegion(reg,
            newReg,
            stack_depth,
            callee);

    reg = newReg;
    return true;
}

void Slicer::handleCallDetailsBackward(AbsRegion &reg,
                                        Context &context) {
    long stack_depth = context.front().stackDepth;

    popContext(context);

    assert(!context.empty());

    slicing_cerr << "\t" << (context.front().func ?
            context.front().func->name() : "NULL")
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
    slicing_cerr << "\t\t\t\t\t Current region is " <<
      current.reg.format() << endl;

    return true;
  }

  bool ret = true;
  // At the end of the block: set up the next blocks.
  bool err = false;

  if (containsCall(current.loc.block)) {
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
  }
  else if (containsRet(current.loc.block)) {
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
  }
  else {
    const Block::edgelist &targets = current.loc.block->targets();
    Block::edgelist::iterator eit = targets.begin();
    for (; eit != targets.end(); ++eit) {
      Element newElement;
      if (handleDefault(*eit,
                        forward,
			current,
			newElement,
			p,
			err)) {
	succ.push(newElement);
      }
      else {
	cerr << " failed handleDefault, err " << err << endl;
      }
    }
  }
  if (err) {
    ret = false;
  }
  
  return ret;
}

bool Slicer::getPredecessors(Element &current, 
			     Elements &pred,
			     Predicates &p) 
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

        slicing_cerr << "\t\t\t\t Adding intra-block predecessor " 
            << std::hex << newElement.loc.addr() << " "  
            << newElement.reg.format() << endl;
        slicing_cerr << "\t\t\t\t Current region is " << current.reg.format() 
            << endl;

        return true;
    }
    
    bool ret = true;
    bool err = false;

    Element newElement;
    Elements newElements;
    SingleContext epred(current.loc.func, true, true);

    const Block::edgelist &sources = current.loc.block->sources();
    Block::edgelist::iterator eit = sources.begin(&epred);
    for ( ; eit != sources.end(); ++eit) {   
      switch ((*eit)->type()) {
      case CALL:
        slicing_cerr << "\t\t Handling call:";
        if (handleCallBackward(*eit,
                    current,
                    newElements,
                    p,
                    err)) {
            slicing_cerr << " succeeded, err " <<err << endl;
            while (newElements.size()) {
                newElement = newElements.front(); newElements.pop();
                pred.push(newElement);
            }
        }
        break;
      case RET:
        slicing_cerr << "\t\t Handling return:";
        if (handleReturnBackward(*eit,
                    current,
                    newElement,
                    p,
                    err)) {
            slicing_cerr << " succeeded, err " << err << endl;
            pred.push(newElement);
        }
        break;
      default:
	    Element newElement;
        if (handleDefault((*eit),
                          backward,
                          current,
                          newElement,
                          p,
                          err)) {
            pred.push(newElement);
        }    
      }
    }
    if (err) {
      ret = false;
    }
    return ret;
 
}


bool Slicer::handleDefault(ParseAPI::Edge *e,
        Direction dir,
        Element &current,
        Element &newElement,
        Predicates &,
        bool &) {
    // Since we're in the same function we can keep the AbsRegion
    // and Context. Instead we only need to update the Location
    newElement = current;

    if (dir == forward) {
        newElement.loc.block = e->trg();

        // Cache the new vector of instruction instances and get iterators into it
        getInsns(newElement.loc);
    } else {
        newElement.loc.block = e->src();

        getInsnsBackward(newElement.loc);
    }

    return true;

}

bool Slicer::handleCall(ParseAPI::Block *block,
			Element &current,
			Element &newElement,
			Predicates &p,
			bool &err) {
  ParseAPI::Block *callee = NULL;
  ParseAPI::Edge *funlink = NULL;

  const Block::edgelist &targets = block->targets();
  Block::edgelist::iterator eit = targets.begin();
  for (; eit != targets.end(); ++eit) {
    if ((*eit)->sinkEdge()) {
      err = true; 
      continue;
    }
    if ((*eit)->type() == CALL) {
      callee = (*eit)->trg();
    }
    else if ((*eit)->type() == CALL_FT) {
      funlink = (*eit);
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
    newElement.loc.func = getEntryFunc(callee);
    assert(newElement.loc.func);
    getInsns(newElement.loc);
    
    // HandleCall updates both an AbsRegion and a context...
    if (!handleCallDetails(newElement.reg,
			   newElement.con,
			   current.loc.block,
			   newElement.loc.func)) {
      slicing_cerr << "Handle of call details failed!" << endl;
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
                       forward,
		       current,
		       newElement,
		       p,
		       err)) {
      slicing_cerr << "handleDefault failed!" << endl;
      err = true;
      return false;
    }
  }
  
  return true;
}

bool Slicer::handleCallBackward(ParseAPI::Edge *edge,
        Element &current,
        Elements &newElements,
        Predicates &,
        bool &)
{
    Element newElement = current;

    // Find the predecessor block...
    Context callerCon = newElement.con;
    callerCon.pop_front();

    if (callerCon.empty()) {
        return false;
    }

    newElement.loc.block = edge->src();

    /* We don't know which function the caller block belongs to,
     * follow each possibility */
    vector<Function *> funcs;
    newElement.loc.block->getFuncs(funcs);
    vector<Function *>::iterator fit;
    for (fit = funcs.begin(); fit != funcs.end(); ++fit) {
        Element curElement = newElement;

        // Pop AbsRegion and Context
        handleCallDetailsBackward(newElement.reg,
                newElement.con);

        curElement.loc.func = *fit;
        getInsnsBackward(curElement.loc);

        newElements.push(curElement);
    }

    return true;
}

bool Slicer::handleReturn(ParseAPI::Block *,
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

  ParseAPI::Block *retBlock = NULL;

#if 0
  // We'd want something like this for a non-context-sensitive traversal.
  // However, what we need here is to strip out the saved block in the
  // Context and use it instead of doing a pointless iteration.
  // Also, looking for CALL_FT edges in a return is... odd... at best.

  const Block::edgelist &targets = current.loc.block->targets();
  Block::edgelist::iterator eit = targets.begin();
  for (; eit != targets.end(); ++eit) {
    if ((*eit)->type() == CALL_FT) {
      retBlock = (*eit)->trg();
      break;
    }
  }
#endif
  retBlock = callerCon.front().block;
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
}

bool Slicer::handleReturnBackward(ParseAPI::Edge *edge,
        Element &current,
        Element &newElement,
        Predicates &p,
        bool &err)
{
    ParseAPI::Block * callee = edge->src();

    if (followReturn(callee, backward, current, p)) {
        if (!callee) {
            err = true;
            return false;
        }

        newElement = current;

        // Update location
        newElement.loc.block = callee;
        newElement.loc.func = getEntryFunc(callee);
        getInsnsBackward(newElement.loc);

        // handleReturnDetailsBackward updates both an AbsRegion and a context
        if (!handleReturnDetailsBackward(newElement.reg,
                    newElement.con,
                    current.loc.block,
                    newElement.loc.func)) {
            err = true;
            return false;
        }
        return true;
    }

    return false;
}

bool Slicer::search(Element &initial,
		    Elements &succ,
		    Predicates &p,
		    int index,
		    Direction dir) {
  bool ret = true;
  
  Assignment::Ptr source = initial.ptr;

  Elements worklist;
  
  if (dir == forward)  {
      slicing_cerr << "\t\t Getting forward successors from " << initial.ptr->format()
          << " - " << initial.reg.format() << endl;
  } else {
      slicing_cerr << "\t\t Getting backward predecessors from " << initial.ptr->format()
          << " - " << initial.reg.format() << endl;
  }

  if (!getNextCandidates(initial, worklist, p, dir)) {
    ret = false;
  }

  // Need this so we don't get trapped in a loop (literally) 
  std::set<Address> visited;
  
  while (!worklist.empty()) {
    Element current = worklist.front();
    worklist.pop();

    if (visited.find(current.addr()) != visited.end()) {
      continue;
    }
    else {
      visited.insert(current.addr());
    }
    
    // After this point we treat current as a scratch space to scribble in
    // and return...

    // Split the instruction up
    std::vector<Assignment::Ptr> assignments;
    Instruction::Ptr insn;
    if (dir == forward)
        insn = current.loc.current->first;
    else
        insn = current.loc.rcurrent->first;
    convertInstruction(insn,
            current.addr(),
            current.loc.func,
            assignments);
    bool keepGoing = true;

    for (std::vector<Assignment::Ptr>::iterator iter = assignments.begin();
	 iter != assignments.end(); ++iter) {
      Assignment::Ptr &assign = *iter;

      findMatches(current, assign, dir, index, succ);

      if (kills(current, assign)) {
	keepGoing = false;
      }
    }
    if (keepGoing) {
      if (!getNextCandidates(current, worklist, p, dir)) {
	ret = false;
      }
    }
  }
  return ret;
}

bool Slicer::getNextCandidates(Element &current, Elements &worklist,
			       Predicates &p, Direction dir) {
  if (dir == forward) {
    return getSuccessors(current, worklist, p);
  }
  else {
    return getPredecessors(current, worklist, p);
  }
}

void Slicer::findMatches(Element &current, Assignment::Ptr &assign, Direction dir, int index, Elements &succ) {
  if (dir == forward) {
    // We compare the AbsRegion in current to the inputs
    // of assign
    for (unsigned k = 0; k < assign->inputs().size(); ++k) {
      const AbsRegion &uReg = assign->inputs()[k];
      if (current.reg.contains(uReg)) {
        // We make a copy of each Element for each Assignment...
        current.ptr = assign;
        current.usedIndex = k;
        succ.push(current);
      }
    }
  }
  else {
    assert(dir == backward);
    const AbsRegion &oReg = assign->out();
    if (current.reg.contains(oReg)) {
      current.ptr = assign;
      current.usedIndex = index;
      succ.push(current);
    }
  }
}

bool Slicer::kills(Element &current, Assignment::Ptr &assign) {
  // Did we find a definition of the same abstract region?
  // TBD: overlaps ins't quite the right thing here. "contained
  // by" would be better, but we need to get the AND/OR
  // of abslocs hammered out.
  return current.reg.contains(assign->out());
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
  ret << "(" << a_->format() << "@" <<
    f_->name() << ")";
  return ret.str();
}

void Slicer::convertInstruction(Instruction::Ptr insn,
				Address addr,
				ParseAPI::Function *func,
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
    getInsnInstances(loc.block, insnCache_[loc.block]);
  }
  
  loc.current = insnCache_[loc.block].begin();
  loc.end = insnCache_[loc.block].end();
}

void Slicer::getInsnsBackward(Location &loc) {
    InsnCache::iterator iter = insnCache_.find(loc.block);
    if (iter == insnCache_.end()) {
      getInsnInstances(loc.block, insnCache_[loc.block]);
    }

    loc.rcurrent = insnCache_[loc.block].rbegin();
    loc.rend = insnCache_[loc.block].rend();
}

void Slicer::insertPair(Graph::Ptr ret,
			Direction dir,
			Element &source,
			Element &target) {
  AssignNode::Ptr s = createNode(source);
  AssignNode::Ptr t = createNode(target);

  slicing_cerr << "Inserting pair: " << s->format() << ", " << t->format() << endl;
  if (dir == forward) {
      ret->insertPair(s, t);

      // Also record which input to t is defined by s
      slicing_cerr << "adding assignment with usedIndex = " << target.usedIndex << endl;
      t->addAssignment(s, target.usedIndex);
  } else {
      ret->insertPair(t, s);
      slicing_cerr << "adding assignment with usedIndex = " << source.usedIndex << endl;
      s->addAssignment(t, target.usedIndex);
  }
}

void Slicer::widen(Graph::Ptr ret,
		   Direction dir,
		   Element &e) {
  if (dir == forward) {
    ret->insertPair(createNode(e),
		    widenNode());
    ret->insertExitNode(widenNode());
  }
  else {
    ret->insertPair(widenNode(), createNode(e));
    ret->insertEntryNode(widenNode());
  }
}

AssignNode::Ptr Slicer::widenNode() {
  if (widen_) {
    return widen_;
  }

  widen_ = AssignNode::create(Assignment::Ptr(),
			      NULL, NULL);
  return widen_;
}

void Slicer::markAsEndNode(Graph::Ptr ret, Direction dir, Element &e) {
  if (dir == forward) {    
    ret->insertExitNode(createNode(e));
  }
  else {
    ret->insertEntryNode(createNode(e));
  }
}

void Slicer::fastForward(Location &loc, Address
			 addr) {
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
    AssignNode::Ptr foozle =
      dyn_detail::boost::dynamic_pointer_cast<AssignNode>(*nbegin);
    //cerr << "Checking " << foozle << "/" << foozle->format() << endl;
    if ((*nbegin)->hasOutEdges()) {
      //cerr << "\t has out edges, leaving in" << endl;
      
        // This cleans up case where we ended a backward slice
        // but never got to mark the node as an entry node
        if (!(*nbegin)->hasInEdges()) {
            ret->markAsEntryNode(foozle);
        }
      continue;
    }
    if (ret->isExitNode(*nbegin)) {
      //cerr << "\t is exit node, leaving in" << endl;
      continue;
    }
    //cerr << "\t deleting" << endl;
    toDelete.push_back(*nbegin);
  }

  for (std::list<Node::Ptr>::iterator tmp =
	 toDelete.begin(); tmp != toDelete.end(); ++tmp) {
    ret->deleteNode(*tmp);
  }
}

bool Slicer::followCall(ParseAPI::Block *target, Direction dir, Element &current, Predicates &p)
{
  // We provide the call stack and the potential callee.
  // It returns whether we follow the call or not.
  
  // A NULL callee indicates an indirect call.
  // TODO on that one...
  
  // Find the callee
  assert(dir == forward);
  ParseAPI::Function *callee = (target ? getEntryFunc(target) : NULL);
  // Create a call stack
  std::stack<std::pair<ParseAPI::Function *, int> > callStack;
  for (Context::reverse_iterator calls = current.con.rbegin();
       calls != current.con.rend();
       ++calls)
    {
      if (calls->func)  {
	//cerr << "Adding " << calls->func->name() << " to call stack" << endl;
	callStack.push(std::make_pair<ParseAPI::Function*, int>(calls->func, calls->stackDepth));
      }
    }
  //cerr << "Calling followCall with stack and " << (callee ? callee->name() : "<NULL>") << endl;
  // FIXME: assuming that this is not a PLT function, since I have no idea at present.
  // -- BW, April 2010
  return p.followCall(callee, callStack, current.reg);
}

bool Slicer::followReturn(ParseAPI::Block *source,
                            Direction dir,
                            Element &current,
                            Predicates &p)
{
    assert(dir == backward);
    ParseAPI::Function * callee = (source ? getEntryFunc(source) : NULL);
    // Create a call stack
    std::stack<std::pair<ParseAPI::Function *, int> > callStack;
    for (Context::reverse_iterator calls = current.con.rbegin();
            calls != current.con.rend();
            ++calls) {
        if (calls->func) {
            callStack.push(std::make_pair<ParseAPI::Function *, int>(calls->func, calls->stackDepth));
        }
    }
    return p.followCall(callee, callStack, current.reg);
}

ParseAPI::Block *Slicer::getBlock(ParseAPI::Edge *e,
				   Direction dir) {
  return ((dir == forward) ? e->trg() : e->src());
}

bool Slicer::isWidenNode(Node::Ptr n) {
  AssignNode::Ptr foozle =
    dyn_detail::boost::dynamic_pointer_cast<AssignNode>(n);
  if (!foozle) return false;
  if (!foozle->assign()) return true;
  return false;
}

void Slicer::insertInitialNode(GraphPtr ret, Direction dir, AssignNode::Ptr aP) {
  if (dir == forward) {
    // Entry node
    ret->insertEntryNode(aP);
  }
  else {
    ret->insertExitNode(aP);
  }
}
  

void Slicer::constructInitialElement(Element &initial, Direction dir) {
  // Cons up the first Element. We need a context, a location, and an
  // abstract region
  ContextElement context(f_);
  initial.con.push_front(ContextElement(f_));
  initial.loc = Location(f_, b_);
  initial.reg = a_->out();
  initial.ptr = a_;

  if (dir == forward) {
    initial.loc.fwd = true;
    getInsns(initial.loc);
    fastForward(initial.loc, a_->addr());
  }
  else {
    initial.loc.fwd = false;
    getInsnsBackward(initial.loc);
    fastBackward(initial.loc, a_->addr());
  }
}   
                                    
