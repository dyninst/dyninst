#ifndef _CODEGEN_LOONGARCH64_H
#include "dyntypes.h"
#include "dyn_register.h"
using namespace Dyninst;
#define _CODEGEN_LOONGARCH64_H

#include <vector>
#include "dyntypes.h"
#include "common/src/dyn_register.h"
#include "common/src/arch-loongarch64.h"
#include "instructionAPI/h/Instruction.h"
#include "patching/patch.h"

class AddressSpace;
class codeGen;

class insnCodeGen {
public:
    enum MoveOp {
        MovOp_ORI = 0,
        MovOp_LU12I = 1,
        MovOp_LU32I = 2,
        MovOp_LU52I = 3
    };

    enum LoadStore {
        Load,
        Store
    };

    enum IndexMode {
        Post,
        Pre,
        Offset
    };

    static void generateBranch(codeGen &gen, long jump_off, bool link = false);
    static void generateBranch(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool link = false);
    static void generateCall(codeGen &gen, Address to, Register rs);
    static void generateMove(codeGen &gen, Register rt, Register rs, int shift);
    static void generateLoadStore(codeGen &gen, int size, Register rt, Register rb, int offset, LoadStore ls);

    static void generateNOOP(codeGen &gen, unsigned size = 4);

    static bool generate(codeGen &gen,
                         NS_loongarch64::instruction &insn,
                         AddressSpace *addrSpace,
                         Address origAddr,
                         Address relocAddr);

    static void saveRegister(codeGen &gen, Register r, int sp_offset, IndexMode im = Offset);
    static void restoreRegister(codeGen &gen, Register r, int sp_offset, IndexMode im = Offset);

    static void generateTrap(codeGen &gen);
    static bool modifyData(Dyninst::Address target, NS_loongarch64::instruction &insn, codeGen &gen);
    static void generateIllegal(codeGen &gen);
    static void insertTrap(codeGen &gen);

    static Register getStackPointer();
    static Register getFramePointer();
    static Register getZeroReg();

private:
    insnCodeGen();
};

#endif /* _CODEGEN_LOONGARCH64_H */

// Additional functions needed by codegen.C
