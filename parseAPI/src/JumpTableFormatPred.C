#include "JumpTableFormatPred.h"
#include "SymbolicExpression.h"
#include "IndirectASTVisitor.h"
#include "SymEval.h"
#include "debug_parse.h"
using namespace Dyninst;
using namespace Dyninst::DataflowAPI;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InstructionAPI;

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
    g->entryNodes(nbegin, nend);

    // This map trakcs the expanded and substituted ASTs for each assignment in the current slice
    std::unordered_map<Assignment::Ptr, AST::Ptr, Assignment::AssignmentPtrHasher> exprs;

    for (; nbegin != nend; ++nbegin) {
        SliceNode::Ptr n = boost::static_pointer_cast<SliceNode>(*nbegin);
	working_list.push(n);
	inQueue.insert(n->assign());
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
	        parsing_printf("\tWARNING: predecessor does not have an expression\n");
		jumpTableFormat = false;
		return false;
	    }
	    AST::Ptr rhs = exprs[p->assign()];	    
	    AST::Ptr lhs = VariableAST::create(Variable(p->assign()->out())); 
	    
	    // TODO: there may be more than one expression for a single variable
	    inputs[lhs] = rhs;
	}
	// TODO: need to consider thunk
	exp = SymbolicExpression::SubstituteAnAST(exp, inputs);
	exprs[n->assign()] = exp;
        // Enumerate every successor and add them to the working list
	n->outs(nbegin, nend);
	for (; nbegin != nend; ++nbegin) {
	    SliceNode::Ptr p = boost::static_pointer_cast<SliceNode>(*nbegin);
	    if (inQueue.find(p->assign()) == inQueue.end()) {
	        inQueue.insert(p->assign());
		working_list.push(p);
	    }
	}

	// The last expression should be the jump target
	jumpTarget = exp;
    }
    JumpTableFormatVisitor jtfv(block);
    jumpTarget->accept(&jtfv);
    if (jtfv.findIncorrectFormat) {
        jumpTableFormat = false;
	return false;
    }
    
    // We do not try to slice on the heap absregion,
    // as it will not help us determine the jump table format.
    for (auto rit = frame.active.begin(); rit != frame.active.end(); ++rit)
        if (rit->first.type() == Absloc::Heap) {
	    frame.active.erase(rit);
	    break;
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
