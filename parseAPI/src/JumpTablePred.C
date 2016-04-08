#include "dyntypes.h"
#include "Node.h"
#include "Graph.h"

#include "debug_parse.h"
#include "CodeObject.h"
#include "JumpTablePred.h"
#include "IndirectASTVisitor.h"

#include "Instruction.h"
#include "InstructionDecoder.h"

#include "AbslocInterface.h"
#include "SymEval.h"

using namespace Dyninst;
using namespace Dyninst::DataflowAPI;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InstructionAPI;
#define SIGNEX_64_32 0xffffffff00000000LL
#define SIGNEX_64_16 0xffffffffffff0000LL
#define SIGNEX_64_8  0xffffffffffffff00LL
#define SIGNEX_32_16 0xffff0000
#define SIGNEX_32_8 0xffffff00

// Assume the table contain less than this many entries.
#define MAX_TABLE_ENTRY 1000000

static void BuildEdgesAux(SliceNode::Ptr srcNode,
                          ParseAPI::Block* curBlock,
			  map<ParseAPI::Block*, map<AssignmentPtr, SliceNode::Ptr> > &targetMap,
			  GraphPtr newG,
			  set<ParseAPI::Block*> &visit,
			  EdgeTypeEnum t,
			  set<ParseAPI::Edge*> allowedEdges) {			 
    if (targetMap.find(curBlock) != targetMap.end()) {
        // This block contains at least one silce node 
	// that is reachable from the source DFS node
        map<AssignmentPtr, SliceNode::Ptr> &candNodes = targetMap[curBlock];
	Address addr = 0;
	for (auto cit = candNodes.begin(); cit != candNodes.end(); ++cit)
	    // The node has to be either in a different block from the source node
	    // or in the same block but has a larger address to be considered 
	    // reachable from the source node
	    if (cit->first->addr() > srcNode->addr() || curBlock != srcNode->block())
	        if (addr == 0 || addr > cit->first->addr()) {
		    addr = cit->first->addr();
		}
	if (addr != 0) {
	    // There may be several assignments locating 
	    // at the same address. Need to connecting all.
	    if (t == _edgetype_end_) t = FALLTHROUGH;
	    for (auto cit = candNodes.begin(); cit != candNodes.end(); ++cit)
	        if (cit->first->addr() > srcNode->addr() || curBlock != srcNode->block())
		    if (addr == cit->first->addr()) {
		        newG->insertPair(srcNode, cit->second, TypedSliceEdge::create(srcNode, cit->second, t));
		    }
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
        if ((*eit)->type() != CALL && (*eit)->type() != RET && allowedEdges.find(*eit) != allowedEdges.end()) {
	    EdgeTypeEnum newT = t; 
	    if (t == _edgetype_end_) {
	        if ((*eit)->type() == COND_TAKEN || (*eit)->type() == COND_NOT_TAKEN) 
		    newT = (*eit)->type();
		else 
		    newT = FALLTHROUGH;
	    } 
	    BuildEdgesAux(srcNode, (*eit)->trg(), targetMap, newG, visit, newT, allowedEdges);	   
	}
}			  


static void BuildEdges(SliceNode::Ptr curNode,
		       map<ParseAPI::Block*, map<AssignmentPtr, SliceNode::Ptr> > &targetMap,
		       GraphPtr newG,
		       set<ParseAPI::Edge*> &allowedEdges) {
    set<ParseAPI::Block*> visit;		     
    BuildEdgesAux(curNode, curNode->block(), targetMap, newG, visit, _edgetype_end_, allowedEdges);
}		       

static bool AssignIsZF(Assignment::Ptr a) {
    return a->out().absloc().type() == Absloc::Register &&
	   (a->out().absloc().reg() == x86::zf || a->out().absloc().reg() == x86_64::zf);
}

static bool IsPushAndChangeSP(Assignment::Ptr a) {
    entryID id = a->insn()->getOperation().getID();
    if (id != e_push) return false;
    Absloc aloc = a->out().absloc();
    if (aloc.type() == Absloc::Register && aloc.reg().isStackPointer()) return true;
    return false;;

}

static int AdjustGraphEntryAndExit(GraphPtr gp) {
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


GraphPtr JumpTablePred::BuildAnalysisGraph(set<ParseAPI::Edge*> &visitedEdges) {
    GraphPtr newG = Graph::createGraph();
    
    NodeIterator gbegin, gend;
    
    // Create a map to help find whether we have reached a node
    map<ParseAPI::Block*, map<AssignmentPtr, SliceNode::Ptr> > targetMap;

    // Assignments that are at these addresses have flag assignment colocated
    set<Address> shouldSkip;
    for (auto ait = currentAssigns.begin(); ait != currentAssigns.end(); ++ait) {
	if (AssignIsZF(*ait))
	    shouldSkip.insert((*ait)->addr());
    }
    // We only need one assignment from xchg instruction at each address
    set<Address> xchgCount;
    set<Assignment::Ptr> xchgAssign;
    for (auto ait = currentAssigns.begin(); ait != currentAssigns.end(); ++ait) {
        if ((*ait)->insn()->getOperation().getID() == e_xchg) {
	    if (xchgCount.find( (*ait)->addr() ) != xchgCount.end() ) continue;
	    xchgCount.insert((*ait)->addr());
	    xchgAssign.insert(*ait);
	}
    }
    
    for (auto ait = currentAssigns.begin(); ait != currentAssigns.end(); ++ait) {
        Assignment::Ptr a = *ait;
	if (   (AssignIsZF(a) || shouldSkip.find(a->addr()) == shouldSkip.end()) 
	    && !IsPushAndChangeSP(a)
	    && (!a->insn()->writesMemory() || MatchReadAST(a))) {
	    if (a->insn()->getOperation().getID() == e_xchg && xchgAssign.find(a) == xchgAssign.end()) continue;
	    SliceNode::Ptr newNode = SliceNode::create(a, a->block(), a->func());
	    targetMap[a->block()][a] = newNode;
	    newG->addNode(newNode);
	}
    }

    // Start from each node to do DFS and build edges
    newG->allNodes(gbegin, gend);
    for (; gbegin != gend; ++gbegin) {
        SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(*gbegin);
	BuildEdges(node, targetMap, newG, visitedEdges);
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


bool JumpTablePred::addNodeCallback(AssignmentPtr ap, set<ParseAPI::Edge*> &visitedEdges) {
    if (currentAssigns.find(ap) != currentAssigns.end()) return true;
    if (currentAssigns.size() > 30) return false; 
    // For flags, we only analyze zf
    if (ap->out().absloc().type() == Absloc::Register && ap->out().absloc().reg().regClass() == (unsigned int)x86::FLAG &&
       ap->out().absloc().reg() != x86::zf && ap->out().absloc().reg() != x86_64::zf) {
	return true;
    }

    pair<AST::Ptr, bool> expandRet = ExpandAssignment(ap);

    currentAssigns.insert(ap);

    parsing_printf("Adding assignment %s in instruction %s at %lx, total %d\n", ap->format().c_str(), ap->insn()->format().c_str(), ap->addr(), currentAssigns.size());

    if (!expandRet.second || expandRet.first == NULL) return true;

    // If this assignment writes memory,
    // we only want to analyze it when it writes to 
    // an AST we have seen before and potentially
    // can used for aliasing
    if (ap->insn()->writesMemory()) {
        if (!MatchReadAST(ap)) return true;
    }

    // If this assignment reads memory,
    // we record the AST of the read so
    // that in the future we can match a
    // corresponding write to identify aliasing
    if (ap->insn()->readsMemory() && expandRet.first->getID() == AST::V_RoseAST) {
        RoseAST::Ptr roseAST = boost::static_pointer_cast<RoseAST>(expandRet.first);
	if (roseAST->val().op == ROSEOperation::derefOp) {
	    readAST.push_back(expandRet.first);
	}
    }
    // I really do not want to redo the analysis from scratch,
    // but it seems like with newly added edges and nodes,
    // it is necessary to redo.

    // We create the CFG based on the found nodes
    GraphPtr g = BuildAnalysisGraph(visitedEdges);

    BoundFactsCalculator bfc(func, g, func->entry() == block, rf, thunks, block->last(), false, expandCache);
    bfc.CalculateBoundedFacts();

    BoundValue target;
    bool ijt = IsJumpTable(g, bfc, target);
    if (ijt) {
        bool ret = !FillInOutEdges(target, outEdges) || outEdges.empty();
	// Now we have stopped slicing in advance, so the cache contents are not complete any more.
	if (!ret) setClearCache(true);
        return ret;
    } else {
        return true;
    }	



}
bool JumpTablePred::FillInOutEdges(BoundValue &target, 
                                                 vector<pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges) {
    outEdges.clear();
    Address tableBase = target.interval.low;
    Address tableLastEntry = target.interval.high;
    Architecture arch = block->obj()->cs()->getArch();
    if (arch == Arch_x86) {
        tableBase &= 0xffffffff;
	tableLastEntry &= 0xffffffff;
    }

#if defined(os_windows)
    tableBase -= block->obj()->cs()->loadAddress();
    tableLastEntry -= block->obj()->cs()->loadAddress();
#endif

    parsing_printf("The final target bound fact:\n");
    target.Print();
    if (!block->obj()->cs()->isValidAddress(tableBase)) {
        parsing_printf("\ttableBase 0x%lx invalid, returning false\n", tableBase);
	return false;
    }

    for (Address tableEntry = tableBase; tableEntry <= tableLastEntry; tableEntry += target.interval.stride) {
	if (!block->obj()->cs()->isValidAddress(tableEntry)) continue;
	Address targetAddress = 0;
	if (target.tableReadSize > 0) {
	    // Assume the table contents are moved in a sign extended way;
	    switch (target.tableReadSize) {
	        case 8:
		    targetAddress = *(const uint64_t *) block->obj()->cs()->getPtrToInstruction(tableEntry);
		    break;
		case 4:
		    targetAddress = *(const uint32_t *) block->obj()->cs()->getPtrToInstruction(tableEntry);
		    if ((arch == Arch_x86_64) && (targetAddress & 0x80000000)) {
		        targetAddress |= SIGNEX_64_32;
		    }
		    break;
		case 2:
		    targetAddress = *(const uint16_t *) block->obj()->cs()->getPtrToInstruction(tableEntry);
		    if ((arch == Arch_x86_64) && (targetAddress & 0x8000)) {
		        targetAddress |= SIGNEX_64_16;
		    }
		    if ((arch == Arch_x86) && (targetAddress & 0x8000)) {
		        targetAddress |= SIGNEX_32_16;
		    }

		    break;
		case 1:
		    targetAddress = *(const uint8_t *) block->obj()->cs()->getPtrToInstruction(tableEntry);
		    if ((arch == Arch_x86_64) && (targetAddress & 0x80)) {
		        targetAddress |= SIGNEX_64_8;
		    }
		    if ((arch == Arch_x86) && (targetAddress & 0x80)) {
		        targetAddress |= SIGNEX_32_8;
		    }

		    break;

		default:
		    parsing_printf("Invalid memory read size %d\n", target.tableReadSize);
		    return false;
	    }
	    if (targetAddress != 0) {
	        if (target.isSubReadContent) 
		    targetAddress = target.targetBase - targetAddress;
		else 
		    targetAddress += target.targetBase; 

	    }
#if defined(os_windows)
            targetAddress -= block->obj()->cs()->loadAddress();
#endif
	} else targetAddress = tableEntry;

	if (block->obj()->cs()->getArch() == Arch_x86) targetAddress &= 0xffffffff;
	parsing_printf("Jumping to target %lx,", targetAddress);
	if (block->obj()->cs()->isCode(targetAddress)) {
	    outEdges.push_back(make_pair(targetAddress, INDIRECT));
	    parsing_printf(" is code.\n" );
	} else {
	    parsing_printf(" not code.\n");
	}
	// If the jump target is resolved to be a constant, 
	if (target.interval.stride == 0) break;
    }
    return true;
}
bool JumpTablePred::IsJumpTable(GraphPtr slice, 
					      BoundFactsCalculator &bfc,
					      BoundValue &target) {
    NodeIterator exitBegin, exitEnd, srcBegin, srcEnd;
    slice->exitNodes(exitBegin, exitEnd);
    SliceNode::Ptr virtualExit = boost::static_pointer_cast<SliceNode>(*exitBegin);
    virtualExit->ins(srcBegin, srcEnd);
    SliceNode::Ptr jumpNode = boost::static_pointer_cast<SliceNode>(*srcBegin);
    
    const Absloc &loc = jumpNode->assign()->out().absloc();
    parsing_printf("Checking final bound fact for %s\n",loc.format().c_str()); 
    BoundFact *bf = bfc.GetBoundFactOut(virtualExit);
    BoundValue *tarBoundValue = bf->GetBound(VariableAST::create(Variable(loc)));
    if (tarBoundValue != NULL) {
        target = *(tarBoundValue);
	uint64_t s = target.interval.size();
	if (s > 0 && s <= MAX_TABLE_ENTRY) return true;
    }
    return false;
}

bool JumpTablePred::MatchReadAST(Assignment::Ptr a) {
    pair<AST::Ptr, bool> expandRet = ExpandAssignment(a);
    if (!expandRet.second || expandRet.first == NULL) return false;
    if (a->out().generator() == NULL) return false;
    AST::Ptr write = SimplifyAnAST(RoseAST::create(ROSEOperation(ROSEOperation::derefOp, a->out().size()), a->out().generator()), a->insn()->size());

    if (write == NULL) return false;
    for (auto ait = readAST.begin(); ait != readAST.end(); ++ait) 
        if (*write == **ait) return true;
    return false;
}

pair<AST::Ptr, bool> JumpTablePred::ExpandAssignment(Assignment::Ptr assign) {
    if (expandCache.find(assign) != expandCache.end()) {
        AST::Ptr ast = expandCache[assign];
        if (ast) return make_pair(ast, true); else return make_pair(ast, false);

    } else {
		parsing_printf("\t\tExpanding instruction @ %x: %s\n", assign->addr(), assign->insn()->format().c_str());
        pair<AST::Ptr, bool> expandRet = SymEval::expand(assign, false);
	if (expandRet.second && expandRet.first) {
parsing_printf("Original expand: %s\n", expandRet.first->format().c_str());

	    AST::Ptr calculation = SimplifyAnAST(expandRet.first, assign->insn()->size());
	    expandCache[assign] = calculation;
	} else {
	    expandCache[assign] = AST::Ptr();
	}
	return make_pair( expandCache[assign], expandRet.second );
    }
}

