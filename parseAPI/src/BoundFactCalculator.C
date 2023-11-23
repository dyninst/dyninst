#include "dyntypes.h"
#include "CodeObject.h"
#include "CodeSource.h"
#include "BoundFactCalculator.h"
#include "IndirectASTVisitor.h"
#include "debug_parse.h"
#include "Instruction.h"
#include "JumpTableIndexPred.h"
#include "registers/x86_64_regs.h"
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

static void BuildEdgeFromVirtualEntry(SliceNode::Ptr virtualEntry,
                                      ParseAPI::Block *curBlock,
				      map<ParseAPI::Block*, vector<SliceNode::Ptr> >&targetMap,
				      set<ParseAPI::Block*> &visit,
				      GraphPtr slice) {
    if (targetMap.find(curBlock) != targetMap.end()) {
        const vector<SliceNode::Ptr> &targets = targetMap[curBlock];
	for (auto nit = targets.begin(); nit != targets.end(); ++nit) {
	    SliceNode::Ptr trgNode = *nit;
	    slice->insertPair(virtualEntry, trgNode, TypedSliceEdge::create(virtualEntry, trgNode, FALLTHROUGH));
	}
	return;
    }
    if (visit.find(curBlock) != visit.end()) return;
    visit.insert(curBlock);
    Block::edgelist targets;
    curBlock->copy_targets(targets);
    for (auto eit = targets.begin(); eit != targets.end(); ++eit)
        if ((*eit)->type() != CALL && (*eit)->type() != RET && (*eit)->type() != CATCH && !(*eit)->interproc()) {
	    BuildEdgeFromVirtualEntry(virtualEntry, (*eit)->trg(), targetMap, visit, slice);
	}
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

    // Create a virtual entry node that has
    // edges into all entry SCCs
    SliceNode::Ptr virtualEntry = SliceNode::create(Assignment::Ptr(), func->entry(), func);
    analysisOrder[virtualEntry] = 0;
    for (int curOrder = 1; curOrder <= orderStamp; ++curOrder) {
        // First determine all nodes in this SCC
        vector<Node::Ptr> curNodes;
	slice->allNodes(nbegin, nend);
	for (; nbegin != nend; ++nbegin) {
	    if (analysisOrder[*nbegin] == curOrder) {
	        curNodes.push_back(*nbegin);
	    }
	}

        // If this SCC does not have any outside edge,
	// it is an entry SCC and we need to connect
	// the virtual entry to it
	if (!HasIncomingEdgesFromLowerLevel(curOrder, curNodes)) {
	    if (curNodes.size() == 1) {
	        // If the SCC has only one node,
		// we connect the virtual entry to this single node
	        SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(*(curNodes.begin()));
	        slice->insertPair(virtualEntry, node, TypedSliceEdge::create(virtualEntry, node, FALLTHROUGH));
	    } else {
	        // If there are more than one node in this SCC,
		// we do a DFS to see which nodes in the SCC can be
		// reached from the entry of the function without passing
		// through other nodes in the SCC.
		// Basically, we only connect edges from the virtual entry
		// to the entries of the SCC
	        set<ParseAPI::Block*> visit;
		map<ParseAPI::Block*, vector<SliceNode::Ptr> >targetMap;
		for (auto nit = curNodes.begin(); nit != curNodes.end(); ++nit) {
		    SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(*nit);
		    ParseAPI::Block * b = node->block();
		    Address addr = node->addr();
		    if (targetMap.find(b) == targetMap.end()) {
		        targetMap[b].push_back(node);
		    } else if (targetMap[b][0]->addr() > addr) {
		        targetMap[b].clear();
			targetMap[b].push_back(node);
		    } else if (targetMap[b][0]->addr() == addr) {
		        targetMap[b].push_back(node);
		    }
		}
		BuildEdgeFromVirtualEntry(virtualEntry, virtualEntry->block(), targetMap, visit, slice);
	    }
	}
    }
    slice->clearEntryNodes();
    slice->markAsEntryNode(virtualEntry);
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
    unordered_set<Node::Ptr, Node::NodePtrHasher> inQueue;
    unordered_map<Node::Ptr, int, Node::NodePtrHasher> inQueueLimit;

    for (int curOrder = 0; curOrder <= orderStamp; ++curOrder) {
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

	if (!HasIncomingEdgesFromLowerLevel(curOrder, curNodes) && !curNodes.empty()) {
	    // If this SCC is an entry SCC,
	    // we choose a node inside the SCC
	    // and let it be top.
	    // This should only contain the virtual entry node
	    parsing_printf("This SCC does not incoming edges from outside\n");
	    boundFactsIn[curNodes[0]] = new BoundFact();
	    boundFactsOut[curNodes[0]] = new BoundFact();
	}
	parsing_printf("Starting analysis inside SCC %d\n", curOrder);
	// We now start iterative analysis inside the SCC
	while (!workingList.empty()) {
	    // We get the current node
	    Node::Ptr curNode = workingList.front();
	    workingList.pop();
	    inQueue.erase(curNode);

	    SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(curNode);
	    ++inQueueLimit[curNode];
	    if (inQueueLimit[curNode] > IN_QUEUE_LIMIT) continue;

	    BoundFact* oldFactIn = GetBoundFactIn(curNode);
	    parsing_printf("Calculate Meet for %lx", node->addr());
	    if (node->assign()) {
	        parsing_printf(", insn: %s, assignment %s\n", node->assign()->insn().format().c_str(), node->assign()->format().c_str());
	    }
	    else {
	        if (node->block() == NULL)
		    parsing_printf(", the VirtualExit node\n");
		else
		    parsing_printf(", the VirtualEntry node\n");

	    }
	    parsing_printf("\tOld fact for %lx:\n", node->addr());
	    if (oldFactIn == NULL) parsing_printf("\t\t do not exist\n"); else oldFactIn->Print();

	    // We find all predecessors of the current node
	    // and calculates the union of the analysis results
	    // from the predecessors
	    BoundFact* newFactIn = Meet(curNode);
	    parsing_printf("\tNew fact at %lx\n", node->addr());
	    if (newFactIn != NULL) newFactIn->Print(); else parsing_printf("\t\tNot calculated\n");

	    // If the current node has not been calcualted yet,
	    // or the new meet results are different from the
	    // old ones, we keep the new results
	    if (newFactIn != NULL && (oldFactIn == NULL || *oldFactIn != *newFactIn)) {
	        parsing_printf("\tFacts change!\n");
		if (oldFactIn != NULL) delete oldFactIn;
		boundFactsIn[curNode] = newFactIn;
		BoundFact* newFactOut = new BoundFact(*newFactIn);

		// The current node has a transfer function
		// that changes the analysis results
		if (!slice->isExitNode(curNode))
		    CalcTransferFunction(curNode, newFactOut);

		if (boundFactsOut.find(curNode) != boundFactsOut.end() && boundFactsOut[curNode] != NULL)
		    delete boundFactsOut[curNode];
		boundFactsOut[curNode] = newFactOut;
		curNode->outs(nbegin, nend);
	        for (; nbegin != nend; ++nbegin)
		    // We only add node inside current SCC into the working list
		    if (inQueue.find(*nbegin) == inQueue.end() && analysisOrder[*nbegin] == curOrder) {
		        workingList.push(*nbegin);
			inQueue.insert(*nbegin);
		    }
	    } else {
	        if (newFactIn != NULL) delete newFactIn;
		parsing_printf("\tFacts do not change!\n");
	    }

        }
    }

    return true;
}

static bool IsConditionalJump(Instruction insn) {
    entryID id = insn.getOperation().getID();
    if (id == e_je || id == e_jne ||
        id == e_jb || id == e_jae ||
	id == e_jbe || id == e_ja ||
	id == e_jb_jnaej_j || id == e_jnb_jae_j ||
	id == e_jle || id == e_jl ||
	id == e_jge || id == e_jg) return true;
    if (id == aarch64_op_b_cond) return true;
    if (id == power_op_bc || id == power_op_bcctr || id == power_op_bclr) return true;
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
	BoundFact *prevFact = GetBoundFactOut(srcNode);
	bool newCopy = false;
	if (prevFact == NULL) {
	    parsing_printf("\t\tIncoming node %lx has not been calculated yet, ignore it\n", srcNode->addr());
	    continue;
	} else {
	    // Otherwise, create a new copy.
	    // We do not want to overwrite the bound fact
	    // of the predecessor
	    prevFact = new BoundFact(*prevFact);
		newCopy = true;
	}
	parsing_printf("\t\tMeet incoming edge from %lx\n", srcNode->addr());
	parsing_printf("\t\tThe fact from %lx before applying transfer function\n", srcNode->addr());
	prevFact->Print();
	if (!srcNode->assign()) {
	    parsing_printf("\t\tThe predecessor node is the virtual entry ndoe\n");
	    if (firstBlock && handleOneByteRead) {
	        // If the indirect jump is in the entry block
	        // of the function, we assume that rax is in
	        // range [0,8] for analyzing the movaps table.
	        // NEED TO HAVE A SAFE WAY TO DO THIS!!
	        parsing_printf("\t\tApplying entry block rax assumption!\n");
	        AST::Ptr axAST;
	        if (func->entry()->obj()->cs()->getAddressWidth() == 8)
	            axAST = VariableAST::create(Variable(AbsRegion(Absloc(x86_64::rax))));
	        else
	            // DOES THIS REALLY SHOW UP IN 32-BIT CODE???
	            axAST = VariableAST::create(Variable(AbsRegion(Absloc(x86::eax))));
	        prevFact->GenFact(axAST, new StridedInterval(1,0,8), false);
	    }
	} else if (srcNode->assign() && IsConditionalJump(srcNode->assign()->insn())) {
	    // If the predecessor is a conditional jump,
	    // we can determine bound fact based on the predicate and the edge type
  	    parsing_printf("\t\tThe predecessor node is a conditional jump!\n");
	    if (!prevFact->ConditionalJumpBound(srcNode->assign()->insn(), edge->type())) {
	        parsing_printf("From %lx to %lx\n", srcNode->addr(), node->addr());
	    }
	}

	parsing_printf("\t\tFact from %lx after applying transfer function\n", srcNode->addr());
	prevFact->Print();
        if (first) {
	    // For the first incoming dataflow fact,
	    // we just copy it.
	    // We can do this because if an a-loc
	    // is missing in the fact map, we assume
	    // the a-loc is top.
	    first = false;
	    if (newCopy) newFact = prevFact; else newFact = new BoundFact(*prevFact);
	} else {
	    newFact->Meet(*prevFact);
	    if (newCopy) delete prevFact;
        }
    }
    return newFact;
}

void BoundFactsCalculator::CalcTransferFunction(Node::Ptr curNode, BoundFact *newFact){
    SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(curNode);
    if (!node->assign()) return;
    if (node->assign() && 
        node->assign()->out().absloc().type() == Absloc::Register &&
	node->assign()->out().absloc().reg().isZeroFlag()) {
	    // zf should be only predecessor of this node
        parsing_printf("\t\tThe predecessor node is zf assignment!\n");
	newFact->SetPredicate(node->assign(), se.ExpandAssignment(node->assign()) );
	return;
    }
    entryID id = node->assign()->insn().getOperation().getID();
    // The predecessor is not a conditional jump,
    // then we can determine buond fact based on the src assignment
    parsing_printf("\t\tThe predecessor node is normal node\n");
    parsing_printf("\t\t\tentry id %u\n", id);

    AbsRegion &ar = node->assign()->out();
    Instruction insn = node->assign()->insn();
    pair<AST::Ptr, bool> expandRet = se.ExpandAssignment(node->assign());

    if (expandRet.first == NULL) {
        parsing_printf("\t\t\t No semantic support for this instruction. Assume it does not affect jump target calculation. Ignore it (Treat as identity function) except for ptest. ptest should kill the current predicate\n");
	if (id == e_ptest) {
	    parsing_printf("\t\t\t\tptest instruction, kill predciate.\n");
	    newFact->pred.valid = false;
	}
	return;
    } else {
        parsing_printf("\tAST: %s\n", expandRet.first->format().c_str());
    }

    AST::Ptr calculation = expandRet.first;
    int derefSize = 0;
    if (node->assign() && node->assign()->insn().isValid() && node->assign()->insn().readsMemory()) {
        Instruction i = node->assign()->insn();
	std::vector<Operand> ops;
	i.getOperands(ops);
	for (auto oit = ops.begin(); oit != ops.end(); ++oit) {
	    Operand o = *oit;
	    if (o.readsMemory()) {
	        Expression::Ptr exp = o.getValue();
		derefSize = exp->size();
		break;
	    }
	}

    }
    BoundCalcVisitor bcv(*newFact, node->block(), handleOneByteRead, derefSize);
    calculation->accept(&bcv);
    AST::Ptr outAST;
    // If the instruction writes memory,
    // we need the AST that represents the memory access and the address.
    // When the AbsRegion represents memory,
    // the generator of the AbsRegion is set to be the AST that represents
    // the memory address during symbolic expansion.
    // In other cases, if the AbsRegion represents a register,
    // the generator is not set.
    if (ar.generator() != NULL)
        outAST = se.SimplifyAnAST(
	                       RoseAST::create(ROSEOperation(ROSEOperation::derefOp, ar.size()), ar.generator()), 
	                       SymbolicExpression::PCValue(node->assign()->addr(), 
			               insn.size(),
				       node->assign()->block()->obj()->cs()->getArch()));

    else
        outAST = VariableAST::create(Variable(ar));
/*
 * Naively, bsf and bsr produces a bound from 0 to the number of bits of the source operands.
 * In pratice, especially in libc, the real bound is usually smaller than the size of the source operand.
 * Ex 1: shl    %cl,%edx
 *       bsf    %rdx,%rcx
 * Here rcx is in range [0,31] rather than [0,63] even though rdx has 64 bits.
 *
 * Ex 2: pmovmskb %xmm0,%edx
 *       bsf    %rdx, %rdx
 * Here rdx is in range[0,15] because pmovmskb only sets the least significat 16 bits
 * In addition, overapproximation of the bound can lead to bogus control flow
 * that causes overlapping blocks or function.
 * It is important to further anaylze the operand in bsf rather than directly conclude the bound
 */
    if (id == e_bsf || id == e_bsr) {
	int size = node->assign()->insn().getOperand(0).getValue()->size();
	newFact->GenFact(outAST, new StridedInterval(StridedInterval(1,0, size * 8 - 1)), false);
        parsing_printf("\t\t\tCalculating transfer function: Output facts\n");
	newFact->Print();
	return;

    }
    if (id == e_xchg) {
        newFact->SwapFact(calculation, outAST);
        parsing_printf("\t\t\tCalculating transfer function: Output facts\n");
	newFact->Print();
	return;
    }

    if (id == e_push) {
         if (calculation->getID() == AST::V_ConstantAST) {
	     ConstantAST::Ptr c = boost::static_pointer_cast<ConstantAST>(calculation);
	     newFact->PushAConst(c->val().val);
	     parsing_printf("\t\t\tCalculating transfer function: Output facts\n");
	     newFact->Print();
	     return;
	 }
    }

    if (id == e_pop) {
        if (newFact->PopAConst(outAST)) {
	     parsing_printf("\t\t\tCalculating transfer function: Output facts\n");
	     newFact->Print();
	     return;
        }
    }

    // Assume all SETxx entry ids are contiguous
    if (id >= e_setb && id <= e_sete) {
        newFact->GenFact(outAST, new StridedInterval(1,0,1), false);
	parsing_printf("\t\t\tCalculating transfer function: Output facts\n");
	newFact->Print();
	return;
    }

    bool findBound = false;
    if (bcv.IsResultBounded(calculation)) {
        findBound = true;
        parsing_printf("\t\t\tGenerate bound fact for %s\n", outAST->format().c_str());
	newFact->GenFact(outAST, new StridedInterval(*bcv.GetResultBound(calculation)), false);
    }
    else {
        parsing_printf("\t\t\tKill bound fact for %s\n", outAST->format().c_str());
	newFact->KillFact(outAST, false);
    }
    if (calculation->getID() == AST::V_VariableAST) {
        // We only track alising between registers
	parsing_printf("\t\t\t%s and %s are equal\n", calculation->format().c_str(), outAST->format().c_str());
	newFact->InsertRelation(calculation, outAST, BoundFact::Equal);
    }

    newFact->AdjustPredicate(outAST, calculation);

    // Now try to track all aliasing.
    // Currently, all variables in the slice are presented as an AST
    // consists of input variables to the slice (the variables that
    // we do not know the sources of their values).
    newFact->TrackAlias(SymbolicExpression::DeepCopyAnAST(calculation), outAST, findBound);

    // Apply tracking relations to the calculation to generate a
    // potentially stricter bound
    StridedInterval *strictValue = newFact->ApplyRelations(outAST);
    if (strictValue != NULL) {
        parsing_printf("\t\t\tGenerate stricter bound fact for %s\n", outAST->format().c_str());
	newFact->GenFact(outAST, strictValue, false);
    }
    // Apply tracking relations to the calculation for expression like
    // r8 - ((r8 >> 4) << 4), which guarantees that 
    // r8 is in [0, 2^4-1]
    strictValue = newFact->ApplyRelations2(outAST);
    if (strictValue != NULL) {
        parsing_printf("\t\t\tGenerate stricter bound fact for %s\n", outAST->format().c_str());
	newFact->GenFact(outAST, strictValue, false);
    }
    parsing_printf("\t\t\tCalculating transfer function: Output facts\n");
    newFact->Print();

}

BoundFact* BoundFactsCalculator::GetBoundFactIn(Node::Ptr node) {
    auto fit = boundFactsIn.find(node);
    if (fit == boundFactsIn.end())
        return NULL;
    else
        return fit->second;
}
BoundFact* BoundFactsCalculator::GetBoundFactOut(Node::Ptr node) {
    auto fit = boundFactsOut.find(node);
    if (fit == boundFactsOut.end())
        return NULL;
    else
        return fit->second;
}
BoundFactsCalculator::~BoundFactsCalculator() {
    for (auto fit = boundFactsIn.begin(); fit != boundFactsIn.end(); ++fit)
        if (fit->second != NULL)
	    delete fit->second;
    boundFactsIn.clear();
    for (auto fit = boundFactsOut.begin(); fit != boundFactsOut.end(); ++fit)
        if (fit->second != NULL)
	    delete fit->second;
    boundFactsOut.clear();

}


