#include "dyntypes.h"
#include "IndirectAnalyzer.h"
#include "BoundFactCalculator.h"
#include "BackwardSlicing.h"
#include "IA_IAPI.h"
#include "debug_parse.h"

#include "CodeObject.h"
#include "Graph.h"

#include "Instruction.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "SymEval.h"
#define SIGNEX_64_32 0xffffffff00000000LL
#define SIGNEX_64_16 0xffffffffffff0000LL
#define SIGNEX_64_8  0xffffffffffffff00LL
#define SIGNEX_32_16 0xffff0000
#define SIGNEX_32_8 0xffffff00

// Assume the table contain less than this many entries.
#define MAX_TABLE_ENTRY 1000000
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InstructionAPI;

bool IndirectControlFlowAnalyzer::FillInOutEdges(BoundValue &target, 
                                                 vector<pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges) {

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
/*
    if (block->last() == 0x805351a) {
        printf("Final fact: %d[%lx,%lx]\n", target.interval.stride, target.interval.low, target.interval.high);
    }
*/
    if (!block->obj()->cs()->isValidAddress(tableBase)) {
        parsing_printf("\ttableBase 0x%lx invalid, returning false\n", tableBase);
	return false;
    }

    for (Address tableEntry = tableBase; tableEntry <= tableLastEntry; tableEntry += target.interval.stride) {
	if (!block->obj()->cs()->isValidAddress(tableEntry)) continue;
	Address targetAddress = 0;
	if (target.isTableRead) {
	    // Two assumptions:
	    // 1. Assume the table contents are moved in a sign extended way;
	    // 2. Assume memory access size is the same as the table stride
	    switch (target.interval.stride) {
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
		    parsing_printf("Invalid table stride %d\n", target.interval.stride);
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

bool IndirectControlFlowAnalyzer::NewJumpTableAnalysis(std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges) {

//    if (block->last() != 0x804e30b) return false;

    parsing_printf("Apply indirect control flow analysis at %lx\n", block->last());

    parsing_printf("Calculate backward slice\n");

    BackwardSlicer bs(func, block, block->last());
    GraphPtr slice =  bs.CalculateBackwardSlicing();

    parsing_printf("Looking for thunk\n");
//  Find all blocks that reach the block containing the indirect jump
//  This is a prerequisit for finding thunks
    GetAllReachableBlock();
//  Now we try to find all thunks in this function.
//  We pass in the slice because we may need to add new ndoes.
    FindAllThunks(slice);
//  Calculates all blocks that can reach
//  and be reachable from thunk blocks
    ReachFact rf(thunks);

    parsing_printf("Calculate bound facts\n");     
    BoundFactsCalculator bfc(func, slice, func->entry() == block, rf, thunks, block->last());
    bfc.CalculateBoundedFacts();

    BoundValue target;
    bool ijt = IsJumpTable(slice, bfc, target);
    if (ijt) {
        return FillInOutEdges(target, outEdges);
    } else return false;
}						       




// Find all blocks that reach the block containing the indirect jump
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

bool IndirectControlFlowAnalyzer::IsJumpTable(GraphPtr slice, 
					      BoundFactsCalculator &bfc,
					      BoundValue &target) {
    NodeIterator exitBegin, exitEnd, srcBegin, srcEnd;
    slice->exitNodes(exitBegin, exitEnd);
    SliceNode::Ptr virtualExit = boost::static_pointer_cast<SliceNode>(*exitBegin);
    virtualExit->ins(srcBegin, srcEnd);
    SliceNode::Ptr jumpNode = boost::static_pointer_cast<SliceNode>(*srcBegin);
    
    const Absloc &loc = jumpNode->assign()->out().absloc();
    parsing_printf("Checking final bound fact for %s\n",loc.format().c_str()); 
    BoundFact *bf = bfc.GetBoundFact(virtualExit);
    BoundValue *tarBoundValue = bf->GetBound(VariableAST::create(Variable(loc)));
    if (tarBoundValue != NULL) {
        target = *(tarBoundValue);
	uint64_t s = target.interval.size();
	if (s > 0 && s <= MAX_TABLE_ENTRY) return true;
    }
    return false;
}

static Address ThunkAdjustment(Address afterThunk, MachRegister reg, GraphPtr slice, ParseAPI::Block *b) {
    // After the call to thunk, there is usually
    // an add insturction like ADD ebx, OFFSET to adjust
    // the value coming out of thunk.
    // This add instruction may not be in the slice.
    // Here assume that if the next instruction after thunk
    // is to add a constant value to the thunk register,
    // we then adjust the value.
    NodeIterator nbegin, nend;
    slice->allNodes(nbegin, nend);
    for (; nbegin != nend; ++nbegin) {
        SliceNode::Ptr cur = boost::static_pointer_cast<SliceNode>(*nbegin);
	// If the next instruction is already in the slice,
	// there is no need to adjust
	if (cur->addr() == afterThunk) return 0;
    }
    
    const unsigned char* buf = (const unsigned char*) (b->obj()->cs()->getPtrToInstruction(afterThunk));
    InstructionDecoder dec(buf, b->end() - b->start(), b->obj()->cs()->getArch());
    Instruction::Ptr nextInsn = dec.decode();
    // It has to be an add
    if (nextInsn->getOperation().getID() != e_add) return 0;
    vector<Operand> operands;
    nextInsn->getOperands(operands);
    RegisterAST::Ptr regAST = boost::dynamic_pointer_cast<RegisterAST>(operands[0].getValue());
    // The first operand should be a register
    if (regAST == 0) return 0;
    if (regAST->getID() != reg) return 0;
    Result res = operands[1].getValue()->eval();
    // A not defined result means that
    // the second operand is not an immediate
    if (!res.defined) return 0;
    return res.convert<Address>();
}

void IndirectControlFlowAnalyzer::FindAllThunks(GraphPtr slice) {
    // Enumuerate every block to find thunk
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
		    t.value += ThunkAdjustment(t.value, t.reg, slice, b);
		    t.block = b;
		    thunks.insert(make_pair(block.getAddr(), t));
		    parsing_printf("\tfind thunk at %lx, storing value %lx to %s\n", block.getAddr(), t.value , t.reg.name().c_str());
		}
	    }
	    block.advance();
	}
    }
}


