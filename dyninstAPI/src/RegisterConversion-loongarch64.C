/*
 * See the dyninst/COPYRIGHT file for copyright information.
 */

#include "RegisterConversion.h"
#include "registerSpace.h"
#include "common/h/dyn_regs.h"
#include <map>

using namespace Dyninst;
using namespace std;

// LoongArch64 register conversion

static map<MachRegister, Register> reverseRegisterMap;

static void initReverseMap() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    for (int i = 0; i < 32; i++) {
        reverseRegisterMap[MachRegister(loongarch64::r0 + i)] = i;
    }
    // FPR registers
    for (int i = 0; i < 32; i++) {
        reverseRegisterMap[MachRegister(loongarch64::f0 + i)] = 32 + i;
    }
    // Special registers
    reverseRegisterMap[loongarch64::sp] = 64;
    reverseRegisterMap[loongarch64::ra] = 65;
    reverseRegisterMap[loongarch64::fp] = 66;
    reverseRegisterMap[loongarch64::pc] = 67;
}

Register convertRegID(MachRegister reg, bool &wasUpcast) {
    wasUpcast = false;
    initReverseMap();

    MachRegister baseReg = MachRegister((reg.getBaseRegister().val() & ~reg.getArchitecture()) | Arch_loongarch64);
    map<MachRegister, Register>::const_iterator found = reverseRegisterMap.find(baseReg);
    if (found == reverseRegisterMap.end()) {
        return registerSpace::ignored;
    }
    return found->second;
}

Register convertRegID(boost::shared_ptr<InstructionAPI::RegisterAST> toBeConverted, bool& wasUpcast) {
    return convertRegID(toBeConverted.get(), wasUpcast);
}

Register convertRegID(InstructionAPI::RegisterAST* toBeConverted, bool& wasUpcast) {
    if (!toBeConverted) {
        return registerSpace::ignored;
    }
    return convertRegID(toBeConverted->getID(), wasUpcast);
}

MachRegister convertRegIDToMachRegister(int regID) {
    if (regID >= 0 && regID < 32) {
        return MachRegister(loongarch64::r0 + regID);
    }
    return Dyninst::InvalidReg;
}

// regToMachReg64 mapping for loongarch64
multimap<Register, MachRegister> regToMachReg64 = {
    {0, MachRegister(loongarch64::r0)},
    {1, MachRegister(loongarch64::r1)},
    {2, MachRegister(loongarch64::r2)},
    {3, MachRegister(loongarch64::r3)},
    {4, MachRegister(loongarch64::r4)},
    {5, MachRegister(loongarch64::r5)},
    {6, MachRegister(loongarch64::r6)},
    {7, MachRegister(loongarch64::r7)},
    {8, MachRegister(loongarch64::r8)},
    {9, MachRegister(loongarch64::r9)},
    {10, MachRegister(loongarch64::r10)},
    {11, MachRegister(loongarch64::r11)},
    {12, MachRegister(loongarch64::r12)},
    {13, MachRegister(loongarch64::r13)},
    {14, MachRegister(loongarch64::r14)},
    {15, MachRegister(loongarch64::r15)},
    {16, MachRegister(loongarch64::r16)},
    {17, MachRegister(loongarch64::r17)},
    {18, MachRegister(loongarch64::r18)},
    {19, MachRegister(loongarch64::r19)},
    {20, MachRegister(loongarch64::r20)},
    {21, MachRegister(loongarch64::r21)},
    {22, MachRegister(loongarch64::r22)},
    {23, MachRegister(loongarch64::r23)},
    {24, MachRegister(loongarch64::r24)},
    {25, MachRegister(loongarch64::r25)},
    {26, MachRegister(loongarch64::r26)},
    {27, MachRegister(loongarch64::r27)},
    {28, MachRegister(loongarch64::r28)},
    {29, MachRegister(loongarch64::r29)},
    {30, MachRegister(loongarch64::r30)},
    {31, MachRegister(loongarch64::r31)},
    {32, MachRegister(loongarch64::f0)},
    {33, MachRegister(loongarch64::f1)},
    {34, MachRegister(loongarch64::f2)},
    {35, MachRegister(loongarch64::f3)},
    {36, MachRegister(loongarch64::f4)},
    {37, MachRegister(loongarch64::f5)},
    {38, MachRegister(loongarch64::f6)},
    {39, MachRegister(loongarch64::f7)},
    {40, MachRegister(loongarch64::f8)},
    {41, MachRegister(loongarch64::f9)},
    {42, MachRegister(loongarch64::f10)},
    {43, MachRegister(loongarch64::f11)},
    {44, MachRegister(loongarch64::f12)},
    {45, MachRegister(loongarch64::f13)},
    {46, MachRegister(loongarch64::f14)},
    {47, MachRegister(loongarch64::f15)},
    {48, MachRegister(loongarch64::f16)},
    {49, MachRegister(loongarch64::f17)},
    {50, MachRegister(loongarch64::f18)},
    {51, MachRegister(loongarch64::f19)},
    {52, MachRegister(loongarch64::f20)},
    {53, MachRegister(loongarch64::f21)},
    {54, MachRegister(loongarch64::f22)},
    {55, MachRegister(loongarch64::f23)},
    {56, MachRegister(loongarch64::f24)},
    {57, MachRegister(loongarch64::f25)},
    {58, MachRegister(loongarch64::f26)},
    {59, MachRegister(loongarch64::f27)},
    {60, MachRegister(loongarch64::f28)},
    {61, MachRegister(loongarch64::f29)},
    {62, MachRegister(loongarch64::f30)},
    {63, MachRegister(loongarch64::f31)},
};

MachRegister convertRegID(Register r, Dyninst::Architecture arch) {
    if (arch == Arch_loongarch64) {
        auto it = regToMachReg64.find(r);
        if (it != regToMachReg64.end()) return it->second;
    }
    return Dyninst::InvalidReg;
}

int convertMachRegisterToRegID(MachRegister reg) {
    if (reg >= MachRegister(loongarch64::r0) &&
        reg <= MachRegister(loongarch64::r31)) {
        return reg - MachRegister(loongarch64::r0);
    }
    return -1;
}