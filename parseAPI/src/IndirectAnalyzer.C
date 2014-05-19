#include "IndirectControlFlow.h"
#include "BackwardSlicing.h"
#include "debug_parse.h"

#include "CodeObject.h"
#include "Graph.h"

#include "Instruction.h"
#include "InstructionDecoder.h"

static bool UsePC(Instruction::Ptr insn) {
    vector<Operand> operands;
    insn->getOperands(operands);
    
    for(unsigned int i=0; i<operands.size();++i) {
        Operand & op = operands[i];
	set<RegisterAST::Ptr> regs;
	op.getReadSet(regs);
	for (auto rit = regs.begin(); rit != regs.end(); ++rit)
	    if ((*rit)->getID().isPC()) return true;

    }
    return false;
}
bool IndirectControlFlowAnalyzer::NewJumpTableAnalysis() {
    parsing_printf("Apply indirect control flow analysis at %lx\n", block->last());
    FindAllConditionalGuards();
    ReachFact rf(guards);
    parsing_printf("Calculate backward slice\n");

    BackwardSlicer bs(func, block, block->last(), guards, rf);
    GraphPtr slice =  bs.CalculateBackwardSlicing();
    
    parsing_printf("Calculate bound facts\n");     
    BoundFactsCalculator bfc(guards, func, slice, func->entry() == block, rf);
    bfc.CalculateBoundedFacts();

    bool ijt = IsJumpTable(slice, bfc);
    return ijt;
}						       



bool IndirectControlFlowAnalyzer::EndWithConditionalJump(ParseAPI::Block * b) {
/*
    const unsigned char * buf = (const unsigned char*) b->obj()->cs()->getPtrToInstruction(b->last());
    InstructionDecoder dec(buf, b->end() - b->last(), b->obj()->cs()->getArch());
    Instruction::Ptr insn = dec.decode();
    entryID id = insn->getOperation().getID();
*/    
    for (auto eit = b->targets().begin(); eit != b->targets().end(); ++eit)
        if ((*eit)->type() == COND_TAKEN) return true;
    return false;

}

void IndirectControlFlowAnalyzer::GetAllReachableBlock(set<ParseAPI::Block*> &reachable) {
    queue<Block*> q;
    q.push(block);
    while (!q.empty()) {
        ParseAPI::Block *cur = q.front();
	q.pop();
	if (reachable.find(cur) != reachable.end()) continue;
	reachable.insert(cur);
	for (auto eit = cur->sources().begin(); eit != cur->sources().end(); ++eit)
	    if ((*eit)->intraproc()) 
	        q.push((*eit)->src());
    }

}

void IndirectControlFlowAnalyzer::SaveGuardData(ParseAPI::Block *prev) {

    Address curAddr = prev->start();
    const unsigned char* buf = (const unsigned char*) prev->obj()->cs()->getPtrToInstruction(prev->start());
    InstructionDecoder dec(buf, prev->end() - prev->start(), prev->obj()->cs()->getArch());
    Instruction::Ptr insn;
    vector<pair<Instruction::Ptr, Address> > insns;
    
    while ( (insn = dec.decode()) != NULL ) {
        insns.push_back(make_pair(insn, curAddr));
	curAddr += insn->size();
    }
    
    for (auto iit = insns.rbegin(); iit != insns.rend(); ++iit) {
        insn = iit->first;
	if (insn->getOperation().getID() == e_cmp || insn->getOperation().getID() == e_test) {
	    guards.insert(GuardData(func, prev, insn, insns.rbegin()->first, iit->second, insns.rbegin()->second));
	    parsing_printf("Find guard and cmp pair: cmp %s, addr %lx, cond jump %s, addr %lx\n", insn->format().c_str(), iit->second, insns.rbegin()->first->format().c_str(), insns.rbegin()->second); 
	    break;
	}    
    }
}

void IndirectControlFlowAnalyzer::FindAllConditionalGuards(){
    set<ParseAPI::Block*> visited, reachable;
    queue<Block*> q;
    q.push(block);
    GetAllReachableBlock(reachable);

    while (!q.empty()) {
        ParseAPI::Block * cur = q.front();
	q.pop();
	if (visited.find(cur) != visited.end()) continue;
	visited.insert(cur);

        // Since a guard has the condition that one branch must always reach the indirect jump,
	// if the current block can reach a block that cannot reach the indirect jump, 
	// then all the sources of the current block is not post-dominated by the indirect jump.
	bool postDominate = true;
	for (auto eit = cur->targets().begin(); eit != cur->targets().end(); ++eit) 
	    if ((*eit)->intraproc() && (*eit)->type() != INDIRECT)
	        if (reachable.find((*eit)->trg()) == reachable.end()) postDominate = false;
	if (!postDominate) continue;

	for (auto eit = cur->sources().begin(); eit != cur->sources().end(); ++eit)
	    if ((*eit)->intraproc()) {
	        ParseAPI::Block* prev = (*eit)->src();
		if (EndWithConditionalJump(prev)) {		   
		    SaveGuardData(prev);
		}
		else {
		    q.push(prev);
		}
	    }
    }
}



bool IndirectControlFlowAnalyzer::IsJumpTable(GraphPtr slice, 
					      BoundFactsCalculator &bfc) {
    NodeIterator sbegin, send;
    slice->exitNodes(sbegin, send);
    for (; sbegin != send; ++sbegin) {
        SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(*sbegin);
	const Absloc &loc = node->assign()->out().absloc();
	parsing_printf("Checking bound fact at %lx for %s\n",node->addr(), loc.format().c_str()); 
	BoundFact &bf = bfc.GetBoundFact(*sbegin);
	if (bf.IsBounded(loc)) {
	    BoundValue val = bf.GetBound(loc);
	    if (val.tableLookup) return true;
	    if (val.tableOffset) return true;
	    if (val.type == LessThan && val.CoeBounded() && val.HasTableBase()) return true;
	}
    }
  

    return false;
}

bool IndirectControlFlowPred::endAtPoint(AssignmentPtr ap) {
        if (ap->insn()->writesMemory()) return true;
	if (UsePC(ap->insn())) return true;
	if (ap->insn()->getCategory() == c_CallInsn) return true;
	return false;
}

bool IndirectControlFlowPred::followCall(ParseAPI::Function* , CallStack_t & , AbsRegion ) {

/*        ParseAPI::Block * b = callee->entry();
	vector<pair<Instruction::Ptr, Address> > insns;

	const unsigned char * buf = (const unsigned char*) b->obj()->cs()->getPtrToInstruction(b->start());
	InstructionDecoder dec(buf, b->end() - b->start(), b->obj()->cs()->getArch());  

	Instruction::Ptr insn;
	Address curAddr = b->start();
	int num = 0;
	while ((insn = dec.decode()) != NULL) {
	    insns.push_back(make_pair(insn, curAddr));
	    curAddr += insn->size();
	    ++num;
	}

	if (num != 2) return false;

	if (insns[1].first->getCategory() != c_ReturnInsn) return false;
	if (insns[0].first->getOperation().getID() != e_mov) return false;
	printf("followCall to %lx\n", callee->addr());
*/
	return false;

}
