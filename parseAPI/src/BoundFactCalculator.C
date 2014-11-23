#include "dyntypes.h"
#include "CodeObject.h"
#include "CodeSource.h"

#include "IndirectControlFlow.h"
#include "IndirectASTVisitor.h"
#include "debug_parse.h"
#include "BackwardSlicing.h"

bool BoundFactsCalculator::CalculateBoundedFacts() {    
    /* We use a dataflow analysis to calculate what registers are bounded 
     * at each node. The key points of the dataflow analysis are how
     * to calculate the meet and how to calculate the transfer function.
     * 1. The meet should be simply an intersection of all the bounded facts 
     * along all paths. 
     * 2. To calculate the transfer function, we first get the symbolic expression
     * of the instrution for the node. Then depending on the instruction operation
     * and the meet result, we know what are still bounded. For example, loading 
     * memory is always unbounded; doing and operation on a register with a constant
     * makes the register bounded. 
     */
    

    queue<Node::Ptr> workingList;
    set<Node::Ptr> inQueue;
    map<Node::Ptr, int> inQueueLimit;

    NodeIterator nbegin, nend;
    slice->allNodes(nbegin, nend);

    for (; nbegin != nend; ++nbegin) {
        workingList.push(*nbegin);
	inQueue.insert(*nbegin);
    }


    while (!workingList.empty()) {
        Node::Ptr curNode = workingList.front();
	workingList.pop();
	inQueue.erase(curNode);
	
	SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(curNode);
	// All nodes should have an associated assignment
	// except for the virtual exit node.
	// And we do not want to skip the virtual exit node
	if (node->assign()) {
	    Absloc loc = node->assign()->out().absloc();
	    // In principle, we can look at any assignment,
	    // we look at zf because it is simplest. 
	    // So, if it is a flag assignment, and it is not zf, 
	    // we ignore it.
            if (loc.type() == Absloc::Register && 
	        loc.reg().regClass() == x86::FLAG &&
		!(loc.reg() == x86::zf || loc.reg() == x86_64::zf)) continue;
	
        }

	++inQueueLimit[curNode];
	if (inQueueLimit[curNode] > IN_QUEUE_LIMIT) continue;

        BoundFact* oldFact = GetBoundFact(curNode);
	parsing_printf("Calculate Meet for %lx", node->addr());
	if (node->assign())
	    parsing_printf(", insn: %s\n", node->assign()->insn()->format().c_str());
	else
	    parsing_printf(", the VirtualExit node\n");
	parsing_printf("\tOld fact for %lx:\n", node->addr());
	if (oldFact == NULL) parsing_printf("\t\t do not exist\n"); else oldFact->Print();
	BoundFact* newFact = Meet(curNode);
        parsing_printf("\tNew fact at %lx\n", node->addr());
	newFact->Print();
	if (oldFact == NULL || *oldFact != *newFact) {
	    parsing_printf("\tFacts change!\n");
	    if (oldFact != NULL) delete oldFact;
	    boundFacts[curNode] = newFact;
	    curNode->outs(nbegin, nend);
	    for (; nbegin != nend; ++nbegin)
	        if (inQueue.find(*nbegin) == inQueue.end()) {
		    workingList.push(*nbegin);
		    inQueue.insert(*nbegin);
		}
	} else parsing_printf("\tFacts do not change!\n");
    }

    return true;
}



/*
void BoundFactsCalculator::ThunkBound(BoundFact* curFact, Node::Ptr src, Node::Ptr trg) {

    // This function checks whether any found thunk is between 
    // the src node and the trg node. If there is any, then we have 
    // extra bound information to be added.
    ParseAPI::Block *srcBlock;
    Address srcAddr = 0;
    if (src == Node::Ptr()) 
        srcBlock = func->entry();
    else {
        SliceNode::Ptr srcNode = boost::static_pointer_cast<SliceNode>(src);
	srcBlock = srcNode->block();
	srcAddr = srcNode->addr();

    }
    SliceNode::Ptr trgNode = boost::static_pointer_cast<SliceNode>(trg);			       
    ParseAPI::Block *trgBlock = trgNode->block();
    Address trgAddr = trgNode->addr();

    for (auto tit = thunks.begin(); tit != thunks.end(); ++tit) {
        ParseAPI::Block* thunkBlock = tit->second.block;
	parsing_printf("Check srcAddr at %lx, trgAddr at %lx, thunk at %lx\n", srcAddr, trgAddr, tit->first);
	if (src != Node::Ptr()) {
	    if (srcBlock == thunkBlock) {
	        if (srcAddr > tit->first) continue;
	    } else {
	        if (rf.thunk_ins[thunkBlock].find(srcBlock) == rf.thunk_ins[thunkBlock].end()) continue;
	    }
	}
	if (trgBlock == thunkBlock) {
	    if (trgAddr < tit->first) continue;
	} else {
	    if (rf.thunk_outs[thunkBlock].find(trgBlock) == rf.thunk_outs[thunkBlock].end()) continue;
	}

	parsing_printf("\t find thunk at %lx between the source and the target. Add fact", tit->first);
	BoundValue *bv = new BoundValue(Equal, tit->second.value);
	bv->Print();
	curFact->GenFact(Absloc(tit->second.reg), bv);
    }


}
*/

static bool IsConditionalJump(Instruction::Ptr insn) {
    entryID id = insn->getOperation().getID();

    if (id == e_jz || id == e_jnz ||
        id == e_jb || id == e_jnb ||
	id == e_jbe || id == e_jnbe) return true;
    return false;
}

BoundFact* BoundFactsCalculator::Meet(Node::Ptr curNode) {

    SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(curNode);     

    EdgeIterator gbegin, gend;
    curNode->ins(gbegin, gend);    
    BoundFact* newFact = NULL;

    if (node->assign() && IsConditionalJump(node->assign()->insn())) {
        // If the current node defines PC, it should be a conditional jump.
	// Its predecessor nodes should define the flags. 
	// There would be multiple predecessor, but we only look 
	// at the first one because we need to process the whole instruction.	
	parsing_printf("\t\tConditional jump! Try to find the zf slice node in its predecessors .\n");
        TypedSliceEdge::Ptr zfEdge = TypedSliceEdge::Ptr();	
        SliceNode::Ptr zfNode; 
	
	for (; gbegin != gend; ++gbegin) {
	    TypedSliceEdge::Ptr edge = boost::static_pointer_cast<TypedSliceEdge>(*gbegin);
	    SliceNode::Ptr srcNode = boost::static_pointer_cast<SliceNode>(edge->source()); 
	    Absloc loc = srcNode->assign()->out().absloc();

	    // In principle, we can look at any assignment,
	    // we look at zf because it is simplest.
	    if (loc.type() == Absloc::Register && (loc.reg() == x86::zf || loc.reg() == x86_64::zf)) {
	        zfEdge = edge;
		zfNode = srcNode;
	        break;
	    }
	}

	if (zfEdge == TypedSliceEdge::Ptr()) {
	    parsing_printf("WARNING: Do not find zf in the predecessors!!\n");
	    // We know nothing about it.
	    // Set to top.  
	    newFact = new BoundFact();
	} else {
	    newFact = GetBoundFact(zfNode);
	    if (newFact == NULL) {
	        parsing_printf("\t\tIncoming node %lx has not been calculated yet, default to top\n", zfNode->addr());
		newFact = new BoundFact();
	    } else {
	       // Otherwise, create a new copy.
	       // We do not want to overwrite the bound fact
	       // of the predecessor
	        newFact = new BoundFact(*newFact); 
	    }
	    newFact->SetPredicate(zfNode->assign());	    
	    parsing_printf("\t\tNew fact after the change is\n");
	    newFact->Print();
	}
    } else {
        // This is not a conditional jump,
	// We process each assignment separately.
	parsing_printf("\t\tNot conditional jump. Process each predecessor.\n");
	bool first = true;	
	for (; gbegin != gend; ++gbegin) {
	    TypedSliceEdge::Ptr edge = boost::static_pointer_cast<TypedSliceEdge>(*gbegin);	
	    SliceNode::Ptr srcNode = boost::static_pointer_cast<SliceNode>(edge->source()); 
	    BoundFact *prevFact = GetBoundFact(srcNode);	    

	    if (prevFact == NULL) {
	        parsing_printf("\t\tIncoming node %lx has not been calculated yet, default to top\n", srcNode->addr());
		prevFact = new BoundFact();
	    } else {	    
	       // Otherwise, create a new copy.
	       // We do not want to overwrite the bound fact
	       // of the predecessor
	       prevFact = new BoundFact(*prevFact);
	    }
	    parsing_printf("\t\tMeet incoming edge from %lx\n", srcNode->addr());
	    parsing_printf("\t\tThe fact from %lx before applying transfer function\n", srcNode->addr());
	    prevFact->Print();
            if (srcNode->assign() && IsConditionalJump(srcNode->assign()->insn())) {
	        // If the predecessor is a conditional jump,
		// we can determine bound fact based on the predicate and the edge type
		parsing_printf("\t\tIncoming node is a conditional jump!\n");
		prevFact->ConditionalJumpBound(srcNode->assign()->insn(), edge->type());
	    } else {
	        // The predecessor is not a conditional jump,
		// then we can determine buond fact based on the src assignment
		parsing_printf("\t\tnot a conditional jump\n");
		CalcTransferFunction(srcNode, prevFact);

	    }
	    
	    if (first) {
	        // For the first incoming dataflow fact,
	        // we just copy it.
	        // We have to do this because if an a-loc
	        // is missing in the fact map, we assume
	        // the a-loc is bottomed. 
	        first = false;
		newFact = prevFact;
	    } else {
	        newFact->Meet(*prevFact);
		delete prevFact;

	    }
	}
    }

    if (newFact == NULL) {
        // This should only happen for nodes without incoming edges;
	newFact = new BoundFact();
    }
    return newFact;
}

void BoundFactsCalculator::CalcTransferFunction(Node::Ptr curNode, BoundFact *newFact){

    SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(curNode);    
    AbsRegion &ar = node->assign()->out();
    Instruction::Ptr insn = node->assign()->insn();

    parsing_printf("\t\t\tExpanding assignment %s in instruction at %lx: %s.\n", node->assign()->format().c_str(), node->addr(), insn->format().c_str());
    
    pair<AST::Ptr, bool> expandRet = SymEval::expand(node->assign(), false);
	
    if (expandRet.first == NULL) {
        // If the instruction is outside the set of instrutions we
        // add instruction semantics. We assume this instruction
        // kills all bound fact.
        parsing_printf("\t\t\t No semantic support for this instruction. Kill all bound fact\n");
	newFact->SetToBottom();
	return;
    }
    
    parsing_printf("\t\t\t AST after expanding (without simplify) %s\n", expandRet.first->format().c_str());
    AST::Ptr calculation = SimplifyAnAST(expandRet.first, insn->size());
    parsing_printf("\t\t\t AST after expanding %s\n", calculation->format().c_str());
	
    BoundCalcVisitor bcv(*newFact);
    calculation->accept(&bcv);

    if (bcv.IsResultBounded(calculation)) { 
        parsing_printf("\t\t\tGenenerate bound fact for %s\n", ar.absloc().format().c_str());
	newFact->GenFact(ar.absloc(), new BoundValue(*bcv.GetResultBound(calculation)));
    }
    else {
        parsing_printf("\t\t\tKill bound fact for %s\n", ar.absloc().format().c_str());
	newFact->KillFact(ar.absloc());
    }
    parsing_printf("\t\t\tCalculating transfer function: Output facts\n");
    newFact->Print();

}						

BoundFact* BoundFactsCalculator::GetBoundFact(Node::Ptr node) {
    auto fit = boundFacts.find(node);
    if (fit == boundFacts.end())
        return NULL;
    else
        return fit->second;
}

BoundFactsCalculator::~BoundFactsCalculator() {
    for (auto fit = boundFacts.begin(); fit != boundFacts.end(); ++fit)
        if (fit->second != NULL)
	    delete fit->second;
    boundFacts.clear();
}


