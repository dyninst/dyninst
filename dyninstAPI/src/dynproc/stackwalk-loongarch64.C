#include "debug.h"
#include "dynProcess.h"
#include "registers/loongarch64_regs.h"
#include <assert.h>

using namespace Dyninst;

bool PCProcess::createStackwalkerSteppers() {
    assert(0 && "createStackwalkerSteppers not implemented for loongarch64");
    return false;
}

bool StackwalkInstrumentationHelper::isInstrumentation(Dyninst::Address,
                                                       Dyninst::Address *,
                                                       unsigned *,
                                                       bool *,
                                                       bool *) {
    return false;
}

using namespace Stackwalker;

FrameFuncHelper::alloc_frame_t DynFrameHelper::allocatesFrame(Address) {
    FrameFuncHelper::alloc_frame_t result;
    result.first = FrameFuncHelper::unknown_t;
    result.second = FrameFuncHelper::unknown_s;
    return result;
}

bool DynWandererHelper::isPrevInstrACall(Address, Address &) {
    return false;
}

WandererHelper::pc_state DynWandererHelper::isPCInFunc(Address, Address) {
    return WandererHelper::unknown_s;
}

bool DynWandererHelper::requireExactMatch() {
    return false;
}
