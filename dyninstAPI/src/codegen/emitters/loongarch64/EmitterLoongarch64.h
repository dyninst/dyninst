#ifndef _EMITTER_LOONGARCH64_H_
#define _EMITTER_LOONGARCH64_H_

#include "emitter.h"

// Minimal stub - loongarch64 emitter implementation needs to be completed
class EmitterLOONGARCH64 : public Emitter {
public:
    EmitterLOONGARCH64() {}
    virtual ~EmitterLOONGARCH64() {}

    // Implement only the most critical pure virtuals as stubs
    virtual bool clobberAllFuncCall(registerSpace *, func_instance *) { return false; }
    virtual Register emitCall(opCode, codeGen &, const std::vector<Dyninst::DyninstAPI::codeGenASTPtr> &, func_instance *) { return 0; }
    virtual void emitGetParam(Register, Register, instPoint::Type, opCode, bool, codeGen &) {}
    virtual Address getInterModuleFuncAddr(func_instance *, codeGen &) { return 0; }
    virtual Address getInterModuleVarAddr(const image_variable *, codeGen &) { return 0; }
    virtual void emitTimesImm(Register, Register, RegValue, codeGen &) {}
    virtual void emitLoad(Register, Address, int, codeGen &) {}
    virtual void emitLoadConst(Register, Address, codeGen &) {}
    virtual void emitLoadIndir(Register, Register, int, codeGen &) {}
    virtual void emitLoadOrigRegRelative(Register, Address, Register, codeGen &, bool) {}
    virtual void emitLoadOrigRegister(Address, Register, codeGen &) {}
    virtual bool emitLoadRelative(Register, Address, Register, int, codeGen &) { return false; }
    virtual void emitLoadShared(opCode, Register, const image_variable *, bool, int, codeGen &, Address) {}
    virtual void emitStore(Address, Register, int, codeGen &) {}
    virtual void emitStoreIndir(Register, Register, int, codeGen &) {}
    virtual void emitStoreRelative(Register, Address, Register, int, codeGen &) {}
    virtual void emitStoreShared(Register, const image_variable *, bool, int, codeGen &) {}
    virtual void emitAddrSpecLoad(const BPatch_addrSpec_NP *, Register, int, codeGen &) {}
    virtual void emitASload(int, int, int, long, Register, int, codeGen &) {}
    virtual void emitCountSpecLoad(const BPatch_countSpec_NP *, Register, codeGen &) {}
    virtual void emitCSload(int, int, int, long, Register, codeGen &) {}
    virtual bool emitMoveRegToReg(Register, Register, codeGen &) { return false; }
    virtual bool emitMoveRegToReg(registerSlot *, registerSlot *, codeGen &) { return false; }
    virtual codeBufIndex_t emitA(opCode, Register, long, codeGen &, Dyninst::DyninstAPI::RegControl) { return 0; }
    virtual Register emitR(opCode, Register, Register, Register, codeGen &, const instPoint *) { return 0; }
};

#endif
