#ifndef TABLE_GUARD_DATA_H
#define TABLE_GUARD_DATA_H

#include "CFG.h"
#include "Instruction.h"

#include "Instruction.h"
#include "DynAST.h"

using namespace Dyninst;
using namespace Dyninst::ParseAPI;
using namespace Dyninst::InstructionAPI;

struct GuardData {
    ParseAPI::Function *func;
    ParseAPI::Block* block;
    Instruction::Ptr cmpInsn;
    Instruction::Ptr jmpInsn;
    Address cmpInsnAddr, jmpInsnAddr;
    AST::Ptr cmpAST;
    uint64_t cmpBound;
    set<MachRegister> usedRegs;
    bool constantBound, jumpWhenNoZF, varSubtrahend;  
    GuardData(ParseAPI::Function *f,  ParseAPI::Block* b, Instruction::Ptr c, Instruction::Ptr j, Address ca, Address ja);
    bool operator< (const GuardData &g) const { return block < g.block; }
};

typedef set<GuardData> GuardSet;


struct ReachFact {
    GuardSet &guards;
    set<ParseAPI::Block*> forbid;
    map<ParseAPI::Block*, set<ParseAPI::Block*> > incoming;
    map<ParseAPI::Block*, set<ParseAPI::Block*> > branch_taken;
    map<ParseAPI::Block*, set<ParseAPI::Block*> > branch_ft;

    void ReverseDFS(ParseAPI::Block *cur, set<ParseAPI::Block*> &visited);
    void NaturalDFS(ParseAPI::Block *cur, set<ParseAPI::Block*> &visited);

    void ReachBlocks();

    ReachFact(GuardSet &g);
};

#endif
