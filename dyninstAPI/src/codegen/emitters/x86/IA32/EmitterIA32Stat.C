#include "arch-regs-x86.h"
#include "EmitterIA32Stat.h"
#include "function.h"
#include "inst-x86.h"
#include "registerSpace/RealRegister.h"
#include "registerSpace/registerSpace.h"

namespace Dyninst { namespace DyninstAPI {

  bool EmitterIA32Stat::emitCallInstruction(codeGen &gen, func_instance *callee, Register ret) {
    AddressSpace *addrSpace = gen.addrSpace();
    Address dest;

    gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EAX), gen); // caller saved regs
    gen.rs()->makeRegisterAvail(RealRegister(REGNUM_ECX), gen);
    gen.rs()->makeRegisterAvail(RealRegister(REGNUM_EDX), gen);

    // Put some dummy virtual registers in ECX and EDX so that the
    //  emitMovPCRMToReg below doesn't try to allocate them.
    //  Associate the return value into EAX now for the same reason.
    //  These shouldn't generate to any acutal code.
    Register placeholder1 = gen.rs()->allocateRegister(gen);
    Register placeholder2 = gen.rs()->allocateRegister(gen);
    gen.rs()->noteVirtualInReal(ret, RealRegister(REGNUM_EAX));
    gen.rs()->noteVirtualInReal(placeholder1, RealRegister(REGNUM_ECX));
    gen.rs()->noteVirtualInReal(placeholder2, RealRegister(REGNUM_EDX));

    // find func_instance reference in address space
    // (refresh func_map)
    std::vector<func_instance *> funcs;
    addrSpace->findFuncsByAll(callee->prettyName(), funcs);

    // test to see if callee is in a shared module
    if(gen.func()->obj() != callee->obj()) {
      emitPLTCall(callee, gen);
    } else {
      dest = callee->addr();
      Address src = gen.currAddr() + 5;
      emitCallRel32(dest - src, gen);
    }

    gen.rs()->freeRegister(placeholder1);
    gen.rs()->freeRegister(placeholder2);
    return true;
  }

  bool EmitterIA32Stat::emitPLTCall(func_instance *callee, codeGen &gen) {
    // create or retrieve jump slot
    Address dest = getInterModuleFuncAddr(callee, gen);
    // load register with address from jump slot
    emitMovPCRMToReg(RealRegister(REGNUM_EAX), dest - gen.currAddr(), gen);
    // emit call *(e_x)
    emitOpRegReg(CALL_RM_OPC1, RealRegister(CALL_RM_OPC2), RealRegister(REGNUM_EAX), gen);
    return true;
  }

  bool EmitterIA32Stat::emitPLTJump(func_instance *callee, codeGen &gen) {
    // create or retrieve jump slot
    Address dest = getInterModuleFuncAddr(callee, gen);
    // load register with address from jump slot
    emitMovPCRMToReg(RealRegister(REGNUM_EAX), dest - gen.currAddr(), gen);
    // emit jump *(e_x)
    emitOpRegReg(JUMP_RM_OPC1, RealRegister(JUMP_RM_OPC2), RealRegister(REGNUM_EAX), gen);
    return true;
  }

}}
