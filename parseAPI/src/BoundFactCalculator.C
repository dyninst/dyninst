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
    

    defined.clear();
    queue<Node::Ptr> workingList;

    NodeIterator nbegin, nend;
    slice->allNodes(nbegin, nend);

    for (; nbegin != nend; ++nbegin) {
        workingList.push(*nbegin);
    }


    while (!workingList.empty()) {
        Node::Ptr curNode = workingList.front();
	workingList.pop();

        BoundFact oldFact = boundFacts[curNode.get()];
	Meet(curNode);
	CalcTransferFunction(curNode);

	if (defined.find(curNode) == defined.end() || oldFact != boundFacts[curNode.get()]) {
	    curNode->outs(nbegin, nend);
	    for (; nbegin != nend; ++nbegin)
	        workingList.push(*nbegin);
	}
	defined.insert(curNode);
    }

    return true;
}


void BoundFactsCalculator::ConditionalJumpBound(BoundFact& curFact, Node::Ptr src, Node::Ptr trg) {

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
        if (!git->constantBound) continue;

        ParseAPI::Block *guardBlock = git->block;
	// Note that the guardBlock and the srcBlock can be the same, 
	// but since the conditional jump will always be the last instruction
	// in the block, if they are in the same block, the src can reach the guard
	if (rf.incoming[guardBlock].find(srcBlock) == rf.incoming[guardBlock].end()) continue;
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

                if (curFact.cmpBoundFactLive == false || git->cmpBound > curFact.cmpBound) {
		    curFact.cmpAST = git->cmpAST;
		    curFact.cmpBound = git->cmpBound;
		    curFact.cmpBoundFactLive = true;
		    curFact.cmpUsedRegs = git->usedRegs;
		}
	    }
	}
    }
    if (!curFact.cmpBoundFactLive && firstBlock) {
        // If this is indirect jump is in the first block,
	// it is possible that it is a jump table for a function 
	// with variable number of arguments. Then the convention
	// is that al contains the number of argument.
        curFact.cmpAST = VariableAST::create(Variable(Absloc(x86_64::rax)));
	curFact.cmpBound = 8;
	curFact.cmpUsedRegs.insert(x86_64::rax);
	curFact.cmpBoundFactLive = true;


    }
}

void BoundFactsCalculator::Meet(Node::Ptr curNode) {

    SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(curNode); 
    parsing_printf("Calculate Meet for %lx\n", node->addr());

    NodeIterator gbegin, gend;
    curNode->ins(gbegin, gend);    
    BoundFact &curFact = boundFacts[curNode.get()];

    if (gbegin != gend) {
        bool first = true;	
	for (; gbegin != gend; ++gbegin) {
	    SliceNode::Ptr meetSliceNode = boost::static_pointer_cast<SliceNode>(*gbegin);          
	    if (defined.find(*gbegin) == defined.end()) {
	        parsing_printf("\tIncoming node %lx has not been calculated yet\n", meetSliceNode->addr());
		continue;
	    }
	    parsing_printf("\tMeet incoming edge from %lx\n", meetSliceNode->addr());
	    BoundFact prevFact = boundFacts[(*gbegin).get()]; 
	    ConditionalJumpBound(prevFact, *gbegin, curNode);
	    if (first) {
	        first = false;
		curFact = prevFact;
	    } else
	        curFact.Intersect(prevFact);
	}
    } else {
        ConditionalJumpBound(curFact, Node::Ptr(), curNode);
	parsing_printf("Meet no incoming nodes\n");
	curFact.Print();
    }
}

void BoundFactsCalculator::CalcTransferFunction(Node::Ptr curNode){

    SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(curNode);    
    AbsRegion &ar = node->assign()->out();

    BoundFact &curFact = boundFacts[curNode.get()];
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
    curFact.Print();

    BoundCalcVisitor bcv(curFact);
    calculation->accept(&bcv);

    if (bcv.IsResultBounded(calculation)) { 
        parsing_printf("Genenerate bound fact for %s\n", ar.absloc().format().c_str());
        curFact.GenFact(ar.absloc(), bcv.GetResultBound(calculation));
    }
    else {
        parsing_printf("Kill bound fact for %s\n", ar.absloc().format().c_str());
        curFact.KillFact(ar.absloc());
    }
    parsing_printf("Calculating transfer function: Output facts\n");
    curFact.Print();
}						


