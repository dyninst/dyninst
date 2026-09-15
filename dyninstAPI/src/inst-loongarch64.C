#include "registers/loongarch64_regs.h"
/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * LoongArch64 architecture-specific instrumentation functions.
 */

#include "inst-loongarch64.h"
#include "instP.h"
#include "ast.h"
#include "codegen.h"
#include "registerSpace.h"
#include "emit-loongarch64.h"
#include "codegen-loongarch64.h"
#include "common/src/arch-loongarch64.h"
#include "common/h/dyn_regs.h"
#include "addressSpace.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "instructionAPI/h/Instruction.h"

using namespace Dyninst::loongarch64;

void pushStack(codeGen &gen) {
    assert(0 && "pushStack not implemented for loongarch64");
}

void popStack(codeGen &gen) {
    assert(0 && "popStack not implemented for loongarch64");
}


// Core code emission function
codeBufIndex_t emitA(opCode op, Register src1, Register /*src2*/, long dest,
        codeGen &gen, RegControl rc, bool /*noCost*/)
{
    codeBufIndex_t retval = 0;

    switch (op) {
        case ifOp:
            retval = gen.codeEmitter()->emitIf(src1, dest, rc, gen);
            break;
        case branchOp:
            insnCodeGen::generateBranch(gen, dest);
            retval = gen.getIndex();
            break;
        default:
            assert(0 && "emitA: op not implemented for loongarch64");
    }

    return retval;
}

// Register emission function
Register emitR(opCode op, Register src1, Register src2, Register dest,
               codeGen &gen, bool /*noCost*/,
               const instPoint *, bool /*for_MT*/)
{
    switch(op) {
        case getRetValOp:
            // Return value in r4 (a0)
            return 4;
        case getParamOp:
            if (src1 <= 7) {
                // Parameters in r4-r11 (a0-a7)
                return 4 + src1;
            } else {
                int stkOffset = TRAMP_FRAME_SIZE_64 + (src1 - 8) * sizeof(long);
                insnCodeGen::restoreRegister(gen, dest, stkOffset);
                return dest;
            }
        default:
            assert(0 && "emitR: op not implemented for loongarch64");
    }
    return dest;
}

// Vector load function
void emitVload(opCode op, Address src1, Register src2, Register dest,
               codeGen &gen, bool /*noCost*/,
               registerSpace * /*rs*/, int size,
               const instPoint * /*location*/, AddressSpace *)
{
    switch(op) {
        case loadConstOp:
            gen.codeEmitter()->emitLoadConst(dest, src1, gen);
            break;
        case loadOp:
            gen.codeEmitter()->emitLoad(dest, src1, size, gen);
            break;
        case loadRegRelativeAddr:
            gen.codeEmitter()->emitLoadOrigRegRelative(dest, src1, src2, gen, false);
            break;
        case loadRegRelativeOp:
            gen.codeEmitter()->emitLoadOrigRegRelative(dest, src1, src2, gen, true);
            break;
        default:
            assert(0 && "emitVload: op not implemented for loongarch64");
    }
}

// Vector store function
void emitVstore(opCode op, Register src1, Register /*src2*/, Address dest,
        codeGen &gen, bool,
        registerSpace * /*rs*/, int size,
        const instPoint * /*location*/, AddressSpace *)
{
    if (op == storeOp) {
        gen.codeEmitter()->emitStore(dest, src1, size, gen);
    } else {
        assert(0 && "emitVstore: op not implemented for loongarch64");
    }
}

// Vector operations
void emitV(opCode op, Register src1, Register src2, Register dest,
        codeGen &gen, bool /*noCost*/,
        registerSpace * /*rs*/, int size,
        const instPoint * /*location*/, AddressSpace *proc, bool s)
{
    switch(op) {
        case storeOp:
            emitVstore(op, src1, src2, dest, gen, s, NULL, size, NULL, proc);
            break;
        case loadOp:
            emitVload(op, dest, src2, src1, gen, s, NULL, size, NULL, proc);
            break;
        default:
            assert(0 && "emitV: op not implemented for loongarch64");
    }
}

// Conditional jump (stub)
void emitJmpMC(int /*condition*/, int /*offset*/, codeGen &) {
    assert(0 && "emitJmpMC not implemented for loongarch64");
}

// Check if value fits in immediate field
bool doNotOverflow(int64_t value) {
    // LoongArch64: 12-bit signed immediate for most instructions
    if ((value >= -2048) && (value <= 2047)) return true;
    // 16-bit signed immediate for some instructions
    if ((value >= -32768) && (value <= 32767)) return true;
    return false;
}

// Emit immediate operations
void emitImm(opCode op, Register src1, RegValue src2imm, Register dest,
        codeGen &gen, bool /*noCost*/, registerSpace * /*rs*/, bool /*s*/)
{
    switch(op) {
        case plusOp:
            // ADDI dest, src1, src2imm
            assert(0 && "emitImm plusOp not implemented for loongarch64");
            break;
        case minusOp:
            // ADDI dest, src1, -src2imm
            assert(0 && "emitImm minusOp not implemented for loongarch64");
            break;
        case timesOp:
        case divOp:
        case xorOp:
        case orOp:
        case andOp:
        case eqOp:
        case neOp:
        case lessOp:
        case greaterOp:
        case leOp:
        case geOp:
            assert(0 && "emitImm: op not implemented for loongarch64");
            break;
        default:
            assert(0 && "emitImm: unknown op");
    }
}

// Dynamic call site argument extraction
bool AddressSpace::getDynamicCallSiteArgs(InstructionAPI::Instruction /*insn*/,
        Dyninst::Address /*addr*/,
        std::vector<AstNodePtr> &/*args*/)
{
    // LoongArch64: indirect calls use JIRL instruction
    // For now, return false to indicate we can't extract args
    return false;
}

// =====================================================================
// Additional includes needed for new functions
// =====================================================================
#include "dyninstAPI/src/dynProcess.h"
#include "dyninstAPI/src/binaryEdit.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/mapped_object.h"
#include "RegisterConversion.h"
#include "emitter.h"
#include "common/src/stats.h"
#include "dyninstAPI/src/baseTramp.h"
#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/src/debug.h"

extern bool isPowerOf2(int value, int &result);

#define DISTANCE(x, y)   ((x<y) ? (y-x) : (x-y))

/************************************* Register Space **************************************/

void registerSpace::initialize32() {
    assert(!"No 32-bit implementation for the LoongArch64 architecture!");
}

void registerSpace::initialize64() {
    static bool done = false;
    if (done)
        return;

    std::vector<registerSlot *> registers;

    // LoongArch64 Caller-saved GPR: r4-r11 (a0-a7), r12-r20 (t0-t8)
    for (unsigned idx = 4; idx <= 20; idx++) {
        char name[32];
        sprintf(name, "r%u", idx);
        registers.push_back(new registerSlot(idx, name, false,
                    registerSlot::deadABI, registerSlot::GPR));
    }

    // LoongArch64 Callee-saved GPR: r22-r31 (fp/s0, s1-s9)
    for (unsigned idx = 22; idx <= 31; idx++) {
        char name[32];
        sprintf(name, "r%u", idx);
        registers.push_back(new registerSlot(idx, name, false,
                    registerSlot::liveAlways, registerSlot::GPR));
    }

    // Special/readonly GPR registers
    registers.push_back(new registerSlot(0, "r0", true,     // zero
                registerSlot::liveAlways, registerSlot::GPR));
    registers.push_back(new registerSlot(1, "r1", true,     // ra
                registerSlot::liveAlways, registerSlot::GPR));
    registers.push_back(new registerSlot(2, "r2", true,     // tp
                registerSlot::liveAlways, registerSlot::GPR));
    registers.push_back(new registerSlot(3, "r3", true,     // sp
                registerSlot::liveAlways, registerSlot::GPR));
    registers.push_back(new registerSlot(21, "r21", true,   // reserved
                registerSlot::liveAlways, registerSlot::GPR));

    // FPR registers: f0-f31
    for (unsigned idx = 0; idx <= 31; idx++) {
        char name[32];
        sprintf(name, "f%u", idx);
        registers.push_back(new registerSlot(idx, name, false,
                    registerSlot::liveAlways, registerSlot::FPR));
    }

    registerSpace::createRegisterSpace64(registers);
    done = true;
}

void registerSpace::initialize() {
    initialize64();
}

/*********************************** Base Tramp ***********************************************/

bool baseTramp::generateSaves(codeGen &gen, registerSpace *) {
    assert(0 && "baseTramp::generateSaves not implemented for loongarch64");
    return false;
}

bool baseTramp::generateRestores(codeGen &gen, registerSpace *) {
    assert(0 && "baseTramp::generateRestores not implemented for loongarch64");
    return false;
}

/*********************************** Function Calls *******************************************/

Register emitFuncCall(opCode, codeGen &, std::vector<AstNodePtr> &, bool, Address) {
    // This function is not used but must exist for linkage
    return 0;
}

Register emitFuncCall(opCode op,
                      codeGen &gen,
                      std::vector<AstNodePtr> &operands, bool noCost,
                      func_instance *callee) {
    if (op != callOp) {
        cerr << "ERROR: emitFuncCall with op == " << op << endl;
    }
    assert(op == callOp);

    if (!callee) {
        char msg[256];
        sprintf(msg, "%s[%d]:  internal error:  emitFuncCall called w/out"
                " callee argument", __FILE__, __LINE__);
        showErrorCallback(80, msg);
        assert(0);
    }

    Address retAddr = 0;
    std::vector<Register> srcs;
    for (unsigned u = 0; u < operands.size(); u++) {
        Register reg = Null_Register;
        if (!operands[u]->generateCode_phase2(gen, noCost, retAddr, reg)) assert(0);
        if (reg == Null_Register) assert(0);
        srcs.push_back(reg);
    }

    Register retReg = gen.codeEmitter()->emitCall(op, gen, operands, noCost, callee);

    return retReg;
}

/*********************************** AS/CS Load ***********************************************/

void emitASload(const BPatch_addrSpec_NP *as, Register dest, int stackShift,
                codeGen &gen, bool) {
    assert(stackShift == 0);
    long int imm = as->getImm();
    int ra  = as->getReg(0);
    int rb  = as->getReg(1);
    int sc  = as->getScale();
    gen.markRegDefined(dest);

    if (ra > -1) {
        if (ra == 64) {
            // Immediate value
            // Load immediate into dest - placeholder
            assert(0 && "emitASload immediate load not implemented for loongarch64");
        } else {
            // Restore original register value
            gen.codeEmitter()->emitLoadOrigRegister(ra, dest, gen);
        }
    } else {
        assert(0 && "emitASload with no register not implemented for loongarch64");
    }

    if (rb > -1) {
        std::vector<Register> exclude;
        exclude.push_back(dest);
        Register scratch = gen.rs()->getScratchRegister(gen, exclude);
        assert(scratch != Null_Register && "cannot get a scratch register");
        gen.markRegDefined(scratch);
        gen.codeEmitter()->emitLoadOrigRegister(rb, scratch, gen);
        assert(0 && "emitASload with two registers not fully implemented for loongarch64");
    }

    if (imm) {
        assert(0 && "emitASload with offset not fully implemented for loongarch64");
    }
    return;
}

void emitCSload(const BPatch_addrSpec_NP *, Register, codeGen &, bool) {
    assert(0 && "emitCSload not implemented for loongarch64");
    return;
}

/*********************************** Misc Functions *******************************************/

int getInsnCost(opCode) {
    return 0;
}

void emitLoadPreviousStackFrameRegister(Address register_num,
                                        Register dest,
                                        codeGen &gen,
                                        int /*size*/,
                                        bool) {
    gen.codeEmitter()->emitLoadOrigRegister(register_num, dest, gen);
}

bool writeFunctionPtr(AddressSpace *p, Address addr, func_instance *f) {
    Address val_to_write = f->addr();
    return p->writeDataSpace((void *) addr, sizeof(Address), &val_to_write);
}

Emitter *AddressSpace::getEmitter() {
    static EmitterLOONGARCH64 emitter64;
    return &emitter64;
}


bool BinaryEdit::doStaticBinarySpecialCases() {
    assert(0 && "doStaticBinarySpecialCases not implemented for loongarch64");
    return false;
}

bool PCProcess::hasBeenBound(const SymtabAPI::relocationEntry &,
                             func_instance *&, Address) {
    return false;
}

bool PCProcess::bindPLTEntry(const SymtabAPI::relocationEntry &,
                             Address,
                             func_instance *,
                             Address) {
    return false;
}

bool PCProcess::createStackwalkerSteppers() {
    assert(0 && "createStackwalkerSteppers not implemented for loongarch64");
    return false;
}

bool PCProcess::getOPDFunctionAddr(Dyninst::Address &) {
    return false;
}

bool AddressSpace::getDyninstRTLibName() {
    startup_printf("dyninstRT_name: %s\n", dyninstRT_name.c_str());
    if (dyninstRT_name.length() == 0) {
        if (getenv("DYNINSTAPI_RT_LIB") != NULL) {
            dyninstRT_name = getenv("DYNINSTAPI_RT_LIB");
        } else {
            std::string msg = std::string("Environment variable ") +
                std::string("DYNINSTAPI_RT_LIB") +
                std::string(" has not been defined");
            showErrorCallback(101, msg);
            return false;
        }
    }
    return true;
}