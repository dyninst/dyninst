#include "registerSpace.h"
#include "registers/loongarch64_regs.h"
#include <map>

using namespace Dyninst;

std::multimap<Register, MachRegister> regToMachReg32;
std::multimap<Register, MachRegister> regToMachReg64;

MachRegister convertRegID(Dyninst::Register r, Dyninst::Architecture arch) {
    if (arch == Arch_loongarch64) {
        auto it = regToMachReg64.find(r);
        if (it != regToMachReg64.end()) return it->second;
    }
    return Dyninst::MachRegister(0);
}

void registerSpace::initialize32() {
    assert(0 && "registerSpace::initialize32 not supported for loongarch64");
}

void registerSpace::initialize64() {
    // Minimal implementation - registers will be populated at runtime
}

void registerSpace::initialize() { initialize64(); }
