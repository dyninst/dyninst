#if defined(cap_liveness)
#ifndef REGISTERMAP_H
#define REGISTERMAP_H

#include <map>
#include "dynutil/h/dyn_regs.h"
using namespace Dyninst;
extern std::map<MachRegister, int> machRegIndex_x86;
extern std::map<MachRegister, int> machRegIndex_x86_64;
extern std::map<MachRegister, int> machRegIndex_ppc;
extern std::map<MachRegister, int> machRegIndex_ppc_64;




#endif //REGISTERMAP_H

#endif //cap_liveness
