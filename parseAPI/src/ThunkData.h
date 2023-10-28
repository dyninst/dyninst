#ifndef THUNK_DATA_H
#define THUNK_DATA_H

#include "CFG.h"
//#include "Instruction.h"
#include "DynAST.h"
#include "registers/MachRegister.h"

using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;
//using namespace Dyninst::InstructionAPI;


struct ThunkInfo {
    MachRegister reg;
    Address value;
    ParseAPI::Block *block;
};

typedef  map<Address, ThunkInfo > ThunkData;


struct ReachFact {
    ThunkData &thunks;

    map<ParseAPI::Block*, set<ParseAPI::Block*> > thunk_ins, thunk_outs;

    void ReverseDFS(ParseAPI::Block *cur, set<ParseAPI::Block*> &visited);
    void NaturalDFS(ParseAPI::Block *cur, set<ParseAPI::Block*> &visited);

    void ReachBlocks();

    ReachFact(ThunkData &t);
};

#endif
