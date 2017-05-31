#include "JumpTableFormatPred.h"
#include "SymbolicExpression.h"
#include "IndirectASTVisitor.h"
#include "SymEval.h"
#include "debug_parse.h"
using namespace Dyninst;
using namespace Dyninst::DataflowAPI;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InstructionAPI;

static int CountInDegree(SliceNode::Ptr n) {
    NodeIterator nbegin, nend; 
    int count = 0;
    n->ins(nbegin, nend);
    for (; nbegin != nend; ++count, ++nbegin);
    return count;
}

bool JumpTableFormatPred::modifyCurrentFrame(Slicer::SliceFrame &frame, Graph::Ptr g) {
    if (!jumpTableFormat) return false;
    if (unknownInstruction) return false;

    /* We start to inspect the current slice graph.
     * 1. If we have determined the jump table format, we can stop this slice.
     * 2. If we have determined the index variable, we can remove the variable 
     *    from the current slice and only keep slicing on other variables.
     * 3. If we have determined that this is not a known jump table, 
     *    we also stop slicing.
     */

    queue<SliceNode::Ptr> working_list;
    unordered_set<Assignment::Ptr, Assignment::AssignmentPtrHasher> inQueue;
    NodeIterator nbegin, nend; 

    g->adjustEntryAndExitNodes();
    // We do not try to slice on the heap absregion,
    // as it will not help us determine the jump table format.
    // But we record this memory read, so that later we can determine
    // whether the size of the read and whether it is zero extended or sign extended
    for (auto rit = frame.active.begin(); rit != frame.active.end(); ++rit)
        if (rit->first.type() == Absloc::Heap || rit->first.type() == Absloc::Stack) {
	    // If this is the first memory read we encoutered,
	    // this is likely to be a jump table read and we need to keep
	    // slice on its address.
	    if (firstMemoryRead) {
	        memLoc = rit->second[0].ptr;
		firstMemoryRead = false;
		frame.active.erase(rit);
	    } else {
	        // For a later memory read, if we have not disqualified this indirect,
		// it is likely to be a jump table. This memory read is assumed 
		// and likely to be a spill for a certain register.
		// We keep slicing on the register.
		SliceNode::Ptr readNode;
		parsing_printf("\t\tfind another memory read %s %s\n", rit->first.format().c_str(), rit->second[0].ptr->format().c_str());
		if (!findSpillRead(g, readNode)) {
		    parsing_printf("\tWARNING: a potential memory spill cannot be handled.\n");
		    jumpTableFormat = false;
		    return false;
		}
		// We then delete all absregions introduced by this read node from the active map
		// and add back the original absregion
		adjustActiveMap(frame, readNode);
		g->deleteNode(readNode);
		return true;

	    }
	    break;
	}


    g->entryNodes(nbegin, nend);

    // This map trakcs the expanded and substituted ASTs for each assignment in the current slice
    std::unordered_map<Assignment::Ptr, AST::Ptr, Assignment::AssignmentPtrHasher> exprs;
    std::unordered_map<Assignment::Ptr, int, Assignment::AssignmentPtrHasher> inDegree;
    for (; nbegin != nend; ++nbegin) {
        SliceNode::Ptr n = boost::static_pointer_cast<SliceNode>(*nbegin);
	working_list.push(n);
	inQueue.insert(n->assign());
    }
    
    g->allNodes(nbegin, nend);
    for (; nbegin != nend; ++nbegin) {
        SliceNode::Ptr n = boost::static_pointer_cast<SliceNode>(*nbegin);
	inDegree[n->assign()] = CountInDegree(n);
    }
    AST::Ptr jumpTarget;
    while (!working_list.empty()) {
        SliceNode::Ptr n = working_list.front();
	working_list.pop();
	if (!n->assign()) {
	    parsing_printf("\tWARNING: Encountering a slice node with no assignment!\n");
	    continue;
	}
	if (exprs.find(n->assign()) != exprs.end()) {
	    parsing_printf("\tWARNING: Jump table format slice contains cycle!\n");
	    jumpTableFormat = false;
	    return false;
	}

	/* We expand this assignment.
	 * The jump table format should only involve basic instructions
	 * such as mov, arithmetic, loading addresses.
	 * If we encounter an instruction that we do not have semantics,
	 * we should inspect this case.
	 */
	pair<AST::Ptr, bool> expandRet = se.ExpandAssignment(n->assign());
	if (!expandRet.second || expandRet.first == NULL) {
	    parsing_printf("\tWARNING: Jump table format slice contains unknown instructions: %s\n", n->assign()->insn()->format().c_str());
	    unknownInstruction = true;
	    jumpTableFormat = false;
	    return false;
	}
	if (n->assign()->out().generator() != NULL) {
	    parsing_printf("\tWARNING: Jump table format slice contains writes to memory\n");
	    jumpTableFormat = false;
	    return false;
	}

	// We make a deep copy of the AST because the AST from ExpandAssignment 
	// may be used later. So, we do not want to destroy the AST of the assignment
	AST::Ptr exp = SymbolicExpression::DeepCopyAnAST(expandRet.first);
	// We start plug in ASTs from predecessors
	n->ins(nbegin, nend);
	map<AST::Ptr, AST::Ptr> inputs;
	for (; nbegin != nend; ++nbegin) {
	    SliceNode::Ptr p = boost::static_pointer_cast<SliceNode>(*nbegin);
	    if (exprs.find(p->assign()) == exprs.end()) {
	        parsing_printf("\tWARNING: For %s, its predecessor %s does not have an expression\n", n->assign()->format().c_str(), p->assign()->format().c_str());
		jumpTableFormat = false;
		return false;
	    }
	    AST::Ptr rhs = exprs[p->assign()];	    
	    AST::Ptr lhs = VariableAST::create(Variable(p->assign()->out())); 
	    // TODO: there may be more than one expression for a single variable
	    inputs.insert(make_pair(lhs,  rhs));
	}
	if (g->isExitNode(n)) {
	    // Here we try to detect the case where there are multiple
	    // paths to the indirect jump, and on some of the paths, the jump
	    // target has constnt values, and on some other path, the jump target
	    // may have a jump table format
	    int nonConstant = 0;
	    int match = 0;
	    for (auto iit = inputs.begin(); iit != inputs.end(); ++iit) {
	        AST::Ptr lhs = iit->first;
		AST::Ptr rhs = iit->second;
	        if (*lhs == *exp) {
		    match++;
		    if (rhs->getID() == AST::V_ConstantAST) {
		        ConstantAST::Ptr c = boost::static_pointer_cast<ConstantAST>(rhs);
			constAddr.insert(c->val().val);
		    } else {
		        nonConstant++;
			jumpTarget = rhs;
		    }
		}
	    }
	    if (match == 0) {
	        // Thiw will happen when the indirect jump directly reads from memory,
		// instead of jumping to the value of a register.
		exp = SymbolicExpression::SubstituteAnAST(exp, inputs);
	        jumpTarget = exp;
		break;
	    }
	    if (nonConstant > 1) {
	        parsing_printf("Find %d different jump target formats\n", nonConstant);
		jumpTableFormat = false;
		return false;
	    } else if (nonConstant == 0) {
	        parsing_printf("Only constant target values found so far, no need to check jump target format\n");
		return true;
	    }
	    break;
	}
	// TODO: need to consider thunk
	exp = SymbolicExpression::SubstituteAnAST(exp, inputs);
	exprs[n->assign()] = exp;
        // Enumerate every successor and add them to the working list
	n->outs(nbegin, nend);
	for (; nbegin != nend; ++nbegin) {
	    SliceNode::Ptr p = boost::static_pointer_cast<SliceNode>(*nbegin);
	    inDegree[p->assign()] --;
	    if (inDegree[p->assign()] == 0 && inQueue.find(p->assign()) == inQueue.end()) {
	        inQueue.insert(p->assign());
		working_list.push(p);
	    }
	}
    }
    if (!jumpTarget) {
        parsing_printf("\t Do not find a potential jump target expression\n");
	jumpTableFormat = false;
	return false;

    }
    JumpTableFormatVisitor jtfv(block);
    assert(jumpTarget);
    jumpTarget = SymbolicExpression::SimplifyAnAST(jumpTarget, 0);
    parsing_printf("Check expression %s\n", jumpTarget->format().c_str());    
    jumpTarget->accept(&jtfv);
    if (jtfv.findIncorrectFormat) {
        jumpTableFormat = false;
	return false;
    }
    
    if (!findIndex && jtfv.findIndex) {
	if (frame.active.find(jtfv.index) == frame.active.end()) {
	    parsing_printf("\tWARNING: found index variable %s, but it is not in the active map of the slice frame!\n", index.format().c_str());
	    jumpTableFormat = false;
	    return false;
	}
	if (frame.active[jtfv.index].size() > 1) {
	    parsing_printf("\tWARNING: index variable has more than one slicing element!\n");
	}
	indexLoc = frame.active[jtfv.index][0].ptr;
	// We have found the index variable.
	// Now we leave it alone and let the jump table index slice to find its bound
	frame.active.erase(jtfv.index);
	index = jtfv.index;
	findIndex = true;
    }
    if (jtfv.findIndex && jtfv.findTableBase) { 
        parsing_printf("\tRecord jump target expr\n");
        jumpTargetExpr = jumpTarget;
        return false;
    }

    // We have not found all elements of the jump table,
    // and we have not rejected this indirect jump as a jump table.
    // Let's continue slicing.
    return true;
}

string JumpTableFormatPred::format() {
    if (jumpTargetExpr) return jumpTargetExpr->format();
    return string("");
}

bool JumpTableFormatPred::findSpillRead(Graph::Ptr g, SliceNode::Ptr &readNode) {
    NodeIterator gbegin, gend;
    g->allNodes(gbegin, gend);
    for (; gbegin != gend; ++gbegin) {
        SliceNode::Ptr n = boost::static_pointer_cast<SliceNode>(*gbegin);
	if (n->assign() == memLoc) {
	    continue;
	}
	if (n->assign()->insn()->readsMemory()) {
	    readNode = n;
	    return true;
	}
    }
    return false;
}

void JumpTableFormatPred::adjustActiveMap(Slicer::SliceFrame &frame, SliceNode::Ptr n) {
    std::vector<AbsRegion>& inputs = n->assign()->inputs();
    for (auto iit = inputs.begin(); iit != inputs.end(); ++iit) {
        parsing_printf("\tdelete %s from active map\n", iit->format().c_str());
        frame.active.erase(*iit);
    }

    NodeIterator nbegin, nend;
    n->outs(nbegin, nend);
    parsing_printf("\tadd %s to active map\n", n->assign()->out().format().c_str());
    for (; nbegin != nend; ++nbegin) {
        SliceNode::Ptr next = boost::static_pointer_cast<SliceNode>(*nbegin);
	frame.active[n->assign()->out()].push_back(Slicer::Element(next->block(), next->func(), n->assign()->out(), next->assign()));
    }
}


