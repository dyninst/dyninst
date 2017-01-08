#include "IA_ARMv6MDetails.h"
#include "IndirectAnalyzer.h"


using namespace Dyninst;
using namespace Dyninst::InsnAdapter;


bool IA_ARMv6MDetails::parseJumpTable(Block*, std::vector<std::pair<Address, EdgeTypeEnum> >&)
{
    assert(0);
    return true;
}

bool IA_ARMv6MDetails::parseJumpTable(Dyninst::ParseAPI::Function* currFunc,
                                      Dyninst::ParseAPI::Block* currBlk,
                                      std::vector<std::pair<Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges)
{
    IndirectControlFlowAnalyzer icfa(currFunc, currBlk);
    bool ret = icfa.NewJumpTableAnalysis(outEdges);

    parsing_printf("Jump table parser returned %d, %d edges\n", ret, outEdges.size());
    for (auto oit = outEdges.begin(); oit != outEdges.end(); ++oit)
        parsing_printf("edge target at %lx\n", oit->first);

    // Update statistics
    currBlk->obj()->cs()->incrementCounter(PARSE_JUMPTABLE_COUNT);
    if (!ret)
        currBlk->obj()->cs()->incrementCounter(PARSE_JUMPTABLE_FAIL);

    return ret;
}