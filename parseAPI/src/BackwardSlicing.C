#include "dyntypes.h"
#include "Node.h"
#include "Graph.h"

#include "debug_parse.h"
#include "CodeObject.h"
#include "BackwardSlicing.h"

#include "Instruction.h"
#include "InstructionDecoder.h"

#include "AbslocInterface.h"
#include "SymEval.h"

using namespace Dyninst;
using namespace Dyninst::DataflowAPI;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InstructionAPI;

inline static void BuildAnEdge(SliceNode::Ptr srcNode,
                               SliceNode::Ptr trgNode,
			       map<AssignmentPtr, SliceNode::Ptr> &nodeMap,
			       GraphPtr newG,
			       EdgeTypeEnum t) {
    SliceNode::Ptr newSrcNode = nodeMap[srcNode->assign()];
    SliceNode::Ptr newTrgNode = nodeMap[trgNode->assign()];
    newG->insertPair(newSrcNode, newTrgNode, TypedSliceEdge::create(newSrcNode, newTrgNode, t));
}			       
		       

static void BuildEdgesAux(SliceNode::Ptr srcNode,
                          ParseAPI::Block* curBlock,
			  map<AssignmentPtr, SliceNode::Ptr> &nodeMap,
			  map<ParseAPI::Block*, map<AssignmentPtr, SliceNode::Ptr> > &targetMap,
			  GraphPtr newG,
			  set<ParseAPI::Block*> &visit,
			  EdgeTypeEnum t) {			 
    if (targetMap.find(curBlock) != targetMap.end()) {
        // This block contains at least one silce node 
	// that is reachable from the source DFS node
        map<AssignmentPtr, SliceNode::Ptr> &candNodes = targetMap[curBlock];
	Address addr = 0;
	SliceNode::Ptr trgNode;
	for (auto cit = candNodes.begin(); cit != candNodes.end(); ++cit)
	    // The node has to be either in a different block from the source node
	    // or in the same block but has a larger address to be considered 
	    // reachable from the source node
	    if (cit->first->addr() > srcNode->addr() || curBlock != srcNode->block())
	        if (addr == 0 || addr > cit->first->addr()) {
		    addr = cit->first->addr();
		    trgNode = cit->second;
		}
	if (addr != 0) {
	    // There may be several assignments locating 
	    // at the same address. Need to connecting all.
	    if (t == _edgetype_end_) t = FALLTHROUGH;
	    for (auto cit = candNodes.begin(); cit != candNodes.end(); ++cit)
	        if (cit->first->addr() > srcNode->addr() || curBlock != srcNode->block())
		    if (addr == cit->first->addr()) 
		        BuildAnEdge(srcNode, cit->second, nodeMap, newG, t);
	    return;
	}
    }

    if (visit.find(curBlock) != visit.end()) return;
    visit.insert(curBlock);
    for (auto eit = curBlock->targets().begin(); eit != curBlock->targets().end(); ++eit)
	// Xiaozhu:
	// Our current slicing code ignores tail calls 
	// (the slice code only checks if an edge type is CALL or not)
 	// so, I should be consistent here.
	// If the slice code considers tail calls, need to change
	// the predicate to (*eit)->interproc()
        if ((*eit)->type() != CALL && (*eit)->type() != RET) {
	    EdgeTypeEnum newT = t; 
	    if (t == _edgetype_end_) {
	        if ((*eit)->type() == COND_TAKEN || (*eit)->type() == COND_NOT_TAKEN) 
		    newT = (*eit)->type();
		else 
		    newT = FALLTHROUGH;
	    } 
	    BuildEdgesAux(srcNode, (*eit)->trg(), nodeMap, targetMap, newG, visit, newT);	   
	}
}			  


static void BuildEdges(SliceNode::Ptr curNode,
                       map<AssignmentPtr, SliceNode::Ptr> &nodeMap,
		       map<ParseAPI::Block*, map<AssignmentPtr, SliceNode::Ptr> > &targetMap,
		       GraphPtr newG) {
    set<ParseAPI::Block*> visit;		     
    BuildEdgesAux(curNode, curNode->block(), nodeMap, targetMap, newG, visit, _edgetype_end_);
}		       

static bool NodeIsZF(SliceNode::Ptr node) {
    return node && node->assign() && node->assign()->out().absloc().type() == Absloc::Register &&
	   (node->assign()->out().absloc().reg() == x86::zf || node->assign()->out().absloc().reg() == x86_64::zf);
}

static bool ShouldSkip(SliceNode::Ptr curNode, GraphPtr g) {
    if (NodeIsZF(curNode)) return false;
    NodeIterator gbegin, gend;
    g->allNodes(gbegin, gend);
    for (; gbegin != gend; ++gbegin) {
        SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(*gbegin);
	if (node->addr() == curNode->addr()) {
	    if (NodeIsZF(node)) return true;
	}	    
    }
    return false;
}

GraphPtr BackwardSlicer::TransformToCFG(GraphPtr gp) {
    GraphPtr newG = Graph::createGraph();
    
    NodeIterator gbegin, gend;
    gp->allNodes(gbegin, gend);
    
    // Create a AssignmentPtr to SliceNode map to maintain the new graph nodes
    map<AssignmentPtr, SliceNode::Ptr> nodeMap;

    // Create a map to help find whether we have reached a node
    map<ParseAPI::Block*, map<AssignmentPtr, SliceNode::Ptr> > targetMap;
    for (; gbegin != gend; ++gbegin) {
        SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(*gbegin);
	if (!ShouldSkip(node, gp)) {
	    SliceNode::Ptr newNode = SliceNode::create(node->assign(), node->block(), node->func());
	    nodeMap[node->assign()] = newNode;
	    targetMap[node->block()][node->assign()] = newNode;
	    newG->addNode(newNode);
	}
    }

    // Start from each node to do DFS and build edges
    gp->allNodes(gbegin, gend);
    for (; gbegin != gend; ++gbegin) {
        SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(*gbegin);
	if (!ShouldSkip(node, gp))
	    BuildEdges(node, nodeMap, targetMap, newG);
    }

    // Build a virtual exit node
    SliceNode::Ptr virtualExit = SliceNode::create(Assignment::Ptr(), NULL, NULL);
    newG->addNode(virtualExit);
    newG->allNodes(gbegin, gend);
    for (; gbegin != gend; ++gbegin) {
        SliceNode::Ptr cur = boost::static_pointer_cast<SliceNode>(*gbegin);
	if (!cur->hasOutEdges() && cur != virtualExit) {
	    newG->insertPair(cur, virtualExit, TypedSliceEdge::create(cur, virtualExit, FALLTHROUGH));
	}
    }

    AdjustGraphEntryAndExit(newG);


    return newG;

}

GraphPtr BackwardSlicer::TransformGraph(GraphPtr gp) {

    bool finish = false;
    while (!finish) {
        finish = true;
	NodeIterator gbegin, gend;
	gp->allNodes(gbegin, gend);
	for (; gbegin != gend; ++gbegin) {
	    Node::Ptr ptr = *gbegin;
	    if (gp->isExitNode(ptr)) continue;
	    SliceNode::Ptr cur = boost::static_pointer_cast<SliceNode>(ptr);
	    if (cur->assign() == NULL) {
		gp->deleteNode(*gbegin);
		finish = false;
		break;
	    }
	    if (cur->assign()->insn()->writesMemory()) {
	        gp->deleteNode(*gbegin);
		finish = false;
		break;
	    }
	}
    }
    return TransformToCFG(gp);
}

static string Classify(AST::Ptr ast) {
    if (ast->getID() == AST::V_RoseAST) {
        RoseAST::Ptr roseAST = boost::static_pointer_cast<RoseAST>(ast);
	switch (roseAST->val().op) {
	    case ROSEOperation::addOp: {
	        if (ast->child(0)->getID() == AST::V_RoseAST && ast->child(1)->getID() == AST::V_ConstantAST) {
  	            // Now check if this subexpression is actually a subtraction
	            RoseAST::Ptr child0 = boost::static_pointer_cast<RoseAST>(ast->child(0));
		    ConstantAST::Ptr child1 = boost::static_pointer_cast<ConstantAST>(ast->child(1));
		    if (child1->val().val == 1 && child0->val().op == ROSEOperation::invertOp) {
		        return " - " + Classify(child0->child(0));
		    }
		}
		string p1 = Classify(roseAST->child(0));
		string p2 = Classify(roseAST->child(1));
		if (p2[1] == '-') return p1 + p2;
		else if (p1[1] == '-') return p2 + p1;
		else if (p1 < p2) return p1 + " + " + p2;
		else return p2 + " + " + p1;
	    }
	    case ROSEOperation::sMultOp:
	    case ROSEOperation::uMultOp:
	    case ROSEOperation::shiftLOp:
	        if (roseAST->child(0)->getID() == AST::V_ConstantAST) {
		    return Classify(roseAST->child(0)) + " * " + "Index";
		}
	        if (roseAST->child(1)->getID() == AST::V_ConstantAST) {
		    return Classify(roseAST->child(1)) + " * " + "Index";
		}
		break;
	    case ROSEOperation::derefOp:
	        return "[ " + Classify(ast->child(0)) + " ]";
	    default:
	        break;
	}
	return " unknown ";
    } else if (ast->getID() == AST::V_VariableAST) {
        VariableAST::Ptr varAST = boost::static_pointer_cast<VariableAST>(ast);
	switch (varAST->val().reg.absloc().type()) {
	    case Absloc::Register: 
	        return "Reg";
	    case Absloc::Stack:
	        return "Stack";
	    case Absloc::Heap:
	        return "Heap";
	    default:
	        return "Unknown";
	}
    } else if (ast->getID() == AST::V_ConstantAST) {
        return "Imm";
    }
    return " unknown ";

}

GraphPtr BackwardSlicer::CalculateBackwardSlicing() {
    const unsigned char * buf = (const unsigned char*) block->obj()->cs()->getPtrToInstruction(addr);
    InstructionDecoder dec(buf, InstructionDecoder::maxInstructionLength, block->obj()->cs()->getArch());
    Instruction::Ptr insn = dec.decode();

    AssignmentConverter ac(true, false);
    vector<Assignment::Ptr> assignments;
    ac.convert(insn, addr, func, block, assignments);

    Slicer s(assignments[0], block, func);

    IndirectControlFlowPred mp;
    GraphPtr slice = s.backwardSlice(mp);
    

// Code for understanding characteristics of
// jump target expressions
/*
    Result_t symRet;
    SymEval::expand(slice, symRet);
    assert(symRet.find(assignments[0]) != symRet.end());    
    symRet[assignments[0]] =  SimplifyAnAST(symRet[assignments[0]], 0);
    string out = Classify(symRet[assignments[0]] );
    fprintf(stderr, "%lx: %s : %s\n", block->last(),  out.c_str(), symRet[assignments[0]]->format().c_str());
*/
// End of this piece of code

    slice = TransformGraph(slice);
/*    
     if (addr == 0x8068ffa) {
        slice->printDOT("target.dot");
	exit(0);
    }
*/   

    return slice;
}

int BackwardSlicer::AdjustGraphEntryAndExit(GraphPtr gp) {
    int nodeCount = 0;
    NodeIterator gbegin, gend;
    gp->allNodes(gbegin, gend);
    for (; gbegin != gend; ++gbegin) {
        ++nodeCount;
        Node::Ptr ptr = *gbegin;
	if (!ptr->hasInEdges()) gp->insertEntryNode(ptr);
	if (!ptr->hasOutEdges()) gp->insertExitNode(ptr);
    }
    return nodeCount;
}

bool IndirectControlFlowPred::endAtPoint(AssignmentPtr ap) {
        if (ap->insn()->writesMemory()) return true;
	return false;
}

bool IndirectControlFlowPred::followCall(ParseAPI::Function*  , CallStack_t & , AbsRegion ) {
/*
    ParseAPI::Block *b = f->entry();

    if (f->entry()->isThunk()) {
        Absloc absloc = reg.absloc();
	if (absloc.type() == Absloc::Register && absloc.reg().base() == x86::ebx) {
	    return true;
	}
    }
    */
    return false;

}

bool IndirectControlFlowPred::addPredecessor(AbsRegion reg) {
    if (reg.absloc().type() == Absloc::Register) {
        MachRegister r = reg.absloc().reg();
        return !r.isPC();
    } 
    return true;
}

