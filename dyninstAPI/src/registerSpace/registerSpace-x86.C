#include "registerSpace.h"
#include "arch-x86.h"
#include "inst-x86.h"

#include <vector>
#include <cstdio>

void registerSpace::initialize32() {
    static bool done = false;
    if (done) return;
    done = true;

    // On 32-bit x86 we use stack slots as "registers"; therefore we can
    // create an arbitrary number, and use them. However, this can bite us
    // if we want to use actual registers. Any ideas?
    
    std::vector<registerSlot *> registers;

    // When we use 
    registerSlot *eax = new registerSlot(REGNUM_EAX,
                                        "eax",
                                        false, // Off-limits due to our "stack slot" register mechanism
                                        registerSlot::liveAlways,
                                        registerSlot::realReg);
    registerSlot *ecx = new registerSlot(REGNUM_ECX,
                                        "ecx",
                                        false,
                                        registerSlot::liveAlways,
                                        registerSlot::realReg);
    registerSlot *edx = new registerSlot(REGNUM_EDX,
                                        "edx",
                                        false,
                                        registerSlot::liveAlways,
                                        registerSlot::realReg);
    registerSlot *ebx = new registerSlot(REGNUM_EBX,
                                        "ebx",
                                        false,
                                        registerSlot::liveAlways,
                                        registerSlot::realReg);
    registerSlot *esp = new registerSlot(REGNUM_ESP,
                                        "esp",
                                        true, // Off-limits...
                                        registerSlot::liveAlways,
                                        registerSlot::realReg); // I'd argue the SP is a special-purpose reg
    registerSlot *ebp = new registerSlot(REGNUM_EBP,
                                        "ebp",
                                        true,
                                        registerSlot::liveAlways,
                                        registerSlot::realReg);
    registerSlot *esi = new registerSlot(REGNUM_ESI,
                                        "esi",
                                        false,
                                        registerSlot::liveAlways,
                                        registerSlot::realReg);
    registerSlot *edi = new registerSlot(REGNUM_EDI,
                                        "edi",
                                        false,
                                        registerSlot::liveAlways,
                                        registerSlot::realReg);
    
    registers.push_back(eax);
    registers.push_back(ecx);
    registers.push_back(edx);
    registers.push_back(ebx);
    registers.push_back(esp);
    registers.push_back(ebp);
    registers.push_back(esi);
    registers.push_back(edi);

    // FPRs...

    // SPRs...
    registerSlot *gs = new registerSlot(REGNUM_GS,
            "gs",
            false,
            registerSlot::liveAlways,
            registerSlot::SPR);

    registers.push_back(gs);

    // "Virtual" registers
    for (unsigned i = 1; i <= NUM_VIRTUAL_REGISTERS; i++) {
		char buf[128];
        sprintf(buf, "virtGPR%u", i);

        registerSlot *virt = new registerSlot(i,
                                              buf,
                                              false,
                                              registerSlot::deadAlways,
                                              registerSlot::GPR);
        registers.push_back(virt);
    }
    // Create a single FPR representation to represent
    // whether any FPR is live
    registerSlot *fpr = new registerSlot(IA32_FPR_VIRTUAL_REGISTER,
                                         "virtFPR",
                                         true, // off-limits...
                                         registerSlot::liveAlways, // because we check this via overapproximation and not the
                                         // regular liveness algorithm, start out *dead* and set live if written
                                         registerSlot::FPR);
    registers.push_back(fpr);

    // And a "do we save the flags" "register"
    registers.push_back(new registerSlot(IA32_FLAG_VIRTUAL_REGISTER,
                                         "virtFlags",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    // Create the global register space
    registerSpace::createRegisterSpace(registers);

}

#if defined(DYNINST_CODEGEN_ARCH_X86_64)
void registerSpace::initialize64() {
    static bool done = false;
    if (done) return;
    done = true;

    // Create the 64-bit registers
    // Well, let's just list them....

    // Calling ABI:
    // rax, rcx, rdx, r8, r9, r10, r11 are not preserved across a call
    // However, rcx, rdx, r8, and r9 are used for arguments, and therefore
    // should be assumed live. 
    // So rax, r10, r11 are dead at a function call.

    registerSlot * rax = new registerSlot(REGNUM_RAX,
                                          "rax",
					  // TODO FIXME but I need it...
                                          false, // We use it implicitly _everywhere_
                                          registerSlot::deadABI,
                                          registerSlot::GPR);
    registerSlot * rcx = new registerSlot(REGNUM_RCX,
                                          "rcx",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * rdx = new registerSlot(REGNUM_RDX,
                                          "rdx",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * rbx = new registerSlot(REGNUM_RBX,
                                          "rbx",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * rsp = new registerSlot(REGNUM_RSP,
                                          "rsp",
                                          true, // Off-limits...
                                          registerSlot::liveAlways,
                                          registerSlot::GPR); 
    registerSlot * rbp = new registerSlot(REGNUM_RBP,
                                          "rbp",
                                          true,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * rsi = new registerSlot(REGNUM_RSI,
                                          "rsi",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * rdi = new registerSlot(REGNUM_RDI,
                                          "rdi",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * r8 = new registerSlot(REGNUM_R8,
                                         "r8",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR);
    registerSlot * r9 = new registerSlot(REGNUM_R9,
                                         "r9",
                                         false,
                                         registerSlot::liveAlways,
                                         registerSlot::GPR);
    registerSlot * r10 = new registerSlot(REGNUM_R10,
                                          "r10",
                                          false,
                                          registerSlot::deadABI,
                                          registerSlot::GPR);
    registerSlot * r11 = new registerSlot(REGNUM_R11,
                                          "r11",
                                          false,
                                          registerSlot::deadABI,
                                          registerSlot::GPR);
    registerSlot * r12 = new registerSlot(REGNUM_R12,
                                          "r12",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * r13 = new registerSlot(REGNUM_R13,
                                          "r13",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * r14 = new registerSlot(REGNUM_R14,
                                          "r14",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);
    registerSlot * r15 = new registerSlot(REGNUM_R15,
                                          "r15",
                                          false,
                                          registerSlot::liveAlways,
                                          registerSlot::GPR);

    std::vector<registerSlot *> registers;
    registers.push_back(rax);
    registers.push_back(rbx);
    registers.push_back(rsp);
    registers.push_back(rbp);
    registers.push_back(r10);
    registers.push_back(r11);
    registers.push_back(r12);
    registers.push_back(r13);
    registers.push_back(r14);
    registers.push_back(r15);

    // Put the call parameter registers last so that we are not
    // likely to allocate them for general purposes
    registers.push_back(r8);
    registers.push_back(r9);
    registers.push_back(rcx);
    registers.push_back(rdx);
    registers.push_back(rsi);
    registers.push_back(rdi);

    registers.push_back(new registerSlot(REGNUM_EFLAGS,
                                         "eflags",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));

    registers.push_back(new registerSlot(REGNUM_OF,
                                         "of",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_SF,
                                         "sf",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_ZF,
                                         "zf",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_AF,
                                         "af",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_PF,
                                         "pf",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_CF,
                                         "cf",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_TF,
                                         "tf",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_IF,
                                         "if",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_DF,
                                         "df",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_NT,
                                         "nt",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));
    registers.push_back(new registerSlot(REGNUM_RF,
                                         "rf",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));

    registers.push_back(new registerSlot(REGNUM_DUMMYFPR,
                                         "dummyFPR",
                                         true,
                                         registerSlot::liveAlways, // because we check this via overapproximation and not the
                                         // regular liveness algorithm, start out *dead* and set live if written
                                         registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM0,
					 "MM0/ST(0)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM1,
					 "MM1/ST(1)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM2,
					 "MM2/ST(2)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM3,
					 "MM3/ST(3)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM4,
					 "MM4/ST(4)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM5,
					 "MM5/ST(5)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM6,
					 "MM6/ST(6)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_MM7,
					 "MM7/ST(7)",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM0,
					 "XMM0",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM1,
					 "XMM1",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM2,
					 "XMM2",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM3,
					 "XMM3",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM4,
					 "XMM4",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM5,
					 "XMM5",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM6,
					 "XMM6",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM7,
					 "XMM7",
					 true,
					 registerSlot::liveAlways,
					 registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM8,
                        "XMM8",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM9,
                        "XMM9",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM10,
                        "XMM10",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM11,
                        "XMM11",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM12,
                        "XMM12",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM13,
                        "XMM13",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM14,
                        "XMM14",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));
    registers.push_back(new registerSlot(REGNUM_XMM15,
                        "XMM15",
                        true,
                        registerSlot::liveAlways,
                        registerSlot::FPR));

    registers.push_back(new registerSlot(REGNUM_FS,
                        "FS",
                        false,
                        registerSlot::liveAlways,
                        registerSlot::SPR));



    // For registers that we really just don't care about.
    registers.push_back(new registerSlot(REGNUM_IGNORED,
                                         "ignored",
                                         true,
                                         registerSlot::liveAlways,
                                         registerSlot::SPR));

    registerSpace::createRegisterSpace64(registers);


}
#endif

void registerSpace::initialize()
{
    static bool inited = false;
    
    if (inited) return;
    inited = true;

    initialize32();
#if defined(DYNINST_CODEGEN_ARCH_X86_64)
    initialize64();
#endif
}
