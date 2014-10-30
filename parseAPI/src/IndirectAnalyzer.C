#include "dyntypes.h"
#include "IndirectControlFlow.h"
#include "BackwardSlicing.h"
#include "IA_IAPI.h"
#include "debug_parse.h"

#include "CodeObject.h"
#include "Graph.h"

#include "Instruction.h"
#include "InstructionDecoder.h"
#include "Register.h"

#define SIGNEX_64_32 0xffffffff00000000LL
#define SIGNEX_64_16 0xffffffffffff0000LL
#define SIGNEX_64_8  0xffffffffffffff00LL
#define SIGNEX_32_16 0xffff0000
#define SIGNEX_32_8 0xffffff00

bool IndirectControlFlowAnalyzer::FillInOutEdges(BoundValue &target, 
                                                 vector<pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges) {
#if defined(os_windows)
    target.tableBase -= block->obj()->cs()->loadAddress();
#endif
    Architecture arch = block->obj()->cs()->getArch();
    if (arch == Arch_x86) target.tableBase &= 0xffffffff;
    parsing_printf("The final target bound fact:\n");
    target.Print();

    if (!block->obj()->cs()->isValidAddress(target.tableBase)) {
        parsing_printf("\ttableBase 0x%lx invalid, returning false\n", target.tableBase);
	return false;
    }

    for (int64_t i = 0; i <= target.value; ++i) {
        Address tableEntry = target.tableBase;
	if (target.addIndexing) tableEntry += target.coe * i; else tableEntry -= target.coe * i;
	if (!block->obj()->cs()->isValidAddress(tableEntry)) continue;
	Address targetAddress = 0;
	if (target.tableLookup || target.tableOffset) {
	    // Assume the table contents are moved in a sign extended way. Fixme if not.
	    switch (target.coe) {
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
		    parsing_printf("Invalid table stride %d\n", target.coe);
		    continue;
	    }
	    if (target.tableOffset && targetAddress != 0) {
	        if (target.addOffset) targetAddress += target.targetBase; else targetAddress = target.targetBase - targetAddress;
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
    }
    return true;
}

bool IndirectControlFlowAnalyzer::NewJumpTableAnalysis(std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges) {

//    if (block->last() != 0x4e4ffb) return false;

    parsing_printf("Apply indirect control flow analysis at %lx\n", block->last());
    FindAllConditionalGuards();
    FindAllThunks();
    ReachFact rf(guards, thunks);
    parsing_printf("Calculate backward slice\n");

    BackwardSlicer bs(func, block, block->last(), guards, rf);
    GraphPtr slice =  bs.CalculateBackwardSlicing();

    return false;

    parsing_printf("Calculate bound facts\n");     
    BoundFactsCalculator bfc(guards, func, slice, func->entry() == block, rf, thunks);
    bfc.CalculateBoundedFacts();

    BoundValue target;
    bool ijt = IsJumpTable(slice, bfc, target);
    if (ijt) {
        return FillInOutEdges(target, outEdges);
    } else return false;
}						       



bool IndirectControlFlowAnalyzer::EndWithConditionalJump(ParseAPI::Block * b) {

    const unsigned char * buf = (const unsigned char*) b->obj()->cs()->getPtrToInstruction(b->last());
    InstructionDecoder dec(buf, b->end() - b->last(), b->obj()->cs()->getArch());
    Instruction::Ptr insn = dec.decode();
    entryID id = insn->getOperation().getID();

    if (id == e_jz || id == e_jnz ||
        id == e_jb || id == e_jnb ||
	id == e_jbe || id == e_jnbe) return true;
   
//    for (auto eit = b->targets().begin(); eit != b->targets().end(); ++eit)
//        if ((*eit)->type() == COND_TAKEN) return true;
    return false;

}

void IndirectControlFlowAnalyzer::GetAllReachableBlock() {
    reachable.clear();
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
    set<ParseAPI::Block*> visited;
    queue<Block*> q;
    q.push(block);
    GetAllReachableBlock();

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
					      BoundFactsCalculator &bfc,
					      BoundValue &target) {
    NodeIterator sbegin, send;
    slice->exitNodes(sbegin, send);
    for (; sbegin != send; ++sbegin) {
        SliceNode::Ptr node = boost::static_pointer_cast<SliceNode>(*sbegin);
	const Absloc &loc = node->assign()->out().absloc();
	parsing_printf("Checking bound fact at %lx for %s\n",node->addr(), loc.format().c_str()); 
	BoundFact *bf = bfc.GetBoundFact(*sbegin);
	if (bf->IsBounded(loc)) {
	    target = *(bf->GetBound(loc));

	    if (target.tableLookup) return true;
	    if (target.tableOffset) return true;
	    if (target.type == LessThan && target.CoeBounded() && target.HasTableBase()) return true;
	}
    }
  

    return false;
}

void IndirectControlFlowAnalyzer::FindAllThunks() {
    for (auto bit = reachable.begin(); bit != reachable.end(); ++bit) {
        // We intentional treat a getting PC call as a special case that does not
	// end a basic block. So, we need to check every instruction to find all thunks
        ParseAPI::Block *b = *bit;
	const unsigned char* buf =
            (const unsigned char*)(b->obj()->cs()->getPtrToInstruction(b->start()));
	if( buf == NULL ) {
	    parsing_printf("%s[%d]: failed to get pointer to instruction by offset\n",FILE__, __LINE__);
	    return;
	}
	parsing_printf("Looking for thunk in block [%lx,%lx).", b->start(), b->end());
	InstructionDecoder dec(buf, b->end() - b->start(), b->obj()->cs()->getArch());
	InsnAdapter::IA_IAPI block(dec, b->start(), b->obj() , b->region(), b->obj()->cs(), b);
	while (block.getAddr() < b->end()) {
	    if (block.getInstruction()->getCategory() == c_CallInsn && block.isThunk()) {
	        bool valid;
		Address addr;
		boost::tie(valid, addr) = block.getCFT();
		const unsigned char *target = (const unsigned char *) b->obj()->cs()->getPtrToInstruction(addr);
		InstructionDecoder targetChecker(target, InstructionDecoder::maxInstructionLength, b->obj()->cs()->getArch());
		Instruction::Ptr thunkFirst = targetChecker.decode();
		set<RegisterAST::Ptr> thunkTargetRegs;
		thunkFirst->getWriteSet(thunkTargetRegs);
		
		for (auto curReg = thunkTargetRegs.begin(); curReg != thunkTargetRegs.end(); ++curReg) {
		    ThunkInfo t;
		    t.reg = (*curReg)->getID();
		    t.value = block.getAddr() + block.getInstruction()->size();
		    t.block = b;
		    thunks.insert(make_pair(block.getAddr(), t));

		    parsing_printf("\tfind thunk at %lx, storing value %lx to %s\n", block.getAddr(), t.value , t.reg.name().c_str());
		}
	    }
	    block.advance();
	}
    }
}


