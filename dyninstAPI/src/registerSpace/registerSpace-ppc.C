#include "arch-power.h"
#include "dyn_register.h"
#include "registerSpace.h"

#include <vector>
#include <cstdio>


// Note that while we have register definitions for r13..r31, we only
// use r0..r12 (well, r3..r12). I believe this is to reduce the number
// of saves that we execute. 
void registerSpace::initialize32() {
    static bool done = false;
    if (done) return;
    done = true;

    std::vector<registerSlot *> registers;

    // At ABI boundary: R0 and R12 are dead, others are live.
    // Also, define registers in reverse order - it helps with
    // function calls
    
    registers.push_back(new registerSlot(r12,
                                         "r12",
                                         false,
                                         registerSlot::deadABI,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r11,
                                         "r11",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r10,
                                         "r10",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r9,
                                         "r9",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r8,
                                         "r8",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r7,
                                         "r7",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r6,
                                         "r6",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r5,
                                         "r5",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r4,
                                         "r4",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r3,
                                         "r3",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));

    // Everyone else
    for (unsigned i = r13; i <= r31; ++i) {
      char name[32];
      sprintf(name, "r%u", i-r0);
      registers.push_back(new registerSlot(i, name,
					   false, 
					   registerSlot::liveAlways,
					   registerSlot::GPR));
    }

    /// Aaaand the off-limits ones.

    registers.push_back(new registerSlot(r0,
                                         "r0",
                                         true, // Don't use r0 - it has all sorts
                                         // of implicit behavior.
                                         registerSlot::deadABI,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r1,
                                         "r1",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r2,
                                         "r2",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));

    for (unsigned i = fpr0; i <= fpr13; i++) {
        char buf[128];
        sprintf(buf, "fpr%u", i - fpr0);
        registers.push_back(new registerSlot(i,
                                             buf,
                                             false,
                                             registerSlot::liveAlways,
                                             registerSlot::FPR));
    }
    registers.push_back(new registerSlot(xer,
                                         "xer",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
 
    registers.push_back(new registerSlot(lr,
                                         "lr",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(cr,
                                         "cr",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(ctr,
                                         "ctr",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(mq,
                                         "mq",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registerSpace::createRegisterSpace(registers);

    // Initialize the sets that encode which registers
    // are used/defined by calls, returns, and syscalls. 
    // These all assume the ABI, of course. 

    // TODO: Linux/PPC needs these set as well.
    

}

void registerSpace::initialize64() {
    static bool done = false;
    if (done) return;
    done = true;

    std::vector<registerSlot *> registers;

    // At ABI boundary: R0 and R12 are dead, others are live.
    // Also, define registers in reverse order - it helps with
    // function calls
    
    registers.push_back(new registerSlot(r12,
                                         "r12",
                                         false,
                                         registerSlot::deadABI,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r11,
                                         "r11",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r10,
                                         "r10",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r9,
                                         "r9",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r8,
                                         "r8",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r7,
                                         "r7",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r6,
                                         "r6",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r5,
                                         "r5",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r4,
                                         "r4",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r3,
                                         "r3",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));


    // Everyone else
    for (unsigned i = r13; i <= r31; ++i) {
      char name[32];
      sprintf(name, "r%u", i-r0);
      registers.push_back(new registerSlot(i, name,
					   false, 
					   registerSlot::liveAlways,
					   registerSlot::GPR));
    }


    /// Aaaand the off-limits ones.

    registers.push_back(new registerSlot(r0,
                                         "r0",
                                         true, // Don't use r0 - it has all sorts
                                         // of implicit behavior.
                                         registerSlot::deadABI,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r1,
                                         "r1",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));
    registers.push_back(new registerSlot(r2,
                                         "r2",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR));

    for (unsigned i = fpr0; i <= fpr13; i++) {
        char buf[128];
        sprintf(buf, "fpr%u", i - fpr0);
        registers.push_back(new registerSlot(i,
                                             buf,
                                             false,
                                             registerSlot::liveAlways,
                                             registerSlot::FPR));
    }
    registers.push_back(new registerSlot(xer,
                                         "xer",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(lr,
                                         "lr",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(cr,
                                         "cr",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(ctr,
                                         "ctr",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(mq,
                                         "mq",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registerSpace::createRegisterSpace64(registers);

    // Initialize the sets that encode which registers
    // are used/defined by calls, returns, and syscalls. 
    // These all assume the ABI, of course. 

    // TODO: Linux/PPC needs these set as well.
    

}

void registerSpace::initialize() {
    initialize32();
    initialize64();
}

unsigned registerSpace::SPR(Dyninst::Register x) {
    // Encodings from architecture manual
    switch (static_cast<powerRegisters_t>(x.getId())) {
    case xer:
        return SPR_XER;
        break;
    case lr:
        return SPR_LR;
        break;
    case ctr:
        return SPR_CTR;
        break;
    case mq:
        return SPR_MQ;
        break;
    case cr:
        fprintf(stderr, "Error: condition register has no encoding!\n");
        return Dyninst::Null_Register;
        break;
    default:
        assert(0);
        return Dyninst::Null_Register;
        break;
    }
    return Dyninst::Null_Register;
}
