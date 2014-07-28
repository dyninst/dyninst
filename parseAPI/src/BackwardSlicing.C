#include "dyntypes.h"
#include "Node.h"
#include "Graph.h"

#include "debug_parse.h"
#include "CodeObject.h"
#include "BackwardSlicing.h"

#include "Instruction.h"
#include "InstructionDecoder.h"

#include "AbslocInterface.h"

using namespace Dyninst::DataflowAPI;

inline static void BuildAnEdge(SliceNode::Ptr srcNode,
                               SliceNode::Ptr trgNode,
			       map<AssignmentPtr, SliceNode::Ptr> &nodeMap,
			       GraphPtr newG) {
    SliceNode::Ptr newSrcNode = nodeMap[srcNode->assign()];
    SliceNode::Ptr newTrgNode = nodeMap[trgNode->assign()];
    newG->insertPair(newSrcNode, newTrgNode);
}			       
		       

static void BuildEdgesAux(SliceNode::Ptr srcNode,
                          ParseAPI::Block* curBlock,
			  map<AssignmentPtr, SliceNode::Ptr> &nodeMap,
			  map<ParseAPI::Block*, map<AssignmentPtr, SliceNode::Ptr> > &targetMap,
			  GraphPtr newG,
			  set<ParseAPI::Block*> &visit) {			 
    if (targetMap.find(curBlock) != targetMap.end()) {
        map<AssignmentPtr, SliceNode::Ptr> &candNodes = targetMap[curBlock];
	Address addr = 0;
	SliceNode::Ptr trgNode;
	for (auto cit = candNodes.begin(); cit != candNodes.end(); ++cit)
	    if (cit->first->addr() > srcNode->addr() || curBlock != srcNode->block())
	        if (addr == 0 || addr > cit->first->addr()) {
		    addr = cit->first->addr();
		    trgNode = cit->second;
		}
	if (addr != 0) {
	    BuildAnEdge(srcNode, trgNode, nodeMap, newG);
	    return;
	}
    }
    if (visit.find(curBlock) != visit.end()) return;
    visit.insert(curBlock);
    for (auto eit = curBlock->targets().begin(); eit != curBlock->targets().end(); ++eit)
        if ((*eit)->intraproc())
	    BuildEdgesAux(srcNode, (*eit)->trg(), nodeMap, targetMap, newG, visit);	   
}			  


static void BuildEdges(SliceNode::Ptr curNode,
                       map<AssignmentPtr, SliceNode::Ptr> &nodeMap,
		       map<ParseAPI::Block*, map<AssignmentPtr, SliceNode::Ptr> > &targetMap,
		       GraphPtr newG) {
    set<ParseAPI::Block*> visit;		     
    BuildEdgesAux(curNode, curNode->block(), nodeMap, targetMap, newG, visit);
}		       

GraphPtr BackwardSlicer::TransformToCFG(GraphPtr gp) {
    GraphPtr newG = Graph::createGraph();
    
    NodeIterator gbegin, gend;
    gp->allNodes(gbegin, gend);
    
    // Create a AssignmentPtr to SliceNode map to maintain the new graph nodes
    map<AssignmentPtr, SliceNode::Ptr> nodeMap;
    map<ParseAPI::Block*, map<AssignmentPtr, SliceNode::Ptr> > targetMap;
    for (; gbegin != gend; ++gbegin) {
        SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(*gbegin);
	SliceNode::Ptr newNode = SliceNode::create(node->assign(), node->block(), node->func());
	nodeMap[node->assign()] = newNode;
	targetMap[node->block()][node->assign()] = newNode;
    }
/*
    for (auto git = guards.begin(); git != guards.end(); ++git) {
        nodeMap[] = SliceNode::create( , git->block, func);
    }
*/
    gp->allNodes(gbegin, gend);
    for (; gbegin != gend; ++gbegin) {
        SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(*gbegin);
	BuildEdges(node, nodeMap, targetMap, newG);
    }
    newG->allNodes(gbegin, gend);
    for (; gbegin != gend; ++gbegin) {
        Node::Ptr ptr = *gbegin;
	if (!ptr->hasInEdges()) newG->insertEntryNode(ptr);
	if (!ptr->hasOutEdges()) newG->insertExitNode(ptr);
    }

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
    
    

    if (AdjustGraphEntryAndExit(gp) == 1)  
        return gp; 
    else {
        DeleteDataDependence(gp);
	if (AdjustGraphEntryAndExit(gp) == 1) return gp; 
        return TransformToCFG(gp);
    }

}

static void DumpNode(GraphPtr g) {
    NodeIterator gbegin, gend;
    g->allNodes(gbegin, gend);
    for (; gbegin != gend; ++gbegin) {
        SliceNode::Ptr cur = boost::static_pointer_cast<SliceNode>(*gbegin);
	parsing_printf(" %lx", cur->addr());
    }
    parsing_printf("\n");
}

GraphPtr BackwardSlicer::CalculateBackwardSlicing() {
    IndirectControlFlowPred mp(guards);
    
    const unsigned char * buf = (const unsigned char*) block->obj()->cs()->getPtrToInstruction(addr);
    InstructionDecoder dec(buf, InstructionDecoder::maxInstructionLength, block->obj()->cs()->getArch());
    Instruction::Ptr insn = dec.decode();

    AssignmentConverter ac(true, false);
    vector<Assignment::Ptr> assignments;
    ac.convert(insn, addr, func, block, assignments);

    Slicer::Predicates p;
    Slicer s(assignments[0], block, func);
    GraphPtr slice = s.backwardSlice(mp);
    slice = TransformGraph(slice);
//    if (addr == 0x80ade30) slice->printDOT("target.dot");
//    DumpSliceInstruction(slice);
//    return AST::Ptr();

    return slice;
}

void BackwardSlicer::DeleteDataDependence(GraphPtr gp) {
    NodeIterator nbegin, nend;
    EdgeIterator ebegin, eend, nextEdgeIt;

    gp->allNodes(nbegin, nend);

    for (; nbegin != nend; ++nbegin) {
        SliceNode::Ptr curNode = boost::static_pointer_cast<SliceNode>(*nbegin);
		curNode->outs(ebegin, eend);
		for (; ebegin != eend; ) {
			SliceNode::Ptr nextNode = boost::static_pointer_cast<SliceNode>((*ebegin)->target());
			if (PassThroughGuards(curNode, nextNode)) {
				nextEdgeIt = ebegin;
				++nextEdgeIt;
				nextNode->deleteInEdge(ebegin);
				curNode->deleteOutEdge(ebegin);
				ebegin = nextEdgeIt;
			}
			else ++ebegin;
		}
    }
    DeleteUnreachableNodes(gp);

}

bool BackwardSlicer::PassThroughGuards(SliceNode::Ptr src, SliceNode::Ptr trg) {
    ParseAPI::Block *srcBlock = src->block();
    ParseAPI::Block *trgBlock = trg->block();
    for (auto git = guards.begin(); git != guards.end(); ++git) {
        if (!git->constantBound) continue;

        ParseAPI::Block *guardBlock = git->block;
	// Note that the guardBlock and the srcBlock can be the same, 
	// but since the conditional jump will always be the last instruction
	// in the block, if they are in the same block, the src can reach the guard
	if (rf.incoming[guardBlock].find(srcBlock) == rf.incoming[guardBlock].end()) continue;
	bool pred_taken = rf.branch_taken[guardBlock].find(trgBlock) != rf.branch_taken[guardBlock].end();
	bool pred_ft = rf.branch_ft[guardBlock].find(trgBlock) != rf.branch_ft[guardBlock].end();
	// If both branches reach the trg node, then this conditional jump 
	// does not bound any variable for the trg node.
	if (pred_taken ^ pred_ft) {
	    // Here I need to figure out which branch bounds the variable and 
	    // check it is the bounded path that reaches the trg node.
	    bool boundedOnTakenBranch = git->varSubtrahend ^ git->jumpWhenNoZF;
	    if (   (boundedOnTakenBranch && pred_taken)
	        // In thic case, we jump when the variable is smaller than the constant.
		// So the condition taken path is the path that bounds the value.
	        || (!boundedOnTakenBranch && pred_ft) ) {
	        // In thic case, we jump when the variable is larger than the constant.
		// So the fallthrough path is the path that bounds the value.

                if (src->assign() != NULL) {
		    Absloc loc = src->assign()->out().absloc();
		    if (loc.type() == Absloc::Register && git->usedRegs.find(loc.reg()) != git->usedRegs.end()) {
		        parsing_printf("(%lx, %lx) pass through guard at %lx\n", src->addr(), trg->addr(), git->jmpInsnAddr);

		        return true;
		    }
		}
	    }
	}
    }
    return false;
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

static void ReverseDFS(Node::Ptr cur, set<Node::Ptr> &visited) {
    if (visited.find(cur) != visited.end()) return;
    visited.insert(cur);
    EdgeIterator ebegin, eend;
    cur->ins(ebegin, eend);
    for (; ebegin != eend; ++ebegin)
        ReverseDFS((*ebegin)->source(), visited);
}

void BackwardSlicer::DeleteUnreachableNodes(GraphPtr gp) {
    set<Node::Ptr> visited;
    NodeIterator nbegin, nend;
    gp->exitNodes(nbegin, nend);
    for (; nbegin != nend; ++nbegin) {
        SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(*nbegin);
	if (node->addr() == addr) break;
    }
    ReverseDFS(*nbegin, visited);
    bool finish = false;
    while (!finish) {
        finish = true;
	gp->allNodes(nbegin, nend);
	for (; nbegin != nend; ++nbegin)
	    if (visited.find(*nbegin) == visited.end()) {
	        gp->deleteNode(*nbegin);
		finish = false;
		break;
	    }
    }        
}

