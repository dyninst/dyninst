#include "dyntypes.h"
#include "IndirectAnalyzer.h"
//#include "BoundFactCalculator.h"
#include "JumpTableFormatPred.h"
#include "SymbolicExpression.h"
#include "IA_IAPI.h"
#include "debug_parse.h"

#include "CodeObject.h"
#include "Graph.h"

#include "Instruction.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "SymEval.h"
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InstructionAPI;


bool IndirectControlFlowAnalyzer::NewJumpTableAnalysis(std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges) {
//    if (block->last() == 0x3ed4f33e9e) dyn_debug_parsing=1; else dyn_debug_parsing=0;
    parsing_printf("Apply indirect control flow analysis at %lx\n", block->last());
    parsing_printf("Looking for thunk\n");

//  Find all blocks that reach the block containing the indirect jump
//  This is a prerequisit for finding thunks
    GetAllReachableBlock();
//  Now we try to find all thunks in this function.
//  We pass in the slice because we may need to add new ndoes.
    FindAllThunks();
//  Calculates all blocks that can reach
//  and be reachable from thunk blocks
    ReachFact rf(thunks);

    // Now we start with the indirect jump instruction,
    // to determine the format of the (potential) jump table
    const unsigned char * buf = (const unsigned char*) block->obj()->cs()->getPtrToInstruction(block->last());
    InstructionDecoder dec(buf, InstructionDecoder::maxInstructionLength, block->obj()->cs()->getArch());
    Instruction::Ptr insn = dec.decode();
    AssignmentConverter ac(true, false);
    vector<Assignment::Ptr> assignments;
    ac.convert(insn, block->last(), func, block, assignments);
    Slicer formatSlicer(assignments[0], block, func, false, false);

    SymbolicExpression se;
    JumpTableFormatPred jtfp(func, block, rf, thunks, se);
    GraphPtr slice = formatSlicer.backwardSlice(jtfp);
    //parsing_printf("\tJump table format: %s\n", jtfp.format().c_str());

    // If the jump target expression is not in a form we recognize,
    // we do not try to resolve it
    if (!jtfp.isJumpTableFormat()) {
        return false;
    }
/*
    Slicer indexSlicer(); 
    JumpTableIndexPred jtip(func, block, jtfp.indexVarible());
    jtip.setSearchForControlFlowDep(true);
    GraphPtr slice = indexSlicer.backwardSlice(jtp);
    
    if (!jtip.indexBounded() && block->obj()->cs()->getArch() != Arch_aarch64) {
        // After the slicing is done, we do one last check to 
        // see if we can resolve the indirect jump by assuming 
        // one byte read is in bound [0,255]
        GraphPtr g = jtp.BuildAnalysisGraph(s.visitedEdges);
	
	BoundFactsCalculator bfc(func, g, func->entry() == block, rf, thunks, true, jtp.expandCache);
	bfc.CalculateBoundedFacts();
	
	BoundValue target;
	bool ijt = jtp.IsJumpTable(g, bfc, target);
	if (ijt) jtp.FillInOutEdges(target, jumpTableOutEdges);
    }

    
    if (!jtip.indexBounded()) {
         indexBound = ScanTable(jtfp);
    } else {
         indexBound = jtip.indexBound();
    }

    std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > > jumpTableOutEdges;
    ReadTable(jtfp, indexBound);

    outEdges.insert(outEdges.end(), jumpTableOutEdges.begin(), jumpTableOutEdges.end());
    return !jumpTableOutEdges.empty();
*/
    return false;
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


static Address ThunkAdjustment(Address afterThunk, MachRegister reg, ParseAPI::Block *b) {
    // After the call to thunk, there is usually
    // an add insturction like ADD ebx, OFFSET to adjust
    // the value coming out of thunk.
   
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

void IndirectControlFlowAnalyzer::FindAllThunks() {
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
	InstructionDecoder dec(buf, b->end() - b->start(), b->obj()->cs()->getArch());
	InsnAdapter::IA_IAPI block(dec, b->start(), b->obj() , b->region(), b->obj()->cs(), b);
	Address cur = b->start();
	while (cur < b->end()) {
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
		    t.value += ThunkAdjustment(t.value, t.reg, b);
		    t.block = b;
		    thunks.insert(make_pair(block.getAddr(), t));
		    parsing_printf("\tfind thunk at %lx, storing value %lx to %s\n", block.getAddr(), t.value , t.reg.name().c_str());
		}
	    }
	    cur += block.getInstruction()->size();
	    if (cur < b->end()) block.advance();
	}
    }
}


