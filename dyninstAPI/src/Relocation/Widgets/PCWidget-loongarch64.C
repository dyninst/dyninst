/*
 * See the dyninst/COPYRIGHT file for copyright information.
 */

#include "PCWidget.h"
#include "instructionAPI/h/Instruction.h"
#include "../dyninstAPI/src/debug.h"
#include "../CFG/RelocBlock.h"
#include "../CodeBuffer.h"
#include "../CodeTracker.h"
#include "patching/function.h"
#include "dyninstAPI/src/addressSpace.h"
#include "registerSpace/RegisterConversion.h"
#include "registerSpace/registerSpace.h"
#include "dyninstAPI/src/emitter.h"
#include "common/src/arch-loongarch64.h"

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;
using namespace NS_loongarch64;

bool PCWidget::PCtoReturnAddr(const codeGen &/*templ*/, const RelocBlock *t, CodeBuffer &buffer) {
    assert(0 && "PCWidget::PCtoReturnAddr not implemented for loongarch64");
    return false;
}

bool PCWidget::PCtoReg(const codeGen &/*templ*/, const RelocBlock *t, CodeBuffer &buffer) {
    assert(0 && "PCWidget::PCtoReg not implemented for loongarch64");
    return false;
}

bool IPPatch::apply(codeGen &/*gen*/, CodeBuffer *) {
    assert(0 && "IPPatch::apply not implemented for loongarch64");
    return false;
}