#ifndef THUNK_DATA_H
#define THUNK_DATA_H

#include "CFG.h"
//#include "Instruction.h"
#include "DynAST.h"
#include "registers/MachRegister.h"

using namespace std;
//using namespace Dyninst::InstructionAPI;


struct ThunkInfo {
    Dyninst::MachRegister reg;
    Dyninst::Address value;
    Dyninst::ParseAPI::Block *block;
};

typedef  map<Dyninst::Address, ThunkInfo > ThunkData;


struct ReachFact {
    ThunkData &thunks;

    map<Dyninst::ParseAPI::Block*, set<Dyninst::ParseAPI::Block*> > thunk_ins, thunk_outs;

    void ReverseDFS(Dyninst::ParseAPI::Block *cur, set<Dyninst::ParseAPI::Block*> &visited);
    void NaturalDFS(Dyninst::ParseAPI::Block *cur, set<Dyninst::ParseAPI::Block*> &visited);

    void ReachBlocks();

    ReachFact(ThunkData &t);
};

#endif
