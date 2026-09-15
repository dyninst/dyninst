/*
 * See the dyninst/COPYRIGHT file for copyright information.
 */

#include "CFWidget.h"
#include "Widget.h"
#include "../CFG/RelocTarget.h"
#include "instructionAPI/h/Instruction.h"
#include "../dyninstAPI/src/debug.h"
#include "../CodeTracker.h"
#include "../CodeBuffer.h"
#include "dyninstAPI/src/addressSpace.h"
#include "registerSpace/registerSpace.h"
#include "common/src/arch-loongarch64.h"

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;
using namespace NS_loongarch64;

bool CFWidget::generateIndirect(CodeBuffer &buffer,
                                Register,
                                const RelocBlock *trace,
                                Instruction insn) {
    assert(0 && "CFWidget::generateIndirect not implemented for loongarch64");
    return false;
}

bool CFWidget::generateIndirectCall(CodeBuffer &buffer,
                                    Register,
                                    Instruction insn,
                                    const RelocBlock *trace,
                                    Address) {
    assert(0 && "CFWidget::generateIndirectCall not implemented for loongarch64");
    return false;
}

bool CFPatch::apply(codeGen &gen, CodeBuffer *buf) {
    assert(0 && "CFPatch::apply not implemented for loongarch64");
    return false;
}

bool CFPatch::applyPLT(codeGen &gen, CodeBuffer *buf) {
    assert(0 && "CFPatch::applyPLT not implemented for loongarch64");
    return false;
}

using namespace Dyninst::Relocation;

