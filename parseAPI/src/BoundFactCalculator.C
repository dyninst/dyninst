#include "dyntypes.h"
#include "CodeObject.h"
#include "CodeSource.h"

#include "BoundFactCalculator.h"
#include "IndirectASTVisitor.h"
#include "debug_parse.h"
#include "BackwardSlicing.h"
#include "Instruction.h"

using namespace Dyninst::InstructionAPI;

void BoundFactsCalculator::NaturalDFS(Node::Ptr cur) {
    nodeColor[cur] = 1;
    NodeIterator nbegin, nend;
    cur->outs(nbegin, nend);

    for (; nbegin != nend; ++nbegin) 
        if (nodeColor.find(*nbegin) == nodeColor.end())
	    NaturalDFS(*nbegin);

    reverseOrder.push_back(cur);
   
}

void BoundFactsCalculator::ReverseDFS(Node::Ptr cur) {
    nodeColor[cur] = 1;
    analysisOrder[cur] = orderStamp;
    NodeIterator nbegin, nend;
    cur->ins(nbegin, nend);

    for (; nbegin != nend; ++nbegin) 
        if (nodeColor.find(*nbegin) == nodeColor.end())
	    ReverseDFS(*nbegin);
}
void BoundFactsCalculator::DetermineAnalysisOrder() {
    NodeIterator nbegin, nend;
    slice->allNodes(nbegin, nend);

    nodeColor.clear();
    reverseOrder.clear();
    analysisOrder.clear();
    for (; nbegin != nend; ++nbegin) 
        if (nodeColor.find(*nbegin) == nodeColor.end()) {
	    NaturalDFS(*nbegin);
	}
    nodeColor.clear();
    orderStamp = 0;
    for (auto nit = reverseOrder.rbegin(); nit != reverseOrder.rend(); ++nit)
        if (nodeColor.find(*nit) == nodeColor.end()) {
	    ++orderStamp;
	    ReverseDFS(*nit);
	} 
}

bool BoundFactsCalculator::HasIncomingEdgesFromLowerLevel(int curOrder, vector<Node::Ptr>& curNodes) {
    for (auto nit = curNodes.begin(); nit != curNodes.end(); ++nit) {
        Node::Ptr cur = *nit;
	NodeIterator nbegin, nend;
	cur->ins(nbegin, nend);
	for (; nbegin != nend; ++nbegin) 
	    if (analysisOrder[*nbegin] < curOrder) return true;
    }
    return false;

}

bool BoundFactsCalculator::CalculateBoundedFacts() {    
    /* We use a dataflow analysis to calculate the value bound
     * of each register and potentially some memory locations.
     * The key steps of the dataflow analysis are 
     * 1. Determine the analysis order:
     *    First calculate all strongly connected components (SCC)
     *    of the graph. The flow analysis inside a SCC is 
     *    iterative. The flow analysis between several SCCs
     *    is done topologically. 
     * 2. For each node, need to calculate the meet and 
     *    calculate the transfer function.
     * 1. The meet should be simply an intersection of all the bounded facts 
     * along all paths. 
     * 2. To calculate the transfer function, we first get the symbolic expression
     * of the instrution for the node. Then depending on the instruction operation
     * and the meet result, we know what are still bounded. For example, loading 
     * memory is always unbounded; doing and operation on a register with a constant
     * makes the register bounded. 
     */
    
    DetermineAnalysisOrder();

    queue<Node::Ptr> workingList;
    set<Node::Ptr> inQueue;
    map<Node::Ptr, int> inQueueLimit;

    for (int curOrder = 1; curOrder <= orderStamp; ++curOrder) {
        // We first determine which nodes are
	// in this SCC
        vector<Node::Ptr> curNodes;
	NodeIterator nbegin, nend;
	slice->allNodes(nbegin, nend);
	for (; nbegin != nend; ++nbegin) {
	    if (analysisOrder[*nbegin] == curOrder) {
	        curNodes.push_back(*nbegin);
		workingList.push(*nbegin);
		inQueue.insert(*nbegin);
	    } 
	}

	if (!HasIncomingEdgesFromLowerLevel(curOrder, curNodes)) {
	    // If this SCC is an entry SCC,
	    // we choose a node inside the SCC
	    // and let it be top
	    parsing_printf("This SCC does not incoming edges from outside\n");
	    boundFacts[curNodes[0]] = new BoundFact();
	}
	parsing_printf("Starting analysis inside SCC %d\n", curOrder);
	// We now start iterative analysis inside the SCC
	while (!workingList.empty()) {
	    Node::Ptr curNode = workingList.front();
	    workingList.pop();
	    inQueue.erase(curNode);
	    
	    SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(curNode);
	    ++inQueueLimit[curNode];
	    if (inQueueLimit[curNode] > IN_QUEUE_LIMIT) continue;
	    
	    BoundFact* oldFact = GetBoundFact(curNode);
	    parsing_printf("Calculate Meet for %lx", node->addr());
	    if (node->assign()) {
	        parsing_printf(", insn: %s\n", node->assign()->insn()->format().c_str());
//	        if (jumpAddr == 0x805351a) printf("insn at %lx: %s\n", node->addr(), node->assign()->insn()->format().c_str());
	    }
	    else {
	        parsing_printf(", the VirtualExit node\n");
 //	        if (jumpAddr == 0x805351a) printf("virtual node\n");

	    }
	    parsing_printf("\tOld fact for %lx:\n", node->addr());
	    if (oldFact == NULL) parsing_printf("\t\t do not exist\n"); else oldFact->Print();
	    BoundFact* newFact = Meet(curNode);
	    parsing_printf("\tNew fact at %lx\n", node->addr());
	    if (newFact != NULL) newFact->Print(); else parsing_printf("\t\tNot calculated\n");
	    if (newFact != NULL && (oldFact == NULL || *oldFact != *newFact)) {
	        parsing_printf("\tFacts change!\n");
		if (oldFact != NULL) delete oldFact;
		boundFacts[curNode] = newFact;
		curNode->outs(nbegin, nend);
	        for (; nbegin != nend; ++nbegin)
		    // We only add node inside current SCC into the working list
		    if (inQueue.find(*nbegin) == inQueue.end() && analysisOrder[*nbegin] == curOrder) {
		        workingList.push(*nbegin);
			inQueue.insert(*nbegin);
		    }
	    } else {
	        if (newFact != NULL) delete newFact;
		parsing_printf("\tFacts do not change!\n");
	    }

        }



/*
	if (jumpAddr == 0x805351a) {
	    VariableAST::Ptr ecx = VariableAST::create(Variable(AbsRegion(Absloc(x86::ecx))));
	    BoundValue *val = newFact->GetBound(ecx);
	    if (val == NULL) {
	        printf("\tecx is top\n");
	    }
	    else {
	        printf("\tecx: %d[%lx,%lx]\n", val->interval.stride, val->interval.low, val->interval.high);
	    }
	}
*/	
    }

    return true;
}




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
	parsing_printf("\t\tCheck srcAddr at %lx, trgAddr at %lx, thunk at %lx\n", srcAddr, trgAddr, tit->first);
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

	parsing_printf("\t\t\tfind thunk at %lx between the source and the target. Add fact", tit->first);
	BoundValue *bv = new BoundValue(tit->second.value);
	bv->Print();
	curFact->GenFact(VariableAST::create(Variable(AbsRegion(Absloc(tit->second.reg)))), bv);
    }


}


static bool IsConditionalJump(Instruction::Ptr insn) {
    entryID id = insn->getOperation().getID();

    if (id == e_jz || id == e_jnz ||
        id == e_jb || id == e_jnb ||
	id == e_jbe || id == e_jnbe ||
	id == e_jb_jnaej_j || id == e_jnb_jae_j ||
	id == e_jle || id == e_jl ||
	id == e_jnl || id == e_jnle) return true;
    return false;
}

BoundFact* BoundFactsCalculator::Meet(Node::Ptr curNode) {

    SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(curNode);     

    EdgeIterator gbegin, gend;
    curNode->ins(gbegin, gend);    
    BoundFact* newFact = NULL;
    
    bool first = true;	
    for (; gbegin != gend; ++gbegin) {
        TypedSliceEdge::Ptr edge = boost::static_pointer_cast<TypedSliceEdge>(*gbegin);	
	SliceNode::Ptr srcNode = boost::static_pointer_cast<SliceNode>(edge->source()); 
	BoundFact *prevFact = GetBoundFact(srcNode);	    
	if (prevFact == NULL) {
	    parsing_printf("\t\tIncoming node %lx has not been calculated yet, ignore it\n", srcNode->addr());
	    continue;
	} else {	    
	    // Otherwise, create a new copy.
	    // We do not want to overwrite the bound fact
	    // of the predecessor
	    prevFact = new BoundFact(*prevFact);
        }
/*
	if (jumpAddr == 0x805351a) {
	    printf("\tMeet incoming edge from %lx\n", srcNode->addr());
	}
*/
	parsing_printf("\t\tMeet incoming edge from %lx\n", srcNode->addr());
	parsing_printf("\t\tThe fact from %lx before applying transfer function\n", srcNode->addr());
	prevFact->Print();

	if (srcNode->assign() && srcNode->assign()->out().absloc().type() == Absloc::Register &&
	    (srcNode->assign()->out().absloc().reg() == x86::zf || srcNode->assign()->out().absloc().reg() == x86_64::zf)) {
	    // zf should be only predecessor of this node
	    parsing_printf("\t\tThe predecessor node is zf assignment!\n");
	    prevFact->SetPredicate(srcNode->assign());	    
	} else if (srcNode->assign() && IsConditionalJump(srcNode->assign()->insn())) {
	    // If the predecessor is a conditional jump,
	    // we can determine bound fact based on the predicate and the edge type
  	    parsing_printf("\t\tThe predecessor node is a conditional jump!\n");
	    if (!prevFact->ConditionalJumpBound(srcNode->assign()->insn(), edge->type())) {
	        fprintf(stderr, "From %lx to %lx\n", srcNode->addr(), node->addr());
		assert(0);
	    }
	} else {
	    // The predecessor is not a conditional jump,
	    // then we can determine buond fact based on the src assignment
	    parsing_printf("\t\tThe predecessor node is normal node\n");
	    if (srcNode->assign()) parsing_printf("\t\t\tentry id %d\n", srcNode->assign()->insn()->getOperation().getID());
  	    CalcTransferFunction(srcNode, prevFact);
        }
	ThunkBound(prevFact, srcNode, node);
	parsing_printf("\t\tFact from %lx after applying transfer function\n", srcNode->addr());
	prevFact->Print();
        if (first) {
	    // For the first incoming dataflow fact,
	    // we just copy it.
	    // We can do this because if an a-loc
	    // is missing in the fact map, we assume
	    // the a-loc is top. 
	    first = false;
	    newFact = prevFact;
	} else {
	    newFact->Meet(*prevFact);
	    delete prevFact;
        }
    }


    if (newFact == NULL) {
        // This should only happen for nodes without incoming edges;
	if (firstBlock) {
	    // If the indirect jump is in the entry block
	    // of the function, we assume that rax is in
	    // range [0,8] for analyzing the movaps table.
	    // NEED TO HAVE A SAFE WAY TO DO THIS!!
	    parsing_printf("\t\tApplying entry block rax assumption!\n");
	    newFact = new BoundFact();
	    AST::Ptr axAST;
	    if (func->entry()->obj()->cs()->getAddressWidth() == 8)
	        axAST = VariableAST::create(Variable(AbsRegion(Absloc(x86_64::rax))));
	    else
	        // DOES THIS REALLY SHOW UP IN 32-BIT CODE???
	        axAST = VariableAST::create(Variable(AbsRegion(Absloc(x86::eax))));
	    newFact->GenFact(axAST, new BoundValue(StridedInterval(1,0,8)));
	    ThunkBound(newFact, Node::Ptr(), node);

	}    
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
//      parsing_printf("\t\t\t No semantic support for this instruction. Kill all bound fact\n");
//	newFact->SetToBottom();
        parsing_printf("\t\t\t No semantic support for this instruction. Assume it does not affect jump target calculation. Ignore it (Treat as identity function)\n");	
	return;
    }
    
    parsing_printf("\t\t\t AST after expanding (without simplify) %s\n", expandRet.first->format().c_str());
    AST::Ptr calculation = SimplifyAnAST(expandRet.first, insn->size());
    parsing_printf("\t\t\t AST after expanding %s\n", calculation->format().c_str());
	
    BoundCalcVisitor bcv(*newFact, node->block());
    calculation->accept(&bcv);
    
    AST::Ptr outAST = VariableAST::create(Variable(ar));
    if (bcv.IsResultBounded(calculation)) { 
        parsing_printf("\t\t\tGenenerate bound fact for %s\n", ar.absloc().format().c_str());
	newFact->GenFact(outAST, new BoundValue(*bcv.GetResultBound(calculation)));
    }
    else {
        parsing_printf("\t\t\tKill bound fact for %s\n", ar.absloc().format().c_str());
	newFact->KillFact(outAST);
    }
    if (calculation->getID() == AST::V_VariableAST) {
        // We only track alising between registers
	parsing_printf("\t\t\t%s and %s are equal\n", calculation->format().c_str(), outAST->format().c_str());
	newFact->InsertRelation(calculation, outAST, BoundFact::Equal);
    }
    newFact->AdjustPredicate(outAST, calculation);

    // Now try to track all aliasing
    newFact->TrackAlias(DeepCopyAnAST(calculation), ar);

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


