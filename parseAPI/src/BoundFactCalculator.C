#include "dyntypes.h"
#include "CodeObject.h"
#include "CodeSource.h"

#include "IndirectControlFlow.h"
#include "IndirectASTVisitor.h"
#include "debug_parse.h"

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
    map<Node::Ptr, int> inQueueLimit;

    NodeIterator nbegin, nend;
    slice->allNodes(nbegin, nend);

    for (; nbegin != nend; ++nbegin) {
        workingList.push(*nbegin);
    }


    while (!workingList.empty()) {
        Node::Ptr curNode = workingList.front();
	workingList.pop();

	++inQueueLimit[curNode];
	if (inQueueLimit[curNode] > IN_QUEUE_LIMIT) continue;

        BoundFact* oldFact = GetBoundFact(curNode);
	BoundFact* newFact = Meet(curNode);
	CalcTransferFunction(curNode, newFact);

	if (oldFact == NULL || *oldFact != *newFact) {
	    if (oldFact != NULL) delete oldFact;
	    boundFacts[curNode] = newFact;
	    curNode->outs(nbegin, nend);
	    for (; nbegin != nend; ++nbegin)
	        workingList.push(*nbegin);
	}
    }

    return true;
}


void BoundFactsCalculator::ConditionalJumpBound(BoundFact* curFact, Node::Ptr src, Node::Ptr trg) {

    /* This function checks whether any potential table guard is between the two nodes
     * that we are calculating the meet. Essentially, if there is a conditional jump 
     * between the two nodes, we know extra bound information.     
     */   

    ParseAPI::Block *srcBlock;
    if (src == Node::Ptr()) 
        srcBlock = func->entry();
    else {
        SliceNode::Ptr srcNode = boost::static_pointer_cast<SliceNode>(src);
	srcBlock = srcNode->block();

    }
    SliceNode::Ptr trgNode = boost::static_pointer_cast<SliceNode>(trg);			       
    ParseAPI::Block *trgBlock = trgNode->block();

    for (auto git = guards.begin(); git != guards.end(); ++git) {
        parsing_printf("Checking guard at %lx\n", git->jmpInsnAddr);
        if (!git->constantBound) {
	    parsing_printf("\t not a constant bound, skip\n");
	    continue;
        }	    

        ParseAPI::Block *guardBlock = git->block;
	// Note that the guardBlock and the srcBlock can be the same, 
	// but since the conditional jump will always be the last instruction
	// in the block, if they are in the same block, the src can reach the guard
	if (src != Node::Ptr() && rf.incoming[guardBlock].find(srcBlock) == rf.incoming[guardBlock].end()) {
	    parsing_printf("\t this guard is not between the source block %lx and the target %lx, skip\n", srcBlock->start(), guardBlock->start());
	    continue;
        }	    
	bool pred_taken = rf.branch_taken[guardBlock].find(trgBlock) != rf.branch_taken[guardBlock].end();
	bool pred_ft = rf.branch_ft[guardBlock].find(trgBlock) != rf.branch_ft[guardBlock].end();
	parsing_printf("pred_taken : %d, pred_ft: %d\n", pred_taken, pred_ft);
	// If both branches reach the trg node, then this conditional jump 
	// does not bound any variable for the trg node.
	if (pred_taken ^ pred_ft) {
	    // Here I need to figure out which branch bounds the variable and 
	    // check it is the bounded path that reaches the trg node.
	    AST::Ptr cmpAST;
	    bool boundedOnTakenBranch = git->varSubtrahend ^ git->jumpWhenNoZF;
	    if (   (boundedOnTakenBranch && pred_taken)
	        // In thic case, we jump when the variable is smaller than the constant.
		// So the condition taken path is the path that bounds the value.
	        || (!boundedOnTakenBranch && pred_ft) ) {
	        // In thic case, we jump when the variable is larger than the constant.
		// So the fallthrough path is the path that bounds the value.

                if (curFact->cmpBoundFactLive == false || git->cmpBound > curFact->cmpBound) {
		    curFact->cmpAST = git->cmpAST;
		    curFact->cmpBound = git->cmpBound;
		    curFact->cmpBoundFactLive = true;
		    curFact->cmpUsedRegs = git->usedRegs;
		}
	    }
	}
    }
    if (!curFact->cmpBoundFactLive && firstBlock && src == Node::Ptr()) {
        // If this is indirect jump is in the first block,
	// it is possible that it is a jump table for a function 
	// with variable number of arguments. Then the convention
	// is that al contains the number of argument.
	MachRegister reg;
	if (func->obj()->cs()->getAddressWidth() == 8) reg = x86_64::rax; else reg = x86::eax;
        curFact->cmpAST = VariableAST::create(Variable(Absloc(reg)));
	curFact->cmpBound = 8;
	curFact->cmpUsedRegs.insert(reg);
	curFact->cmpBoundFactLive = true;
    }
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


BoundFact* BoundFactsCalculator::Meet(Node::Ptr curNode) {

    SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(curNode); 
    parsing_printf("Calculate Meet for %lx\n", node->addr());

    NodeIterator gbegin, gend;
    curNode->ins(gbegin, gend);    
    BoundFact* newFact = new BoundFact();

    if (gbegin != gend) {
        bool first = true;	
	for (; gbegin != gend; ++gbegin) {
	    SliceNode::Ptr meetSliceNode = boost::static_pointer_cast<SliceNode>(*gbegin); 
	    BoundFact *prevFact = GetBoundFact(*gbegin);	    
	    if (prevFact == NULL) {
	        parsing_printf("\tIncoming node %lx has not been calculated yet\n", meetSliceNode->addr());
		continue;
	    }
	    prevFact = new BoundFact(*prevFact);
	    parsing_printf("\tMeet incoming edge from %lx\n", meetSliceNode->addr());

	    ConditionalJumpBound(prevFact, *gbegin, curNode);
	    ThunkBound(prevFact, *gbegin, curNode);
	    if (first) {
	        first = false;
		*newFact = *prevFact;
	    } else {
	        newFact->Intersect(*prevFact);
	    }
	    parsing_printf("New fact after the change is\n");
	    newFact->Print();
	    delete prevFact;

	}
    } else {
        ConditionalJumpBound(newFact, Node::Ptr(), curNode);
	ThunkBound(newFact, Node::Ptr(), curNode);

	parsing_printf("Meet no incoming nodes\n");
	newFact->Print();
    }
    return newFact;
}

void BoundFactsCalculator::CalcTransferFunction(Node::Ptr curNode, BoundFact *newFact){

    SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(curNode);    
    AbsRegion &ar = node->assign()->out();
    parsing_printf("Expanding assignment %s in instruction at %lx: %s.\n", node->assign()->format().c_str(), node->addr(), node->assign()->insn()->format().c_str());
    pair<AST::Ptr, bool> expandRet = SymEval::expand(node->assign(), false);

    if (expandRet.first == NULL) {
        // If the instruction is outside the set of instrutions we
	// add instruction semantics. We assume this instruction
	// kills all bound fact.
	parsing_printf("\t No semantic support for this instruction. Kill all bound fact\n");
	return;
    }
    parsing_printf("\t AST after expanding (without simplify) %s\n", expandRet.first->format().c_str());

    AST::Ptr calculation = SimplifyAnAST(expandRet.first, node->assign()->insn()->size());

    parsing_printf("\t AST after expanding %s\n", calculation->format().c_str());
    parsing_printf("Calculating transfer function: Input facts\n");
    newFact->Print();

    BoundCalcVisitor bcv(*newFact);
    calculation->accept(&bcv);

    if (bcv.IsResultBounded(calculation)) { 
        parsing_printf("Genenerate bound fact for %s\n", ar.absloc().format().c_str());
        newFact->GenFact(ar.absloc(), new BoundValue(*bcv.GetResultBound(calculation)));
    }
    else {
        parsing_printf("Kill bound fact for %s\n", ar.absloc().format().c_str());
        newFact->KillFact(ar.absloc());
    }
    parsing_printf("Calculating transfer function: Output facts\n");
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


