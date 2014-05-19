#include "TableGuardData.h"
#include "IndirectASTVisitor.h"
#include "CFG.h"
#include "Absloc.h"
#include "AbslocInterface.h"
#include "debug_parse.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::DataflowAPI;
using namespace Dyninst::ParseAPI;

static void GetUsedRegisters(set<MachRegister> &regs, AST::Ptr ast) {
    unsigned totalChildren = ast->numChildren();
    for (unsigned i = 0 ; i < totalChildren; ++i) {
        GetUsedRegisters(regs, ast->child(i));
    }
    if (ast->getID() == AST::V_VariableAST) {
        VariableAST::Ptr varAST = boost::static_pointer_cast<VariableAST>(ast);
	const Absloc & loc = varAST->val().reg.absloc();
	if (loc.type() == Absloc::Register)
	    regs.insert(loc.reg());
    }
}

GuardData::GuardData(ParseAPI::Function *f,
                     ParseAPI::Block* b, 
		     Instruction::Ptr c, 
		     Instruction::Ptr j, 
		     Address ca, 
		     Address ja):
        func(f), block(b), cmpInsn(c), jmpInsn(j), cmpInsnAddr(ca), jmpInsnAddr(ja) {

    Result_t res;
    AssignmentConverter ac(true, false);
    vector<Assignment::Ptr> assignments;
    parsing_printf("cmpInsn %s at %lx\n", cmpInsn->format().c_str(), cmpInsnAddr);


    ac.convert(cmpInsn, cmpInsnAddr, func, block, assignments);    

    for (auto ait = assignments.begin(); ait != assignments.end(); ++ait) {
        res[*ait] = AST::Ptr();
    }
    set<Instruction::Ptr> failedInsns;
    SymEval::expand(res, failedInsns, false);

    // Extract variable and bound from the symbolic expression of zf
    AST::Ptr cmp;
    for (auto ait = assignments.begin(); ait != assignments.end(); ++ait) {
        Absloc loc = (*ait)->out().absloc();
	if (loc.type() == Absloc::Register && (loc.reg() == x86::zf || loc.reg() == x86_64::zf)) {
	    cmp = res[*ait];
	    break;
	}
    }
   

    ComparisonVisitor cv;
    cmp->accept(&cv);
    parsing_printf("jumpInsn %s at %lx\n", jmpInsn->format().c_str(), jmpInsnAddr);

    res.clear();
    ac.convert(jmpInsn, jmpInsnAddr, func, block, assignments);
    for (auto ait = assignments.begin(); ait != assignments.end(); ++ait)
        res[*ait] = AST::Ptr();
    SymEval::expand(res, failedInsns, false);

    JumpCondVisitor jcv;
    res.begin()->second->accept(&jcv);

    constantBound = (cv.minuend->getID() == AST::V_ConstantAST) || (cv.subtrahend->getID() == AST::V_ConstantAST);
    if (!constantBound) return;

    jumpWhenNoZF = jcv.invertFlag;
   
    if (cv.minuend->getID() == AST::V_ConstantAST) {
        ConstantAST::Ptr minuendAST = boost::static_pointer_cast<ConstantAST>(cv.minuend);
        cmpBound = minuendAST->val().val;
	cmpAST = SimplifyAnAST(cv.subtrahend, cmpInsn->size());
	varSubtrahend = true;
    } else {
        ConstantAST::Ptr subtrahendAST = boost::static_pointer_cast<ConstantAST>(cv.subtrahend);
        cmpBound = subtrahendAST->val().val;
	cmpAST = SimplifyAnAST(cv.minuend, cmpInsn->size());
	varSubtrahend = false;

    }

    GetUsedRegisters(usedRegs, cmpAST);
    
    parsing_printf("constantBound = %d, JumpWhenNoZF = %d, varSubtrahend = %d\n", constantBound, jumpWhenNoZF, varSubtrahend);
    parsing_printf("subtrahend AST %s\n", cv.subtrahend->format().c_str());
    parsing_printf("minuend AST %s\n", cv.minuend->format().c_str());

}	
void ReachFact::ReverseDFS(ParseAPI::Block *cur, set<ParseAPI::Block*> &visited) {
    if (visited.find(cur) != visited.end()) return;
    visited.insert(cur);

    for (auto eit = cur->sources().begin(); eit != cur->sources().end(); ++eit) 
        if ((*eit)->intraproc() && (*eit)->type() != INDIRECT) ReverseDFS((*eit)->src(), visited);
}

void ReachFact::NaturalDFS(ParseAPI::Block *cur, set<ParseAPI::Block*> &visited) {
    if (forbid.find(cur) != forbid.end()) return;
    if (visited.find(cur) != visited.end()) return;
    visited.insert(cur);

    for (auto eit = cur->targets().begin(); eit != cur->targets().end(); ++eit) 
        if ((*eit)->intraproc() && (*eit)->type() != INDIRECT) NaturalDFS((*eit)->trg(), visited);
}



void ReachFact::ReachBlocks() {
    for (auto git = guards.begin(); git != guards.end(); ++git)
        forbid.insert(git->block);
    for (auto git = guards.begin(); git != guards.end(); ++git) {
        ParseAPI::Block * jmpBlock = git->block;
	ReverseDFS(jmpBlock, incoming[jmpBlock]);
	
	ParseAPI::Block *condTakenBlock, *condFTBlock;
	for (auto eit = jmpBlock->targets().begin(); eit != jmpBlock->targets().end(); ++eit)
	    if ((*eit)->type() == COND_TAKEN) condTakenBlock = (*eit)->trg();
	    else condFTBlock = (*eit)->trg(); 
	
	NaturalDFS(condTakenBlock, branch_taken[jmpBlock]);
	NaturalDFS(condFTBlock, branch_ft[jmpBlock]);
    }
}

ReachFact::ReachFact(GuardSet &g): guards(g) {
    ReachBlocks();
}

