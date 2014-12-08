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
    
    slice = TransformGraph(slice);

/*
    if (addr == 0x430ec3) {
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

