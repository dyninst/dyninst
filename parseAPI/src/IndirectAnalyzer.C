#include "dyntypes.h"
#include "IndirectAnalyzer.h"
#include "BoundFactCalculator.h"
#include "JumpTableFormatPred.h"
#include "JumpTableIndexPred.h"
#include "SymbolicExpression.h"
#include "IndirectASTVisitor.h"
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
    parsing_printf("Apply indirect control flow analysis at %lx\n", block->last());
    parsing_printf("Looking for thunk\n");
//    if (block->last() == 0x526e74) dyn_debug_parsing=1; else dyn_debug_parsing=0;

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
    fprintf(stderr, "Address %lx, jump target format %s, index loc %s,", block->last(), jtfp.format().c_str(), jtfp.indexLoc ? jtfp.indexLoc->format().c_str() : "");

    if (!jtfp.isJumpTableFormat()) {
        fprintf(stderr, " not jump table\n");
        return false;
    }

    Slicer indexSlicer(jtfp.indexLoc, jtfp.indexLoc->block(), func, false, false); 
    JumpTableIndexPred jtip(func, block, jtfp.index, se);
    jtip.setSearchForControlFlowDep(true);
    slice = indexSlicer.backwardSlice(jtip);
    
    if (!jtip.findBound && block->obj()->cs()->getArch() != Arch_aarch64) {
        // After the slicing is done, we do one last check to 
        // see if we can resolve the indirect jump by assuming 
        // one byte read is in bound [0,255]
        GraphPtr g = jtip.BuildAnalysisGraph(indexSlicer.visitedEdges);
	
	BoundFactsCalculator bfc(func, g, func->entry() == block,  true, se);
	bfc.CalculateBoundedFacts();
	
	StridedInterval target;
	jtip.IsIndexBounded(g, bfc, target);
    }
    if (jtip.findBound) {
        fprintf(stderr, " bound %s", jtip.bound.format().c_str());
    } else {
        fprintf(stderr, " Cannot find bound\n");
	return false;
    }
/* 
    if (!jtip.findBound()) {
         indexBound = ScanTable(jtfp);
    } else {
         indexBound = jtip.indexBound();
    }
*/
    std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > > jumpTableOutEdges;
    ReadTable(jtfp.jumpTargetExpr, 
              jtfp.index, 
	      jtip.bound, 
	      GetMemoryReadSize(jtfp.memLoc), 
	      jumpTableOutEdges);
    fprintf(stderr, ", find %d edges\n", jumpTableOutEdges.size());	      
    outEdges.insert(outEdges.end(), jumpTableOutEdges.begin(), jumpTableOutEdges.end());
    return !jumpTableOutEdges.empty();
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

void IndirectControlFlowAnalyzer::ReadTable(AST::Ptr jumpTargetExpr, 
                                            AbsRegion index,
					    StridedInterval &indexBound,   
					    int memoryReadSize,
					    std::vector<std::pair<Address, Dyninst::ParseAPI::EdgeTypeEnum> > &targetEdges) {
    CodeSource *cs = block->obj()->cs();					    
    set<Address> jumpTargets;
    for (int v = indexBound.low; v <= indexBound.high; v += indexBound.stride) {
        // TODO: need to detect whether the memory is a zero extend or a sign extend
        JumpTableReadVisitor jtrv(index, v, cs, false, memoryReadSize);
	jumpTargetExpr->accept(&jtrv);
	if (jtrv.valid && cs->isCode(jtrv.targetAddress)) {
	    jumpTargets.insert(jtrv.targetAddress);
	} else {
	    // We have a bad entry. We stop here, as we have wrong information
	    // In this case, we keep the good entries
	    parsing_printf("WARNING: resolving jump tables leads to a bad address %lx\n", jtrv.targetAddress);
	    break;
	}
	if (indexBound.stride == 0) break;
    }
    for (auto tit = jumpTargets.begin(); tit != jumpTargets.end(); ++tit) {
        targetEdges.push_back(make_pair(*tit, INDIRECT));
    }
}					    

int IndirectControlFlowAnalyzer::GetMemoryReadSize(Assignment::Ptr memLoc) {
    Instruction::Ptr i = memLoc->insn();
    std::vector<Operand> ops;
    i->getOperands(ops);
    for (auto oit = ops.begin(); oit != ops.end(); ++oit) {
        Operand o = *oit;
	if (o.readsMemory()) {
	    Expression::Ptr exp = o.getValue();
	    return exp->size();
	    break;
	}
    }
}
