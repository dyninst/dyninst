#ifndef IA_ARMV6MDETAILS_H
#define IA_ARMV6MDETAILS_H

#include "CFG.h"
#include "dyntypes.h"
#include "IA_platformDetails.h"


namespace Dyninst {
namespace InsnAdapter {

class IA_ARMv6MDetails : public IA_platformDetails
{
public:
    bool parseJumpTable(Dyninst::ParseAPI::Block* currBlk,
                        std::vector<std::pair<Address, Dyninst::ParseAPI::EdgeTypeEnum> >& outEdges);
    bool parseJumpTable(Dyninst::ParseAPI::Function* currFunc,
                        Dyninst::ParseAPI::Block* currBlk,
                        std::vector<std::pair<Address, Dyninst::ParseAPI::EdgeTypeEnum> >& outEdges);

private:
    friend IA_platformDetails* makePlatformDetails(Dyninst::Architecture Arch, const IA_IAPI* cb);
    IA_ARMv6MDetails(const IA_IAPI* cb) :
        IA_platformDetails(cb)
    {
    }

};

}
}

#endif